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

#include "invkeymap.h"

struct uinput_opts_t {
  unsigned long pre_pause;	/* microseconds */
  unsigned long inter_pause;	/* microseconds */
};

int emit_string(const char* string, const keyvent* invkeymap,
		const struct uinput_opts_t* uinput_opts);
