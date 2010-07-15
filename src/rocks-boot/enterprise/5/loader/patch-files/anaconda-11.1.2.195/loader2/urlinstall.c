/*
 * urlinstall.c - code to set up url (ftp/http) installs
 *
 * Erik Troan <ewt@redhat.com>
 * Matt Wilson <msw@redhat.com>
 * Michael Fulbright <msf@redhat.com>
 * Jeremy Katz <katzj@redhat.com>
 *
 * Copyright 1997 - 2003 Red Hat, Inc.
 *
 * This software may be freely redistributed under the terms of the GNU
 * General Public License.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <newt.h>
#include <popt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>

#ifdef  ROCKS
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include "../isys/isys.h"
#include "../isys/imount.h"
#endif

#include "../isys/nl.h"

#include "kickstart.h"
#include "loader.h"
#include "loadermisc.h"
#include "lang.h"
#include "log.h"
#include "method.h"
#include "net.h"
#include "method.h"
#include "urlinstall.h"
#include "cdinstall.h"
#include "urls.h"
#include "windows.h"

/* boot flags */
extern uint64_t flags;

#ifdef	ROCKS
static int num_cpus();
#endif

static int loadSingleUrlImage(struct iurlinfo * ui, char * file,
                              char * dest, char * mntpoint, char * device,
                              int silentErrors) {
    int fd;
    int rc = 0;
    char * newFile = NULL;
    char filepath[1024];
    char *ehdrs = NULL;

    snprintf(filepath, sizeof(filepath), "%s", file);

    if (ui->protocol == URL_METHOD_HTTP) {
        ehdrs = (char *) malloc(24+strlen(VERSION));
        sprintf(ehdrs, "User-Agent: anaconda/%s\r\n", VERSION);
    }

    fd = urlinstStartTransfer(ui, filepath, ehdrs);

    if (fd == -2) {
        if (ehdrs) free (ehdrs);
        return 2;
    }

    if (fd < 0) {
        /* file not found */

        newFile = alloca(strlen(filepath) + 20);
        sprintf(newFile, "disc1/%s", filepath);

        fd = urlinstStartTransfer(ui, newFile, ehdrs);
        if (ehdrs) free (ehdrs);

        if (fd == -2) return 2;
        if (fd < 0) {
            if (!silentErrors)
                newtWinMessage(_("Error"), _("OK"),
                               _("Unable to retrieve %s://%s/%s/%s."),
                               (ui->protocol == URL_METHOD_FTP ? "ftp" :
                                "http"),
                               ui->address, ui->prefix, filepath);
            return 2;
        }
    }

    if (dest != NULL) {
        rc = copyFileAndLoopbackMount(fd, dest, device, mntpoint);
    }

    urlinstFinishTransfer(ui, fd);

    if (newFile) {
        newFile = malloc(strlen(ui->prefix) + 20);
        sprintf(newFile, "%s/disc1", ui->prefix);
        free(ui->prefix);
        ui->prefix = newFile;
    }

    return rc;
}


static int loadUrlImages(struct iurlinfo * ui) {
    char *stage2img;
    char tmpstr1[1024], tmpstr2[1024];
    int rc;

    /*    setupRamdisk();*/

    /* grab the updates.img before netstg1.img so that we minimize our
     * ramdisk usage */
    if (!loadSingleUrlImage(ui, "images/updates.img",
                            "/tmp/ramfs/updates-disk.img", "/tmp/update-disk",
                            "loop7", 1)) {
        copyDirectory("/tmp/update-disk", "/tmp/updates");
        umountLoopback("/tmp/update-disk", "loop7");
        unlink("/tmp/ramfs/updates-disk.img");
        unlink("/tmp/update-disk");
    }

    /* grab the product.img before netstg1.img so that we minimize our
     * ramdisk usage */
    if (!loadSingleUrlImage(ui, "images/product.img",
                            "/tmp/ramfs/product-disk.img", "/tmp/product-disk",
                            "loop7", 1)) {
        copyDirectory("/tmp/product-disk", "/tmp/product");
        umountLoopback("/tmp/product-disk", "loop7");
        unlink("/tmp/ramfs/product-disk.img");
        unlink("/tmp/product-disk");
    }

    /* require 128MB for use of graphical stage 2 due to size of image */
    if (totalMemory() < GUI_STAGE2_RAM) {
	stage2img = "minstg2.img";
	logMessage(WARNING, "URLINSTALL falling back to non-GUI stage2 "
                       "due to insufficient RAM");
    } else {
	stage2img = "stage2.img";
    }

    snprintf(tmpstr1, sizeof(tmpstr1), "images/%s", stage2img);
    snprintf(tmpstr2, sizeof(tmpstr2), "/tmp/ramfs/%s", stage2img);

#ifdef  ROCKS
    rc = loadSingleUrlImage(ui, tmpstr1, tmpstr2, "/mnt/runtime", "loop2", 0);
#else
    rc = loadSingleUrlImage(ui, tmpstr1, tmpstr2,
                            "/mnt/runtime", "loop0", 0);
#endif
    if (rc) {
        if (rc != 2) 
            newtWinMessage(_("Error"), _("OK"),
                           _("Unable to retrieve the install image."));
        return 1;
    }

#ifndef	ROCKS
    /*
     * ROCKS - we don't want to check the stamp since we routinely rebuild
     * the distribution.
     */

    /* now verify the stamp... */
    if (!verifyStamp("/mnt/runtime")) {
	char * buf;

	buf = sdupprintf(_("The %s installation tree in that directory does "
			   "not seem to match your boot media."), 
                         getProductName());

	newtWinMessage(_("Error"), _("OK"), buf);

	umountLoopback("/mnt/runtime", "loop0");
	return 1;
    }
#endif

    return 0;

}

static char * getLoginName(char * login, struct iurlinfo ui) {
    int i;

    i = 0;
    /* password w/o login isn't useful */
    if (ui.login && strlen(ui.login)) {
        i += strlen(ui.login) + 5;
        if (strlen(ui.password))
            i += 3*strlen(ui.password) + 5;
        
        if (ui.login || ui.password) {
            login = malloc(i);
            strcpy(login, ui.login);
            if (ui.password) {
                char * chptr;
                char code[4];
                
                strcat(login, ":");
                for (chptr = ui.password; *chptr; chptr++) {
                    sprintf(code, "%%%2x", *chptr);
                    strcat(login, code);
                }
                strcat(login, "@");
            }
        }
    }
    
    return login;
}

#ifdef ROCKS
void
start_httpd()
{
	/*
	 * the first two NULLs are place holders for the 'nextServer' info
	 */
	char	*args[] = { "/lighttpd/sbin/lighttpd", 
				"-f", "/lighttpd/conf/lighttpd.conf",
				"-D", NULL };
	int	pid;
	int	i;
	struct device	**devs;

	/*
	 * try to mount the CD
	 */
	devs = probeDevices(CLASS_CDROM, BUS_UNSPEC, 0);
	if (devs) {
		for (i = 0; devs[i]; i++) {
			if (!devs[i]->device) {
				continue;
			}

			devMakeInode(devs[i]->device, "/tmp/rocks-cdrom");

			logMessage(INFO,
				"start_httpd:trying to mount device %s",
				devs[i]->device);
			if (doPwMount("/tmp/rocks-cdrom", "/mnt/cdrom",
				"iso9660", IMOUNT_RDONLY, NULL)) {

				logMessage(ERROR,
					"start_httpd:doPwMount failed\n");
			} else {
				/*
				 * if there are multiple CD drives, exit this
				 * loop after the first successful mount
				 */ 
				break;
			}
		}
	}

	/*
	 * start the service
	 */
	pid = fork();
	if (pid != 0) {
#ifdef	LATER
		/*
		 * don't close stdin or stdout. this causes problems
		 * with mini_httpd as it uses these file descriptors for
		 * it's CGI processing
		 */
		close(2);
#endif
		execv(args[0], args);
		logMessage(ERROR, "start_httpd:lighttpd failed\n");
	}
}
#endif

char * mountUrlImage(struct installMethod * method,
                     char * location, struct loaderData_s * loaderData,
                     moduleInfoSet modInfo, moduleList modLoaded,
                     moduleDeps * modDeps) {
    int rc;
    char * url, *p;
    struct iurlinfo ui;
    char needsSecondary = ' ';
    int dir = 1;
    char * login;
    char * finalPrefix;
    char * cdurl;

    enum { URL_STAGE_MAIN, URL_STAGE_SECOND, URL_STAGE_FETCH, 
           URL_STAGE_DONE } stage = URL_STAGE_MAIN;

    enum urlprotocol_t proto = 
        !strcmp(method->name, "FTP") ? URL_METHOD_FTP : URL_METHOD_HTTP;

    /* JKFIXME: we used to do another ram check here... keep it? */

    memset(&ui, 0, sizeof(ui));

    while (stage != URL_STAGE_DONE) {
        switch(stage) {
        case URL_STAGE_MAIN:
            if ((loaderData->method == METHOD_FTP ||
                 loaderData->method == METHOD_HTTP) &&
                loaderData->methodData) {
		
                url = ((struct urlInstallData *)loaderData->methodData)->url;

                logMessage(INFO, "URL_STAGE_MAIN - url is %s", url);

                if (!url) {
                    logMessage(ERROR, "missing url specification");
                    loaderData->method = -1;
                    break;
                }
		
		/* explode url into ui struct */
		convertURLToUI(url, &ui);

		/* ks info was adequate, lets skip to fetching image */
		stage = URL_STAGE_FETCH;
		dir = 1;
		break;
	    } else if (urlMainSetupPanel(&ui, proto, &needsSecondary)) {
                return NULL;
            }

	    /* got required information from user, proceed */
	    stage = (needsSecondary != ' ') ? URL_STAGE_SECOND : 
		URL_STAGE_FETCH;
	    dir = 1;
            break;

        case URL_STAGE_SECOND:
            rc = urlSecondarySetupPanel(&ui, proto);
            if (rc) {
                stage = URL_STAGE_MAIN;
                dir = -1;
            } else {
                stage = URL_STAGE_FETCH;
                dir = 1;
            }
            break;

        case URL_STAGE_FETCH:
            if (FL_TESTING(flags)) {
                stage = URL_STAGE_DONE;
                dir = 1;
                break;
            }

#ifdef  ROCKS
	    start_httpd();
#endif

	    /* ok messy - see if we have a stage2 on local CD */
	    /* before trying to pull one over network         */
	    cdurl = findAnacondaCD(location, modInfo, modLoaded, 
				 *modDeps, 0);
	    if (cdurl) {
		logMessage(INFO, "Detected stage 2 image on CD");
		winStatus(50, 3, _("Media Detected"), 
			  _("Local installation media detected..."), 0);
		sleep(3);
		newtPopWindow();

                stage = URL_STAGE_DONE;
                dir = 1;
            } else {
		/* need to find stage 2 on remote site */
		if (loadUrlImages(&ui)) {
		    stage = URL_STAGE_MAIN;
		    dir = -1;
		    if (loaderData->method >= 0) {
			loaderData->method = -1;
		    }
		} else {
		    stage = URL_STAGE_DONE;
		    dir = 1;
		}
	    }
            break;

        case URL_STAGE_DONE:
            break;
        }
    }

    login = "";
    login = getLoginName(login, ui);

    if (!strcmp(ui.prefix, "/"))
        finalPrefix = "/.";
    else
        finalPrefix = ui.prefix;

    url = malloc(strlen(finalPrefix) + 25 + strlen(ui.address) +
                 strlen(login));

    /* sanitize url so we dont have problems like bug #101265 */
    /* basically avoid duplicate /'s                          */
    if (ui.protocol == URL_METHOD_HTTP) {
        for (p=finalPrefix; *p == '/'; p++);
        finalPrefix = p;
    }

    sprintf(url, "%s://%s%s/%s", 
	    ui.protocol == URL_METHOD_FTP ? "ftp" : "http",
	    login, ui.address, finalPrefix);

    return url;
}

#ifdef	ROCKS
char *
get_driver_name(char *dev)
{
	FILE	*file;
	int	retval = 1;
	char	field1[80];
	char	device[80];
	char	module[80];

	if ((file = fopen("/tmp/modprobe.conf", "r")) == NULL) {
		return(strdup("none"));
	}

	while (retval != EOF) {
		memset(field1, 0, sizeof(field1));
		memset(device, 0, sizeof(device));
		memset(module, 0, sizeof(module));

		retval = fscanf(file, "%s", field1);
		if ((retval == 1) && (strcmp(field1, "alias") == 0)) {

			retval = fscanf(file, "%s %s", device, module);
			if ((retval == 2) && (strcmp(device, dev) == 0)) {
				fclose(file);
				return(strdup(module));
			}
		}
	}

	fclose(file);
	return(strdup("none"));
}


int 
copyFileSSL(BIO *sbio, char * dest) 
{
    int outfd;
    char buf[4096];
    int i;
    int rc = 0;

    outfd = open(dest, O_CREAT | O_RDWR, 0666);

    if (outfd < 0) {
	logMessage(ERROR, "failed to open %s: %s", dest, strerror(errno));
	return 1;
    }

    while (1) {
	i = BIO_read(sbio, buf, sizeof(buf));
	if (!i)
		break;
	if (i<0) {
		logMessage(ERROR, "ROCKS:copyFileSSL:SSL read error");
		return 1;
	}

	if (write(outfd, buf, i) != i) {
	    rc = 1;
	    break;
	}
    }

    close(outfd);

    return rc;
}


/*
 * the file /tmp/interfaces will help us on frontend installs when
 * we are associating MAC addresses with IP address and with driver
 * names
 *
 * XXX: this code segment is copy of the code in the function below, so
 * if in future releases of the anaconda installer if the loop in the
 * code segment below changes, then this function will have to be updated
 * too.
 */

void
writeInterfacesFile(struct loaderData_s *loaderData)
{
	struct device	**devices;
	int		i, fd;
	char		*dev, *mac, tmpstr[128], *drivername;

	logMessage(INFO, "ROCKS:writeInterfacesFile");

	if ((fd = open("/tmp/interfaces",
					O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0) {
		logMessage(ERROR, "ROCKS:writeInterfacesFile:failed to open '/tmp/interfaces'");
		return;
	}

	devices = probeDevices(CLASS_NETWORK, BUS_UNSPEC, PROBE_LOADED);
	for (i = 0; devices && devices[i]; i++) {
		dev = devices[i]->device;
		mac = nl_mac2str(dev);

		drivername = get_driver_name(dev);

		if (mac) {
			/* A hint as to our primary interface. */
			if (!strcmp(dev, loaderData->netDev)) {
				snprintf(tmpstr, sizeof(tmpstr),
				"X-RHN-Provisioning-MAC-%d: %s %s %s ks\r\n",
				i, dev, mac, drivername);
			} else {
				snprintf(tmpstr, sizeof(tmpstr),
				"X-RHN-Provisioning-MAC-%d: %s %s %s\r\n",
				i, dev, mac, drivername);
			}

			if (write(fd, tmpstr, strlen(tmpstr)) < 0) {
				logMessage(ERROR,
				    "ROCKS:writeInterfacesFile::write failed");
			}

			free(mac);
		}

		free(drivername);
	}

	close(fd);
	return;
}
#endif

int getFileFromUrl(char * url, char * dest, 
                   struct loaderData_s * loaderData) {
    char ret[47];
    struct iurlinfo ui;
    enum urlprotocol_t proto = 
        !strncmp(url, "ftp://", 6) ? URL_METHOD_FTP : URL_METHOD_HTTP;
    char * host = NULL, * file = NULL, * chptr = NULL;
    int fd, rc;
    struct networkDeviceConfig netCfg;
    char * ehdrs = NULL;
    ip_addr_t *tip;
#ifdef  ROCKS
    char *drivername;
#endif

#ifdef  ROCKS
     /*
      * Call non-interactive, exhaustive NetworkUp() if we are
      * a cluster appliance.
      */
    if (!strlen(url)) {
	    logMessage(INFO, "ROCKS:getFileFromUrl:calling rocksNetworkUp");
            rc = rocksNetworkUp(loaderData, &netCfg);
    } else {
	    logMessage(INFO, "ROCKS:getFileFromUrl:calling kickstartNetworkUp");
            rc = kickstartNetworkUp(loaderData, &netCfg);
    }

    if (rc) return 1;
    fd = 0;

    /*
     * this will be used when starting up mini_httpd()
     *
     * Get the nextServer from PUMP if we DHCP, otherwise it
     * better be on the command line.
     */

    if ( netCfg.dev.set & PUMP_INTFINFO_HAS_BOOTFILE ) {
    	tip = &(netCfg.dev.nextServer);
    	inet_ntop(tip->sa_family, IP_ADDR(tip), ret, IP_STRLEN(tip));

    	if (strlen(ret) > 0) {
		loaderData->nextServer = strdup(ret);
	} else {
        	loaderData->nextServer = NULL;
	}
    }

    /*
     * If no nextServer use the gateway.
     */
    if ( !loaderData->nextServer ) {
    	loaderData->nextServer = strdup(loaderData->gateway);
    }

    logMessage(INFO, "%s: nextServer %s",
		"ROCKS:getFileFromUrl", loaderData->nextServer);
#else
    if (kickstartNetworkUp(loaderData, &netCfg)) {
        logMessage(ERROR, "unable to bring up network");
        return 1;
    }
#endif

    memset(&ui, 0, sizeof(ui));
    ui.protocol = proto;

#ifdef	ROCKS
{
	struct sockaddr_in	*sin;
	int			string_size;
	int			ncpus;
	char			np[16];
	char			*arch;
	char			*base;

#if defined(__i386__)
	arch = "i386";
#elif defined(__ia64__)
	arch = "ia64";
#elif defined(__x86_64__)
	arch = "x86_64";
#endif

	if (!strlen(url)) {
		base = strdup("install/sbin/kickstart.cgi");
		host = strdup(loaderData->nextServer);
	}
	else {
		char	*p, *q;

		base = NULL;
		host = NULL;

		p = strstr(url, "//");
		if (p != NULL) {
			p += 2;

			/*
			 * 'base' is the file name
			 */
			base = strchr(p, '/');
			if (base != NULL) {
				base += 1;
			}

			/*
		 	 * now get the host portion of the URL
			 */
			q = strchr(p, '/');
			if (q != NULL) {
				*q = '\0';
				host = strdup(p);
			}
		}
		
		if (!base || !host) {
			logMessage(ERROR,
				"kickstartFromUrl:url (%s) not well formed.\n",
				url);
			return(1);
		}
	}

	/* We always retrieve our kickstart file via HTTPS, 
	 * however the official install method (for *.img and rpms)
	 * is still HTTP.
	 */
	ui.protocol = URL_METHOD_HTTPS;

	winStatus(40, 3, _("Secure Kickstart"), 
	        _("Looking for Kickstart keys..."));

	getCert(loaderData);

	newtPopWindow();

	/* seed random number generator with our IP: unique for our purposes.
	 * Used for nack backoff.
	 */
	tip = &(netCfg.dev.nextServer);
	sin = (struct sockaddr_in *)IP_ADDR(tip);
	if (sin == NULL) {
		srand(time(NULL));
	} else {
		srand((unsigned int)sin->sin_addr.s_addr);
	}

	ncpus = num_cpus();
	sprintf(np, "%d", ncpus);

	string_size = strlen(base) + strlen("?arch=") + strlen(arch) +
		strlen("&np=") + strlen(np) + 1;

	if ((file = alloca(string_size)) == NULL) {
		logMessage(ERROR, "kickstartFromUrl:alloca failed\n");
		return(1);
	}
	memset(file, 0, string_size);

	sprintf(file, "/%s?arch=%s&np=%s", base, arch, np);
}

	logMessage(INFO, "ks location: https://%s%s", host, file);

#else
    tip = &(netCfg.dev.ip);
    inet_ntop(tip->sa_family, IP_ADDR(tip), ret, IP_STRLEN(tip));
    getHostandPath((proto == URL_METHOD_FTP ? url + 6 : url + 7), 
                   &host, &file, ret);

    logMessage(INFO, "file location: %s://%s/%s", 
               (proto == URL_METHOD_FTP ? "ftp" : "http"), host, file);
#endif

    chptr = strchr(host, '/');
    if (chptr == NULL) {
        ui.address = strdup(host);
        ui.prefix = strdup("/");
    } else {
        *chptr = '\0';
        ui.address = strdup(host);
        host = chptr;
        *host = '/';
        ui.prefix = strdup(host);
    }

    if (proto == URL_METHOD_HTTP) {
        ehdrs = (char *) malloc(24+strlen(VERSION));
        sprintf(ehdrs, "User-Agent: anaconda/%s\r\n", VERSION);
    }

    if (proto == URL_METHOD_HTTP && FL_KICKSTART_SEND_MAC(flags)) {
        /* find all ethernet devices and make a header entry for each one */
        int i;
        unsigned int hdrlen;
        char *dev, *mac, tmpstr[128];
        struct device ** devices;

        hdrlen = 0;
        devices = probeDevices(CLASS_NETWORK, BUS_UNSPEC, PROBE_LOADED);
        for (i = 0; devices && devices[i]; i++) {
            dev = devices[i]->device;
            mac = nl_mac2str(dev);
#ifdef  ROCKS
            drivername = get_driver_name(dev);
#endif

            if (mac) {
#ifdef  ROCKS

                /* A hint as to our primary interface. */
                if (!strcmp(dev, loaderData->netDev)) {
                        snprintf(tmpstr, sizeof(tmpstr),
                                "X-RHN-Provisioning-MAC-%d: %s %s %s ks\r\n",
                                i, dev, mac, drivername);
                } else {
                        snprintf(tmpstr, sizeof(tmpstr),
                                "X-RHN-Provisioning-MAC-%d: %s %s %s\r\n",
                                i, dev, mac, drivername);
                }
#else
                snprintf(tmpstr, sizeof(tmpstr),
                         "X-RHN-Provisioning-MAC-%d: %s %s\r\n", i, dev, mac);
#endif

#ifdef  ROCKS
                free(drivername);
#endif
                free(mac);

                if (!ehdrs) {
                    hdrlen = 128;
                    ehdrs = (char *) malloc(hdrlen);
                    *ehdrs = '\0';
                } else if ( strlen(tmpstr) + strlen(ehdrs) + 2 > hdrlen) {
                    hdrlen += 128;
                    ehdrs = (char *) realloc(ehdrs, hdrlen);
                }

                strcat(ehdrs, tmpstr);
            }
        }
    }
	
#ifdef ROCKS
{
        /* Retrieve the kickstart file via HTTPS */

        BIO *sbio;

        sbio = urlinstStartSSLTransfer(&ui, file, ehdrs, 1, flags,
		loaderData->nextServer);
        if (!sbio) {
                logMessage(ERROR, "failed to retrieve https://%s/%s",
                        ui.address, file);
                return 1;
        }

        rc = copyFileSSL(sbio, dest);
        if (rc) {
                unlink (dest);
                logMessage(ERROR, "failed to copy file to %s", dest);
                return 1;
        }

        urlinstFinishSSLTransfer(sbio);
        if (haveCertificate())
                umount("/mnt/rocks-disk");
}
#else
    fd = urlinstStartTransfer(&ui, file, ehdrs);
    if (fd < 0) {
        logMessage(ERROR, "failed to retrieve http://%s/%s/%s", ui.address, ui.prefix, file);
        if (ehdrs) free(ehdrs);
        return 1;
    }
           
    rc = copyFileFd(fd, dest);
    if (rc) {
        unlink (dest);
        logMessage(ERROR, "failed to copy file to %s", dest);
        if (ehdrs) free(ehdrs);
        return 1;
    }

    urlinstFinishTransfer(&ui, fd);
#endif

    if (ehdrs) free(ehdrs);

    return 0;
}

/* pull kickstart configuration file via http */
int kickstartFromUrl(char * url, struct loaderData_s * loaderData) {
    return getFileFromUrl(url, "/tmp/ks.cfg", loaderData);
}

void setKickstartUrl(struct loaderData_s * loaderData, int argc,
		    char ** argv) {

    char *url = NULL;
    poptContext optCon;
    int rc;
    struct poptOption ksUrlOptions[] = {
        { "url", '\0', POPT_ARG_STRING, &url, 0, NULL, NULL },
        { 0, 0, 0, 0, 0, 0, 0 }
    };

    logMessage(INFO, "kickstartFromUrl");
    optCon = poptGetContext(NULL, argc, (const char **) argv, ksUrlOptions, 0);
    if ((rc = poptGetNextOpt(optCon)) < -1) {
        startNewt();
        newtWinMessage(_("Kickstart Error"), _("OK"),
                       _("Bad argument to Url kickstart method "
                         "command %s: %s"),
                       poptBadOption(optCon, POPT_BADOPTION_NOALIAS), 
                       poptStrerror(rc));
        return;
    }

    if (!url) {
        newtWinMessage(_("Kickstart Error"), _("OK"),
                       _("Must supply a --url argument to Url kickstart method."));
        return;
    }

    /* determine install type */
    if (strstr(url, "http://"))
	loaderData->method = METHOD_HTTP;
    else if (strstr(url, "ftp://"))
	loaderData->method = METHOD_FTP;
    else {
        newtWinMessage(_("Kickstart Error"), _("OK"),
                       _("Unknown Url method %s"), url);
        return;
    }

    loaderData->methodData = calloc(sizeof(struct urlInstallData *), 1);
    ((struct urlInstallData *)loaderData->methodData)->url = url;

    logMessage(INFO, "results of url ks, url %s", url);
}

#ifdef	ROCKS
static int
num_cpus()
{
	FILE	*file;
	int	cpus = 0;
	char	str[128];

	if ((file = fopen("/proc/cpuinfo", "r")) != NULL) {

		while (fscanf(file, "%s", str) != EOF) {
			if (strcmp(str, "processor") == 0) {
				++cpus;
			}
		}

		fclose(file);
	}

	/*
	 * always return at least 1 CPU
	 */
	if (cpus == 0) {
		cpus = 1;
	}

	return(cpus);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4: */
