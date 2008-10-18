# --------------------------------------------------- -*- Makefile -*- --
# $Id: Makefile,v 1.12 2008/10/18 00:56:12 mjk Exp $
#
# @Copyright@
# 
# 				Rocks(r)
# 		         www.rocksclusters.org
# 		           version 5.1  (VI)
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
# Revision 1.12  2008/10/18 00:56:12  mjk
# copyright 5.1
#
# Revision 1.11  2008/03/06 23:41:56  mjk
# copyright storm on
#
# Revision 1.10  2007/06/23 04:03:49  mjk
# mars hill copyright
#
# Revision 1.9  2006/09/11 22:49:12  mjk
# monkey face copyright
#
# Revision 1.8  2006/08/10 00:11:15  mjk
# 4.2 copyright
#
# Revision 1.7  2006/03/24 20:25:17  bruno
# make mini_httpd statically linked
#
# Revision 1.6  2006/03/18 18:10:34  bruno
# updated to latest version
#
# nuked the spec file -- now builds strickly from the Makefile
#
# Revision 1.2  2006/03/16 17:34:00  bruno
# added preferences to remove toolbars and to not warn about insecure form
# posting
#
# Revision 1.1  2006/02/02 03:54:46  bruno
# the installer web browser is ready
#
# Revision 1.1  2005/12/31 07:50:53  mjk
# build python from source
#

PKGROOT		= /usr/bin
REDHAT.ROOT     = $(PWD)/../../
ROCKSROOT       = ../../../../..
-include $(ROCKSROOT)/etc/Rules.mk
include Rules.mk

# CHROMEDIR = $(PKGROOT)/lib/firefox-$(FIREFOX_VERSION)/defaults/profile/chrome


build:
	tar zxvf $(NAME)-$(VERSION).$(RELEASE).tar.gz
	cd patch-files && find $(NAME)-$(VERSION).$(RELEASE) -type f | \
		grep -v CVS | cpio -pduv ../
	( \
		cd $(NAME)-$(VERSION).$(RELEASE) ; \
		make SSL_INCDIR=/usr/include/openssl \
			SSL_LIBDIR=/usr/lib \
			SSL_DEFS=-DUSE_SSL \
			SSL_INC=-I/usr/include/openssl \
			SSL_LIBS='-lssl -lcrypto -L/usr/kerberos/lib \
				-L/usr/kerberos/lib64 \
				-lkrb5 -lk5crypto -lcom_err -lresolv' \
			CFLAGS='-static -g -DUSE_SSL -DSDSC \
				-I/usr/include/openssl \
				-I/usr/kerberos/include' ; \
	)

install::
	mkdir -p $(ROOT)/$(PKGROOT)
	(								\
		cd $(NAME)-$(VERSION).$(RELEASE) ;			\
		install --mode=755 mini_httpd $(ROOT)/$(PKGROOT) ;	\
	)

clean::
	rm -rf $(NAME)-$(VERSION).$(RELEASE)
	rm -f $(NAME).spec.in

