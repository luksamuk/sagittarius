</$objtype/mkfile

TARG=sagittarius
LIB=libpanel/libpanel.$O.a
CFILES= \
	sagittarius.c

OFILES=${CFILES:%.c=%.$O} version.$O
HFILES=libpanel/panel.h libpanel/rtext.h
BIN=/$objtype/bin
</sys/src/cmd/mkone

CFLAGS=-Dplan9 -Ilibpanel
version.c:	$CFILES
	date|sed 's/^... //;s/ +/-/g;s/.*/char version[]="&";/' >version.c

$LIB:V:
	cd libpanel
	mk

clean nuke:V:
	@{ cd libpanel; mk $target }
	rm -f *.[$OS] [$OS].out $TARG
