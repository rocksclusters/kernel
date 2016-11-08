NAME		= opt-gtk+
PKGROOT		= /opt/devel
VERSION		= 3.16.7
RELEASE		= 0
SUBDIR		= gtk+-$(VERSION)
TARFILE		= $(SUBDIR).tar.xz
RPM.REQUIRES	= opt-atk,opt-glib2,opt-devel-module
RPM.FILES	= "$(PKGROOT)/bin/*\\n$(PKGROOT)/etc/*\\n$(PKGROOT)/include/*\\n$(PKGROOT)/lib/*\\n$(PKGROOT)/share/*"
