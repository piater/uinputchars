# uinputchars
A Linux utility to type character strings into `/dev/uinput`


## Abandonment/Deprecation Notice

I recently stumbled upon [wtype](https://github.com/atx/wtype), which
first appeared on GitHub just after I created uinputchars. For my only
use case typing credentials into various programs running under
Wayland, this is a much better tool – no need for special permissions,
inverse keymaps, or milli-sleeps. It works flawlessly as a drop-in
replacement for uinputchars.

Thus I declare uinputchars dormant and discourage its use (unless it
solves a problem that xdotool/wtype cannot solve for you).


## Usage

A typical use case is [entering
passwords](https://github.com/clawoflight/puma). In contrast to
classical password managers, uinputchars can type character strings
into anything - Web forms, e-mail clients, Emacs - without any client
support.  In contrast to xdotool, it does not require X; it also works
under Wayland or even on the console.

Generally, uinputchars is run without arguments.

Character strings are read from standard input.  There is currently no
option to pass them on the command line because this would make them
appear in the process table.


### Notes

  * Applications receiving events from `/dev/uinput` must be given
	time to process them.  This is achieved by brief sleeps that can
	be adjusted via command-line options.

  * This utility reads the console keymap (as activated with
	loadkeys).  Thus, to work as expected, this should match the
	keyboard layout used by the desktop environment, if any (under
	Wayland or X).

	In particular, the `uinput` device is not the same as the real
	keyboard and may have a different keymap associated with it.  For
	example, in [sway](https://github.com/swaywm/sway), configure all
	inputs to use the same keymap (e.g., `input "*" xkb_layout de`),
	not just the actual keyboard, to include `/dev/uinput`.

  * The current implementation only accepts ISO-8859-1 characters
	(encoded as specified by the locale).  Depending on the console
	keymap in use, only a subset of them may be accessible.

  * Retrieving the console keymap and writing into `/dev/uinput` will
	generally require root rights. It is recommended to configure this
	in `/etc/sudoers`.


## Installation

Arch Linux users can install from the
[AUR](https://aur.archlinux.org/packages/uinputchars/).

Generally under Linux, just run (GNU) <kbd>make</kbd>, and move or link the
resulting executable to wherever you want.  If this does not work, I
welcome your patches.
