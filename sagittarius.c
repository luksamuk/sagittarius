#include <u.h>
#include <libc.h>
#include <libsec.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include <panel.h>
#include <ctype.h>
#include <bio.h>
#include <String.h>
#include "icons.h"

/* FORWARD DECLARATIONS */
void message(char *s, ...);
void rendertext();


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

// File descriptor for current request
int reqfd;

// Mimetype for current request
char *mimetype;

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


/* CONNECTION */
int
request(char *addr)
{
	Thumbprint *th;
	TLSconn    conn;
	int fd, okcert, status_type, status;
	char buffer[1024];
	int len, i;
	char *itr, *dialaddr;

	addr = strdup(addr); // Weird. I know.
	char *tlsaddr = smprint("gemini://%s", addr);

startreq:
	// Dialing
	dialaddr = netmkaddr(addr, "tcp", "1965");
	message("Dialing %s...", dialaddr);
	fd = dial(dialaddr, 0, 0, 0);
	if(fd < 0) {
		message("dial: %r");
		goto failure;
	}

	// TLS handshake
	message("TLS handshake...");
	th = initThumbprints("/sys/lib/ssl/gemini", nil, "x509");
	memset(&conn, 0, sizeof(conn));

	{
		int oldfd = fd;
		fd = tlsClient(fd, &conn);
		close(oldfd);
	}
	if(fd < 0) {
		message("tls: %r");
		goto failure;
	}

	if(th != nil) {
		okcert = okCertificate(conn.cert, conn.certlen, th);
		freeThumbprints(th);
		if(!okcert) {
			message("warn: untrusted certificate");
			// echo 'x509 %r server=<addr> >> /sys/lib/ssl/gemini
		}
	}

	// TLS response
	message("Finishing TLS handshake...");
	fprint(fd, "%s\r\n", tlsaddr);

	message("Parsing header...");
	for(len = 0; len < sizeof(buffer) - 1; len++) {
		if((i = read(fd, buffer + len, 1)) < 0) {
			message("read: %r");
			goto failure;
		}
		if(i == 0 || buffer[len] == '\n')
			break;
	}
	buffer[len] = 0;

	itr = buffer;
	for(len--; len >= 0 &&
		  (itr[len] == '\r' || itr[len] == '\n');
		len--) {
		itr[len] = 0;
	}

	if(!isdigit(itr[0]) || !isdigit(itr[1])) {
		message("error: invalid status code");
		goto failure;
	}

	// Parse status code
	status_type = 10 * (itr[0] - '0');
	status      = status_type + (itr[1] - '0');


	// Cleanup whitespace until next field
	while(isspace(*itr)) {
		itr++;
	}
	itr += 2;
	while(isspace(*itr)) {
		itr++;
	}

	char *tokstart;
	switch(status_type) {
	case 10: // 1x input
		message("%d input", status);
		mimetype = nil;
		break;
	case 20: // 2x success
		if(mimetype != nil) free(mimetype);
		mimetype = strdup(itr[0] ? itr : "text/gemini");
		message("Success");
		break;
	case 30: // 3x redirect
		mimetype = nil;
		free(addr);
		free(tlsaddr);
		tlsaddr = strdup(itr);
		if((tokstart = strpbrk(itr, ":/")) != nil
		   && tokstart[0] == ':'
		   && tokstart[1] == '/'
		   && tokstart[2] == '/') {
			tokstart[2] = 0;
			tokstart = cleanname(tokstart + 3);
			addr = strdup(tokstart);
		} else addr = tlsaddr;
		message("Redirecting to %s...", addr);
		goto startreq;
		break;
	case 40: // 4x temporary failure
		mimetype = nil;
		message("%d temporary failure", status);
		break;
	case 50: // 5x permanent failure
		mimetype = nil;
		message("%d permanent failure", status);
		break;
	default:
		mimetype = nil;
		message("Unknown status code %d", status);
		break;
	}

failure:
	free(addr);
	free(tlsaddr);
	return fd;
}

void
visit(char *addr)
{
	message("Visiting %s...", addr);
	reqfd = request(addr);
	message("Rendering...");
	rendertext();
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
	//message("Command entered: %s", t);

	// If not a command, then...
	visit(t);

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

void
rendertext()
{
	if((reqfd < 0) ||
       (mimetype == nil) ||
       (strncmp(mimetype, "text/", 5) != 0)) {
		return;
	}
	message("Rendering...");
	
	String *buffer;
	Rtext *text = nil;
	Biobuf bp; // Open using file descriptor
	char c;
	int n;

	Binit(&bp, reqfd, OREAD);

	plrtstr(&text, 1000000, 0, 0, font, strdup(" "), 0, 0);
	buffer = s_new();
	n = 0;
	while(1) {
		c = Bgetc(&bp);
		if(c < 0) {
			break;
		} else if (c == '\n') {
			s_terminate(buffer);
			plrtstr(&text, 1000000, 0, 0, font, strdup(s_to_c(buffer)), 0, 0);
			s_reset(buffer);
		} else {
			s_putc(buffer, c);
		}
	}
	s_terminate(buffer);
	plrtstr(&text, 1000000, 0, 0, font, strdup(s_to_c(buffer)), 0, 0);
	s_free(buffer);

	plinittextview(textp, PACKE|EXPAND, ZP, text, nil);
	pldraw(textp, screen);

	//plinitlabel(urlp, PACKN|FILLX, url);
	
	close(reqfd);
	message("sagittarius!");
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
	reqfd = -1;
	mimetype = nil;
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
		// visit gemini://gemini.circumlunar.space
		visit("gemini.circumlunar.space");
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
