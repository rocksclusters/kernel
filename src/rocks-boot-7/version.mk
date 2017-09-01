NAME		= rocks-boot
PKGROOT		= /boot/kickstart/default
RELEASE		= 0
RPM.PREFIX	= $(PKGROOT)
INSTALLERISO	= rocks-installer-$(VERSION.MAJOR)-$(VERSION.MINOR).iso

# ISFINAL = --isfinal
ifdef ISFINAL
CENTRAL=central-$(VERSION)-x86-64.rocksclusters.org
else
CENTRAL=beta7.rocksclusters.org
endif
RPM.DESCRIPTION = rocks-boot installer media with central=$(CENTRAL)
