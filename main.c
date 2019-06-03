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

#include <getopt.h>
#include <stdio.h>
#include <iconv.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include "keymap.h"
#include "uinput.h"


static void print_help(const char* argv0, struct uinput_opts_t* uinput_opts) {
  printf("Usage: %s [options]\n"
	 "Reads characters from stdin and types them into /dev/uinput.\n"
	 "Currently limited to ISO-8859-1 characters.  Depending on the "
	 "currently-active\nkernel keymap, only a subset of these may be "
	 "available.\nMust typically be run as root.\n"
	 "Options:\n"
	 "-h --help         display this help and exit\n"
	 "-p --pre-pause    milliseconds to wait after /dev/uinput setup "
	 "(%lu ms)\n"
	 "-i --inter-pause  milliseconds to wait between characters typed "
	 "(%lu ms)\n"
	 "-d --dump         display inverse keymap and exit\n",
	 argv0,
	 uinput_opts->pre_pause / 1000, uinput_opts->inter_pause / 1000);
}


int main(int argc, char* argv[]) {
  struct uinput_opts_t uinput_opts = { 200000, 10000 };
  int dump = 0;

  static struct option options[] =
    { { "help",        no_argument,       NULL, 'h' },
      { "pre-pause",   required_argument, NULL, 'p' },
      { "inter-pause", required_argument, NULL, 'i' },
      { "dump",        no_argument,       NULL, 'd' },
      { NULL,          0,                 NULL,  0  } };
  while (1) {
    int c = getopt_long(argc, argv, "hp:i:d", options, NULL);
    if (c == -1)
      break;
    switch (c) {
    case 'h':
      print_help(argv[0], &uinput_opts);
      exit(0);
    case 'p':
      uinput_opts.pre_pause = strtoul(optarg, NULL, 10) * 1000;
      break;
    case 'i':
      uinput_opts.inter_pause = strtoul(optarg, NULL, 10) * 1000;
      break;
    case 'd':
      dump = 1;
      break;
    case '?':
      exit(1);
    default:
      fprintf(stderr, "unrecognized option: %c\n", c);
      exit(1);
    }
  }
  
  keyvent* invkeymap = get_invkeymap();
  if (!invkeymap)
    exit(2);

  if (dump) {
    print_invkeymap(invkeymap);
    release_invkeymap(invkeymap);
    exit(0);
  }

  int result = 0;
  
  setlocale(LC_CTYPE, "");
  iconv_t cd = iconv_open("ISO-8859-1", "");

  const int bufsize = 1024;
  char localebuf[bufsize];
  while (fgets(localebuf, bufsize, stdin)) {
    char latin1buf[1024];
    size_t inbytesleft = strlen(localebuf);
    size_t outbytesleft = bufsize;
    char* inbuf = localebuf;
    char* outbuf = latin1buf;
    size_t converted = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    *outbuf = 0;
    if (converted == (size_t)-1) {
      perror("Error: iconv");
      result = 3;
      break;
    }

    if (emit_string(latin1buf, invkeymap, &uinput_opts)) {
      result = 4;
      break;
    }
  }

  iconv_close(cd);
  release_invkeymap(invkeymap);
  exit(result);
}
