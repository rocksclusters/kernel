NAME		= foundation-sqlite
PKGROOT		= /opt/sqlite
VERSION		= 3.7.10
TARVERSION	= 3071000
RELEASE		= 0
SUBDIR		= sqlite-autoconf-$(TARVERSION)
TARFILE		= $(SUBDIR).tar.gz
RPM.FILES	= /opt/sqlite
RPM.EXTRAS="%define _use_internal_dependency_generator 0\\n%define __find_provides %{_builddir}/%{name}-%{version}/filter-provides.sh\\n%define __find_requires %{_rpmconfigdir}/find-requires\\n%define __os_install_post %{nil}"

