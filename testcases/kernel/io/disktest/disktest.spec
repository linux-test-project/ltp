Summary: Test tool for driving IO to block, raw, filesystem targets
Name: disktest
Version: v1.4.1
Vendor: IBM Corp.
Release: 1
Copyright: GPL
Group: Applications/System
BuildRoot: /tmp/%{name}-buildroot
Source: disktest-%{version}.tar.gz
Requires: man rpm

%description
This package provides the disk testing utility for performing IO testing to
block, raw, and filesystem targets.

Authors:
--------
    Brent Yardley <yardleyb@us.ibm.com>


%prep
RPM_SOURCE_DIR="~/work/SOURCES"
%setup
make all-clean

%build
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT;
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/man/man1

install -m 775 disktest $RPM_BUILD_ROOT/usr/bin
install -m 775 man1/disktest.1.gz $RPM_BUILD_ROOT/usr/man/man1

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README LICENSE CHANGELOG
/usr/bin/disktest
/usr/man/man1/disktest.1.gz

%changelog
* Tue Oct 19 2006 Brent Yardley <yardleyb@us.ibm.com>
- Added signal handlers
* Tue Jun 13 2006 Brent Yardley <yardleyb@us.ibm.com>
- First rpm package build
