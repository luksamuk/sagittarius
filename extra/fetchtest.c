#include <u.h>
#include <libc.h>
#include <libsec.h>
#include <stdio.h>
#include <ctype.h>
#include <bio.h>


void
main()
{
	Thumbprint *th;
	TLSconn conn;
	int fd, tlsfd, okcert;
	char buffer[1024];

	char *addr = strdup("gemini.circumlunar.space");
	char *tlsaddr =
		strdup(
		"gemini://gemini.circumlunar.space");

startreq:
	printf("Dialing\n");
	fd = dial(netmkaddr(addr, "tcp", "1965"), 0, 0, 0);
	if(fd < 0) {
		sysfatal("dial: %r");
		exits("dial");
	}

	printf("TLS connection\n");
	th = initThumbprints("/sys/lib/ssl/gemini", nil, "x509");
	memset(&conn, 0, sizeof(conn));
	tlsfd = tlsClient(fd, &conn);
	close(fd);
	if(tlsfd < 0) {
		sysfatal("tls: %r");
		exits("tls");
	}

	// Certificate
	printf("Connection\n");
	if(th != nil) {
		okcert = okCertificate(conn.cert, conn.certlen, th);
		freeThumbprints(th);
		if(!okcert) {
			//fprint(2, "echo 'x509 %r server=%s' >> /sys/lib/ssl/gemini\n", addr);
			//sysfatal("untrusted cert");
			//exits("untrusted cert");
		}
	}

	printf("TLS response\n");
	fprint(tlsfd, "%s\r\n", tlsaddr);
	int len, i;
	for(len = 0; len < sizeof(buffer) - 1; len++) {
		if((i = read(tlsfd, buffer+len, 1)) < 0) {
			sysfatal("read: %r");
			exits("read");
		}
		if(i == 0 || buffer[len] == '\n')
			break;
	}

	buffer[len] = 0;

	// Now buffer contains a whitespace-separated header.
	// Parse it.
	char *itr;
	itr = buffer;

	for(len--; len >= 0 && (itr[len] == '\r' || itr[len] == '\n'); len--) {
		itr[len] = 0;
	}

	if(!isdigit(itr[0]) || !isdigit(itr[1])) {
		sysfatal("invalid status");
		exits("status");
	}

	int status_type, status;
	status_type = 10 * (itr[0] - '0');
	status = status_type + (itr[1] - '0');
	
	while(isspace(*itr)) {
		itr++;
	}
	itr += 2;
	while(isspace(*itr)) {
		itr++;
	}
	
	char *tokstart;
	char *status_text;
	char *mime = nil;
	switch(status_type) {
	case 10: // 1x input
		status_text = "Input";
		break;
	case 20: // 2x success
		status_text = "Success";
		mime = strdup(itr[0] ? itr : "text/gemini");
		break;
	case 30: // 3x redirect
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
		printf("Redirected to %s\n", addr);
		goto startreq;
		break;
	case 40: // 4x temporary failure
		status_text = "Temporary failure";
		break;
	case 50: // 5x permanent failure
		status_text = "Permanent failure";
		break;
	case 60: // 6x client cert required
		status_text = "Client certificate required";
		break;
	default:
		sysfatal("Unknown status code %d\n", status);
		exits("status parse");
		break;
	}

	printf("Status %d: %s\n", status, status_text);

	if(status_type == 20) {
		printf("Mimetype: %s\n", mime);

		// Now get and show the body.
		// File descriptor is at tlsfd.
		char *bbuf;
		Biobuf body;

		Binit(&body, tlsfd, OREAD);
		while((bbuf = Brdstr(&body, '\n', 1)) != nil) {
			if((len = Blinelen(&body)) > 0) {
				bbuf[len] = 0;
			}
			printf("%s\n", bbuf);
		}
	}

	close(tlsfd);
	free(addr);
	free(tlsaddr);
	exits(nil);	
}
