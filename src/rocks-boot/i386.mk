# --------------------------------------------------- -*- Makefile -*- --
# $Id: i386.mk,v 1.12 2009/05/01 19:07:20 mjk Exp $
#
# @Copyright@
# 
# 				Rocks(r)
# 		         www.rocksclusters.org
# 		       version 5.2 (Chimichanga)
# 
# Copyright (c) 2000 - 2009 The Regents of the University of California.
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
# $Log: i386.mk,v $
# Revision 1.12  2009/05/01 19:07:20  mjk
# chimi con queso
#
# Revision 1.11  2008/10/18 00:56:12  mjk
# copyright 5.1
#
# Revision 1.10  2008/03/06 23:41:56  mjk
# copyright storm on
#
# Revision 1.9  2008/01/04 21:40:53  bruno
# closer to V
#
# Revision 1.8  2007/06/23 04:03:49  mjk
# mars hill copyright
#
# Revision 1.7  2006/09/11 22:49:12  mjk
# monkey face copyright
#
# Revision 1.6  2006/08/10 00:11:15  mjk
# 4.2 copyright
#
# Revision 1.5  2005/10/12 18:10:02  mjk
# final copyright for 4.1
#
# Revision 1.4  2005/09/16 01:03:39  mjk
# updated copyright
#
# Revision 1.3  2005/05/24 21:23:02  mjk
# update copyright, release is not any closer
#
# Revision 1.2  2005/03/03 00:08:17  bruno
# for RHEL 4
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
# Revision 1.8  2004/03/25 03:15:15  bruno
# touch 'em all!
#
# update version numbers to 3.2.0 and update copyrights
#
# Revision 1.7  2003/10/27 18:12:46  bruno
# update for enterprise/3
#
# Revision 1.6  2003/10/07 20:45:45  bruno
# taroon tweaks
#
# Revision 1.5  2003/09/25 23:23:06  bruno
# tweaks for 9
#
# Revision 1.4  2003/08/15 22:34:46  mjk
# 3.0.0 copyright
#
# Revision 1.3  2003/05/22 16:39:27  mjk
# copyright
#
# Revision 1.2  2003/02/17 18:43:04  bruno
# updated copyright to 2003
#
# Revision 1.1  2002/10/30 23:32:27  mjk
# 7.2-hybrid for ia64
#

REDHAT_RELEASE	= enterprise/5

