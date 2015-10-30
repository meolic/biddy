# SPEC file for package bddscout
# Usage: rpmbuild -bb bddscout.spec
#
# You may have to create/change your ~/.rpmmacros
# http://rpm5.org/docs/api/macros.html
#
# Here is my ~/.rpmmacros which has only a single line:
# %_topdir        /home/meolic/biddy/rpm
#
# This file has been constructed using the following examples:
#
# http://www.rpm-based.org/how-to-create-rpm-package
# http://www.rpm.org/max-rpm/
# http://kmymoney2.sourceforge.net/phb/rpm-example.html
# http://dries.ulyssis.org/apt/packages/gnunet/gnunet-spec.html
#

Name: bddscout
Requires: libbiddy1, tcl, tk, bwidget
Summary: Visualization of Binary Decision Diagrams
Group: Education
Version: THISVERSION
Release: THISRELEASE
License: GPL
URL: http://biddy.meolic.com/
Source: bddscout-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}
Packager: meolic

%description
BDD Scout is a tool for visualization of Binary Decision Diagrams (BDDs).

 You can move nodes and subgraphs. The zoom-out capability allows you to
see the whole BDD. Graph can be saved, printed or exported as PNG image
and PDF document. Currently, only Biddy BDD package is supported.

http://biddy.meolic.com/

%package ifip-data
Summary: IFIP benchmark data
Group: Education

%description ifip-data
 Benchmark files used on IMEC-IFIP International Workshop on Applied Formal
Methods For Correct VLSI Design, Houthalen, Belgium, November 1989.
Every file contains a pair of circuits which have to be compared.

http://biddy.meolic.com/

%package bddtraces-data
Summary: Sample BDD traces
Group: Education

%description bddtraces-data
 These benchmark files are sample-traces from free software called
 bdd-trace-driver-0.9.

http://biddy.meolic.com/

%package ifip
Requires: bddscout, bddscout-ifip-data
Summary: BDD Scout extension (IFIP benchmark)
Group: Education

%description ifip
BDD Scout is a tool for visualization of Binary Decision Diagrams (BDDs).

 You can move nodes and subgraphs. The zoom-out capability allows you to
see the whole BDD. Graph can be saved, printed or exported as PNG image
and PDF document. Currently, only Biddy BDD package is supported.

http://biddy.meolic.com/

This package contains extension bddscoutIFIP (IFIP benchmark).

%package bddtraces
Requires: bddscout, bddscout-bddtraces-data
Summary: BDD Scout extension (BDD Traces)
Group: Education

%description bddtraces
BDD Scout is a tool for visualization of Binary Decision Diagrams (BDDs).

 You can move nodes and subgraphs. The zoom-out capability allows you to
see the whole BDD. Graph can be saved, printed or exported as PNG image
and PDF document. Currently, only Biddy BDD package is supported.

http://biddy.meolic.com/

This package contains extension bddscoutBDDTRACES (BDD Traces).

%package bra
Requires: bddscout
Summary: BDD Scout extension (Best Reordering Algorithm)
Group: Education

%description bra
BDD Scout is a tool for visualization of Binary Decision Diagrams (BDDs).

 You can move nodes and subgraphs. The zoom-out capability allows you to
see the whole BDD. Graph can be saved, printed or exported as PNG image
and PDF document. Currently, only Biddy BDD package is supported.

http://biddy.meolic.com/

This package contains extension bddscoutBRA (Best Reordering Algorithm).

%prep
echo "Preparing %{name}-%{version}-%{release}"
%setup -q
echo "Sources OK. Let's go building!"

%build
%{__make} package -f Makefile.Linux "BIDDYDIR = /usr/include -I/home/meolic/est/biddy" "BIDDYLIB = /usr/lib -L/home/meolic/est/biddy/bin" "BIDDYLIBEXT = -L/usr/lib -L/home/meolic/est/biddy/bin -lbiddy" "BINDIR = `pwd`"

%define _sharedir %{_prefix}/share

%install
%{__rm} -rf %{buildroot}
%{__make} -f Makefile.Linux DESTDIR=%{buildroot}/usr install "LIBDIR=lib64" "BINDIR=`pwd`"
%{__make} -f Makefile.Linux DESTDIR=%{buildroot}/usr install_bddscoutIFIP "LIBDIR=lib64" "BINDIR=`pwd`"
%{__make} -f Makefile.Linux DESTDIR=%{buildroot}/usr install_bddscoutBRA "LIBDIR=lib64" "BINDIR=`pwd`"
%{__make} -f Makefile.Linux DESTDIR=%{buildroot}/usr install_bddscoutBDDTRACES "LIBDIR=lib64" "BINDIR=`pwd`"
%{__make} -f Makefile.Linux DESTDIR=%{buildroot}/usr install_IFIP "BINDIR=`pwd`"
%{__make} -f Makefile.Linux DESTDIR=%{buildroot}/usr install_BDDTRACES "BINDIR=`pwd`"

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root, 0755)
%doc VERSION CHANGES
%{_bindir}/bddscout
%{_libdir}/bddscout/bddscout.tcl
%{_libdir}/bddscout/bddview.tcl
%{_libdir}/bddscout/bddscout.so
%{_libdir}/bddscout/pkgIndex.tcl
%{_sharedir}/bddscout/README
%{_sharedir}/bddscout/README.Linux
%{_sharedir}/bddscout/example.bddview
%{_sharedir}/bddscout/example.bdd
%{_sharedir}/bddscout/example.bf

%files ifip-data
%defattr(-, root, root, 0755)
%{_sharedir}/IFIP/

%files bddtraces-data
%defattr(-, root, root, 0755)
%{_sharedir}/BDDTRACES/

%files ifip
%defattr(-, root, root, 0755)
%{_libdir}/bddscout/bddscoutIFIP.so
%{_libdir}/bddscout/bddscoutIFIP.tcl
%{_libdir}/bddscout/pkgExtensionIFIP.tcl

%files bddtraces
%defattr(-, root, root, 0755)
%{_libdir}/bddscout/bddscoutBDDTRACES.so
%{_libdir}/bddscout/bddscoutBDDTRACES.tcl
%{_libdir}/bddscout/pkgExtensionBDDTRACES.tcl

%files bra
%defattr(-, root, root, 0755)
%{_libdir}/bddscout/bddscoutBRA.so
%{_libdir}/bddscout/bddscoutBRA.tcl
%{_libdir}/bddscout/pkgExtensionBRA.tcl

%changelog
