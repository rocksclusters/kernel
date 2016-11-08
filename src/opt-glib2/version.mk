NAME		= opt-glib2
PKGROOT		= /opt/devel
VERSION		= 2.44.1
RELEASE		= 0
SUBDIR		= glib-$(VERSION)
TARFILE		= $(SUBDIR).tar.xz
RPM.REQUIRES	= opt-devel-module
RPM.FILES	= "$(PKGROOT)/bin/*\\n$(PKGROOT)/include/*\\n$(PKGROOT)/lib/*\\n$(PKGROOT)/share/*"
