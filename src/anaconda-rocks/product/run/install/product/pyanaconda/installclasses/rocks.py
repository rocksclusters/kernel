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

class RHELBaseInstallClass(BaseInstallClass):
    name = "Rocks"
    sortPriority = 20002
    ## productName MUST be the same as how Lorax builds the
    ## boot media. See rocks-boot-7 Makefile
    if not productName.startswith("Rocks"):
        hidden = True
    defaultFS = "ext4"

    bootloaderTimeoutDefault = 5

    ignoredPackages = ["ntfsprogs", "reiserfs-utils", "hfsplus-tools"]

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


    def setSteps(self, anaconda):
        BaseInstallClass.setSteps(self, anaconda)

        if os.path.exists('/tmp/rocks-skip-welcome'):
            anaconda.dispatch.skipStep("welcome", skip = 1)
        else:
            anaconda.dispatch.skipStep("welcome", skip = 0)

        #
        # skip the following 'graphical' screens
        #
        anaconda.dispatch.skipStep("parttype", skip = 1)
        anaconda.dispatch.skipStep("filtertype", skip = 1)
        anaconda.dispatch.skipStep("cleardiskssel", skip = 1)
        anaconda.dispatch.skipStep("bootloader", permanent = 1)
        anaconda.dispatch.skipStep("timezone", permanent = 1)
        anaconda.dispatch.skipStep("accounts", permanent = 1)
        anaconda.dispatch.skipStep("tasksel", permanent = 1)

        if os.path.exists('/tmp/manual-partitioning'):
            anaconda.dispatch.skipStep("partition", skip = 0)
            anaconda.dispatch.skipStep("autopartitionexecute",
                skip = 1)
        else:
            anaconda.dispatch.skipStep("partition", skip = 1)
            anaconda.dispatch.skipStep("autopartitionexecute",
                skip = 0)

        anaconda.dispatch.skipStep("group-selection", permanent = 1)
        anaconda.dispatch.skipStep("complete", permanent = 1)

        # from gui import stepToClass
        from dispatch import installSteps
        from packages import turnOnFilesystems
        from rocks_getrolls import RocksGetRolls

        #
        # need to move the making of the file systems up before
        # we download the rolls.
        #
        # rocks doesn't use the 'timezone' or 'accounts' screens,
        # so we'll override them with our functions
        #
        #index = 0
        #for key in installSteps:
             #if key[0] == "timezone":
             #break
                     #index = index + 1

        #installSteps[index] = ("rocksenablefilesystems",
        #turnOnFilesystems,)
        #anaconda.dispatch.skipStep("rocksenablefilesystems", skip = 0)
        #anaconda.dispatch.skipStep("enablefilesystems", skip = 1)

                index = 0
                for key in installSteps:
                        if key[0] == "upgbootloader":
                                break
                        index = index + 1

                installSteps[index] = ("downloadrolls", RocksGetRolls,)
        anaconda.dispatch.skipStep("downloadrolls", skip = 0)


    def getBackend(self):
        return yuminstall.YumBackend


    def __init__(self):
        BaseInstallClass.__init__(self)

class RHELAtomicInstallClass(RHELBaseInstallClass):
    name = "CentOS Atomic Host"
    sortPriority=21001
    hidden = not productName.startswith(("CentOS Atomic Host", "CentOS Linux Atomic"))

    def setDefaultPartitioning(self, storage):
        autorequests = [PartSpec(mountpoint="/", fstype=storage.defaultFSType,
                                size=Size("1GiB"), maxSize=Size("3GiB"), grow=True, lv=True)]

        bootreqs = platform.setDefaultPartitioning()
        if bootreqs:
            autorequests.extend(bootreqs)

        disk_space = getAvailableDiskSpace(storage)
        swp = swap.swapSuggestion(disk_space=disk_space)
        autorequests.append(PartSpec(fstype="swap", size=swp, grow=False,
                                    lv=True, encrypted=True))

        for autoreq in autorequests:
            if autoreq.fstype is None:
                if autoreq.mountpoint == "/boot":
                    autoreq.fstype = storage.defaultBootFSType
                    autoreq.size = Size("300MiB")
                else:
                    autoreq.fstype = storage.defaultFSType

        storage.autoPartitionRequests = autorequests
