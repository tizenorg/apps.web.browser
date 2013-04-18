%define appdir /usr/apps/org.tizen.browser
%define appdatadir /opt/usr/apps/org.tizen.browser

Name:       org.tizen.browser
Summary:    webkit browser with EFL
Version: 0.1.23
Release:    0
Group:      misc
License:    Flora License, Version 1
Source0:    %{name}-%{version}.tar.gz
Requires(post): /usr/bin/sqlite3

BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(tapi)
BuildRequires:  pkgconfig(secure-storage)
BuildRequires:  pkgconfig(libsoup-2.4)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(libssl)
BuildRequires:  pkgconfig(capi-location-manager)
BuildRequires:  pkgconfig(accounts-svc)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(ecore-imf)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(ewebkit2)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(devman)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(capi-web-url-download)
BuildRequires:  pkgconfig(capi-network-connection)
BuildRequires:  pkgconfig(capi-network-wifi)
BuildRequires:  pkgconfig(haptic)
BuildRequires:  pkgconfig(shortcut)

BuildRequires:  pkgconfig(appcore-common)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(gnutls)
BuildRequires:  pkgconfig(embryo)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(syspopup-caller)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(vconf-internal-keys)
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(capi-system-sensor)
BuildRequires:  pkgconfig(capi-network-nfc)
BuildRequires:  pkgconfig(capi-media-image-util)


BuildRequires:  cmake
BuildRequires:  gettext
BuildRequires:  edje-tools

#This is for SMACK
Requires: sys-assert

%description
webkit browser with EFL.

%prep
%setup -q

%build
CFLAGS+=" -fPIC";export CFLAGS
CXXFLAGS+=" -fPIC -fvisibility=hidden -fvisibility-inlines-hidden";export CXXFLAGS
export LDFLAGS+="-Wl,--rpath=%{appdir}/lib -Wl,--hash-style=both -Wl,--as-needed"
LDFLAGS="$LDFLAGS"
cmake . -DCMAKE_INSTALL_PREFIX=%{appdir}

make %{?jobs:-j%jobs}

%install
%make_install

%post
mkdir -p %{appdatadir}/data/db/
##### Certificate ######
if [ ! -f %{appdatadir}/data/db/.certificate.db ];
then
        sqlite3 %{appdatadir}/data/db/.certificate.db 'PRAGMA journal_mode=PERSIST;
        create table certificate(id integer primary key autoincrement, pem, allow INTEGER);'
fi

##### Geolocation ######
if [ ! -f %{appdatadir}/data/db/.browser-geolocation.db ];
then
	sqlite3 %{appdatadir}/data/db/.browser-geolocation.db 'PRAGMA journal_mode=PERSIST;
	CREATE TABLE geolocation(id integer primary key autoincrement, address, title, accept INTEGER, updatedate DATETIME);'
fi

##### html5 custom handler db ######
if [ ! -f %{appdatadir}/data/db/.html5-custom-handler.db ];
then
	sqlite3 %{appdatadir}/data/db/.html5-custom-handler.db 'PRAGMA journal_mode=PERSIST;
	create table custom_protocol_handler(id INTEGER PRIMARY KEY, base_uri TEXT, protocol TEXT, uri TEXT, allow INTEGER);
	create table custom_content_handler(id INTEGER PRIMARY KEY, base_uri TEXT, mime TEXT, uri TEXT, allow INTEGER);'
fi

# Change db file owner & permission
chmod 660 %{appdatadir}/data/db/.browser-history.db
chmod 660 %{appdatadir}/data/db/.browser-history.db-journal
chmod 660 %{appdatadir}/data/db/.browser-bookmark.db
chmod 660 %{appdatadir}/data/db/.browser-bookmark.db-journal
chmod 660 %{appdatadir}/data/db/.certificate.db
chmod 660 %{appdatadir}/data/db/.certificate.db-journal
chmod 660 %{appdatadir}/data/db/.browser-geolocation.db
chmod 660 %{appdatadir}/data/db/.browser-geolocation.db-journal
chmod 660 %{appdatadir}/data/db/.html5-custom-handler.db
chmod 660 %{appdatadir}/data/db/.html5-custom-handler.db-journal

# Change file owner
chown -R 5000:5000 %{appdatadir}/data

# Apply SMACK label to database files
if [ -f /usr/lib/rpm-plugins/msm.so ]
then
	chsmack -a 'org.tizen.browser' %{appdatadir}/data/db/.browser-history.db*
	chsmack -a 'org.tizen.browser' %{appdatadir}/data/db/.browser-bookmark.db*
	chsmack -a 'org.tizen.browser' %{appdatadir}/data/db/.certificate.db*
	chsmack -a 'org.tizen.browser' %{appdatadir}/data/db/.browser-geolocation.db*
	chsmack -a 'org.tizen.browser' %{appdatadir}/data/db/.html5-custom-handler.db*
fi

%files
%manifest org.tizen.browser.manifest
%defattr(-,root,root,-)
%{appdir}/bin/browser
%{appdir}/res/edje/*.edj
%{appdir}/res/html/*
%{appdir}/res/js/*
%{appdir}/res/template/
%{appdir}/res/template/default_application_icon.png
%{appdir}/res/template/config_sample.xml
%{appdir}/res/template/template_bluetooth_content_share.html
/usr/share/icons/default/small/org.tizen.browser.png
%{appdir}/res/images/*
%{appdir}/res/locale/*/*/browser.mo
%{appdatadir}/data/xml/
/usr/share/packages/*
