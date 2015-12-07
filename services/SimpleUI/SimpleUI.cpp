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

EXPORT_SERVICE(SimpleUI, "org.tizen.browser.simpleui")

const std::string HomePageURL = "about:home";
const std::string ResetBrowserPopupMsg = "Do you really want to reset browser?" \
                                         " If you press reset, delete all data" \
                                         " and return to initial setting.";
SimpleUI::SimpleUI()
    : AbstractMainWindow()
    , m_config(config::DefaultConfigUniquePtr(new config::DefaultConfig()))
    , m_webPageUI()
    , m_moreMenuUI()
#if PROBILE_MOBILE
    , m_bookmarkFlowUI()
#endif
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
    m_config->load("");

    Evas_Object *main_window = elm_win_util_standard_add("browserApp", "browserApp");
    if (main_window == nullptr)
        BROWSER_LOGE("Failed to create main window");

    setMainWindow(main_window);
    m_viewManager.init(main_window);
    evas_object_size_hint_weight_set(main_window, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (main_window, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_win_resize_object_add(main_window, m_viewManager.getContent());
    evas_object_show(main_window);
}

SimpleUI::~SimpleUI() {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_storageService->getSessionStorage().deleteSession(m_currentSession);
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

services::SharedBookmarkFolderList SimpleUI::getBookmarkFolders()
{
    return m_storageService->getSessionStorage().getFolders();
}

const std::string SimpleUI::getBookmarkFolderName(int folder_id)
{
    return m_favoriteService->getBookmarkFolderName(folder_id);
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
            m_tabLimit = boost::any_cast <int> (m_config->get("TAB_LIMIT"));
            m_favoritesLimit = boost::any_cast <int> (m_config->get("FAVORITES_LIMIT"));


            loadUIServices();
            loadModelServices();

            connectModelSignals();
            connectUISignals();

            // initModelServices() needs to be called after initUIServices()
            initUIServices();
            initModelServices();

            //Push first view to stack.
            m_viewManager.pushViewToStack(m_webPageUI.get());
        }
        m_currentSession = std::move(m_storageService->getSessionStorage().createSession());

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
    m_moreMenuUI->setWebTitle(title);
    m_webPageUI->setPageTitle(title);
    if (!m_webEngine->isPrivateMode(m_webEngine->currentTabId())) {
        m_currentSession.updateItem(tabId, m_webEngine->getURI(), title);
    }
}

void SimpleUI::restoreLastSession()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_storageService);
    storage::Session lastSession = std::move(
            m_storageService->getSessionStorage().getLastSession());

    if (lastSession.items().size() >= 1) {
        for (auto iter = lastSession.items().begin(), end =
                lastSession.items().end(); iter != end; ++iter) {
            auto newTabId = m_tabService->convertTabId(iter->first);
            openNewTab(iter->second.first,
                    lastSession.getUrlTitle(iter->second.first), newTabId);
        }
        m_storageService->getSessionStorage().deleteSession(lastSession);
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
#if PROBILE_MOBILE
    m_bookmarkFlowUI =
        std::dynamic_pointer_cast
        <tizen_browser::base_ui::BookmarkFlowUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkflowui"));
#endif
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
    m_webPageUI->getUrlHistoryList()->uriChanged.connect(boost::bind(&SimpleUI::filterURL, this, _1));
    m_webPageUI->backPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webPageUI->backPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::back, m_webEngine.get()));
    m_webPageUI->reloadPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webPageUI->showTabUI.connect(boost::bind(&SimpleUI::showTabUI, this));
    m_webPageUI->showMoreMenu.connect(boost::bind(&SimpleUI::showMoreMenu, this));
    m_webPageUI->forwardPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::forward, m_webEngine.get()));
    m_webPageUI->stopLoadingPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::stopLoading, m_webEngine.get()));
    m_webPageUI->reloadPage.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::reload, m_webEngine.get()));
    m_webPageUI->showQuickAccess.connect(boost::bind(&SimpleUI::showQuickAccess, this));
    m_webPageUI->hideQuickAccess.connect(boost::bind(&QuickAccess::hideUI, m_quickAccess));
    m_webPageUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
#if PROFILE_MOBILE
    m_webPageUI->setWebViewTouchEvents.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::setTouchEvents, m_webEngine.get(), _1));
    m_webPageUI->hideMoreMenu.connect(boost::bind(&SimpleUI::closeMoreMenu, this));
    m_webPageUI->getURIEntry().mobileEntryFocused.connect(boost::bind(&WebPageUI::mobileEntryFocused, m_webPageUI));
    m_webPageUI->getURIEntry().mobileEntryUnfocused.connect(boost::bind(&WebPageUI::mobileEntryUnfocused, m_webPageUI));
#endif

    M_ASSERT(m_quickAccess.get());
    m_quickAccess->getDetailPopup().openURLInNewTab.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, _2));
    m_quickAccess->openURLInNewTab.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, _2));
    m_quickAccess->mostVisitedTileClicked.connect(boost::bind(&SimpleUI::onMostVisitedTileClicked, this, _1, _2));
    m_quickAccess->getMostVisitedItems.connect(boost::bind(&SimpleUI::onMostVisitedClicked, this));
    m_quickAccess->getBookmarksItems.connect(boost::bind(&SimpleUI::onBookmarkButtonClicked, this));
    m_quickAccess->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
    m_quickAccess->switchViewToWebPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));

    M_ASSERT(m_tabUI.get());
    m_tabUI->closeTabUIClicked.connect(boost::bind(&SimpleUI::closeTabUI, this));
    m_tabUI->newTabClicked.connect(boost::bind(&SimpleUI::newTabClicked, this));
    m_tabUI->tabClicked.connect(boost::bind(&SimpleUI::tabClicked, this,_1));
    m_tabUI->closeTabsClicked.connect(boost::bind(&SimpleUI::closeTabsClicked, this,_1));
    m_tabUI->isIncognito.connect(boost::bind(&SimpleUI::isIncognito, this, _1));
#if PROFILE_MOBILE
    bool desktop_ua = false;
#else
    bool desktop_ua = true;
#endif
    m_tabUI->newIncognitoTabClicked.connect(boost::bind(&SimpleUI::openNewTab, this, "", "", boost::none, desktop_ua, true));
    m_tabUI->tabsCount.connect(boost::bind(&SimpleUI::tabsCount, this));

    M_ASSERT(m_historyUI.get());
    m_historyUI->clearHistoryClicked.connect(boost::bind(&SimpleUI::onClearHistoryClicked, this));
    m_historyUI->closeHistoryUIClicked.connect(boost::bind(&SimpleUI::closeHistoryUI, this));
    m_historyUI->historyItemClicked.connect(boost::bind(&SimpleUI::onOpenURLInNewTab, this, _1, desktop_ua));

    M_ASSERT(m_settingsUI.get());
    m_settingsUI->closeSettingsUIClicked.connect(boost::bind(&SimpleUI::closeSettingsUI, this));
    m_settingsUI->deleteSelectedDataClicked.connect(boost::bind(&SimpleUI::settingsDeleteSelectedData, this,_1));
    m_settingsUI->resetMostVisitedClicked.connect(boost::bind(&SimpleUI::settingsResetMostVisited, this));
    m_settingsUI->resetBrowserClicked.connect(boost::bind(&SimpleUI::settingsResetBrowser, this));
#if PROFILE_MOBILE
    m_settingsUI->getWebEngineSettingsParam.connect(boost::bind(&basic_webengine::AbstractWebEngine<Evas_Object>::getSettingsParam, m_webEngine.get(), _1));
    m_settingsUI->setWebEngineSettingsParam.connect(boost::bind(&basic_webengine::AbstractWebEngine<Evas_Object>::setSettingsParam, m_webEngine.get(), _1, _2));
    m_settingsUI->setWebEngineSettingsParam.connect(boost::bind(&storage::SettingsStorage::setParam, &m_storageService->getSettingsStorage(),  _1, _2));
#endif

    M_ASSERT(m_moreMenuUI.get());
    m_moreMenuUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::showBookmarkManagerUI, this));
    m_moreMenuUI->historyUIClicked.connect(boost::bind(&SimpleUI::showHistoryUI, this));
    m_moreMenuUI->settingsClicked.connect(boost::bind(&SimpleUI::showSettingsUI, this));
    m_moreMenuUI->closeMoreMenuClicked.connect(boost::bind(&SimpleUI::closeMoreMenu, this));
    m_moreMenuUI->switchToMobileMode.connect(boost::bind(&SimpleUI::switchToMobileMode, this));
    m_moreMenuUI->switchToDesktopMode.connect(boost::bind(&SimpleUI::switchToDesktopMode, this));
    m_moreMenuUI->isBookmark.connect(boost::bind(&SimpleUI::checkBookmark, this));
    m_moreMenuUI->deleteBookmark.connect(boost::bind(&SimpleUI::deleteBookmark, this));
    m_moreMenuUI->zoomUIClicked.connect(boost::bind(&SimpleUI::showZoomUI, this));
    m_moreMenuUI->bookmarkFlowClicked.connect(boost::bind(&SimpleUI::showBookmarkFlowUI, this, _1));
    //m_moreMenuUI->bookmarkFlowClicked.connect(boost::bind(&SimpleUI::onNewFolderClicked, this));

#if PROFILE_MOBILE
    M_ASSERT(m_bookmarkFlowUI.get());
    m_bookmarkFlowUI->addFolder.connect(boost::bind(&SimpleUI::onNewFolderClicked, this));
    m_bookmarkFlowUI->closeBookmarkFlowClicked.connect(boost::bind(&SimpleUI::closeBookmarkFlowUI, this));
    m_bookmarkFlowUI->saveBookmark.connect(boost::bind(&SimpleUI::addBookmark, this, _1));
    m_bookmarkFlowUI->editBookmark.connect(boost::bind(&SimpleUI::editBookmark, this, _1));
    m_bookmarkFlowUI->removeBookmark.connect(boost::bind(&SimpleUI::deleteBookmark, this));
#endif

    M_ASSERT(m_bookmarkManagerUI.get());
    m_bookmarkManagerUI->closeBookmarkManagerClicked.connect(boost::bind(&SimpleUI::closeBookmarkManagerUI, this));
    m_bookmarkManagerUI->bookmarkItemClicked.connect(boost::bind(&SimpleUI::onBookmarkClicked, this, _1));
    m_bookmarkManagerUI->customFolderClicked.connect(boost::bind(&SimpleUI::onBookmarkCustomFolderClicked, this, _1));
    m_bookmarkManagerUI->allFolderClicked.connect(boost::bind(&SimpleUI::onBookmarkAllFolderClicked, this));
#if PROFILE_MOBILE
    m_bookmarkManagerUI->mobileFolderClicked.connect(boost::bind(&SimpleUI::onBookmarkMobileClicked, this));
    m_bookmarkManagerUI->newFolderItemClicked.connect(boost::bind(&SimpleUI::onNewFolderClicked, this));
    m_bookmarkManagerUI->editFolderButtonClicked.connect(boost::bind(&SimpleUI::onEditFolderClicked, this, _1));
    m_bookmarkManagerUI->deleteFolderButtonClicked.connect(boost::bind(&SimpleUI::onDeleteFolderClicked, this, _1));
#endif

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

    m_storageService =
        std::dynamic_pointer_cast
        <tizen_browser::services::StorageService,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.storageservice"));

    m_favoriteService =
        std::dynamic_pointer_cast
        <tizen_browser::interfaces::AbstractFavoriteService,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.favoriteservice"));

    m_historyService =
        std::dynamic_pointer_cast
        <tizen_browser::services::HistoryService,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyservice"));

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
    m_webPageUI->init(m_viewManager.getContent());

    M_ASSERT(m_quickAccess.get());
    m_quickAccess->init(m_webPageUI->getContent());

    M_ASSERT(m_tabUI.get());
    m_tabUI->init(m_viewManager.getContent());

    M_ASSERT(m_historyUI.get());
    m_historyUI->init(m_viewManager.getContent());

    M_ASSERT(m_moreMenuUI.get());
#if PROFILE_MOBILE
    m_moreMenuUI->init(m_webPageUI->getContent());
#else
    m_moreMenuUI->init(m_viewManager.getContent());
#endif

    M_ASSERT(m_settingsUI.get());
    m_settingsUI->init(m_viewManager.getContent());

#if PROFILE_MOBILE
    M_ASSERT(m_bookmarkFlowUI.get());
    m_bookmarkFlowUI->init(m_viewManager.getContent());
#endif

    M_ASSERT(m_bookmarkManagerUI.get());
    m_bookmarkManagerUI->init(m_viewManager.getContent());

    M_ASSERT(m_zoomUI.get());
    m_zoomUI->init(m_viewManager.getContent());
}

void SimpleUI::initModelServices()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(m_webEngine);
    M_ASSERT(m_webPageUI->getContent());
    m_webEngine->init(m_webPageUI->getContent());

#if PROFILE_MOBILE
    M_ASSERT(m_storageService->getSettingsStorage());
    m_storageService->getSettingsStorage().initWebEngineSettingsFromDB();
#endif

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
    m_webEngine->switchToWebPage.connect(boost::bind(&SimpleUI::switchViewToWebPage, this));
    m_webEngine->titleChanged.connect(boost::bind(&SimpleUI::titleChanged, this, _1, _2));
    m_webEngine->windowCreated.connect(boost::bind(&SimpleUI::windowCreated, this));
    m_webEngine->createTabId.connect(boost::bind(&SimpleUI::onCreateTabId, this));

    m_favoriteService->bookmarkAdded.connect(boost::bind(&SimpleUI::onBookmarkAdded, this,_1));
    m_favoriteService->bookmarkDeleted.connect(boost::bind(&SimpleUI::onBookmarkRemoved, this, _1));

    m_historyService->historyDeleted.connect(boost::bind(&SimpleUI::onHistoryRemoved, this,_1));

    m_tabService->generateThumb.connect(boost::bind(&SimpleUI::onGenerateThumb, this, _1));

    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    m_platformInputManager->backPressed.connect(boost::bind(&SimpleUI::onBackPressed, this));
    m_platformInputManager->escapePressed.connect(boost::bind(&SimpleUI::onEscapePressed, this));
    m_platformInputManager->mouseClicked.connect(
            boost::bind(&SimpleUI::onMouseClick, this));
    m_platformInputManager->redPressed.connect(boost::bind(&SimpleUI::onRedKeyPressed, this));
    m_platformInputManager->yellowPressed.connect(boost::bind(&SimpleUI::onYellowKeyPressed, this));

#if PROFILE_MOBILE
    m_storageService->getSettingsStorage().setWebEngineSettingsParam.connect(boost::bind(&basic_webengine::AbstractWebEngine<Evas_Object>::setSettingsParam, m_webEngine.get(), _1, _2));
    m_platformInputManager->menuButtonPressed.connect(boost::bind(&SimpleUI::onMenuButtonPressed, this));
#endif
}

void SimpleUI::switchViewToWebPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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
}

void SimpleUI::showQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_quickAccess->showUI();
}

void SimpleUI::switchViewToQuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_webPageUI->switchViewToQuickAccess(m_quickAccess->getContent());
    m_webEngine->disconnectCurrentWebViewSignals();
    m_viewManager.popStackTo(m_webPageUI.get());
}

void SimpleUI::switchViewToIncognitoPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_webPageUI->toIncognito(true);
    m_webPageUI->switchViewToIncognitoPage();
    m_viewManager.popStackTo(m_webPageUI.get());
}

void SimpleUI::openNewTab(const std::string &uri, const std::string& title,
        const boost::optional<int> adaptorId, bool desktopMode,
        bool incognitoMode)
{
    BROWSER_LOGD("[%s:%d] uri =%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    tizen_browser::basic_webengine::TabId tab = m_webEngine->addTab(uri,
            nullptr, adaptorId, title, desktopMode, incognitoMode);
    switchToTab(tab);
    m_webPageUI->toIncognito(incognitoMode);
    incognitoMode ? switchViewToIncognitoPage() : m_currentSession.updateItem(tab.toString(), uri, title);
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
    m_tabService->clearThumb(id);
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
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.popStackTo(m_webPageUI.get());
    std::string historyAddress = historyItem->getUrl();
    openNewTab(historyAddress, historyItem->getTitle(), boost::none, desktopMode);
}

void SimpleUI::onOpenURLInNewTab(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem)
{
    onOpenURLInNewTab(historyItem, m_quickAccess->isDesktopMode());
}

void SimpleUI::onMostVisitedTileClicked(std::shared_ptr< services::HistoryItem > historyItem, int itemsNumber)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_quickAccess->openDetailPopup(historyItem, m_historyService->getHistoryItemsByURL(historyItem->getUrl(), itemsNumber));
}

void SimpleUI::onClearHistoryClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyService->clearAllHistory();
}

void SimpleUI::onMostVisitedClicked()
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   m_quickAccess->setMostVisitedItems(getMostVisitedItems());
}

void SimpleUI::onBookmarkButtonClicked()
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   m_quickAccess->setBookmarksItems(getBookmarks());
}

void SimpleUI::onBookmarkClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.popStackTo(m_webPageUI.get());
    std::string bookmarkAddress = bookmarkItem->getAddress();

    if(tabsCount() == 0 || !m_webEngine->isPrivateMode(m_webEngine->currentTabId())){
        openNewTab(bookmarkAddress);
    }
    else {
        std::string bookmarkTitle = bookmarkItem->getTitle();
        m_webPageUI->switchViewToWebPage(m_webEngine->getLayout(), bookmarkAddress, bookmarkTitle);
        m_webEngine->setURI(bookmarkAddress);
        m_webPageUI->setPageTitle(bookmarkTitle);
        m_webPageUI->getURIEntry().clearFocus();
        closeBookmarkManagerUI();
    }
}

void SimpleUI::onNewFolderClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    InputPopup *inputPopup = InputPopup::createPopup(m_viewManager.getContent(), "New Folder", "Add New Folder?",
                                                          "New Folder #", "Add", "Cancel", true);
#else
    InputPopup *inputPopup = InputPopup::createPopup(m_viewManager.getContent(), "New Folder", "Add new folder for adding to bookmark?",
                                                          "Folder #", "Cancel", "Add to bookmark", false);
#endif
    inputPopup->button_clicked.connect(boost::bind(&SimpleUI::onNewFolderPopupClick, this, _1));
    inputPopup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    inputPopup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    inputPopup->show();
}

void SimpleUI::onNewFolderPopupClick(const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_storageService->getSessionStorage().ifFolderExists(folder_name)) {
        BROWSER_LOGD("[%s:%d] Folder already exists.", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    unsigned int id = m_storageService->getSessionStorage().addFolder(folder_name);
#if PROFILE_MOBILE
    if (m_viewManager.topOfStack() == m_bookmarkManagerUI.get()) {
        SharedBookmarkFolder folder = m_storageService->getSessionStorage().getFolder(id);
        SharedBookmarkFolderList list;
        list.push_back(folder);
        m_bookmarkManagerUI->addCustomFolders(list);
    }
#else
    BookmarkUpdate update;
    update.folder_id = 0;
    //TODO: Change 0 to id
    M_UNUSED(id);
    addBookmark(update);
#endif
}
#if PROFILE_MOBILE
void SimpleUI::onEditFolderClicked(const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = InputPopup::createInputPopup(m_viewManager.getContent(), "Edit Folder name", "Edit folder name?",
                                                          folder_name, "Done", "Cancel");
    inputPopup->button_clicked.connect(boost::bind(&SimpleUI::onEditFolderPopupClicked, this, _1));
    inputPopup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    inputPopup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    m_folder_name = folder_name;
    inputPopup->show();
}

void SimpleUI::onDeleteFolderClicked(const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
    popup->setRightButton(DELETE);
    popup->setLeftButton(CANCEL);
    popup->setTitle("Delete");
    popup->setMessage("<b>Delete '" + folder_name + "'?</b><br>If you delete this Folder, All Bookmarks in the folder will also be deleted.");
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteFolderPopupClicked, this, _1));
    popup->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    popup->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    m_folder_name = folder_name;
    popup->show();
}

void SimpleUI::onEditFolderPopupClicked(const std::string& newName)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_storageService->getSessionStorage().ifFolderExists(m_folder_name)) {
        unsigned int id = m_storageService->getSessionStorage().getFolderId(m_folder_name);
        m_storageService->getSessionStorage().updateFolderName(id, newName);
        m_bookmarkManagerUI->hideUI();
        showBookmarkManagerUI();
    }
}

void SimpleUI::onDeleteFolderPopupClicked(PopupButtons button)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (button == DELETE && m_storageService->getSessionStorage().ifFolderExists(m_folder_name)) {
        unsigned int id = m_storageService->getSessionStorage().getFolderId(m_folder_name);
        m_storageService->getSessionStorage().deleteFolder(id);
        m_bookmarkManagerUI->hideUI();
        showBookmarkManagerUI();
    }
}
#endif

void SimpleUI::onGenerateThumb(basic_webengine::TabId tabId)
{
    const int THUMB_WIDTH = boost::any_cast<int>(
            m_config->get(CONFIG_KEY::TABSERVICE_THUMB_WIDTH));
    const int THUMB_HEIGHT = boost::any_cast<int>(
            m_config->get(CONFIG_KEY::TABSERVICE_THUMB_HEIGHT));
    tools::BrowserImagePtr snapshotImage = m_webEngine->getSnapshotData(tabId,
            THUMB_WIDTH, THUMB_HEIGHT);
    m_tabService->onThumbGenerated(tabId, snapshotImage);
}

void SimpleUI::onCreateTabId()
{
    int id = m_tabService->createTabId();
    m_webEngine->onTabIdCreated(id);
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
    if (m_popupVector.size() > 0) {
        m_popupVector.back()->onBackPressed();
    } else if ((m_viewManager.topOfStack() == m_tabUI.get()) && m_tabUI->isEditMode()) {
        m_tabUI->onBackKey();
    } else if (m_viewManager.topOfStack() == m_bookmarkManagerUI.get()) {
        m_viewManager.popTheStack();
    } else if (m_webPageUI->stateEquals(WPUState::QUICK_ACCESS) && m_quickAccess->canBeBacked(m_webEngine->tabsCount())) {
        m_quickAccess->backButtonClicked();
    } else if (m_viewManager.topOfStack() == nullptr) {
        switchViewToQuickAccess();
    } else if ((m_viewManager.topOfStack() == m_webPageUI.get())) {
        m_webEngine->backButtonClicked();
#if PROFILE_MOBILE
    } else if ((m_viewManager.topOfStack() == m_settingsUI.get()) && m_settingsUI->isSubpage()) {
        m_settingsUI->onBackKey();
#endif
    } else {
        m_viewManager.popTheStack();
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
    if (!m_webEngine->isPrivateMode(m_webEngine->currentTabId()))
        m_currentSession.updateItem(m_webEngine->currentTabId().toString(), m_webEngine->getURI(), m_webEngine->getTitle());
    m_tabService->clearThumb(m_webEngine->currentTabId());
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
        m_historyService->addHistoryItem(std::make_shared<tizen_browser::services::HistoryItem>(
                                             m_webEngine->getURI(),
                                             m_webEngine->getTitle(),
                                             m_webEngine->getFavicon()),
                                         m_webEngine->getSnapshotData(
                                             QuickAccess::MAX_THUMBNAIL_WIDTH,
                                             QuickAccess::MAX_THUMBNAIL_HEIGHT));
    m_tabService->updateThumb(m_webEngine->currentTabId());
    m_webPageUI->loadFinished();
}

void SimpleUI::loadStopped()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!m_webEngine->isPrivateMode(m_webEngine->currentTabId()))
        m_historyService->addHistoryItem(std::make_shared<tizen_browser::services::HistoryItem>(
                                             m_webEngine->getURI(),
                                             m_webEngine->getURI(),
                                             std::make_shared<tizen_browser::tools::BrowserImage>()),
                                         std::make_shared<tizen_browser::tools::BrowserImage>());
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
            m_webPageUI->getUrlHistoryList()->getItemsNumberMax();
    int minKeywordLength =
            m_webPageUI->getUrlHistoryList()->getKeywordLengthMin();
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
        m_viewManager.popStackTo(m_webPageUI.get());
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
    m_viewManager.pushViewToStack(m_tabUI.get());

    std::vector<basic_webengine::TabContentPtr> tabsContents =
            m_webEngine->getTabContents();
    m_tabService->fillThumbs(tabsContents);
    m_tabUI->addTabItems(tabsContents);
}

void SimpleUI::closeTabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_tabUI.get())
        m_viewManager.popTheStack();
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
    m_viewManager.popStackTo(m_webPageUI.get());
    m_webPageUI->toIncognito(m_webEngine->isPrivateMode(tabId));
    switchToTab(tabId);
}

bool SimpleUI::isIncognito(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d:%s] ", __PRETTY_FUNCTION__, __LINE__, __func__);
    return m_webEngine->isPrivateMode(tabId);
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
        if (m_webPageUI->stateEquals(WPUState::MAIN_WEB_PAGE))
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

        std::string entryTextStyle = "DEFAULT='font=Sans:style=Regular font_size=20 ellipsis=0.0'";
        Evas_Object *loginEntry = elm_entry_add(popup_content);
        elm_entry_text_style_user_push(loginEntry, entryTextStyle.c_str());
        elm_object_part_content_set(popup_content, "login", loginEntry);

        Evas_Object *passwordEntry = elm_entry_add(popup_content);
        elm_entry_password_set(passwordEntry, EINA_TRUE);
        elm_object_part_content_set(popup_content, "password", passwordEntry);

        SimplePopup *popup = SimplePopup::createPopup(m_viewManager.getContent());
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
    m_viewManager.pushViewToStack(m_historyUI.get());
    m_historyUI->addHistoryItems(m_historyService->getHistoryToday(),
            HistoryPeriod::HISTORY_TODAY);
    m_historyUI->addHistoryItems(m_historyService->getHistoryYesterday(),
            HistoryPeriod::HISTORY_YESTERDAY);
    m_historyUI->addHistoryItems(m_historyService->getHistoryLastWeek(),
            HistoryPeriod::HISTORY_LASTWEEK);
}

void SimpleUI::closeHistoryUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_historyUI.get())
        m_viewManager.popTheStack();
}

void SimpleUI::showSettingsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.pushViewToStack(m_settingsUI.get());
}

void SimpleUI::closeSettingsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_settingsUI.get())
        m_viewManager.popTheStack();
}

void SimpleUI::showMoreMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

#if PROFILE_MOBILE
    M_ASSERT(m_webPageUI);
    if (evas_object_visible_get(m_moreMenuUI->getContent()))
        m_moreMenuUI->hideUI();
    else {
        m_moreMenuUI->blockThumbnails(m_webPageUI->stateEquals(WPUState::QUICK_ACCESS));
        m_moreMenuUI->showUI();
    }
#else
    bool desktopMode = m_webPageUI->stateEquals(WPUState::QUICK_ACCESS) ? m_quickAccess->isDesktopMode() : m_webEngine->isDesktopMode();
    m_moreMenuUI->setDesktopMode(desktopMode);
    m_viewManager.pushViewToStack(m_moreMenuUI.get());
    m_moreMenuUI->showCurrentTab();

    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_moreMenuUI->setFavIcon(m_webEngine->getFavicon());
        m_moreMenuUI->setWebTitle(m_webEngine->getTitle());
        m_moreMenuUI->setURL(m_webEngine->getURI());
    }
    else {
        m_moreMenuUI->setHomePageInfo();
    }
#endif
    m_moreMenuUI->setIsBookmark(checkBookmark());
}

void SimpleUI::closeMoreMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

#if PROFILE_MOBILE
    M_ASSERT(m_webPageUI);
    if (evas_object_visible_get(m_moreMenuUI->getContent()))
        m_moreMenuUI->hideUI();
#else
    if (m_viewManager.topOfStack() == m_moreMenuUI.get())
        m_viewManager.popTheStack();
    else
        BROWSER_LOGD("[%s:%d] WARNING!!! closeMoreMenu is not topOfStack", __PRETTY_FUNCTION__, __LINE__);
#endif
}

void SimpleUI::switchToMobileMode()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        m_webEngine->switchToMobileMode();
        m_viewManager.popStackTo(m_webPageUI.get());
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
    m_viewManager.pushViewToStack(m_bookmarkFlowUI.get());
    std::string uri = m_webEngine->getURI();
    m_bookmarkFlowUI->setURL(uri);
    m_bookmarkFlowUI->setState(state);
    tizen_browser::services::BookmarkItem item;
    if(m_favoriteService->bookmarkExists(uri) && m_favoriteService->getItem(uri, &item))
        m_bookmarkFlowUI->setTitle(item.getTitle());
    else
        m_bookmarkFlowUI->setTitle(m_webEngine->getTitle());
#else
    BookmarkFlowUI *bookmarkFlow = BookmarkFlowUI::createPopup(m_viewManager.getContent());
    bookmarkFlow->popupShown.connect(boost::bind(&SimpleUI::showPopup, this, _1));
    bookmarkFlow->popupDismissed.connect(boost::bind(&SimpleUI::dismissPopup, this, _1));
    bookmarkFlow->addFolder.connect(boost::bind(&SimpleUI::onNewFolderClicked, this));
    bookmarkFlow->saveBookmark.connect(boost::bind(&SimpleUI::addBookmark, this, _1));
    bookmarkFlow->show();
    bookmarkFlow->gridAddNewFolder();
    bookmarkFlow->gridAddCustomFolders(getBookmarkFolders());
#endif
}
#if PROFILE_MOBILE
void SimpleUI::closeBookmarkFlowUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_bookmarkFlowUI.get())
        m_viewManager.popTheStack();
}
#endif

void SimpleUI::showBookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_viewManager.pushViewToStack(m_bookmarkManagerUI.get());
#if PROFILE_MOBILE
    m_bookmarkManagerUI->addNewFolder();
    #endif
    m_bookmarkManagerUI->addAllFolder(getBookmarks(tizen_browser::services::ALL_BOOKMARKS_ID),
                                      getBookmarkFolderName(tizen_browser::services::ALL_BOOKMARKS_ID));
#if PROFILE_MOBILE
    m_bookmarkManagerUI->addMobileFolder(getBookmarks(tizen_browser::services::ROOT_FOLDER_ID),
                                         getBookmarkFolderName(tizen_browser::services::ROOT_FOLDER_ID));
#endif
    m_bookmarkManagerUI->addCustomFolders(getBookmarkFolders());
    m_bookmarkManagerUI->showUI();
}


void SimpleUI::onBookmarkAllFolderClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
        //TODO ON TV
    m_bookmarkManagerUI->getDetailsContent();
    m_bookmarkManagerUI->addDetails(getBookmarks(tizen_browser::services::ALL_BOOKMARKS_ID),
                                    getBookmarkFolderName(tizen_browser::services::ALL_BOOKMARKS_ID));
    m_bookmarkManagerUI->showDetailsUI();
#endif
}
#if PROFILE_MOBILE
void SimpleUI::onBookmarkMobileClicked()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_bookmarkManagerUI->getDetailsContent();
    m_bookmarkManagerUI->addDetails(getBookmarks(tizen_browser::services::ROOT_FOLDER_ID),
                                    getBookmarkFolderName(tizen_browser::services::ROOT_FOLDER_ID));
    m_bookmarkManagerUI->showDetailsUI();
}
#endif
void SimpleUI::onBookmarkCustomFolderClicked(int folderId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    //TODO ON TV
    m_bookmarkManagerUI->getDetailsContent();
    m_bookmarkManagerUI->addDetails(getBookmarks(folderId), m_storageService->getSessionStorage().getFolderName(folderId));
    //m_bookmarkManagerUI->addDetails(getBookmarks(folderId), getBookmarkFolderName(folderId));
    m_bookmarkManagerUI->showDetailsUI();
#else
    M_UNUSED(folderId);
#endif
}

void SimpleUI::closeBookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_viewManager.topOfStack() == m_bookmarkManagerUI.get())
    m_viewManager.popTheStack();
}

void SimpleUI::settingsDeleteSelectedData(const std::string& str)
{
    BROWSER_LOGD("[%s]: Deleting selected data", __func__);
    M_ASSERT(m_viewManager);
    if((str.find("CACHE")    != std::string::npos)  ||
       (str.find("COOKIES")  != std::string::npos)  ||
       (str.find("HISTORY")  != std::string::npos)  ||
       (str.find("PASSWORD") != std::string::npos)  ||
       (str.find("FORMDATA") != std::string::npos)) {
        NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager.getContent());
        popup->show("Delete Web Browsing Data");
        onDeleteSelectedDataButton(str);
        popup->dismiss();
    }
}

void SimpleUI::onDeleteSelectedDataButton(const std::string& dataText)
{
    BROWSER_LOGD("[%s]: TYPE : %s", __func__, dataText.c_str());
    if (dataText.find("CACHE") != std::string::npos)
        m_webEngine->clearCache();
    if (dataText.find("COOKIES") != std::string::npos)
        m_webEngine->clearCookies();
    if (dataText.find("HISTORY") != std::string::npos)
        m_historyService->clearAllHistory();
    if (dataText.find("PASSWORD") != std::string::npos)
        m_webEngine->clearPasswordData();
    if (dataText.find("FORMDATA") != std::string::npos)
        m_webEngine->clearFormData();
}

void SimpleUI::settingsResetMostVisited()
{
    BROWSER_LOGD("[%s]: Deleting most visited sites", __func__);
    NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager.getContent());
    popup->show("Delete Web Browsing Data");
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
#if PROFILE_MOBILE
    TextPopup* popup = TextPopup::createPopup(m_viewManager.getContent());
    popup->setRightButton(RESET);
    popup->setLeftButton(CANCEL);
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onResetBrowserButton, this, _1, nullptr));
#else
    SimplePopup* popup = SimplePopup::createPopup(m_viewManager.getContent());
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

        NotificationPopup *popup = NotificationPopup::createNotificationPopup(m_viewManager.getContent());
        popup->show("Reset Browser");

        m_webEngine->clearPrivateData();
        m_historyService->clearAllHistory();
        m_favoriteService->deleteAllBookmarks();
        m_webEngine->clearPasswordData();
        m_webEngine->clearFormData();

        // Close all openend tabs
        std::vector<std::shared_ptr<tizen_browser::basic_webengine::TabContent>> openedTabs = m_webEngine->getTabContents();
        for (auto it = openedTabs.begin(); it < openedTabs.end(); ++it) {
            tizen_browser::basic_webengine::TabId id = it->get()->getId();
            m_currentSession.removeItem(id.toString());
            m_tabService->clearThumb(id);
            m_webEngine->closeTab(id);
        }
        m_storageService->getSessionStorage().deleteAllFolders();
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
        SimplePopup *popup = SimplePopup::createPopup(m_viewManager.getContent());
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
    int tabs = m_webEngine->tabsCount();
    BROWSER_LOGD("[%s] Opened tabs: %d", __func__, tabs);
    if (tabs == 0) {
        switchViewToQuickAccess();
    } else if (!m_webPageUI->stateEquals(WPUState::QUICK_ACCESS)) {
        switchViewToWebPage();
    }
    m_webPageUI->setTabsNumber(tabs);
}

void SimpleUI::windowCreated()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switchViewToWebPage();
}

void SimpleUI::tabClosed(const tizen_browser::basic_webengine::TabId& id) {
    m_currentSession.removeItem(id.toString());
    m_tabService->clearThumb(id);
    updateView();
}

void SimpleUI::searchWebPage(std::string &text, int flags)
{
    m_webEngine->searchOnWebsite(text, flags);
}

void SimpleUI::addBookmark(BookmarkUpdate bookmark_update)
{
    BROWSER_LOGD("[%s,%d],", __func__, __LINE__);
    if (m_favoriteService) {
        if (m_webEngine && !m_webEngine->getURI().empty()) {
            m_favoriteService->addBookmark(m_webEngine->getURI(),
#if PROFILE_MOBILE
                                           bookmark_update.title,
#else
                                           m_webEngine->getTitle(),
#endif
                                           std::string(), m_webEngine->getSnapshotData(373, 240),
                                           m_webEngine->getFavicon(), (unsigned int)bookmark_update.folder_id);
        }
    }
}

#if PROFILE_MOBILE
void SimpleUI::editBookmark(BookmarkUpdate bookmark_update)
{
    BROWSER_LOGD("[%s,%d],", __func__, __LINE__);
    if (m_favoriteService) {
        if (m_webEngine && !m_webEngine->getURI().empty()) {
               m_favoriteService->editBookmark(m_webEngine->getURI(), bookmark_update.title, (unsigned int)bookmark_update.folder_id);
        }
    }
}
#endif

//TODO: Replace by direct call.
void SimpleUI::deleteBookmark()
{
	if (m_favoriteService)
		m_favoriteService->deleteBookmark(m_webEngine->getURI());
}
}
}
