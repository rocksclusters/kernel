#!/usr/bin/python
#$Log: create_boot.msg.py,v $
#Revision 1.1  2012/01/23 20:43:56  phil
#Latest Anaconda for 6
#
#Revision 1.2  2008/03/17 16:55:26  bruno
#splash screen is done.
#
#Revision 1.1  2007/01/11 23:18:44  bruno
#first clean compile
#
#there is no way in hell this works
#
#Revision 1.1  2005/01/03 21:22:47  bruno
#first whack at RHEL 4 (nahant-beta)
#
#Revision 1.1  2004/11/23 02:41:14  bruno
#made the kernel roll bootable.
#
#moved 'rocks-boot' here -- it is now uses vmlinuz and builds the initrd.img
#file from the local (if present in the local RPMS directory) or from the
#current distro.
#
#if you want to use a specific kernel, just drop it in the RPMS directory.
#
#Revision 1.1  2003/11/12 20:02:39  fds
#Updated Dmac splash work.
#
#Revision 1.3  2003/08/28 20:07:26  dmac
#now indents in boot.msg which makes it look better
#cleaned up code
#
#Revision 1.2  2003/08/20 21:40:26  dmac
#fixed log issue
#
import string
import sys
import os

#Creates the file boot.msg, reading from a file
#called message. It indents the contents, and adds
#magic for syslinux to read the image logo, then saves
#as a file
class create_boot_msg:
	def open_file(self, file_path, arguments='rb'):
		try:
			fileobj=open(file_path, arguments)
		except:
			print '%s cannot be opened' % (file_path)
			sys.exit()
	
		return fileobj
	def close_file(self, fileobj):
		try:
			file_obj.close()
		except:
			print 'Error closing file'
	
	def read_file(self, fileobj):
		list=[]
		for line in fileobj.readlines():
			list.append(line)
		return list
	def write_file(self, fileobj, list):
		for line in list:
			fileobj.write(line)
	def write_magic(self, fileobj, name_image):
		# clear the screen and home the cursor
		fileobj.write('\014\n')

		# a reference to the splash screen
		fileobj.write('\030')
		fileobj.write(name_image)
		fileobj.write('\n')
	
	def indent_list(self, list):
		newlist=[]
		#keeps track if previous line indented
		#1 is yes, 0 is no
		indented=0
		for line in list:
			#if line is supposed to be indented,
			#make it tabbed, else add a newline
			if line[0]=='\t' or line[0]==' ':
				line=line[1:]
				line=string.join(['\t',line],'')	
				indented=1
			elif indented==1:
				line=string.join(['\n',line],'')
				indented=0
			else:
				pass
			newlist.append(line)
		
		return newlist
	
	def run(self, path_file_in, path_file_out):
		infile=self.open_file(path_file_in, 'rb')
		outfile=self.open_file(path_file_out, 'wb')
		
		messagelist=self.read_file(infile)
		new_messagelist=self.indent_list(messagelist)
		
		self.write_file(outfile,new_messagelist)
		#File name of image is assumed to be splash.lss
		self.write_magic(outfile, 'splash.lss')

		infile.close()
		outfile.close()

if __name__=='__main__':
	import sys
	
	arguments='%s\n<arg1>file that contains message\n<arg2>\
file that will hold message w/ special magic\n' % (sys.argv[0])
	
	#Check to make sure there are the correct number of args
	if len(sys.argv)!= 3:
		print arguments
		sys.exit()
		
	boot_msg=create_boot_msg()
	boot_msg.run(sys.argv[1], sys.argv[2])	
		
