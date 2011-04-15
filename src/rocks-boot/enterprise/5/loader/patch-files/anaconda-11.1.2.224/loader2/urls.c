/*
 * urls.c - url handling code
 *
 * Erik Troan <ewt@redhat.com>
 * Matt Wilson <msw@redhat.com>
 * Michael Fulbright <msf@redhat.com>
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

#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <newt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "../isys/dns.h"

#include "ftp.h"
#include "lang.h"
#include "loader.h"
#include "loadermisc.h"
#include "urls.h"
#include "log.h"
#include "windows.h"
#include "net.h"

#ifdef  ROCKS
#include <sys/ioctl.h>
/* interval of seconds to retry getting a kickstart file */
#define KS_RETRY_MAX 30
#define KS_RETRY_MIN 5
#endif

/* boot flags */
extern uint64_t flags;

/* convert a url (ftp or http) to a ui */
int convertURLToUI(char *url, struct iurlinfo *ui) {
    char *chptr;

    memset(ui, 0, sizeof(*ui));
    
    if (!strncmp("ftp://", url, 6)) {
	ui->protocol = URL_METHOD_FTP;
	url += 6;
	
	/* There could be a username/password on here */
	if ((chptr = strchr(url, '@'))) {
	    if ((chptr = strchr(url, ':'))) {
		*chptr = '\0';
		ui->login = strdup(url);
		url = chptr + 1;
		
		chptr = strchr(url, '@');
		*chptr = '\0';
		ui->password = strdup(url);
		url = chptr + 1;
	    } else {
		*chptr = '\0';
		ui->login = strdup(url);
		url = chptr + 1;
	    }
	}
    } else if (!strncmp("http://", url, 7)) {
	ui->protocol = URL_METHOD_HTTP;
	url +=7;
    } else {
	logMessage(ERROR, "unknown url protocol '%s'", url);
	return -1;
    }
    
    /* url is left pointing at the hostname */
    chptr = strchr(url, '/');
    if (chptr != NULL) {
        *chptr = '\0';
        ui->address = strdup(url);
        url = chptr;
        *url = '/';
        ui->prefix = strdup(url);
    }
    else {
       ui->address = strdup(url);
       ui->prefix = strdup("/");
    }
    
    logMessage(DEBUGLVL, "url address %s", ui->address);
    logMessage(DEBUGLVL, "url prefix %s", ui->prefix);

    return 0;
}

static char * getLoginName(char * login, struct iurlinfo *ui) {
    int i;

    i = 0;
    /* password w/o login isn't useful */
    if (ui->login && strlen(ui->login)) {
	i += strlen(ui->login) + 5;
	if (strlen(ui->password))
	    i += 3*strlen(ui->password) + 5;

	if (ui->login || ui->password) {
	    login = malloc(i);
	    strcpy(login, ui->login);
	    if (ui->password) {
		char * chptr;
		char code[4];

		strcat(login, ":");
		for (chptr = ui->password; *chptr; chptr++) {
		    sprintf(code, "%%%2x", *chptr);
		    strcat(login, code);
		}
		strcat(login, "@");
	    }
	}
    }

    return login;
}

/* convert a UI to a URL, returns allocated string */
char *convertUIToURL(struct iurlinfo *ui) {
    char * login;
    char * finalPrefix;
    char * url;

    if (!strcmp(ui->prefix, "/"))
	finalPrefix = "/.";
    else
	finalPrefix = ui->prefix;

    login = "";
    login = getLoginName(login, ui);
    
    url = malloc(strlen(finalPrefix) + 25 + strlen(ui->address) + strlen(login));
    sprintf(url, "%s://%s%s/%s", 
	    ui->protocol == URL_METHOD_FTP ? "ftp" : "http",
	    login, ui->address, finalPrefix);

    return url;
}

/* extraHeaders only applicable for http and used for pulling ks from http */
/* see ftp.c:httpGetFileDesc() for details */
/* set to NULL if not needed */
int urlinstStartTransfer(struct iurlinfo * ui, char * filename, 
                         char *extraHeaders) {
    char * buf;
    int fd, port;
    int family = -1;
    char * finalPrefix;
    struct in_addr addr;
    struct in6_addr addr6;
    char *hostname, *portstr;

    if (!strcmp(ui->prefix, "/"))
        finalPrefix = "";
    else
        finalPrefix = ui->prefix;

    buf = alloca(strlen(finalPrefix) + strlen(filename) + 20);
    if (*filename == '/')
        sprintf(buf, "%s%s", finalPrefix, filename);
    else
        sprintf(buf, "%s/%s", finalPrefix, filename);
    
    logMessage(INFO, "transferring %s://%s/%s to a fd",
               ui->protocol == URL_METHOD_FTP ? "ftp" : "http",
               ui->address, buf);

    splitHostname(ui->address, &hostname, &portstr);
    if (portstr == NULL)
        port = -1;
    else
        port = atoi(portstr);

    if (inet_pton(AF_INET, hostname, &addr) >= 1)
        family = AF_INET;
    else if (inet_pton(AF_INET6, hostname, &addr6) >= 1)
        family = AF_INET6;
    else {
        if (mygethostbyname(hostname, &addr, AF_INET) == 0) {
            family = AF_INET;
        } else if (mygethostbyname(hostname, &addr6, AF_INET6) == 0) {
            family = AF_INET6;
        } else {
            logMessage(ERROR, "cannot determine address family of %s",
                       hostname);
        }
    }

    if (ui->protocol == URL_METHOD_FTP) {
        ui->ftpPort = ftpOpen(hostname, family,
                              ui->login ? ui->login : "anonymous", 
                              ui->password ? ui->password : "rhinstall@", 
                              NULL, port);
        if (ui->ftpPort < 0)
            return -2;

        fd = ftpGetFileDesc(ui->ftpPort, addr6, family, buf);
        if (fd < 0) {
            close(ui->ftpPort);
            return -1;
        }
    } else {
#ifdef	ROCKS
	{
		/*
		 * try harder to start HTTP transfer
		 */
		int	tries = 1;
		int	rc;

		logMessage(INFO, "ROCKS:urlinstStartTransfer:http://%s/%s\n"
			"Headers:%s\n",
			ui->address, filename, extraHeaders);
		fd = -1;

		while ((fd < 0) && (tries < 10)) {
			fd = httpGetFileDesc(ui->address, -1, buf,
				extraHeaders);
			if (fd == FTPERR_FAILED_DATA_CONNECT) {
				/* Server busy, backoff */
				sleep(60);
				tries = 1;
				continue;
			}

			logMessage(INFO,
				"ROCKS:urlinstStartTransfer:attempt (%d)",
				tries);

			sleep(1);

			++tries;
		}

		if (fd < 0) {
			logMessage(ERROR, "ROCKS:urlinstStartTransfer:Failed");
			rc = newtWinChoice(_("GET File Error"), 
				_("Retry"), _("Cancel"), 
				_("Could not get file:\n\nhttp://%s/%s\n\n%s"), 
				ui->address, filename, 
				ftpStrerror(fd, URL_METHOD_HTTP));
			if (rc == 1)
				return urlinstStartTransfer(ui, filename,
					extraHeaders);
			else	
				return -1;
		}
	}
#else
        fd = httpGetFileDesc(hostname, port, buf, extraHeaders);
        if (fd < 0)
            return -1;
#endif /* ROCKS */
    }

    if (!FL_CMDLINE(flags))
        winStatus(70, 3, _("Retrieving"), "%s %s...", _("Retrieving"), 
                  filename);

    return fd;
}

int urlinstFinishTransfer(struct iurlinfo * ui, int fd) {
    if (ui->protocol == URL_METHOD_FTP)
        close(ui->ftpPort);
    close(fd);

    if (!FL_CMDLINE(flags))
        newtPopWindow();

    return 0;
}

char * addrToIp(char * hostname) {
    struct in_addr ad;
    struct in6_addr ad6;
    char *ret;

    if ((ret = malloc(48)) == NULL)
        return hostname;

    if (inet_ntop(AF_INET, &ad, ret, INET_ADDRSTRLEN) != NULL)
        return ret;
    else if (inet_ntop(AF_INET6, &ad6, ret, INET6_ADDRSTRLEN) != NULL)
        return ret;
    else if (mygethostbyname(hostname, &ad, AF_INET) == 0)
        return hostname;
    else if (mygethostbyname(hostname, &ad6, AF_INET6) == 0)
        return hostname;
    else
        return NULL;
}

int urlMainSetupPanel(struct iurlinfo * ui, urlprotocol protocol,
                      char * doSecondarySetup) {
    newtComponent form, okay, cancel, siteEntry, dirEntry;
    newtComponent answer, text;
    newtComponent cb = NULL;
    char * site, * dir;
    char * reflowedText = NULL;
    int width, height;
    newtGrid entryGrid, buttons, grid;
    char * chptr;
    char * buf = NULL;

    if (ui->address) {
        site = ui->address;
        dir = ui->prefix;
    } else {
        site = "";
        dir = "";
    }

    if (ui->login || ui->password || ui->proxy || ui->proxyPort)
        *doSecondarySetup = '*';
    else
        *doSecondarySetup = ' ';

    buttons = newtButtonBar(_("OK"), &okay, _("Back"), &cancel, NULL);
    
    switch (protocol) {
    case URL_METHOD_FTP:
        buf = sdupprintf(_(netServerPrompt), _("FTP"), getProductName());
        reflowedText = newtReflowText(buf, 47, 5, 5, &width, &height);
        free(buf);
        break;
    case URL_METHOD_HTTP:
        buf = sdupprintf(_(netServerPrompt), _("Web"), getProductName());
        reflowedText = newtReflowText(buf, 47, 5, 5, &width, &height);
        free(buf);
        break;

#ifdef ROCKS
    case URL_METHOD_HTTPS:
        buf = sdupprintf(_(netServerPrompt), "Secure Web", getProductName());
        reflowedText = newtReflowText(buf, 47, 5, 5, &width, &height);
        free(buf);
        break;
#endif /* ROCKS */
    }
    text = newtTextbox(-1, -1, width, height, NEWT_TEXTBOX_WRAP);
    newtTextboxSetText(text, reflowedText);
    free(reflowedText);

    siteEntry = newtEntry(22, 8, site, 24, (const char **) &site, 
                          NEWT_ENTRY_SCROLL);
    dirEntry = newtEntry(22, 9, dir, 24, (const char **) &dir, 
                         NEWT_ENTRY_SCROLL);

    entryGrid = newtCreateGrid(2, 2);
    newtGridSetField(entryGrid, 0, 0, NEWT_GRID_COMPONENT,
                     newtLabel(-1, -1, (protocol == URL_METHOD_FTP) ?
                                        _("FTP site name:") :
                                        _("Web site name:")),
                     0, 0, 1, 0, NEWT_ANCHOR_LEFT, 0);
    newtGridSetField(entryGrid, 0, 1, NEWT_GRID_COMPONENT,
                     newtLabel(-1, -1, 
                               sdupprintf(_("%s directory:"), 
                                          getProductName())),
                     0, 0, 1, 0, NEWT_ANCHOR_LEFT, 0);
    newtGridSetField(entryGrid, 1, 0, NEWT_GRID_COMPONENT, siteEntry,
                     0, 0, 0, 0, 0, 0);
    newtGridSetField(entryGrid, 1, 1, NEWT_GRID_COMPONENT, dirEntry,
                     0, 0, 0, 0, 0, 0);

    grid = newtCreateGrid(1, 4);
    newtGridSetField(grid, 0, 0, NEWT_GRID_COMPONENT, text,
                     0, 0, 0, 1, 0, 0);
    newtGridSetField(grid, 0, 1, NEWT_GRID_SUBGRID, entryGrid,
                     0, 0, 0, 1, 0, 0);

    if (protocol == URL_METHOD_FTP) {
        cb = newtCheckbox(3, 11, _("Use non-anonymous ftp"),
                          *doSecondarySetup, NULL, doSecondarySetup);
        newtGridSetField(grid, 0, 2, NEWT_GRID_COMPONENT, cb,
                         0, 0, 0, 1, NEWT_ANCHOR_LEFT, 0);
    }
        
    newtGridSetField(grid, 0, 3, NEWT_GRID_SUBGRID, buttons,
                     0, 0, 0, 0, 0, NEWT_GRID_FLAG_GROWX);

    newtGridWrappedWindow(grid, (protocol == URL_METHOD_FTP) ? _("FTP Setup") :
                          _("HTTP Setup"));

    form = newtForm(NULL, NULL, 0);
    newtGridAddComponentsToForm(grid, form, 1);    

    do {
        answer = newtRunForm(form);
        if (answer != cancel) {
            if (!strlen(site)) {
                newtWinMessage(_("Error"), _("OK"),
                               _("You must enter a server name."));
                continue;
            }
            if (!strlen(dir)) {
                newtWinMessage(_("Error"), _("OK"),
                               _("You must enter a directory."));
                continue;
            }

            if (!addrToIp(site)) {
                newtWinMessage(_("Unknown Host"), _("OK"),
                        _("%s is not a valid hostname."), site);
                continue;
            }
        }

        break;
    } while (1);
    
    if (answer == cancel) {
        newtFormDestroy(form);
        newtPopWindow();
        
        return LOADER_BACK;
    }

    if (ui->address) free(ui->address);
    ui->address = strdup(site);

    if (ui->prefix) free(ui->prefix);

    /* add a slash at the start of the dir if it is missing */
    if (*dir != '/') {
        if (asprintf(&(ui->prefix), "/%s", dir) == -1)
            ui->prefix = strdup(dir);
    } else {
        ui->prefix = strdup(dir);
    }

    /* Get rid of trailing /'s */
    chptr = ui->prefix + strlen(ui->prefix) - 1;
    while (chptr > ui->prefix && *chptr == '/') chptr--;
    chptr++;
    *chptr = '\0';

    if (*doSecondarySetup != '*') {
        if (ui->login)
            free(ui->login);
        if (ui->password)
            free(ui->password);
        if (ui->proxy)
            free(ui->proxy);
        if (ui->proxyPort)
            free(ui->proxyPort);
        ui->login = ui->password = ui->proxy = ui->proxyPort = NULL;
    }

    ui->protocol = protocol;

    newtFormDestroy(form);
    newtPopWindow();

    return 0;
}

int urlSecondarySetupPanel(struct iurlinfo * ui, urlprotocol protocol) {
    newtComponent form, okay, cancel, answer, text, accountEntry = NULL;
    newtComponent passwordEntry = NULL, proxyEntry = NULL;
    newtComponent proxyPortEntry = NULL;
    char * account, * password, * proxy, * proxyPort;
    newtGrid buttons, entryGrid, grid;
    char * reflowedText = NULL;
    int width, height;

    if (protocol == URL_METHOD_FTP) {
        reflowedText = newtReflowText(
        _("If you are using non anonymous ftp, enter the account name and "
          "password you wish to use below."),
        47, 5, 5, &width, &height);
    } else {
        reflowedText = newtReflowText(
        _("If you are using a HTTP proxy server "
          "enter the name of the HTTP proxy server to use."),
        47, 5, 5, &width, &height);
    }
    text = newtTextbox(-1, -1, width, height, NEWT_TEXTBOX_WRAP);
    newtTextboxSetText(text, reflowedText);
    free(reflowedText);

    if (protocol == URL_METHOD_FTP) {
        accountEntry = newtEntry(-1, -1, NULL, 24, (const char **) &account, 
                                 NEWT_FLAG_SCROLL);
        passwordEntry = newtEntry(-1, -1, NULL, 24, (const char **) &password, 
                                  NEWT_FLAG_SCROLL | NEWT_FLAG_PASSWORD);
    }
    proxyEntry = newtEntry(-1, -1, ui->proxy, 24, (const char **) &proxy, 
                           NEWT_ENTRY_SCROLL);
    proxyPortEntry = newtEntry(-1, -1, ui->proxyPort, 6, 
                               (const char **) &proxyPort, NEWT_FLAG_SCROLL);

    entryGrid = newtCreateGrid(2, 4);
    if (protocol == URL_METHOD_FTP) {
        newtGridSetField(entryGrid, 0, 0, NEWT_GRID_COMPONENT,
                     newtLabel(-1, -1, _("Account name:")),
                     0, 0, 2, 0, NEWT_ANCHOR_LEFT, 0);
        newtGridSetField(entryGrid, 0, 1, NEWT_GRID_COMPONENT,
                     newtLabel(-1, -1, _("Password:")),
                     0, 0, 2, 0, NEWT_ANCHOR_LEFT, 0);
    }
    if (protocol == URL_METHOD_FTP) {
        newtGridSetField(entryGrid, 1, 0, NEWT_GRID_COMPONENT, accountEntry,
                         0, 0, 0, 0, 0, 0);
        newtGridSetField(entryGrid, 1, 1, NEWT_GRID_COMPONENT, passwordEntry,
                         0, 0, 0, 0, 0, 0);
    }

    buttons = newtButtonBar(_("OK"), &okay, _("Back"), &cancel, NULL);

    grid = newtCreateGrid(1, 3);
    newtGridSetField(grid, 0, 0, NEWT_GRID_COMPONENT, text, 0, 0, 0, 0, 0, 0);
    newtGridSetField(grid, 0, 1, NEWT_GRID_SUBGRID, entryGrid, 
                     0, 1, 0, 0, 0, 0);
    newtGridSetField(grid, 0, 2, NEWT_GRID_SUBGRID, buttons, 
                     0, 1, 0, 0, 0, NEWT_GRID_FLAG_GROWX);

    if (protocol == URL_METHOD_FTP) {
        newtGridWrappedWindow(grid, _("Further FTP Setup"));
    } else {
        if (protocol == URL_METHOD_HTTP)
            newtGridWrappedWindow(grid, _("Further HTTP Setup"));
    }

    form = newtForm(NULL, NULL, 0);
    newtGridAddComponentsToForm(grid, form, 1);
    newtGridFree(grid, 1);

    answer = newtRunForm(form);
    if (answer == cancel) {
        newtFormDestroy(form);
        newtPopWindow();
        
        return LOADER_BACK;
    }
 
    if (protocol == URL_METHOD_FTP) {
        if (ui->login) free(ui->login);
        if (strlen(account))
            ui->login = strdup(account);
        else
            ui->login = NULL;
        
        if (ui->password) free(ui->password);
        if (strlen(password))
            ui->password = strdup(password);
        else
            ui->password = NULL;
    }
    
    newtFormDestroy(form);
    newtPopWindow();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4: */

#ifdef ROCKS

/* Forgive me for the global. We need the cert file information in
 * ftp.c:httpGetFileDesc() and have no good way of getting it there. 
 */ 
struct loaderData_s * rocks_global_loaderData;

#include "../isys/isys.h"
#include "../isys/imount.h"

#include <errno.h>

/*
 * From make-bootable-disks.c by Bruno.
 * Marks a disk as bootable. Returns 1 if disk was already
 * bootable, 0 if disk was previously unbootable. Returns -1 on error.
 */
static int
bootable (char *device, int major, int minor)
{
	mode_t	mode;
	int	fd;
	char	devicepath[128];
	unsigned char	buf[2];


	sprintf(devicepath, "/tmp/rocks-%s", device);

	mode = S_IFBLK | S_IRUSR | S_IWUSR;

	if (mknod(devicepath, mode, makedev(major, minor)) < 0) {
		logMessage(ERROR, "bootable:mknod failed: %s",
			strerror(errno));
		return -1;
	}

	/* Make sure this partition is bootable. */

	if ((fd = open(devicepath, O_RDWR)) < 0) {
		logMessage(ERROR, "bootable:open %s failed: %s", 
			devicepath, strerror(errno));
		unlink(devicepath);
		return -1;
	}

	lseek(fd, 510, SEEK_SET);

	if (read(fd, buf, sizeof(buf)) < 0) {
		logMessage(ERROR, "bootable:read failed: %s", strerror(errno));
		return -1;
	}
	if (buf[0] == 0x55 && buf[1] == 0xaa)
		return 1;

	lseek(fd, 510, SEEK_SET);

	/* The magic bits: 10101010 01010101 */
	buf[0] = 0x55;
	buf[1] = 0xaa;

	if (write(fd, buf, sizeof(buf)) < 0) {
		logMessage(ERROR, "bootable:write failed: %s", strerror(errno));
		return -1;
	}

	/*
	 * now tell the kernel to re-read the partition table
	 */
	if (ioctl(fd, BLKRRPART) != 0) {
		logMessage(ERROR, "bootable:ioctl failed: %s",
			strerror(errno));
		return -1;
	}

	close(fd);
	unlink(devicepath);

	return 0;
}


/* Returns 1 on success, 0 on failure. */
static int
ignoreCert()
{
	struct loaderData_s *loaderData;

	loaderData = rocks_global_loaderData;
	
	if (loaderData->dropCert)
		return 1;
	else
		return 0;
}

	
/*
 * From make-bootable-disks.c by Bruno.
 * Checks if a partition is a rocks root partition.  If so, the partition is
 * mounted to a well-known dir. Returns 0 on success, 1 on failure.
 */

static int
mountRocksDevice (char *device, int major, int minor, char *marker)
{
	mode_t	mode;
	char	devicepath[128];
	char	markerpath[128];
	char 	*fstype[10];
	char	*mountpoint = "/mnt/rocks-disk";
	int 	rc, i=0;

	if (ignoreCert())
		return 1;
		
	sprintf(devicepath, "/tmp/rocks-%s", device);

	mode = S_IFBLK | S_IRUSR | S_IWUSR;

	if (mknod(devicepath, mode, makedev(major, minor)) < 0) {
		logMessage(ERROR, "mountRocksDevice:mknod failed: %s",
			strerror(errno));
		return 1;
	}

	mkdirChain(mountpoint);

	/* Mount the partition. I dont like this exhaustive search. */

	fstype[i++] = "ext3";
	fstype[i++] = "ext2";
	fstype[i++] = "vfat";
	fstype[i++] = "xfs";
	fstype[i++] = 0;

	for (i=0; fstype[i]; i++) {
		rc = doPwMount(devicepath, mountpoint, fstype[i], 0, NULL);
		if (!rc)
			break;
	}
	if (!fstype[i])
		goto error;


	snprintf(markerpath, 128, "%s/%s", mountpoint, marker);
	if (!access(markerpath, F_OK)) {
		logMessage(INFO, "mountRocksDevice: found rocks device "
			"at %s, mounted on %s", 
			devicepath, mountpoint);
		return 0;
	}

error:
	unlink(devicepath);
	umount(mountpoint);
	return 1;
}


static int
mountRocksDisk (char *device, int major, int minor)
{
	return mountRocksDevice(device, major, minor, 
		"/.rocks-release");
}


static int 
mountRocksUSB (char *device, int major, int minor)
{
	return mountRocksDevice(device, major, minor, 
		"/rocks-usbkey");
}


/* Reads the kernel partition table in /proc/partitions. Returns
 * the contents of this file, or NULL on error. Caller must free the
 * result.
 */
static char *
getPartitions()
{
	int	fd;
	int	bytesread;
	char	*contents = 0;
	int 	offset, contents_size;
	int 	chunk = 2048;

	fd = open("/proc/partitions", O_RDONLY);
	if (fd<0) {
		logMessage(ERROR, "getPartitions:open failed "
			"for /proc/partitions");
		return 0;
	}

	offset = 0;
	contents = (char*) malloc(chunk);
	contents_size = chunk;
	memset(contents, 0, contents_size);
	while (1) {
		bytesread = read(fd, contents+offset, chunk);
		if (bytesread < 0) {
			logMessage(ERROR, "getPartitions:/proc read error");
			break;
		}
		if (!bytesread || (bytesread < chunk)) {
			break;
		}
		else {
			contents_size += chunk;
			contents = (char*) realloc((void*) contents,
				contents_size);
			if (!contents) {
				logMessage(ERROR,
					"getPartitions:realloc error");
				return 0;
			}
			offset += bytesread;

			/*
			 * make sure the newly added bytes of the buffer
			 * are initialized to zeroes
			 */
			memset((contents + offset), 0, chunk);
		}
	}
	close(fd);
	return contents;
}



/*
 * Finds and mounts an existing Rocks Partition, if one exists.
 * Returns 0 on success, 1 otherwise. Mounts to a well-known dir.
 */
static int
getRocksPartition()
{
	int	rc=1;
	int	major, minor, blocks;
	char	dev[32];
	char	*diskdevice;
	char	*line;
	char	*contents;

	contents = getPartitions();

	diskdevice = NULL;

	/*
	 * eat the first two lines
	 *
	 * there is a two line header on the output of
	 * /proc/partitions -- toss those lines, then do
	 * the work
	 */
	line = contents;
	line = strstr(line, "\n") + 1;

	while (1) {
		line = strstr(line, "\n");
		if (!line)
			break;
		line += 1;
		if (!strlen(line))
			continue;

		sscanf(line, "%d %d %d %32s", &major, &minor, &blocks, dev);

		if (!diskdevice || 
			strncmp(dev, diskdevice, strlen(diskdevice))) {
			/* A disk device name, like hda */
			diskdevice = dev;
			logMessage(INFO, "ROCKS:found disk device %s", dev);
			rc = bootable(diskdevice, major, minor);
			if (!rc) {
				/* Restart the parse */
				free(contents);
				return getRocksPartition();
			}
		}

		else if (!strncmp(dev, diskdevice, strlen(diskdevice))) {
			/* A disk partition name, like hda1 */
			logMessage(INFO, "ROCKS:found partition %s", dev);
			rc = mountRocksDisk(dev, major, minor);
			if (!rc)
				break;		/* Success */
		}
	}
	free(contents);
	return rc;
}


/*
 * Like getRocksPartition() but for USB disks.
 * Returns 0 on success, 1 otherwise. Mounts to a well-known dir.
 */
static int
getRocksUSB()
{
	int	rc=1;
	int	major, minor, blocks;
	char	dev[32];
	char	*diskdevice;
	char	*line;
	char	*contents;

	contents = getPartitions();

	diskdevice = NULL;

	/*
	 * eat the first two lines
	 *
	 * there is a two line header on the output of
	 * /proc/partitions -- toss those lines, then do
	 * the work
	 */
	line = contents;
	line = strstr(line, "\n") + 1;

	while (1) {
		line = strstr(line, "\n");
		if (!line)
			break;
		line += 1;
		if (!strlen(line))
			continue;

		sscanf(line, "%d %d %d %32s", &major, &minor, &blocks, dev);

		logMessage(INFO,
			"ROCKS:searching for USB keys on device %s", dev);
		rc = mountRocksUSB(dev, major, minor);
		if (!rc)
			break;		/* Success */
	}
	free(contents);
	return rc;
}


/* Retrives old crypto keys and certificates from disk. 
 * Returns 0 on success, 1 if no cert was found, -1 on error..
 */
int
getCert(struct loaderData_s *loaderData)
{
	char *priv, *cert, *ca;

	rocks_global_loaderData = loaderData;
	
	/* Search local disks for an existing certificate */
	if (haveCertificate())
		return 0;


	if (!getRocksUSB()) {
		priv = "/mnt/rocks-disk/security/cluster-cert.key";
		cert = "/mnt/rocks-disk/security/cluster-cert.crt";
		ca = "/mnt/rocks-disk/security/cluster-ca.crt";
	}
	else if (!getRocksPartition()) {
		priv = "/mnt/rocks-disk/etc/security/cluster-cert.key";
		cert = "/mnt/rocks-disk/etc/security/cluster-cert.crt";
		ca = "/mnt/rocks-disk/etc/security/cluster-ca.crt";
	}
	else {
		/* Could not find a usb drive nor a rocks disk */
		if (ignoreCert()) 
			logMessage(INFO,
				"ROCKS:getCert: ignoring old certificate");
		else
			logMessage(INFO, "ROCKS:getCert: No Rocks disks found");
		return 1;
	}
	
	if (access(priv, R_OK) || access(cert, R_OK) || access(ca, R_OK)) {
		logMessage(ERROR,
			"ROCKS:getCert: could not read %s, %s, and %s",
			priv, cert, ca);
		umount("/mnt/rocks-disk");
		return -1;
	}
	loaderData->cert_filename = cert;
	loaderData->priv_filename = priv;
	loaderData->ca_filename = ca;
	logMessage(INFO, "ROCKS:getCert: Found Keys on disk");

	return 0;
}


/* Returns true if we have found a security certificate.
 * Assumes that getCert() has already been run.
 */
int 
haveCertificate()
{
	struct loaderData_s *loaderData;

	loaderData = rocks_global_loaderData;

	if (loaderData->cert_filename)
		return 1;
	else
		return 0;
}

/* Forces the authentication check of our parents identity
 * to always pass.
 */
static void
forceParentAuth()
{
	struct loaderData_s *loaderData;

	loaderData = rocks_global_loaderData;

	loaderData->authParent = 0;
}



/* Adds the 'public' element to the kickstart url. Used to
 * try IP-based authentication to parent instead of cert-based.
 * Returns 0 if it changed the url, 1 if it did not, -1 on error.
 */
static int
addPublic(char **urlptr)
{
	char *p;
	char *url;
	char *line;

	url = *urlptr;

	/* Dont add public more than once */
	if (strstr(url, "public/"))
		return 1;

	/* We cannot use realloc since we are not sure url was malloced
	 * in the first place. This function eats memory. */
	line = (char*) malloc(strlen(url) + strlen("public/") + 1);
	if (!line)
		return -1;

	p = strrchr(url, '/');
	if (!p) {
		logMessage(ERROR, "addPublic:url not well formed.\n");
		return -1;
	}
	sprintf(line, "%.*s/public/%s", (int) (p-url), 
		url, p+1);

	*urlptr = line;
	return 0;
}

void
writeAvalancheInfo(char *trackers, char *pkgservers)
{
	int	fd;
	char	str[512];

	if ((fd = open("/tmp/rocks.conf",
					O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0) {
		logMessage(ERROR, "ROCKS:writeAvalancheInfo:failed to open '/tmp/rocks.conf'");
	}

	/*
	 * the next server (the ip address of the server that gave us a
	 * kickstart file), is passed to lighttpd through a configuration file.
	 * write that value into it.
	 */
	if (trackers != NULL) {
		sprintf(str, "var.trackers = \"%s\"\n", trackers);
	} else {
		sprintf(str, "var.trackers = \"127.0.0.1\"\n");
	}

	if (write(fd, str, strlen(str)) < 0) {
		logMessage(ERROR, "ROCKS:writeAvalancheInfo:write failed");
	}

	if (pkgservers != NULL) {
		sprintf(str, "var.pkgservers = \"%s\"\n", pkgservers);
	} else {
		sprintf(str, "var.pkgservers = \"127.0.0.1\"\n");
	}

	if (write(fd, str, strlen(str)) < 0) {
		logMessage(ERROR, "ROCKS:writeAvalancheInfo:write failed");
	}

	close(fd);
}

/* Use SSL. Must be entirely different since the return
 * type is a pointer to an SSL structure.
 */
BIO *
urlinstStartSSLTransfer(struct iurlinfo * ui, char * filename, 
                         char *extraHeaders, int silentErrors,
                         int flags, char *nextServer) {

	extern  void watchdog_reset();
	int	tries = 1;
	int	rc;
	int	errorcode = -1;
	int	sleepmin = KS_RETRY_MIN;
	char	*returnedHeaders;
	BIO	*sbio = 0;

	logMessage(INFO, "ROCKS:transferring https://%s/%s\nHeaders:%s\n",
	       ui->address, filename, extraHeaders);

	/* Add 'public' path element if we dont have a cert. */
	if (!haveCertificate())
		addPublic(&filename);

	while ((errorcode < 0) && (tries < 10)) {
		sbio = httpsGetFileDesc(ui->address, -1, filename,
			extraHeaders, &errorcode, &returnedHeaders);

		if (errorcode == 0) {
			char	*ptr;
			char	trackers[256];
			char	pkgservers[256];

			if ((ptr = strstr(returnedHeaders,
					"X-Avalanche-Trackers:")) != NULL) {
				sscanf(ptr, "X-Avalanche-Trackers: %256s",
					trackers);
			} else {
				if (nextServer != NULL) {
					snprintf(trackers, sizeof(trackers) - 1,
						"%s", nextServer);
				} else {
					strcpy(trackers, "127.0.0.1");
				}
			}

			if ((ptr = strstr(returnedHeaders,
					"X-Avalanche-Pkg-Servers:")) != NULL) {
				sscanf(ptr, "X-Avalanche-Pkg-Servers: %256s",
					pkgservers);
			} else {
				if (nextServer != NULL) {
					snprintf(pkgservers,
						sizeof(pkgservers) - 1, "%s",
						nextServer);
				} else {
					strcpy(pkgservers, "127.0.0.1");
				}
			}

			writeAvalancheInfo(trackers, pkgservers);

		} else if (errorcode == FTPERR_FAILED_DATA_CONNECT) {
			/*
			 * read the retry value from the return message
			 */
			char	*ptr;
			int	sleeptime = 0;

			if ((ptr = strstr(returnedHeaders, "Retry-After:"))
					!= NULL) {
				sscanf(ptr, "Retry-After: %d", &sleeptime);
			}

			if (sleeptime <= 0) {
				/*
				 * Backoff a random interval between
				 * KS_RETRY_MIN and KS_RETRY_MAX
				 */
				sleeptime = sleepmin +
					((KS_RETRY_MAX - sleepmin) *
					(rand()/(float)RAND_MAX));
			}

			winStatus(55, 3, _("Server Busy"), _("I will retry "
				"for a ks file after a %d sec sleep..."), 
				sleeptime, 0);

			/*
			 * this must be in a loop, as the alarm associated
			 * with the watchdog timer is sending a signal which
			 * interrupts the sleep().
			 */
			while ((sleeptime = sleep(sleeptime)) != 0) {
				;
			}

			newtPopWindow();

			tries = 1;
			/* Don't let the watchdog fire if the kickstart
			   server is reporting busy */
			watchdog_reset();
			continue;
		} else if (errorcode == FTPERR_REFUSED) {
			/*
			 * always accept the parent credentials
			 */
			forceParentAuth();
			continue;
		}

		logMessage(INFO, "ROCKS:urlinstStartSSLTransfer:attempt (%d)",
			tries);

		sleep(1);

		++tries;
	}

	if (errorcode < 0) {
		logMessage(ERROR, "ROCKS:urlinstStartSSLTransfer:Failed");
		rc = 0;

		/* Go through the public door automatically, but only
		 * if we have not tried it already. */
		if (errorcode == FTPERR_SERVER_SECURITY && !addPublic(&filename)) {
			rc = 1;
		}
		else {
			rc = newtWinChoice(_("GET File Error"), 
				_("Retry"), _("Cancel"), 
				_("Could not get file:\n\nhttps://%s/%s\n\n%s"),
				ui->address, filename, 
				ftpStrerror(errorcode, URL_METHOD_HTTP));
		}
		if (rc==1) 	/* Retry */
			return urlinstStartSSLTransfer(ui, filename,
				extraHeaders, silentErrors, flags, nextServer);
		else	/* Cancel */
			return NULL;
	}

	if (!FL_CMDLINE(flags))
		winStatus(70, 3, _("Retrieving"), "%s %.*s...", 
			_("Retrieving"), 60, filename);

	return sbio;
}


int
urlinstFinishSSLTransfer(BIO *sbio)
{
	int sock=0;

	if (!sbio)
		return 1;

	BIO_get_fd(sbio, &sock);
	if (sock)
		close(sock);

	BIO_free_all(sbio);
	return 0;
}

#endif /* ROCKS */
