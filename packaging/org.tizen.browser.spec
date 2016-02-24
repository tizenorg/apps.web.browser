%define appdir /usr/apps/org.tizen.browser
%define appdatadir /opt/usr/apps/org.tizen.browser

%define _unpackaged_files_terminate_build 0

Name:       org.tizen.browser
Summary:    reference browser
Version: 5.4.3
Release:   0
Group:      misc
License:    Flora-1.1 and Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /usr/bin/sqlite3

# Excluded wearable profile build
# chromium-efl doesn't support the wearable profile
%if "%{?tizen_profile_name}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

%if "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

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
BuildRequires:  pkgconfig(libsoup-2.4)
BuildRequires:  pkgconfig(libssl)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(ecore-imf)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(deviced)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(capi-appfw-preference)
BuildRequires:  pkgconfig(capi-network-connection)
BuildRequires:  pkgconfig(capi-content-media-content)
BuildRequires:  pkgconfig(capi-network-wifi)
BuildRequires:  pkgconfig(appcore-common)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(gnutls)
BuildRequires:  pkgconfig(embryo)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(vconf-internal-keys)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(capi-media-image-util)
BuildRequires:  pkgconfig(capi-web-tab)
BuildRequires:  pkgconfig(capi-web-bookmark)
BuildRequires:  pkgconfig(capi-web-history)
BuildRequires:  pkgconfig(capi-web-scrap)
BuildRequires:  pkgconfig(capi-media-image-util)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(storage)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(json-glib-1.0)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  cmake
BuildRequires:  gettext
BuildRequires:  edje-tools
BuildRequires:  hash-signer
BuildRequires:  pkgconfig(ewebkit2)

#This is for SMACK
Requires(post): sys-assert

%description
org.tizen.browser

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
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.Flora-1.1 %{buildroot}/usr/share/license/org.tizen.browser

PKG_ID=%{name}
%define tizen_sign 1
%define tizen_sign_base /usr/apps/${PKG_ID}
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1

%post
mkdir -p %{appdatadir}/data/
mkdir -p %{appdatadir}/data/db/

##### Certificate ######
if [ ! -f %{appdatadir}/data/db/.certificate.db ];
then
	sqlite3 %{appdatadir}/data/db/.certificate.db 'PRAGMA journal_mode=PERSIST;
	create table certificate(id integer primary key autoincrement, pem, host, allow INTEGER);'
fi

##### USER AGENTS ######
if [ ! -f %{appdatadir}/data/db/.browser.db ];
then
        sqlite3 %{appdatadir}/data/db/.browser.db 'PRAGMA journal_mode=PERSIST;
        create table user_agents(name primary key, value)'
        # mobile
        sqlite3 %{appdatadir}/data/db/.browser.db 'PRAGMA journal_mode=PERSIST;
        insert into user_agents values("Mobile - Kiran", "Mozilla/5.0 (Linux; Tizen 2.3; SAMSUNG SM-Z130H) AppleWebKit/537.3 (KHTML, like Gecko) SamsungBrowser/1.0 Mobile Safari/537.3");
        insert into user_agents values("Mobile - Firefox", "Mozilla/5.0 (Android; Mobile; rv:29.0) Gecko/29.0 Firefox/29.0");
        insert into user_agents values("Mobile - Chrome for android", "Mozilla/5.0 (Linux; Android 4.4.2; SM-G900K Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.122 Mobile Safari/537.36");
        insert into user_agents values("Mobile - Opera", "Mozilla/5.0 (Linux; Android 4.4.2; SM-G900K Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/34.0.1847.116 Mobile Safari/537.36 OPR/21.0.1437.74904");
        insert into user_agents values("Mobile - Opera Mini", "Opera/9.80 (Android; Opera Mini/7.5.35199/34.2152; U; en) Presto/2.8.119 Version/11.10");
        insert into user_agents values("Desktop - Firefox 22.0", "Mozilla/5.0 (Windows NT 6.1; rv:22.0) Gecko/20100101 Firefox/22.0");
        insert into user_agents values("Desktop - Chrome 35.0", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.114 Safari/537.36");
        insert into user_agents values("Desktop - Internet Explorer 10", "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Trident/6.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; InfoPath.3; MS-RTC LM 8; Tablet PC 2.0; .NET4.0E)");
        insert into user_agents values("Desktop - Opera 21.0", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/34.0.1847.132 Safari/537.36 OPR/21.0.1432.67 (Edition Campaign 38)");
        insert into user_agents values("Desktop - Safari 5.1.7", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/534.57.2 (KHTML, like Gecko) Version/5.1.7 Safari/534.57.2");
        insert into user_agents values("Desktop - Safari 6.0", "Mozilla/5.0 (iPad; CPU OS 6_0 like Mac OS X) AppleWebKit/536.26 (KHTML, like Gecko) Version/6.0 Mobile/10A5355d Safari/8536.25");
        insert into user_agents values("Apple iOS 7.0 - mobile", "Mozilla/5.0 (iPhone; CPU iPhone OS 7_0 like Mac OS X) AppleWebKit/537.51.1 (KHTML, like Gecko) Version/7.0 Mobile/11A465 Safari/9537.53");
        insert into user_agents values("Apple iOS 6.0 - pad", "Mozilla/5.0 (iPad; CPU OS 6_0 like Mac OS X) AppleWebKit/536.26 (KHTML, like Gecko) Version/6.0 Mobile/10A5376e Safari/8536.25");
        insert into user_agents values("Apple iOS 6.0 - mobile", "Mozilla/5.0 (iPhone; CPU iPhone OS 6_0 like Mac OS X) AppleWebKit/536.26 (KHTML, like Gecko) Version/6.0 Mobile/10A5376e Safari/8536.25");
        insert into user_agents values("Apple iOS 5.0 - pad", "Mozilla/5.0 (iPad; CPU OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3");
        insert into user_agents values("Apple iOS 5.0 - mobile", "Mozilla/5.0 (iPhone; CPU iPhone OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3");
        insert into user_agents values("Galaxy S5", "Mozilla/5.0 (Linux; Android 4.4.2; en-us; SAMSUNG SM-G900K/KTU1AND8 Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Version/1.6 Chrome/28.0.1500.94 Mobile Safari/537.36");
        insert into user_agents values("Galaxy S4", "Mozilla/5.0 (Linux; Android 4.2.2; en-gb; SAMSUNG GT-I9500 Build/JDQ39) AppleWebKit/535.19 (KHTML, like Gecko) Version/1.0 Chrome/18.0.1025.308 Mobile Safari/535.19");
        insert into user_agents values("Galaxy S note2", "Mozilla/5.0 (Linux; U; Android 4.3; ko-kr; SHV-E250K/KKUENC3 Build/JSS15J) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30");
        insert into user_agents values("System user agent", "")'
fi

# set default vconf values
vconftool set -t string db/browser/browser_user_agent "System user agent" -g 5000 -f -s tizen::vconf::platform::rw
vconftool set -t string db/browser/custom_user_agent "" -g 5000 -f -s tizen::vconf::platform::rw

# Change db file owner & permission
chown -R 5000:5000 %{appdir}/data
chown -R 5000:5000 %{appdatadir}/data
chmod 660 %{appdatadir}/data/db/.certificate.db
chmod 660 %{appdatadir}/data/db/.certificate.db-journal
chmod 660 %{appdatadir}/data/db/.browser.db
chmod 660 %{appdatadir}/data/db/.browser.db-journal

# Apply SMACK label to database files
if [ -f /usr/lib/rpm-plugins/msm.so ]
then
#	chsmack -a 'org.tizen.browser' %{appdir}/data/.pref.db*
	chsmack -a 'org.tizen.browser' %{appdatadir}/data/.pref.db*
	chsmack -a 'org.tizen.browser' %{appdatadir}/data/db/.certificate.db*
	chsmack -a 'org.tizen.browser' %{appdatadir}/data/db/.browser.db*
fi

%files
%manifest org.tizen.browser.manifest
%defattr(-,root,root,-)
%{appdir}/bin/browser
%{appdir}/res/edje/*.edj
%{appdir}/res/html/*
%{appdir}/res/js/*
/usr/share/icons/default/small/org.tizen.browser.png
%{appdir}/res/images/*
%{appdir}/res/locale/*/*/browser.mo
%{appdir}/*.xml
%{appdatadir}/data/.pref.db
%{appdatadir}/data/.pref.db-journal
/usr/share/packages/org.tizen.browser.xml
/usr/share/license/org.tizen.browser
/etc/smack/accesses.d/org.tizen.browser.efl
%{appdir}/lib/ug/*.so
