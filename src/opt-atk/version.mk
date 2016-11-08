NAME		= opt-atk
PKGROOT		= /opt/devel
VERSION		= 2.16.0
RELEASE		= 0
SUBDIR		= atk-$(VERSION)
TARFILE		= $(SUBDIR).tar.xz
RPM.FILES	= "$(PKGROOT)/include/*\\n$(PKGROOT)/lib/*\\n$(PKGROOT)/share/*"
