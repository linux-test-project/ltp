%define prefix /usr

Name: openhpi
Summary: openhpi - implementation of SAForum Hardware Platform Interface 
Version: 0.5.0
Release: 1
Copyright: BSD
URL: http://openhpi.sourceforge.net
Group: Utilities
Vendor: OpenHPI Project
Packager: TariqX
Source: openhpi-0.5.0.tar.gz
Buildroot: /var/tmp/openhpi-root
BuildRequires: pkgconfig glib-devel
BuildRequires: docbook-utils docbook-utils-pdf docbook-dtd41-sgml
Summary: Library that provides SAForum's HPI
Group: Utilities
AutoReq: no
Requires: libc.so.6 libglib-1.2.so.0 libpthread.so.0

%package devel
Summary: Development files for HPI
Group: Utilities
AutoReq: yes
Requires: openhpi 
Requires: pkgconfig

%package clients
Summary: HPI command line applications
Group: Utilities
AutoReq: yes

%description 
This package contains an implementation of Service Availability Forum's 
HPI specification.  It includes support for multiple different types of
hardware including: IPMI, IBM Blade Center (via SNMP), Linux Watchdog devices,
and Sysfs based systems.

%description devel
Contains additional files need for a developer to create applications
and/or middleware that depends uses the Service Availability Forum's
HPI specification

%description clients
This package contains hpi command line utilities 

###################################################
%prep
###################################################

###################################################
%setup
###################################################

###################################################
%build
###################################################
./configure '--enable-ipmi' '--enable-sysfs' '--enable-snmp_bc' '--enable-snmp_client' '--enable-ipmidirect' --sysconfdir=/etc \
    --localstatedir=/var --prefix=%{prefix}
make
make -C docs/hld pdf-am
make -C docs/hld openhpi-manual/book1.html

###################################################
%install
###################################################
if
  [ ! -z "${RPM_BUILD_ROOT}"  -a "${RPM_BUILD_ROOT}" != "/" ]
then
  rm -rf $RPM_BUILD_ROOT
fi
make DESTDIR=$RPM_BUILD_ROOT install
mkdir -p $RPM_BUILD_ROOT/etc/openhpi
mkdir -p $RPM_BUILD_ROOT/var/lib/openhpi
install -C -m 644 examples/openhpi.conf.example $RPM_BUILD_ROOT/etc/openhpi/openhpi.conf
%post

###################################################
%files
###################################################
%defattr(-,root,root)
%doc README COPYING ChangeLog
%doc docs/hld/*pdf
%doc docs/hld/openhpi-manual
%config /etc/openhpi/*
%dir /var/lib/openhpi
%{prefix}/lib/libopen*
%{prefix}/lib/openhpi/lib*
# dummy*
#%{prefix}/lib/openhpi/libsysfs*
#%{prefix}/lib/openhpi/libsnmp_bc*
#%{prefix}/lib/openhpi/libwatchdog*
#%{prefix}/lib/openhpi/libipmidirect*
#%{prefix}/lib/openhpi/libipmi*

###################################################
%files devel
###################################################
%defattr(-,root,root)
%dir %{prefix}/include/openhpi
%{prefix}/include/openhpi/*h
%{prefix}/lib/pkgconfig/openhpi.pc

###################################################
%files clients
###################################################
%defattr(-,root,root)
%{prefix}/bin/hpi*

###################################################
%clean
###################################################
if
    [ -z "${RPM_BUILD_ROOT}"  -a "${RPM_BUILD_ROOT}" != "/" ]
then
    rm -rf $RPM_BUILD_ROOT
fi
rm -rf $RPM_BUILD_DIR/openhpi-0.5.0

