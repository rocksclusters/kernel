# --------------------------------------------------- -*- Makefile -*- --
# $Id: Makefile,v 1.24 2012/11/27 00:49:05 phil Exp $
#
# @Copyright@
# 
# 				Rocks(r)
# 		         www.rocksclusters.org
# 		       version 6.1.1 (Sand Boa)
# 
# Copyright (c) 2000 - 2014 The Regents of the University of California.
# All rights reserved.	
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
# notice unmodified and in its entirety, this list of conditions and the
# following disclaimer in the documentation and/or other materials provided 
# with the distribution.
# 
# 3. All advertising and press materials, printed or electronic, mentioning
# features or use of this software must display the following acknowledgement: 
# 
# 	"This product includes software developed by the Rocks(r)
# 	Cluster Group at the San Diego Supercomputer Center at the
# 	University of California, San Diego and its contributors."
# 
# 4. Except as permitted for the purposes of acknowledgment in paragraph 3,
# neither the name or logo of this software nor the names of its
# authors may be used to endorse or promote products derived from this
# software without specific prior written permission.  The name of the
# software includes the following terms, and any derivatives thereof:
# "Rocks", "Rocks Clusters", and "Avalanche Installer".  For licensing of 
# the associated name, interested parties should contact Technology 
# Transfer & Intellectual Property Services, University of California, 
# San Diego, 9500 Gilman Drive, Mail Code 0910, La Jolla, CA 92093-0910, 
# Ph: (858) 534-5815, FAX: (858) 534-7345, E-MAIL:invent@ucsd.edu
# 
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @Copyright@
#
# $Log: Makefile,v $
# Revision 1.24  2012/11/27 00:49:05  phil
# Copyright Storm for Emerald Boa
#
# Revision 1.23  2012/05/06 05:49:14  phil
# Copyright Storm for Mamba
#
# Revision 1.22  2012/02/01 20:47:34  phil
# Support prespec (copy of version-specific spec.in file) before spec file is made
#
# Revision 1.21  2012/01/24 05:36:32  phil
# now a version-specific spec file
#
# Revision 1.20  2012/01/23 20:49:32  phil
# Support for build under 5 or 6
#
# Revision 1.19  2011/07/23 02:31:14  phil
# Viper Copyright
#
# Revision 1.18  2010/09/07 23:53:23  bruno
# star power for gb
#
# Revision 1.17  2010/06/22 21:55:10  mjk
# remove ROCKSROOT
#
# Revision 1.16  2009/05/01 19:07:20  mjk
# chimi con queso
#
# Revision 1.15  2009/01/06 18:27:31  bruno
# more versioning of the vmlinuz and initrd.img
#
# Revision 1.14  2008/10/18 00:56:12  mjk
# copyright 5.1
#
# Revision 1.13  2008/03/06 23:41:56  mjk
# copyright storm on
#
# Revision 1.12  2007/06/23 04:03:49  mjk
# mars hill copyright
#
# Revision 1.11  2007/05/14 19:28:37  bruno
# cluster-kickstart-pxe moved to a node XML file
#
# Revision 1.10  2007/01/23 01:45:05  anoop
# Completed move of cluster-kickstart-solaris.c from kernel roll to base roll
#
# Revision 1.9  2006/12/06 00:25:46  anoop
# The XML file for MYSQL service on solaris. These two files are solaris specific
#
# Revision 1.8  2006/09/11 22:49:12  mjk
# monkey face copyright
#
# Revision 1.7  2006/08/10 00:11:15  mjk
# 4.2 copyright
#
# Revision 1.6  2006/07/14 01:05:12  bruno
# clean more
#
# Revision 1.5  2006/04/02 06:54:45  phil
# Compile cluster-kickstart statically
# create a little util program (prep-grub) that will rewrite
# rocks.conf files for 3.3 and later to use a new central, remove the restore
# flag and add "upgrade dropcert"
# Create a tar file of the vmlinuz/initrd.img. Also for dropping on
# older systems to do a remote upgrade (older versions of rpm may take a
# new rocks-boot rpm)
#
# Revision 1.4  2005/10/12 18:10:02  mjk
# final copyright for 4.1
#
# Revision 1.3  2005/09/16 01:03:39  mjk
# updated copyright
#
# Revision 1.2  2005/05/24 21:23:02  mjk
# update copyright, release is not any closer
#
# Revision 1.1  2004/11/23 02:41:13  bruno
# made the kernel roll bootable.
#
# moved 'rocks-boot' here -- it is now uses vmlinuz and builds the initrd.img
# file from the local (if present in the local RPMS directory) or from the
# current distro.
#
# if you want to use a specific kernel, just drop it in the RPMS directory.
#
# Revision 1.40  2004/11/02 00:39:51  fds
# Added bootdisk target to toplevel.
# For making a small bootimage for wan installs.
#
# Revision 1.39  2004/04/30 22:23:17  bruno
# add support to ensure PXE install of a node
#
# Revision 1.38  2004/03/25 03:15:15  bruno
# touch 'em all!
#
# update version numbers to 3.2.0 and update copyrights
#
# Revision 1.37  2003/11/12 20:04:52  fds
# Cleaning in images/splash
#
# Revision 1.36  2003/10/29 21:57:16  fds
# Rollback prev commit. Build worked but clean didn't.
#
# Revision 1.34  2003/10/07 20:45:45  bruno
# taroon tweaks
#
# Revision 1.33  2003/08/15 22:34:46  mjk
# 3.0.0 copyright
#
# Revision 1.32  2003/05/22 16:39:27  mjk
# copyright
#
# Revision 1.31  2003/03/21 16:32:51  mjk
# larger ramdisk
#
# Revision 1.30  2003/02/17 18:43:04  bruno
# updated copyright to 2003
#
# Revision 1.29  2002/11/27 04:56:03  bruno
# fix for not including *all* the module drivers
#
# Revision 1.28  2002/11/06 00:56:41  mjk
# ia64 boots
#
# Revision 1.27  2002/10/30 23:32:27  mjk
# 7.2-hybrid for ia64
#
# Revision 1.26  2002/10/18 21:33:25  mjk
# Rocks 2.3 Copyright
#
# Revision 1.25  2002/10/08 22:42:01  mjk
# blind cvs commit
#
# Revision 1.24  2002/10/04 05:33:26  mjk
# more rpm target fixes
#
# Revision 1.23  2002/10/03 23:06:08  mjk
# futzed with CVS repository structure
#
# Revision 1.22  2002/10/01 20:54:47  bruno
# removed rocks-splash.xpm.gz
#
# Revision 1.21  2002/08/24 05:01:50  bruno
# closer
#
# Revision 1.20  2002/07/03 23:32:35  bruno
# added splash screen!
#
# Revision 1.19  2002/06/05 06:45:59  bruno
# initial release
#
# Revision 1.18  2002/04/22 20:58:39  mjk
# everyone gets cluster-kickstart
#
# Revision 1.17  2002/03/08 16:24:26  bruno
# added more 'driver' disks
#
# Revision 1.16  2002/02/21 21:33:27  bruno
# added new copyright
#
# Revision 1.15  2002/02/06 21:22:44  bruno
# all the little things that releases find ...
#
# Revision 1.14  2002/01/18 23:39:32  bruno
# changes to support 7.2
#
# Revision 1.13  2001/11/27 22:53:17  bruno
# new gm and rocks-boot fixes
#
# Revision 1.12  2001/11/08 18:59:03  mjk
# Bumped version number
#
# Revision 1.11  2001/11/08 18:42:08  mjk
# NPACI Rocks 2.1.1 Release Copyright Notice
#
# Revision 1.10  2001/11/02 19:40:42  bruno
# tweaks for redhat 7.2
#
# Revision 1.9  2001/11/02 16:52:09  bruno
# need to set RPMHOME since this package is currently being built as 'root'
#
# Revision 1.8  2001/10/30 23:08:28  bruno
# new root check
#
# Revision 1.7  2001/10/25 16:19:11  bruno
# updates for local building
#
# Revision 1.6  2001/10/12 18:43:10  mjk
# - chkconfig added to spec file
# - watchdog set to 30 minutes
# - more ia64 reboot changes
#
# Revision 1.5  2001/10/05 01:01:12  bruno
# removed 'DEV' code which figured out what the root device is -- no longer
# needed ever since i pulled my head out of my ass
#
# Revision 1.4  2001/10/04 23:24:12  mjk
# Fixed clean targets
#
# Revision 1.3  2001/10/03 23:49:51  mjk
# - Works
#
# Revision 1.2  2001/10/02 00:57:28  mjk
# *** empty log message ***
#
# Revision 1.1  2001/10/01 23:30:35  mjk
# *** empty log message ***
#

NAME		= rocks-boot
RELEASE		= 1
USERID		= $(shell id -u)
MAKE.rpmroot	= 1
BUILDROOT = $(REDHAT.VAR)/tmp/$(NAME)-buildroot

MAKEPRESPEC 	= True

default: $(NAME).spec ../$(NAME)-$(VERSION).tar.gz

REDHAT.ROOT	= $(PWD)/../../

-include $(ROCKSROOT)/etc/Rules.mk
include Rules.mk

SEDSPEC += -e s%@REDHAT_RELEASE@%$(REDHAT_RELEASE)%g 

prespec:: 
	/bin/cp $(NAME).spec.in-$(VERSION.MAJOR) $(NAME).spec.in

build:: cluster-kickstart prep-initrd.py

install build clean::
	$(MAKE) ROOT=$(ROOT) PROJECT_NAME=$(PROJECT_NAME) REDHAT_RELEASE=$(REDHAT_RELEASE) VERSION=$(VERSION) -C $(REDHAT_RELEASE)/images/$(ARCH) $@
	$(MAKE) ROOT=$(ROOT) PROJECT_NAME=$(PROJECT_NAME) REDHAT_RELEASE=$(REDHAT_RELEASE) VERSION=$(VERSION) -C $(REDHAT_RELEASE)/images/splash $@
	$(MAKE) ROOT=$(ROOT) PROJECT_NAME=$(PROJECT_NAME) REDHAT_RELEASE=$(REDHAT_RELEASE) VERSION=$(VERSION) -C $(REDHAT_RELEASE)/loader $@

install:: 
	install cluster-kickstart $(ROOT)/boot/kickstart/
	install init.d/rocks-grub $(ROOT)/etc/rc.d/init.d/
	install -m 755 prep-grub.sh $(ROOT)/boot/kickstart/prep-grub
	tar cf $(ROOT)/boot/kickstart/rocks-install-kernel-$(VERSION)-$(ARCH).tar -C $(ROOT) boot/kickstart/default/vmlinuz-$(VERSION)-$(ARCH) boot/kickstart/default/initrd.img-$(VERSION)-$(ARCH)


cluster-kickstart: cluster-kickstart.o
	$(CC) $(CFLAGS) $(STATIC_LINK_FLAG) -o$@ $^

prep-initrd.py:
	install -m 755 prep-initrd-$(VERSION.MAJOR).py prep-initrd.py



BOOTDISK=rocks-bootdisk-$(VERSION).$(ARCH).iso

bootdisk:
	$(MAKE) -C $(REDHAT_RELEASE)/images/$(ARCH) $@
	cp $(REDHAT_RELEASE)/images/$(ARCH)/boot.iso \
		./$(BOOTDISK)
	@echo "Wrote: $(BOOTDISK)"

clean::
	-rm -f *.spec *.spec.in *.o cluster-kickstart $(BOOTDISK)
	rm -rf /tmp/hdstg2* /tmp/instimage* /tmp/keymaps* /tmp/netstg2* /tmp/stage2.img /tmp/treedir*
