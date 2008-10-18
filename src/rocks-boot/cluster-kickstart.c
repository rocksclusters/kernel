static char rcsid[] = "$Id: cluster-kickstart.c,v 1.12 2008/10/18 00:56:12 mjk Exp $";
/* -----------------------------------------------------------------------
 *
 * $RCSfile: cluster-kickstart.c,v $
 *
 * -----------------------------------------------------------------------
 *
 * Rekickstart a node.  Based on Bruno's script but made into C code
 * so we can 'safely' make it SUID root.  Then hacked up to support
 * ELILO.
 *
 * -----------------------------------------------------------------------
 *
 * @Copyright@
 * 
 * 				Rocks(r)
 * 		         www.rocksclusters.org
 * 		           version 5.1  (VI)
 * 
 * Copyright (c) 2000 - 2008 The Regents of the University of California.
 * All rights reserved.	
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice unmodified and in its entirety, this list of conditions and the
 * following disclaimer in the documentation and/or other materials provided 
 * with the distribution.
 * 
 * 3. All advertising and press materials, printed or electronic, mentioning
 * features or use of this software must display the following acknowledgement: 
 * 
 * 	"This product includes software developed by the Rocks(r)
 * 	Cluster Group at the San Diego Supercomputer Center at the
 * 	University of California, San Diego and its contributors."
 * 
 * 4. Except as permitted for the purposes of acknowledgment in paragraph 3,
 * neither the name or logo of this software nor the names of its
 * authors may be used to endorse or promote products derived from this
 * software without specific prior written permission.  The name of the
 * software includes the following terms, and any derivatives thereof:
 * "Rocks", "Rocks Clusters", and "Avalanche Installer".  For licensing of 
 * the associated name, interested parties should contact Technology 
 * Transfer & Intellectual Property Services, University of California, 
 * San Diego, 9500 Gilman Drive, Mail Code 0910, La Jolla, CA 92093-0910, 
 * Ph: (858) 534-5815, FAX: (858) 534-7345, E-MAIL:invent@ucsd.edu
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @Copyright@
 *
 * $Log: cluster-kickstart.c,v $
 * Revision 1.12  2008/10/18 00:56:12  mjk
 * copyright 5.1
 *
 * Revision 1.11  2008/03/26 23:31:32  bruno
 * have cluster-kickstart deal with real files rather than symlinks. this is
 * helpful when folks want to add kernel RPMS on the fly.
 *
 * Revision 1.10  2008/03/06 23:41:56  mjk
 * copyright storm on
 *
 * Revision 1.9  2007/06/23 04:03:49  mjk
 * mars hill copyright
 *
 * Revision 1.8  2006/11/06 15:38:54  bruno
 * one more reference to ramdisk_size for ia64
 *
 * Revision 1.7  2006/09/11 22:49:12  mjk
 * monkey face copyright
 *
 * Revision 1.6  2006/08/10 00:11:15  mjk
 * 4.2 copyright
 *
 * Revision 1.5  2005/10/12 18:10:02  mjk
 * final copyright for 4.1
 *
 * Revision 1.4  2005/09/16 01:03:39  mjk
 * updated copyright
 *
 * Revision 1.3  2005/05/24 21:23:02  mjk
 * update copyright, release is not any closer
 *
 * Revision 1.2  2005/04/14 22:25:42  bruno
 * added a '--halt' flag. this addresses bug 118
 *
 * Revision 1.1  2004/11/23 02:41:13  bruno
 * made the kernel roll bootable.
 *
 * moved 'rocks-boot' here -- it is now uses vmlinuz and builds the initrd.img
 * file from the local (if present in the local RPMS directory) or from the
 * current distro.
 *
 * if you want to use a specific kernel, just drop it in the RPMS directory.
 *
 * Revision 1.28  2004/10/20 16:29:22  bruno
 * set all references to 'ramdisk_size' to 150000
 *
 * Revision 1.27  2004/08/26 23:12:11  fds
 * Dont scrub disks unbootable if we are in shepherd mode.
 *
 * Revision 1.26  2004/08/20 00:00:03  fds
 * cluster shepherd support.
 *
 * Revision 1.25  2004/05/05 03:19:13  bruno
 * added full support for ia64 PXE installs
 *
 * Revision 1.24  2004/04/08 07:29:09  bruno
 * needs kssendmac
 *
 * Revision 1.23  2004/03/25 03:15:15  bruno
 * touch 'em all!
 *
 * update version numbers to 3.2.0 and update copyrights
 *
 * Revision 1.22  2003/11/14 20:04:13  bruno
 * shoot-node support ia64
 *
 * Revision 1.21  2003/10/17 20:44:50  mjk
 * missing fs0: for std boot (fix from Donaldx Meyer at Intel)
 *
 * Revision 1.20  2003/10/10 01:50:26  fds
 * Added opteron support
 *
 * Revision 1.19  2003/08/15 22:34:46  mjk
 * 3.0.0 copyright
 *
 * Revision 1.18  2003/07/10 15:35:58  bruno
 * bumped up ramdisk size to 100000
 *
 * Revision 1.17  2003/05/22 16:39:27  mjk
 * copyright
 *
 * Revision 1.16  2003/03/07 21:10:50  bruno
 * patched loader to *not* do version checking on boot.img, netstg1.img
 * and stage2.img
 *
 * Revision 1.15  2003/03/04 21:30:28  bruno
 * ia64 reboot needs to call i386 reboot routine
 *
 * Revision 1.14  2003/02/22 17:41:19  bruno
 * fixed ramdisk_size parameter
 *
 * Revision 1.13  2003/02/17 18:43:04  bruno
 * updated copyright to 2003
 *
 * Revision 1.12  2002/11/02 00:37:16  bruno
 * made all links to *.conf files relative to the current directory.
 *
 * Revision 1.11  2002/10/29 18:14:47  bruno
 * took out making the disk bootable again when hit shoot-node.
 * we have developed a better solution within ekv.
 *
 * Revision 1.10  2002/10/28 21:58:09  bruno
 * removed printfs
 *
 * Revision 1.9  2002/10/28 21:46:39  bruno
 * now read /proc/partitions with one read. this is an atomic function, as
 * we found that reading parts of /proc/partitions, then writing to the first
 * sector of the disk caused consistency problems when reading the remainder
 * of the /proc/partitions table.
 *
 * Revision 1.8  2002/10/18 21:33:25  mjk
 * Rocks 2.3 Copyright
 *
 * Revision 1.7  2002/08/20 21:17:55  bruno
 * added code to wipe the drive if we pxe installed
 *
 * Revision 1.6  2002/06/17 19:50:02  bruno
 * 7.3-isms
 *
 * Revision 1.5  2002/02/21 21:33:27  bruno
 * added new copyright
 *
 * Revision 1.4  2001/10/12 18:43:10  mjk
 * - chkconfig added to spec file
 * - watchdog set to 30 minutes
 * - more ia64 reboot changes
 *
 * Revision 1.3  2001/10/05 01:01:12  bruno
 * removed 'DEV' code which figured out what the root device is -- no longer
 * needed ever since i pulled my head out of my ass
 *
 * Revision 1.2  2001/10/04 23:19:09  mjk
 * IA64 first past
 *
 * Revision 1.1  2001/10/03 23:49:51  mjk
 * - Works
 *
 * Revision 1.24  2001/07/28 23:13:58  bruno
 * took out 'file:' option -- patched loader to natively get ks.cfg with http
 *
 * Revision 1.23  2001/06/30 00:54:23  bruno
 * added a version (VER) to the /proc/cmdline -- for rocks kickstart
 *
 * Revision 1.22  2001/06/26 23:58:36  mjk
 * - removed bool_l enum
 *
 * Revision 1.21  2001/06/26 21:48:37  mjk
 * - ia64 elilo changes
 * - still works on ia32
 *
 * Revision 1.20  2001/06/13 21:21:34  bruno
 * added loader-rocks (a watchdog for the installation)
 *
 * modified cluster-kickstart to add 'file:' to the command line (use this to
 * trick anaconda into getting kickstart files over the network)
 *
 * bumped the version number
 *
 * Revision 1.19  2001/06/05 01:47:37  mjk
 * - Fixed backwards symlink
 * - Tested on ia32 nodes
 *
 * Revision 1.18  2001/06/05 00:32:13  mjk
 * RPM now hold all bootdist for a given architecture
 *
 * Revision 1.17  2001/05/09 20:17:11  bruno
 * bumped copyright 2.1
 *
 * Revision 1.16  2001/04/10 14:16:27  bruno
 * updated copyright
 *
 * Revision 1.15  2001/03/08 23:13:15  mjk
 * Call setreuid
 *
 * Revision 1.14  2001/03/06 22:35:28  bruno
 * need to look at 'mtab' not 'fstab'
 *
 * Revision 1.13  2001/02/14 20:16:29  mjk
 * Release 2.0 Copyright
 *
 * Revision 1.12  2001/02/12 22:08:44  bruno
 * needed to remove '/var/lock/subsys/rocks-lilo" in order to get rocks-lilo
 * *not* to be called when cluster-kickstart was run.
 *
 * yes, there are some dumbasses within linux
 *
 * Revision 1.11  2001/02/03 15:51:40  bruno
 * added code to gracefully shutdown the node.
 *
 * this umounts all filesystems, so on the next boot after the reinstall, all
 * the non-root filesystem will be clean when they're remounted.
 *
 * Revision 1.10  2001/01/29 23:46:48  bruno
 * support for 'noreboot'
 *
 * Revision 1.9  2001/01/27 00:23:08  bruno
 * had to change PART,DEV to DEV
 *
 * Revision 1.8  2001/01/17 19:53:17  bruno
 * added 'label=rocks' back in
 *
 * Revision 1.7  2001/01/17 05:15:33  bruno
 * added PART and DEV to lilo.conf to support not nuking non-root partitions
 * during reinstall.
 *
 * Revision 1.6  2001/01/16 22:13:01  bruno
 * added ROOTDEV, MAJOR and MINOR to boot line
 *
 * Revision 1.5  2001/01/11 05:19:04  bruno
 * redhat 7.0 updates
 *
 * Revision 1.4  2000/10/10 15:28:33  bruno
 * changed the lilo label to 'rocks-install'
 *
 * Revision 1.3  2000/10/06 18:20:49  bruno
 * shoot the node rather than nicely reboot it.
 *
 * Revision 1.2  2000/08/30 20:49:04  mjk
 * Added postconfig script to fix file perms on cluster-kickstart
 *
 * Revision 1.1  2000/08/29 19:55:42  mjk
 * *** empty log message ***
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>

typedef enum mode {
	start,
	stop,
	reboot
} method_t;

typedef struct {
	char		*arch;
	void		(*start)(const char*);
	void		(*stop)(void);
	void		(*reboot)(void);
} loader_t;

static void	grub_start(const char*);
static void	grub_stop(void);
static void	grub_reboot(void);
static void	elilo_start(const char*);
static void	elilo_stop(void);
static void	elilo_reboot(void);
static void	nuke_it(char *, int);
static int	scrub(int);


static loader_t	loader_table[] = {
	{ "i386",	grub_start,	grub_stop,	grub_reboot },
	{ "i486",	grub_start,	grub_stop,	grub_reboot },
	{ "i586",	grub_start,	grub_stop,	grub_reboot },
	{ "i686",	grub_start,	grub_stop,	grub_reboot },
	{ "ia64",	elilo_start,	elilo_stop,	elilo_reboot },
	{ "x86_64",	grub_start,	grub_stop,	grub_reboot },
	{ NULL,		NULL,		NULL,		NULL },
};


static struct option	options[] = {
#define OPT_START 0
		{ "start",	0, 0, 0 },
#define OPT_STOP 1
		{ "stop",	0, 0, 0 },
#define OPT_SHEPHERD 2
		{ "shepherd",	0, 0, 0 },
#define OPT_HALT 3
		{ "halt",	0, 0, 0 },
		{ NULL,		0, 0, 0 }
	};

/*
 * disk scrubing flags
 */
#define	UNBOOTABLE	0
#define	BOOTABLE	1

/* Global variable. If false, never make disks unbootable */
static int doScrub = 1;

/* if true, then reboot. if false, just halt the node */
static int doReboot = 1;

int
main(int argc, char *argv[])
{
	loader_t	*p;
	int		option_index;
	method_t	method		= reboot;
	loader_t	*loader		= NULL;
	struct utsname	uts;
	char		*target		= "rocks.conf";
	int		opt;
	/* Global: int doScrub */

	openlog("cluster-kickstart", 0, LOG_LOCAL0);

				/* Query the machine arch to find the
                                   correct boot loader methods. */
	if ( uname(&uts) ) {
		perror("uname failed");
		exit(-1);
	}
	for (p=loader_table; p->arch && strcmp(p->arch, uts.machine); p++)
		;
	if ( p->arch ) {
		loader = p;
	} else {
				/* Complain on both channels for local
                                   and remote use. */
		syslog(LOG_ALERT, "unknown architecture");
		fprintf(stderr,   "unknown architecture");
		exit(-1);
	}

				/* Force the umask to owner so lilo
                                   won't compalin abut group write
                                   access on files. */
	umask(0022);

	
	do {
		opt = getopt_long(argc, argv, "", options, &option_index);
		switch (opt) {
		case 0:
			switch ( option_index ) {
			case OPT_START:
				method = start; 
				break;
			case OPT_SHEPHERD:
				target = "shepherd.conf";
				doScrub = 0;
				break;
			case OPT_STOP:
				method = stop;
				break;
			case OPT_HALT:
				doReboot = 0;
				break;
			default:
				;
			}
			break;
		default:
			;
		}
	} while (opt != -1);

				/* Really make us SUID */
	setreuid(0, 0);

	switch ( method ) {
	case start:
		loader->start(target);
		break;
	case stop:
		loader->stop();
		break;
	case reboot:
		loader->start(target);
		loader->reboot();
	default:
		;
	}

	closelog();
	return 0;
} /* main */


/*
 * grub Boot Loader Methods (ia32)
 */

static void
grub_start(const char *target)
{
	struct stat	buf;
	char		errstr[128];
	char		cmd[128];
	
	if (chdir("/boot/grub") != 0) {
		perror("cannot chdir to /boot/grub/");
		exit(-1);
	}

	if (stat("grub.conf", &buf) == 0) {
		if (unlink("grub.conf") < 0) {
			perror("cannot unlink /boot/grub/grub.conf");
			exit(-1);
		}
	}

	sprintf(cmd, "cp %s grub.conf", target);
	system(cmd);

	/*
	 * if the node PXE installed, then make the boot disk unbootable
	 */
	if (doScrub && stat("/boot/grub/pxe-install", &buf) == 0) {
		scrub(UNBOOTABLE);
	}

} /* grub_start */


static void
grub_stop(void)
{
	struct stat	buf;

	if (chdir("/boot/grub") != 0) {
		perror("cannot chdir to /boot/grub/");
		exit(-1);
	}

	if (stat("grub.conf", &buf) == 0) {
		if (unlink("grub.conf") < 0) {
			perror("cannot unlink /boot/grub/grub.conf");
			exit(-1);
		}
	}

	system("cp grub-orig.conf grub.conf");

	/*
	 * if the node PXE installed, then put the boot record back
	 */
	if (stat("/boot/grub/pxe-install", &buf) == 0) {
		scrub(BOOTABLE);
	}
} /* grub_stop */


static void
grub_reboot(void)
{
	syslog(LOG_ALERT, "kickstart node");

	/*
	 * On a standard reboot rocks-grub will revert back to a
	 * standard reboot.  To remain in kickstart mode we need to
	 * kill the service.  But killing the service invokes this
	 * code with the "--stop" flag.  So we cheat, and remove the
	 * lock file, and then remove the startup script.
	 *
	 * Additionally we shutdown nicely to keep the persistent
	 * partitions happy.
	 */
	 
	if (unlink("/var/lock/subsys/rocks-grub") < 0) {
		perror("cannot remove lock: unlink failed");
	}

	if ( system("/sbin/chkconfig --del rocks-grub") ) {
		perror("cannot reboot: /sbin/chkconfig failed");
	}

	/*
	 * Syslog causes problems, kill it early
	 */ 
	if ( system("/etc/rc.d/init.d/syslog stop") ) {
		perror("cannot reboot: "
		       "/etc/rc.d/init.d/syslog failed");
	}
	
	if (doReboot == 1) {
		if ( system("/sbin/reboot") ) {
			perror("cannot reboot: /sbin/reboot failed");
		}
	} else {
		if ( system("/sbin/halt") ) {
			perror("cannot halt: /sbin/halt failed");
		}
	}
} /* grub_reboot */



/*
 * ELILO Boot Loader Methods (ia64)
 */

static
void elilo_start(const char *target)
{
	FILE		*fout;
	struct stat	buf;

				/* Copy over the default kickstart
                                   floppy images */
	if ( system("cp /boot/kickstart/default/vmlinuz "
		    "/boot/efi/efi/redhat/vmlinuz-ks") )
	{
		perror("cannot copy kickstart kernel");
	}
	if ( system("cp /boot/kickstart/default/initrd.img "
		    "/boot/efi/efi/redhat/initrd-ks.img") )
	{
		perror("cannot copy kickstart ramdisk");
	}

				/* Create elilo-ks.conf */
	fout = fopen("/boot/efi/efi/redhat/elilo-ks.conf", "w");
	if ( !fout ) {
		perror("cannot open /boot/efi/efi/redhat/elilo-ks.conf for writing");
		exit(-1);
	}
	fputs("timeout=5\n", fout);
	fputs("image=vmlinuz-ks\n", fout);
	fputs("\tlabel=linux\n", fout);
	fputs("\tinitrd=initrd-ks.img\n", fout);
	fputs("\tread-only\n", fout);
	fputs("\tappend=\"ramdisk_size=300000 ks "
		"ksdevice=eth0 kssendmac\"\n", fout);
	fclose(fout);


				/* Write the startup script */
	fout = fopen("/boot/efi/startup.nsh", "w");
	if ( !fout ) {
		perror("cannot open /boot/efi/startup.nsh for writing");
		exit(-1);
	}

	if (stat("/boot/efi/pxe-install", &buf) == 0) {
		/*
		 * this node PXE installed
		 */
		fputs("lanboot\n", fout);
	} else {
		/*
		 * this node installed from the CD/DVD media
		 */
		fputs("fs0:\n", fout);
		fputs("cd efi\\redhat\n", fout);
		fputs("elilo -C elilo-ks.conf\n", fout);
	}

	fclose(fout);
} /* elilo_start */


static
void elilo_stop(void)
{
	FILE	*fout;

				/* Write the startup script */
	fout = fopen("/boot/efi/startup.nsh", "w");
	if ( !fout ) {
		perror("cannot open /boot/efi/startup.nsh for writing");
		exit(-1);
	}
	fputs("fs0:\n", fout);
	fputs("cd efi\\redhat\n", fout);
	fputs("elilo\n", fout);
	fclose(fout);
} /* elilo_stop */


static void
elilo_reboot(void)
{
	grub_reboot();
} /* elilo_reboot */


/*
 * disk scrubing (and unscrubing) procedures
 */

/*
 * by writing 0x00 0x00 to the end of the first block on a disk, will make
 * the disk unbootable.
 */
static void
nuke_it(char *device, int flag)
{
	int	fd;
	char	devicepath[128];
	char	buf[2];

	sprintf(devicepath, "/dev/%s", device);

	if ((fd = open(devicepath, O_WRONLY)) < 0) {
		perror("nuke_it:open failed\n");
		return;
	}

	lseek(fd, 510, SEEK_SET);

	if (flag == UNBOOTABLE) {
		buf[0] = 0x00;
		buf[1] = 0x00;
	} else {
		buf[0] = 0x55;
		buf[1] = 0xaa;
	}

	if (write(fd, buf, sizeof(buf)) < 0) {
		perror("nuke_it:write failed");
	}

	close(fd);
	return;
}

static int
scrub(int bootable)
{
	unsigned int	part_size;
	int		fd;
	int		major, minor, blocks;
	int		bytesread;
	char		done;
	char		*buf;
	char		*dev;
	char		*diskdevice;
	char		*line;
	char		*ptr;

	part_size = 2048;
	done = 0;
	while (!done) {
		if ((buf = (char *)malloc(part_size)) == NULL) {
			perror("scrub:malloc failed");
			return(-1);
		}

		if ((fd = open("/proc/partitions", O_RDONLY)) < 0) {
			perror("scrub:open failed for /proc/partitions");
			return(-1);
		}

		bytesread = read(fd, buf, part_size);
		if (bytesread < 0) {
			perror("scrub:read failed for /proc/partitions");
			return(-1);
		}

		if (bytesread < part_size) {
			done = 1;
		} else {
			free(buf);
			part_size = part_size * 2;
		}

		close(fd);
	}

	diskdevice = NULL;

	/*
	 * eat the first two lines
	 *
	 * there is a two line header on the output of
	 * /proc/partitions -- toss those lines, then do
	 * the work
	 */
	ptr = buf;
	line = strsep(&ptr, "\n");
	line = strsep(&ptr, "\n");
	
	while ((line = strsep(&ptr, "\n")) != NULL) {
		if (strcmp(line, "") != 0) {
			major = atoi(strtok(line, " "));
			minor = atoi(strtok(NULL, " "));
			blocks = atoi(strtok(NULL, " "));
			dev = strtok(NULL, " ");
		
			if (diskdevice == NULL) {
				diskdevice = strdup(dev);
				nuke_it(diskdevice, bootable);
			} else {
				if (strncmp(dev, diskdevice,
						strlen(diskdevice)) != 0) {

					free(diskdevice);
					diskdevice = strdup(dev);
					nuke_it(diskdevice, bootable);
				}
			}
		}
	}

	if (diskdevice != NULL) {
		free(diskdevice);
	}

	free(buf);
	return(0);
}
