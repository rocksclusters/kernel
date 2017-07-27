NAME		= anaconda-rocks-dracut
RELEASE		= 0
DRACUT_DEST 	= /lib/dracut/modules.d
MODDIR		= 70rocks
SYSTEMD_FOLDER = /lib/systemd/system
SYSTEMD_INITSCRIPT = lighttpd.service
SYSTEMD_PARTSCRIPT = rockspart.service
KSFETCH = fetchRocksKS.py

SYSTEMD_SCRIPTS=$(SYSTEMD_INITSCRIPT) $(SYSTEMD_PARTSCRIPT)
SYSTEMD_DEST=$(SYSTEMD_FOLDER)

RPM.FILES = "$(SYSTEMD_DEST)/*\\n$(DRACUT_DEST)/*\\n/$(KSFETCH)"
RPM.EXTRAS = "%define __python /bin/true"
