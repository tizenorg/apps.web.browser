%define appdir /opt/apps/org.tizen.browser

Name:       org.tizen.browser
Summary:    webkit browser with EFL
Version:	0.1.14
Release:    2
Group:      Applications
License:    Samsung Proprietary License
Source0:    %{name}-%{version}.tar.bz2
#Patch0:     change-float-abi.patch
Requires(post): /usr/bin/sqlite3
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(gnutls)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(devman)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(ecore-evas)
BuildRequires: pkgconfig(ecore-imf)
BuildRequires: pkgconfig(ecore-input)
BuildRequires: pkgconfig(ecore-x)
BuildRequires: pkgconfig(edbus)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(elm-webview)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(ewebkit)
BuildRequires: pkgconfig(libsoup-2.4)
BuildRequires: pkgconfig(libssl)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(secure-storage)
BuildRequires: pkgconfig(sensor)
BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(ui-gadget)
BuildRequires: pkgconfig(utilX)
BuildRequires: pkgconfig(accounts-svc)
BuildRequires: pkgconfig(libsoup-2.4)
BuildRequires: pkgconfig(ewebkit)
BuildRequires: pkgconfig(syspopup-caller)
BuildRequires: pkgconfig(capi-web-url-download)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(devman_haptic)

BuildRequires:  cmake
BuildRequires:  gettext-tools
BuildRequires:  edje-tools

%description
webkit browser with EFL.



%prep
%setup -q
#%patch0 -p1

%build
export LDFLAGS+="-Wl,--rpath=%{appdir}/lib -Wl,--hash-style=both -Wl,--as-needed"
LDFLAGS="$LDFLAGS"
cmake . -DCMAKE_INSTALL_PREFIX=%{appdir}

make %{?jobs:-j%jobs}

%install
%make_install

%post
# Change file owner
if [ ${USER} == "root" ]
then
    # Change file owner
    chown -R 5000:5000 /opt/apps/org.tizen.browser/data
fi

### Bookmark ### 
if [ ! -f /opt/dbspace/.internet_bookmark.db ];
then
	sqlite3 /opt/dbspace/.internet_bookmark.db 'PRAGMA journal_mode=PERSIST;
	CREATE TABLE bookmarks(id INTEGER PRIMARY KEY AUTOINCREMENT, type INTEGER, parent INTEGER, address, title, creationdate, sequence INTEGER, updatedate, editable INTEGER, accesscount INTEGER, favicon BLOB, favicon_length INTEGER, favicon_w INTEGER, favicon_h INTEGER);
	create index idx_bookmarks_on_parent_type on bookmarks(parent, type);

	insert into bookmarks (type, parent, title, creationdate, editable, sequence, accesscount) values(1, 0, "Bookmarks", DATETIME("now"),  0, 1, 0);'
fi

##### History ######
if [ ! -f /opt/dbspace/.browser-history.db ];
then
	sqlite3 /opt/dbspace/.browser-history.db 'PRAGMA journal_mode=PERSIST;
	CREATE TABLE history(id INTEGER PRIMARY KEY AUTOINCREMENT, address, title, counter INTEGER, visitdate DATETIME, favicon BLOB, favicon_length INTEGER, favicon_w INTEGER, favicon_h INTEGER);'
fi

mkdir -p /opt/apps/org.tizen.browser/data/db
##### Notification #####
if [ ! -f /opt/apps/org.tizen.browser/data/db/.browser-notification.db ];
then
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser-notification.db 'PRAGMA journal_mode=PERSIST;
	CREATE TABLE notification_table(id INTEGER PRIMARY KEY AUTOINCREMENT, notification INTEGER, title, body, url, iconURL, iconValidity INTEGER);
	CREATE TABLE notification_permitted_domains(domain PRIMARY KEY);'
fi

##### Password ######
if [ ! -f /opt/apps/org.tizen.browser/data/db/.browser-credential.db ];
then
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser-credential.db 'PRAGMA journal_mode=PERSIST;
	create table passwords(id integer primary key autoincrement, address, login, password)'
fi

#### USER AGENTS #####
#initDB
#rm /opt/apps/org.tizen.browser/data/db/.browser.db
# create db
#FILE = /opt/apps/org.tizen.browser/data/db/.browser.db
if [ ! -f /opt/apps/org.tizen.browser/data/db/.browser.db ];
then
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser.db 'PRAGMA journal_mode=PERSIST;
	create table user_agents(name primary key, value)'
	# mobile 
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser.db 'PRAGMA journal_mode=PERSIST;
	insert into user_agents values("Galaxy S", "Mozilla/5.0 (Linux; U; Android 2.3.3; en-gb; GT-I9000 Build/GINGERBREAD) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1");
	insert into user_agents values("Galaxy S II", "Mozilla/5.0 (Linux; U; Android 2.3.4; en-us; GT-I9100 Build/GINGERBREAD) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1");
	insert into user_agents values("SLP Galaxy", "Mozilla/5.0 (Linux; U; Android 2.3.4; en-us; GT-I9500 Build/GINGERBREAD) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1");
	insert into user_agents values("Tizen", "Mozilla/5.0 (Linux; U; Tizen 1.0; en-us) AppleWebKit/534.46 (KHTML, like Gecko) Mobile Tizen Browser/1.0");
	insert into user_agents values("Galaxy Nexus", "Mozilla/5.0 (Linux; U; Android 4.0.1; en-us; Galaxy Nexus Build/ITL31) AppleWebkit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30");
	insert into user_agents values("Samsung", "Mozilla/5.0 (SAMSUNG; SAMSUNG-GT-I9200/1.0; U; Linux/SLP/2.0; ko-kr) AppleWebKit/534.4 (KHTML, like Gecko) Dolfin/2.0 Mobile");
	insert into user_agents values("Samsung Dolfin", "SAMSUNG-GT-S8500/S8500XXJD2 SHP/VPP/R5 Dolfin/2.0 Nextreaming SMM-MMS/1.2.0 profile/MIDP-2.1 configuration/CLDC-1.1");
	insert into user_agents values("Apple iPhone 3", "Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/420.1 (KHTML, like Gecko) Version/3.0 Mobile/1A542a Safari/419.3");
	insert into user_agents values("Apple iPhone 4", "Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_2_1 like Mac OS X; en-us) AppleWebKit/533.17.9 (KHTML, like Gecko) Mobile/8C148 Safari/6533.18.5");
	insert into user_agents values("Apple iOS 5", "Mozilla/5.0 (iPhone; CPU iPhone OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.0.2 Mobile/9A5259f Safari/6533.18.5");
	insert into user_agents values("Android 2.1 (Nexus One)", "Mozilla/5.0 (Linux; U; Android 2.1; en-us; Nexus One Build/ERD62) AppleWebKit/530.17 (KHTML, like Gecko) Version/4.0 Mobile Safari/530.17");
	insert into user_agents values("Opera Mobi", "Opera/9.80 (Windows NT 6.1; Opera Mobi/49; U; en) Presto/2.4.18 Version/10.00");
	insert into user_agents values("Samsung Bada", "Mozilla/5.0 (SAMSUNG; SAMSUNG-GT-S8500/1.0; U; Bada/1.0; en-us) AppleWebKit/533.1 (KHTML, like Gecko) Dolfin/2.0 Mobile WVGA SMM-MMS/1.2.0 OPN-B");
	insert into user_agents values("Orange TV", "Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en) AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543a Safari/419.3 OrangeAppliTV/2.3.9");
	insert into user_agents values("Chrome Browser for android", "Mozilla/5.0 (Linux; U; Android 4.0.1; ko-kr; Galaxy Nexus Build/ITL41F) AppleWebKit/535.7 (KHTML, like Gecko) CrMo/16.0.912.75 Mobile Safari/535.7");
	insert into user_agents values("Samsung Bada 2.0", "Mozilla/5.0 (SAMSUNG; SAMSUNG-GT-S8500/1.0; U; Bada/2.0; en-us) AppleWebKit/534.20 (KHTML, like Gecko) Mobile WVGA SMM-MMS/1.2.0 OPN-B Dolfin/3.0")'

	# desktop
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser.db 'PRAGMA journal_mode=PERSIST;
	insert into user_agents values("Samsung Desktop", "Mozilla/5.0 (U; Linux/SLP/2.0; ko-kr) AppleWebKit/533.1 (KHTML, like Gecko)");
	insert into user_agents values("Firefox 5", "Mozilla/5.0 (Windows NT 5.1; rv:5.0) Gecko/20110706 Firefox/5.0 ");
	insert into user_agents values("Firefox 5 Fennec(Mobile)", "Mozilla/5.0 (Android; Linux armv7l; rv:5.0) Gecko/20110615 Firefox/5.0 Fennec/5.0");
	insert into user_agents values("Safari 5.0", "Mozilla/5.0 (Windows; U; Windows NT 6.1; ja-JP) AppleWebKit/533.16 (KHTML, like Gecko) Version/5.0 Safari/533.16");
	insert into user_agents values("Google Chrome 13.0", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/13.0.782.41 Safari/535.1");
	insert into user_agents values("Internet Explorer 9", "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)");
	insert into user_agents values("Galaxy Tab 10.1", "Mozilla/5.0 (Linux; U; Android 3.0.1; en-us; GT-P7100 Build/HRI83) AppleWebKit/534.13 (KHTML, like Gecko) Version/4.0 Safari/534.13");
	insert into user_agents values("iPad 2", "Mozilla/5.0(iPad; U; CPU OS 4_3 like Mac OS X; en-us) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8F191 Safari/6533.18.5")'
fi

#init cookies DB
#rm /optpps/org.tizen.browser/data/db/dbspace/.browser-cookies.db
# create cookies db
#FILE = /opt/apps/org.tizen.browser/data/db/.browser-cookies.db

if [ ! -f /opt/apps/org.tizen.browser/data/db/.browser-cookies.db ];
then
	sqlite3 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db 'CREATE TABLE moz_cookies (id INTEGER PRIMARY KEY, name TEXT, value TEXT, host TEXT, path TEXT,expiry INTEGER, lastAccessed INTEGER, isSecure INTEGER, isHttpOnly INTEGER);'
	#test cookie
	#sqlite3 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db 'INSERT INTO moz_cookies values(NULL, "cookies_name_test", "cookies_value_test", "www.cookies_test.com", "cookies_path_test", 2011, NULL, 1, 0);'
fi

# Change db file owner & permission
if [ ${USER} == "root" ]  
then  
	#chown root:root /opt/apps/org.tizen.browser/data/db  
	chown -R 5000:5000 /opt/apps/org.tizen.browser/data/db 
	chown :6002 /opt/apps/org.tizen.browser/data/db/.browser.db
	chown :6002 /opt/apps/org.tizen.browser/data/db/.browser.db-journal
	chown :6002 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db
	chown :6002 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db-journal
	chown :6002 /opt/dbspace/.browser-history.db
	chown :6002 /opt/dbspace/.browser-history.db-journal
	chown :6002 /opt/dbspace/.internet_bookmark.db
	chown :6002 /opt/dbspace/.internet_bookmark.db-journal
	chown :6002 /opt/apps/org.tizen.browser/data/db/.browser-credential.db
	chown :6002 /opt/apps/org.tizen.browser/data/db/.browser-credential.db-journal
	chown :6002 /opt/apps/org.tizen.browser/data/db/.browser-notification.db
	chown :6002 /opt/apps/org.tizen.browser/data/db/.browser-notification.db-journal
	chmod 660 /opt/apps/org.tizen.browser/data/db/.browser.db
	chmod 660 /opt/apps/org.tizen.browser/data/db/.browser.db-journal
	chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db
	chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-cookies.db-journal
	chmod 666 /opt/dbspace/.browser-history.db
	chmod 666 /opt/dbspace/.browser-history.db-journal
	chmod 660 /opt/dbspace/.internet_bookmark.db
	chmod 660 /opt/dbspace/.internet_bookmark.db-journal
	chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-credential.db
	chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-credential.db-journal
	chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-notification.db
	chmod 660 /opt/apps/org.tizen.browser/data/db/.browser-notification.db-journal
fi

##################################################
# set default vconf values
##################################################
if [ ${USER} == "root" ]
then
	vconftool set -t bool db/browser/ShowMySitesGuide 1 -g 6514
        vconftool set -t string db/browser/Last/Url "" -g 6514
        vconftool set -t string db/browsersetting/LastVisitedUrl "" -g 6514
	vconftool set -t int db/browser/BrowserBrightnessLevel -1 -g 6514
# Browser settings vconf values
# Set vconf values with -g/-u options
	vconftool set -t string db/browsersetting/Homepage "Most visited sites" -g 6514 # "Most visited sites", "user set site", "Empty page"
	vconftool set -t string db/browsersetting/HomepageMode "MOST_VISITED_SITES" -g 6514
	vconftool set -t string db/browsersetting/UserAgent "Tizen" -g 6514
	vconftool set -t string db/browsersetting/UserHomepage "www.tizen.org" -g 6514 # default site is www.tizen.org
	vconftool set -t string db/browsersetting/DefaultViewLevel "Readable" -g 6514 # "Readable", "Fit to width"
	vconftool set -t bool db/browsersetting/EnableLandscape 1 -g 6514
	vconftool set -t bool db/browsersetting/RunJavaScript 1 -g 6514
	vconftool set -t bool db/browsersetting/DisplayImages 1 -g 6514
	vconftool set -t bool db/browsersetting/BlockPopup 1 -g 6514
	vconftool set -t string db/browsersetting/AutoSaveIDPassword "Always ask" -g 6514 # "On", "Off"
	vconftool set -t string db/browsersetting/SaveIDPassword "ALWAYS_ASK" -g 6514 # "On", "Off"
	vconftool set -t string db/browsersetting/CustomUserAgent "" -g 6514
	# set default vconf value for reader
	vconftool set -t bool db/browsersetting/RunReader 1 -g 6514
	vconftool set -t int db/browsersetting/FontSize 16 -g 6514
	# set default vconf value for plugins
	vconftool set -t bool db/browsersetting/RunPlugins 1 -g 6514
	vconftool set -t bool db/browsersetting/RunFlash 0 -g 6514
	vconftool set -t bool db/browsersetting/PauseFlash 1 -g 6514
	# set search vconf
	vconftool set -t string db/browsersetting/SearchEngine "Google" -g 6514 # "Google", "Yahoo", "Bing"
	vconftool set -t string db/browsersetting/SearchUrl "http://www.google.com/m/search?q=" -g 6514
	vconftool set -t bool db/browsersetting/SearchCaseSensitive 0 -g 6514
	# privacy
	vconftool set -t string db/browsersetting/CookieOption "Accept all" -g 6514
	vconftool set -t bool db/browsersetting/CookieOptionInt 1 -g 6514
	# performance
	vconftool set -t bool db/browsersetting/FastRendering 1 -g 6514
	vconftool set -t bool db/browsersetting/LargeRenderingBuffer 1 -g 6514
	vconftool set -t bool db/browsersetting/AcceleratedComposition 1 -g 6514
	vconftool set -t bool db/browsersetting/SamsungAppsInstall 0 -g 6514
	vconftool set -t bool db/browsersetting/ExternalVideoPlayer 0 -g 6514 # if the AcceleratedComposition is 0, this must be 0
	vconftool set -t bool db/browsersetting/CompositedRenderLayerBorders 0 -g 6514 # if the AcceleratedComposition is 0, this must be 0
	vconftool set -t bool db/browsersetting/PhysicsEngine 0 -g 6514
	vconftool set -t bool db/browsersetting/RecordingSurface 0 -g 6514
	vconftool set -t bool db/browsersetting/RemoteWebInspector 0 -g 6514
	vconftool set -t bool db/browsersetting/DemoSetting 0 -g 6514
	vconftool set -t bool db/browsersetting/DemoMode 0 -g 6514
else
	vconftool set -t bool db/browser/ShowMySitesGuide 1
        vconftool set -t string db/browser/Last/Url ""
        vconftool set -t string db/browsersetting/LastVisitedUrl ""
	vconftool set -t int db/browser/BrowserBrightnessLevel -1
# Browser settings vconf values
# Set without -g, -u options
	vconftool set -t string db/browsersetting/Homepage "Recently visited site" # "Recently visited site", "user set site", "Empty page"
	vconftool set -t string db/browsersetting/HomepageMode "MOST_VISITED_SITES"
	vconftool set -t string db/browsersetting/UserHomepage "www.tizen.org" # default site is www.tizen.org
	vconftool set -t string db/browsersetting/UserAgent "Tizen"
	vconftool set -t string db/browsersetting/DefaultViewLevel "Readable" # "Readable", "Fit to width"
	vconftool set -t bool db/browsersetting/EnableLandscape 1
	vconftool set -t bool db/browsersetting/RunJavaScript 1
	vconftool set -t bool db/browsersetting/DisplayImages 1
	vconftool set -t bool db/browsersetting/BlockPopup 1
	vconftool set -t string db/browsersetting/AutoSaveIDPassword "Always ask" # "On", "Off"
	vconftool set -t string db/browsersetting/SaveIDPassword "ALWAYS_ASK"
	vconftool set -t string db/browsersetting/CustomUserAgent ""
	# set default vconf value for reader
	vconftool set -t bool db/browsersetting/RunReader 1
	vconftool set -t int db/browsersetting/FontSize 16
	# set default vconf value for plugins
	vconftool set -t bool db/browsersetting/RunPlugins 1
	vconftool set -t bool db/browsersetting/RunFlash 0
	# set search vconf
	vconftool set -t string db/browsersetting/SearchEngine "Google" # "Google", "Yahoo", "Bing"
	vconftool set -t string db/browsersetting/SearchUrl "http://www.google.com/m/search?q="
	vconftool set -t bool db/browsersetting/SearchCaseSensitive 0
	# privacy
	vconftool set -t string db/browsersetting/CookieOption "Accept all"
	vconftool set -t bool db/browsersetting/CookieOptionInt 1
	# performance
	vconftool set -t bool db/browsersetting/FastRendering 1
	vconftool set -t bool db/browsersetting/LargeRenderingBuffer 1
	vconftool set -t bool db/browsersetting/AcceleratedComposition 1
	vconftool set -t bool db/browsersetting/SamsungAppsInstall 0
	vconftool set -t bool db/browsersetting/ExternalVideoPlayer 0 # if the AcceleratedComposition is 0, this must be 0
	vconftool set -t bool db/browsersetting/CompositedRenderLayerBorders 0 # if the AcceleratedComposition is 0, this must be 0
	vconftool set -t bool db/browsersetting/PhysicsEngine 0
	vconftool set -t bool db/browsersetting/RecordingSurface 0
	vconftool set -t bool db/browsersetting/RemoteWebInspector 0
	vconftool set -t bool db/browsersetting/DemoMode 0
	vconftool set -t bool db/browsersetting/DemoSetting 0
fi

%files
/opt/apps/org.tizen.browser/bin/browser
%dir %attr(-,inhouse,inhouse) /opt/apps/org.tizen.browser/data
%dir %attr(-,inhouse,inhouse) /opt/apps/org.tizen.browser/data/db
/opt/apps/org.tizen.browser/data/screenshots/default_0
/opt/apps/org.tizen.browser/data/screenshots/default_1
/opt/apps/org.tizen.browser/data/screenshots/default_2
/opt/apps/org.tizen.browser/data/screenshots/default_3
/opt/apps/org.tizen.browser/res/html/54_bg.png
/opt/apps/org.tizen.browser/res/html/EmbedInHtml.html
/opt/apps/org.tizen.browser/res/html/default_page.html
/opt/apps/org.tizen.browser/res/html/logo.png
/opt/apps/org.tizen.browser/res/html/notFoundPage.html
/opt/apps/org.tizen.browser/res/icons/default/small/org.tizen.browser.png
/opt/apps/org.tizen.browser/res/locale/*/*/browser.mo
/opt/apps/org.tizen.browser/res/images/*
/opt/apps/org.tizen.browser/res/edje/*
/opt/share/applications/*
