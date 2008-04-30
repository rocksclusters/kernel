/*
 * ftp.c - ftp code
 *
 * Erik Troan <ewt@redhat.com>
 * Matt Wilson <msw@redhat.com>
 * Jeremy Katz <katzj@redhat.com>
 *
 * Copyright 1997 - 2002 Red Hat, Inc.
 *
 * This software may be freely redistributed under the terms of the GNU
 * General Public License.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */



#define HAVE_ALLOCA_H 1
#define HAVE_NETINET_IN_SYSTM_H 1
#define HAVE_SYS_SOCKET_H 1
#define USE_ALT_DNS 1

#if HAVE_ALLOCA_H
# include <alloca.h>
#endif

#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#if HAVE_NETINET_IN_SYSTM_H
# include <sys/types.h>
# include <netinet/in_systm.h>
#endif

#if ! HAVE_HERRNO
extern int h_errno;
#endif

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define TIMEOUT_SECS 60
#define BUFFER_SIZE 4096

#ifndef IPPORT_FTP
# define IPPORT_FTP 21
#endif

#if defined(USE_ALT_DNS) && USE_ALT_DNS 
#include "../isys/dns.h"
#endif

#include "ftp.h"
#ifdef	SDSC
#include "log.h"
#endif

static int ftpCheckResponse(int sock, char ** str);
static int ftpCommand(int sock, char * command, ...);
static int getHostAddress(const char * host, struct in_addr * address);
#ifdef	SDSC
void watchdog_reset(void);
#endif

static int ftpCheckResponse(int sock, char ** str) {
    static char buf[BUFFER_SIZE + 1];
    int bufLength = 0; 
    fd_set emptySet, readSet;
    char * chptr, * start;
    struct timeval timeout;
    int bytesRead, rc = 0;
    int doesContinue = 1;
    char errorCode[4];
 
    errorCode[0] = '\0';
    
    do {
	FD_ZERO(&emptySet);
	FD_ZERO(&readSet);
	FD_SET(sock, &readSet);

	timeout.tv_sec = TIMEOUT_SECS;
	timeout.tv_usec = 0;
    
	rc = select(sock + 1, &readSet, &emptySet, &emptySet, &timeout);
	if (rc < 1) {
	    if (rc==0) 
		return FTPERR_BAD_SERVER_RESPONSE;
	    else
		rc = FTPERR_UNKNOWN;
	} else
	    rc = 0;

	bytesRead = read(sock, buf + bufLength, sizeof(buf) - bufLength - 1);

	bufLength += bytesRead;

	buf[bufLength] = '\0';

	/* divide the response into lines, checking each one to see if 
	   we are finished or need to continue */

	start = chptr = buf;

	do {
	    while (*chptr != '\n' && *chptr) chptr++;

	    if (*chptr == '\n') {
		*chptr = '\0';
		if (*(chptr - 1) == '\r') *(chptr - 1) = '\0';
		if (str) *str = start;

		if (errorCode[0]) {
		    if (!strncmp(start, errorCode, 3) && start[3] == ' ')
			doesContinue = 0;
		} else {
		    strncpy(errorCode, start, 3);
		    errorCode[3] = '\0';
		    if (start[3] != '-') {
			doesContinue = 0;
		    } 
		}

		start = chptr + 1;
		chptr++;
	    } else {
		chptr++;
	    }
	} while (*chptr);

	if (doesContinue && chptr > start) {
	    memcpy(buf, start, chptr - start - 1);
	    bufLength = chptr - start - 1;
	} else {
	    bufLength = 0;
	}
    } while (doesContinue && !rc);

    if (*errorCode == '4' || *errorCode == '5') {
	if (!strncmp(errorCode, "550", 3)) {
	    return FTPERR_FILE_NOT_FOUND;
	}

	return FTPERR_BAD_SERVER_RESPONSE;
    }

    if (rc) return rc;

    return 0;
}

int ftpCommand(int sock, char * command, ...) {
    va_list ap;
    int len;
    char * s;
    char * buf;
    int rc;

    va_start(ap, command);
    len = strlen(command) + 2;
    s = va_arg(ap, char *);
    while (s) {
	len += strlen(s) + 1;
	s = va_arg(ap, char *);
    }
    va_end(ap);

    buf = alloca(len + 1);

    va_start(ap, command);
    strcpy(buf, command);
    strcat(buf, " ");
    s = va_arg(ap, char *);
    while (s) {
	strcat(buf, s);
	strcat(buf, " ");
	s = va_arg(ap, char *);
    }
    va_end(ap);

    buf[len - 2] = '\r';
    buf[len - 1] = '\n';
    buf[len] = '\0';
     
    if (write(sock, buf, len) != len) {
        return FTPERR_SERVER_IO_ERROR;
    }

    if ((rc = ftpCheckResponse(sock, NULL)))
	return rc;

    return 0;
}


static int getHostAddress(const char * host, struct in_addr * address) {
    if (isdigit(host[0])) {
      if (!inet_aton(host, address)) {
	  return FTPERR_BAD_HOST_ADDR;
      }
    } else {
      if (mygethostbyname((char *) host, address)) {
	  errno = h_errno;
	  return FTPERR_BAD_HOSTNAME;
      }
    }
    
    return 0;
}

int ftpOpen(char * host, char * name, char * password, char * proxy,
	    int port) {
    static int sock;
    /*static char * lastHost = NULL;*/
    struct in_addr serverAddress;
    struct sockaddr_in destPort;
    struct passwd * pw;
    char * buf;
    int rc;

    if (port < 0) port = IPPORT_FTP;

    if (!name)
	name = "anonymous";

    if (!password) {
	password = "root@";
	if (getuid()) {
	    pw = getpwuid(getuid());
	    if (pw) {
		password = alloca(strlen(pw->pw_name) + 2);
		strcpy(password, pw->pw_name);
		strcat(password, "@");
	    }
	}
    }

    if (proxy) {
	buf = alloca(strlen(name) + strlen(host) + 5);
	sprintf(buf, "%s@%s", name, host);
	name = buf;
	host = proxy;
    }

    if ((rc = getHostAddress(host, &serverAddress))) return rc;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        return FTPERR_FAILED_CONNECT;
    }

    destPort.sin_family = AF_INET;
    destPort.sin_port = htons(port);
    destPort.sin_addr = serverAddress;

    if (connect(sock, (struct sockaddr *) &destPort, sizeof(destPort))) {
	close(sock);
        return FTPERR_FAILED_CONNECT;
    }

    /* ftpCheckResponse() assumes the socket is nonblocking */
    if (fcntl(sock, F_SETFL, O_NONBLOCK)) {
	close(sock);
        return FTPERR_FAILED_CONNECT;
    }

    if ((rc = ftpCheckResponse(sock, NULL))) {
        return rc;     
    }

    if ((rc = ftpCommand(sock, "USER", name, NULL))) {
	close(sock);
	return rc;
    }

    if ((rc = ftpCommand(sock, "PASS", password, NULL))) {
	close(sock);
	return rc;
    }

    if ((rc = ftpCommand(sock, "TYPE", "I", NULL))) {
	close(sock);
	return rc;
    }

    return sock;
}

int ftpGetFileDesc(int sock, char * remotename) {
    int dataSocket;
    struct sockaddr_in dataAddress;
    int i, j;
    char * passReply;
    char * chptr;
    char * retrCommand;
    int rc;

    if (write(sock, "PASV\r\n", 6) != 6) {
        return FTPERR_SERVER_IO_ERROR;
    }
    if ((rc = ftpCheckResponse(sock, &passReply)))
	return FTPERR_PASSIVE_ERROR;

    chptr = passReply;
    while (*chptr && *chptr != '(') chptr++;
    if (*chptr != '(') return FTPERR_PASSIVE_ERROR; 
    chptr++;
    passReply = chptr;
    while (*chptr && *chptr != ')') chptr++;
    if (*chptr != ')') return FTPERR_PASSIVE_ERROR;
    *chptr-- = '\0';

    while (*chptr && *chptr != ',') chptr--;
    if (*chptr != ',') return FTPERR_PASSIVE_ERROR;
    chptr--;
    while (*chptr && *chptr != ',') chptr--;
    if (*chptr != ',') return FTPERR_PASSIVE_ERROR;
    *chptr++ = '\0';
    
    /* now passReply points to the IP portion, and chptr points to the
       port number portion */

    dataAddress.sin_family = AF_INET;
    if (sscanf(chptr, "%d,%d", &i, &j) != 2) {
	return FTPERR_PASSIVE_ERROR;
    }
    dataAddress.sin_port = htons((i << 8) + j);

    chptr = passReply;
    while (*chptr++) {
	if (*chptr == ',') *chptr = '.';
    }

    if (!inet_aton(passReply, &dataAddress.sin_addr)) 
	return FTPERR_PASSIVE_ERROR;

    dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (dataSocket < 0) {
        return FTPERR_FAILED_CONNECT;
    }

    retrCommand = alloca(strlen(remotename) + 20);
    sprintf(retrCommand, "RETR %s\r\n", remotename);
    i = strlen(retrCommand);
   
    if (write(sock, retrCommand, i) != i) {
        return FTPERR_SERVER_IO_ERROR;
    }

    if (connect(dataSocket, (struct sockaddr *) &dataAddress, 
	        sizeof(dataAddress))) {
	close(dataSocket);
        return FTPERR_FAILED_DATA_CONNECT;
    }

    if ((rc = ftpCheckResponse(sock, NULL))) {
	close(dataSocket);
	return rc;
    }

    return dataSocket;
}

int ftpGetFileDone(int sock) {
    if (ftpCheckResponse(sock, NULL)) {
	return FTPERR_BAD_SERVER_RESPONSE;
    }

    return 0;
}

const char *ftpStrerror(int errorNumber) {
  switch (errorNumber) {
    case FTPERR_BAD_SERVER_RESPONSE:
      return ("Bad FTP server response");

    case FTPERR_SERVER_IO_ERROR:
      return("FTP IO error");

    case FTPERR_SERVER_TIMEOUT:
      return("FTP server timeout");

    case FTPERR_BAD_HOST_ADDR:
      return("Unable to lookup FTP server host address");

    case FTPERR_BAD_HOSTNAME:
      return("Unable to lookup FTP server host name");

    case FTPERR_FAILED_CONNECT:
      return("Failed to connect to FTP server");

    case FTPERR_FAILED_DATA_CONNECT:
      return("Failed to establish data connection to FTP server");

    case FTPERR_FILE_IO_ERROR:
      return("IO error to local file");

    case FTPERR_PASSIVE_ERROR:
      return("Error setting remote server to passive mode");

    case FTPERR_FILE_NOT_FOUND:
      return("File not found on server");

#ifdef SDSC
	case FTPERR_REFUSED:
		return ("Authentication Credentials Refused");
	case FTPERR_CLIENT_SECURITY:
		return ("Error setting up Client Security Credentials");
	case FTPERR_SERVER_SECURITY:
		return ("Could not get Access to Server");
#endif

    case FTPERR_UNKNOWN:
    default:
      return("FTP Unknown or unexpected error");
  }
}

/* extraHeaders is either NULL or a string with extra headers separated by '\r\n', ending with
 * '\r\n'
 */
int httpGetFileDesc(char * hostname, int port, char * remotename, char *extraHeaders) {
    char * buf;
#ifndef	SDSC
    struct timeval timeout;
#endif
    char headers[4096];
    char * nextChar = headers;
    char *realhost;
    char *hstr;
    int checkedCode;
    struct in_addr serverAddress;
    int sock;
    int rc;
    struct sockaddr_in destPort;
#ifndef	SDSC
    fd_set readSet;
#endif
#ifdef	SDSC
    int	bufsize;
    int byteswritten;
#endif

    realhost = hostname;
    if (port < 0) {
	char *colonptr = strchr(hostname, ':');
	if (colonptr != NULL) {
	    int realhostlen = colonptr - hostname;
	    port = atoi(colonptr + 1);
	    realhost = alloca (realhostlen + 1);
	    memcpy (realhost, hostname, realhostlen);
	    realhost[realhostlen] = '\0';
	} else {
	    port = 80;
	}
    } 

#ifdef SDSC
	rc = getHostAddress(hostname, &serverAddress);
	if (rc) {
		logMessage("ROCKS:httpGetFileDesc:1");
		return FTPERR_BAD_HOST_ADDR;
	}
#else
    if ((rc = getHostAddress(realhost, &serverAddress))) return rc;
#endif

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        return FTPERR_FAILED_CONNECT;
    }

    destPort.sin_family = AF_INET;
    destPort.sin_port = htons(port);
    destPort.sin_addr = serverAddress;

    if (connect(sock, (struct sockaddr *) &destPort, sizeof(destPort))) {
	close(sock);
        return FTPERR_FAILED_CONNECT;
    }

    if (extraHeaders)
	hstr = extraHeaders;
    else
	hstr = "";

#ifdef	SDSC
    bufsize = strlen(remotename) + strlen(hostname) + strlen(hstr) + 30;

    if ((buf = malloc(bufsize)) == NULL) {
		logMessage("ROCKS:httpGetFileDesc:malloc failed");
		return FTPERR_FAILED_CONNECT;
    }
#else
    buf = alloca(strlen(remotename) + strlen(realhost) + strlen(hstr) + 25);
#endif

    sprintf(buf, "GET %s HTTP/1.0\r\nHost: %s\r\n%s\r\n", remotename, realhost, hstr);

#ifdef	SDSC
	byteswritten = write(sock, buf, strlen(buf));

	logMessage("ROCKS:httpGetFileDesc:byteswritten(%d)", byteswritten);
	logMessage("ROCKS:httpGetFileDesc:bufsize(%d)", (int)strlen(buf));
#else
    write(sock, buf, strlen(buf));
#endif

#ifdef	SDSC
    free(buf);
#endif

    /* This is fun; read the response a character at a time until we:

	1) Get our first \r\n; which lets us check the return code
	2) Get a \r\n\r\n, which means we're done */

    *nextChar = '\0';
    checkedCode = 0;
    while (!strstr(headers, "\r\n\r\n")) {
#ifndef	SDSC
	FD_ZERO(&readSet);
	FD_SET(sock, &readSet);

	timeout.tv_sec = TIMEOUT_SECS;
	timeout.tv_usec = 0;
    
	rc = select(sock + 1, &readSet, NULL, NULL, &timeout);
	if (rc == 0) {
	    close(sock);
	    return FTPERR_SERVER_TIMEOUT;
	} else if (rc < 0) {
	    close(sock);
	    return FTPERR_SERVER_IO_ERROR;
	}
#endif

	if (read(sock, nextChar, 1) != 1) {
	    close(sock);
	    return FTPERR_SERVER_IO_ERROR;
	}

	nextChar++;
	*nextChar = '\0';

	if (nextChar - headers == sizeof(headers)) {
	    close(sock);
	    return FTPERR_SERVER_IO_ERROR;
	}

	if (!checkedCode && strstr(headers, "\r\n")) {
	    char * start, * end;

	    checkedCode = 1;
	    start = headers;
	    while (!isspace(*start) && *start) start++;
	    if (!*start) {
		close(sock);
		return FTPERR_SERVER_IO_ERROR;
	    }

#ifdef  SDSC
            while (isspace(*start) && *start) start++;
#else   
	    start++;
#endif

	    end = start;
	    while (!isspace(*end) && *end) end++;
	    if (!*end) {
		close(sock);
		return FTPERR_SERVER_IO_ERROR;
	    }

	    *end = '\0';
	    if (!strcmp(start, "404")) {
		close(sock);
		return FTPERR_FILE_NOT_FOUND;
#ifdef SDSC
	} else if (!strcmp(start, "503")) {
		/* A server nack - busy */
		close(sock);
		logMessage("ROCKS:server busy");
		watchdog_reset();
		return FTPERR_FAILED_DATA_CONNECT;
#endif
	    } else if (strcmp(start, "200")) {
		close(sock);
		return FTPERR_BAD_SERVER_RESPONSE;
	    }

	    *end = ' ';
	}
    }
    
    return sock;
}

#ifdef SDSC

#include <openssl/ssl.h>
#include <newt.h>
#include "lang.h"


/* From urlinstall.c */
extern struct loaderData_s * rocks_global_loaderData;

/* Knows how to show a certificate's name list. Does some
 * label mapping to look nice, but does not effectively wrap
 * long names.
 * 
 * Perhaps these two newt-aware functions could be put in another file.
 */

static newtGrid
show_names(X509_NAME *name)
{
	char value[1024];
	const char *element;
	char *label;
	int i, nameCount, nid;
	X509_NAME_ENTRY *entry;
	newtGrid grid;

	nameCount = X509_NAME_entry_count(name);

	grid = newtCreateGrid(2, nameCount+1);

	for (i=0; i<nameCount; i++) {
		entry = X509_NAME_get_entry(name, i);
		if (!entry) {
			logMessage("Could not find a certificate name");
			return NULL;
		}
		nid = OBJ_obj2nid(entry->object);
		element = OBJ_nid2sn(nid);
		switch (*element) {
			case 'C':
				if (!strcmp(element, "CN"))
					label = "Name";
				else
					label = "Country";
				break;
			case 'S':
				label = "State";
				break;
			case 'L':
				label = "Locality";
				break;
			case 'e':
			case 'E':
				label = "Contact";
				break;
			case 'O':
				if (!strcmp(element, "OU"))
					label = "Org Unit";
				else
					label = "Org";
				break;
			default:
				label = (char*) element;
		}
		snprintf(value, sizeof(value), "%*s",
			entry->value->length,
			entry->value->data);

		newtGridSetField(grid, 0, i+1, NEWT_GRID_COMPONENT,
			newtLabel(-1,-1, _(label)), 0, 0, 0, 0,
			NEWT_ANCHOR_LEFT, 0);

		newtGridSetField(grid, 1, i+1, NEWT_GRID_COMPONENT,
			newtLabel(-1,-1, _(value)), 2, 0, 0, 0,
			NEWT_ANCHOR_LEFT, NEWT_FLAG_WRAP);
	}
	return grid;
}


/* Constructs newt screens showing server certificate credentials.
 * Returns 1 if user says ok, 2 if not.
 */
static int
show_cert (X509 *cert)
{
	X509_NAME *name;
	newtGrid grid, serverGrid, issuerGrid, certGrid, buttons;
	newtComponent ok, cancel, form, answer;

	grid = newtCreateGrid(1,2);
	certGrid = newtCreateGrid(2,2);

	/* Subject */
	name = X509_get_subject_name(cert);

	serverGrid = show_names(name);

	newtGridSetField(certGrid, 0,0, NEWT_GRID_COMPONENT,
		newtLabel(-1,-1, _("Installation Server:")),
		2, 0, 0, 0,  NEWT_ANCHOR_LEFT | NEWT_ANCHOR_TOP, 0);

	/* Issuer (CA) */
	name = X509_get_issuer_name(cert);

	issuerGrid = show_names(name);

	newtGridSetField(certGrid, 1,0, NEWT_GRID_COMPONENT,
		newtLabel(-1,-1, _("Certified by:")),
		2, 0, 0, 0,  NEWT_ANCHOR_LEFT | NEWT_ANCHOR_TOP, 0);

	/* Setup screen */
	buttons = newtButtonBar(_("Proceed"), &ok, _("Cancel"), &cancel, NULL);


	newtGridSetField(certGrid, 0, 1, NEWT_GRID_SUBGRID, serverGrid,
		0, 0, 2, 0, NEWT_ANCHOR_LEFT | NEWT_ANCHOR_TOP, 0);
	newtGridSetField(certGrid, 1, 1, NEWT_GRID_SUBGRID, issuerGrid,
		0, 0, 0, 0, NEWT_ANCHOR_RIGHT | NEWT_ANCHOR_TOP, 0);

	newtGridSetField(grid, 0, 0, NEWT_GRID_SUBGRID, certGrid,
		0, 0, 0, 1, NEWT_ANCHOR_LEFT,
		NEWT_GRID_FLAG_GROWX);

	newtGridSetField(grid, 0, 1, NEWT_GRID_SUBGRID, buttons,
		0, 0, 0, 0, 0, NEWT_GRID_FLAG_GROWX);

	newtGridWrappedWindow(grid, _("Authenticate Installation Server"));
	form = newtForm(NULL, NULL, 0);
	newtGridAddComponentsToForm(grid, form, 1);

	answer = newtRunForm(form);
	newtPopWindow();
	if (answer == ok)
		return 1;
	else if (answer == cancel)
		return 2;

	return -1;
}


/* 
 * We have torn up this file. This function uses the OpenSSL
 * library to initiate an HTTPS connection. Used to retrieve
 * a Rocks kickstart file.
 */

BIO *
httpsGetFileDesc(char * hostname, int port, char * remotename,
	char *extraHeaders, int *errorcode) 
{
	char * buf;
	char headers[4096];
	char * nextChar = headers;
	char *hstr;
	int sock;
	int rc;
	int checkedCode;

	int bufsize;
	int byteswritten;
	struct loaderData_s * loaderData;
	SSL_CTX *ssl_context;
	SSL *ssl;
	BIO *sbio = 0;
	X509 *server_cert;

	*errorcode = 0;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sock < 0) {
		logMessage("ROCKS:httpsGetFileDesc:Could not get a socket");
		*errorcode = FTPERR_FAILED_CONNECT;
		return NULL;
	}

	OpenSSL_add_all_algorithms();

	ssl_context = SSL_CTX_new(SSLv23_client_method());
	if (!ssl_context) {
		logMessage("Could not create SSLv2,3 context");
		*errorcode = FTPERR_FAILED_CONNECT;
		goto error;
	}

	/* Pull in the Global Loader Data structure. */
	loaderData = rocks_global_loaderData;

	/* I have a Certificate */
	if (loaderData->cert_filename) {
		rc = SSL_CTX_use_certificate_file(ssl_context, 
			loaderData->cert_filename,
			SSL_FILETYPE_PEM);
		if (!rc) {
			logMessage("Could not read Cluster Certificate");
			*errorcode = FTPERR_CLIENT_SECURITY;
			goto error;
		}

		rc = SSL_CTX_use_PrivateKey_file(ssl_context, 
			loaderData->priv_filename,
			SSL_FILETYPE_PEM);
		if (!rc) {
			logMessage("Could not read Cluster cert private key");
			*errorcode = FTPERR_CLIENT_SECURITY;
			goto error;
		}

		/* Only connect to servers that have certs signed by
		 * our trusted CA. */
		if (loaderData->authParent) {
			rc = SSL_CTX_load_verify_locations(ssl_context,
				loaderData->ca_filename, 0);
			if (!rc) {
				logMessage("Could not read Server CA cert");
				*errorcode = FTPERR_CLIENT_SECURITY;
				goto error;
			}
			SSL_CTX_set_verify(ssl_context, SSL_VERIFY_PEER, 0);
			SSL_CTX_set_verify_depth(ssl_context, 1);
		}
	}

	sbio = BIO_new_ssl_connect(ssl_context);
	if (!sbio) {
		logMessage("Could not create SSL object");
		*errorcode = FTPERR_CLIENT_SECURITY;
		goto error;
	}

	BIO_get_ssl(sbio, &ssl);
	if (!ssl) {
		logMessage("Could not find ssl pointer.");
		*errorcode = FTPERR_CLIENT_SECURITY;
		goto error;
	}

	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

	BIO_set_conn_hostname(sbio, hostname);
	BIO_set_conn_port(sbio, "https");

	rc = BIO_do_connect(sbio);
	if (rc<=0) {
		rc = SSL_get_verify_result(ssl);
		if (rc) {
			logMessage("Could not verify %s's identity", hostname);	
			*errorcode = FTPERR_REFUSED;
			goto error;
		}
		logMessage("Could not connect to %s:https", hostname);
		*errorcode = FTPERR_FAILED_CONNECT;
		goto error;
	}

	rc = BIO_do_handshake(sbio);
	if (rc<=0) {
		logMessage("Could not establish HTTPS connection with %s", 
			hostname);
		*errorcode = FTPERR_FAILED_CONNECT;
		goto error;
	}

	server_cert = SSL_get_peer_certificate(ssl);

	/* Show credentials if appropriate. */
	if (!loaderData->cert_filename && loaderData->fromCentral) {

		rc = show_cert(server_cert);
		if (rc != 1) {
			*errorcode = FTPERR_REFUSED;
			goto error;
		}
	}

	if (extraHeaders)
		hstr = extraHeaders;
	else
		hstr = "";

	bufsize = strlen(remotename) + strlen(hostname) + strlen(hstr) + 30;

	if ((buf = malloc(bufsize)) == NULL) {
			logMessage("ROCKS:httpsGetFileDesc:malloc failed");
			*errorcode = FTPERR_FAILED_CONNECT;
			goto error;
	}

	sprintf(buf, "GET %s HTTP/1.0\r\nHost: %s\r\n%s\r\n", remotename, 
		hostname, hstr);

	byteswritten = BIO_puts(sbio, buf);

	logMessage("ROCKS:httpsGetFileDesc:byteswritten(%d)", byteswritten);
	logMessage("ROCKS:httpsGetFileDesc:bufsize(%d)", (int)strlen(buf));

	free(buf);

	/* This is fun; read the response a character at a time until we:
	1) Get our first \r\n; which lets us check the return code
	2) Get a \r\n\r\n, which means we're done */

	*nextChar = '\0';
	checkedCode = 0;
	while (!strstr(headers, "\r\n\r\n")) {

		if (BIO_read(sbio, nextChar, 1) != 1) {
			*errorcode = FTPERR_SERVER_SECURITY;
			goto error;
		}

		nextChar++;
		*nextChar = '\0';

		if (nextChar - headers == sizeof(headers)) {
			goto error;
		}

		if (!checkedCode && strstr(headers, "\r\n")) {
			char * start, * end;

			checkedCode = 1;
			start = headers;
			while (!isspace(*start) && *start) start++;
			if (!*start) {
				goto error;
			}

			while (isspace(*start) && *start) start++;

			end = start;
			while (!isspace(*end) && *end) end++;
			if (!*end) {
				goto error;
			}

			logMessage("ROCKS:httpsGetFileDesc:status %s.", start);

			*end = '\0';
			if (!strcmp(start, "404"))
				goto error;
			else if (!strcmp(start, "403")) {
				*errorcode = FTPERR_SERVER_SECURITY;
				goto error;
			}
			else if (!strcmp(start, "503")) {
				/* A server nack - busy */
				logMessage("ROCKS:server busy");
				watchdog_reset();
				*errorcode = FTPERR_FAILED_DATA_CONNECT;
				goto error;
			}
			else if (strcmp(start, "200")) {
				*errorcode = FTPERR_BAD_SERVER_RESPONSE;
				goto error;
			}

			*end = ' ';
		}
	}

	return sbio;

error:
	close(sock);
	if (sbio)
		BIO_free_all(sbio);
	if (!*errorcode)
		*errorcode = FTPERR_SERVER_IO_ERROR;
	logMessage("ROCKS:httpsGetFileDesc:Error %s", 
		ftpStrerror(*errorcode));
	return NULL;
}
#endif
