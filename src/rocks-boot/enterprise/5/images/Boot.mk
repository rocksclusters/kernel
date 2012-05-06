#
# $Id: Boot.mk,v 1.29 2012/05/06 05:49:14 phil Exp $
#
# WARNING: You must be root to run this makefile.  We do a lot of
# mounts (over loopback) and mknods (for initrd /dev entries) so you
# really need root access.
#
# @Copyright@
# 
# 				Rocks(r)
# 		         www.rocksclusters.org
# 		         version 5.5 (Mamba)
# 		         version 6.0 (Mamba)
# 
# Copyright (c) 2000 - 2012 The Regents of the University of California.
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
# Revision 1.29  2012/05/06 05:49:14  phil
# Copyright Storm for Mamba
#
# Revision 1.28  2011/12/14 20:17:11  phil
# busybox version of rpm was "in the way"
#
# Revision 1.27  2011/11/11 15:44:31  phil
# remove libz from the list that copies from /usr/lib[64]. It's only really in /lib and /lib64 on Centos5.7
#
# Revision 1.26  2011/11/11 05:59:01  phil
# Anaconda and updates from RHEL 5.7
#
# Revision 1.25  2011/07/23 02:31:14  phil
# Viper Copyright
#
# Revision 1.24  2011/04/15 19:41:59  phil
# Updates to build under CentOS 5.6 and new anaconda version.
# Calling this version5.4.3. Codename Viper.
#
# Had to rebuild our own kudzu lib because the CentOS 5.6 version on initial
# release was bad. See bug ID 4813 on bugs.centos.org. That was a not fun debug.
#
# Splash screen is work in progress.
#
# Revision 1.23  2010/11/03 21:44:31  bruno
# take out debug code
#
# Revision 1.22  2010/11/03 21:29:07  bruno
# strip the ext4 module out of modules.cgz in initrd.img. that way, we'll avoid
# any panic related to the ext4 driver.
#
# Revision 1.21  2010/09/07 23:53:23  bruno
# star power for gb
#
# Revision 1.20  2010/07/15 19:01:00  bruno
# support for different trackers and package servers
#
# Revision 1.19  2010/03/03 21:59:02  bruno
# add 'peer-done' to initrd
#
# Revision 1.18  2010/02/25 05:45:48  bruno
# makin' progress
#
# Revision 1.17  2009/10/26 19:28:40  bruno
# suppress errors
#
# Revision 1.16  2009/10/07 17:42:14  bruno
# add a library for firefox
#
# Revision 1.15  2009/10/03 00:19:40  bruno
# can build compute nodes with RHEL 5.4
#
# Revision 1.14  2009/08/28 21:49:53  bruno
# the start of the "most scalable installer in the universe!"
#
# Revision 1.13  2009/05/01 19:07:20  mjk
# chimi con queso
#
# Revision 1.12  2009/01/06 18:27:31  bruno
# more versioning of the vmlinuz and initrd.img
#
# Revision 1.11  2008/10/18 00:56:12  mjk
# copyright 5.1
#
# Revision 1.10  2008/05/22 21:02:07  bruno
# rocks-dist is dead!
#
# moved default location of distro from /export/home/install to
# /export/rocks/install
#
# Revision 1.9  2008/04/17 21:59:17  bruno
# tweaks to build driver disks on V
#
# Revision 1.8  2008/03/06 23:41:56  mjk
# copyright storm on
#
# Revision 1.7  2008/01/04 21:40:53  bruno
# closer to V
#
# Revision 1.6  2007/12/03 20:18:10  bruno
# updated kernel for V
#
# Revision 1.5  2007/08/16 00:31:18  bruno
# package up a xen installation kernel and initrd
#
# Revision 1.4  2007/06/23 04:03:49  mjk
# mars hill copyright
#
# Revision 1.3  2007/02/22 02:32:24  bruno
# replace mini_httpd with lighttpd and other fixes for V
#
# Revision 1.2  2007/01/12 18:55:55  bruno
# another snapshot
#
# Revision 1.1  2007/01/11 23:18:44  bruno
# first clean compile
#
# there is no way in hell this works
#
#
ifeq ($(ARCH),)
ARCH=$(shell $(ROCKSROOT)/bin/arch)
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

KERNELBASENAME	= `uname -r | sed -e 's/xen//' -e 's/PAE//'`


#initrd.img:
prep-initrd:
	$(BOOTBASE)/prep-initrd.py

make-driver-disk:
	(cd ../drivers && \
		make KERNEL_SOURCE_ROOT=../../../$(ARCH)/kernel clean)
	(cd kernels/lib/modules ; ls -d * > ../../../../drivers/kversions)
	(cd ../drivers && make \
		KERNEL_SOURCE_ROOT=../../../$(ARCH)/kernel diskimg)

install:
	install isolinux/vmlinuz \
		$(ROOT)/boot/kickstart/default/vmlinuz-$(VERSION)-$(ARCH)
	install isolinux/initrd.img \
		$(ROOT)/boot/kickstart/default/initrd.img-$(VERSION)-$(ARCH)
	install isolinux/* $(ROOT)/rocks/isolinux/

	mkdir -p $(ROOT)/rocks/images/
	install rocks-dist/$(ARCH)/images/stage2.img $(ROOT)/rocks/images/

	mkdir -p $(ROOT)/boot/kickstart/xen/
	install rocks-dist/$(ARCH)/images/xen/vmlinuz \
		$(ROOT)/boot/kickstart/xen/vmlinuz-$(VERSION)-$(ARCH)
	install initrd-xen.iso.gz \
		$(ROOT)/boot/kickstart/xen/initrd-xen.iso.gz-$(VERSION)-$(ARCH)


$(LOADER)/loader:
	make -C $(LOADER)

$(SPLASH)/splash.lss:
	$(MAKE) -C $(SPLASH)


make-stage2:
	rm -f stage2/modules/module*
	cp modules-kernel/module-info stage2/modules/
	cp modules-kernel/modules.dep stage2/modules/
	cp modules-kernel/pci.ids stage2/modules/
	cp modules-$(basename $@).cgz stage2/modules/modules.cgz

	#
	# point the installer at the rocks images
	#
	( cd stage2/usr/share/anaconda/pixmaps ; \
		rm -rf rnotes ; \
		ln -s /tmp/updates/pixmaps/rnotes rnotes ; \
	)

	#
	# nuke the .mo file which throws off en_US installs
	#
	( cd stage2 ; find . -type f -name 4Suite.mo | xargs rm -f )

	#
	# create mo files for the supported languages
	#
	if [ ! -d po ]; then mkdir po ; fi
	( cd po ; \
	for pofile in ../../../rocks-po/*po ; do \
		pobase=`basename $${pofile}` ; \
		echo pobase: $${pobase} ; \
		cat ../../../loader/anaconda-$(ANACONDA_VERSION)/po/$${pobase} \
			$${pofile} > $${pobase} ; \
		langbase=`basename $${pobase} .po` ; \
		msgfmt --check -o $${langbase}.mo $${langbase}.po ; \
		mkdir -p ../../../images/$(ARCH)/stage2/usr/share/locale/$${langbase}/LC_MESSAGES ; \
		gzip -9 -c $${langbase}.mo > ../../../images/$(ARCH)/stage2/usr/share/locale/$${langbase}/LC_MESSAGES/anaconda.mo ; \
	done ; \
	)

	rm -f stage2.img
	mksquashfs stage2 stage2.img


#initrd-%: %.img
	#gunzip -c $< > $@


initrd-%.iso: $(LOADER)/loader prep-initrd make-driver-disk
	gunzip -c $(basename $@).img > $(basename $@)

	if [ ! -x $@.new ]; then mkdir $@.new; fi

	cd $@.new && cpio -iv < ../$(basename $@)

	#
	# nuke ext4 module from modules.cgz
	#
	# Anaconda 224, don't nuke ext4 support
	#mkdir nukeext4
	#( cd nukeext4 ; \
	#	gunzip -c ../$@.new/modules/modules.cgz | cpio -idu ; \
	#	rm -f `find . | grep ext4.ko` ; \
	#	find . -type f | cpio -H crc -o | \
	#		gzip -9 > ../$@.new/modules/modules.cgz ; \
	#)
	#rm -rf nukeext4

	cp $(LOADER)/loader $@.new/sbin/loader
	cp $(LOADER)/init $@.new/sbin/init

	# For xterm
	cp /etc/termcap $@.new/etc/termcap

	# For lighttpd
	mkdir -p $@.new/mnt/cdrom
	cp -R lighttpd/lighttpd $@.new/
	mkdir -p $@.new/lib
	mkdir -p $@.new/lib64
	-cp -d /lib/libpcre* $@.new/lib
	-cp -d /lib64/libpcre* $@.new/lib64

	# the rocks tracker client
	mkdir -p $@.new/tracker
	cp rocks-tracker/opt/rocks/bin/tracker-client $@.new/tracker/
	cp rocks-tracker/opt/rocks/bin/peer-done $@.new/tracker/
	-for i in curl idn gssapi_krb5 krb5 k5crypto krb5support ; do \
		cp -d /usr/lib/lib$$i.so* $@.new/lib ; \
		cp -d /usr/lib64/lib$$i.so* $@.new/lib64 ; \
	done
	# special handling for libz (required for CentOS 5.7)
	-for i in z; do \
		cp -d /lib/lib$$i.so* $@.new/lib ; \
		cp -d /lib64/lib$$i.so* $@.new/lib64 ; \
	done

	# remove busybox version of rpm. we need the real one (CentOS 5.7)
	if [ -h $@.new/sbin/rpm ]; then \
		/bin/rm $@.new/sbin/rpm; \
	fi

	# for firefox
	-cp -d /lib/libasound* $@.new/lib
	-cp -d /lib64/libasound* $@.new/lib64

	# For createrepo
	#mkdir -p $@.new/usr/share
	#( cd $@.new/usr/share ; \
		#ln -s /tmp/updates/usr/share/createrepo createrepo )

	#
	# get some files off the stage2 image
	#
	if [ ! -d stage2 ] ; then mkdir stage2 ; fi
	mount -o loop -t squashfs \
		rocks-dist/$(ARCH)/images/stage2.img stage2
	cp -r -d stage2/lib* $@.new/
	cp -r stage2/etc $@.new/
	umount stage2

	#( if [ $(basename $@) == 'initrd-xen' ] ; then \
			#cd modules-kernel-xen ; else cd modules-kernel ; fi ; \
		#find . -type f | cpio -H crc -o | \
		#gzip -9 > ../$@.new/modules/modules.cgz )

	# LATER
	# LATER
	# LATER
	#( cd modules ; \
		#cp module-info ../$@.new/modules/ ; \
		#cp modules.dep ../$@.new/modules/ ; \
		#cp pci.ids ../$@.new/modules/ ; \
		#cp modules.alias ../$@.new/modules/ )

	echo "TERM=vt100" >> $@.new/.profile
	echo "export TERM" >> $@.new/.profile

	#cp $@.new/modules/modules.cgz modules-$(basename $@).cgz

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
	# stage2 building used to be here
	#
	
	( cd $@.new && find . | cpio --quiet -c -o | gzip -9 > ../$@.gz )

	#rm -rf $@.new


# Compress the ramdisk image so it can go back into the boot disk.
initrd-%.iso.gz: initrd-%.iso
	#gzip -9 -f $<

isolinux: /usr/lib/syslinux/isolinux.bin initrd-boot.iso.gz initrd-xen.iso.gz $(SPLASH)/splash.lss
	if [ ! -x $@ ]; then mkdir $@; fi
	cp /usr/lib/syslinux/isolinux.bin $@
	cp $(SPLASH)/{boot.msg,splash.lss} isolinux.cfg $@
	cp initrd-boot.iso.gz   $@/initrd.img
	cp rocks-dist/$(ARCH)/isolinux/vmlinuz $@/vmlinuz
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
	cp isolinux/vmlinuz /tftpboot/pxelinux/vmlinuz-$(VERSION)-$(ARCH)
	cp isolinux/initrd.img /tftpboot/pxelinux/initrd.img-$(VERSION)-$(ARCH)

# To refresh the boot.iso with a new loader without running
# the long buildinstall script (done by prep-initrd.py).
bootclean:
	rm -f initrd-boot.iso.gz

clean::
	rm -rf $(DEFAULT) bootall.img
	rm -rf $(MODULES) modules-kernel modules-kernel-xen
	rm -f initrd-bootall.iso
	rm -f initrd-boot.iso.gz
	rm -f initrd-boot
	rm -f initrd-xen initrd-xen.img.gz initrd-xen.iso.gz
	rm -rf initrd*.{old,new,img}
	rm -rf cachedir
	rm -rf kernel*
	rm -rf mnt
	rm -rf lighttpd
	rm -rf rocks-tracker
	rm -rf hwdata
	rm -rf anaconda-runtime
	rm -rf rpmdb
	rm -rf rocks-dist
	rm -f ../drivers/kversions
	rm -f boot.iso
	rm -rf stage2
	rm -f stage2.img

