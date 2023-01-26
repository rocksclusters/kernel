#
# $Id: Makefile,v 1.3 2012/11/27 00:49:07 phil Exp $
#
# @Copyright@
# 
# 				Rocks(r)
# 		         www.rocksclusters.org
# 		         version 6.2 (SideWinder)
# 		         version 7.0 (Manzanita)
# 
# Copyright (c) 2000 - 2017 The Regents of the University of California.
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
#
#
#

REDHAT.ROOT     = $(CURDIR)/../../
-include $(ROCKSROOT)/etc/Rules.mk
include Rules.mk

LORAXBUILD = loraxtmp
LORAXCONF = lorax.conf
ISOCONFIG = usr/share/lorax/config_files/x86/isolinux.cfg 
APPENDLINE = central=$(CENTRAL)
build:
	## create a local rocks distribution
	/opt/rocks/bin/rocks create distro
	## Get the current lorax RPM and install the templates from it
	rpm2cpio `find rocks-dist -name 'lorax*$(ARCH).rpm'` | cpio --extract -c --make-directories '*/usr/share/lorax/*'
	## Apply Our patches to the template
	(cd patch-files; $(TAR) cf - *) | $(TAR) xvf - 
	## Patch the isolinux.cfg to set up our central
	$(SED) -i -e '/append/s/$$/ $(APPENDLINE)/' $(ISOCONFIG)
	echo "[lorax]" > $(LORAXCONF)
	echo "sharedir=`pwd`/usr/share/lorax" >> $(LORAXCONF)
	## Create a local repo of THIS roll's RPMS
	$(MAKE) -C $(REDHAT.ROOT) createlocalrepo
	## Use Lorax to Build images
	lorax --rootfs-size 3 $(ISFINAL) -p Rocks -v $(ROCKS_VERSION) -r $(RELEASE) -s `pwd`/rocks-dist/$(ARCH) -s $(REDHAT.ROOT)/localrepo -c `pwd`/$(LORAXCONF) --noupgrade `pwd`/$(LORAXBUILD) 


showvars:
	echo "CENTRAL=" $(CENTRAL)
install::
	mkdir -p $(ROOT)/$(PKGROOT)
	(cd $(LORAXBUILD); $(TAR) cf - --exclude=boot.iso * ) | \
	(cd $(ROOT)/$(PKGROOT); $(TAR) xvf - )	
	(cd $(LORAXBUILD)/images; mv boot.iso $(REDHAT.ROOT)$(INSTALLERISO))

clean::
	- /bin/rm -rf rocks-dist usr
	- /bin/rm *rpm 
	- /bin/rm  $(INSTALLERISO) 

