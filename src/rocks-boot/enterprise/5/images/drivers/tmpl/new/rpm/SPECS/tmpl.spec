###############################################################################
###############################################################################
##
##  Copyright (C) 2004-2006 Red Hat, Inc.  All rights reserved.
##
##  This copyrighted material is made available to anyone wishing to use,
##  modify, copy, or redistribute it subject to the terms and conditions
##  of the GNU General Public License v.2.
##
###############################################################################
###############################################################################

Source10: kmodtool
%define kmodtool bash %{SOURCE10}
%{!?kversion: %define kversion 2.6.18-8.1.10.el5}

%define kmod_name tmpl
%define kverrel %(%{kmodtool} verrel %{?kversion} 2>/dev/null)

%ifarch i686 x86_64 ia64
%define xenvar xen
%endif

%define upvar ""
%{!?kvariants: %define kvariants %{?upvar} %{?xenvar}}

Name:           %{kmod_name}-kmod
Version:        1
Release:        1.0%{?dist}
Summary:        %{kmod_name} kernel module

Group:          System Environment/Kernel
License:        GPL
URL:            http://www.intel.com/
Source0:        tmpl-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
ExclusiveArch:  i686 x86_64

%description
tmpl - Driver Update Disk template RPM SPEC file.

# magic hidden here:
%{expand:%(%{kmodtool} rpmtemplate_kmp %{kmod_name} %{kverrel} %{kvariants} 2>/dev/null)}

%prep
# to understand the magic better or to debug it, uncomment this:
#{kmodtool} rpmtemplate_kmp %{kmod_name} %{kverrel} %{kvariants} 2>/dev/null
#sleep 5
%setup -q -c -T -a 0
for kvariant in %{kvariants} ; do
    cp -a qla2xxx-8170 _kmod_build_$kvariant
done


%build
for kvariant in %{kvariants}
do
    ksrc=%{_usrsrc}/kernels/%{kverrel}${kvariant:+-$kvariant}-%{_target_cpu}
    pushd _kmod_build_$kvariant
    make -C "${ksrc}" modules M=$PWD
    popd
done


%install
export INSTALL_MOD_PATH=$RPM_BUILD_ROOT
export INSTALL_MOD_DIR=extra/%{kmod_name}
for kvariant in %{kvariants}
do
    ksrc=%{_usrsrc}/kernels/%{kverrel}${kvariant:+-$kvariant}-%{_target_cpu}
    pushd _kmod_build_$kvariant
    make -C "${ksrc}" modules_install M=$PWD
    popd
done

mkdir -p ${RPM_BUILD_ROOT}/lib/firmware

%clean
rm -rf $RPM_BUILD_ROOT

%changelog
* Wed Oct  3 2007 Jon Masters <jcm@redhat.com> - 1.0
- Initial template SPECFILE.
