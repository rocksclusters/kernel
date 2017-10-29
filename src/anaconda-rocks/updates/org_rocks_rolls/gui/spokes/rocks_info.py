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
import logging
gi.require_version('Gtk','3.0')
from gi.repository import Gtk, GObject

### the path to addons is in sys.path so we can import things from org_rocks_rolls
from org_rocks_rolls.categories.RocksRolls import RocksRollsCategory
from org_rocks_rolls import RocksEnv
from pyanaconda.ui.gui import GUIObject
from pyanaconda.ui.gui.spokes import NormalSpoke
from pyanaconda.ui.communication import hubQ
from pyanaconda import network
from pyanaconda import nm
import thread
import json
import inspect

# export only the spoke, no helper functions, classes or constants
__all__ = ["RocksConfigSpoke"]

# File that holds JSON of clusterInfo variables.
INFOFILE = "infoVars.json"
PROFILES = "/tmp/rocks/export/profile"
FIELDNAMES = ['param','value','varname','infoHelp','required','display','validate', 'color','derived','colorlabel','initialize']
VARIDX = FIELDNAMES.index("varname")
VALIDX = FIELDNAMES.index("value")
VISIDX = FIELDNAMES.index("display")
COLORIDX = FIELDNAMES.index("color")
REQUIREDIDX = FIELDNAMES.index("required")
DERIVEDIDX = FIELDNAMES.index("derived")
INITIALIZEIDX = FIELDNAMES.index("initialize")

# Field names are:
#        param: String. Displayed Parameter Text 
#        value: String. initial value of this parameter 
#        varname: String.  Name of parameter as it appears in a kickstart file 
#        infoHelp: String. Mouse over help string in interface 
#        required: Boolean.  Required that that this parameter has a value 
#        display: Boolean. Display on the UI 
#        validate: String.  Validation Method 
#        color: String.  Color of cell for disply 
#        colorlabel: String.  Color of label 
#        initialize: String.  name of existing attr to initialize this param
# These fields are mapped into gui's ClusterInfoStore (in RocksInfo.glade).
# ClusterInfoStore only has indices, so must keep the map of these dictionary 
# values consistent with their indices in the UI


def addRecord(ksdata, varname, value, \
        param='',infoHelp='',required=False,display=False,validate=None,\
        colorlabel="green"):
    tuple = FIELDNAMES
    tuple[FIELDNAMES.index("param")]=param
    tuple[FIELDNAMES.index("varname")]=varname
    tuple[FIELDNAMES.index("value")]=value
    tuple[FIELDNAMES.index("infoHelp")]=infoHelp
    tuple[FIELDNAMES.index("required")]=required
    tuple[FIELDNAMES.index("display")]=display
    tuple[FIELDNAMES.index("validate")]=validate
    tuple[FIELDNAMES.index("colorlabel")]=colorlabel

    try:
        vars = map(lambda x: x[VARIDX],ksdata.addons.org_rocks_rolls.info)
    except:
        ## info didn't exist
        vars=[]
        ksdata.addons.org_rocks_rolls.info = []
    
    if varname in vars:
        idx = vars.index(varname)
        ksdata.addons.org_rocks_rolls.info[idx] = tuple 
    else:
        ksdata.addons.org_rocks_rolls.info.append(tuple) 
        
## get the value in the info store
def getValue(ksdata,varname):
    ## don't fail if info hasn't been initialized
    try:
        info = ksdata.addons.org_rocks_rolls.info
    except:
        info=[]

    for row in info:
        if row[VARIDX] == varname:
            return row[VALIDX]
    ## Not found
    return None 

## Set the value in info store
def setValue(info, varname,value):
    log = logging.getLogger('anaconda')
    log.info("Trying to set '%s' to '%s' in object '%s'" % \
        (varname,value.__str__(),hex(id(info))))
    for row in info:
        if row[VARIDX] == varname:
            log.info("Setting '%s' to '%s'" % (varname,value.__str__())) 
            row[VALIDX] = value.__str__()
            break 

class RocksConfigSpoke(NormalSpoke):
    """
    Class for the RocksConfig spoke. This spoke will be in the RocksRollsCategory 
    category and thus on the Summary hub. It is a very simple example of a unit
    for the Anaconda's graphical user interface. 


    :see: pyanaconda.ui.common.UIObject
    :see: pyanaconda.ui.common.Spoke
    :see: pyanaconda.ui.gui.GUIObject
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
        self.clientInstall = RocksEnv.RocksEnv().clientInstall
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
        self.infoFilter = self.builder.get_object("ClusterInfoFilter")
        self.infoFilter.set_visible_column(FIELDNAMES.index("display"))
        
        jsoninfo = self.populate()
        self.mapAnacondaValues(jsoninfo)
        # merge entries into self.data.addons.org_rocks_rolls.info 
        self.merge(jsoninfo)
        self.visited = False
        self.readyState = True
        

    def refresh(self):
        """
        The refresh method that is called every time the spoke is displayed.
        It should update the UI elements according to the contents of
        self.data.

        :see: pyanaconda.ui.common.UIObject.refresh

        """
        ### Master of information is rocks_rolls.info structure.
        self.readRollJSON()
        self.mapAnacondaValues(self.data.addons.org_rocks_rolls.info)
        self.log.info("ROCKS: refresh() info %s" % hex(id(self.data.addons.org_rocks_rolls.info)))
        self.log.info("ROCKS: refresh() info %s" % self.data.addons.org_rocks_rolls.info.__str__())
        self.infoStore.clear()
        for infoEntry in self.data.addons.org_rocks_rolls.info:
            if type(infoEntry[1]) is list:
                infoEntry[1] = ",".join(infoEntry[1])
            if type(infoEntry[1]) is not str:
                infoEntry[1] = infoEntry[1].__str__()
            self.infoStore.append(infoEntry)
        self.setColors()

    def apply(self):
        """
        The apply method that is called when the spoke is left. It should
        update the contents of self.data with values set in the GUI elements.

        """

        # need to create a copy of selectStore entries, otherwise deepcopy
        # used in other anaconda widgets won't work
        infoParams = []
        for r in self.infoStore:
            setValue(self.data.addons.org_rocks_rolls.info,r[2],r[1])
        # self.data.addons.org_rocks_rolls.info = infoParams 
        self.log.info("ROCKS: info %s" % hex(id(self.data.addons.org_rocks_rolls.info)))
        self.log.info("ROCKS: info %s" % self.data.addons.org_rocks_rolls.info.__str__())
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
        ## Let's see if we have a network default route set 
        try:
            device = network.default_route_device()
            if device is None:
                self.readyState=False
                return False 
        except:
            self.readyState=False
            return False

        # When we change from not ready to ready, send a HubQ message
        if self.readyState is False:
            self.readyState = True
            hubQ.send_ready(self.__class__.__name__, True)

        self.log.info("rocks_info.py:ready")
        return True


    @property
    def completed(self):
        """
        The completed property that tells whether all mandatory items on the
        spoke are set, or not. The spoke will be marked on the hub as completed
        or uncompleted acording to the returned value.

        :rtype: bool

        """
        rval = self._completed
        self.log.info("rocks_info.py:completed:%s" % rval)
        return rval

    @property
    def _completed(self):
        """
        The completed property that tells whether all mandatory items on the
        spoke are set, or not. The spoke will be marked on the hub as completed
        or uncompleted acording to the returned value.

        :rtype: bool

        """

        if self.clientInstall:
            return True
        if self.infoStore is None:
            return False
        self.readRollJSON()
        required = filter(lambda x: x[4] ,self.data.addons.org_rocks_rolls.info)
        completed = filter(lambda x: x[1] is not None and len(x[1]) > 0, required) 
        self.log.info("ROCKS: completed() required:%d; completed:%d" % (len(required),len(completed)))
        if self.visited and len(required) == len(completed):
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
        if self.clientInstall:
            return "Configuration Defined By Kickstart"
        if not self.ready:
            return "Please Configure your Public Network"
        if  self._completed:
            return "All Configuration Entered"
        else:
            return "Configure Your Cluster" 

    ### handlers ###
    def InfoUpdated(self,cr,path,text):
        # path is the index of "visible" variables, there may be others
        # not shown. need to adjust path with offset
        target = int(path)
        offset = 0
        for i in range(0,len(self.infoStore)):
            if not self.infoStore[i][VISIDX]: offset += 1
            if target == 0: break
            if self.infoStore[i][VISIDX]: target -= 1
        idx = int(path) + offset
        # set the value only if not a derived value
        if not self.infoStore[idx][DERIVEDIDX]:
            self.infoStore[idx][VALIDX] = text
        self.setColors()

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
        return initialParams

    def readRollJSON(self):
        """Read roll JSON files to add attributes.  This needs to be called
           whenever completed methods or enter methods are invoked"""
    
        ## Find all files in PROFILES path that are json
        allfiles = []
        candidates = filter(lambda x: x[0].endswith('include/json'), \
            os.walk(PROFILES))
        self.log.info("ROCKS: readRollJSON candidates (%s)" % str(candidates))              
        for line in candidates:
            files = map(lambda x: os.path.join(line[0],x), line[2])  
            if len(files) > 0:
                allfiles.extend(files)
        ## allfiles has the full path of all roll json files (if any)
        ## if any are misformatted, not json, etc. just keep going 
        self.log.info("ROCKS: readRollJSON allfiles (%s)" % str(allfiles))              
        for p in allfiles:
            try:
                f = open(p)
                rollInfo = json.load(f)
                f.close()
                rollParams = [[z[idx] for idx in FIELDNAMES ] for z in rollInfo] 
                # Try to initialize params using other attributes.
                # If no initialize is defined for a particular attr, just
                # keep trying.
                for param in rollParams:
                    try:
                        param[VALIDX] = getValue(self.data,param[INITIALIZEIDX])
                    except Exception as e:
                        self.log.info("ROCKS: readRollJSON param set exception (%s)" % str(e))              
                self.log.info("ROCKS: readRollJSON rollParams (%s)" % str(rollParams))              

                self.merge(rollParams)
            except Exception as e:
                self.log.info("ROCKS: readRollJSON exception (%s)" % str(e))              
    
    def merge(self, defaultinfo):
        """ merge data from defaultinfo into org_rocks_rolls.info 
            data struct, if a varname is already in rolls.info, don't
            change rolls.info """

        try:
            ivars = \
                map(lambda x: x[VARIDX],self.data.addons.org_rocks_rolls.info)
        except:
            ivars = []
            self.data.addons.org_rocks_rolls.info=[]
        for row in defaultinfo:
            if row[VARIDX] not in ivars:
                self.data.addons.org_rocks_rolls.info.append(row)

    def mapAnacondaValues(self,info):
        """ Should be called only after json file has loaded 
            These are variables read directly from anaconda objects or 
            derived from them.  
            XXX: This really should be a file of mappings"""
        ## This method basically has to reverse engineer what 
        ## other parts of anaconda is doing       
        ksdata = self.data
        mapping = {} 
        mapping["Kickstart_Lang"]="ksdata.lang.lang"
        mapping["Kickstart_Langsupport"]=mapping["Kickstart_Lang"]
        mapping["Kickstart_Timezone"] = "ksdata.timezone.timezone"
        # Need to find a better way to get NTP info.  
        # mapping["Kickstart_PublicNTPHost"] = "ksdata.timezone.ntpservers"
        mapping["Kickstart_PublicDNSServers"] = "self.readDNSConfig()" 
        ## Network may not be up, so these may fail
        try:
            mapping["Kickstart_PublicInterface"] = "network.default_route_device()"
            mapping["Kickstart_PublicAddress"] = "network.get_default_device_ip()"
            mapping["Kickstart_PublicFQDN"]="subprocess.check_output(['hostname']).strip()"
            mapping["Kickstart_PublicHostname"]="subprocess.check_output(['hostname']).strip().split('.',1)[0]"
            mapping["Kickstart_PublicDNSDomain"]="subprocess.check_output(['hostname']).strip().split('.',1)[1]"
            ## get the public networking values
            pubif = eval(mapping["Kickstart_PublicInterface"])
            pubaddr = eval(mapping["Kickstart_PublicAddress"])
            cidr = nm.nm_device_ip_config(pubif)[0][0][1]
            gateway = nm.nm_device_ip_config(pubif)[0][0][2]
            netmask = network.prefix2netmask(cidr) 
            nparts = map(lambda x: int(x),netmask.split('.'))
            aparts = map(lambda x: int(x),pubaddr.split('.'))
            netaddr = map(lambda x: nparts[x] & aparts[x], range(0,len(nparts)))
            pubnetwork=".".join(map(lambda x: x.__str__(),netaddr))
            mapping["Kickstart_PublicNetwork"] = "pubnetwork"
            mapping["Kickstart_PublicNetmask"] = "netmask"
            mapping["Kickstart_PublicNetmaskCIDR"] = "cidr"
            mapping["Kickstart_PublicGateway"] = "gateway"
            mtu = nm.nm_device_property(pubif,'Mtu')
            mapping["Kickstart_PublicMTU"] = mtu.__str__()
        except:
            pass
        
        
        ## set the values in our own info structure
        for var in mapping.keys():
            try: 
                setValue(info, var,eval(mapping[var]))
            except Exception as e:
                self.log.info("ROCKS: Exception(%s) setting var (%s)" % (e,var))              
    ## Set colors of value record for better visual feedback
    ## when validation methods are defined, this is where they should be
    ## be invoked
    def setColors(self):
        for row in self.infoStore:
            if len(row[VALIDX]) == 0 and row[REQUIREDIDX]:
                row[COLORIDX] = "red"
            else:
                row[COLORIDX] = "white"
            if row[DERIVEDIDX]:
                row[COLORIDX] = "light gray"

    def readDNSConfig(self):
        """ Read resolv.conf and return a comma-delimited list of 
            of servers. Empty string if resolv.conf isn't written yet"""
        servers = []
        try:
            with open("/etc/resolv.conf") as f:
                for line in f.readlines():
                    if "nameserver" in line:
                            servers.append(line.split()[1])
        except:
            pass

        return  ",".join(servers)


class Foo():
    def __init__(self):
        pass

if __name__ == "__main__":
    from gi.repository import Gtk
    data = Foo()
    data.addons = Foo()
    data.addons.org_rocks_rolls = Foo()
    rr = RocksConfigSpoke(data,None,None,None)
    rr.initialize()
    rr.refresh()
    Gtk.main()

# vim:sw=4:ts=4:et
