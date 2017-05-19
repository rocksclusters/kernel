#
# Copyright (C) 2013  Red Hat, Inc.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions of
# the GNU General Public License v.2, or (at your option) any later version.
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY expressed or implied, including the implied warranties of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.  You should have received a copy of the
# GNU General Public License along with this program; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.  Any Red Hat trademarks that are incorporated in the
# source code or documentation are not subject to the GNU General Public
# License and may only be used or replicated with the express permission of
# Red Hat, Inc.
#
# Red Hat Author(s): Vratislav Podzimek <vpodzime@redhat.com>
#

"""Module with the RocksRollsSpoke class."""

# import gettext
# _ = lambda x: gettext.ldgettext("hello-world-anaconda-plugin", x)

# will never be translated
_ = lambda x: x
N_ = lambda x: x

import os
import sys
import gi
import urllib
import subprocess
gi.require_version('Gtk','3.0')
from gi.repository import Gtk, GObject

### the path to addons is in sys.path so we can import things from org_rocks_rolls
from org_rocks_rolls.categories.RocksRolls import RocksRollsCategory
from org_rocks_rolls import RocksEnv
from pyanaconda.ui.gui import GUIObject
from pyanaconda.ui.gui.spokes import NormalSpoke
from pyanaconda.ui.common import FirstbootSpokeMixIn
import thread




## Defines for Roll Source
NETWORK = 0
CD = 1

# export only the spoke, no helper functions, classes or constants
__all__ = ["RocksRollsSpoke"]

class RocksRollsSpoke(FirstbootSpokeMixIn, NormalSpoke):
    """
    Class for the RocksRolls spoke. This spoke will be in the RocksRollsCategory 
    category and thus on the Summary hub. It is a very simple example of a unit
    for the Anaconda's graphical user interface. Since it is also inherited form
    the FirstbootSpokeMixIn, it will also appear in the Initial Setup (successor
    of the Firstboot tool).


    :see: pyanaconda.ui.common.UIObject
    :see: pyanaconda.ui.common.Spoke
    :see: pyanaconda.ui.gui.GUIObject
    :see: pyanaconda.ui.common.FirstbootSpokeMixIn
    :see: pyanaconda.ui.gui.spokes.NormalSpoke

    """

    ### class attributes defined by API ###

    # list all top-level objects from the .glade file that should be exposed
    # to the spoke or leave empty to extract everything
    # builderObjects = ["rocksRollsSpokeWindow", "buttonImage"]
    # builderObjects = ["RollsWindow"]

    # the name of the main window widget
    mainWidgetName = "RollsWindow"

    # name of the .glade file in the same directory as this source
    uiFile = "RocksRolls.glade"

    # category this spoke belongs to
    category = RocksRollsCategory

    # spoke icon (will be displayed on the hub)
    # preferred are the -symbolic icons as these are used in Anaconda's spokes
    icon = "emblem-system-symbolic"

    # title of the spoke (will be displayed on the hub)
    title = N_("_ROCKS ROLLS")

    ### methods defined by API ###
    def __init__(self, data, storage, payload, instclass):
        """
        :see: pyanaconda.ui.common.Spoke.__init__
        :param data: data object passed to every spoke to load/store data
                     from/to it
        :type data: pykickstart.base.BaseHandler
        :param storage: object storing storage-related information
                        (disks, partitioning, bootloader, etc.)
        :type storage: blivet.Blivet
        :param payload: object storing packaging-related information
        :type payload: pyanaconda.packaging.Payload
        :param instclass: distribution-specific information
        :type instclass: pyanaconda.installclass.BaseInstallClass

        """

        NormalSpoke.__init__(self, data, storage, payload, instclass)
        self.clientInstall = RocksEnv.RocksEnv().clientInstall

        self.defaultUrl ="http://beta6.rocksclusters.org/install/rolls"
        self.defaultCDPath = "/run/install/repo"

        self.selectAll = True
        self.rollSource = NETWORK
        self.version = '7.0'
        self.requireDB = True

        self.requiredRolls = ('core','base','kernel')


    def initialize(self):
        """
        The initialize method that is called after the instance is created.
        The difference between __init__ and this method is that this may take
        a long time and thus could be called in a separated thread.

        :see: pyanaconda.ui.common.UIObject.initialize

        """

        NormalSpoke.initialize(self)

        import logging
        self.log = logging.getLogger('anaconda')
        self.log.info("Rocks was here")

        #self.builder.connect_signals(self)

        #Get the Main Window, and connect the "destroy" event
        # self.window = self.builder.get_object("RollsWindow")
        self.rollUrl = self.builder.get_object("rollUrl")
        self.rollUrl.set_text(self.defaultUrl)

        self.listStore = self.builder.get_object("listRoll")
        self.selectStore = self.builder.get_object("selectedRolls")
        self.rollSelectCombo = self.builder.get_object("rollSelectCombo")

        sys.path.append('/opt/rocks/lib/python2.7/site-packages')
        import rocks.media
        import rocks.installcgi
        self.media=rocks.media.Media()
        self.install = rocks.installcgi.InstallCGI(rootdir="/tmp/rocks")
        ## from template
        # self._entry = self.builder.get_object("textEntry")

    def refresh(self):
        """
        The refresh method that is called every time the spoke is displayed.
        It should update the UI elements according to the contents of
        self.data.

        :see: pyanaconda.ui.common.UIObject.refresh

        """

        # self._entry.set_text(self.data.addons.org_rocks_rolls.text)

    def apply(self):
        """
        The apply method that is called when the spoke is left. It should
        update the contents of self.data with values set in the GUI elements.

        """

        self.data.addons.org_rocks_rolls.text = "Rocks Rolls Visited" 
        # need to create a copy of selectStore entries, otherwise deepcopy
        # used in other anaconda widgets won't work
        rolls = []
        for r in self.selectStore:
            rolls.append((r[:]))
        self.data.addons.org_rocks_rolls.rolls = rolls 
        self.log.info("ROCKS: data %s" % dir(self.data))
        self.log.info("ROCKS: data %s" % dir(self.data.addons))
        self.log.info("ROCKS: data %s" % dir(self.data.addons.org_rocks_rolls))
        self.log.info("ROCKS: data %s" % self.data.addons.org_rocks_rolls.rolls.__str__())

    def execute(self):
        """
        The excecute method that is called when the spoke is left. It is
        supposed to do all changes to the runtime environment according to
        the values set in the GUI elements.

        """

        if self.completed and not self.clientInstall:
            self.writeRollsXML()
            if self.requireDB:
                self.builddb()
                self.requireDB = False

    @property
    def ready(self):
        """
        The ready property that tells whether the spoke is ready (can be visited)
        or not. The spoke is made (in)sensitive based on the returned value.

        :rtype: bool

        """

        # this spoke is always ready
        return True

    @property
    def completed(self):
        """
        The completed property that tells whether all mandatory items on the
        spoke are set, or not. The spoke will be marked on the hub as completed
        or uncompleted acording to the returned value.

        :rtype: bool

        """

        if self.clientInstall:
            return True
        #return bool(self.data.addons.org_rocks_rolls.text)
        sel = map(lambda x: x[0], self.selectStore)
        req = filter(lambda x: x in self.requiredRolls, sel)
        if len(req) >= len(self.requiredRolls):
            self.data.addons.org_rocks_rolls.haverolls = True 
            return True
        else:
            self.data.addons.org_rocks_rolls.haverolls = False 
            return False

    @property
    def mandatory(self):
        """
        The mandatory property that tells whether the spoke is mandatory to be
        completed to continue in the installation process.

        :rtype: bool

        """

        # this is an optional spoke that is not mandatory to be completed
        return True

    @property
    def status(self):
        """
        The status property that is a brief string describing the state of the
        spoke. It should describe whether all values are set and if possible
        also the values themselves. The returned value will appear on the hub
        below the spoke's title.

        :rtype: str

        """

        return "%d rolls selected" % len (self.selectStore)

    ### handlers ###
    def on_entry_icon_clicked(self, entry, *args):
        """Handler for the textEntry's "icon-release" signal."""

        pass
        # entry.set_text("")

    def on_main_button_clicked(self, *args):
        """Handler for the mainButton's "clicked" signal."""

        # every GUIObject gets ksdata in __init__
        dialog = RocksRollsDialog(self.data)

        # show dialog above the lightbox
        with self.main_window.enlightbox(dialog.window):
            dialog.run()

    ## Handlers for the RocksRolls 
    def selectCombo(self,widget):
        id = widget.get_active_text() 
        if id.lower().startswith("network"):
            self.rollSource = NETWORK
            self.rollUrl.set_text(self.defaultUrl)    
        if id.lower().startswith("cd"):
            self.rollSource = CD 
            self.rollUrl.set_text(self.defaultCDPath)    

    def listRolls(self,widget):

        ###  Need to be able to get Rocks python includes
        rollList=[]
        url = self.rollUrl.get_text()

        #
        # if this is a CD-based roll, then mount the disk
	# just read from a local path
        #
        if self.rollSource == CD:
            # XXX Fix this to really do mounting in 7 XXX
            #self.media.mountCD(path=url)
            #diskid = self.media.getId(path=url)
            diskid = "repo" 
            for d,s,f in os.walk(url):
                if d.endswith("RedHat"):
                    (roll,version,arch) = d.split(os.path.sep)[-4:-1]
                    rollList.append((roll, version, arch, "file:%s" % url, diskid))
                    
        if self.rollSource == NETWORK:
            netRoll = []
            self.media.listRolls(url, url, netRoll) 
            for (name,version,arch,url) in netRoll:
                rollList.append((name, version, arch, url, None))

        ## Put the available rolls into the listStore
        self.listStore.clear()
        for roll in rollList:
            (name,version,arch,url,diskid) = roll    
            self.listStore.append(row=(False,name,version,arch,url,diskid))


    def selectRolls(self,widget):
        selected = filter(lambda x : x[0], self.listStore)
        for r in selected:
            name,version,arch,url,diskid = r[1],r[2],r[3],r[4],r[5]
            for a in self.selectStore:
                if (a[0],a[1],a[2]) == (name,version,arch):
                    self.selectStore.remove(a.iter)
            self.selectStore.append((name,version,arch,url,diskid))      
            self.log.info("ROCKS - Select Rolls %s" % (name,version,arch,url,diskid).__str__()) 
            self.install.getKickstartFiles((name,version,arch,"%s/" % url,diskid))
        self.listStore.clear()

    def doPopup(self,tview,path,c):
        dialog = Gtk.Dialog("Remove Selected Roll?",
            parent=self.main_window,flags=0,
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


    ### Helper Methods

    def writeRollsXML(self):
        fname = "/tmp/rolls.xml"
        f = open(fname,"w")
        f.write('<rolls>\n')
        for roll in self.selectStore:
            #
            # rewrite rolls.xml
            #
            (rollname, rollver, rollarch, rollurl, diskid) = roll

            str = '<roll\n'
            str += '\tname="%s"\n' % (rollname)
            str += '\tversion="%s"\n' % (rollver)
            str += '\tarch="%s"\n' % (rollarch)
            str += '\turl="%s"\n' % (rollurl)
            str += '\tdiskid="%s"\n' % (diskid)
            str += '/>'

            f.write('%s\n' % (str))

        f.write('</rolls>\n')
        f.close()
    def builddb(self):
        dialog = Gtk.Dialog("Build Database", parent=self.main_window, flags=0,
            buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
             Gtk.STOCK_OK, Gtk.ResponseType.OK))
        dialog.set_default_size(150, 100)

        label = Gtk.Label("Building Rocks Database. Please Wait")
        box = dialog.get_content_area()
        box.add(label)
        dialog.show_all()
        thread.start_new_thread(self.buildit,(dialog,))
        #self.buildit(dialog)

    def buildit(self,dialog):
        cmd = ["/opt/rocks/bin/builddb.sh", "/tmp/rocks"]
        p1 = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        p1.communicate() 
        dialog.destroy()

class RocksRollsDialog(GUIObject):
    """
    Class for the sample dialog.

    :see: pyanaconda.ui.common.UIObject
    :see: pyanaconda.ui.gui.GUIObject

    """

    builderObjects = ["sampleDialog"]
    mainWidgetName = "sampleDialog"
    uiFile = "rocks_rolls.glade"

    def __init__(self, *args):
        GUIObject.__init__(self, *args)

    def initialize(self):
        GUIObject.initialize(self)

    def run(self):
        """
        Run dialog and destroy its window.

        :returns: respond id
        :rtype: int

        """

        ret = self.window.run()
        self.window.destroy()

        return ret

if __name__ == "__main__":
    from gi.repository import Gtk
    rr = RocksRollsSpoke(None,None,None,None)
    rr.initialize()
    rr.refresh()
    Gtk.main()

# vim:sw=4:ts=4:et
