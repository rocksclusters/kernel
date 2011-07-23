#!/bin/bash
# $Id: prep-grub.sh,v 1.9 2011/07/23 02:31:14 phil Exp $
# $Log: prep-grub.sh,v $
# Revision 1.9  2011/07/23 02:31:14  phil
# Viper Copyright
#
# Revision 1.8  2010/09/07 23:53:23  bruno
# star power for gb
#
# Revision 1.7  2009/05/01 19:07:20  mjk
# chimi con queso
#
# Revision 1.6  2008/10/18 00:56:12  mjk
# copyright 5.1
#
# Revision 1.5  2008/03/06 23:41:56  mjk
# copyright storm on
#
# Revision 1.4  2007/06/23 04:03:49  mjk
# mars hill copyright
#
# Revision 1.3  2006/09/11 22:49:12  mjk
# monkey face copyright
#
# Revision 1.2  2006/08/10 00:11:15  mjk
# 4.2 copyright
#
# Revision 1.1  2006/04/02 07:01:41  phil
# Some fixups
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
# 
# Purpose: edit the rocks.conf file to reflect a new central, ip, dns, gateway,
# netmask.  When coupled with new versions of 
#            /boot/kickstart/default/vmlinuz
#            /boot/kickstart/default/initrd.img
#
# A frontend can be upgraded remotely
#
#
# Typical usage scenario:
# A) install the vmlinuz/initrd.img files either from a tar file or
#    rpm -Uvh rocks-boot-[new version].<arch>.rpm
#    (For version 3.3 of rocks to 4.X, one will have to install from
#     tar, since RPM formats have changed)
# B) prep-grub -c <newcentral> > /boot/grub/rocks.conf.new
# C) mv /boot/grub/rocks.conf.new /boot/grub/rocks.conf
# D) /boot/kickstart/cluster-kickstart
#
# 

function usage () 

{
 	echo "$0 [-h][-v][-c central][-d dns][-i ip][-n netmask][-g gateway]"
	echo "modify /boot/grub/rocks.conf to use a different central for upgrade"  
}

function cleanup ()
{
/bin/rm -f $1
}

SED=/bin/sed
central=''
netmask=''
gateway=''
dns=''
ip=''
verbose=''
help=''
grubfile='/boot/grub/rocks.conf'
tmpfile=$grubfile".$$"

while getopts c:d:g:i:n:hv  opt
do
    case "$opt" in
      v)  verbose="true";;
      h)  usage; exit 1;;
      c)  central="$OPTARG";;
      d)  dns="$OPTARG";;
      g)  gateway="$OPTARG";;
      i)  ip="$OPTARG";;
      n)  netmask="$OPTARG";;
      \?)		# unknown flag
      	  echo >&2 \
	  usage 
	  exit 1;;
    esac
done
shift `expr $OPTIND - 1`

if [ ! -f $grubfile ]; then
	echo "Error: $grubfile does not exist"
        exit 1;
fi

# make a temporary copy of the grub file
/bin/cp $grubfile $tmpfile

# replace restore with upgrade
$SED -i "s/restore/upgrade/" $tmpfile 
# remove dropcert if there
$SED -i "s/dropcert//" $tmpfile 
# now put dropcert back 
$SED -i "s/upgrade/upgrade dropcert/" $tmpfile 

# Now change anything that needs changing
if [ "x$central" != "x" ]; then
         substring="s/central=[[:alnum:][:punct:]]*/central=$central/"
	 $SED -i "$substring" $tmpfile 
fi 

if [ "x$dns" != "x" ]; then
         substring="s/dns=[[:alnum:][:punct:]]*/dns=$dns/"
	 $SED -i "$substring" $tmpfile 
fi 

if [ "x$ip" != "x" ]; then
         substring="s/ip=[[:alnum:][:punct:]]*/ip=$ip/"
	 $SED -i "$substring" $tmpfile 
fi 

if [ "x$netmask" != "x" ]; then
         substring="s/netmask=[[:alnum:][:punct:]]*/netmask=$netmask/"
	 $SED -i "$substring" $tmpfile 
fi 

if [ "x$gateway" != "x" ]; then
         substring="s/gateway=[[:alnum:][:punct:]]*/gateway=$gateway/"
	 $SED -i "$substring" $tmpfile 
fi 

cat $tmpfile
cleanup $tmpfile

