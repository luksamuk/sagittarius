sagittarius
===========
Yet another Gemini browser for Plan 9

Author: Lucas S. Vieira <lucasvieira at protonmail dot com>


What is this?
-------------
Sagittarius is a Gemini browser for Plan 9. Gemini[1] is
a plain text protocol resembling Gopher, for a much
simpler and elegant web.

This project is HEAVILY inspired by Gopher[2], a browser
for the homonimous protocol, and Gemnine[3], a console
Gemini browser for Plan 9.

- [1] https://gemini.circumlunar.space/docs/specification.html
- [2] https://github.com/telephil9/gopher
- [3] https://git.sr.ht/~ft/gemnine


Installation
------------
NOTE: THIS APPLICATION DOES NOT WORK YET. You way want to
wait for more updates before installing it on your system.

First of all, notice that this project was designed to work
with Plan 9, more specifically with 9front. I have not
tested this with other Plan 9 distributions nor with tools
such as plan9port for example.

You can build and install with

	mk install

If you only wish to build and produce a local binary, use

	mk

...which will generate a $O.out file (where $O is a number
related to your system architecture).


Usage
-----
To open sagittarius, use

	sagittarius [address]

if address is not supplied, it will automatically load
gemini://gemini.circumlunar.space.


Extras
------
I might make some extra goodies to help on development.
And I might also include them here. Here is a list of what
you may find.

- extra/geminispec.rc:
  A small script which opens mothra on the Gemini spec.
- extra/fetchtest.c
  Study on connection over Gemini and TLS. Inspired by Gemnine.

License
-------
This project uses an MIT License, except for the libpanel
part, which was created by Tom Duff. You can read more about
it in libpanel/panel.pdf. Just plumb the file.

