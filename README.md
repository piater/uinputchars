# uinputchars
A Linux utility to type character strings into `/dev/uinput`


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

The current implementation only accepts ISO-8859-1 characters (encoded
as specified by the locale).  Depending on the kernel keymap in use,
only a subset of them may be accessible.

The uinputchars utility needs to retrieve the kernel keymap and to
write into `/dev/uinput`; both will generally require root rights.  It
is recommended to configure this in `/etc/sudoers`.

Applications receiving events from `/dev/uinput` must be given time to
process them.  This is achieved by brief sleeps that can be adjusted
via command-line options.


## Installation

Under Linux, just run (GNU) <kbd>make</kbd>, and move or link the
resulting executable to wherever you want.  If this does not work, I
welcome your patches.
