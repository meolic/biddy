# SPEC file for package libbiddy1 and libbiddy1-devel
# Usage: rpmbuild -bb biddy.spec
#
# This file has been constructed using the following examples:
#
# http://www.rpm-based.org/how-to-create-rpm-package
# http://www.rpm.org/max-rpm/
# http://kmymoney2.sourceforge.net/phb/rpm-example.html
# http://dries.ulyssis.org/apt/packages/gnunet/gnunet-spec.html
#

Summary: Multi-platform academic BDD package
Name: libbiddy1
Version: THISVERSION
Release: THISRELEASE
License: GPL
Group: Other
URL: http://biddy.meolic.com/
Source: libbiddy1-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}
Packager: meolic

%description
Biddy is a special purpose mathematical library (Binary Decision Diagrams package).

 Biddy supports ROBDDs, 0-sup-BDDs, and tagged 0-sup-BDDs.
 Biddy is oriented towards readable and comprehensible source code in C.
 Biddy is used in BDD Scout (an interactive BDD visualization tool)
and EST (a tool for formal verification of systems).

http://biddy.meolic.com/

%package devel
Requires: libbiddy1 = %{version}-%{release}
Summary: Multi-platform academic BDD package (development files)
Group: Other

%description devel
Biddy is a special purpose mathematical library (Binary Decision Diagrams package).

 Biddy supports ROBDDs with complement edges, 0-sup-BDDs with complement edges, and TZBDDs.
 Biddy is oriented towards readable and comprehensible source code in C.
 Biddy is used in EST, a tool for formal verification of systems.

http://biddy.meolic.com/

This package contains the development headers, only.

%prep
echo "Preparing %{name}-%{version}-%{release}"
%setup -q
echo "Sources OK. Let's go building!"

%build
%{__make} dynamic "BINDIR=`pwd`"
%{__make} clean "BINDIR=`pwd`"
%{__make} static "BINDIR=`pwd`"

%define _sharedir %{_prefix}/share

%install
%{__rm} -rf %{buildroot}
%{__make} -f Makefile.Linux DESTDIR=%{buildroot}/usr install "LIBDIR=lib64" "BINDIR=`pwd`"
%{__make} -f Makefile.Linux DESTDIR=%{buildroot}/usr installdev "LIBDIR=lib64" "BINDIR=`pwd`"

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root, 0755)
%doc VERSION CHANGES
%{_libdir}/libbiddy.so.%{version}
%{_libdir}/libbiddy.so.1
%{_sharedir}/biddy/README.md

%files devel
%defattr(-, root, root, 0755)
%{_includedir}/
%{_libdir}/libbiddy.so
%{_libdir}/libbiddy.a

%changelog
