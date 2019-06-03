/* Copyright 2019 Justus Piater, with one exception documented below */
/* This file is part of UINPUTCHARS.

   UINPUTCHARS is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   UINPUTCHARS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with UINPUTCHARS.  If not, see <https://www.gnu.org/licenses/>. */

/* Some documentation relevant to this code:
   man 4 console_ioctl
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <linux/input-event-codes.h>

#include "keymap.h"

#define MAX_KEYVENTS 0x100
#define LINEFEED 0x0a


/* The two functions is_a_console() and getfd() below evolved from
   code copied from kbd/src/getfd.c (GPL; Copyright (C) 1994-1999
   Andries E. Brouwer) from http://kbd-project.org/.

   This reused GPL (v2 or later) code is one reason why all parts of
   UINPUTCHARS are released under the GPL. */

static int is_a_console(int fd) {
  char arg = 0;
  return (isatty(fd)                          &&
	  ioctl(fd, KDGKBTYPE, &arg) == 0     &&
	  ((arg == KB_101) || (arg == KB_84))   );
}


static int getfd() {
  static char *conspath[] =
    { "/proc/self/fd/0",
      "/dev/tty",
      "/dev/tty0",
      "/dev/vc/0",
      "/dev/systty",
      "/dev/console",
      NULL            };

  for (int i = 0; conspath[i]; i++) {
    /* Permissions don't matter: */
    int          fd = open(conspath[i], O_RDWR);
    if (fd < 0)  fd = open(conspath[i], O_WRONLY);
    if (fd < 0)  fd = open(conspath[i], O_RDONLY);
    if (fd >= 0) {
      if (is_a_console(fd))
	return fd;
      close(fd);
    }
  }

  for (int fd = 0; fd < 3; fd++)
    if (is_a_console(fd))
      return fd;

  fprintf(stderr,
	  "Couldn't get a file descriptor referring to the console\n");
  return -1;
}


static void print_keyventry(unsigned int c, unsigned char modifiers,
			    unsigned char principal) {
  printf("[%02x=%c %02x %3u]\n", c, c, modifiers, principal);
}

static void print_keyvent(const keyvent* invkeymap, unsigned char c) {
  print_keyventry(c, invkeymap[c].modifiers, invkeymap[c].principal);
}


void print_invkeymap(const keyvent* invkeymap) {
  for (int c = 0; c < MAX_KEYVENTS; c++) {
    if (invkeymap[c].principal) {
      printf("%02x ", c);
      print_keyvent(invkeymap, c);
    }
  }
}


static void get_keys(int fd, keyvent* invkeymap) {
  /* This loop was inspired by lk_kernel_keys() 
     in kbd/src/libkeymap/kernel.c (GPL) from http://kbd-project.org/. */
  for (int table = 0; table < MAX_NR_KEYMAPS; table++) {
    for (int index = 0; index < NR_KEYS; index++) {
      struct kbentry kbe;
      kbe.kb_table = table;
      kbe.kb_index = index;
      kbe.kb_value = 0;

      if (ioctl(fd, KDGKBENT, (unsigned long)&kbe))
	fprintf(stderr, "KDGKBENT: %s: error at index %d in table %d\n",
		strerror(errno), index, table);

      if (!index && kbe.kb_value == K_NOSUCHMAP)
	break;

      if (KTYP(kbe.kb_value) == KT_LATIN ||
	  KTYP(kbe.kb_value) == KT_LETTER  ) {
	int charvalue = KVAL(kbe.kb_value);
	if (!invkeymap[charvalue].principal) {
	  /* Prefer lower-valued tables */
	  invkeymap[charvalue].modifiers = table;
	  invkeymap[charvalue].principal = index;
	}
      }
      else if (kbe.kb_value == K_ENTER &&
	       !invkeymap[LINEFEED].principal) {
	invkeymap[LINEFEED].modifiers = table;
	invkeymap[LINEFEED].principal = index;
      }
    }
  }
}


keyvent* get_invkeymap() {
  int fd = getfd();
  if (fd < 0)
    return NULL;

  keyvent* invkeymap = calloc(MAX_KEYVENTS, sizeof(*invkeymap));
  if (!invkeymap) {
    perror("invkeymap");
    return NULL;
  }

  get_keys(fd, invkeymap);
  close(fd);

  return invkeymap;
}


void release_invkeymap(keyvent* invkeymap) {
  free(invkeymap);
}
