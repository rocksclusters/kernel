# $Id: Makefile,v 1.11 2008/03/06 23:41:56 mjk Exp $
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
# $Log: Makefile,v $
# Revision 1.11  2008/03/06 23:41:56  mjk
# copyright storm on
#
# Revision 1.10  2007/06/23 04:03:49  mjk
# mars hill copyright
#
# Revision 1.9  2006/09/11 22:49:14  mjk
# monkey face copyright
#
# Revision 1.8  2006/08/10 00:11:17  mjk
# 4.2 copyright
#
# Revision 1.7  2006/06/16 21:46:57  bruno
# cleanup mo files
#
# Revision 1.6  2006/01/26 04:04:48  bruno
# push localization files into the netstg2
#
# Revision 1.5  2005/10/12 18:10:06  mjk
# final copyright for 4.1
#
# Revision 1.4  2005/09/16 01:03:43  mjk
# updated copyright
#
# Revision 1.3  2005/05/24 21:23:05  mjk
# update copyright, release is not any closer
#
# Revision 1.2  2005/02/16 17:14:56  bruno
# initial checkin for official RHEL 4 anaconda
#
# Revision 1.1  2005/01/03 21:22:48  bruno
# first whack at RHEL 4 (nahant-beta)
#
#
include ../version.mk

PATCH_FILES = $(shell find patch-files/anaconda-$(ANACONDA_VERSION)/ -type f)

PROJECT_NAME = rocks

build:	loader

install:

loader: anaconda-$(ANACONDA_VERSION) $(PATCH_FILES) 
	cd patch-files && find anaconda-$(ANACONDA_VERSION) -type f | \
		grep -v CVS | cpio -pduv ../
	env --unset=DISPLAY LANG=C make depend -C $< 
	env --unset=DISPLAY LANG=C make PROJECT_NAME=$(PROJECT_NAME) -C $<
	strip -o loader anaconda-$(ANACONDA_VERSION)/loader2/loader
	strip -o init anaconda-$(ANACONDA_VERSION)/loader2/init

anaconda-$(ANACONDA_VERSION):
	bzcat anaconda-$(ANACONDA_VERSION).tar.bz2 | tar -x


clean::
	-rm -rf anaconda-$(ANACONDA_VERSION)
	-rm -rf patch-files/anaconda-$(ANACONDA_VERSION)/po/*mo
	-rm -f loader-network loader-pcmcia loader-local loader init

