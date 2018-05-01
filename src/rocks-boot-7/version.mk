NAME		= rocks-boot
PKGROOT		= /boot/kickstart/default
RELEASE		= 1
RPM.PREFIX	= $(PKGROOT)
INSTALLERISO	= rocks-installer-$(VERSION.MAJOR)-$(VERSION.MINOR).iso

ISFINAL = --isfinal
ifdef ISFINAL
CENTRAL=central-$(shell echo $(VERSION) | tr . -)-x86-64.rocksclusters.org
else
CENTRAL=beta7.rocksclusters.org
endif
RPM.DESCRIPTION = rocks-boot installer media with central=$(CENTRAL)
