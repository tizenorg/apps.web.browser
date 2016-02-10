/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "browser_config.h"

#include <ewk_chromium.h>
#include <boost/any.hpp>
#include <memory>
#include <algorithm>
#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_Wayland.h>
#include <Edje.h>
#include <Evas.h>
#include <app.h>
#include "Config.h"
#include "app_i18n.h"
#include "TabService.h"
#include "BrowserLogger.h"
#include "ServiceManager.h"
#include "AbstractWebEngine.h"
#include "TabId.h"
#include "Tools/EflTools.h"
#include "BrowserImage.h"
#include "SimpleUI.h"
#include "WebPageUIStatesManager.h"
#include "BookmarkItem.h"
#include "Tools/EflTools.h"
#include "BrowserImage.h"
#include "HistoryItem.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "SessionStorage.h"
#include "DetailPopup.h"
#include "UrlHistoryList/UrlHistoryList.h"
#include "NotificationPopup.h"


namespace tizen_browser{
namespace base_ui{

//EXPORT_SERVICE(SimpleUI, "org.tizen.browser.simpleui")

const std::string HomePageURL = "about:home";
const std::string ResetBrowserPopupMsg = "Do you really want to reset browser?" \
                                         " If you press reset, delete all data" \
                                         " and return to initial setting.";

SimpleUI& SimpleUI::getInstance()
{
    static SimpleUI instance;
    return instance;
}

SimpleUI::SimpleUI()
    : AbstractMainWindow()
//    , m_webPageUI()
//    , m_moreMenuUI()
#if PROFILE_MOBILE
//    , m_bookmarkFlowUI()
//    , m_findOnPageUI()
#endif
//    , m_bookmarkManagerUI()
//    , m_quickAccess()
//    , m_historyUI()
//    , m_settingsUI()
//    , m_tabUI()
    , m_initialised(false)
    , m_tabLimit(0)
    , m_favoritesLimit(0)
    , m_wvIMEStatus(false)
#if PROFILE_MOBILE
    , m_current_angle(0)
    , m_temp_angle(0)
    , m_rotation_transit(nullptr)
#endif
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_init(0, nullptr);

    main_window = elm_win_util_standard_add("browserApp", "browserApp");
    elm_win_conformant_set(main_window, EINA_TRUE);
    if (main_window == nullptr)
        BROWSER_LOGE("Failed to create main window");


    setMainWindow(main_window);
    ViewManager::getInstance().init(main_window);

    elm_win_resize_object_add(main_window, ViewManager::getInstance().getContent());
    evas_object_show(main_window);

#if PROFILE_MOBILE
    app_event_handler_h rotation_handler;
    ui_app_add_event_handler(&rotation_handler, APP_EVENT_DEVICE_ORIENTATION_CHANGED,
                             __orientation_changed, this);
#endif
}

SimpleUI::~SimpleUI() {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_del(m_window.get());
}

void SimpleUI::destroyUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    basic_webengine::webengine_service::WebEngineService::getInstance().destroyTabs();
}

std::string SimpleUI::edjePath(const std::string &file)
{
    return std::string(EDJE_DIR) + file;
}

std::shared_ptr<services::HistoryItemVector> SimpleUI::getMostVisitedItems()
{
    return services::HistoryService::getInstance().getMostVisitedHistoryItems();
}

std::shared_ptr<services::HistoryItemVector> SimpleUI::getHistory()
{
    return services::HistoryService::getInstance().getHistoryToday();
}

int SimpleUI::exec(const std::string& _url)
{
    BROWSER_LOGD("[%s] _url=%s, initialised=%d", __func__, _url.c_str(), m_initialised);
    std::string url = _url;

    if(!m_initialised){
        if (m_window.get()) {
            m_tabLimit = boost::any_cast <int> (tizen_browser::config::Config::getInstance().get("TAB_LIMIT"));
            m_favoritesLimit = boost::any_cast <int> (tizen_browser::config::Config::getInstance().get("FAVORITES_LIMIT"));


            loadUIServices();
            loadModelServices();

            connectModelSignals();
            connectUISignals();

            // initModelServices() needs to be called after initUIServices()
            initUIServices();
            initModelServices();

            //Push first view to stack.
            ViewManager::getInstance().pushViewToStack(m_webPageUI.get());
#if PROFILE_MOBILE
            // Register H/W back key callback
            m_platformInputManager->registerHWKeyCallback(ViewManager::getInstance().getContent());
            m_platformInputManager->registerHWKeyCallback(MoreMenuUI::getInstance().getContent());
#endif
        }
        m_currentSession = std::move(services::StorageService::getInstance().getSessionStorage().createSession());

        if (url.empty())
        {
            BROWSER_LOGD("[%s]: restore last session", __func__);
            switchViewToQuickAccess();
            restoreLastSession();
        }
        m_initialised = true;
    }

    if (!url.empty())
    {
        BROWSER_LOGD("[%s]: open new tab", __func__);
        openNewTab(url);
    }

    BROWSER_LOGD("[%s]:%d url=%s", __func__, __LINE__, url.c_str());
    return 0;
}

void SimpleUI::titleChanged(const std::string& title, const std::string& tabId)
{
    MoreMenuUI::getInstance().setWebTitle(title);
    m_webPageUI->setPageTitle(title);
    if (!basic_webengine::webengine_service::WebEngineService::getInstance().isPrivateMode(basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId())) {
        m_currentSession.updateItem(tabId, basic_webengine::webengine_service::WebEngineService::getInstance().getURI(), title);
    }
}

void SimpleUI::faviconChanged(tools::BrowserImagePtr favicon)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    services::HistoryService::getInstance().updateHistoryItemFavicon(basic_webengine::webengine_service::WebEngineService::getInstance().getURI(), favicon);

}

void SimpleUI::restoreLastSession()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(services::StorageService::getInstance());
    storage::Session lastSession = std::move(
            services::StorageService::getInstance().getSessionStorage().getLastSession());

    if (lastSession.items().size() >= 1) {
        for (auto iter = lastSession.items().begin(), end =
                lastSession.items().end(); iter != end; ++iter) {
            auto newTabId = services::TabService::getInstance().convertTabId(iter->first);
            openNewTab(iter->second.first,
                    lastSession.getUrlTitle(iter->second.first), newTabId);
        }
        services::StorageService::getInstance().getSessionStorage().deleteSession(lastSession);
    }
}


//TODO: Move all service creation here:
void SimpleUI::loadUIServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

//    m_webPageUI =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::WebPageUI,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webpageui"));

//    m_quickAccess =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::QuickAccess,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.quickaccess"));

//    m_tabUI =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::TabUI,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.tabui"));

//    m_historyUI =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::HistoryUI,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyui"));

//    m_settingsUI =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::SettingsUI,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.settingsui"));

//    m_moreMenuUI =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::MoreMenuUI,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.moremenuui"));

//    m_bookmarkDetailsUI =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::BookmarkDetailsUI,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkdetailsui"));
#if PROFILE_MOBILE
//    m_bookmarkFlowUI =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::BookmarkFlowUI,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkflowui"));

//    m_findOnPageUI =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::FindOnPageUI,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.findonpageui"));
#endif
//    m_bookmarkManagerUI =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::BookmarkManagerUI,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkmanagerui"));

//    m_zoomUI =
//        std::dynamic_pointer_cast
//        <tizen_browser::base_ui::ZoomUI, tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.zoomui"));
}

void SimpleUI::connectUISignals()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    ViewManager::getInstance().isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
#endif

    M_ASSERT(m_webPageUI.get());
    m_webPageUI->getURIEntry().uriChanged.connect(boost::bind(&SimpleUI::filterURL, this, _1));
    m_webPageUI->getURIEntry().uriEntryEditingChangedByUser.connect(boost::bind(&SimpleUI::onURLEntryEditedByUser, this, _1));
    m_webPageUI->getUrlHistoryList()->openURLInNewTab.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1));
    m_webPageUI->getUrlHistoryList()->uriChanged.connect(boost::bind(&SimpleUI::filterURL, this, _1));
    m_webPageUI->backPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
//    m_webPageUI->backPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::back, &basic_webengine::webengine_service::WebEngineService::getInstance()));
    m_webPageUI->reloadPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webPageUI->showTabUI.connect(boost::bind(&SimpleUI::showTabUI, this));
    m_webPageUI->showMoreMenu.connect(boost::bind(&SimpleUI::showMoreMenu, this));
//    m_webPageUI->forwardPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::forward, &basic_webengine::webengine_service::WebEngineService::getInstance()));
//    m_webPageUI->stopLoadingPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::stopLoading, &basic_webengine::webengine_service::WebEngineService::getInstance()));
//    m_webPageUI->reloadPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::reload, &basic_webengine::webengine_service::WebEngineService::getInstance()));
    m_webPageUI->showQuickAccess.connect(boost::bind(&SimpleUI::showQuickAccess, this));
    m_webPageUI->hideQuickAccess.connect(boost::bind(&QuickAccess::hideUI, &QuickAccess::getInstance()));
    m_webPageUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
#if PROFILE_MOBILE
//    m_webPageUI->setWebViewTouchEvents.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::setTouchEvents, &basic_webengine::webengine_service::WebEngineService::getInstance(), _1));
    m_webPageUI->hideMoreMenu.connect(boost::bind(&SimpleUI::closeMoreMenu, this));
    m_webPageUI->getURIEntry().mobileEntryFocused.connect(boost::bind(&WebPageUI::mobileEntryFocused, m_webPageUI));
    m_webPageUI->getURIEntry().mobileEntryUnfocused.connect(boost::bind(&WebPageUI::mobileEntryUnfocused, m_webPageUI));
    m_webPageUI->qaOrientationChanged.connect(boost::bind(&QuickAccess::orientationChanged, &QuickAccess::getInstance()));
#endif

    M_ASSERT(&QuickAccess::getInstance());
    QuickAccess::getInstance().getDetailPopup().openURLInNewTab.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, _2));
    QuickAccess::getInstance().openURLInNewTab.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, _2));
    QuickAccess::getInstance().mostVisitedTileClicked.connect(boost::bind(&SimpleUI::onMostVisitedTileClicked, this, _1, _2));
    QuickAccess::getInstance().getMostVisitedItems.connect(boost::bind(&SimpleUI::onMostVisitedClicked, this));
    QuickAccess::getInstance().getBookmarksItems.connect(boost::bind(&SimpleUI::onBookmarkButtonClicked, this));
    QuickAccess::getInstance().bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
    QuickAccess::getInstance().switchViewToWebPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
#if PROFILE_MOBILE
    QuickAccess::getInstance().isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
#endif

    M_ASSERT(m_tabUI.get());
    TabUI::getInstance().closeTabUIClicked.connect(boost::bind(&SimpleUI::closeTabUI, this));
    TabUI::getInstance().newTabClicked.connect(boost::bind(&SimpleUI::newTabClicked, this));
    TabUI::getInstance().tabClicked.connect(boost::bind(&SimpleUI::tabClicked, this,_1));
    TabUI::getInstance().closeTabsClicked.connect(boost::bind(&SimpleUI::closeTabsClicked, this,_1));
    TabUI::getInstance().isIncognito.connect(boost::bind(&SimpleUI::isIncognito, this, _1));
#if PROFILE_MOBILE
    TabUI::getInstance().isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
    bool desktop_ua = false;
#else
    bool desktop_ua = true;
#endif
    TabUI::getInstance().newIncognitoTabClicked.connect(boost::bind(&SimpleUI::openNewTab, this, "", "", boost::none, desktop_ua, true));
    TabUI::getInstance().tabsCount.connect(boost::bind(&SimpleUI::tabsCount, this));

    M_ASSERT(&HistoryUI::getInstance());
    HistoryUI::getInstance().clearHistoryClicked.connect(boost::bind(&SimpleUI::onClearHistoryAllClicked, this));
    HistoryUI::getInstance().signalDeleteHistoryItems.connect(boost::bind(&SimpleUI::onDeleteHistoryItems, this, _1));
    HistoryUI::getInstance().closeHistoryUIClicked.connect(boost::bind(&SimpleUI::closeHistoryUI, this));
    HistoryUI::getInstance().signalHistoryItemClicked.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, _2, desktop_ua));

    M_ASSERT(&SettingsUI::getInstance());
    Settings::getInstance().closeSettingsUIClicked.connect(boost::bind(&SimpleUI::closeSettingsUI, this));
    Settings::getInstance().deleteSelectedDataClicked.connect(boost::bind(&SimpleUI::settingsDeleteSelectedData, this,_1));
    Settings::getInstance().resetMostVisitedClicked.connect(boost::bind(&SimpleUI::settingsResetMostVisited, this));
    Settings::getInstance().resetBrowserClicked.connect(boost::bind(&SimpleUI::settingsResetBrowser, this));
#if PROFILE_MOBILE
//    Settings::getInstance().getWebEngineSettingsParam.connect(boost::bind(&basic_webengine::AbstractWebEngine<Evas_Object>::getSettingsParam, &basic_webengine::webengine_service::WebEngineService::getInstance(), _1));
//    Settings::getInstance().setWebEngineSettingsParam.connect(boost::bind(&basic_webengine::AbstractWebEngine<Evas_Object>::setSettingsParam, &basic_webengine::webengine_service::WebEngineService::getInstance(), _1, _2));
    Settings::getInstance().setWebEngineSettingsParam.connect(boost::bind(&storage::SettingsStorage::setParam, &services::StorageService::getInstance().getSettingsStorage(), _1, _2));
    Settings::getInstance().isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
#endif

    M_ASSERT(&MoreMenuUI::getInstance());
    MoreMenuUI::getInstance().bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
    MoreMenuUI::getInstance().historyUIClicked.connect(boost::bind(&SimpleUI::showHistoryUI, this));
    MoreMenuUI::getInstance().settingsClicked.connect(boost::bind(&SimpleUI::showSettingsUI, this));
    MoreMenuUI::getInstance().closeMoreMenuClicked.connect(boost::bind(&SimpleUI::closeMoreMenu, this));
    MoreMenuUI::getInstance().switchToMobileMode.connect(boost::bind(&SimpleUI::switchToMobileMode, this));
    MoreMenuUI::getInstance().switchToDesktopMode.connect(boost::bind(&SimpleUI::switchToDesktopMode, this));
    MoreMenuUI::getInstance().isBookmark.connect(boost::bind(&SimpleUI::checkBookmark, this));
    MoreMenuUI::getInstance().zoomUIClicked.connect(boost::bind(&SimpleUI::showZoomUI, this));
    MoreMenuUI::getInstance().bookmarkFlowClicked.connect(boost::bind(&SimpleUI::showBookmarkFlowUI, this, _1));
#if PROFILE_MOBILE
    MoreMenuUI::getInstance().findOnPageClicked.connect(boost::bind(&SimpleUI::showFindOnPageUI, this));
    MoreMenuUI::getInstance().isRotated.connect(boost::bind(&SimpleUI::isLandscape, this));
    m_webPageUI->isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
#endif

    M_ASSERT(BookmarkDetailsUI::getInstance());
    BookmarkDetailsUI::getInstance().bookmarkItemClicked.connect(boost::bind(&SimpleUI::onBookmarkClicked, this, _1));
    BookmarkDetailsUI::getInstance().closeBookmarkDetailsClicked.connect(boost::bind(&SimpleUI::closeBookmarkDetailsUI, this));
#if PROFILE_MOBILE
    BookmarkDetailsUI::getInstance().editFolderButtonClicked.connect(boost::bind(&SimpleUI::onEditFolderClicked, this, _1));
    BookmarkDetailsUI::getInstance().deleteFolderButtonClicked.connect(boost::bind(&SimpleUI::onDeleteFolderClicked, this, _1));
    BookmarkDetailsUI::getInstance().removeFoldersButtonClicked.connect(boost::bind(&SimpleUI::onRemoveFoldersClicked, this, _1));

    M_ASSERT(&BookmarkFlowUI::getInstance());
    BookmarkFlowUI::getInstance().addFolder.connect(boost::bind(&SimpleUI::onNewFolderClicked, this));
    BookmarkFlowUI::getInstance().closeBookmarkFlowClicked.connect(boost::bind(&SimpleUI::closeBookmarkFlowUI, this));
    BookmarkFlowUI::getInstance().saveBookmark.connect(boost::bind(&SimpleUI::addBookmark, this, _1));
    BookmarkFlowUI::getInstance().editBookmark.connect(boost::bind(&SimpleUI::editBookmark, this, _1));
    BookmarkFlowUI::getInstance().removeBookmark.connect(boost::bind(&SimpleUI::deleteBookmark, this));
    BookmarkFlowUI::getInstance().isRotated.connect(boost::bind(&SimpleUI::isLandscape, this));

    M_ASSERT(m_findOnPageUI.get());
    m_findOnPageUI->closeFindOnPageUIClicked.connect(boost::bind(&SimpleUI::closeFindOnPageUI, this));
    m_findOnPageUI->startFindingWord.connect(boost::bind(&SimpleUI::findWord, this, _1));
#endif

    M_ASSERT(&BookmarkManagerUI::getInstance());
    BookmarkManagerUI::getInstance().closeBookmarkManagerClicked.connect(boost::bind(&SimpleUI::closeBookmarkManagerUI, this));
#if PROFILE_MOBILE
    BookmarkManagerUI::getInstance().newFolderItemClicked.connect(boost::bind(&SimpleUI::onNewFolderClicked, this));
    BookmarkManagerUI::getInstance().isLandscape.connect(boost::bind(&SimpleUI::isLandscape, this));
#endif

    M_ASSERT(m_zoomUI.get());
    m_zoomUI->setZoom.connect(boost::bind(&SimpleUI::setZoomFactor, this, _1));
    m_zoomUI->getZoom.connect(boost::bind(&SimpleUI::getZoomFactor, this));
    m_zoomUI->scrollView.connect(boost::bind(&SimpleUI::scrollView, this, _1, _2));
}

void SimpleUI::loadModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
//
//    m_webEngine =
//        std::dynamic_pointer_cast
//        <basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

//    m_storageService =
//        std::dynamic_pointer_cast
//        <tizen_browser::services::StorageService,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.storageservice"));
//
//    m_favoriteService =
//        std::dynamic_pointer_cast
//        <tizen_browser::interfaces::AbstractFavoriteService,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.favoriteservice"));

//    m_historyService =
//        std::dynamic_pointer_cast
//        <tizen_browser::services::HistoryService,tizen_browser::core::AbstractService>
//        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyservice"));

    m_tabService = std::dynamic_pointer_cast<
            tizen_browser::services::TabService,
            tizen_browser::core::AbstractService>(
            tizen_browser::core::ServiceManager::getInstance().getService(
                    "org.tizen.browser.tabservice"));

    m_platformInputManager =
        std::dynamic_pointer_cast
        <tizen_browser::services::PlatformInputManager,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.platforminputmanager"));
}

void SimpleUI::initUIServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(m_webPageUI.get());
    m_webPageUI->init(ViewManager::getInstance().getContent());

    M_ASSERT(&QuickAccess::getInstance());
    QuickAccess::getInstance().init(m_webPageUI->getContent());

    M_ASSERT(m_tabUI.get());
    TabUI::getInstance().init(ViewManager::getInstance().getContent());

    M_ASSERT(&HistoryUI::getInstance());
    HistoryUI::getInstance().init(ViewManager::getInstance().getContent());

    M_ASSERT(&MoreMenuUI::getInstance());
#if PROFILE_MOBILE
    MoreMenuUI::getInstance().init(m_webPageUI->getContent());
#else
    MoreMenuUI::getInstance().init(ViewManager::getInstance().getContent());
#endif

    M_ASSERT(&SettingsUI::getInstance());
    Settings::getInstance().init(ViewManager::getInstance().getContent());

    M_ASSERT(BookmarkDetailsUI::getInstance());
    BookmarkDetailsUI::getInstance().init(ViewManager::getInstance().getContent());

#if PROFILE_MOBILE
    M_ASSERT(&BookmarkFlowUI::getInstance());
    BookmarkFlowUI::getInstance().init(ViewManager::getInstance().getContent());
    BookmarkFlowUI::getInstance().setSpecialFolderId(services::StorageService::getInstance().getFoldersStorage().SpecialFolder);

    M_ASSERT(m_findOnPageUI.get());
    m_findOnPageUI->init(m_webPageUI->getContent());
#endif

    M_ASSERT(&BookmarkManagerUI::getInstance());
    BookmarkManagerUI::getInstance().init(ViewManager::getInstance().getContent());
    BookmarkManagerUI::getInstance().setFoldersId(services::StorageService::getInstance().getFoldersStorage().AllFolder, services::StorageService::getInstance().getFoldersStorage().SpecialFolder);

    M_ASSERT(m_zoomUI.get());
    m_zoomUI->init(ViewManager::getInstance().getContent());
}

void SimpleUI::initModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(&services::BookmarkService::getInstance());
    M_ASSERT(m_webPageUI->getContent());
    basic_webengine::webengine_service::WebEngineService::getInstance().init(m_webPageUI->getContent());

#if PROFILE_MOBILE
    M_ASSERT(services::StorageService::getInstance().getSettingsStorage());
    M_ASSERT(services::StorageService::getInstance().getFoldersStorage());
    services::StorageService::getInstance().getSettingsStorage().initWebEngineSettingsFromDB();
#endif

    M_ASSERT(m_favoriteService);
    services::BookmarkService::getInstance().synchronizeBookmarks();
    services::BookmarkService::getInstance().getBookmarks();

    M_ASSERT(m_platformInputManager);
    m_platformInputManager->init(m_window.get());
}

void SimpleUI::connectModelSignals()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    basic_webengine::webengine_service::WebEngineService::getInstance().uriChanged.connect(boost::bind(&SimpleUI::webEngineURLChanged, this, _1));
    basic_webengine::webengine_service::WebEngineService::getInstance().uriChanged.connect(boost::bind(&URIEntry::changeUri, &m_webPageUI->getURIEntry(), _1));
    basic_webengine::webengine_service::WebEngineService::getInstance().webViewClicked.connect(boost::bind(&URIEntry::clearFocus, &m_webPageUI->getURIEntry()));
    basic_webengine::webengine_service::WebEngineService::getInstance().backwardEnableChanged.connect(boost::bind(&WebPageUI::setBackButtonEnabled, m_webPageUI.get(), _1));
    basic_webengine::webengine_service::WebEngineService::getInstance().forwardEnableChanged.connect(boost::bind(&WebPageUI::setForwardButtonEnabled, m_webPageUI.get(), _1));
    basic_webengine::webengine_service::WebEngineService::getInstance().loadStarted.connect(boost::bind(&SimpleUI::loadStarted, this));
    basic_webengine::webengine_service::WebEngineService::getInstance().loadProgress.connect(boost::bind(&SimpleUI::progressChanged,this,_1, _2, _3));
    basic_webengine::webengine_service::WebEngineService::getInstance().loadFinished.connect(boost::bind(&SimpleUI::loadFinished, this));
    basic_webengine::webengine_service::WebEngineService::getInstance().loadStop.connect(boost::bind(&SimpleUI::loadStopped, this));
    basic_webengine::webengine_service::WebEngineService::getInstance().loadError.connect(boost::bind(&SimpleUI::loadError, this));
    basic_webengine::webengine_service::WebEngineService::getInstance().ready.connect(boost::bind(&SimpleUI::webEngineReady, this, _1));
    basic_webengine::webengine_service::WebEngineService::getInstance().confirmationRequest.connect(boost::bind(&SimpleUI::handleConfirmationRequest, this, _1));
    basic_webengine::webengine_service::WebEngineService::getInstance().tabCreated.connect(boost::bind(&SimpleUI::tabCreated, this));
    basic_webengine::webengine_service::WebEngineService::getInstance().checkIfCreate.connect(boost::bind(&SimpleUI::checkIfCreate, this));
    basic_webengine::webengine_service::WebEngineService::getInstance().tabClosed.connect(boost::bind(&SimpleUI::tabClosed,this,_1));
    basic_webengine::webengine_service::WebEngineService::getInstance().IMEStateChanged.connect(boost::bind(&SimpleUI::setwvIMEStatus, this, _1));
    basic_webengine::webengine_service::WebEngineService::getInstance().switchToWebPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    basic_webengine::webengine_service::WebEngineService::getInstance().titleChanged.connect(boost::bind(&SimpleUI::titleChanged, this, _1, _2));
    basic_webengine::webengine_service::WebEngineService::getInstance().favIconChanged.connect(boost::bind(&SimpleUI::faviconChanged, this, _1));
    basic_webengine::webengine_service::WebEngineService::getInstance().createTabId.connect(boost::bind(&SimpleUI::onCreateTabId, this));
    basic_webengine::webengine_service::WebEngineService::getInstance().snapshotCaptured.connect(boost::bind(&SimpleUI::onSnapshotCaptured, this, _1));

    services::BookmarkService::getInstance().bookmarkAdded.connect(boost::bind(&SimpleUI::onBookmarkAdded, this,_1));
    services::BookmarkService::getInstance().bookmarkDeleted.connect(boost::bind(&SimpleUI::onBookmarkRemoved, this, _1));

    services::HistoryService::getInstance().historyDeleted.connect(boost::bind(&SimpleUI::onHistoryRemoved, this,_1));

    services::TabService::getInstance().generateThumb.connect(boost::bind(&SimpleUI::onGenerateThumb, this, _1));

    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    m_platformInputManager->backPressed.connect(boost::bind(&SimpleUI::onBackPressed, this));
    m_platformInputManager->escapePressed.connect(boost::bind(&SimpleUI::onEscapePressed, this));
    m_platformInputManager->mouseClicked.connect(
            boost::bind(&SimpleUI::onMouseClick, this, _1, _2));
    m_platformInputManager->redPressed.connect(boost::bind(&SimpleUI::onRedKeyPressed, this));
    m_platformInputManager->yellowPressed.connect(boost::bind(&SimpleUI::onYellowKeyPressed, this));

#if PROFILE_MOBILE
//    services::StorageService::getInstance().getSettingsStorage().setWebEngineSettingsParam.connect(boost::bind(&basic_webengine::AbstractWebEngine<Evas_Object>::setSettingsParam, &basic_webengine::webengine_service::WebEngineService::getInstance(), _1, _2));
    m_platformInputManager->menuButtonPressed.connect(boost::bind(&SimpleUI::onMenuButtonPressed, this));
    basic_webengine::webengine_service::WebEngineService::getInstance().registerHWKeyCallback.connect(boost::bind(&SimpleUI::registerHWKeyCallback, this));
    basic_webengine::webengine_service::WebEngineService::getInstance().unregisterHWKeyCallback.connect(boost::bind(&SimpleUI::unregisterHWKeyCallback, this));
#endif
}

#if PROFILE_MOBILE
void SimpleUI::registerHWKeyCallback()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_platformInputManager->registerHWKeyCallback(basic_webengine::webengine_service::WebEngineService::getInstance().getLayout());
}

void SimpleUI::unregisterHWKeyCallback()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_platformInputManager->unregisterHWKeyCallback(basic_webengine::webengine_service::WebEngineService::getInstance().getLayout());
}
#endif


void SimpleUI::switchViewToWebPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(basic_webengine::webengine_service::WebEngineService::getInstance().isSuspended())
        basic_webengine::webengine_service::WebEngineService::getInstance().resume();
    m_webPageUI->switchViewToWebPage(basic_webengine::webengine_service::WebEngineService::getInstance().getLayout(), basic_webengine::webengine_service::WebEngineService::getInstance().getURI(), basic_webengine::webengine_service::WebEngineService::getInstance().getTitle());
    m_webPageUI->toIncognito(basic_webengine::webengine_service::WebEngineService::getInstance().isPrivateMode(basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId()));
}

void SimpleUI::switchToTab(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId() != tabId) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        basic_webengine::webengine_service::WebEngineService::getInstance().switchToTab(tabId);
    }
    if(basic_webengine::webengine_service::WebEngineService::getInstance().isLoadError()){
        BROWSER_LOGD("[%s:%d] LOAD ERROR!", __PRETTY_FUNCTION__, __LINE__);
        loadError();
    return;
    }
    BROWSER_LOGD("[%s:%d] swiching to web_view ", __PRETTY_FUNCTION__, __LINE__);
    switchViewToWebPage();
}

void SimpleUI::showQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    QuickAccess::getInstance().showUI();
}

void SimpleUI::switchViewToQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webPageUI->switchViewToQuickAccess(QuickAccess::getInstance().getContent());
    basic_webengine::webengine_service::WebEngineService::getInstance().disconnectCurrentWebViewSignals();
    ViewManager::getInstance().popStackTo(m_webPageUI.get());
}

void SimpleUI::switchViewToIncognitoPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->switchViewToIncognitoPage();
    ViewManager::getInstance().popStackTo(m_webPageUI.get());
}

void SimpleUI::openNewTab(const std::string &uri, const std::string& title,
        const boost::optional<int> adaptorId, bool desktopMode,
        bool incognitoMode)
{
    BROWSER_LOGD("[%s:%d] uri =%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    tizen_browser::basic_webengine::TabId tab = basic_webengine::webengine_service::WebEngineService::getInstance().addTab(uri,
            nullptr, adaptorId, title, desktopMode, incognitoMode);
    switchToTab(tab);
    m_webPageUI->toIncognito(incognitoMode);
    incognitoMode ? switchViewToIncognitoPage() : m_currentSession.updateItem(tab.toString(), uri, title);
}

void SimpleUI::closeTab()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto tabId = basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId();
    closeTab(tabId);
}

void SimpleUI::closeTab(const tizen_browser::basic_webengine::TabId& id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_currentSession.removeItem(id.toString());
    services::TabService::getInstance().clearThumb(id);
    basic_webengine::webengine_service::WebEngineService::getInstance().closeTab(id);
    updateView();
}

bool SimpleUI::checkBookmark()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_webPageUI->stateEquals(WPUState::QUICK_ACCESS))
        return false;

    if(services::BookmarkService::getInstance().bookmarkExists(basic_webengine::webengine_service::WebEngineService::getInstance().getURI())) {
        BROWSER_LOGD("[%s] There is bookmark for this site [%s], set indicator on!", __func__, basic_webengine::webengine_service::WebEngineService::getInstance().getURI().c_str());
        return true;
    }
    else {
        BROWSER_LOGD("[%s] There is no bookmark for this site [%s], set indicator off", __func__, basic_webengine::webengine_service::WebEngineService::getInstance().getURI().c_str());
        return false;
    }
    return false;
}
// Consider removing these functions
void SimpleUI::onBookmarkAdded(std::shared_ptr<tizen_browser::services::BookmarkItem>)
{
    if (m_moreMenuUI) {
        MoreMenuUI::getInstance().changeBookmarkStatus(true);
        MoreMenuUI::getInstance().createToastPopup( (std::string(basic_webengine::webengine_service::WebEngineService::getInstance().getTitle()) + std::string(" added to bookmark")).c_str() );
    }
}

void SimpleUI::onBookmarkRemoved(const std::string& uri)
{
    BROWSER_LOGD("[%s] deleted %s", __func__, uri.c_str());
    if (m_moreMenuUI) {
        MoreMenuUI::getInstance().changeBookmarkStatus(false);
        MoreMenuUI::getInstance().createToastPopup( (std::string(basic_webengine::webengine_service::WebEngineService::getInstance().getTitle()) + std::string(" removed from bookmark")).c_str() );
    }
}

void SimpleUI::onOpenURLInNewTab(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, bool desktopMode)
{
    onOpenURLInNewTab(historyItem->getUrl(), historyItem->getTitle(), desktopMode);
}

void SimpleUI::onOpenURLInNewTab(const std::string& url)
{
    // TODO: desktop mode should be checked in WebView or QuickAcces
    // (depends on which view is active)
    onOpenURLInNewTab(url, "", QuickAccess::getInstance().isDesktopMode());
}

void SimpleUI::onOpenURLInNewTab(const std::string& url, const std::string& title, bool desktopMode)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ViewManager::getInstance().popStackTo(m_webPageUI.get());
    openNewTab(url, title, boost::none, desktopMode);
}

void SimpleUI::onMostVisitedTileClicked(std::shared_ptr< services::HistoryItem > historyItem, int itemsNumber)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    QuickAccess::getInstance().openDetailPopup(historyItem, services::HistoryService::getInstance().getHistoryItemsByURL(historyItem->getUrl(), itemsNumber));
}

void SimpleUI::onClearHistoryAllClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    services::HistoryService::getInstance().clearAllHistory();
}

void SimpleUI::onDeleteHistoryItems(
        std::shared_ptr<const std::vector<int>> itemIds)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (const int& id : *itemIds)
        services::HistoryService::getInstance().deleteHistoryItem(id);
}

void SimpleUI::onMostVisitedClicked()
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   QuickAccess::getInstance().setMostVisitedItems(getMostVisitedItems());
}

void SimpleUI::onBookmarkButtonClicked()
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   QuickAccess::getInstance().setBookmarksItems(services::BookmarkService::getInstance().getBookmarks(-1));
}

void SimpleUI::onBookmarkClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ViewManager::getInstance().popStackTo(m_webPageUI.get());
    std::string bookmarkAddress = bookmarkItem->getAddress();

    if(tabsCount() == 0 || !basic_webengine::webengine_service::WebEngineService::getInstance().isPrivateMode(basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId())){
        openNewTab(bookmarkAddress);
    }
    else {
        std::string bookmarkTitle = bookmarkItem->getTitle();
        m_webPageUI->switchViewToWebPage(basic_webengine::webengine_service::WebEngineService::getInstance().getLayout(), bookmarkAddress, bookmarkTitle);
        basic_webengine::webengine_service::WebEngineService::getInstance().setURI(bookmarkAddress);
        m_webPageUI->setPageTitle(bookmarkTitle);
        m_webPageUI->getURIEntry().clearFocus();
        closeBookmarkManagerUI();
    }
}

void SimpleUI::onNewFolderClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    InputPopup *inputPopup = InputPopup::createPopup(ViewManager::getInstance().getContent(), "New Folder", "Add New Folder?",
                                                          "New Folder #", _("IDS_BR_OPT_ADD"), _("IDS_BR_SK_CANCEL_ABB"), true);
#else
    InputPopup *inputPopup = InputPopup::createPopup(ViewManager::getInstance().getContent(), "New Folder", "Add new folder for adding to bookmark?",
                                                          "Folder #", _("IDS_BR_SK_CANCEL_ABB"), "Add to bookmark", false);
#endif
    services::SharedBookmarkFolderList badWords = services::StorageService::getInstance().getFoldersStorage().getFolders();
    for (auto it = badWords.begin(); it != badWords.end(); ++it)
        inputPopup->addBadWord((*it)->getName());
    inputPopup->button_clicked.connect(boost::bind(&SimpleUI::onNewFolderPopupClick, this, _1));
    inputPopup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    inputPopup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    inputPopup->show();
}

void SimpleUI::onNewFolderPopupClick(const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (services::StorageService::getInstance().getFoldersStorage().ifFolderExists(folder_name)) {
        BROWSER_LOGD("[%s:%d] Folder already exists.", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    unsigned int id = services::StorageService::getInstance().getFoldersStorage().addFolder(folder_name);
#if PROFILE_MOBILE
    SharedBookmarkFolder folder = services::StorageService::getInstance().getFoldersStorage().getFolder(id);
    SharedBookmarkFolderList list;
    list.push_back(folder);
    if (ViewManager::getInstance().topOfStack() == &BookmarkManagerUI::getInstance()) {
        BookmarkManagerUI::getInstance().addCustomFolders(list);
    } else if (ViewManager::getInstance().topOfStack() == &BookmarkFlowUI::getInstance()) {
        BookmarkFlowUI::getInstance().addCustomFolders(list);
        BookmarkFlowUI::getInstance().setFolder(folder->getId(), folder->getName());
    }
#else
    BookmarkUpdate update;
    update.folder_id = id;
    M_UNUSED(id);
    addBookmark(update);
#endif
}
#if PROFILE_MOBILE
void SimpleUI::onEditFolderClicked(const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = InputPopup::createPopup(ViewManager::getInstance().getContent(), "Edit Folder name", "Edit folder name?",
                                                        folder_name, _("IDS_BR_SK_DONE"), _("IDS_BR_SK_CANCEL_ABB"), true);
    services::SharedBookmarkFolderList badWords = services::StorageService::getInstance().getFoldersStorage().getFolders();
    for (auto it = badWords.begin(); it != badWords.end(); ++it)
        inputPopup->addBadWord((*it)->getName());
    inputPopup->button_clicked.connect(boost::bind(&SimpleUI::onEditFolderPopupClicked, this, _1));
    inputPopup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    inputPopup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    m_folder_name = folder_name;
    inputPopup->show();
}

void SimpleUI::onDeleteFolderClicked(const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TextPopup* popup = TextPopup::createPopup(ViewManager::getInstance().getContent());
    popup->setRightButton(DELETE);
    popup->setLeftButton(CANCEL);
    popup->setTitle(_("IDS_BR_SK_DELETE"));
    popup->setMessage("<b>Delete '" + folder_name + "'?</b><br>If you delete this Folder, All Bookmarks in the folder will also be deleted.");
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteFolderPopupClicked, this, _1));
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    m_folder_name = folder_name;
    popup->show();
}

void SimpleUI::onRemoveFoldersClicked(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem>> items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (auto it = items.begin(); it != items.end(); ++it) {
        services::StorageService::getInstance().getFoldersStorage().removeNumberInFolder((*it)->getDir());
        services::BookmarkService::getInstance().deleteBookmark((*it)->getAddress());
    }
}

void SimpleUI::onEditFolderPopupClicked(const std::string& newName)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (services::StorageService::getInstance().getFoldersStorage().ifFolderExists(m_folder_name)) {
        unsigned int id = services::StorageService::getInstance().getFoldersStorage().getFolderId(m_folder_name);
        services::StorageService::getInstance().getFoldersStorage().updateFolderName(id, newName);
        BookmarkDetailsUI::getInstance().onBackPressed();
    }
}

void SimpleUI::onDeleteFolderPopupClicked(PopupButtons button)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (button == DELETE && services::StorageService::getInstance().getFoldersStorage().ifFolderExists(m_folder_name)) {
        unsigned int id = services::StorageService::getInstance().getFoldersStorage().getFolderId(m_folder_name);
        onRemoveFoldersClicked(services::BookmarkService::getInstance().getBookmarks(id));
        services::StorageService::getInstance().getFoldersStorage().deleteFolder(id);
        BookmarkDetailsUI::getInstance().onBackPressed();
    }
}
#endif

void SimpleUI::onSnapshotCaptured(std::shared_ptr<tizen_browser::tools::BrowserImage> snapshot)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    services::TabService::getInstance().onThumbGenerated(basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId(), snapshot);
}

void SimpleUI::onGenerateThumb(basic_webengine::TabId tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    const int THUMB_WIDTH = boost::any_cast<int>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::TABSERVICE_THUMB_WIDTH));
    const int THUMB_HEIGHT = boost::any_cast<int>(
            tizen_browser::config::Config::getInstance().get(CONFIG_KEY::TABSERVICE_THUMB_HEIGHT));
    tools::BrowserImagePtr snapshotImage = basic_webengine::webengine_service::WebEngineService::getInstance().getSnapshotData(tabId, THUMB_WIDTH, THUMB_HEIGHT, false);
    services::TabService::getInstance().onThumbGenerated(tabId, snapshotImage);
}

void SimpleUI::onCreateTabId()
{
    int id = services::TabService::getInstance().createTabId();
    basic_webengine::webengine_service::WebEngineService::getInstance().onTabIdCreated(id);
}

void SimpleUI::onHistoryRemoved(const std::string& uri)
{
    BROWSER_LOGD("[%s] deleted %s", __func__, uri.c_str());
}

void SimpleUI::onReturnPressed(MenuButton *m)
{
    BROWSER_LOGD("[%s]", __func__);
    m_platformInputManager->returnPressed.disconnect_all_slots();
    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    m->hidePopup();
}

void SimpleUI::setwvIMEStatus(bool status)
{
    BROWSER_LOGD("[%s]", __func__);
    m_wvIMEStatus = status;
}

void SimpleUI::onBackPressed()
{
    BROWSER_LOGD("[%s]", __func__);
#if !PROFILE_MOBILE
    if (m_zoomUI->isVisible()) {
        m_zoomUI->escapeZoom();
    } else
#endif
    if (m_wvIMEStatus) {    // if IME opened
        return;
    } else if (m_popupVector.size() > 0) {
        m_popupVector.back()->onBackPressed();
#if PROFILE_MOBILE
    } else if (evas_object_visible_get(MoreMenuUI::getInstance().getContent())) {
        MoreMenuUI::getInstance().hideUI();
#endif
    } else if ((ViewManager::getInstance().topOfStack() == m_tabUI.get()) && TabUI::getInstance().isEditMode()) {
        TabUI::getInstance().onBackKey();
    } else if ((ViewManager::getInstance().topOfStack() == &BookmarkDetailsUI::getInstance())) {
        BookmarkDetailsUI::getInstance().onBackPressed();
    } else if (ViewManager::getInstance().topOfStack() == &BookmarkManagerUI::getInstance()) {
        ViewManager::getInstance().popTheStack();
    } else if (ViewManager::getInstance().topOfStack() == nullptr) {
        switchViewToQuickAccess();
    } else if ((ViewManager::getInstance().topOfStack() == m_webPageUI.get())) {
        if (m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
            if (QuickAccess::getInstance().canBeBacked(basic_webengine::webengine_service::WebEngineService::getInstance().tabsCount())) {
                QuickAccess::getInstance().backButtonClicked();
            } else {
                ui_app_exit();
            }
        } else {
            basic_webengine::webengine_service::WebEngineService::getInstance().backButtonClicked();
        }
#if PROFILE_MOBILE
    } else if ((ViewManager::getInstance().topOfStack() == &SettingsUI::getInstance()) && Settings::getInstance().isSubpage()) {
        Settings::getInstance().onBackKey();
#endif
    } else {
        ViewManager::getInstance().popTheStack();
    }
}

void SimpleUI::showPopup(interfaces::AbstractPopup* popup)
{
    BROWSER_LOGD("[%s]", __func__);
    m_popupVector.push_back(popup);
}

void SimpleUI::dismissPopup(interfaces::AbstractPopup* popup)
{
    BROWSER_LOGD("[%s]", __func__);
    std::vector<interfaces::AbstractPopup*>::reverse_iterator it = m_popupVector.rbegin();
    for (; it != m_popupVector.rend(); ++it) {
        if (popup == *it) {
            delete *it;
            m_popupVector.erase(--it.base());
            break;
        }
    }
}

void SimpleUI::onEscapePressed()
{
    BROWSER_LOGD("[%s]", __func__);
    m_zoomUI->escapeZoom();
}

#if PROFILE_MOBILE
void SimpleUI::onMenuButtonPressed()
{
    BROWSER_LOGD("[%s]", __func__);
    showMoreMenu();
}

void SimpleUI::__after_rotation(void *data, Elm_Transit */*transit*/)
{
    SimpleUI* simpleUI = static_cast<SimpleUI*>(data);
    simpleUI->m_rotation_transit = nullptr;
    elm_win_rotation_with_resize_set(simpleUI->main_window, simpleUI->m_current_angle);
    BookmarkDetailsUI::getInstance().setLandscape((simpleUI->m_current_angle % 180) == 0);
    simpleUI->MoreMenuUI::getInstance().resetContent();
    BookmarkFlowUI::getInstance().resetContent();
    Settings::getInstance().orientationChanged();
    BookmarkManagerUI::getInstance().orientationChanged();
    simpleUI->m_webPageUI->orientationChanged();
    simpleUI->TabUI::getInstance().orientationChanged();
    //TODO: ewk_view_orientation_send for an active web_view, when you change tab or create new one.
}

void SimpleUI::onRotation()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_rotation_transit) {
        //Situation when we get additional signal while animation is still not finished.
        elm_transit_del(m_rotation_transit);
        m_rotation_transit = nullptr;
    }

    m_rotation_transit = elm_transit_add();
    int to_degree = 0;
    int diff_degree = m_temp_angle - m_current_angle;
    elm_transit_object_add(m_rotation_transit, ViewManager::getInstance().getContent());

    if (diff_degree == 270)
        to_degree = 90;
    else if (diff_degree == -270)
        to_degree = -90;
    else
        to_degree = diff_degree * -1;

    elm_transit_effect_rotation_add(m_rotation_transit, 0, to_degree);
    elm_transit_duration_set(m_rotation_transit, 0.25);

    elm_transit_del_cb_set(m_rotation_transit, __after_rotation, this);
    elm_transit_objects_final_state_keep_set(m_rotation_transit, EINA_FALSE);
    elm_transit_go(m_rotation_transit);

    m_current_angle = m_temp_angle;
}

void SimpleUI::__orientation_changed(app_event_info_h event_info, void* data)
{
    SimpleUI* simpleUI = static_cast<SimpleUI*>(data);
    app_device_orientation_e event_angle = APP_DEVICE_ORIENTATION_0;
    app_event_get_device_orientation(event_info, &event_angle);
    if (simpleUI->m_current_angle != event_angle) {
        simpleUI->m_temp_angle = event_angle;
        BROWSER_LOGD("[%s:%d] previous angle: [%d] event angle: [%d]", __PRETTY_FUNCTION__, __LINE__,
                        simpleUI->m_current_angle, simpleUI->m_temp_angle);
        simpleUI->onRotation();
    }
}

bool SimpleUI::isLandscape()
{
    return elm_win_rotation_get(main_window) % 180;
}
#endif

void SimpleUI::reloadEnable(bool enable)
{
    m_webPageUI->setReloadButtonEnabled(enable);
}

void SimpleUI::stopEnable(bool enable)
{
    m_webPageUI->setStopButtonEnabled(enable);
}

void SimpleUI::loadStarted()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!basic_webengine::webengine_service::WebEngineService::getInstance().isPrivateMode(basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId()))
        m_currentSession.updateItem(basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId().toString(), basic_webengine::webengine_service::WebEngineService::getInstance().getURI(), basic_webengine::webengine_service::WebEngineService::getInstance().getTitle());
    services::TabService::getInstance().clearThumb(basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId());
    m_webPageUI->loadStarted();
}

void SimpleUI::progressChanged(double progress, basic_webengine::TabId id, bool first)
{
    if (first && !basic_webengine::webengine_service::WebEngineService::getInstance().isPrivateMode(id))
        services::HistoryService::getInstance().addHistoryItem(basic_webengine::webengine_service::WebEngineService::getInstance().getURI(),
                basic_webengine::webengine_service::WebEngineService::getInstance().getTitle(),
                basic_webengine::webengine_service::WebEngineService::getInstance().getFavicon(),
                                         std::make_shared<tizen_browser::tools::BrowserImage>());
    m_webPageUI->progressChanged(progress);
}

void SimpleUI::loadFinished()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->loadFinished();
}

void SimpleUI::loadStopped()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!basic_webengine::webengine_service::WebEngineService::getInstance().isPrivateMode(basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId()))
        services::HistoryService::getInstance().addHistoryItem(basic_webengine::webengine_service::WebEngineService::getInstance().getURI(),
                basic_webengine::webengine_service::WebEngineService::getInstance().getURI(),
                                         std::make_shared<tizen_browser::tools::BrowserImage>(),
                                         std::make_shared<tizen_browser::tools::BrowserImage>());
    m_webPageUI->loadStopped();
}

void SimpleUI::loadError()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->switchViewToErrorPage();
}

void SimpleUI::webEngineReady(basic_webengine::TabId id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    services::HistoryService::getInstance().updateHistoryItemSnapshot(basic_webengine::webengine_service::WebEngineService::getInstance().getURI(), basic_webengine::webengine_service::WebEngineService::getInstance().getSnapshotData(
            QuickAccess::MAX_THUMBNAIL_WIDTH, QuickAccess::MAX_THUMBNAIL_HEIGHT));
    services::TabService::getInstance().updateThumb(id);
}

void SimpleUI::filterURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] url=%s", __PRETTY_FUNCTION__, __LINE__, url.c_str());
    //check for special urls (like:  'about:home')
    //if there will be more addresses may be we should
    //create some kind of std::man<std::string url, bool *(doSomethingWithUrl)()>
    //and then just map[url]() ? basic_webengine::webengine_service::WebEngineService::getInstance().setURI(url) : /*do nothing*/;;
    if(/*url.empty() ||*/ url == HomePageURL){
        m_webPageUI->getURIEntry().changeUri("");
    } else if (!url.empty()){

    //check if url is in favorites

    //check if url is in blocked

    //no filtering

        if (m_webPageUI->stateEquals(WPUState::QUICK_ACCESS))
            openNewTab(url);
        else
            basic_webengine::webengine_service::WebEngineService::getInstance().setURI(url);

        if (basic_webengine::webengine_service::WebEngineService::getInstance().isPrivateMode(basic_webengine::webengine_service::WebEngineService::getInstance().currentTabId()) ||
                m_webPageUI->stateEquals(WPUState::MAIN_ERROR_PAGE))
            switchViewToWebPage();
    }
    m_webPageUI->getURIEntry().clearFocus();
}

void SimpleUI::onURLEntryEditedByUser(const std::shared_ptr<std::string> editedUrlPtr)
{
    string editedUrl(*editedUrlPtr);
    int historyItemsVisibleMax =
            m_webPageUI->getUrlHistoryList()->getItemsNumberMax();
    int minKeywordLength =
            m_webPageUI->getUrlHistoryList()->getKeywordLengthMin();
    std::shared_ptr<services::HistoryItemVector> result =
            services::HistoryService::getInstance().getHistoryItemsByKeywordsString(editedUrl,
                    historyItemsVisibleMax, minKeywordLength, true);
    m_webPageUI->getUrlHistoryList()->onURLEntryEditedByUser(editedUrl, result);
}

void SimpleUI::onMouseClick(int x, int y)
{
    m_webPageUI->getUrlHistoryList()->onMouseClick(x, y);
}

void SimpleUI::onRedKeyPressed()
{
    m_webPageUI->onRedKeyPressed();
}

void SimpleUI::onYellowKeyPressed()
{
    m_webPageUI->onYellowKeyPressed();
}

void SimpleUI::webEngineURLChanged(const std::string url)
{
    BROWSER_LOGD("webEngineURLChanged:%s", url.c_str());
    m_webPageUI->getURIEntry().clearFocus();
}

void SimpleUI::showZoomUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(! m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        ViewManager::getInstance().popStackTo(m_webPageUI.get());
        m_webPageUI->showTabUI.connect(boost::bind(&SimpleUI::closeZoomUI, this));
        m_webPageUI->showMoreMenu.connect(boost::bind(&SimpleUI::closeZoomUI, this));
        m_zoomUI->show(m_window.get());
    }
}

void SimpleUI::closeZoomUI()
{
    M_ASSERT(m_zoomUI);
    m_zoomUI->hideUI();
}

void SimpleUI::setZoomFactor(int level)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    basic_webengine::webengine_service::WebEngineService::getInstance().setZoomFactor(level);
}

int SimpleUI::getZoomFactor()
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, basic_webengine::webengine_service::WebEngineService::getInstance().getZoomFactor());
    return basic_webengine::webengine_service::WebEngineService::getInstance().getZoomFactor();
}

void SimpleUI::scrollView(const int& dx, const int& dy)
{
    basic_webengine::webengine_service::WebEngineService::getInstance().scrollView(dx, dy);
}

#if PROFILE_MOBILE
void SimpleUI::showFindOnPageUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_findOnPageUI);
    m_findOnPageUI->show();
}

void SimpleUI::findWord(const struct FindData& fdata)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    basic_webengine::webengine_service::WebEngineService::getInstance().findWord(fdata.input_str, fdata.forward, fdata.func, fdata.data);
}

void SimpleUI::closeFindOnPageUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_findOnPageUI);
    m_findOnPageUI->hideUI();
}
#endif

void SimpleUI::showTabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ViewManager::getInstance().pushViewToStack(m_tabUI.get());

    std::vector<basic_webengine::TabContentPtr> tabsContents =
            basic_webengine::webengine_service::WebEngineService::getInstance().getTabContents();
    services::TabService::getInstance().fillThumbs(tabsContents);
    TabUI::getInstance().addTabItems(tabsContents);
}

void SimpleUI::closeTabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (ViewManager::getInstance().topOfStack() == m_tabUI.get())
        ViewManager::getInstance().popTheStack();
}

void SimpleUI::newTabClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!checkIfCreate())
        return;

    switchViewToQuickAccess();
}

void SimpleUI::tabClicked(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ViewManager::getInstance().popStackTo(m_webPageUI.get());
    m_webPageUI->toIncognito(basic_webengine::webengine_service::WebEngineService::getInstance().isPrivateMode(tabId));
    switchToTab(tabId);
}

bool SimpleUI::isIncognito(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__, __func__);
    return basic_webengine::webengine_service::WebEngineService::getInstance().isPrivateMode(tabId);
}

void SimpleUI::closeTabsClicked(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    basic_webengine::webengine_service::WebEngineService::getInstance().closeTab(tabId);
}

int SimpleUI::tabsCount()
{
    return basic_webengine::webengine_service::WebEngineService::getInstance().tabsCount();
}

void SimpleUI::handleConfirmationRequest(basic_webengine::WebConfirmationPtr webConfirmation)
{
    BROWSER_LOGD("%s", __func__);
    switch(webConfirmation->getConfirmationType())
    {
        case basic_webengine::WebConfirmation::ConfirmationType::Authentication:
        {
        if (m_webPageUI->stateEquals(WPUState::MAIN_WEB_PAGE))
        {
        basic_webengine::AuthenticationConfirmationPtr auth = std::dynamic_pointer_cast<basic_webengine::AuthenticationConfirmation, basic_webengine::WebConfirmation>(webConfirmation);

        Evas_Object *popup_content = elm_layout_add(m_webPageUI->getContent());
        std::string edjFilePath = EDJE_DIR;
        edjFilePath.append("SimpleUI/AuthenticationPopup.edj");
        Eina_Bool layoutSetResult = elm_layout_file_set(popup_content, edjFilePath.c_str(), "authentication_popup");
        if (!layoutSetResult)
            throw std::runtime_error("Layout file not found: " + edjFilePath);

        elm_object_translatable_part_text_set(popup_content, "login_label", "IDS_BR_BODY_LOGIN");
        elm_object_translatable_part_text_set(popup_content, "password_label", "IDS_BR_BODY_PASSWORD");

        std::string entryTextStyle = "DEFAULT='font=Sans:style=Regular font_size=20 ellipsis=0.0'";
        Evas_Object *loginEntry = elm_entry_add(popup_content);
        elm_entry_text_style_user_push(loginEntry, entryTextStyle.c_str());
        elm_object_part_content_set(popup_content, "login", loginEntry);

        Evas_Object *passwordEntry = elm_entry_add(popup_content);
        elm_entry_password_set(passwordEntry, EINA_TRUE);
        elm_object_part_content_set(popup_content, "password", passwordEntry);

        SimplePopup *popup = SimplePopup::createPopup(ViewManager::getInstance().getContent());
        popup->setTitle("Authentication request");
        popup->addButton(OK);
        popup->addButton(CANCEL);
        popup->setContent(popup_content);
        std::shared_ptr<AuthenticationPopupData> popupData = std::make_shared<AuthenticationPopupData>();
        popupData->loginEntry = loginEntry;
        popupData->passwordEntry = passwordEntry;
        popupData->auth = auth;
        popup->setData(popupData);
        popup->buttonClicked.connect(boost::bind(&SimpleUI::authPopupButtonClicked, this, _1, _2));
        popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
        popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
        popup->show();
        break;
        }
        }
        /* no break */
        case basic_webengine::WebConfirmation::ConfirmationType::CertificateConfirmation:
        case basic_webengine::WebConfirmation::ConfirmationType::Geolocation:
        case basic_webengine::WebConfirmation::ConfirmationType::UserMedia:
        case basic_webengine::WebConfirmation::ConfirmationType::Notification:
        {
        // Implicitly accepted
        BROWSER_LOGE("NOT IMPLEMENTED: popups to confirm Ceritificate, Geolocation, UserMedia, Notification");
        webConfirmation->setResult(tizen_browser::basic_webengine::WebConfirmation::ConfirmationResult::Confirmed);
        basic_webengine::webengine_service::WebEngineService::getInstance().confirmationResult(webConfirmation);
        break;
        }

    default:
        break;
    }
}

void SimpleUI::authPopupButtonClicked(PopupButtons button, std::shared_ptr<PopupData> popupData)
{
    std::shared_ptr<AuthenticationPopupData> authPopupData = std::dynamic_pointer_cast<AuthenticationPopupData, PopupData>(popupData);
    switch(button){
        case OK:
            authPopupData->auth->setLogin(elm_entry_entry_get(authPopupData->loginEntry) ? elm_entry_entry_get(authPopupData->loginEntry) : "");
            authPopupData->auth->setPassword(elm_entry_entry_get(authPopupData->passwordEntry) ? elm_entry_entry_get(authPopupData->passwordEntry) : "");
            authPopupData->auth->setResult(basic_webengine::WebConfirmation::ConfirmationResult::Confirmed);
            basic_webengine::webengine_service::WebEngineService::getInstance().confirmationResult(authPopupData->auth);
            break;
        case CANCEL:
            authPopupData->auth->setResult(basic_webengine::WebConfirmation::ConfirmationResult::Rejected);
            basic_webengine::webengine_service::WebEngineService::getInstance().confirmationResult(authPopupData->auth);
            break;
    case YES:
    case NO:
    case CLOSE:
    case CONNECT:
        break;
    default:
        break;
    }
}

void SimpleUI::showHistoryUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
//    HistoryUI::getInstance().addHistoryItems(services::HistoryService::getInstance().getHistoryToday(),
//            HistoryPeriod::HISTORY_TODAY);
//    HistoryUI::getInstance().addHistoryItems(services::HistoryService::getInstance().getHistoryYesterday(),
//            HistoryPeriod::HISTORY_YESTERDAY);
//    HistoryUI::getInstance().addHistoryItems(services::HistoryService::getInstance().getHistoryLastWeek(),
//            HistoryPeriod::HISTORY_LASTWEEK);
//    HistoryUI::getInstance().addHistoryItems(services::HistoryService::getInstance().getHistoryLastMonth(),
//            HistoryPeriod::HISTORY_LASTMONTH);
//    ViewManager::getInstance().pushViewToStack(&HistoryUI::getInstance());
}

void SimpleUI::closeHistoryUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (ViewManager::getInstance().topOfStack() == &HistoryUI::getInstance())
        ViewManager::getInstance().popTheStack();
}

void SimpleUI::showSettingsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ViewManager::getInstance().pushViewToStack(&SettingsUI::getInstance());
}

void SimpleUI::closeSettingsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (ViewManager::getInstance().topOfStack() == &SettingsUI::getInstance())
        ViewManager::getInstance().popTheStack();
}

void SimpleUI::showMoreMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

#if PROFILE_MOBILE
    M_ASSERT(m_webPageUI);
    if (evas_object_visible_get(MoreMenuUI::getInstance().getContent()))
        MoreMenuUI::getInstance().hideUI();
    else {
        MoreMenuUI::getInstance().shouldShowFindOnPage(!basic_webengine::webengine_service::WebEngineService::getInstance().getURI().empty());
        MoreMenuUI::getInstance().blockThumbnails(m_webPageUI->stateEquals(WPUState::QUICK_ACCESS));
        basic_webengine::webengine_service::WebEngineService::getInstance().moreKeyPressed();
        MoreMenuUI::getInstance().showUI();
    }
#else
    bool desktopMode = m_webPageUI->stateEquals(WPUState::QUICK_ACCESS) ? QuickAccess::getInstance().isDesktopMode() : basic_webengine::webengine_service::WebEngineService::getInstance().isDesktopMode();
    MoreMenuUI::getInstance().setDesktopMode(desktopMode);
    ViewManager::getInstance().pushViewToStack(&MoreMenuUI::getInstance());
    MoreMenuUI::getInstance().showCurrentTab();

    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        MoreMenuUI::getInstance().setFavIcon(basic_webengine::webengine_service::WebEngineService::getInstance().getFavicon());
        MoreMenuUI::getInstance().setWebTitle(basic_webengine::webengine_service::WebEngineService::getInstance().getTitle());
        MoreMenuUI::getInstance().setURL(basic_webengine::webengine_service::WebEngineService::getInstance().getURI());
    }
    else {
        MoreMenuUI::getInstance().setHomePageInfo();
    }
#endif
    MoreMenuUI::getInstance().setIsBookmark(checkBookmark());
}

void SimpleUI::closeMoreMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

#if PROFILE_MOBILE
    M_ASSERT(m_webPageUI);
    if (evas_object_visible_get(MoreMenuUI::getInstance().getContent()))
        MoreMenuUI::getInstance().hideUI();
#else
    if (ViewManager::getInstance().topOfStack() == &MoreMenuUI::getInstance())
        ViewManager::getInstance().popTheStack();
    else
        BROWSER_LOGD("[%s:%d] WARNING!!! closeMoreMenu is not topOfStack", __PRETTY_FUNCTION__, __LINE__);
#endif
}

void SimpleUI::switchToMobileMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        basic_webengine::webengine_service::WebEngineService::getInstance().switchToMobileMode();
        ViewManager::getInstance().popStackTo(m_webPageUI.get());
        basic_webengine::webengine_service::WebEngineService::getInstance().reload();
    } else {
        QuickAccess::getInstance().setDesktopMode(false);
    }
}

void SimpleUI::switchToDesktopMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        basic_webengine::webengine_service::WebEngineService::getInstance().switchToDesktopMode();
        basic_webengine::webengine_service::WebEngineService::getInstance().reload();
    } else {
        QuickAccess::getInstance().setDesktopMode(true);
    }
}

void SimpleUI::showBookmarkFlowUI(bool state)
{
#if !PROFILE_MOBILE
    if (state) {
        deleteBookmark();
        return;
    }
#endif
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    ViewManager::getInstance().pushViewToStack(&BookmarkFlowUI::getInstance());
    std::string uri = basic_webengine::webengine_service::WebEngineService::getInstance().getURI();
    BookmarkFlowUI::getInstance().setURL(uri);
    BookmarkFlowUI::getInstance().setState(state);
    tizen_browser::services::BookmarkItem item;
    if(services::BookmarkService::getInstance().bookmarkExists(uri) && services::BookmarkService::getInstance().getItem(uri, &item))
        BookmarkFlowUI::getInstance().setTitle(item.getTitle());
    else
        BookmarkFlowUI::getInstance().setTitle(basic_webengine::webengine_service::WebEngineService::getInstance().getTitle());
    BookmarkFlowUI::getInstance().addCustomFolders(services::StorageService::getInstance().getFoldersStorage().getFolders());
    unsigned int id = state ? item.getDir() : services::StorageService::getInstance().getFoldersStorage().SpecialFolder;
    BookmarkFlowUI::getInstance().setFolder(id, services::StorageService::getInstance().getFoldersStorage().getFolderName(id));
#else
    BookmarkFlowUI *bookmarkFlow = BookmarkFlowUI::createPopup(ViewManager::getInstance().getContent());
    bookmarkFlow->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    bookmarkFlow->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    bookmarkFlow->addFolder.connect(boost::bind(&SimpleUI::onNewFolderClicked, this));
    bookmarkFlow->saveBookmark.connect(boost::bind(&SimpleUI::addBookmark, this, _1));
    bookmarkFlow->show();
    bookmarkFlow->addNewFolder();
    bookmarkFlow->addCustomFolders(services::StorageService::getInstance().getFoldersStorage().getFolders());
#endif
}
#if PROFILE_MOBILE
void SimpleUI::closeBookmarkFlowUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (ViewManager::getInstance().topOfStack() == &BookmarkFlowUI::getInstance())
        ViewManager::getInstance().popTheStack();
}
#endif

void SimpleUI::showBookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ViewManager::getInstance().pushViewToStack(&BookmarkManagerUI::getInstance());
#if PROFILE_MOBILE
    BookmarkManagerUI::getInstance().addNewFolder();
#endif
    BookmarkManagerUI::getInstance().addCustomFolders(services::StorageService::getInstance().getFoldersStorage().getFolders());

    BookmarkManagerUI::getInstance().showUI();
}


void SimpleUI::onBookmarkAllFolderClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ViewManager::getInstance().pushViewToStack(&BookmarkDetailsUI::getInstance());
    BookmarkDetailsUI::getInstance().addBookmarks(services::BookmarkService::getInstance().getBookmarks(tizen_browser::services::ALL_BOOKMARKS_ID),
            services::StorageService::getInstance().getFoldersStorage().getFolderName(services::StorageService::getInstance().getFoldersStorage().AllFolder));
    BookmarkDetailsUI::getInstance().showUI();
}

void SimpleUI::onBookmarkSpecialFolderClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ViewManager::getInstance().pushViewToStack(&BookmarkDetailsUI::getInstance());
    BookmarkDetailsUI::getInstance().addBookmarks(services::BookmarkService::getInstance().getBookmarks(services::StorageService::getInstance().getFoldersStorage().SpecialFolder),
            services::StorageService::getInstance().getFoldersStorage().getFolderName(services::StorageService::getInstance().getFoldersStorage().SpecialFolder));
    BookmarkDetailsUI::getInstance().showUI();
}

void SimpleUI::onBookmarkCustomFolderClicked(int folderId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ViewManager::getInstance().pushViewToStack(&BookmarkDetailsUI::getInstance());
    BookmarkDetailsUI::getInstance().addBookmarks(services::BookmarkService::getInstance().getBookmarks(folderId), services::StorageService::getInstance().getFoldersStorage().getFolderName(folderId));
    BookmarkDetailsUI::getInstance().showUI();
}

void SimpleUI::closeBookmarkDetailsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (ViewManager::getInstance().topOfStack() == &BookmarkDetailsUI::getInstance())
        ViewManager::getInstance().popTheStack();
    showBookmarkManagerUI();
}

void SimpleUI::closeBookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (ViewManager::getInstance().topOfStack() == &BookmarkManagerUI::getInstance())
    ViewManager::getInstance().popTheStack();
}

void SimpleUI::settingsDeleteSelectedData(const std::string& str)
{
    BROWSER_LOGD("[%s]: Deleting selected data", __func__);
    M_ASSERT(ViewManager::getInstance());
    if((str.find("CACHE")    != std::string::npos)  ||
       (str.find("COOKIES")  != std::string::npos)  ||
       (str.find("HISTORY")  != std::string::npos)  ||
       (str.find("PASSWORD") != std::string::npos)  ||
       (str.find("FORMDATA") != std::string::npos)) {
#if PROFILE_MOBILE
           TextPopup* popup = TextPopup::createPopup(ViewManager::getInstance().getContent());
           popup->setRightButton(OK);
           popup->setLeftButton(CANCEL);
#else
           SimplePopup* popup = SimplePopup::createPopup(ViewManager::getInstance().getContent());
           popup->addButton(OK);
           popup->addButton(CANCEL);
#endif
           popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteSelectedDataButton, this, _1, str));
           popup->setTitle("Delete");
           popup->setMessage("The selected web browsing data will be deleted");
           popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
           popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
           popup->show();
    }
}

void SimpleUI::onDeleteSelectedDataButton(const PopupButtons& button, const std::string& dataText)
{
    BROWSER_LOGD("[%s]: TYPE : %s", __func__, dataText.c_str());
    if(button == OK){
        NotificationPopup *popup = NotificationPopup::createNotificationPopup(ViewManager::getInstance().getContent());
        popup->show("Delete Web Browsing Data");

        if (dataText.find("CACHE") != std::string::npos)
            basic_webengine::webengine_service::WebEngineService::getInstance().clearCache();
        if (dataText.find("COOKIES") != std::string::npos)
            basic_webengine::webengine_service::WebEngineService::getInstance().clearCookies();
        if (dataText.find("HISTORY") != std::string::npos)
            services::HistoryService::getInstance().clearAllHistory();
        if (dataText.find("PASSWORD") != std::string::npos)
            basic_webengine::webengine_service::WebEngineService::getInstance().clearPasswordData();
        if (dataText.find("FORMDATA") != std::string::npos)
            basic_webengine::webengine_service::WebEngineService::getInstance().clearFormData();

        popup->dismiss();
    }
}

void SimpleUI::settingsResetMostVisited()
{
    BROWSER_LOGD("[%s]: Deleting most visited sites", __func__);
    NotificationPopup *popup = NotificationPopup::createNotificationPopup(ViewManager::getInstance().getContent());
    popup->show("Delete Web Browsing Data");
    onDeleteMostVisitedButton(nullptr);
    popup->dismiss();
}

void SimpleUI::onDeleteMostVisitedButton(std::shared_ptr< PopupData > /*popupData*/)
{
    BROWSER_LOGD("[%s]: Deleting most visited", __func__);
    services::HistoryService::getInstance().cleanMostVisitedHistoryItems();
}

void SimpleUI::settingsResetBrowser()
{
    BROWSER_LOGD("[%s]: Resetting browser", __func__);
#if PROFILE_MOBILE
    TextPopup* popup = TextPopup::createPopup(ViewManager::getInstance().getContent());
    popup->setRightButton(RESET);
    popup->setLeftButton(CANCEL);
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onResetBrowserButton, this, _1, nullptr));
#else
    SimplePopup* popup = SimplePopup::createPopup(ViewManager::getInstance().getContent());
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onResetBrowserButton, this, _1, _2));
#endif
    popup->setTitle("Reset browser");
    popup->setMessage(ResetBrowserPopupMsg);
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    popup->show();
}

void SimpleUI::onResetBrowserButton(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if (button == OK || button == RESET) {
        BROWSER_LOGD("[%s]: OK", __func__);
        BROWSER_LOGD("[%s]: Resetting browser", __func__);

        NotificationPopup *popup = NotificationPopup::createNotificationPopup(ViewManager::getInstance().getContent());
        popup->show("Reset Browser");

        basic_webengine::webengine_service::WebEngineService::getInstance().clearPrivateData();
        services::HistoryService::getInstance().clearAllHistory();
        services::BookmarkService::getInstance().deleteAllBookmarks();
        basic_webengine::webengine_service::WebEngineService::getInstance().clearPasswordData();
        basic_webengine::webengine_service::WebEngineService::getInstance().clearFormData();

        // Close all openend tabs
        std::vector<std::shared_ptr<tizen_browser::basic_webengine::TabContent>> openedTabs = basic_webengine::webengine_service::WebEngineService::getInstance().getTabContents();
        for (auto it = openedTabs.begin(); it < openedTabs.end(); ++it) {
            tizen_browser::basic_webengine::TabId id = it->get()->getId();
            m_currentSession.removeItem(id.toString());
            services::TabService::getInstance().clearThumb(id);
            basic_webengine::webengine_service::WebEngineService::getInstance().closeTab(id);
        }
        services::StorageService::getInstance().getFoldersStorage().deleteAllFolders();
        //TODO: add here any missing functionality that should be cleaned.

        popup->dismiss();
    }
}

void SimpleUI::tabLimitPopupButtonClicked(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if (button == CLOSE_TAB) {
        BROWSER_LOGD("[%s]: CLOSE TAB", __func__);
        closeTab();
    }
}

void SimpleUI::tabCreated()
{
    int tabs = basic_webengine::webengine_service::WebEngineService::getInstance().tabsCount();
    m_webPageUI->setTabsNumber(tabs);
}

bool SimpleUI::checkIfCreate()
{
    int tabs = basic_webengine::webengine_service::WebEngineService::getInstance().tabsCount();

    if (tabs >= m_tabLimit) {
        SimplePopup *popup = SimplePopup::createPopup(ViewManager::getInstance().getContent());
        popup->setTitle("Maximum tab count reached.");
        popup->addButton(OK);
        popup->setMessage("Close other tabs to open another new tab");
        popup->buttonClicked.connect(boost::bind(&SimpleUI::tabLimitPopupButtonClicked, this, _1, _2));
        popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
        popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
        popup->show();
        return false;
    }
    else
        return true;
}

void SimpleUI::updateView() {
    int tabs = basic_webengine::webengine_service::WebEngineService::getInstance().tabsCount();
    BROWSER_LOGD("[%s] Opened tabs: %d", __func__, tabs);
    if (tabs == 0) {
        switchViewToQuickAccess();
    } else if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        switchViewToWebPage();
    }
    m_webPageUI->setTabsNumber(tabs);
}

void SimpleUI::tabClosed(const tizen_browser::basic_webengine::TabId& id) {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_currentSession.removeItem(id.toString());
    services::TabService::getInstance().clearThumb(id);
    updateView();
}

void SimpleUI::searchWebPage(std::string &text, int flags)
{
    basic_webengine::webengine_service::WebEngineService::getInstance().searchOnWebsite(text, flags);
}

void SimpleUI::addBookmark(BookmarkUpdate bookmark_update)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (&services::BookmarkService::getInstance()) {
        if (&basic_webengine::webengine_service::WebEngineService::getInstance() && !basic_webengine::webengine_service::WebEngineService::getInstance().getURI().empty()) {
            services::BookmarkService::getInstance().addBookmark(basic_webengine::webengine_service::WebEngineService::getInstance().getURI(),
#if PROFILE_MOBILE
                                           bookmark_update.title,
#else
                                           basic_webengine::webengine_service::WebEngineService::getInstance().getTitle(),
#endif
                                           std::string(), basic_webengine::webengine_service::WebEngineService::getInstance().getSnapshotData(373, 240),
                                           basic_webengine::webengine_service::WebEngineService::getInstance().getFavicon(), bookmark_update.folder_id);
            services::StorageService::getInstance().getFoldersStorage().addNumberInFolder(bookmark_update.folder_id);
        }
    }
}

#if PROFILE_MOBILE
void SimpleUI::editBookmark(BookmarkUpdate bookmark_update)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (&services::BookmarkService::getInstance()) {
        if (&services::BookmarkService::getInstance() && !basic_webengine::webengine_service::WebEngineService::getInstance().getURI().empty()) {
            services::BookmarkItem oldItem;
            services::BookmarkService::getInstance().getItem(basic_webengine::webengine_service::WebEngineService::getInstance().getURI(), &oldItem);
            if (services::BookmarkService::getInstance().editBookmark(basic_webengine::webengine_service::WebEngineService::getInstance().getURI(), bookmark_update.title, bookmark_update.folder_id)) {
                services::StorageService::getInstance().getFoldersStorage().removeNumberInFolder(oldItem.getDir());
                services::StorageService::getInstance().getFoldersStorage().addNumberInFolder(bookmark_update.folder_id);
            }
        }
    }
}
#endif

//TODO: Replace by direct call.
void SimpleUI::deleteBookmark()
{
    std::string uri = basic_webengine::webengine_service::WebEngineService::getInstance().getURI();
    tizen_browser::services::BookmarkItem item;
    if (services::BookmarkService::getInstance().bookmarkExists(uri) && services::BookmarkService::getInstance().getItem(uri, &item))
        services::StorageService::getInstance().getFoldersStorage().removeNumberInFolder(item.getDir());
    services::BookmarkService::getInstance().deleteBookmark(uri);
}
}
}
