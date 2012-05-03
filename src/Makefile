# $Id: Makefile,v 1.19 2012/05/03 05:17:20 phil Exp $
#
# @Copyright@
# 
# 				Rocks(r)
# 		         www.rocksclusters.org
# 		         version 5.4.3 (Viper)
# 
# Copyright (c) 2000 - 2011 The Regents of the University of California.
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
# 	Development Team at the San Diego Supercomputer Center at the
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
# Revision 1.19  2012/05/03 05:17:20  phil
# Build keyutils on 5
#
# Revision 1.18  2012/05/02 05:26:22  phil
# Don't build keyutils
#
# Revision 1.17  2011/07/23 02:31:14  phil
# Viper Copyright
#
# Revision 1.16  2010/09/07 23:53:23  bruno
# star power for gb
#
# Revision 1.15  2009/05/01 19:07:20  mjk
# chimi con queso
#
# Revision 1.14  2008/10/18 00:56:12  mjk
# copyright 5.1
#
# Revision 1.13  2008/03/06 23:41:55  mjk
# copyright storm on
#
# Revision 1.12  2007/06/23 04:03:48  mjk
# mars hill copyright
#
# Revision 1.11  2006/12/02 01:04:59  anoop
# Ridiculously big ass commit.
# Also known as the week after thanksgiving 2006. Or "The day Anoop
# broke Rocks".
#
# Main Changes.
#
# 1. Added a roll-profile.mk file. This is meant as a makefile for building
# the profile rpm containing all the XML files meant for the roll. This is a
# breakaway from the spec.in file method of building the profile RPM.
#
# 2. The variable PWD is now changed to CURDIR. The main reason for this is
# PWD is supplied by the shell. CURDIR is the variable supplied by gmake itself.
# This means we can have a slightly more platform independant way of doing things.
# Also Solaris was failing to set PWD correctly in the source directories, wreaking
# havoc on the location of the BUILD and PKG directories.
#
# Revision 1.10  2006/09/11 22:49:11  mjk
# monkey face copyright
#
# Revision 1.9  2006/08/10 00:11:15  mjk
# 4.2 copyright
#
# Revision 1.8  2005/10/12 18:10:01  mjk
# final copyright for 4.1
#
# Revision 1.7  2005/09/16 01:03:38  mjk
# updated copyright
#
# Revision 1.6  2005/07/27 01:54:39  bruno
# checkpoint
#
# Revision 1.5  2005/06/29 02:37:32  bruno
# now have ability to build a kernel rpm from kernel.org
#
# Revision 1.4  2005/05/24 21:23:01  mjk
# update copyright, release is not any closer
#
# Revision 1.3  2005/04/12 18:16:36  bruno
# GM (the myrinet driver) now runs under 4.0.0
#
# Revision 1.2  2005/03/25 01:02:36  bruno
# be clean
#
# Revision 1.1  2004/11/23 02:41:12  bruno
# made the kernel roll bootable.
#
# moved 'rocks-boot' here -- it is now uses vmlinuz and builds the initrd.img
# file from the local (if present in the local RPMS directory) or from the
# current distro.
#
# if you want to use a specific kernel, just drop it in the RPMS directory.
#

#
# make sure kernel.org is built first and rocks-boot is built last
#
ROCKS.OS.VERSION.MAJOR=$(shell lsb_release -rs | cut -d . -f 1)

SRCDIRS = `find . -type d -maxdepth 1 \
	-not -name CVS \
	-not -name rocks-kernel \
	-not -name kernel.org \
	-not -name keyutils \
	-not -name rocks-boot \
	-not -name .` 

ifeq ($(strip $(ROCKS.OS.VERSION.MAJOR)), 5)
SRCDIRS += keyutils
endif

SRCDIRS += rocks-boot

default:	pkg

foo:
	echo $(SRCDIRS)
	echo $(ROCKS.OS.VERSION.MAJOR)

copyright rpm pkg clean::
	for i in $(SRCDIRS) ; do \
		(cd $$i; make $@) ; \
	done

