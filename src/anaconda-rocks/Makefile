#
# $Id: Makefile,v 1.7 2012/11/27 00:49:04 phil Exp $
#
# @Copyright@
# 
# 				Rocks(r)
# 		         www.rocksclusters.org
# 		         version 6.2 (SideWinder)
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

REDHAT.ROOT     = $(CURDIR)/../../
-include $(ROCKSROOT)/etc/Rules.mk
include Rules.mk
ADDON = org_rocks_rolls
ANACONDA_USR = usr
ANACONDA_SHARE = $(ANACONDA_USR)/share/anaconda/addons
UPDATES	= updates
PRODUCT = product
PRODUCT.SRC  = $(PRODUCT)
UPDATES.IMG = updates.img
SUBDIRS = $(PRODUCT) $(UPDATES)
SYS.PYPATH = $(ANACONDA_USR)/lib/python2.7/site-packages
ROCKS.PYPATH = /opt/rocks/lib/python2.7/site-packages
UPDATES_DIRS = opt
PRODUCT_DIR = run 


build:

$(UPDATES.IMG): clean
	mkdir -p $(ANACONDA_SHARE) 
	(cd $(UPDATES); $(TAR) cf - $(ADDON)) | (cd $(ANACONDA_SHARE); $(TAR) xvf -)
	mkdir -p $(SYS.PYPATH)
	(cd $(ROCKS.PYPATH); $(TAR) cf - _xmlplus) | (cd $(SYS.PYPATH); $(TAR) xvf -)

	(cd $(UPDATES); $(TAR) cf - $(UPDATES_DIRS)) | $(TAR) xvf - 	
	-mkdir -p $(PRODUCT_DIR)
	(cd $(PRODUCT.SRC); $(TAR) cf - *) | $(TAR) xvf -
	find $(ANACONDA_USR) $(UPDATES_DIRS) $(PRODUCT_DIR) | cpio -c -o | gzip -9 > $(UPDATES.IMG) 

install::
	mkdir -p $(ROOT)/$(PKGROOT)
	for i in $(SUBDIRS); do 					 \
		$(TAR) cf - $$i | (cd $(ROOT)/$(PKGROOT); $(TAR) xvf -); \
	done
	
clean::
	rm -rf $(NAME)-$(VERSION)
	rm -f $(NAME).spec.in
	-rm -rf $(ANACONDA_USR) $(UPDATES_DIRS) $(PRODUCT_DIR)
	-rm $(UPDATES.IMG) 

