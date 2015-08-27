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

#include <boost/format.hpp>
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
#include "SimpleURI.h"
#include "SimpleUI.h"
#include "BookmarkItem.h"
#include "Tools/EflTools.h"
#include "BrowserImage.h"
#include "HistoryItem.h"
#include "BookmarkItem.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "NetworkErrorHandler.h"
#include "SqlStorage.h"


namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SimpleUI, "org.tizen.browser.simpleui")

const std::string HomePageURL = "about:home";
const int ROOT_FOLDER = 0;

SimpleUI::SimpleUI()
    : AbstractMainWindow()
    , m_mainLayout(nullptr)
    , m_progressBar(nullptr)
    , m_popup(nullptr)
    , m_settings()
    , m_moreMenuUI()
    , m_tabUI()
    , m_bookmarkManagerUI()
    , m_mainUI()
    , m_initialised(false)
    , m_isHomePageActive(false)
    , m_currentZoom(ZOOM_TYPE_100)
    , items_vector()
    , m_networkErrorPopup(0)
    , m_wvIMEStatus(false)
    , m_ewkContext(ewk_context_new())
{
    elm_init(0, nullptr);
    Evas_Object *main_window = elm_win_util_standard_add("browserApp", "browserApp");
    if (main_window == nullptr)
        BROWSER_LOGE("Failed to create main window");

    setMainWindow(main_window);
}

SimpleUI::~SimpleUI() {
    BROWSER_LOGD("%s", __func__);
    m_sessionService->getStorage()->deleteSession(m_currentSession);
    /// \todo Auto-generated destructor stub
    evas_object_del(m_window.get());
    ewk_context_delete(m_ewkContext);
}

void SimpleUI::destroyUI()
{
    BROWSER_LOGD("%s", __func__);
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

std::shared_ptr<services::HistoryItemVector> SimpleUI::getHistory()
{
    return m_historyService->getMostVisitedHistoryItems();
}


std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > SimpleUI::getBookmarkFolders(int folder_id)
{
    return m_favoriteService->getBookmarkFolders(folder_id);
}



int SimpleUI::exec(const std::string& _url)
{
    BROWSER_LOGD("[%s] _url=%s, initialised=%d", __func__, _url.c_str(), m_initialised);
    std::string url = _url;
    Session::Session lastSession;

    if(!m_initialised){
        if (m_window.get()) {
            config::DefaultConfig config;
            config.load("");
            m_tabLimit = boost::any_cast <int> (config.get("TAB_LIMIT"));
            m_favoritesLimit = boost::any_cast <int> (config.get("FAVORITES_LIMIT"));
            elm_win_alpha_set(m_window.get(), EINA_FALSE);

            // creating main window
            int width = 0;
            int height = 0;
            ecore_wl_screen_size_get(&width, &height);
            evas_object_move(m_window.get(), 0, 0);
            evas_object_resize(m_window.get(), width, height);

            // create main layout
            m_mainLayout = elm_layout_add(m_window.get());
            evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            elm_win_resize_object_add(m_window.get(), m_mainLayout);

            ///\todo Integrate with pointer / tab mode switch
            elm_win_focus_highlight_style_set(m_window.get(), "invisible");
            //elm_config_focus_highlight_animate_set(EINA_TRUE);

            m_errorLayout = elm_layout_add(m_window.get());
            evas_object_size_hint_weight_set(m_errorLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            //elm_win_resize_object_add(m_window.get(), m_errorLayout);

            //set global show tooltip timeout
            elm_config_tooltip_delay_set( boost::any_cast <double> (config.get("TOOLTIP_DELAY")));

            loadThemes();

            if(!elm_layout_file_set(m_mainLayout, edjePath("SimpleUI/MainLayout.edj").c_str(), "main_layout"))
                throw std::runtime_error("Layout file not found: " + edjePath("SimpleUI/MainLayout.edj"));

            //elm_object_style_set(m_errorLayout, "error_message");
            if(!elm_layout_file_set(m_errorLayout, edjePath("SimpleUI/ErrorMessage.edj").c_str(), "error_message"))
                throw std::runtime_error("Layout file not found: " + edjePath("SimpleUI/ErrorMessage.edj"));

            // load && initialize components
            // simpleURI
            BROWSER_LOGD("[%s:%d] service: simpleURI ", __PRETTY_FUNCTION__, __LINE__);
            m_simpleURI =
                std::dynamic_pointer_cast
                <tizen_browser::base_ui::SimpleURI,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleuri"));
            M_ASSERT(m_simpleURI);

            // webengine
            BROWSER_LOGD("[%s:%d] service: webkitengineservice ", __PRETTY_FUNCTION__, __LINE__);
            m_webEngine =
                std::dynamic_pointer_cast
                <basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));
            M_ASSERT(m_webEngine);
            m_webEngine->init(m_mainLayout);

            // bookmarks UI
            BROWSER_LOGD("[%s:%d] service: bookmarksui ", __PRETTY_FUNCTION__, __LINE__);

            // favorites service
            BROWSER_LOGD("[%s:%d] service: favoriteservice ", __PRETTY_FUNCTION__, __LINE__);
            std::string favoriteService(boost::any_cast < std::string > (config.get("favorite_service_name")));
            BROWSER_LOGD("favorite config");
            m_favoriteService =
                std::dynamic_pointer_cast
                <tizen_browser::interfaces::AbstractFavoriteService,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService(favoriteService));
            BROWSER_LOGD("favorite create");
            M_ASSERT(m_favoriteService);
            m_favoriteService->synchronizeBookmarks();
            BROWSER_LOGD("favorite before getBookmarks");
            m_favoriteService->getBookmarks();
            BROWSER_LOGD("favorite after getBookmarks");

            // history service
            BROWSER_LOGD("[%s:%d] service: historyservice ", __PRETTY_FUNCTION__, __LINE__);
            m_historyService =
                std::dynamic_pointer_cast
                <tizen_browser::services::HistoryService,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyservice"));
            M_ASSERT(m_historyService);


            // Platforminputmanager
            BROWSER_LOGD("[%s:%d] service: platforminputmanager ", __PRETTY_FUNCTION__, __LINE__);
            m_platformInputManager =
                std::dynamic_pointer_cast
                <tizen_browser::services::PlatformInputManager,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.platforminputmanager"));
            M_ASSERT(m_platformInputManager);
            m_platformInputManager->init(m_window.get());
            m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
            m_platformInputManager->backPressed.connect(boost::bind(&SimpleUI::onBackPressed, this));

            createActions();

            // left buttons
            leftButtonBar = std::make_shared<ButtonBar>(m_mainLayout, "SimpleUI/LeftButtonBar.edj", "left_button_bar");
            leftButtonBar->addAction(m_back, "prev_button");
            leftButtonBar->addAction(m_forward, "next_button");
            leftButtonBar->addAction(m_reload, "refresh_stop_button");
            //register action that will be used later by buttons
            leftButtonBar->registerEnabledChangedCallback(m_stopLoading, "refresh_stop_button");

            // right buttons
            rightButtonBar = std::make_shared<ButtonBar>(m_mainLayout, "SimpleUI/RightButtonBar.edj", "right_button_bar");
            rightButtonBar->addAction(m_tab, "tab_button");
            rightButtonBar->addAction(m_showMoreMenu, "setting_button");

            m_progressBar = elm_progressbar_add(m_mainLayout);
            elm_object_style_set(m_progressBar,"play_buffer");

            webTitleBar = std::make_shared<WebTitleBar>(m_mainLayout, "SimpleUI/WebTitleBar.edj", "web_title_bar");
            elm_object_part_content_set(m_mainLayout, "web_title_bar", webTitleBar->getContent());

            elm_object_part_content_set(m_mainLayout, "uri_entry", m_simpleURI->getContent(m_mainLayout));
            elm_object_part_content_set(m_mainLayout, "uri_bar_buttons_left", leftButtonBar->getContent());
            elm_object_part_content_set(m_mainLayout, "uri_bar_buttons_right", rightButtonBar->getContent());

            // connecting all together
            m_simpleURI->uriChanged.connect(boost::bind(&SimpleUI::filterURL, this, _1));
            m_webEngine->uriChanged.connect(boost::bind(&SimpleUI::webEngineURLChanged, this, _1));
            m_webEngine->uriOnTabChanged.connect(boost::bind(&SimpleUI::checkTabId,this,_1));
            m_webEngine->webViewClicked.connect(boost::bind(&SimpleURI::clearFocus, m_simpleURI.get()));
            m_webEngine->backwardEnableChanged.connect(boost::bind(&SimpleUI::backEnable, this, _1));
            m_webEngine->forwardEnableChanged.connect(boost::bind(&SimpleUI::forwardEnable, this, _1));
            m_webEngine->loadStarted.connect(boost::bind(&SimpleUI::loadStarted, this));
            m_webEngine->loadProgress.connect(boost::bind(&SimpleUI::progressChanged,this,_1));
            m_webEngine->loadFinished.connect(boost::bind(&SimpleUI::loadFinished, this));
            m_webEngine->loadStop.connect(boost::bind(&SimpleUI::loadFinished, this));
            m_webEngine->loadError.connect(boost::bind(&SimpleUI::loadError, this));
            m_webEngine->confirmationRequest.connect(boost::bind(&SimpleUI::handleConfirmationRequest, this, _1));
            m_webEngine->tabCreated.connect(boost::bind(&SimpleUI::tabCreated, this));
            m_webEngine->tabClosed.connect(boost::bind(&SimpleUI::tabClosed,this,_1));
            m_webEngine->titleChanged.connect(boost::bind(&WebTitleBar::show, webTitleBar.get(), _1));
            m_webEngine->IMEStateChanged.connect(boost::bind(&SimpleUI::setwvIMEStatus, this, _1));

            m_favoriteService->bookmarkAdded.connect(boost::bind(&SimpleUI::onBookmarkAdded, this,_1));
            m_favoriteService->bookmarkDeleted.connect(boost::bind(&SimpleUI::onBookmarkRemoved, this, _1));

            //m_historyService->historyEmpty.connect(boost::bind(&SimpleUI::disableHistoryButton, this, _1));
            m_historyService->historyAdded.connect(boost::bind(&SimpleUI::onHistoryAdded, this,_1));
            m_historyService->historyDeleted.connect(boost::bind(&SimpleUI::onHistoryRemoved, this,_1));

            connectActions();

            elm_layout_signal_callback_add(m_simpleURI->getContent(), "slide_websearch", "elm", SimpleUI::favicon_clicked, this);

            // show main layout and window
            evas_object_show(m_mainLayout);
            evas_object_show(m_window.get());

            m_netErrorHandler = std::unique_ptr<tizen_browser::basic_ui::NetworkErrorHandler>(new tizen_browser::basic_ui::NetworkErrorHandler);
            m_netErrorHandler->networkError.connect(boost::bind(&SimpleUI::onNetworkError, this));
            m_netErrorHandler->networkConnected.connect(boost::bind(&SimpleUI::onNetworkConnected, this));

            m_sessionService = std::dynamic_pointer_cast
            <
                tizen_browser::services::SessionStorage,
                tizen_browser::core::AbstractService
            >(tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.sessionStorageService"));
            if(m_sessionService){
                lastSession = std::move(m_sessionService->getStorage()->getLastSession());
                m_currentSession = std::move(m_sessionService->getStorage()->createSession());
            }

        }
        m_initialised = true;

        m_mainUI = std::dynamic_pointer_cast <tizen_browser::base_ui::MainUI,tizen_browser::core::AbstractService>
                    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.mainui"));
        M_ASSERT(m_mainUI.get());

        m_historyService->historyAllDeleted.connect(boost::bind(&tizen_browser::base_ui::MainUI::clearHistoryGenlist, m_mainUI.get()));
        m_mainUI->historyClicked.connect(boost::bind(&SimpleUI::onHistoryClicked, this,_1));
        m_mainUI->mostVisitedClicked.connect(boost::bind(&SimpleUI::onMostVisitedClicked, this,_1));
        m_mainUI->bookmarkClicked.connect(boost::bind(&SimpleUI::onBookmarkButtonClicked, this,_1));
        m_mainUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::onBookmarkManagerButtonClicked, this,_1));

	}

	if (url.empty()) {
		BROWSER_LOGD("[%s]: changing to homeUrl", __func__);
		switchViewToHomePage();
#if MERGE_ME // Not sure if this should be enabled
		filterURL(HomePageURL);
#endif
		if(lastSession.items().size() >= 1){
			for(auto iter=lastSession.items().begin(),
				end=lastSession.items().end();
				iter != end;
				iter++
				){
					openNewTab(iter->second);
				}
				m_sessionService->getStorage()->deleteSession(lastSession);
			}
		}
		else
			openNewTab(url);

	BROWSER_LOGD("[%s]:%d url=%s", __func__, __LINE__, url.c_str());

    m_simpleURI->setFocus();

    return 0;
}

void SimpleUI::loadThemes()
{
    elm_theme_extension_add(nullptr, edjePath("SimpleUI/ZoomItem.edj").c_str());
    elm_theme_extension_add(nullptr, edjePath("SimpleUI/TabItem.edj").c_str());
    elm_theme_extension_add(nullptr, edjePath("SimpleUI/ErrorMessage.edj").c_str());

    elm_theme_overlay_add(0, edjePath("SimpleUI/ScrollerDefault.edj").c_str());
    elm_theme_overlay_add(0, edjePath("SimpleUI/Tooltip.edj").c_str());
}

void SimpleUI::createActions()
{
    ///\todo Add MulitStateAction. and convert m_stopLoading and m_reload actons to it?

    m_back = sharedAction(new Action("Back"));
    m_back->setToolTip("Previous");
    m_back->setIcon("browser/toolbar_prev");

    m_forward = sharedAction(new Action("Next"));
    m_forward->setToolTip("Next");
    m_forward->setIcon("browser/toolbar_next");

    m_stopLoading = sharedAction(new Action("Stop"));
    m_stopLoading->setToolTip("Stop");
    m_stopLoading->setIcon("browser/toolbar_stop");

    m_reload = sharedAction(new Action("Reload"));
    m_reload->setToolTip("Reload");
    m_reload->setIcon("browser/toolbar_reload");
    m_tab = sharedAction(new Action("Tabs"));
    m_tab->setToolTip("Tab page");
    m_tab->setIcon("browser/toolbar_tab");
    //m_tab->setCheckable(true);

    m_showMoreMenu = sharedAction(new Action("Settings"));
    m_showMoreMenu->setToolTip("Settings");
    m_showMoreMenu->setIcon("browser/toolbar_setting");
/*
    m_settingPointerMode = sharedAction(new Action("Pointer mode"));
    m_settingPointerMode->setToolTip("Switch to Pointer Mode");
    m_settingPointerMode->setCheckable(true);
    m_settingPointerMode->setChecked(m_platformInputManager->getPointerModeEnabled());
    m_settingPointerMode->setEnabled(true);
*/

    m_settingPrivateBrowsing = sharedAction(new Action("Private browsing"));
    m_settingPrivateBrowsing->setToolTip("On exit from private mode all cookies, history, and stored data will be deleted");
    m_settingPrivateBrowsing->setCheckable(true);
    m_settingPrivateBrowsing->setChecked(m_webEngine->isPrivateMode());
    m_settingPrivateBrowsing->setEnabled(true);

    m_settingDeleteHistory = sharedAction(new Action("Delete history"));
    m_settingDeleteHistory->setToolTip("Delete History");

    m_settingDeleteData = sharedAction(new Action("Delete data"));
    m_settingDeleteData->setToolTip("Delete Data");

    m_settingDeleteFavorite = sharedAction(new Action("Delete favorite site"));
    m_settingDeleteFavorite->setToolTip("Delete favorite site");

    m_bookmarks_manager_Add_NewFolder = sharedAction(new Action("+ New Folder"));
    m_bookmarks_manager_Add_NewFolder->setToolTip("Add a new Folder");

    m_bookmarks_manager_BookmarkBar = sharedAction(new Action("Bookmark Bar"));
    m_bookmarks_manager_BookmarkBar->setToolTip("show Bookmark bar");

    m_bookmarks_manager_Folder3 = sharedAction(new Action("Folder 3"));
    m_bookmarks_manager_Folder3->setToolTip("open Folder 3");

    m_bookmarks_manager_Folder2 = sharedAction(new Action("Folder 2"));
    m_bookmarks_manager_Folder2->setToolTip("open Folder 2");

    m_bookmarks_manager_Folder1 = sharedAction(new Action("Folder 1"));
    m_bookmarks_manager_Folder1->setToolTip("open Folder 1");
}

void SimpleUI::connectActions()
{
    //left bar
    m_back->triggered.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::back, m_webEngine.get()));
    m_back->triggered.connect(boost::bind(&SimpleUI::updateBrowserView, this));
    m_forward->triggered.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::forward, m_webEngine.get()));
    m_stopLoading->triggered.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::stopLoading, m_webEngine.get()));
    m_reload->triggered.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::reload, m_webEngine.get()));
    m_reload->triggered.connect(boost::bind(&SimpleUI::updateBrowserView, this));

    //right bar
    m_tab->triggered.connect(boost::bind(&SimpleUI::showTabUI, this));
    m_showMoreMenu->triggered.connect(boost::bind(&SimpleUI::showMoreMenu, this));

//  m_settingPointerMode->toggled.connect(boost::bind(&tizen_browser::services::PlatformInputManager::setPointerModeEnabled, m_platformInputManager.get(), _1));
    m_settingPrivateBrowsing->toggled.connect(boost::bind(&SimpleUI::settingsPrivateModeSwitch, this, _1));
//  m_settingDeleteHistory->triggered.connect(boost::bind(&SimpleUI::settingsDeleteHistory, this));
//  m_settingDeleteData->triggered.connect(boost::bind(&SimpleUI::settingsDeleteData, this));;
//  m_settingDeleteFavorite->triggered.connect(boost::bind(&SimpleUI::settingsDeleteFavorite, this));;
}

void SimpleUI::updateURIBarView()
{
	m_simpleURI->changeUri(m_webEngine->getURI());
	leftButtonBar->setActionForButton("refresh_stop_button", m_reload);
	stopEnable(true);
	reloadEnable(true);
	hideProgressBar();
}

void SimpleUI::updateWebView()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(elm_object_part_content_get(m_mainLayout, "web_view"));
    elm_object_part_content_unset(m_mainLayout, "web_view");
    elm_object_part_content_set(m_mainLayout, "web_view", m_webEngine->getLayout());
    evas_object_show(m_webEngine->getLayout());
}

void SimpleUI::updateBrowserView()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
	if(isHomePageActive())
        hideMainUI();

	updateWebView();

	updateURIBarView();
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
    updateBrowserView();
}

bool SimpleUI::isHomePageActive()
{
    BROWSER_LOGD("[%s:%d] isHomePageActive : %d", __PRETTY_FUNCTION__, __LINE__, m_isHomePageActive);
    return m_isHomePageActive;
}

bool SimpleUI::isErrorPageActive()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return elm_object_part_content_get(m_mainLayout, "web_view") == m_errorLayout;
}

void SimpleUI::switchViewToHomePage()
{
#if MERGE_ME // remove
    if(!isHomePageActive()){
        evas_object_hide(elm_object_part_content_get(m_mainLayout,"web_view"));
        elm_object_part_content_unset(m_mainLayout, "web_view");
        elm_object_part_content_set(m_mainLayout, "web_view", m_bookmarksUI->getContent());
        evas_object_show(m_bookmarksUI->getContent());
        //m_simpleURI->changeUri(HomePageURL);
        filterURL(HomePageURL);
        }
#endif
    BROWSER_LOGD("[%s:%d] isHomePageActive : %d", __PRETTY_FUNCTION__, __LINE__, m_isHomePageActive);
    if(isHomePageActive())
	return;

    showMainUI();
    filterURL(HomePageURL);

    m_webEngine->disconnectCurrentWebViewSignals();

    leftButtonBar->setActionForButton("refresh_stop_button", m_reload);

    stopEnable(false);
    reloadEnable(false);
    forwardEnable(false);
    backEnable(false);
    evas_object_hide(leftButtonBar->getContent());
    elm_object_signal_emit(m_mainLayout, "shiftback_uri", "ui");
    elm_object_signal_emit(m_simpleURI->getContent(), "shiftback_uribg", "ui");

    webTitleBar->hide();

    hideProgressBar();
}

void SimpleUI::checkTabId(const tizen_browser::basic_webengine::TabId& id){
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if(m_webEngine->currentTabId() != id || isErrorPageActive()){
        BROWSER_LOGD("URL changed on non-visible tab, updating browser view");
        switchToTab(id);
    }
}

void SimpleUI::openNewTab(const std::string &uri)
{
    switchToTab(m_webEngine->addTab(uri));
}

void SimpleUI::closeTab(){
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    auto tabId = m_webEngine->currentTabId();
    closeTab(tabId);
}

void SimpleUI::closeTab(const tizen_browser::basic_webengine::TabId& id)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_currentSession.removeItem(id.toString());
    m_webEngine->closeTab(id);
    updateView();
}

void SimpleUI::bookmarkCheck()
{
    if (isHomePageActive())
        return;

    if(m_favoriteService->bookmarkExists(m_webEngine->getURI())){
        BROWSER_LOGD("[%s] There is bookmark for this site [%s], set indicator on!", __func__, m_webEngine->getURI().c_str());
        // MERGE_ME
        //leftButtonBar->setActionForButton("bookmark_button", m_unbookmark);
    }
    else{
        BROWSER_LOGD("[%s] There is no bookmark for this site [%s], set indicator off", __func__, m_webEngine->getURI().c_str());
        // MERGE_ME
        //leftButtonBar->setActionForButton("bookmark_button", m_bookmark);
        //addBookmarkEnable(m_favoriteService->countBookmarksAndSubFolders() < m_favoritesLimit);
    }
}

void SimpleUI::onBookmarkAdded(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
#if 0
    BROWSER_LOGI("Bookmark added with address %s",bookmarkItem->getAddress().c_str());
    bookmarkCheck();
    webTitleBar->removeFavIcon();
    webTitleBar->show("Added to favorites");
#endif
}

void SimpleUI::onHistoryAdded(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem)
{
#if 0
    m_mainUI->addHistoryItem(historyItem);
    BROWSER_LOGI("Bookmark added with address %s",historyItem->getUrl().c_str());
#endif
}

void SimpleUI::onHistoryClicked(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem)
{
    std::string historyAddress = historyItem->getUrl();
    if(m_historyUI) {                // TODO: remove this section when naviframes will be available
        m_historyUI->clearItems();
        m_historyUI = nullptr;
    }

    if(m_moreMenuUI) {               // TODO: remove this section when naviframes will be available
        m_moreMenuUI->clearItems();
        m_moreMenuUI = nullptr;
    }
    openNewTab(historyAddress);
}

void SimpleUI::onClearHistoryClicked(const std::string&)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_historyService->clearAllHistory();
    m_mainUI->clearHistoryGenlist();
    m_mainUI->showHistory();
}

void SimpleUI::onMostVisitedClicked(const std::string&)
{
   BROWSER_LOGD("[%s]", __func__);
   m_mainUI->clearHistoryGenlist();
   m_mainUI->clearBookmarkGengrid();
   m_mainUI->addHistoryItems(getHistory());
   m_mainUI->showHistory();
}

void SimpleUI::onBookmarkButtonClicked(const std::string&)
{
   BROWSER_LOGD("[%s]", __func__);
   m_mainUI->clearBookmarkGengrid();
   m_mainUI->clearHistoryGenlist();
   m_mainUI->addBookmarkItems(getBookmarks());
   m_mainUI->showBookmarks();
}

void SimpleUI::onBookmarkManagerButtonClicked(const std::string&)
{
    BROWSER_LOGD("[%s]", __func__);
    showBookmarkManagerMenu();
}

void SimpleUI::onBookmarkClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    std::string bookmarkAddress = bookmarkItem->getAddress();
    if(m_bookmarkManagerUI) {                // TODO: remove this section when naviframes will be available
        m_bookmarkManagerUI->clearItems();
        m_bookmarkManagerUI = nullptr;
    }

    if(m_moreMenuUI) {               // TODO: remove this section when naviframes will be available
        m_moreMenuUI->clearItems();
        m_moreMenuUI = nullptr;
    }
    openNewTab(bookmarkAddress);
}

void SimpleUI::onBookmarkDeleteClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    BROWSER_LOGD("[%s] delete %s", __func__, bookmarkItem->getAddress().c_str());
    m_favoriteService->deleteBookmark(bookmarkItem->getAddress());
}

void SimpleUI::onBookmarkRemoved(const std::string& uri)
{
#if 0
    BROWSER_LOGD("[%s] deleted %s", __func__, uri.c_str());
    bookmarkCheck();
    webTitleBar->removeFavIcon();
    webTitleBar->show("Removed from favorites");
#endif
}

void SimpleUI::onHistoryRemoved(const std::string& uri)
{
    BROWSER_LOGD("[%s] deleted %s", __func__, uri.c_str());
    //m_mainUI->removeHistoryItem(uri.c_str());
}

void SimpleUI::onReturnPressed(MenuButton *m)
{
    BROWSER_LOGD("[%s]", __func__);
    m_platformInputManager->returnPressed.disconnect_all_slots();
    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    hidePopup.disconnect_all_slots();
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
    if (!m_simpleURI->hasFocus() && !m_wvIMEStatus && !isHomePageActive() && m_back->isEnabled())
        m_webEngine->backButtonClicked();
}

void SimpleUI::backEnable(bool enable)
{
    m_back->setEnabled(enable);
}

void SimpleUI::forwardEnable(bool enable)
{
    m_forward->setEnabled(enable);
}

void SimpleUI::reloadEnable(bool enable)
{
    m_reload->setEnabled(enable);
}

void SimpleUI::stopEnable(bool enable)
{
    m_stopLoading->setEnabled(enable);
}

void SimpleUI::addBookmarkEnable(bool enable)
{
    //MERGE_ME
    //m_bookmark->setEnabled(enable);
}

void SimpleUI::removeBookmarkEnable(bool enable)
{
    //MERGE_ME
    //m_unbookmark->setEnabled(enable);
}
void SimpleUI::zoomEnable(bool enable)
{
    m_zoom_in->setEnabled(enable);
}

void SimpleUI::settingsButtonEnable(bool enable)
{
    m_showMoreMenu->setEnabled(enable);
}

void SimpleUI::loadStarted()
{
    BROWSER_LOGD("Switching \"reload\" to \"stopLoading\".");
    showProgressBar();
    elm_object_signal_emit(m_simpleURI->getContent(), "shiftright_uribg", "ui");
    elm_object_signal_emit(m_mainLayout, "shiftright_uri", "ui");
    evas_object_show(leftButtonBar->getContent());
    leftButtonBar->setActionForButton("refresh_stop_button", m_stopLoading);
    addBookmarkEnable(false);
    if(!m_webEngine->isPrivateMode()){
        m_currentSession.updateItem(m_webEngine->currentTabId().toString(), m_webEngine->getURI());
    }

#if 0
    if(!m_platformInputManager->getPointerModeEnabled())
        elm_object_focus_set(leftButtonBar->getButton("refresh_stop_button"), EINA_TRUE);
#endif
}

void SimpleUI::progressChanged(double progress)
{
    if(progress == 1.0){
        hideProgressBar();
    } else {
        elm_progressbar_value_set(m_progressBar,progress);
    }
}

void SimpleUI::loadFinished()
{
    elm_object_signal_emit(m_mainLayout, "hide_progressbar_bg", "ui");
    BROWSER_LOGD("Switching \"stopLoading\" to \"reload\".");

    leftButtonBar->setActionForButton("refresh_stop_button", m_reload);

    addBookmarkEnable(m_favoriteService->countBookmarksAndSubFolders() < m_favoritesLimit);

    if(m_webEngine->isLoadError()){
        loadError();
    }

    if(!m_webEngine->isPrivateMode()){
        m_historyService->addHistoryItem(std::make_shared<tizen_browser::services::HistoryItem> (m_webEngine->getURI(),
                                                                                                m_webEngine->getTitle(),
                                                                                                m_webEngine->getFavicon()));
    }
#if 0
    if(!m_platformInputManager->getPointerModeEnabled())
        elm_object_focus_set(leftButtonBar->getButton("refresh_stop_button"), EINA_TRUE);
#endif
}

void SimpleUI::loadError()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    hideWebView();
    elm_object_part_content_set(m_mainLayout, "web_view",m_errorLayout);
    //evas_object_show(m_errorLayout);
}

void SimpleUI::setErrorButtons()
{
//  leftButtonBar->setActionForButton("bookmark_button", m_bookmark);
    leftButtonBar->setActionForButton("refresh_stop_button", m_reload);
//  addBookmarkEnable(false);
    stopEnable(false);
    reloadEnable(true);
    forwardEnable(false);
    evas_object_hide(m_progressBar);
}

void SimpleUI::filterURL(const std::string& url)
{
    BROWSER_LOGD("[%s] url=%s", __func__, url.c_str());

    //check for special urls (like:  'about:home')
    //if there will be more addresses may be we should
    //create some kind of std::man<std::string url, bool *(doSomethingWithUrl)()>
    //and then just map[url]() ? m_webEngine->setURI(url) : /*do nothing*/;;
    if(/*url.empty() ||*/ url == HomePageURL){
        m_simpleURI->changeUri("");
    } else if (!url.empty()){

    //check if url is in favorites

    //check if url is in blocked

    //no filtering
        if (isHomePageActive())
            openNewTab(url);
        else
            m_webEngine->setURI(url);
    }
    m_simpleURI->clearFocus();
    //addBookmarkEnable(false);
}

void SimpleUI::webEngineURLChanged(const std::string& url)
{
    BROWSER_LOGD("webEngineURLChanged:%s", url.c_str());

    m_simpleURI->changeUri(url);
    m_simpleURI->clearFocus();
    bookmarkCheck();
}

void SimpleUI::showZoomMenu()
{
    if(!m_zoomList){
        BROWSER_LOGD("[%s:%d] - create ", __PRETTY_FUNCTION__, __LINE__);
        m_zoomList = std::make_shared<ZoomList>(m_window,
                                                rightButtonBar->getButton("zoom_in_button"));
        m_zoomList->addItem("300%", ZOOM_TYPE_300);
        m_zoomList->addItem("200%", ZOOM_TYPE_200);
        m_zoomList->addItem("150%", ZOOM_TYPE_150);
        m_zoomList->addItem("100%", ZOOM_TYPE_100);
        m_zoomList->addItem("75%", ZOOM_TYPE_75);
        m_zoomList->addItem("50%", ZOOM_TYPE_50);

        m_zoomList->zoomChanged.connect(boost::bind(&SimpleUI::zoomLevelChanged, this, _1));
    }

    zoom_type currentZoomType;

    currentZoomType = static_cast<zoom_type> (m_webEngine->getZoomFactor());

    //Handling Initial case
    if(m_webEngine->getZoomFactor() == 0){
        m_webEngine->setZoomFactor(100);
        m_currentZoom = static_cast<zoom_type> (100);
        currentZoomType = ZOOM_TYPE_100;
    }
    //each tab may have another zoom, but there is only one instance of zoomlist
    //so we should refresh value of zoom every time
    m_zoomList->setZoom(currentZoomType);
    BROWSER_LOGD("Current zoom factor from webkit %d%%", m_webEngine->getZoomFactor());

    m_platformInputManager->returnPressed.disconnect_all_slots();
    m_platformInputManager->returnPressed.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_zoomList.get()));
    hidePopup.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_zoomList.get()));
    m_zoomList->showPopup();
}

void SimpleUI::zoomLevelChanged(int zoom_level)
{
    m_webEngine->setZoomFactor(zoom_level);
    m_currentZoom = static_cast<zoom_type> (zoom_level);
}

void SimpleUI::AddBookmarkPopup(std::string& str)
{
   BROWSER_LOGD("[%s]", __func__);
   if(!m_bookmarks_manager){
        m_bookmarks_manager = std::make_shared<BookmarksManager>(m_window,
                                                rightButtonBar->getButton("bookmark_button"));
        m_bookmarks_manager->addAction( m_bookmarks_manager_Add_NewFolder);
        m_bookmarks_manager->addAction( m_bookmarks_manager_BookmarkBar);
        m_bookmarks_manager->addAction( m_bookmarks_manager_Folder1);
        m_bookmarks_manager->addAction( m_bookmarks_manager_Folder2);
        m_bookmarks_manager->addAction( m_bookmarks_manager_Folder3);
    }
    m_platformInputManager->returnPressed.disconnect_all_slots();
    m_platformInputManager->returnPressed.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_bookmarks_manager.get()));
    hidePopup.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_bookmarks_manager.get()));
    m_bookmarks_manager->showPopup();
}

void SimpleUI::AddNewFolderPopup(std::string& str)
{
   BROWSER_LOGD("[%s]", __func__);
}

void SimpleUI::showTabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_tabUI = std::dynamic_pointer_cast<tizen_browser::base_ui::TabUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.tabui"));
    M_ASSERT(m_tabUI);
    m_tabUI->closeTabUIClicked.connect(boost::bind(&SimpleUI::closeTabUI, this,_1));
    m_tabUI->newTabClicked.connect(boost::bind(&SimpleUI::newTabClicked, this,_1));
    m_tabUI->tabClicked.connect(boost::bind(&SimpleUI::tabClicked, this,_1));
    m_tabUI->newIncognitoTabClicked.connect(boost::bind(&SimpleUI::newTabClicked, this,_1));
    m_tabUI->show(m_window.get());
    m_tabUI->addTabItems(m_webEngine->getTabContents());
}

void SimpleUI::closeTabUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_tabUI = nullptr;
}

void SimpleUI::newTabClicked(const std::string& str)
{
    BROWSER_LOGD("%s", __func__);
    switchViewToHomePage();
}

void SimpleUI::tabClicked(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("%s", __func__);
    switchToTab(tabId);
}

void SimpleUI::handleConfirmationRequest(basic_webengine::WebConfirmationPtr webConfirmation)
{
    BROWSER_LOGD("%s", __func__);
    switch(webConfirmation->getConfirmationType())
    {
        case basic_webengine::WebConfirmation::ConfirmationType::Authentication:
        {
        basic_webengine::AuthenticationConfirmationPtr auth = std::dynamic_pointer_cast<basic_webengine::AuthenticationConfirmation, basic_webengine::WebConfirmation>(webConfirmation);

        Evas_Object *popup_content = elm_layout_add(m_mainLayout);
        std::string edjFilePath = EDJE_DIR;
        edjFilePath.append("SimpleUI/AuthenticationPopup.edj");
        Eina_Bool layoutSetResult = elm_layout_file_set(popup_content, edjFilePath.c_str(), "authentication_popup");
        if(!layoutSetResult)
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

#if 0
void SimpleUI::hideURIBar()
{
    evas_object_hide(m_simpleURI->getContent(m_mainLayout));
        evas_object_hide(rightButtonBar->getContent());
        evas_object_hide(leftButtonBar->getContent());
    elm_object_part_content_unset(m_mainLayout, "uri_entry");
        elm_object_part_content_unset(m_mainLayout, "uri_bar_buttons_left");
        elm_object_part_content_unset(m_mainLayout, "uri_bar_buttons_right");
}

void SimpleUI::showURIBar()
{
    elm_object_part_content_unset(m_mainLayout, "uri_entry");
        elm_object_part_content_unset(m_mainLayout, "uri_bar_buttons_left");
        elm_object_part_content_unset(m_mainLayout, "uri_bar_buttons_right");
    elm_object_part_content_set(m_mainLayout, "uri_entry", m_simpleURI->getContent(m_mainLayout));
        elm_object_part_content_set(m_mainLayout, "uri_bar_buttons_left", leftButtonBar->getContent());
        elm_object_part_content_set(m_mainLayout, "uri_bar_buttons_right", rightButtonBar->getContent());
    evas_object_show(m_simpleURI->getContent(m_mainLayout));
    evas_object_show(leftButtonBar->getContent());
    evas_object_show(rightButtonBar->getContent());
    m_simpleURI->setFocus();
}
#endif

void SimpleUI::hideWebView()
{
	evas_object_hide(elm_object_part_content_get(m_mainLayout,"web_view"));
        elm_object_part_content_unset(m_mainLayout, "web_view");
}

void SimpleUI::hideMainUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_mainUI)
        m_mainUI->hide();
    m_mainUI = nullptr;
    m_isHomePageActive = false;
}

void SimpleUI::showMainUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_mainUI = std::dynamic_pointer_cast
                <tizen_browser::base_ui::MainUI,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.mainui"));
    M_ASSERT(m_mainUI);
    hideWebView();
    m_mainUI->show(m_window.get());
    m_mainUI->addHistoryItems(getHistory());
    m_mainUI->addBookmarkItems(getBookmarks());
    m_isHomePageActive = true;
}

void SimpleUI::showHistoryUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyUI =
    std::dynamic_pointer_cast<tizen_browser::base_ui::HistoryUI,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.historyui"));
    M_ASSERT(m_historyUI);
    m_historyUI->clearHistoryClicked.connect(boost::bind(&SimpleUI::onClearHistoryClicked, this,_1));
    m_historyUI->closeHistoryUIClicked.connect(boost::bind(&SimpleUI::closeHistoryUI, this,_1));
    m_historyUI->historyItemClicked.connect(boost::bind(&SimpleUI::onHistoryClicked, this,_1));
    m_historyUI->addHistoryItems(getHistory());
    m_historyUI->show(m_window.get());
}

void SimpleUI::closeHistoryUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyUI = nullptr;
}

void SimpleUI::showSettingsUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_settingsUI){
        m_settingsUI =
                std::dynamic_pointer_cast
                <tizen_browser::base_ui::SettingsUI,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.settingsui"));
        M_ASSERT(m_settingsUI);
        m_settingsUI->closeSettingsUIClicked.disconnect_all_slots();
        m_settingsUI->closeSettingsUIClicked.connect(boost::bind(&SimpleUI::closeSettingsUI, this,_1));
        m_settingsUI->deleteSelectedDataClicked.disconnect_all_slots();
        m_settingsUI->deleteSelectedDataClicked.connect(boost::bind(&SimpleUI::settingsDeleteSelectedData, this,_1));
        m_settingsUI->resetMostVisitedClicked.disconnect_all_slots();
        m_settingsUI->resetMostVisitedClicked.connect(boost::bind(&SimpleUI::settingsResetMostVisited, this,_1));
        m_settingsUI->resetBrowserClicked.disconnect_all_slots();
        m_settingsUI->resetBrowserClicked.connect(boost::bind(&SimpleUI::settingsResetBrowser, this,_1));
        m_settingsUI->show(m_window.get());
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    }
}

void SimpleUI::closeSettingsUI(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_settingsUI.reset();
}

void SimpleUI::showMoreMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bool current_tab_as_new_tab = isHomePageActive() || (m_historyService->getHistoryItemsCount() == 0);
    if(!m_moreMenuUI){
        m_moreMenuUI =
                std::dynamic_pointer_cast
                <tizen_browser::base_ui::MoreMenuUI,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.moremenuui"));
        M_ASSERT(m_moreMenuUI);
        m_webEngine->favIconChanged.disconnect_all_slots();
        m_webEngine->favIconChanged.connect(boost::bind(&MoreMenuUI::setFavIcon, m_moreMenuUI.get(), _1));
        m_webEngine->titleChanged.disconnect_all_slots();
        m_webEngine->titleChanged.connect(boost::bind(&MoreMenuUI::setWebTitle, m_moreMenuUI.get(), _1));
        m_webEngine->uriChanged.disconnect_all_slots();
        m_webEngine->uriChanged.connect(boost::bind(&MoreMenuUI::setURL, m_moreMenuUI.get(), _1));
        m_moreMenuUI->bookmarkManagerClicked.connect(boost::bind(&SimpleUI::onBookmarkManagerButtonClicked, this, _1));
        m_moreMenuUI->historyUIClicked.connect(boost::bind(&SimpleUI::showHistoryUI, this,_1));
        m_moreMenuUI->settingsClicked.connect(boost::bind(&SimpleUI::showSettingsUI, this,_1));
        m_moreMenuUI->closeMoreMenuClicked.connect(boost::bind(&SimpleUI::closeMoreMenu, this,_1));
        m_moreMenuUI->addToBookmarkClicked.connect(boost::bind(&SimpleUI::addBookmarkFolders, this));
        m_moreMenuUI->AddBookmarkInput.connect(boost::bind(&SimpleUI::addToBookmarks, this,_1));
        m_moreMenuUI->BookmarkFolderCreated.connect(boost::bind(&SimpleUI::newFolderMoreMenu, this,_1,_2));

        m_moreMenuUI->show(m_window.get());
        m_moreMenuUI->showCurrentTab();
        m_moreMenuUI->setFavIcon(m_webEngine->getFavicon());
        m_moreMenuUI->setWebTitle(m_webEngine->getTitle());
        m_moreMenuUI->setURL(m_webEngine->getURI());
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    }
}

void SimpleUI::closeMoreMenu(const std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_moreMenuUI.reset();
}

void SimpleUI::showBookmarkManagerMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_bookmarkManagerUI){
        m_bookmarkManagerUI =
                std::dynamic_pointer_cast
                <tizen_browser::base_ui::BookmarkManagerUI,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarkmanagerui"));
        M_ASSERT(m_bookmarkManagerUI);
        m_bookmarkManagerUI->closeBookmarkManagerClicked.disconnect_all_slots();
        m_bookmarkManagerUI->closeBookmarkManagerClicked.connect(boost::bind(&SimpleUI::closeBookmarkManagerMenu, this,_1));
        m_bookmarkManagerUI->saveFolderClicked.disconnect_all_slots();
        m_bookmarkManagerUI->saveFolderClicked.connect(boost::bind(&SimpleUI::newFolderBookmarkManager, this,_1,_2));
        m_bookmarkManagerUI->bookmarkItemClicked.disconnect_all_slots();
        m_bookmarkManagerUI->bookmarkItemClicked.connect(boost::bind(&SimpleUI::onBookmarkClicked, this, _1));
        m_bookmarkManagerUI->folderItemClicked.disconnect_all_slots();
        m_bookmarkManagerUI->folderItemClicked.connect(boost::bind(&SimpleUI::updateBookmarkManagerGenGrid, this,_1));
        m_bookmarkManagerUI->show(m_window.get());
        m_bookmarkManagerUI->addBookmarkFolderItems(getBookmarkFolders(ROOT_FOLDER));
        m_bookmarkManagerUI->showTopContent();
        m_curr_folder_id = ROOT_FOLDER;
    }
}

void SimpleUI::updateBookmarkManagerGenGrid(int folder_id)
{
    m_bookmarkManagerUI->updateGengrid();
    m_bookmarkManagerUI->addBookmarkFolderItems(getBookmarkFolders(folder_id));
    m_bookmarkManagerUI->addBookmarkItems(getBookmarks(folder_id));
    m_bookmarkManagerUI->showTopContent();
    m_curr_folder_id = folder_id;
}

void SimpleUI::closeBookmarkManagerMenu(std::string& str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_bookmarkManagerUI.reset();
}

void SimpleUI::openLinkFromPopup(const std::string &uri)
{
    filterURL(uri);
    hidePopup();
}

void SimpleUI::hideHistory()
{
    hidePopup();
}

void SimpleUI::settingsPointerModeSwitch(bool newState)
{
    BROWSER_LOGD("%s: Setting Pointer mode to:%s", __func__, (newState ? "true" : "false"));
}

void SimpleUI::settingsPrivateModeSwitch(bool newState)
{
    BROWSER_LOGD("%s: Setting Private mode to: %s", __func__, (newState ? "true" : "false"));
    m_webEngine->setPrivateMode(newState);
    BROWSER_LOGD("[%s:%d] webEngine private mode: %s", __PRETTY_FUNCTION__, __LINE__, (m_webEngine->isPrivateMode() ? "true" : "false"));
}

void SimpleUI::settingsDeleteSelectedData(const std::string& str)
{
    BROWSER_LOGD("[%s]: Deleting Hisory", __func__);
    SimplePopup *popup = SimplePopup::createPopup();
    popup->setTitle("Delete selected data");
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->setMessage("Are you sure you want to delete all selected data?");
    std::shared_ptr<EntryPopupData> popupData = std::make_shared<EntryPopupData>();
    popupData->text = str;
    popup->setData(popupData);
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteSelectedDataButton, this, _1, _2));
    popup->show();
}

void SimpleUI::onDeleteSelectedDataButton(PopupButtons button, std::shared_ptr< PopupData > popupData)
{
    if(button == OK){
        BROWSER_LOGD("[%s]: OK", __func__);
	std::string dataText = std::static_pointer_cast<EntryPopupData>(popupData)->text;
	BROWSER_LOGD("[%s]: TYPE : %s", __func__, dataText.c_str());
	if (dataText.find("CACHE") != std::string::npos)
		m_webEngine->clearPrivateData();
	if (dataText.find("COOKIES") != std::string::npos)
		m_webEngine->clearPrivateData();
	if (dataText.find("HISTORY") != std::string::npos)
		m_historyService->clearAllHistory();
        //webTitleBar->removeFavIcon();
        //webTitleBar->show("History deleted");
    }
}

void SimpleUI::settingsResetMostVisited(const std::string& str)
{
    BROWSER_LOGD("[%s]: Deleting Hisory", __func__);
    SimplePopup *popup = SimplePopup::createPopup();
    popup->setTitle("Delete most visited");
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->setMessage("Are you sure you want to delete most visited sites?");
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteMostVisitedButton, this, _1, _2));
    popup->show();
}

void SimpleUI::onDeleteMostVisitedButton(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if(button == OK){
        BROWSER_LOGD("[%s]: OK", __func__);
        BROWSER_LOGD("[%s]: Deleting most visited", __func__);
        /*m_historyService->clearAllHistory();
        webTitleBar->removeFavIcon();
        webTitleBar->show("History deleted");*/
    }
}

void SimpleUI::settingsResetBrowser(const std::string& str)
{
    BROWSER_LOGD("[%s]: Deleting Hisory", __func__);
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
    if(button == OK){
        BROWSER_LOGD("[%s]: OK", __func__);
        BROWSER_LOGD("[%s]: Resetting browser", __func__);
        /*webTitleBar->removeFavIcon();
        webTitleBar->show("History deleted");*/
    }
}

void SimpleUI::tabLimitPopupButtonClicked(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if(button == CLOSE_TAB){
        BROWSER_LOGD("[%s]: CLOSE TAB", __func__);
        closeTab();
    }
}

void SimpleUI::tabCreated()
{
    int tabs = m_webEngine->tabsCount();

    if(tabs > m_tabLimit)
    {
        SimplePopup *popup = SimplePopup::createPopup();
        popup->setTitle("Too many tabs open");
        popup->addButton(CONTINUE);
        popup->addButton(CLOSE_TAB);
        popup->setMessage("Browser might slow down. Are you sure you want to continue?");
        popup->buttonClicked.connect(boost::bind(&SimpleUI::tabLimitPopupButtonClicked, this, _1, _2));
        popup->show();
    }
    elm_object_part_text_set(rightButtonBar->getContent(), "tabs_number", (boost::format("%1%") % tabs).str().c_str());
}

void SimpleUI::updateView() {
    int tabs = m_webEngine->tabsCount();
    BROWSER_LOGD("[%s] Opened tabs: %d", __func__, tabs);
    if (tabs == 0) {
        switchViewToHomePage();
        elm_object_part_text_set(rightButtonBar->getContent(), "tabs_number", "");
    } else {
        if (!isHomePageActive()) {
            updateBrowserView();
        }
        elm_object_part_text_set(rightButtonBar->getContent(), "tabs_number", (boost::format("%1%") % tabs).str().c_str());
    }
}

void SimpleUI::tabClosed(const tizen_browser::basic_webengine::TabId& id) {
    updateView();
}

void SimpleUI::onNetworkError()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_networkErrorPopup){
        m_networkErrorPopup = SimplePopup::createPopup();
        m_networkErrorPopup->setTitle("Network Error");
        m_networkErrorPopup->addButton(CONNECT);
        m_networkErrorPopup->addButton(CANCEL);
        m_networkErrorPopup->setMessage("Network is disconnected. Please check the connection.");
        m_networkErrorPopup->buttonClicked.connect(boost::bind(&SimpleUI::onNetErrorButtonPressed, this, _1, _2));
        m_networkErrorPopup->show();
    }
}

void SimpleUI::onNetErrorButtonPressed(PopupButtons, std::shared_ptr< PopupData >)
{
    m_networkErrorPopup = 0;
}

void SimpleUI::onNetworkConnected()
{
    if(m_networkErrorPopup){
        delete m_networkErrorPopup;
        m_networkErrorPopup = 0;
    }
}


void SimpleUI::showProgressBar()
{
    elm_object_signal_emit(m_mainLayout, "show_progressbar_bg", "ui");
    elm_object_part_content_set(m_mainLayout,"progress_bar",m_progressBar);
}

void SimpleUI::hideProgressBar()
{
    elm_object_signal_emit(m_mainLayout, "hide_progressbar_bg", "ui");
    elm_progressbar_value_set(m_progressBar,0.0);
    elm_object_part_content_unset(m_mainLayout,"progress_bar");
    evas_object_hide(m_progressBar);
}

void SimpleUI::searchWebPage(std::string &text, int flags)
{
    m_webEngine->searchOnWebsite(text, flags);
}

void SimpleUI::favicon_clicked(void *data, Evas_Object */*obj*/, const char */*emission*/, const char */*source*/)
{
    BROWSER_LOGD("[%s],", __func__);
    SimpleUI *self = reinterpret_cast<SimpleUI*>(data);
    if (!self->isHomePageActive() && !self->isErrorPageActive())
    {
        self->m_simpleURI->clearFocus();
    }
}

void SimpleUI::addToBookmarks(int folder_id)
{
    BROWSER_LOGD("[%s,%d],", __func__, __LINE__);
	if (m_favoriteService)
	    {   if( m_webEngine && !m_webEngine->getURI().empty())
                  {  m_favoriteService->addToBookmarks(m_webEngine->getURI(), m_webEngine->getTitle(), std::string(),
										  m_webEngine->getSnapshotData(373, 240),
		   								  m_webEngine->getFavicon(),(unsigned int)folder_id);
                 }
        }
}

void SimpleUI::addBookmarkFolders(void)
{
    if(m_moreMenuUI)
        m_moreMenuUI->getBookmarkFolderList(getBookmarkFolders(ROOT_FOLDER));
}


void SimpleUI::newFolderBookmarkManager(const char* title, int by_operator)
{
    BROWSER_LOGD("[%s,%d],", __func__, __LINE__);
    int id = -1;
	if (m_favoriteService)
		m_favoriteService->save_folder(title, &id, by_operator);
    if (id >= 0 )
    {
        BROWSER_LOGD("[%s], Added New Folder", __func__);
        if(m_bookmarkManagerUI)
        {
            updateBookmarkManagerGenGrid(m_curr_folder_id);   // TODO: correct folder displaying behaviour needs to be implemented
        }
    }
}

void SimpleUI::newFolderMoreMenu(const char* title, int by_operator)
{
    int id = -1;
	if (m_favoriteService)
        m_favoriteService->save_folder(title, &id, by_operator);
    if (id >= 0)
    {
        BROWSER_LOGD("[%s], Added New Folder", __func__);
        addToBookmarks(id);
    }
}

void SimpleUI::deleteBookmark(void)
{
	if (m_favoriteService)
		m_favoriteService->deleteBookmark(m_webEngine->getURI());
}
}
}
