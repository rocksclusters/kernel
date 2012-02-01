#!/opt/rocks/bin/python
#
# $Id: prep-initrd-6.py,v 1.2 2012/02/01 20:48:28 phil Exp $
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
# $Log: prep-initrd-6.py,v $
# Revision 1.2  2012/02/01 20:48:28  phil
# Use subprocess instead of popen2 module
#
# Revision 1.1  2012/01/23 20:49:32  phil
# Support for build under 5 or 6
#
#
#

import string
import subprocess 
import rocks.kickstart
import os
import os.path
import sys
import rocks.bootable
from rocks.dist import DistError


class Distribution:

	def __init__(self, arch, name='rocks-dist'):
		self.arch = arch
		self.tree = None
		self.name = name

		#
		# the 'native' cpu is always first
		#
		self.cpus = []
		i86cpus = [ 'i686', 'i586', 'i486', 'i386' ]

		native = os.uname()[4]
		self.cpus.append(native)

		if native in i86cpus:
			self.cpus += i86cpus

		self.cpus.append('noarch')

	
	def getPath(self):
		return os.path.join(self.name, self.arch)
		
	def generate(self, flags=""):
		rocks.util.system('/opt/rocks/bin/rocks create distro')

		self.tree = rocks.file.Tree(os.path.join(os.getcwd(), 
			self.getPath()))
		
	def getRPMS(self):
		return self.tree.getFiles(os.path.join('RedHat', 'RPMS'))

	def getSRPMS(self):
		return self.tree.getFiles('SRPMS')
		
	def getRPM(self, name):
		l = []
		for rpm in self.getRPMS():
			try:
				if rpm.getPackageName() == name:
					l.append(rpm)
			except:
				pass
		if len(l) > 0:
			return l
		return None

	

class App(rocks.app.Application):
	"""Preps the initrd for the rocks-boot package by
	using the native kernel pakcage to build a custom
	initrd image. Uses the kickstart Application to locate 
	packages in the local distribution."""
	
	def __init__(self, argv):
		rocks.app.Application.__init__(self, argv)
		self.usage_name = 'Prep initrd'
		self.usage_version = '@VERSION@'
		

	def thinkLocally(self, name, arch):
		import rocks.file

		localtree = rocks.file.Tree(os.path.join(os.getcwd(),
			'..', '..', '..', '..', '..', '..', 'RPMS'))	

		locallist = {}
		for dir in localtree.getDirs():
			if 'CVS' in string.split(dir, os.sep):
				continue # skip CVS metadata

			for rpm in localtree.getFiles(dir):
				try:
					s = '%s-%s' % (name, arch)
					# print 's (%s), rpm (%s)' % \
						# (s, rpm.getUniqueName())
					if s == rpm.getUniqueName():
						return rpm
				except:
					pass
		

		return None


	def patchImage(self):
		#
		# patch stage2.img 
		#
		stage2_path = os.path.join(self.dist.getPath(),
					'images','stage2.img')

		self.boot.patchImage(stage2_path)

		return


	def run(self):
		print "Prep-initrd starting..."
		print "Arch:", self.getArch()

		self.dist = Distribution(self.getArch())
		self.dist.generate()

		self.boot = rocks.bootable.Bootable(self.dist)

		#
		# install some packages to this local build tree
		#
		pkgs = [ 'anaconda', 'lighttpd', 'rocks-tracker' ]

		for pkg in pkgs:
			RPM = self.thinkLocally(pkg, self.getArch())
			if not RPM:
				RPM = self.boot.getBestRPM(pkg)

			if not RPM:
				raise DistError, "Could not find %s rpm" % (pkg)

			self.boot.applyRPM(RPM, 
				os.path.join(os.getcwd(), pkg),
					flags='--noscripts --excludedocs')

                #
                # patch mk-images to not use EFI 
                #

                subprocess.call('/bin/cp ../../../../mk-images anaconda/usr/lib/anaconda-runtime/', shell=True)

		#
		# make stage2.img 
		#
		cwd = os.getcwd()
		os.chdir('anaconda/usr/lib/anaconda-runtime')
		version = "%s.%s.%s" % \
			(self.projectVersionMajor, 
			self.projectVersionMinor, 
			self.projectVersionMicro)

		# Capitalize first letter in project. Selfishly rocks biased.
		projectName = self.projectName[0].upper() + self.projectName[1:]

		cmd = './buildinstall --product "%s" ' % projectName \
			+ '--brand "rocks" ' \
			+ '--version "%s" ' % version \
			+ '--release "0" ' \
			+ os.path.join(cwd, self.dist.getPath())
		print "Buildinstall cmd:", cmd
		subprocess.call(cmd, shell=True)
		os.chdir(cwd)

		if self.getArch() == 'ia64':
			images = os.path.join(self.dist.getPath(), 'images')
			subprocess.call('cp %s/boot.img .' % (images), shell=True)

			subprocess.call('mount -o loop boot.img mnt', shell=True)
			subprocess.call('cp mnt/initrd.img . ', shell=True)
			subprocess.call('umount mnt', shell=True)
		else:
			cmd = 'cp -f %s initrd-boot.img' % \
				os.path.join(self.dist.getPath(),
                                'isolinux', 'initrd.img')
			rc = subprocess.call(cmd, shell=True)
			if rc != 0:
				raise DistError, "Could not find initrd. " + \
					"Did buildinstall complete?"

			#cmd = 'cp -f %s initrd-xen.img' % \
			#	os.path.join(self.dist.getPath(),
                        #        'images', 'xen', 'initrd.img')
			# rc = subprocess.call(cmd, shell=True)

		#
		# install the kernel RPMs. this is used when building 
		# driver disks
		#
		pkgs = [ 'kernel', 'kernel-devel' ]

		if self.getArch() == "i386":
			kernelarch = "i686"
			pkgs.append('kernel-PAE')
			pkgs.append('kernel-PAE-devel')
		else:
			kernelarch = self.getArch()

		for pkg in pkgs:
			RPM = self.thinkLocally(pkg, kernelarch)
			if not RPM:
				RPM = self.boot.getBestRPM(pkg)

			if not RPM:
				raise DistError, "Could not find %s rpm" % (pkg)

			self.boot.applyRPM(RPM, 
				os.path.join(os.getcwd(), 'kernels'),
					flags='--noscripts --excludedocs')

		return


# Main
app = App(sys.argv)
app.parseArgs()
app.run()

