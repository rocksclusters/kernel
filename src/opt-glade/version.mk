NAME		= opt-glade
PKGROOT		= /opt/devel
VERSION		= 3.19.0
RELEASE		= 0
SUBDIR		= glade-$(VERSION)
TARFILE		= $(SUBDIR).tar.xz
RPM.REQUIRES	= opt-gtk+,opt-devel-module
RPM.FILES	= "$(PKGROOT)/bin/*\\n$(PKGROOT)/include/*\\n$(PKGROOT)/lib/*\\n$(PKGROOT)/share/*"
