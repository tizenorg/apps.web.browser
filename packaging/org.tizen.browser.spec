Name:       org.tizen.browser
Summary:    Tizen TV Open Browser
Version:    0.0.8
Release:    0
Group:      Applications/Web
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(ecore-imf)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(eeze)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(embryo)
BuildRequires:  pkgconfig(evas)
#BuildRequires:  pkgconfig(ewebkit2)
BuildRequires:  pkgconfig(chromium-efl)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-network-connection)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(utilX)
BuildRequires: browser-provider-devel

BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  boost-devel
BuildRequires:  boost-thread
#BuildRequires:  boost-date_time
BuildRequires:  boost-filesystem
BuildRequires:  boost-system

%define BUILD_UT  %{?build_ut:ON}%{!?build_ut:OFF}
%if %BUILD_UT == "ON"
BuildRequires:  boost-test
%endif
%ifarch armv7l
#BuildRequires:  pkgconfig(ewebkit2)
BuildRequires:  pkgconfig(chromium-efl)
BuildRequires:	pkgconfig(dlog)
%endif

%define _appdir /usr/apps/%{name}
%define _bindir %{_appdir}/bin
%define _appdatadir /opt/usr/apps/%{name}
%define COVERAGE_STATS %{?coverage_stats:ON}%{!?coverage_stats:OFF}

%define _manifestdir /usr/share/packages
%define _icondir /usr/share/icons/default/small
%define _demodir /usr/apps/org.tizen.browser/res/demo

%description
WebKit browser with EFL for Tizen TV Platform.

%prep
%setup -q

%build
%define _build_dir build-tizen
mkdir -p %{_build_dir}
cd %{_build_dir}

cmake .. \
    -DCMAKE_BUILD_TYPE=%{?build_type}%{!?build_type:RELEASE} \
    -DCMAKE_INSTALL_PREFIX=%{_appdir} \
    -DPACKAGE_NAME=%{name} \
    -DBINDIR=%{_bindir} \
    -DVERSION=%{version} \
    -DMANIFESTDIR=%{_manifestdir} \
    -DICONDIR=%{_icondir} \
    -DBUILD_UT=%{BUILD_UT} \
    -DCOVERAGE_STATS=%{COVERAGE_STATS}

make %{!?verbose_make}%{?verbose_make:VERBOSE=1} -j%{?jobs}%{!?jobs:1}

%install
cd %{_build_dir}
%make_install

%post

#Prepare files
if [ ! -f %{_appdatadir}/res/db/bookmark.db ];
then
    mkdir -p %{_appdatadir}/res/db
    chsmack -a "dtv-org.tizen.browser" %{_appdatadir}/res/db
    sqlite3 %{_appdatadir}/res/db/bookmark.db ''
    chsmack -a "dtv-org.tizen.browser" %{_appdatadir}/res/db/bookmark.db
fi

mkdir -p /opt/usr/data/webkit/storage
mkdir -p /opt/usr/data/webkit/favicon

#Change ownership and privileges
chown -R 5000:5000 %{_appdatadir}/res/db
chown -R 5000:5000 /opt/usr/data/webkit
chmod -R 777 %{_appdatadir}/res/db
chmod 777 %{_appdatadir}/res/db/bookmark.db
chmod -R 660 /opt/usr/data/webkit

%files
%manifest org.tizen.browser.manifest
%{_icondir}/org.tizen.browser.png
%{_icondir}/apps_img_web_default_4x2.png
%{_demodir}/*
%{_manifestdir}/%{name}.xml
%defattr(-,root,root,-)
%{_appdir}/bin/browser
%{_appdir}/res/edje/*/*.edj
%if %BUILD_UT == "ON"
%exclude %{_appdir}/services/libTestService*
%endif
%{_appdir}/services/*
%{_appdir}/lib/*
%defattr(-,app,app,-)
#%{_appdir}/res/*.png
#%{_appdir}/res/*.ico
%{_appdir}/res/certs/*

#-----------------------------------
%if %BUILD_UT == "ON"
%package ut
Summary:    BrowserAPP Unit Tests
#Requires:	org.tizen.browser

%description ut
BrowserAPP Unit Tests.

%files ut
%defattr(-,root,root,-)
%{_appdir}/bin/browser-ut
%{_appdir}/services/libTestService*

%endif
