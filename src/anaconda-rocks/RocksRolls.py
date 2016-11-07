#!/usr/bin/env python

import os
import sys
import gi
import urllib
gi.require_version('Gtk','3.0')
from gi.repository import Gtk

## Need to be able to get Rocks python includes
sys.path.append('/opt/rocks/lib/python2.7/site-packages')
import rocks.media
import rocks.roll


NETWORK = 0
CD = 1
##
class RocksRollsGTK:
	"""This is an Hello World GTK application"""

	def __init__(self):
		
		self.defaultUrl ="http://central-6-2-x86-64.rocksclusters.org/install/rolls"
		self.defaultCDPath = "/mnt/cdrom"

		self.selectAll = True
		self.rollSource = NETWORK
		self.version = '7.0'

		#Set the Glade file
		self.gladefile = "RocksRolls.glade"  
		builder = Gtk.Builder()
		builder.add_from_file(self.gladefile)	
		builder.connect_signals(self)

		#Get the Main Window, and connect the "destroy" event
		self.window = builder.get_object("RollsWindow")
		self.rollUrl = builder.get_object("rollUrl")
		self.rollUrl.set_text(self.defaultUrl)

		self.listStore = builder.get_object("listRoll")
		self.selectStore = builder.get_object("selectedRolls")
		self.rollSelectCombo = builder.get_object("rollSelectCombo")
		if (self.window):
			self.window.connect("delete-event",Gtk.main_quit)
			self.window.show_all()


## Handlers
	def selectCombo(self,widget):
		id = widget.get_active_id() 
		if id.lower().startswith("network"):
			self.rollSource = NETWORK
			self.rollUrl.set_text(self.defaultUrl)	
		if id.lower().startswith("cd"):
			self.rollSource = CD 
			self.rollUrl.set_text(self.defaultCDPath)	

	def listRolls(self,widget):
		rollList=[]
		self.media=rocks.media.Media()
		url = self.rollUrl.get_text()

		#
		# if this is a CD-based roll, then mount the disk
		#
		if self.rollSource == CD:
			self.media.mountCD(path=url)
			diskid = self.media.getId(path=url)
			for d,s,f in os.walk(url):
				if d.endswith("RedHat"):
					(roll,version,arch) = d.split(os.path.sep)[-4:-1]
					rollList.append((roll, version, arch, diskid))
					
		if self.rollSource == NETWORK:
			self.media.listRolls(url, url, rollList)

		## Put the available rolls into the listStore
		self.listStore.clear()
		for roll in rollList:
		 	(name,version,arch,url) = roll	
			self.listStore.append(row=(False,name,version,arch,url))


	def selectRolls(self,widget):
		selected = filter(lambda x : x[0], self.listStore)
		for r in selected:
			name,version,arch,url = r[1],r[2],r[3],r[4]
			for a in self.selectStore:
				if (a[0],a[1],a[2]) == (name,version,arch):
					self.selectStore.remove(a.iter)
			self.selectStore.append((name,version,arch,url))
			
		self.listStore.clear()

	def doPopup(self,tview,path,c):
		dialog = Gtk.Dialog("Remove Selected Roll?",
			parent=self.window,flags=0,
			buttons=(Gtk.STOCK_CANCEL,Gtk.ResponseType.CANCEL,
			Gtk.STOCK_OK,Gtk.ResponseType.OK))
		response = dialog.run()
		if response == Gtk.ResponseType.OK:
			iter = self.selectStore.get_iter(path)
			self.selectStore.remove(iter)
		dialog.destroy()
	def removeSelected(self,a,b,c):
		print a
		print b
		print c
		self.dialog.destroy()

	def selectRoll_toggle(self,toggle,idx):
		row = self.listStore[idx]
		row[0] = not row[0]

	def selectAllRolls(self,widget):
		for row in self.listStore:
			row [0] = self.selectAll
		self.selectAll = not self.selectAll
if __name__ == "__main__":
	rr = RocksRollsGTK()
	Gtk.main()


