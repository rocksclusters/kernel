#
# rhel.py
#
# Copyright (C) 2010  Red Hat, Inc.  All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

from pyanaconda.installclass import BaseInstallClass
from pyanaconda.product import productName
from pyanaconda import network
from pyanaconda import nm
from pyanaconda.kickstart import getAvailableDiskSpace
from blivet.partspec import PartSpec
from blivet.platform import platform
from blivet.devicelibs import swap
from blivet.size import Size
import logging

class RHELBaseInstallClass(BaseInstallClass):
    name = "Rocks CentOS-based Linux"
    sortPriority = 20003
    if not productName.startswith("Rocks"):
        hidden = True
    defaultFS = "ext4"

    bootloaderTimeoutDefault = 5

    ignoredPackages = ["ntfsprogs", "reiserfs-utils", "hfsplus-tools", "dracut-config-rescue"]

    installUpdates = False

    _l10n_domain = "comps"

    efi_dir = "centos"

    help_placeholder = "CentOSPlaceholder.html"
    help_placeholder_with_links = "CentOSPlaceholder.html"

    def configure(self, anaconda):
        BaseInstallClass.configure(self, anaconda)
        self.setDefaultPartitioning(anaconda.storage)

    def setNetworkOnbootDefault(self, ksdata):
        if network.has_some_wired_autoconnect_device():
            return
        # choose the device used during installation
        # (ie for majority of cases the one having the default route)
        dev = network.default_route_device() \
              or network.default_route_device(family="inet6")
        if not dev:
            return
        # ignore wireless (its ifcfgs would need to be handled differently)
        if nm.nm_device_type_is_wifi(dev):
            return
        network.update_onboot_value(dev, "yes", ksdata)

    def getBackend(self):
        """Use the Rocks-defined version of yumpayload"""
        return RocksYumPayload
        
    def __init__(self):
        BaseInstallClass.__init__(self)


from pyanaconda.packaging.yumpayload import YumPayload
from rocks_getrolls import RocksGetRolls
class RocksYumPayload(YumPayload):
    """ A YumPayload installs packages onto the target system using yum.

        User-defined (aka: addon) repos exist both in ksdata and in yum. They
        are the only repos in ksdata.repo. The repos we find in the yum config
        only exist in yum. Lastly, the base repo exists in yum and in
        ksdata.method.

       This is where we selectively overwrite some of yumpayload.py in present
       in the packaging/ directory
    """


    def __init__(self, data):
        super(RocksYumPayload,self).__init__(data)

    def preInstall(self, packages=None, groups=None):
        """ Perform pre-installation tasks. """ 
        log = logging.getLogger("packaging")
        log.info("RocksYumPayload preInstallHook  - downloading rolls")
        RocksGetRolls()
        log.info("RocksYumPayload preInstallHook  - rolls downloaded")
        super(RocksYumPayload, self).preInstall(packages, groups)
