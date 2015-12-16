Name:       org.tizen.browser
Summary:    Tizen TV Open Browser
Version:    0.0.9
Release:    0
Group:      Applications/Web
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

# Excluded tizen v3.0 wayland on tv profile build
# TODO: Please remove following code block once wayland build is supported.
#%if "%{?_with_wayland}" == "1"
#ExcludeArch: armv7l i586 i686 x86_64 aarch64
#%endif

%if "%{?_with_wayland}" == "1"
BuildRequires: pkgconfig(ecore-wayland)
%else
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(utilX)
%endif

BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(ecore-imf)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(eeze)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(embryo)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(chromium-efl)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-network-connection)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(libtzplatform-config)
%if "%{?profile}" == "mobile"
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(haptic)
%endif
BuildRequires:  browser-provider-devel
BuildRequires:  pkgconfig(efl-extension)

BuildRequires:  cmake
BuildRequires:  gettext
BuildRequires:  edje-tools
BuildRequires:  boost-devel
BuildRequires:  boost-thread
#BuildRequires:  boost-date_time
BuildRequires:  boost-filesystem
BuildRequires:  boost-system

%if "%{?profile}" == "mobile"
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(capi-content-media-content)
%endif

%define BUILD_UT  %{?build_ut:ON}%{!?build_ut:OFF}
%if %BUILD_UT == "ON"
BuildRequires:  boost-test
%endif
%ifarch armv7l
BuildRequires:  pkgconfig(chromium-efl)
BuildRequires:	pkgconfig(dlog)
%endif

%define _appdir /usr/apps/%{name}
%define _bindir %{_appdir}/bin
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
    -DCOVERAGE_STATS=%{COVERAGE_STATS} \
    -DPROFILE=%{profile} \
%if "%{?_with_wayland}" == "1"
    -DWAYLAND_SUPPORT=On
%else
    -DWAYLAND_SUPPORT=Off
%endif

make %{!?verbose_make}%{?verbose_make:VERBOSE=1} -j%{?jobs}%{!?jobs:1}

%install
cd %{_build_dir}
%make_install

%post

mkdir -p /opt/usr/data/webkit/storage
mkdir -p /opt/usr/data/webkit/favicon

#Change ownership and privileges
chown -R 5000:5000 /opt/usr/data/webkit
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
%{_appdir}/services/*
%{_appdir}/lib/*
%{_appdir}/res/certs/*
%{_appdir}/res/locale/*/*/browser.mo

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

%endif
