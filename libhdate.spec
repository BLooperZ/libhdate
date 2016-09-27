Name:	     libhdate
Version:     1.6.02
Release:     1%{?dist}
Summary:     C,C++ library for Hebrew calendar and dates
License:     GPLv3
URL:         http://libhdate.sourceforge.net
Source0:     http://downloads.sourceforge.net/%{name}/%{name}-%{version}.tar.bz2

%description
libhdate is a C library for Hebrew calendar information,
including dates, holidays, and times of day. 
The package includes: 
hcal, a command line Hebrew calendar program; 
hdate, a command line program for date and times of day information; 
and a collection of library bindings to other programming languages 
(C++, pascal, python, perl, ruby, php).

%prep
%autosetup

%build
autoreconf -fi
%configure
make %{?_smp_mflags}

%install
rm -rf %{buildroot} 
%makeinstall

%clean
rm -rf %{buildroot}

%files
%license COPYING
%doc ABOUT-NLS AUTHORS ChangeLog INSTALL NEWS README USE
%{_bindir}/hcal
%{_bindir}/hdate
%{_includedir}/hdate.h
%{_includedir}/hdatepp.h
%{_libdir}/libhdate.a
%{_libdir}/libhdate.la
%{_libdir}/libhdate.so
%{_libdir}/libhdate.so.1
%{_libdir}/libhdate.so.1.0.6
%{_libdir}/pkgconfig/libhdate.pc
%{_mandir}/man1/hcal.1.*
%{_mandir}/man1/hdate.1.*
%{_mandir}/man3/hdate.3.*
%{_mandir}/man3/hdate.h.3.*
%{_mandir}/man3/hdate_Hdate.3.*
%{_mandir}/man3/hdate_struct.3.*
%{_mandir}/man3/hdatepp.h.3.*
%{_docdir}/libhdate/COPYING
%{_docdir}/libhdate/examples/bindings/*
%{_docdir}/libhdate/examples/bindings/pascal/*
%{_docdir}/libhdate/examples/hcal/*
%{_datadir}/locale/es_MX/LC_MESSAGES/libhdate.mo
%{_datadir}/locale/fr/LC_MESSAGES/libhdate.mo
%{_datadir}/locale/he/LC_MESSAGES/libhdate.mo
%{_datadir}/locale/ru/LC_MESSAGES/libhdate.mo
%{_datadir}/locale/sv/LC_MESSAGES/libhdate.mo

%changelog
* Tue Sep 27 2016 Niv Baehr
- Initial version of the package spec file
