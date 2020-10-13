%define name	wav2cdr
%define ver     2.3.4
%define rel     10
%define prefix  /usr
#
# Imported from ~/.rpmmacros:
# SUSE Linux 10.0, Volker Kuhlmann <VolkerKuhlmann@gmx.de>

Summary:	Convert and cut (split) audio files for CD making
Summary(de):    Aufbereitung von Audio-Daten in ein für CDs brauchbares Format
Name:		%{name}
Version:	%{ver}
Release:	%{rel}
Copyright:	GPL
Group:		Productivity/Multimedia/Sound/Editors and Convertors
Source:		http://volker.dnsalias.net/soft/pck/wav2cdr-%{ver}.tar.bz2
URL:		http://volker.dnsalias.net/soft/
Vendor:         Volker Kuhlmann <VolkerKuhlmann@gmx.de>
BuildRoot:	/tmp/%{name}-%{version}-%{release}-%(id -u -n)
Prefix:		%{prefix}


%description
A conversion program for some audio formats into a format suitable for
CDs. Adapts to big and little endian machines automatically. Can perform
cutting on sampled tracks (useful when digitising records), and a few
other operations like volume changes or fading.

%description -l de
Umwandlungsprogramm für verschiedene Audioformate in ein Format,
welches auf CD geschrieben werden kann. Paßt sich automatisch an big-
und little-endian-Maschinen an. Verschiedene Operationen können mit
den Audiodaten durchgeführt werden, z.B. Zerschneiden (hilfreich beim
Digitalisieren von Schallplatten).


%prep
%setup -n %{name} -q

%build
# The FORTIFY_SOURCE is a tad stupid and barfs too much. Reduce from 2 to 1.
#make XCFLAGS="$RPM_OPT_FLAGS"
# The RPM_OPT_FLAGS may supply a -m32 for XLDFLAGS.
make \
  XCFLAGS="`echo "$RPM_OPT_FLAGS" | sed s/FORTIFY_SOURCE=2/FORTIFY_SOURCE=1/`" \
  XLDFLAGS="$RPM_OPT_FLAGS"

%install
test "$RPM_BUILD_ROOT" != "/" -a -d "$RPM_BUILD_ROOT" \
	&& rm -rf "$RPM_BUILD_ROOT"
#make -f Makefile.gmake prefix=${RPM_BUILD_ROOT}%{prefix} install
mkdir -p ${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}%{prefix}/bin
mkdir -p ${RPM_BUILD_ROOT}%{_mandir}/man1
install -c -m 755 wav2cdr ${RPM_BUILD_ROOT}%{prefix}/bin
install -c -m 644 wav2cdr.1 ${RPM_BUILD_ROOT}%{_mandir}/man1


%clean
test "$RPM_BUILD_ROOT" != "/" -a -d "$RPM_BUILD_ROOT" \
	&& rm -rf "$RPM_BUILD_ROOT"


%files
%defattr(-,root,root)
%{prefix}/bin/*
%doc README BUGS TODO
%doc %{_mandir}/*/*


%changelog

* Wed Jan 18 2006 - Volker Kuhlmann <VolkerKuhlmann@gmx.de>
- Package available from: http://volker.dnsalias.net/soft/
- Updated to 2.3.4, for SUSE 10.0

* Sun May 11 2003 Volker Kuhlmann <VolkerKuhlmann@gmx.de>
- Package available from: http://volker.dnsalias.net/soft/
- For SuSE 8.2

* Sun Feb 09 2003 Volker Kuhlmann <VolkerKuhlmann@gmx.de>
- For SuSE 8.1

* Tue Jun 27 2002 Volker Kuhlmann <VolkerKuhlmann@gmx.de>
- For SuSE 8.0

* Tue Nov 06 2001 Volker Kuhlmann <VolkerKuhlmann@gmx.de>
- For SuSE 7.3

* Sun Oct 29 2000 Volker Kuhlmann <VolkerKuhlmann@gmx.de>
- For SuSE 7.0; added support for $RPM_OPT_FLAGS

* Sun Feb 13 2000 Volker Kuhlmann <v.kuhlmann@elec.canterbury.ac.nz>
- For SuSE 6.3.

* Sun Aug 22 1999 Volker Kuhlmann <v.kuhlmann@elec.canterbury.ac.nz>
- for wav2cdr 2.3.2; 2 bug fixes, seconds may be fractions now

* Mon Jul 19 1999 Volker Kuhlmann <v.kuhlmann@elec.canterbury.ac.nz>
- for wav2cdr 2.3

* Sun Jun 06 1999 Volker Kuhlmann <v.kuhlmann@elec.canterbury.ac.nz>
- for wav2cdr 2.2.2

* Wed May 26 1999 Volker Kuhlmann <v.kuhlmann@elec.canterbury.ac.nz>
- for wav2cdr 2.2.1 + Red Hat 6.0

* Fri Apr 23 1999 Volker Kuhlmann <v.kuhlmann@elec.canterbury.ac.nz>
- Created first version of rpm
