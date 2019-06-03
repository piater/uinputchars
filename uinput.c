/* Copyright 2019 Justus Piater */
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
   https://www.kernel.org/doc/html/v5.1/input/uinput.html */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <linux/uinput.h>

#include "uinput.h"


static void emit(int fd, int type, int code, int val)
{
  struct input_event ie = {};
  ie.type = type;
  ie.code = code;
  ie.value = val;

  if (write(fd, &ie, sizeof(ie)) < sizeof(ie))
    perror("emit");
}


int emit_string(const char* string, const keyvent* invkeymap,
		const struct uinput_opts_t* uinput_opts) {
  int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (fd < 0) {
    perror("open");
    return -1;
  }

  ioctl(fd, UI_SET_EVBIT, EV_KEY);
  int nchars = strlen(string);
  for (int i = 0; i < nchars; i++) {
    unsigned char principal =
      invkeymap[(int)(unsigned char)string[i]].principal;
    if (principal)
      ioctl(fd, UI_SET_KEYBIT, principal);
    else {
      fprintf(stderr, "Error: no key for character 0x%02x=%c\n",
	      (unsigned char)string[i], string[i]);
      close(fd);
      return -1;
    }
  }
  ioctl(fd, UI_SET_KEYBIT, KEY_LEFTSHIFT);
  ioctl(fd, UI_SET_KEYBIT, KEY_RIGHTALT);

  struct uinput_setup usetup = {};
  strcpy(usetup.name, "Synthesized Keyboard Input");
  ioctl(fd, UI_DEV_SETUP, &usetup);
  ioctl(fd, UI_DEV_CREATE);

  usleep(uinput_opts->pre_pause);

  for (int i = 0; i < nchars; i++) {
    const keyvent* kev = invkeymap + (unsigned char)string[i];

    if (kev->modifiers & K_SHIFTTAB)
      emit(fd, EV_KEY, KEY_LEFTSHIFT, 1);
    if (kev->modifiers & K_ALTTAB)
      emit(fd, EV_KEY, KEY_RIGHTALT, 1);

    emit(fd, EV_KEY, kev->principal, 1);
    emit(fd, EV_KEY, kev->principal, 0);

    if (kev->modifiers & K_ALTTAB)
      emit(fd, EV_KEY, KEY_RIGHTALT, 0);
    if (kev->modifiers & K_SHIFTTAB)
      emit(fd, EV_KEY, KEY_LEFTSHIFT, 0);

    emit(fd, EV_SYN, SYN_REPORT, 0);
    usleep(uinput_opts->inter_pause);
  }

  ioctl(fd, UI_DEV_DESTROY);
  close(fd);
  return 0;
}
