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
from org_rocks_rolls.gui.spokes import rocks_info 
from pyanaconda.ui.gui import GUIObject
from pyanaconda.ui.gui.spokes import NormalSpoke
from pyanaconda.ui.common import FirstbootSpokeMixIn
from pyanaconda import network
from pyanaconda import nm
import thread
import json
import inspect

# export only the spoke, no helper functions, classes or constants
__all__ = ["RocksPrivateIfaceSpoke"]

# map of local variable to  addon.rocks.info parameter space
# That info space is where we keep information, GUIs can only
# update/change
#
infoMap = {}
infoMap['MTU'] = 'Kickstart_PrivateMTU'
infoMap['privateHostname']= 'Kickstart_PrivateHostname'
infoMap['privateDNS']= 'Kickstart_PrivateDNSDomain'
infoMap['privateIP']= 'Kickstart_PrivateAddress'
infoMap['privateNetmask']='Kickstart_PrivateNetmask'
infoMap['privateNetwork']='Kickstart_PrivateNetwork'
infoMap['ifaceSelected'] ='Kickstart_PrivateInterface'

FIELDNAMES=["label","device","type","mac"]
LABELIDX = FIELDNAMES.index("label")
DEVICEIDX = FIELDNAMES.index("device")
TYPEIDX = FIELDNAMES.index("type")
MACIDX = FIELDNAMES.index("mac")

class RocksPrivateIfaceSpoke(FirstbootSpokeMixIn, NormalSpoke):
    """
    Class for the RocksConfig spoke. This spoke will be in the RocksRollsCategory 
    category and thus on the Summary hub. 
    """

    ### class attributes defined by API ###

    # the name of the main window widget
    mainWidgetName = "RocksPrivateIfaceWindow"

    # name of the .glade file in the same directory as this source
    uiFile = "RocksPrivateIface.glade"

    # category this spoke belongs to
    category = RocksRollsCategory

    # spoke icon (will be displayed on the hub)
    # preferred are the -symbolic icons as these are used in Anaconda's spokes
    icon = "emblem-system-symbolic"

    # title of the spoke (will be displayed on the hub)
    title = N_("_CLUSTER PRIVATE NETWORK")

    ### methods defined by API ###
    def __init__(self, data, storage, payload, instclass):
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
        self.log.info("Initialize Private Interface Config")

        self.MTUStore = self.builder.get_object("MTUstore")
        self.MTUComboBox = self.builder.get_object("MTUBox")
        self.deviceStore = self.builder.get_object("deviceStore")
        self.ifaceCombo = self.builder.get_object("ifaceCombo")
        self.privateDNS_Entry = self.builder.get_object("privateDNS")
        self.IPv4_Address = self.builder.get_object("IPv4_Address")
        self.IPv4_Netmask = self.builder.get_object("IPv4_Netmask")
        # Populate the private interface combo
        self.ifaceSelected=''
        self.refresh()
        self.ifaceCombo.set_active(0)
        self.ifaceSelected = self.ifaceCombo.get_active_id().split(';')[0]
        # intialize DNS,IPV4 addr/netmask
        self.MTU = self.MTUComboBox.get_active_id().split()[0]
        self.privateHostname = network.getHostname().split('.',1)[0]
        self.privateIP = self.IPv4_Address.get_text()
        self.privateNetmask = self.IPv4_Netmask.get_text()
        self.privateDNS = self.privateDNS_Entry.get_text()
        self.visited = False

    @property
    def privateNetwork(self):
        nparts = map(lambda x: int(x),self.privateNetmask.split('.'))
        aparts = map(lambda x: int(x),self.privateIP.split('.'))
        netaddr = map(lambda x: nparts[x] & aparts[x], range(0,len(nparts)))
        return ".".join(map(lambda x: x.__str__(),netaddr))

    def refresh(self):
        """
        The refresh method that is called every time the spoke is displayed.
        It should update the UI elements according to the contents of
        self.data.

        :see: pyanaconda.ui.common.UIObject.refresh

        """
        ## Every time we enter, make a list of all the devices that
        # are not the public interface (user might have changed this)
        pubif = network.default_route_device()
        allifs = filter(lambda x: nm.nm_device_type_is_ethernet(x),\
                nm.nm_devices())
        privates = filter(lambda x: x != pubif,allifs)
        idx = self.ifaceCombo.get_active()
        self.deviceStore.clear()
        for x in privates:
            entry=[None,None,None,None]
            entry[DEVICEIDX] = x
            entry[TYPEIDX] = "ethernet"
            entry[MACIDX] = nm.nm_device_perm_hwaddress(x)
            entry[LABELIDX] = "%s;%s" % (x,entry[MACIDX])
            self.deviceStore.append(entry)
        if len(privates) == 0:
            entry=[None,None,None,None]
            entry[DEVICEIDX] = "%s:0" % pubif
            entry[LABELIDX] = "%s;virtual interface" % entry[DEVICEIDX] 
            entry[TYPEIDX] = "virtual"
            entry[MACIDX] = ""
            self.deviceStore.append(entry)

        # Set the active entry, even if we reodered 
        self.ifaceCombo.set_active(idx)

    def apply(self):
        """
        The apply method that is called when the spoke is left. It should
        update the contents of self.data with values set in the GUI elements.

        """

        # This is all about setting variables in the 
        # self.data.addons.org_rocks_rolls.info 
        for var in infoMap.keys(): 
            rocks_info.setValue(self.data.addons.org_rocks_rolls.info, \
                infoMap[var], eval("self.%s"%var))
        self.visited = True

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
        return True
        #if self.data.addons.org_rocks_rolls.haverolls is None:
        #    return False
        # return self.data.addons.org_rocks_rolls.haverolls 

    @property
    def completed(self):
        """
        The completed property that tells whether all mandatory items on the
        spoke are set, or not. The spoke will be marked on the hub as completed
        or uncompleted acording to the returned value.

        :rtype: bool

        """
        # return True
        return self.visited 

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
        if  not self.ready:
            return "Please Select Rolls First"
        if self.completed:
            return "All Required Configuration Entered"
        else:
            return "Configure Private Network" 

    ### handlers ###
    def ifaceCombo_handler(self,widget):
        id = widget.get_active_id();
        if id is not None:
            self.ifaceSelected = id.split(';')[0]

    def selectMTU_handler(self,widget):
        self.MTU = widget.get_active_id().split()[0]

    def IPv4_Address_handler(self,widget):
        self.privateAddress = widget.get_text() 
        widget.set_text(self.privateAddress)

    def IPv4_Netmask_handler(self,widget):
        self.privateNetmask = widget.get_text()
        widget.set_text(self.privateNetmask)

    def privateDNS_handler(self,widget):
        self.privateDNS = widget.get_text()
        widget.set_text(self.privateDNS)

    def on_entry_icon_clicked(self, entry, *args):
        """Handler for the textEntry's "icon-release" signal."""
        pass
    def on_main_button_clicked(self, *args):
        """Handler for the mainButton's "clicked" signal."""
        pass

    ### Other methods
    def addInfo(self,record):
       self.infoStore.append(record)

    def setValue(self,varname,value):
        for row in self.infoStore:
            if row[VARIDX] == varname:
                row[VALIDX] = value

if __name__ == "__main__":
    from gi.repository import Gtk
    rr = RocksPrivateIfaceSpoke(None,None,None,None)
    rr.initialize()
    rr.refresh()
    Gtk.main()

# vim:sw=4:ts=4:et
