sagittarius
===========
Yet another Gemini browser for Plan 9

Lucas S. Vieira <lucasvieira@protonmail.com>

What is this?
-------------
Sagittarius is a Gemini browser for Plan 9. Gemini is a
plain text protocol resembling Gopher, for a much simpler
and elegant web.

This project is HEAVILY inspired by Gopher[1], a browser
for the homonimous protocol, and Gemnine[2], a
console-only Gemini browser for Plan 9.

[1] https://github.com/telephil9/gopher
[2] https://git.sr.ht/~ft/gemnine

Installation
------------
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

License
-------
This project uses an MIT License, except for the libpanel
part, which was created by Tom Duff. You can read more about
it in libpanel/panel.pdf. Just plumb the file.

