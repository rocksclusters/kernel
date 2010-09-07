# Make file for DMAC-inspired splash screen.
#
# @Copyright@
# 
# 				Rocks(r)
# 		         www.rocksclusters.org
# 		         version 5.4 (Maverick)
# 
# Copyright (c) 2000 - 2010 The Regents of the University of California.
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
# Revision 1.9  2010/09/07 23:53:23  bruno
# star power for gb
#
# Revision 1.8  2009/05/01 19:07:21  mjk
# chimi con queso
#
# Revision 1.7  2008/10/18 00:56:13  mjk
# copyright 5.1
#
# Revision 1.6  2008/03/17 16:55:26  bruno
# splash screen is done.
#
# Revision 1.5  2008/03/14 21:04:24  bruno
# works
#
# Revision 1.4  2008/03/06 23:41:56  mjk
# copyright storm on
#
# Revision 1.3  2007/12/13 02:53:41  bruno
# can now build a bootable kernel CD and build a physical frontend with V
# on RHEL 5 update 1
#
# Revision 1.2  2007/06/23 04:03:49  mjk
# mars hill copyright
#
# Revision 1.1  2007/01/11 23:18:44  bruno
# first clean compile
#
# there is no way in hell this works
#
# Revision 1.16  2006/09/13 21:23:19  mjk
# *** empty log message ***
#
# Revision 1.15  2006/09/13 00:05:15  mjk
# fixup splash screen
#
# Revision 1.14  2006/09/11 22:49:13  mjk
# monkey face copyright
#
# Revision 1.13  2006/08/10 00:11:17  mjk
# 4.2 copyright
#
# Revision 1.12  2006/01/27 22:29:45  bruno
# stable (mostly) after integration of new foundation and localization code
#
# Revision 1.11  2006/01/25 22:22:56  bruno
# compute nodes build again
#
# Revision 1.10  2006/01/21 04:17:31  mjk
# use a _lang file
#
# Revision 1.9  2006/01/18 23:43:50  mjk
# added log tag
#
#
# 11-2003 fds
# Run in folder with boot_logo.py and create_boot.msg.py
# Obviates the need for generate_boot_lss
# Needs ImageMagik.
#
# Requires "message" text file and "label.png" image to
# be present in the local directory.

default build install: boot.msg

LANG=en_US
# LANG=ko

splash.png: 
	cp label.$(LANG).png splash.png
	#convert -resize 640x480! label.$(LANG).png splash.png
	#convert -colors 14 label.$(LANG).png -write splash.png
	#mogrify -colors 8 splash.png

boot.msg: splash.png
	pngtopnm splash.png | ppmtolss16 '#fdfffc=0' '#050807=7' > splash.lss
	./create_boot.msg.py message boot.msg

clean:
	rm -f splash*
	rm -f boot.msg
