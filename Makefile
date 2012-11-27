#
# $Id: Makefile,v 1.21 2012/11/27 00:49:04 phil Exp $
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
# Revision 1.21  2012/11/27 00:49:04  phil
# Copyright Storm for Emerald Boa
#
# Revision 1.20  2012/05/06 05:49:13  phil
# Copyright Storm for Mamba
#
# Revision 1.19  2011/07/23 02:31:14  phil
# Viper Copyright
#
# Revision 1.18  2010/09/07 23:53:23  bruno
# star power for gb
#
# Revision 1.17  2009/05/01 19:07:20  mjk
# chimi con queso
#
# Revision 1.16  2008/10/31 16:34:56  bruno
# convert more rolls to use new development environment
#
# Revision 1.15  2008/10/18 00:56:12  mjk
# copyright 5.1
#
# Revision 1.14  2008/03/06 23:41:55  mjk
# copyright storm on
#
# Revision 1.13  2007/06/23 04:03:48  mjk
# mars hill copyright
#
# Revision 1.12  2006/09/11 22:49:09  mjk
# monkey face copyright
#
# Revision 1.11  2006/08/10 00:11:13  mjk
# 4.2 copyright
#
# Revision 1.10  2006/06/19 04:20:18  bruno
# build tweaks
#
# Revision 1.9  2006/01/27 22:29:44  bruno
# stable (mostly) after integration of new foundation and localization code
#
# Revision 1.8  2005/10/12 18:10:00  mjk
# final copyright for 4.1
#
# Revision 1.7  2005/09/16 01:03:37  mjk
# updated copyright
#
# Revision 1.6  2005/05/24 21:23:01  mjk
# update copyright, release is not any closer
#
# Revision 1.5  2005/05/23 20:59:09  mjk
# *** empty log message ***
#
# Revision 1.4  2005/04/29 01:14:27  mjk
# Get everything in before travel.  Rocks-roll is looking pretty good and
# can now build the os roll (centos with updates).  It looks like only the
# first CDROM of our os/centos roll is needed with 3 extra disks.
#
# - rocks-dist cleanup (tossed a ton of code)
# - rocks-roll growth (added 1/2 a ton of code)
# - bootable rolls do not work
# - meta rolls are untested
# - rocks-dist vs. rocks-roll needs some redesign but fine for 4.0.0
#
# Revision 1.3  2004/12/13 20:28:35  bruno
# bugzilla test (bug 4) from an apple
# laptop
#
# Revision 1.2  2004/12/13 20:24:25  bruno
# bugzilla test
#
# look at bug 4
#
# Revision 1.1  2004/09/15 01:24:53  bruno
# phil -- you hate me, you love me.
#
#

-include $(ROLLSROOT)/etc/Rolls.mk
include Rolls.mk


default: roll

clean::
	rm -rf rocks-dist-bootable

