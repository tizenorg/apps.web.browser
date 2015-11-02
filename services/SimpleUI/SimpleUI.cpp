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

#if defined(USE_EWEBKIT)
#include <ewk_chromium.h>
#endif

#include <boost/any.hpp>
#include <memory>
#include <algorithm>
#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_Wayland.h>
#include <Edje.h>
#include <Evas.h>
#include "Config.h"

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
#include "SqlStorage.h"
#include "DetailPopup.h"
#include "UrlHistoryList/UrlHistoryList.h"
#include "NotificationPopup.h"


namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SimpleUI, "org.tizen.browser.simpleui")

const std::string HomePageURL = "about:home";
const int ROOT_FOLDER = 0;


SimpleUI::SimpleUI()
    : AbstractMainWindow()
    , m_popup(nullptr)
    , m_webPageUI()
    , m_moreMenuUI()
    , m_bookmarkManagerUI()
    , m_quickAccess()
    , m_historyUI()
    , m_settingsUI()
    , m_tabUI()
    , m_initialised(false)
    , m_wvIMEStatus(false)
    , m_ewkContext(ewk_context_new())
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_init(0, nullptr);

    config::DefaultConfig config;
    config.load("");
    if (config.isMobileProfile()) {
        elm_config_scale_set(boost::any_cast<double>(config.get("mobile_scale")));
    }

    Evas_Object *main_window = elm_win_util_standard_add("browserApp", "browserApp");
    if (main_window == nullptr)
        BROWSER_LOGE("Failed to create main window");

    setMainWindow(main_window);
    m_viewManager = new ViewManager(main_window);
    evas_object_size_hint_weight_set(main_window, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (main_window, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_win_resize_object_add(main_window, m_viewManager->getContent());
    evas_object_show(main_window);
}

SimpleUI::~SimpleUI() {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_sessionService->getStorage()->deleteSession(m_currentSession);
    evas_object_del(m_window.get());
    ewk_context_delete(m_ewkContext);
}

void SimpleUI::destroyUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_del(m_window.get());
}

std::string SimpleUI::edjePath(const std::string &file)
{
    return std::string(EDJE_DIR) + file;
}

std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > SimpleUI::getBookmarks(int folder_id)
{
    return m_favoriteService->getBookmarks(folder_id);
}

std::shared_ptr<services::HistoryItemVector> SimpleUI::getMostVisitedItems()
{
    return m_historyService->getMostVisitedHistoryItems();
}

std::shared_ptr<services::HistoryItemVector> SimpleUI::getHistory()
{
    return m_historyService->getHistoryToday();
}

int SimpleUI::exec(const std::string& _url)
{
    BROWSER_LOGD("[%s] _url=%s, initialised=%d", __func__, _url.c_str(), m_initialised);
    std::string url = _url;

    if(!m_initialised){
        if (m_window.get()) {
            config::DefaultConfig config;
            config.load("");
            m_tabLimit = boost::any_cast <int> (config.get("TAB_LIMIT"));
            m_favoritesLimit = boost::any_cast <int> (config.get("FAVORITES_LIMIT"));


            loadUIServices();
            loadModelServices();

            // initModelServices() needs to be called after initUIServices()
            initUIServices();
            initModelServices();


            connectModelSignals();
            connectUISignals();

            //Push first view to stack.
            m_viewManager->pushViewToStack(m_webPageUI.get());
        }
        m_initialised = true;
    }

    m_currentSession = std::move(m_sessionService->getStorage()->createSession());

    if (url.empty())
    {
        BROWSER_LOGD("[%s]: changing to homeUrl", __func__);
        switchViewToQuickAccess();
        restoreLastSession();
    } else {
        openNewTab(url);
    }

    BROWSER_LOGD("[%s]:%d url=%s", __func__, __LINE__, url.c_str());
    return 0;
}

void SimpleUI::restoreLastSession()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_sessionService);
    Session::Session lastSession = std::move(m_sessionService->getStorage()->getLastSession());
    if(lastSession.items().size() >= 1)
    {
        for(auto iter=lastSession.items().begin(), end=lastSession.items().end(); iter != end; ++iter)
        {
            openNewTab(iter->second);
        }
        m_sessionService->getStorage()->deleteSession(lastSession);
    }
}


//TODO: Move all service creation here:
void SimpleUI::loadUIServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webPageUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::WebPageUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webpageui"));

    m_quickAccess =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::QuickAccess,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.quickaccess"));

    m_tabUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::TabUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.tabui"));

    m_historyUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::HistoryUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyui"));

    m_settingsUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::SettingsUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.settingsui"));

    m_moreMenuUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::MoreMenuUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.moremenuui"));

    m_bookmarkManagerUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::BookmarkManagerUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkmanagerui"));

    m_zoomUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::ZoomUI, tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.zoomui"));
}

void SimpleUI::connectUISignals()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(m_webPageUI.get());
    m_webPageUI->getURIEntry().uriChanged.connect(boost::bind(&SimpleUI::filterURL, this, _1));
    m_webPageUI->getURIEntry().uriEntryEditingChangedByUser.connect(boost::bind(&SimpleUI::onURLEntryEditedByUser, this, _1));
    m_webPageUI->getUrlHistoryList()->openURLInNewTab.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1));
    m_webPageUI->backPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webPageUI->backPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::back, m_webEngine.get()));
    m_webPageUI->backPage.connect(boost::bind(&ZoomUI::showNavigation, m_zoomUI.get()));
    m_webPageUI->reloadPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webPageUI->showTabUI.connect(boost::bind(&SimpleUI::showTabUI, this));
    m_webPageUI->showMoreMenu.connect(boost::bind(&SimpleUI::showMoreMenu, this));
    m_webPageUI->forwardPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::forward, m_webEngine.get()));
    m_webPageUI->forwardPage.connect(boost::bind(&ZoomUI::showNavigation, m_zoomUI.get()));
    m_webPageUI->stopLoadingPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::stopLoading, m_webEngine.get()));
    m_webPageUI->reloadPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::reload, m_webEngine.get()));
    m_webPageUI->reloadPage.connect(boost::bind(&ZoomUI::showNavigation, m_zoomUI.get()));
    m_webPageUI->showQuickAccess.connect(boost::bind(&SimpleUI::showQuickAccess, this));
    m_webPageUI->hideQuickAccess.connect(boost::bind(&QuickAccess::hideUI, m_quickAccess));
    m_webPageUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
    m_webPageUI->showZoomNavigation.connect(boost::bind(&ZoomUI::showNavigation, m_zoomUI.get()));

    M_ASSERT(m_quickAccess.get());
    m_quickAccess->getDetailPopup().openURLInNewTab.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, _2));
    m_quickAccess->openURLInNewTab.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, _2));
    m_quickAccess->mostVisitedTileClicked.connect(boost::bind(&SimpleUI::onMostVisitedTileClicked, this, _1, _2));
    m_quickAccess->mostVisitedClicked.connect(boost::bind(&SimpleUI::onMostVisitedClicked, this));
    m_quickAccess->bookmarkClicked.connect(boost::bind(&SimpleUI::onBookmarkButtonClicked, this));
    m_quickAccess->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
    m_quickAccess->switchViewToWebPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));

    M_ASSERT(m_tabUI.get());
    m_tabUI->closeTabUIClicked.connect(boost::bind(&SimpleUI::closeTabUI, this));
    m_tabUI->closeTabUIClicked.connect(boost::bind(&ZoomUI::showNavigation, m_zoomUI.get()));
    m_tabUI->newTabClicked.connect(boost::bind(&SimpleUI::newTabClicked, this));
    m_tabUI->tabClicked.connect(boost::bind(&SimpleUI::tabClicked, this,_1));
    m_tabUI->closeTabsClicked.connect(boost::bind(&SimpleUI::closeTabsClicked, this,_1));
    m_tabUI->newIncognitoTabClicked.connect(boost::bind(&SimpleUI::openNewTab, this, "", false, true));
    m_tabUI->tabsCount.connect(boost::bind(&SimpleUI::tabsCount, this));

    M_ASSERT(m_historyUI.get());
    m_historyUI->clearHistoryClicked.connect(boost::bind(&SimpleUI::onClearHistoryClicked, this));
    m_historyUI->closeHistoryUIClicked.connect(boost::bind(&SimpleUI::closeHistoryUI, this));
    // desktop mode as default
    m_historyUI->historyItemClicked.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, true));

    M_ASSERT(m_settingsUI.get());
    m_settingsUI->closeSettingsUIClicked.connect(boost::bind(&SimpleUI::closeSettingsUI, this));
    m_settingsUI->deleteSelectedDataClicked.connect(boost::bind(&SimpleUI::settingsDeleteSelectedData, this,_1));
    m_settingsUI->resetMostVisitedClicked.connect(boost::bind(&SimpleUI::settingsResetMostVisited, this));
    m_settingsUI->resetBrowserClicked.connect(boost::bind(&SimpleUI::settingsResetBrowser, this));

    M_ASSERT(m_moreMenuUI.get());
    m_moreMenuUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
    m_moreMenuUI->historyUIClicked.connect(boost::bind(&SimpleUI::showHistoryUI, this));
    m_moreMenuUI->settingsClicked.connect(boost::bind(&SimpleUI::showSettingsUI, this));
    m_moreMenuUI->closeMoreMenuClicked.connect(boost::bind(&SimpleUI::closeMoreMenu, this));
    m_moreMenuUI->closeMoreMenuClicked.connect(boost::bind(&ZoomUI::showNavigation, m_zoomUI.get()));
    m_moreMenuUI->switchToMobileMode.connect(boost::bind(&SimpleUI::switchToMobileMode, this));
    m_moreMenuUI->switchToDesktopMode.connect(boost::bind(&SimpleUI::switchToDesktopMode, this));
    m_moreMenuUI->addToBookmarkClicked.connect(boost::bind(&SimpleUI::addToBookmarks, this, _1));
    m_moreMenuUI->isBookmark.connect(boost::bind(&SimpleUI::checkBookmark, this));
    m_moreMenuUI->deleteBookmark.connect(boost::bind(&SimpleUI::deleteBookmark, this));
    m_moreMenuUI->zoomUIClicked.connect(boost::bind(&SimpleUI::showZoomUI, this));

    M_ASSERT(m_bookmarkManagerUI.get());
    m_bookmarkManagerUI->closeBookmarkManagerClicked.connect(boost::bind(&SimpleUI::closeBookmarkManagerUI, this));
    m_bookmarkManagerUI->bookmarkItemClicked.connect(boost::bind(&SimpleUI::onBookmarkClicked, this, _1));

    M_ASSERT(m_zoomUI.get());
    m_zoomUI->setZoom.connect(boost::bind(&SimpleUI::setZoomFactor, this, _1));
    m_zoomUI->getZoom.connect(boost::bind(&SimpleUI::getZoomFactor, this));
    m_zoomUI->scrollView.connect(boost::bind(&SimpleUI::scrollView, this, _1, _2));
}

void SimpleUI::loadModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webEngine =
        std::dynamic_pointer_cast
        <basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    m_favoriteService =
        std::dynamic_pointer_cast
        <tizen_browser::interfaces::AbstractFavoriteService,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.favoriteservice"));

    m_historyService =
        std::dynamic_pointer_cast
        <tizen_browser::services::HistoryService,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyservice"));

    m_platformInputManager =
        std::dynamic_pointer_cast
        <tizen_browser::services::PlatformInputManager,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.platforminputmanager"));

    m_sessionService =
        std::dynamic_pointer_cast
        <tizen_browser::services::SessionStorage,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.sessionStorageService"));
}

void SimpleUI::initUIServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT (m_viewManager);

    M_ASSERT(m_webPageUI.get());
    m_webPageUI->init(m_viewManager->getContent());

    M_ASSERT(m_quickAccess.get());
    m_quickAccess->init(m_webPageUI->getContent());

    M_ASSERT(m_tabUI.get());
    m_tabUI->init(m_viewManager->getContent());

    M_ASSERT(m_historyUI.get());
    m_historyUI->init(m_viewManager->getContent());

    M_ASSERT(m_moreMenuUI.get());
    m_moreMenuUI->init(m_viewManager->getContent());

    M_ASSERT(m_settingsUI.get());
    m_settingsUI->init(m_viewManager->getContent());

    M_ASSERT(m_bookmarkManagerUI.get());
    m_bookmarkManagerUI->init(m_viewManager->getContent());

    M_ASSERT(m_zoomUI.get());
    m_zoomUI->init(m_viewManager->getContent());
}

void SimpleUI::initModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(m_webEngine);
    M_ASSERT(m_webPageUI->getContent());
    m_webEngine->init(m_webPageUI->getContent());

    M_ASSERT(m_favoriteService);
    m_favoriteService->synchronizeBookmarks();
    m_favoriteService->getBookmarks();

    M_ASSERT(m_platformInputManager);
    m_platformInputManager->init(m_window.get());
}

void SimpleUI::connectModelSignals()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webEngine->uriChanged.connect(boost::bind(&SimpleUI::webEngineURLChanged, this, _1));
    m_webEngine->uriChanged.connect(boost::bind(&URIEntry::changeUri, &m_webPageUI->getURIEntry(), _1));
    m_webEngine->webViewClicked.connect(boost::bind(&URIEntry::clearFocus, &m_webPageUI->getURIEntry()));
    m_webEngine->backwardEnableChanged.connect(boost::bind(&WebPageUI::setBackButtonEnabled, m_webPageUI.get(), _1));
    m_webEngine->forwardEnableChanged.connect(boost::bind(&WebPageUI::setForwardButtonEnabled, m_webPageUI.get(), _1));
    m_webEngine->loadStarted.connect(boost::bind(&SimpleUI::loadStarted, this));
    m_webEngine->loadProgress.connect(boost::bind(&SimpleUI::progressChanged,this,_1));
    m_webEngine->loadFinished.connect(boost::bind(&SimpleUI::loadFinished, this));
    m_webEngine->loadStop.connect(boost::bind(&SimpleUI::loadStopped, this));
    m_webEngine->loadError.connect(boost::bind(&SimpleUI::loadError, this));
    m_webEngine->confirmationRequest.connect(boost::bind(&SimpleUI::handleConfirmationRequest, this, _1));
    m_webEngine->tabCreated.connect(boost::bind(&SimpleUI::tabCreated, this));
    m_webEngine->checkIfCreate.connect(boost::bind(&SimpleUI::checkIfCreate, this));
    m_webEngine->tabClosed.connect(boost::bind(&SimpleUI::tabClosed,this,_1));
    m_webEngine->IMEStateChanged.connect(boost::bind(&SimpleUI::setwvIMEStatus, this, _1));
    m_webEngine->windowCreated.connect(boost::bind(&SimpleUI::windowCreated, this));
    m_webEngine->titleChanged.connect(boost::bind(&WebPageUI::setPageTitle, m_webPageUI.get(), _1));
    m_webEngine->switchToWebPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));

    m_favoriteService->bookmarkAdded.connect(boost::bind(&SimpleUI::onBookmarkAdded, this,_1));
    m_favoriteService->bookmarkDeleted.connect(boost::bind(&SimpleUI::onBookmarkRemoved, this, _1));

    m_historyService->historyDeleted.connect(boost::bind(&SimpleUI::onHistoryRemoved, this,_1));

    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    m_platformInputManager->backPressed.connect(boost::bind(&SimpleUI::onBackPressed, this));
    m_platformInputManager->escapePressed.connect(boost::bind(&SimpleUI::onEscapePressed, this));
    m_platformInputManager->mouseClicked.connect(
            boost::bind(&SimpleUI::onMouseClick, this));
    m_platformInputManager->redPressed.connect(boost::bind(&SimpleUI::onRedKeyPressed, this));

}

void SimpleUI::switchViewToWebPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    if(m_webEngine->isSuspended())
        m_webEngine->resume();
    m_webPageUI->switchViewToWebPage(m_webEngine->getLayout(), m_webEngine->getURI(), m_webEngine->getTitle());
    m_webPageUI->toIncognito(m_webEngine->isPrivateMode(m_webEngine->currentTabId()));
}

void SimpleUI::switchToTab(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_webEngine->currentTabId() != tabId) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        m_webEngine->switchToTab(tabId);
    }
    if(m_webEngine->isLoadError()){
        BROWSER_LOGD("[%s:%d] LOAD ERROR!", __PRETTY_FUNCTION__, __LINE__);
        loadError();
	return;
    }
    BROWSER_LOGD("[%s:%d] swiching to web_view ", __PRETTY_FUNCTION__, __LINE__);
    switchViewToWebPage();
    m_zoomUI->showNavigation();
}

void SimpleUI::showQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_quickAccess->showMostVisited(getMostVisitedItems());
    m_quickAccess->showUI();
}

void SimpleUI::switchViewToQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);

    m_webPageUI->switchViewToQuickAccess(m_quickAccess->getContent());
    m_webEngine->disconnectCurrentWebViewSignals();
    m_viewManager->popStackTo(m_webPageUI.get());
}

void SimpleUI::switchViewToIncognitoPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    m_webPageUI->toIncognito(true);
    m_webPageUI->switchViewToIncognitoPage();
    m_viewManager->popStackTo(m_webPageUI.get());
}

void SimpleUI::openNewTab(const std::string &uri, bool desktopMode, bool incognitoMode)
{
    BROWSER_LOGD("[%s:%d] uri =%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    tizen_browser::basic_webengine::TabId tab = m_webEngine->addTab(uri, nullptr, desktopMode, incognitoMode);
    switchToTab(tab);
    m_webPageUI->toIncognito(incognitoMode);
    if (incognitoMode)
        switchViewToIncognitoPage();
}

void SimpleUI::closeTab()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto tabId = m_webEngine->currentTabId();
    closeTab(tabId);
}

void SimpleUI::closeTab(const tizen_browser::basic_webengine::TabId& id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_currentSession.removeItem(id.toString());
    m_webEngine->closeTab(id);
    updateView();
}

bool SimpleUI::checkBookmark()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_webPageUI->stateEquals(WPUState::QUICK_ACCESS))
        return false;

    if(m_favoriteService->bookmarkExists(m_webEngine->getURI())) {
        BROWSER_LOGD("[%s] There is bookmark for this site [%s], set indicator on!", __func__, m_webEngine->getURI().c_str());
        return true;
    }
    else {
        BROWSER_LOGD("[%s] There is no bookmark for this site [%s], set indicator off", __func__, m_webEngine->getURI().c_str());
        return false;
    }
}
// Consider removing these functions
void SimpleUI::onBookmarkAdded(std::shared_ptr<tizen_browser::services::BookmarkItem>)
{
    if (m_moreMenuUI) {
        m_moreMenuUI->changeBookmarkStatus(true);
        m_moreMenuUI->createToastPopup( (std::string(m_webEngine->getTitle()) + std::string(" added to bookmark")).c_str() );
    }
}

void SimpleUI::onBookmarkRemoved(const std::string& uri)
{
    BROWSER_LOGD("[%s] deleted %s", __func__, uri.c_str());
    if (m_moreMenuUI) {
        m_moreMenuUI->changeBookmarkStatus(false);
        m_moreMenuUI->createToastPopup( (std::string(m_webEngine->getTitle()) + std::string(" removed from bookmark")).c_str() );
    }
}

void SimpleUI::onOpenURLInNewTab(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, bool desktopMode)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_viewManager->popStackTo(m_webPageUI.get());
    std::string historyAddress = historyItem->getUrl();
    openNewTab(historyAddress, desktopMode);
}

void SimpleUI::onOpenURLInNewTab(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem)
{
    onOpenURLInNewTab(historyItem, m_quickAccess->isDesktopMode());
}

void SimpleUI::onMostVisitedTileClicked(std::shared_ptr< services::HistoryItem > historyItem, int itemsNumber)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_quickAccess->openDetailPopup(historyItem, m_historyService->getHistoryItemsByURL(historyItem->getUrl(), itemsNumber));
}

void SimpleUI::onClearHistoryClicked()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_historyService->clearAllHistory();
}

void SimpleUI::onMostVisitedClicked()
{
   BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
   m_quickAccess->showMostVisited(getMostVisitedItems());
}

void SimpleUI::onBookmarkButtonClicked()
{
   BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
   m_quickAccess->showBookmarks(getBookmarks());
}

void SimpleUI::onBookmarkClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    m_viewManager->popStackTo(m_webPageUI.get());
    std::string bookmarkAddress = bookmarkItem->getAddress();
    if (!m_webEngine->isPrivateMode(m_webEngine->currentTabId()))
        openNewTab(bookmarkAddress);
    else {
        std::string bookmarkTitle = bookmarkItem->getTittle();
        m_webPageUI->switchViewToWebPage(m_webEngine->getLayout(), bookmarkAddress, bookmarkTitle);
        m_webEngine->setURI(bookmarkAddress);
        m_webPageUI->setPageTitle(bookmarkTitle);
        m_webPageUI->getURIEntry().clearFocus();
        closeBookmarkManagerUI();
    }
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

void SimpleUI::windowCreated()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switchViewToWebPage();
}

void SimpleUI::onBackPressed()
{
    BROWSER_LOGD("[%s]", __func__);
    if (m_zoomUI->isVisible()) {
        m_zoomUI->escapeZoom();
    } else if ((m_viewManager->topOfStack() == m_tabUI.get()) && m_tabUI->isEditMode()) {
        m_tabUI->onBackKey();
    } else if (m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_quickAccess->backButtonClicked();
    } else if (!m_webPageUI->getURIEntry().hasFocus() && !m_wvIMEStatus) {
        if ((m_viewManager->topOfStack() == m_webPageUI.get())) {
            m_webEngine->backButtonClicked();
        } else {
            m_viewManager->popTheStack();
        }
    }
}

void SimpleUI::onEscapePressed()
{
    BROWSER_LOGD("[%s]", __func__);
    m_zoomUI->escapeZoom();
}

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
    if (!m_webEngine->isPrivateMode(m_webEngine->currentTabId()))
        m_currentSession.updateItem(m_webEngine->currentTabId().toString(), m_webEngine->getURI());
    m_webPageUI->loadStarted();
}

void SimpleUI::progressChanged(double progress)
{
    m_webPageUI->progressChanged(progress);
}

void SimpleUI::loadFinished()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!m_webEngine->isPrivateMode(m_webEngine->currentTabId()))
        m_historyService->addHistoryItem(std::make_shared<tizen_browser::services::HistoryItem>(m_webEngine->getURI(),
                                                                                                m_webEngine->getTitle(),
                                                                                                m_webEngine->getFavicon()),
                                                                                                m_webEngine->getSnapshotData(
                                                                                                    QuickAccess::MAX_THUMBNAIL_WIDTH,
                                                                                                    QuickAccess::MAX_THUMBNAIL_HEIGHT));
    m_webPageUI->loadFinished();
}

void SimpleUI::loadStopped()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!m_webEngine->isPrivateMode(m_webEngine->currentTabId())) {
        m_historyService->addHistoryItem(std::make_shared<tizen_browser::services::HistoryItem>(
                                             m_webEngine->getURI(),
                                             m_webEngine->getURI(),
                                             std::make_shared<tizen_browser::tools::BrowserImage>()),
                                         std::make_shared<tizen_browser::tools::BrowserImage>());
    }
    m_webPageUI->loadStopped();
}

void SimpleUI::loadError()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->switchViewToErrorPage();
}

void SimpleUI::filterURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] url=%s", __PRETTY_FUNCTION__, __LINE__, url.c_str());
    //check for special urls (like:  'about:home')
    //if there will be more addresses may be we should
    //create some kind of std::man<std::string url, bool *(doSomethingWithUrl)()>
    //and then just map[url]() ? m_webEngine->setURI(url) : /*do nothing*/;;
    if(/*url.empty() ||*/ url == HomePageURL){
        m_webPageUI->getURIEntry().changeUri("");
    } else if (!url.empty()){

    //check if url is in favorites

    //check if url is in blocked

    //no filtering

        if (m_webPageUI->stateEquals(WPUState::QUICK_ACCESS))
            openNewTab(url);
        else
            m_webEngine->setURI(url);

        if (m_webEngine->isPrivateMode(m_webEngine->currentTabId()))
            switchViewToWebPage();
    }
    m_webPageUI->getURIEntry().clearFocus();
}

void SimpleUI::onURLEntryEditedByUser(const std::shared_ptr<std::string> editedUrlPtr)
{
    string editedUrl(*editedUrlPtr);
    int historyItemsVisibleMax =
            m_webPageUI->getUrlHistoryList()->getVisibleItemsMax();
    int minKeywordLength =
            m_webPageUI->getUrlHistoryList()->getMinKeywordLength();
    std::shared_ptr<services::HistoryItemVector> result =
            m_historyService->getHistoryItemsByKeywordsString(editedUrl,
                    historyItemsVisibleMax, minKeywordLength);
    m_webPageUI->getUrlHistoryList()->onURLEntryEditedByUser(editedUrl, result);
}

void SimpleUI::onMouseClick()
{
    m_webPageUI->getUrlHistoryList()->onMouseClick();
}

void SimpleUI::onRedKeyPressed()
{
    m_webPageUI->onRedKeyPressed();
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
        M_ASSERT(m_viewManager);
        m_viewManager->popStackTo(m_webPageUI.get());
        m_webPageUI->showTabUI.connect(boost::bind(&SimpleUI::closeZoomUI, this));
        m_webPageUI->showMoreMenu.connect(boost::bind(&SimpleUI::closeZoomUI, this));
        m_zoomUI->show(m_window.get());
    }
}

void SimpleUI::closeZoomUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_zoomUI);
    m_zoomUI->hideUI();
}

void SimpleUI::setZoomFactor(int level)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_webEngine->setZoomFactor(level);
}

int SimpleUI::getZoomFactor()
{
    BROWSER_LOGD("[%s:%d] %d", __PRETTY_FUNCTION__, __LINE__, m_webEngine->getZoomFactor());
    return m_webEngine->getZoomFactor();
}

void SimpleUI::scrollView(const int& dx, const int& dy)
{
    m_webEngine->scrollView(dx, dy);
}

void SimpleUI::showTabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    m_viewManager->pushViewToStack(m_tabUI.get());
    m_tabUI->addTabItems(m_webEngine->getTabContents());
}

void SimpleUI::closeTabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    if (m_viewManager->topOfStack() == m_tabUI.get())
        m_viewManager->popTheStack();
}

void SimpleUI::newTabClicked()
{
    if (!checkIfCreate())
        return;

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->toIncognito(false);
    switchViewToQuickAccess();
}

void SimpleUI::tabClicked(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager->popStackTo(m_webPageUI.get());
    m_webPageUI->toIncognito(m_webEngine->isPrivateMode(tabId));
    switchToTab(tabId);
}

void SimpleUI::closeTabsClicked(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webEngine->closeTab(tabId);
}

int SimpleUI::tabsCount()
{
    return m_webEngine->tabsCount();
}

void SimpleUI::handleConfirmationRequest(basic_webengine::WebConfirmationPtr webConfirmation)
{
    BROWSER_LOGD("%s", __func__);
    switch(webConfirmation->getConfirmationType())
    {
        case basic_webengine::WebConfirmation::ConfirmationType::Authentication:
        {
        basic_webengine::AuthenticationConfirmationPtr auth = std::dynamic_pointer_cast<basic_webengine::AuthenticationConfirmation, basic_webengine::WebConfirmation>(webConfirmation);

        Evas_Object *popup_content = elm_layout_add(m_webPageUI->getContent());
        std::string edjFilePath = EDJE_DIR;
        edjFilePath.append("SimpleUI/AuthenticationPopup.edj");
        Eina_Bool layoutSetResult = elm_layout_file_set(popup_content, edjFilePath.c_str(), "authentication_popup");
        if (!layoutSetResult)
            throw std::runtime_error("Layout file not found: " + edjFilePath);

    #if PLATFORM(TIZEN)
        elm_object_translatable_part_text_set(popup_content, "login_label", "Login");
        elm_object_translatable_part_text_set(popup_content, "password_label", "Password");
    #else
        elm_object_part_text_set(popup_content, "login_label", "Login");
        elm_object_part_text_set(popup_content, "password_label", "Password");
    #endif

        Evas_Object *loginEntry = elm_entry_add(popup_content);
        elm_object_part_content_set(popup_content, "login", loginEntry);

        Evas_Object *passwordEntry = elm_entry_add(popup_content);
        elm_object_part_content_set(popup_content, "password", passwordEntry);

        SimplePopup *popup = SimplePopup::createPopup();
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
        popup->show();
        break;
        }
        case basic_webengine::WebConfirmation::ConfirmationType::CertificateConfirmation:
        case basic_webengine::WebConfirmation::ConfirmationType::Geolocation:
        case basic_webengine::WebConfirmation::ConfirmationType::UserMedia:
        case basic_webengine::WebConfirmation::ConfirmationType::Notification:
        {
        // Implicitly accepted
        BROWSER_LOGE("NOT IMPLEMENTED: popups to confirm Ceritificate, Geolocation, UserMedia, Notification");
        webConfirmation->setResult(tizen_browser::basic_webengine::WebConfirmation::ConfirmationResult::Confirmed);
        m_webEngine->confirmationResult(webConfirmation);
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
            m_webEngine->confirmationResult(authPopupData->auth);
            break;
        case CANCEL:
            authPopupData->auth->setResult(basic_webengine::WebConfirmation::ConfirmationResult::Rejected);
            m_webEngine->confirmationResult(authPopupData->auth);
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
    M_ASSERT(m_viewManager);
    m_viewManager->pushViewToStack(m_historyUI.get());
    m_historyUI->addHistoryItems(getHistory());
}

void SimpleUI::closeHistoryUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    if (m_viewManager->topOfStack() == m_historyUI.get())
        m_viewManager->popTheStack();
}

void SimpleUI::showSettingsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    m_viewManager->pushViewToStack(m_settingsUI.get());
}

void SimpleUI::closeSettingsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    if (m_viewManager->topOfStack() == m_settingsUI.get())
        m_viewManager->popTheStack();
}

void SimpleUI::showMoreMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);

    bool desktopMode = m_webPageUI->stateEquals(WPUState::QUICK_ACCESS) ? m_quickAccess->isDesktopMode() : m_webEngine->isDesktopMode();
    m_moreMenuUI->setDesktopMode(desktopMode);
    m_viewManager->pushViewToStack(m_moreMenuUI.get());
    m_moreMenuUI->showCurrentTab();

    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_moreMenuUI->setFavIcon(m_webEngine->getFavicon());
        m_moreMenuUI->setWebTitle(m_webEngine->getTitle());
        m_moreMenuUI->setURL(m_webEngine->getURI());
    }
    else {
        m_moreMenuUI->setHomePageInfo();
    }
}

void SimpleUI::closeMoreMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    if (m_viewManager->topOfStack() == m_moreMenuUI.get())
        m_viewManager->popTheStack();
    else
        BROWSER_LOGD("[%s:%d] WARNING!!! closeMoreMenu is not topOfStack", __PRETTY_FUNCTION__, __LINE__);
}

void SimpleUI::switchToMobileMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_webEngine->switchToMobileMode();
        m_viewManager->popStackTo(m_webPageUI.get());
        m_webEngine->reload();
    } else {
        m_quickAccess->setDesktopMode(false);
    }
}

void SimpleUI::switchToDesktopMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_webEngine->switchToDesktopMode();
        m_webEngine->reload();
    } else {
        m_quickAccess->setDesktopMode(true);
    }
}

void SimpleUI::showBookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    m_viewManager->pushViewToStack(m_bookmarkManagerUI.get());
    m_bookmarkManagerUI->addBookmarkItems(getBookmarks(ROOT_FOLDER));
}

void SimpleUI::closeBookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_viewManager);
    if (m_viewManager->topOfStack() == m_bookmarkManagerUI.get())
    m_viewManager->popTheStack();
}

void SimpleUI::settingsDeleteSelectedData(const std::string& str)
{
    BROWSER_LOGD("[%s]: Deleting selected data", __func__);
    M_ASSERT(m_viewManager);
    NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager->getContent());
    popup->show("Delete Web Browsing Data");
    onDeleteSelectedDataButton(str);
    popup->dismiss();
}

void SimpleUI::onDeleteSelectedDataButton(const std::string& dataText)
{
	BROWSER_LOGD("[%s]: TYPE : %s", __func__, dataText.c_str());
	if (dataText.find("CACHE") != std::string::npos)
		m_webEngine->clearPrivateData();
	if (dataText.find("COOKIES") != std::string::npos)
		m_webEngine->clearPrivateData();
	if (dataText.find("HISTORY") != std::string::npos)
		m_historyService->clearAllHistory();
}

void SimpleUI::settingsResetMostVisited()
{
    BROWSER_LOGD("[%s]: Deleting most visited sites", __func__);
    M_ASSERT(m_viewManager);
    NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager->getContent());
    popup->show("Reset Most Visited Sites");
    onDeleteMostVisitedButton(nullptr);
    popup->dismiss();
}

void SimpleUI::onDeleteMostVisitedButton(std::shared_ptr< PopupData > /*popupData*/)
{
    BROWSER_LOGD("[%s]: Deleting most visited", __func__);
    m_historyService->cleanMostVisitedHistoryItems();
}

void SimpleUI::settingsResetBrowser()
{
    BROWSER_LOGD("[%s]: Resetting browser", __func__);
    SimplePopup *popup = SimplePopup::createPopup();
    popup->setTitle("Reset browser");
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->setMessage("Are you sure you want to reset browser?");
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onResetBrowserButton, this, _1, _2));
    popup->show();
}

void SimpleUI::onResetBrowserButton(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if (button == OK) {
        BROWSER_LOGD("[%s]: OK", __func__);
        BROWSER_LOGD("[%s]: Resetting browser", __func__);
        M_ASSERT(m_viewManager);

        NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager->getContent());
        popup->show("Reset Browser");

        m_webEngine->clearPrivateData();
        m_historyService->clearAllHistory();
        m_favoriteService->deleteAllBookmarks();

        // Close all openend tabs
        std::vector<std::shared_ptr<tizen_browser::basic_webengine::TabContent>> openedTabs = m_webEngine->getTabContents();
        for (auto it = openedTabs.begin(); it < openedTabs.end(); ++it) {
            tizen_browser::basic_webengine::TabId id = it->get()->getId();
            m_currentSession.removeItem(id.toString());
            m_webEngine->closeTab(id);
        }
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
    int tabs = m_webEngine->tabsCount();
    m_webPageUI->setTabsNumber(tabs);
}

bool SimpleUI::checkIfCreate()
{
    int tabs = m_webEngine->tabsCount();

    if (tabs >= m_tabLimit) {
        SimplePopup *popup = SimplePopup::createPopup();
        popup->setTitle("Maximum tab count reached.");
        popup->addButton(OK);
        popup->setMessage("Close other tabs to open another new tab");
        popup->buttonClicked.connect(boost::bind(&SimpleUI::tabLimitPopupButtonClicked, this, _1, _2));
        popup->show();
        return false;
    }
    else
        return true;
}

void SimpleUI::updateView() {
    int tabs = m_webEngine->tabsCount();
    BROWSER_LOGD("[%s] Opened tabs: %d", __func__, tabs);
    if (tabs == 0) {
        switchViewToQuickAccess();
    } else if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        switchViewToWebPage();
    }
    m_webPageUI->setTabsNumber(tabs);
}

void SimpleUI::tabClosed(const tizen_browser::basic_webengine::TabId& id) {
    m_currentSession.removeItem(id.toString());
    updateView();
}

void SimpleUI::searchWebPage(std::string &text, int flags)
{
    m_webEngine->searchOnWebsite(text, flags);
}

void SimpleUI::addToBookmarks(int folder_id)
{
    BROWSER_LOGD("[%s,%d],", __func__, __LINE__);
    if (m_favoriteService)
    {
        if (m_webEngine && !m_webEngine->getURI().empty())
         {
               m_favoriteService->addToBookmarks(m_webEngine->getURI(), m_webEngine->getTitle(), std::string(),
                                                 m_webEngine->getSnapshotData(373, 240),
                                                 m_webEngine->getFavicon(),(unsigned int)folder_id);
         }
    }
}

//TODO: Replace by direct call.
void SimpleUI::deleteBookmark()
{
	if (m_favoriteService)
		m_favoriteService->deleteBookmark(m_webEngine->getURI());
}
}
}
