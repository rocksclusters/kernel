#
# $Id: Boot.mk,v 1.18 2008/03/06 23:41:56 mjk Exp $
#
# WARNING: You must be root to run this makefile.  We do a lot of
# mounts (over loopback) and mknods (for initrd /dev entries) so you
# really need root access.
#
# @Copyright@
# 
# 				Rocks(r)
# 		         www.rocksclusters.org
# 		            version 5.0 (V)
# 
# Copyright (c) 2000 - 2008 The Regents of the University of California.
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
# $Log: Boot.mk,v $
# Revision 1.18  2008/03/06 23:41:56  mjk
# copyright storm on
#
# Revision 1.17  2007/06/23 04:03:49  mjk
# mars hill copyright
#
# Revision 1.16  2006/11/29 23:12:40  bruno
# prototype support for lights out frontend installs
#
# Revision 1.15  2006/09/19 21:13:55  bruno
# don't nuke the X11R6 directory anymore -- we can use it
#
# Revision 1.14  2006/09/11 22:49:12  mjk
# monkey face copyright
#
# Revision 1.13  2006/08/10 00:11:16  mjk
# 4.2 copyright
#
# Revision 1.12  2006/07/28 20:13:19  bruno
# remove the rotating images from the package installation section of the
# installer.
#
# Revision 1.11  2006/06/05 17:57:40  bruno
# first steps towards 4.2 beta
#
# Revision 1.10  2006/01/27 22:29:44  bruno
# stable (mostly) after integration of new foundation and localization code
#
# Revision 1.9  2006/01/26 04:04:48  bruno
# push localization files into the netstg2
#
# Revision 1.8  2006/01/25 22:22:56  bruno
# compute nodes build again
#
# Revision 1.7  2005/12/21 00:22:02  bruno
# support for adding device drivers to initrd.img
#
# Revision 1.6  2005/10/12 18:10:04  mjk
# final copyright for 4.1
#
# Revision 1.5  2005/09/16 01:03:42  mjk
# updated copyright
#
# Revision 1.4  2005/06/30 19:15:37  bruno
# patch netstg2.img in kernel roll, not with rocks-dist
#
# Revision 1.3  2005/05/24 21:23:04  mjk
# update copyright, release is not any closer
#
# Revision 1.2  2005/03/03 00:08:18  bruno
# for RHEL 4
#
# Revision 1.1  2005/01/03 21:22:46  bruno
# first whack at RHEL 4 (nahant-beta)
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
# Revision 1.6  2004/08/30 21:44:59  bruno
# i386 now uses kernel package
#
# all arches now incorporate the kernel-unsupported modules (bug 31) -- this
# should increase the number of hardware devices that the installer supports
#
# Revision 1.5  2004/07/08 22:22:45  fds
# Helper target
#
# Revision 1.4  2004/03/31 02:22:58  fds
# Very important. Need a mountpoint for the cd.
#
# Revision 1.3  2004/03/25 03:15:21  bruno
# touch 'em all!
#
# update version numbers to 3.2.0 and update copyrights
#
# Revision 1.2  2004/03/19 03:21:38  bruno
# needed to up the size of initrd -- it dictates the size of '/' in the
# kickstart environment
#
# Revision 1.1  2004/01/15 00:58:06  fds
# Common Makefile called Boot.mk not Rules.mk.
#
# Revision 1.7  2003/11/13 05:03:00  bruno
# added changes in order to support ia64
#
# Revision 1.6  2003/11/12 20:03:41  fds
# Rocks splash screen automatically generated.
#
# Revision 1.5  2003/11/12 18:29:54  bruno
# tweaks
#
# Revision 1.4  2003/11/11 18:23:07  fds
# PXE target works again.
#
# Revision 1.3  2003/11/04 19:17:01  bruno
# added rocks-boot-netstage package
#
# Revision 1.2  2003/10/30 22:57:00  bruno
# updated for RHEL 3
#
# Revision 1.1  2003/10/27 18:10:43  bruno
# initial release
#
# Revision 1.3  2003/10/23 18:54:45  fds
# Better cleaning
#
# Revision 1.2  2003/10/21 18:32:00  fds
# Moved logic into Rules.mk for data normalization between the taroon architectures.
#
# Revision 1.2  2003/10/16 17:31:48  fds
# Better behaved
#
# Revision 1.7  2003/10/07 01:11:13  fds
# Building loader, put in better loader dependancies.
#
# Revision 1.6  2003/10/06 22:48:23  fds
# Added buildstamp to let new loader verify the initrd.
# Also added pxe convenience target.
#
# Revision 1.5  2003/10/03 17:24:04  fds
# Need these.
#
# Revision 1.4  2003/10/02 20:13:03  fds
# Clean is cleaner
#
# Revision 1.3  2003/09/28 21:59:47  fds
# New prep-initrd is smarter.
#
# Revision 1.2  2003/09/18 22:49:19  fds
# Cleaned again.
#
# Revision 1.1  2003/09/18 22:31:40  fds
# Opteron image. Cleaner Makefile. Common python helper scripts.
#
# Revision 1.5  2003/08/15 22:34:46  mjk
# 3.0.0 copyright
#
# Revision 1.4  2003/05/30 22:24:41  bruno
# added watchdog
#
# Revision 1.3  2003/05/22 16:39:27  mjk
# copyright
#
# Revision 1.2  2003/05/10 04:49:57  bruno
# got it to build
#
# Revision 1.1  2003/05/09 23:22:49  bruno
# initial release
#
#
ifeq ($(ARCH),)
ROCKSBIN=../../../../../../../../../bin
ARCH=$(shell $(ROCKSBIN)/arch)
endif

MOUNT   = mount -oloop
UMOUNT  = umount
MKISOFS = mkisofs

# IMAGES  = boot bootnet pcmcia bootall
IMAGES  = boot
LOADER.initrd-bootnet = -network
LOADER.initrd-boot    = -local
LOADER.initrd-pcmcia  = -pcmcia
LOADER.initrd-all     =
MODULES = $(addsuffix .cgz, $(addprefix modules-initrd-, $(IMAGES)))
DEFAULT  = isolinux


initrd.img:
	$(BOOTBASE)/prep-initrd.py

make-driver-disk:
	(cd ../drivers && \
		make KERNEL_SOURCE_ROOT=../../../$(ARCH)/kernel clean)
	(cd ../drivers && make \
		KERNEL_SOURCE_ROOT=../../../$(ARCH)/kernel diskimg)

install:
	install isolinux/{initrd.img,vmlinuz}	$(ROOT)/boot/kickstart/default/
	install isolinux/*			$(ROOT)/rocks/isolinux/
	install modules/module-info		$(ROOT)/rocks/modules/
	install modules/modules.dep		$(ROOT)/rocks/modules/
	install modules/pcitable		$(ROOT)/rocks/modules/
	install modules-initrd-boot.cgz	$(ROOT)/rocks/modules/modules.cgz
	install netstg2.img			$(ROOT)/rocks/RedHat/base
	install netstg2.img	$(ROOT)/rocks/RedHat/base/stage2.img
	install ../product.img			$(ROOT)/rocks/RedHat/base
	touch   $(ROOT)/rocks/modules/rocks-was-here


$(LOADER)/loader:
	make -C $(LOADER)

$(SPLASH)/splash.lss:
	$(MAKE) -C $(SPLASH)

# Suck the initial ramdisk and kernel images out of each boot floppy
# image.  The resulting ramdisk image is uncompressed, but the kernel
# remains untouched.

# initrd-%: %.img
#	gunzip -c $< > $@


initrd-%.iso: $(LOADER)/loader initrd.img make-driver-disk
	gunzip -c initrd.img > initrd-boot

	if [ ! -x $@.old ]; then mkdir $@.old; fi
	if [ ! -x $@.new ]; then mkdir $@.new; fi
	if [ ! -x $@.img ]; then mkdir $@.img; fi

	$(MOUNT) initrd-boot $@.old
	cd $@.old && find . | cpio -pduv ../$@.new

	cp $(LOADER)/loader $@.new/sbin/loader
	cp $(LOADER)/init $@.new/sbin/init

	# For xterm
	cp /etc/termcap $@.new/etc/termcap

	# For Mini HTTP
	mkdir -p $@.new/mnt/cdrom
	cp mini_httpd/usr/bin/mini_httpd $@.new/sbin/mini_httpd
	cp -r netstg/lib* $@.new/
	cp -r netstg/etc $@.new/

	( cd modules ; \
		find . -type f | cpio -H crc -o | \
		gzip -9 > ../$@.new/modules/modules.cgz ) 

	( cd modules ; \
	cp module-info ../$@.new/modules/ ; \
	cp modules.dep ../$@.new/modules/ ; \
	cp pcitable ../$@.new/modules/ )

	echo "TERM=vt100" >> $@.new/.profile
	echo "export TERM" >> $@.new/.profile

	cp $@.new/modules/modules.cgz modules-$(basename $@).cgz

	rm -f $@.new/etc/arch

	mkdir -p $@.new/tmp/
	if [ -f /tmp/site-frontend.xml ] ; then \
		cp /tmp/site-frontend.xml $@.new/tmp/site.xml ; \
	fi
	if [ -f /tmp/rolls-frontend.xml ] ; then \
		cp /tmp/rolls-frontend.xml $@.new/tmp/rolls.xml ; \
	fi

	#
	# if a driver disk exists, copy it over
	#
	if [ -f ../drivers/images/dd.img.gz ] ; then \
		gunzip -c ../drivers/images/dd.img.gz > $@.new/dd.img ; \
	fi

	#
	# now fix up the existing netstg2
	#
	rm -f netstg/modules/module*
	cp modules/module-info netstg/modules/
	cp modules/modules.dep netstg/modules/
	cp modules/pcitable netstg/modules/
	cp modules-$(basename $@).cgz netstg/modules/modules.cgz

	#
	# point the installer at the rocks images
	#
	( cd netstg/usr/share/anaconda/pixmaps ; \
		rm -rf rnotes ; \
		ln -s /tmp/updates/pixmaps/rnotes rnotes ; \
	)

	#
	# nuke the .mo file which throws off en_US installs
	#
	( cd netstg ; find . -type f -name 4Suite.mo | xargs rm -f )

	#
	# create mo files for the supported languages
	#
	( cd ../../loader/patch-files/anaconda-$(ANACONDA_VERSION)/po ; \
		for pofile in *po ; do \
			echo pofile: $${pofile} ; \
			langbase=`basename $${pofile} .po` ; \
			msgfmt --check -o $${langbase}.mo $${langbase}.po ; \
			mkdir -p ../../../../images/$(ARCH)/netstg/usr/share/locale/$${langbase}/LC_MESSAGES ; \
			gzip -9 -c $${langbase}.mo > ../../../../images/$(ARCH)/netstg/usr/share/locale/$${langbase}/LC_MESSAGES/anaconda.mo ; \
		done ; \
	)

	rm -f netstg2.img
	mkcramfs netstg netstg2.img
	
	$(UMOUNT) $@.old
	rm -rf $@.old

	dd if=/dev/zero of=$@ bs=512 count=`du -s --block-size=512 $@.new | \
		awk '{ print int($$1*4.0) }'`
	echo y | /sbin/mkfs -t ext2 $@

	$(MOUNT) $@ $@.img
	cd $@.new && find . | cpio -pdu ../$@.img
	$(UMOUNT) $@.img

	rm -rf $@.img
	rm -rf $@.new


# Compress the ramdisk image so it can go back into the boot disk.
initrd-%.iso.gz: initrd-%.iso
	gzip -9 -f $<

isolinux: /usr/lib/syslinux/isolinux.bin initrd-boot.iso.gz $(SPLASH)/splash.lss
	if [ ! -x $@ ]; then mkdir $@; fi
	cp /usr/lib/syslinux/isolinux.bin $@
	cp $(SPLASH)/{boot.msg,splash.lss} isolinux.cfg $@
	cp initrd-boot.iso.gz   $@/initrd.img
	cp kernel/boot/vmlinuz-*   $@/vmlinuz
	echo
	ls -l $@
	echo


bootdisk: isolinux
	(cd isolinux; mkisofs -V "Rocks Boot Disk" \
		-r -T -f \
		-o ../boot.iso \
		-b isolinux.bin \
		-c boot.cat \
		-no-emul-boot \
		-boot-load-size 4 -boot-info-table .)

pxe: isolinux
	cp isolinux/initrd.img /tftpboot/pxelinux/
	cp isolinux/vmlinuz /tftpboot/pxelinux/

# To refresh the boot.iso with a new loader without running
# the long buildinstall script (done by prep-initrd.py).
bootclean:
	rm -f initrd-boot.iso.gz

clean::
	rm -rf $(DEFAULT) bootall.img
	rm -rf $(MODULES) modules
	rm -f initrd-bootall.iso
	rm -f initrd-boot.iso.gz
	rm -f initrd-boot
	rm -f netstg2.img
	rm -rf initrd*.{old,new,img}
	rm -rf netstg
	rm -rf kernel*
	rm -rf mnt
	rm -rf mini_httpd
	rm -rf hwdata
	rm -rf anaconda-runtime
	rm -rf rpmdb
	rm -rf rocks-dist
	rm -f ../drivers/kversions
	rm -f boot.iso

