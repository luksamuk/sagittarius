#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include <panel.h>
#include <bio.h>
#include "icons.h"

/* FORWARD DECLARATIONS */
void message(char *s, ...);


/* GLOBALS */

// Panels
Panel *root,    // Window root
	  *statusp, // Upper status bar
	  *navbarp, // Input navbar
	  *urlp,    // URL indicator
	  *textp;   // Text area

// Button images
Image *backi,
	  *fwdi,
	  *reloadi;

// Mouse handler
Mouse *mouse;

// Right-click popup options
char *popupopts[] = {
	"search",
	"exit",
	0
};

enum
{
	Msearch,
	Mexit
};


/* HELPERS */

// Update status panel
void
message(char *s, ...)
{
	static char buffer[1024];
	char *end;
	va_list args;

	// concat args in buffer
	va_start(args, s);
	end = buffer + vsnprint(buffer, sizeof buffer, s, args);
	va_end(args);
	*end = '\0';
	
	// Offload buffer contents to status panel
	plinitlabel(statusp, PACKN|FILLX, buffer);

	// Force redisplay
	pldraw(statusp, screen);
	flushimage(display, 1);
}


/* PANEL CONFIGURATION */

// Pop-up menu callback
void
popupcallb(int button, int item)
{
	USED(button);
	switch(item) {
	case Msearch: // Search
		break;
	case Mexit:
		exits(nil);
		break;
	}
}

// Back button callback
void
backcb(Panel *p, int b)
{
	USED(p);
	if(!b) return;

	// TODO
	message("Pressed back");
}

// Forward button callback
void
fwdcb(Panel *p, int b)
{
	USED(p);
	if(!b) return;

	// TODO
	message("Pressed forward");
}

// Refresh button callback
void
reloadcb(Panel *p, int b)
{
	USED(p);
	if(!b) return;

	// TODO
	message("Pressed reload");
}

void
navbarcb(Panel *p, char *t)
{
	USED(p);
	message("Command entered: %s", t);

	// Reset input and force redisplay
	plinitentry(navbarp, PACKN|FILLX, 0, "", navbarcb);
	pldraw(root, screen);
}

// Panel creation on startup
void
makepanels(void)
{
	Panel *p, *ybar, *xbar, *m;

	// Generate popup menu
	m = plmenu(0, 0, popupopts, PACKN|FILLX, popupcallb);
	root = plpopup(0, EXPAND, 0, 0, m);

	// Upper group
	p = plgroup(root, PACKN|FILLX);

	// Program label
	statusp = pllabel(p, PACKN|FILLX, "sagittarius");
	plplacelabel(statusp, PLACEW);
	// Nav buttons
	plbutton(p, PACKW|BITMAP|NOBORDER, backi, backcb);
	plbutton(p, PACKW|BITMAP|NOBORDER, fwdi, fwdcb);
	plbutton(p, PACKW|BITMAP|NOBORDER, reloadi, reloadcb);

	// Navbar with label
	pllabel(p, PACKW, "Go:");
	// TODO: Replace last zero with callbacks
	navbarp = plentry(p, PACKN|FILLX, 0, "", navbarcb);

	// URL group
	p = plgroup(root, PACKN|FILLX);

	// URL indicator
	urlp = pllabel(p, PACKN|FILLX, "");
	plplacelabel(urlp, PLACEW);

	// Text viewer group
	p = plgroup(root, PACKN|EXPAND);

	// Text viewer with scrollbar
	ybar = plscrollbar(p, PACKW|USERFL);
	xbar = plscrollbar(p, IGNORE);
	textp = pltextview(p, PACKE|EXPAND, ZP, nil, nil);
	plscroll(textp, xbar, ybar);

	// Focus on navbar
	plgrabkb(navbarp);
}


// Icon-related stuff
Image*
loadicon(Rectangle r, uchar *data, int ndata)
{
	Image *i;
	int n;

	i = allocimage(display, r, RGBA32, 0, DNofill);
	if(i == nil)
		sysfatal("allocimage: %r");
	n = loadimage(i, r, data, ndata);
	if(n < 0)
		sysfatal("loadimage: %r");
	return i;
}

void
loadicons(void)
{
	Rectangle r = Rect(0, 0, 16, 16);
	backi   = loadicon(r, ibackdata, sizeof ibackdata);
	fwdi    = loadicon(r, ifwddata, sizeof ifwddata);
	reloadi = loadicon(r, ireloaddata, sizeof ireloaddata);
}


// Libpanel callback
void
eresized(int new)
{
	if(new && getwindow(display, Refnone) < 0)
		sysfatal("cannot reattach: %r");
	plpack(root, screen->r);
	pldraw(root, screen);
}

void
main(int argc, char **argv)
{
	Event e;
	quotefmtinstall();

	if(initdraw(nil, nil, "sagittarius") < 0)
		sysfatal("initdraw: %r");
	einit(Emouse|Ekeyboard);
	plinit(screen->depth);

	loadicons();
	makepanels();

	if(argc == 2) {
		// visit argv[1]
	} else {
		// visit gopher://gemini.circumlunar.space
	}

	eresized(0);

	while(1) {
		switch(event(&e)) {
		case Ekeyboard:
			switch(e.kbdc) {
				case Kdel:
					exits(nil);
					break;
				default:
					plgrabkb(navbarp);
					plkeyboard(e.kbdc);
					break;
			}
			break;
		case Emouse:
			mouse = &e.mouse;
			plmouse(root, mouse);
			break;
		default: break;
		}
	}
	exits(0);
}
