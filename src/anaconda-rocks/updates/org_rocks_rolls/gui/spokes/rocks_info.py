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
from pyanaconda.ui.gui import GUIObject
from pyanaconda.ui.gui.spokes import NormalSpoke
from pyanaconda.ui.common import FirstbootSpokeMixIn
from pyanaconda import network
import thread
import json
import inspect

# export only the spoke, no helper functions, classes or constants
__all__ = ["RocksConfigSpoke"]

# File that holds JSON of clusterInfo variables.
INFOFILE = "infoVars.json"
FIELDNAMES = ['param','value','varname','infoHelp','required','display','validate' ]
VARIDX = FIELDNAMES.index("varname")
VALIDX = FIELDNAMES.index("value")

# Field names are:
#        param: String. Displayed Parameter Text 
#        value: String. initial value of this parameter 
#        varname: String.  Name of parameter as it appears in a kickstart file 
#        infoHelp: String. Mouse over help string in interface 
#        required: Boolean.  Required that that this parameter has a value 
#        display: Boolean. Display on the UI 
#        validate: String.  Validation Method 
# 
# These field are mapped into gui's ClusterInfoStore (in RocksInfo.glade).
# ClusterInfoStore only has indices, so must keep the map of these dictionary 
# values consistent with their indices in the UI

class RocksConfigSpoke(FirstbootSpokeMixIn, NormalSpoke):
    """
    Class for the RocksConfig spoke. This spoke will be in the RocksRollsCategory 
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

    # the name of the main window widget
    mainWidgetName = "RocksInfoWindow"

    # name of the .glade file in the same directory as this source
    uiFile = "RocksInfo2.glade"

    # category this spoke belongs to
    category = RocksRollsCategory

    # spoke icon (will be displayed on the hub)
    # preferred are the -symbolic icons as these are used in Anaconda's spokes
    icon = "emblem-system-symbolic"

    # title of the spoke (will be displayed on the hub)
    title = N_("_CLUSTER CONFIG")

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
        self.log.info("Initialize Cluster Config")

        self.infoStore = self.builder.get_object("ClusterInfoStore")
        self.populate()

    def refresh(self):
        """
        The refresh method that is called every time the spoke is displayed.
        It should update the UI elements according to the contents of
        self.data.

        :see: pyanaconda.ui.common.UIObject.refresh

        """
        pass

    def apply(self):
        """
        The apply method that is called when the spoke is left. It should
        update the contents of self.data with values set in the GUI elements.

        """

        # need to create a copy of selectStore entries, otherwise deepcopy
        # used in other anaconda widgets won't work
        infoParams = []
        for r in self.infoStore:
            infoParams.append((r[:]))
        self.data.addons.org_rocks_rolls.info = infoParams 
        self.log.info("ROCKS: info %s" % self.data.addons.org_rocks_rolls.info.__str__())

    def execute(self):
        """
        The excecute method that is called when the spoke is left. It is
        supposed to do all changes to the runtime environment according to
        the values set in the GUI elements.

        """
        pass 
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

        if self.infoStore is None:
            return False
        required = filter(lambda x: x[4] ,self.infoStore)
        completed = filter(lambda x: x[1] is not None and len(x[1]) > 0, required) 
        if len(required) == len(completed):
            return True
        else:
            return False

    @property
    def mandatory(self):
        """
        The mandatory property that tells whether the spoke is mandatory to be
        completed to continue in the installation process.

        :rtype: bool

        """
        # this is a mandatory spoke that is not mandatory to be completed
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
        if (self.completed):
            return "Required Config Entered"
        else:
            return "Configure Your Cluster" 

    ### handlers ###
    def InfoUpdated(self,cr,path,text):
        self.infoStore[path][1] = text

    def on_entry_icon_clicked(self, entry, *args):
        """Handler for the textEntry's "icon-release" signal."""
        pass
    def on_main_button_clicked(self, *args):
        """Handler for the mainButton's "clicked" signal."""
        pass

    ### Other methods
    def populate(self):
        """Populate the infoStore with this initial settings. Note, this
           should be generalized"""
        p = os.path.join(os.path.dirname(inspect.getfile(self.__class__)),INFOFILE)
        f = open(p)
        allInfo = json.load(f)
        initialParams = [[z[idx] for idx in FIELDNAMES ] for z in allInfo] 
        for r in initialParams:
            self.addInfo(r)
        self.mapAnacondaValues()

    def mapAnacondaValues(self):
        self.setValue("Kickstart_PublicHostname",network.getHostname())
   
    def addInfo(self,record):
       self.infoStore.append(record)

    def setValue(self,varname,value):
        for row in self.infoStore:
            if row[VARIDX] == varname:
                row[VALIDX] = value

if __name__ == "__main__":
    from gi.repository import Gtk
    rr = RocksInfoSpoke(None,None,None,None)
    rr.initialize()
    rr.refresh()
    Gtk.main()

# vim:sw=4:ts=4:et
