#
# $Id: Makefile,v 1.19 2012/11/27 00:49:05 phil Exp $
#
# @Copyright@
# 
# 				Rocks(r)
# 		         www.rocksclusters.org
# 		         version 5.6 (Emerald Boa)
# 		         version 6.1 (Emerald Boa)
# 
# Copyright (c) 2000 - 2013 The Regents of the University of California.
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
# Revision 1.19  2012/11/27 00:49:05  phil
# Copyright Storm for Emerald Boa
#
# Revision 1.18  2012/05/06 05:49:14  phil
# Copyright Storm for Mamba
#
# Revision 1.17  2011/07/23 02:31:14  phil
# Viper Copyright
#
# Revision 1.16  2010/09/07 23:53:23  bruno
# star power for gb
#
# Revision 1.15  2010/06/22 21:55:09  mjk
# remove ROCKSROOT
#
# Revision 1.14  2009/05/01 19:07:20  mjk
# chimi con queso
#
# Revision 1.13  2008/10/18 00:56:12  mjk
# copyright 5.1
#
# Revision 1.12  2008/07/31 19:19:46  bruno
# can build all varieties of kernels now
#
# Revision 1.11  2008/04/10 21:16:07  bruno
# first whack at building kernels in V
#
# Revision 1.10  2008/04/10 20:10:03  bruno
# build kernels in V
#
# Revision 1.9  2008/03/06 23:41:55  mjk
# copyright storm on
#
# Revision 1.8  2007/06/23 04:03:48  mjk
# mars hill copyright
#
# Revision 1.7  2006/09/11 22:49:11  mjk
# monkey face copyright
#
# Revision 1.6  2006/08/10 00:11:15  mjk
# 4.2 copyright
#
# Revision 1.5  2006/07/19 20:12:24  anoop
# Made sure that both smp and normal custom kernels build correctly and boot correctly
#
# Revision 1.4  2005/10/12 18:10:01  mjk
# final copyright for 4.1
#
# Revision 1.3  2005/09/16 01:03:39  mjk
# updated copyright
#
# Revision 1.2  2005/06/29 16:54:26  bruno
# touch up
#
# Revision 1.1  2005/06/29 02:37:32  bruno
# now have ability to build a kernel rpm from kernel.org
#
#

# MAKE.iscontrib  = 1
REDHAT.ROOT     = $(PWD)/../../

-include $(ROCKSROOT)/etc/Rules.mk
include Rules.mk

ifeq ($(PAE),1)
SEDSPEC += -e 's%@KERNELNAME@%$(NAME)-PAE%g'
MAKE.rpmflag += --define="with_extension 1" --define="kernelextension PAE"
endif

ifeq ($(XEN),1)
SEDSPEC += -e 's%@KERNELNAME@%$(NAME)-xen%g'
MAKE.rpmflag += --define="with_extension 1" --define="kernelextension xen"
endif

ifeq ($(PAE)$(XEN),00)
SEDSPEC += -e 's%@KERNELNAME@%$(NAME)%g'
MAKE.rpmflag += --define="with_extension 0"
endif

ifeq ($(ARCH),i386)
MAKE.rpmflag += --target i686
endif


default:	rpm

pretar::	rpm-mkdirs
	if [ $(XEN) -eq 1 ]  && [ $(PAE) -eq 1 ] ; then \
		echo ; \
		echo ; \
		echo "      XEN and PAE flags are both set in version.mk" ; \
		echo "      You can't set both." ; \
		echo ; \
		echo ; \
		exit -1 ; \
	fi
	cp linux-$(VERSION).tar.gz $(REDHAT.SOURCES)
	cp config-$(VERSION) $(REDHAT.SOURCES)/.config

