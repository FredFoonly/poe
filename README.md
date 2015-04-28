# poe
Small and quick curses programmer's editor, modeled after the IBM PE2 editor.

Currently it's roughly functionally equivalent to PE2.  The .dir file isn't yet supported, though .keys and .unknown are.  As I get more time to work on it I'll push it farther along.  Profile handling will be changing quite a bit.  I want to do undo in a different way than PE2 did it and different from the way other programmers editors do it (with an undo/redo stack);  both methods have their advantages and I'd like to get the best of both worlds.

This version has been tested on OpenBSD x86-64, FreeBSD x86-64, and Linux ARM (Debian/Raspbian Raspberry Pi 2).
