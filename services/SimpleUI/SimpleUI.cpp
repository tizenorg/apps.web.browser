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
//#include <EWebKit2.h>
//#include <ewk_chromium.h>
#endif

#include <boost/format.hpp>
#include <boost/any.hpp>
#include <memory>
#include <algorithm>
#include <Elementary.h>
#include <Ecore.h>
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
#include "../BookmarksUI/BookmarksUI.h"
#include "BookmarkItem.h"
#include "Tools/EflTools.h"
#include "BrowserImage.h"
#include "HistoryItem.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "NetworkErrorHandler.h"
#include "SqlStorage.h"


namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SimpleUI, "org.tizen.browser.simpleui")

const std::string HomePageURL = "about:home";

SimpleUI::SimpleUI(/*Evas_Object *window*/)
    : AbstractMainWindow()
    , m_mainLayout(NULL)
    , m_progressBar(NULL)
    , m_popup(NULL)
    , m_settings()
    , m_initialised(false)
    , m_currentZoom(ZOOM_TYPE_100)
    , items_vector()
    , m_networkErrorPopup(0)
{
    elm_init(static_cast<int>(NULL), static_cast<char**>(NULL));
    Evas_Object *main_window = elm_win_util_standard_add("browserApp", "browserApp");
    if (main_window == NULL)
        BROWSER_LOGE("Failed to create main window");
//    m_zoomList = NULL;
    setMainWindow(main_window);
}

SimpleUI::~SimpleUI() {
    m_sessionService->getStorage()->deleteSession(m_currentSession);
    /// \todo Auto-generated destructor stub
    evas_object_del(m_window.get());
}

void SimpleUI::destroyUI()
{
    evas_object_del(m_window.get());
}

std::string SimpleUI::edjePath(const std::string &file)
{
    return std::string(EDJE_DIR) + file;
}

std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > SimpleUI::getBookmarks()
{
    return m_favoriteService->getBookmarks();
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

            // creatin main window
            //int width = 1920;
            //int height = 1080;
            //ecore_x_window_size_get(ecore_x_window_root_first_get(), &width, &height);
            //evas_object_move(m_window.get(), 0, 0);
            //evas_object_resize(m_window.get(), width, height);

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
            m_bookmarksUI =
                std::dynamic_pointer_cast
                <tizen_browser::base_ui::BookmarksUI,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.bookmarksui"));
            M_ASSERT(m_bookmarksUI);
            m_bookmarksUI->init(m_mainLayout);

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
//             m_favoriteService->synchronizeBookmarks();
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

/*
            // Platforminputmanager
            BROWSER_LOGD("[%s:%d] service: platforminputmanager ", __PRETTY_FUNCTION__, __LINE__);
            m_platformInputManager =
                std::dynamic_pointer_cast
                <tizen_browser::services::PlatformInputManager,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.platforminputmanager"));
            M_ASSERT(m_platformInputManager);
            m_platformInputManager->init(m_window.get());
            m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
*/
            createActions();
            // left buttons
            leftButtonBar = std::make_shared<ButtonBar>(m_mainLayout, "SimpleUI/LeftButtonBar.edj", "left_button_bar");
            leftButtonBar->addAction(m_back, "prev_button");
            leftButtonBar->addAction(m_forward, "next_button");
            leftButtonBar->addAction(m_reload, "refresh_stop_button");
            leftButtonBar->addAction(m_bookmark, "bookmark_button");
            //register action that will be used later by buttons
            leftButtonBar->registerEnabledChangedCallback(m_stopLoading, "refresh_stop_button");
            leftButtonBar->registerEnabledChangedCallback(m_unbookmark, "bookmark_button");

            // right buttons
            rightButtonBar = std::make_shared<ButtonBar>(m_mainLayout, "SimpleUI/RightButtonBar.edj", "right_button_bar");
            rightButtonBar->addAction(m_tab, "tab_button");
            rightButtonBar->addAction(m_history, "history_button");
            rightButtonBar->addAction(m_zoom_in, "zoom_in_button");
            rightButtonBar->addAction(m_showSettingsPopup, "setting_button");

            m_progressBar = elm_progressbar_add(m_mainLayout);
            elm_object_style_set(m_progressBar,"play_buffer");

            webTitleBar = std::make_shared<WebTitleBar>(m_mainLayout, "SimpleUI/WebTitleBar.edj", "web_title_bar");
            elm_object_part_content_set(m_mainLayout, "web_title_bar", webTitleBar->getContent());

            // put components on layout
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
            m_webEngine->favIconChanged.connect(boost::bind(&SimpleURI::setFavIcon, m_simpleURI.get(), _1));
            m_webEngine->favIconChanged.connect(boost::bind(&WebTitleBar::setFavIcon, webTitleBar.get(), _1));
            m_webEngine->titleChanged.connect(boost::bind(&WebTitleBar::show, webTitleBar.get(), _1));

            m_favoriteService->bookmarkAdded.connect(boost::bind(&SimpleUI::onBookmarkAdded, this,_1));
            m_favoriteService->bookmarkDeleted.connect(boost::bind(&SimpleUI::onBookmarkRemoved, this, _1));
            m_favoriteService->bookmarksDeleted.connect(boost::bind(&tizen_browser::base_ui::BookmarksUI::deleteAllItems, m_bookmarksUI.get()));

            m_bookmarksUI->bookmarkClicked.connect(boost::bind(&SimpleUI::onBookmarkClicked, this,_1));
            m_bookmarksUI->bookmarkDeleteClicked.connect(boost::bind(&SimpleUI::onBookmarkDeleteClicked, this,_1));

            m_historyService->historyEmpty.connect(boost::bind(&SimpleUI::disableHistoryButton, this, _1));

            connectActions();

            elm_layout_signal_callback_add(m_simpleURI->getContent(), "slide_websearch", "elm", SimpleUI::favicon_clicked, this);

            // add bookmarks for main screen
            m_bookmarksUI->addBookmarkItems(getBookmarks());

            // show main layout and window
            evas_object_show(m_mainLayout);
            evas_object_show(m_window.get());

            m_netErrorHandler = std::unique_ptr<tizen_browser::basic_ui::NetworkErrorHandler>(new tizen_browser::basic_ui::NetworkErrorHandler);
            m_netErrorHandler->networkError.connect(boost::bind(&SimpleUI::onNetworkError, this));
            m_netErrorHandler->networkConnected.connect(boost::bind(&SimpleUI::onNetworkConnected, this));



            m_searchBox = std::make_shared<tizen_browser::base_ui::SearchBox>(m_window.get());
            m_searchBox->textChanged.connect(boost::bind(&SimpleUI::searchWebPage, this, _1, _2));
            elm_object_part_content_set(m_mainLayout, "search_box", m_searchBox->getContent());

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

        // only when first run
        if (url.empty()) {
            BROWSER_LOGD("[%s]: changing to homeUrl", __func__);
            switchViewToHomePage();
            filterURL(HomePageURL);
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
    }

    BROWSER_LOGD("[%s]:%d url=%s", __func__, __LINE__, url.c_str());

    m_simpleURI->setFocus();

    return 0;
}

void SimpleUI::loadThemes()
{
    elm_theme_extension_add(NULL, edjePath("SimpleUI/ZoomItem.edj").c_str());
    elm_theme_extension_add(NULL, edjePath("SimpleUI/TabItem.edj").c_str());
    elm_theme_extension_add(NULL, edjePath("SimpleUI/ErrorMessage.edj").c_str());
    elm_theme_extension_add(NULL, edjePath("SimpleUI/SearchBox.edj").c_str());

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

    m_bookmark = sharedAction(new Action("Bookmark"));
    m_bookmark->setToolTip("Add to favorite");
    m_bookmark->setIcon("browser/toolbar_bookmark");

    m_unbookmark = sharedAction(new Action("Unbookmark"));
    m_unbookmark->setToolTip("Unfavorite");
    m_unbookmark->setIcon("browser/toolbar_unbookmark");

    m_tab = sharedAction(new Action("Tabs"));
    m_tab->setToolTip("Tab page");
    m_tab->setIcon("browser/toolbar_tab");
    //m_tab->setCheckable(true);

    m_history = sharedAction(new Action("History"));
    m_history->setToolTip("History");
    m_history->setIcon("browser/toolbar_history");
    if(m_historyService->getHistoryItemsCount() == 0){
        m_history->setEnabled(false);
    }

    m_zoom_in = sharedAction(new Action("Zoom in"));
    m_zoom_in->setToolTip("Zoom in/out");
    m_zoom_in->setIcon("browser/toolbar_zoom_in");


    m_showSettingsPopup = sharedAction(new Action("Settings"));
    m_showSettingsPopup->setToolTip("Settings");
    m_showSettingsPopup->setIcon("browser/toolbar_setting");
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
}

void SimpleUI::connectActions()
{
    //left bar
    m_back->triggered.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::back, m_webEngine.get()));
    m_back->triggered.connect(boost::bind(&SimpleUI::switchViewToBrowser, this));
    m_forward->triggered.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::forward, m_webEngine.get()));
    m_stopLoading->triggered.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::stopLoading, m_webEngine.get()));
    m_reload->triggered.connect(boost::bind(&tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>::reload, m_webEngine.get()));
    m_reload->triggered.connect(boost::bind(&SimpleUI::switchViewToBrowser, this));

    m_bookmark->triggered.connect(boost::bind(&SimpleUI::addToBookmarks, this));
    m_unbookmark->triggered.connect(boost::bind(&SimpleUI::deleteBookmark, this));

    //right bar
    m_tab->triggered.connect(boost::bind(&SimpleUI::showTabMenu, this));
    m_zoom_in->triggered.connect(boost::bind(&SimpleUI::showZoomMenu, this));
    m_history->triggered.connect(boost::bind(&SimpleUI::showHistory, this));
    m_showSettingsPopup->triggered.connect(boost::bind(&SimpleUI::showSettingsMenu, this));

//    m_settingPointerMode->toggled.connect(boost::bind(&tizen_browser::services::PlatformInputManager::setPointerModeEnabled, m_platformInputManager.get(), _1));
    m_settingPrivateBrowsing->toggled.connect(boost::bind(&SimpleUI::settingsPrivateModeSwitch, this, _1));
    m_settingDeleteHistory->triggered.connect(boost::bind(&SimpleUI::settingsDeleteHistory, this));
    m_settingDeleteData->triggered.connect(boost::bind(&SimpleUI::settingsDeleteData, this));;
    m_settingDeleteFavorite->triggered.connect(boost::bind(&SimpleUI::settingsDeleteFavorite, this));;
}

void SimpleUI::updateBrowserView()
{
    evas_object_hide(elm_object_part_content_get(m_mainLayout, "web_view"));
    elm_object_part_content_unset(m_mainLayout, "web_view");
    elm_object_part_content_set(m_mainLayout, "web_view", m_webEngine->getLayout());
    evas_object_show(m_webEngine->getLayout());
}

void SimpleUI::switchToTab(const tizen_browser::basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_webEngine->currentTabId() != tabId) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        m_webEngine->switchToTab(tabId);
        switchViewToBrowser();
    }
//     if(!m_webEngine->isLoadError()){
//         BROWSER_LOGD("[%s:%d] swiching to web_view ", __PRETTY_FUNCTION__, __LINE__);
//         updateBrowserView();
//         switchViewToBrowser();
//     } else {
//         BROWSER_LOGD("[%s:%d] LOAD ERROR!", __PRETTY_FUNCTION__, __LINE__);
//         loadError();
//     }

    if(m_webEngine->isLoadError()){
        BROWSER_LOGD("[%s:%d] LOAD ERROR!", __PRETTY_FUNCTION__, __LINE__);
        loadError();
    } else {
        BROWSER_LOGD("[%s:%d] swiching to web_view ", __PRETTY_FUNCTION__, __LINE__);
        updateBrowserView();
        switchViewToBrowser();
    }
}

bool SimpleUI::isHomePageActive()
{
    BROWSER_LOGD("[%s]:%d", __func__, elm_object_part_content_get(m_mainLayout,"web_view") == m_bookmarksUI->getContent());
    return elm_object_part_content_get(m_mainLayout,"web_view") == m_bookmarksUI->getContent();
}

bool SimpleUI::isErrorPageActive()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return elm_object_part_content_get(m_mainLayout, "web_view") == m_errorLayout;
}


void SimpleUI::switchViewToHomePage()
{
    if(!isHomePageActive()){
        evas_object_hide(elm_object_part_content_get(m_mainLayout,"web_view"));
        elm_object_part_content_unset(m_mainLayout, "web_view");
        elm_object_part_content_set(m_mainLayout, "web_view", m_bookmarksUI->getContent());
        evas_object_show(m_bookmarksUI->getContent());
        //m_simpleURI->changeUri(HomePageURL);
        filterURL(HomePageURL);

    }

    m_webEngine->disconnectCurrentWebViewSignals();


    leftButtonBar->setActionForButton("bookmark_button", m_bookmark);
    leftButtonBar->setActionForButton("refresh_stop_button", m_reload);

    zoomEnable(false);
    addBookmarkEnable(false);
    removeBookmarkEnable(false);
    stopEnable(false);
    reloadEnable(false);
    forwardEnable(false);
    backEnable(false);

    m_simpleURI->setSearchIcon();
    webTitleBar->hide();
    hideSearchBox();


    hideProgressBar();
}

void SimpleUI::switchViewToBrowser()
{
    if (isHomePageActive() || isErrorPageActive()) {
        updateBrowserView();
    }

    m_simpleURI->changeUri(m_webEngine->getURI());
    leftButtonBar->setActionForButton("refresh_stop_button", m_reload);
    zoomEnable(true);
    removeBookmarkEnable(true);
    stopEnable(true);
    reloadEnable(true);
    bookmarkCheck();

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
    auto tabId = m_webEngine->currentTabId();
    closeTab(tabId);
}

void SimpleUI::closeTab(const tizen_browser::basic_webengine::TabId& id)
{

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
        leftButtonBar->setActionForButton("bookmark_button", m_unbookmark);
    }
    else{
        BROWSER_LOGD("[%s] There is no bookmark for this site [%s], set indicator off", __func__, m_webEngine->getURI().c_str());
        leftButtonBar->setActionForButton("bookmark_button", m_bookmark);
        addBookmarkEnable(m_favoriteService->countBookmarksAndSubFolders() < m_favoritesLimit);
    }
}

void SimpleUI::onBookmarkAdded(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    m_bookmarksUI->addBookmarkItem(bookmarkItem);
    BROWSER_LOGI("Bookmark added with address %s",bookmarkItem->getAddress().c_str());
    bookmarkCheck();
    webTitleBar->removeFavIcon();
    webTitleBar->show("Added to favorites");
}

void SimpleUI::onBookmarkClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    std::string bookmarkAddress = bookmarkItem->getAddress();
    openNewTab(bookmarkAddress);
}

void SimpleUI::onBookmarkDeleteClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem)
{
    BROWSER_LOGD("[%s] delete %s", __func__, bookmarkItem->getAddress().c_str());
    m_favoriteService->deleteBookmark(bookmarkItem->getAddress());
}

void SimpleUI::onBookmarkRemoved(const std::string& uri)
{
    BROWSER_LOGD("[%s] deleted %s", __func__, uri.c_str());
    m_bookmarksUI->removeBookmarkItem(uri.c_str());
    bookmarkCheck();

    webTitleBar->removeFavIcon();
    webTitleBar->show("Removed from favorites");
}
/*
void SimpleUI::onReturnPressed(MenuButton *m)
{
    BROWSER_LOGD("[%s]", __func__);
    m_platformInputManager->returnPressed.disconnect_all_slots();
    m_platformInputManager->returnPressed.connect(boost::bind(&elm_exit));
    hidePopup.disconnect_all_slots();
    m->hidePopup();
}
*/
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
    m_bookmark->setEnabled(enable);
}

void SimpleUI::removeBookmarkEnable(bool enable)
{
    m_unbookmark->setEnabled(enable);
}
void SimpleUI::zoomEnable(bool enable)
{
    m_zoom_in->setEnabled(enable);
}


void SimpleUI::settingsButtonEnable(bool enable)
{
    m_showSettingsPopup->setEnabled(enable);
}


void SimpleUI::loadStarted()
{
    BROWSER_LOGD("Switching \"reload\" to \"stopLoading\".");
    showProgressBar();
    leftButtonBar->setActionForButton("refresh_stop_button", m_stopLoading);
    addBookmarkEnable(false);
    zoomEnable(false);
    if(!m_webEngine->isPrivateMode()){
        m_currentSession.updateItem(m_webEngine->currentTabId().toString(), m_webEngine->getURI());
    }

    //if(!m_platformInputManager->getPointerModeEnabled())
    //    elm_object_focus_set(leftButtonBar->getButton("refresh_stop_button"), EINA_TRUE);
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

    zoomEnable(true);

    if(m_simpleURI->getCurrentIconTyep() != SimpleURI::IconTypeFav){
        m_simpleURI->setFavIcon(m_webEngine->getFavicon());
        webTitleBar->setFavIcon(m_webEngine->getFavicon());
    }
    if(m_webEngine->isLoadError()){
        loadError();
    }

    if(!m_webEngine->isPrivateMode()){
        m_historyService->addHistoryItem(std::make_shared<tizen_browser::services::HistoryItem> (m_webEngine->getURI(),
                                                                                                m_webEngine->getTitle(),
                                                                                                m_webEngine->getFavicon()));
        m_history->setEnabled(true);
    }
/*
    if(!m_platformInputManager->getPointerModeEnabled())
        elm_object_focus_set(leftButtonBar->getButton("refresh_stop_button"), EINA_TRUE);
*/
}

void SimpleUI::loadError()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
//     if(!isHomePageActive()){
//         evas_object_hide(elm_object_part_content_get(m_mainLayout,"web_view"));
//         elm_object_part_content_unset(m_mainLayout, "web_view");
//         elm_object_part_content_set(m_mainLayout, "web_view", m_bookmarksUI->getContent());
//         evas_object_show(m_bookmarksUI->getContent());
//         //m_simpleURI->changeUri(HomePageURL);
//         m_bookmarksUI->showErrorMsg();
//         //filterURL(HomePageURL);
//     }
    setErrorButtons();
    evas_object_hide(elm_object_part_content_get(m_mainLayout,"web_view"));
    elm_object_part_content_unset(m_mainLayout, "web_view");
    elm_object_part_content_set(m_mainLayout, "web_view",m_errorLayout);
    //evas_object_show(m_errorLayout);
}

void SimpleUI::setErrorButtons()
{
    leftButtonBar->setActionForButton("bookmark_button", m_bookmark);
    leftButtonBar->setActionForButton("refresh_stop_button", m_reload);
    zoomEnable(false);
    addBookmarkEnable(false);
    stopEnable(false);
    reloadEnable(true);
    forwardEnable(false);
    evas_object_hide(m_progressBar);
}


void SimpleUI::showSearchBox()
{
    if (m_searchBox.get())
        m_searchBox->show();
}

void SimpleUI::hideSearchBox()
{
    if (m_searchBox.get())
        m_searchBox->hide();
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
    addBookmarkEnable(false);
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

//    m_platformInputManager->returnPressed.disconnect_all_slots();
//    m_platformInputManager->returnPressed.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_zoomList.get()));
//    hidePopup.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_zoomList.get()));
    m_zoomList->showPopup();
}

void SimpleUI::zoomLevelChanged(int zoom_level)
{
    m_webEngine->setZoomFactor(zoom_level);
    m_currentZoom = static_cast<zoom_type> (zoom_level);
}
void SimpleUI::showTabMenu()
{
    if( !m_tabList ){
        BROWSER_LOGD("[%s:%d] - make_shared ", __PRETTY_FUNCTION__, __LINE__);
        m_tabList = std::make_shared<tizen_browser::base_ui::TabList>(m_window,
                                                                    rightButtonBar->getButton("tab_button"));
        m_tabList->newTabClicked.connect(boost::bind(&SimpleUI::newTabClicked,this));
        m_tabList->tabClicked.connect(boost::bind(&SimpleUI::tabClicked, this, _1));
        m_tabList->tabDelete.connect(boost::bind(&SimpleUI::closeTab, this, _1));

//        m_platformInputManager->rightPressed.connect(boost::bind(&TabList::rightPressed, m_tabList.get()));
//        m_platformInputManager->leftPressed.connect(boost::bind(&TabList::leftPressed, m_tabList.get()));
//        m_platformInputManager->enterPressed.connect(boost::bind(&TabList::enterPressed, m_tabList.get()));
    }
//    m_platformInputManager->returnPressed.disconnect_all_slots();
//    m_platformInputManager->returnPressed.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_tabList.get()));
//    hidePopup.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_tabList.get()));
    m_tabList->addItems(m_webEngine->getTabContents());
    m_tabList->setCurrentTabId(m_webEngine->currentTabId());
    m_tabList->disableNewTabBtn(m_tabList->getItemcCount() >= m_tabLimit);
    m_tabList->showPopup();
}

void SimpleUI::newTabClicked()
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
    std::cerr<<"Web confirmation signal received"<<std::endl;
    switch(webConfirmation->getConfirmationType()) {
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
        }
        break;
        case basic_webengine::WebConfirmation::ConfirmationType::CertificateConfirmation:
        {
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

void SimpleUI::showSettingsMenu()
{
    if(!m_settings){
        m_settings = std::make_shared<Settings>(m_window,
                                                rightButtonBar->getButton("setting_button"));
        m_settings->addAction( m_settingPointerMode);
        m_settings->addAction( m_settingPrivateBrowsing);
        m_settings->addAction( m_settingDeleteHistory);
        m_settings->addAction( m_settingDeleteData);
        m_settings->addAction( m_settingDeleteFavorite);
        m_settingPointerMode->toggled.connect(boost::bind(&tizen_browser::base_ui::Settings::setPointerModeEnabled, m_settings.get(), _1));
    }
//    m_platformInputManager->returnPressed.disconnect_all_slots();
//    m_platformInputManager->returnPressed.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_settings.get()));
//    hidePopup.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_settings.get()));
    m_settingDeleteHistory->setEnabled(m_historyService->getHistoryItemsCount());
    m_settingDeleteFavorite->setEnabled(m_favoriteService->countBookmarksAndSubFolders());
    m_settings->showPopup();

}

void SimpleUI::showHistory()
{
    if(!m_historyList) {
        m_historyList = std::make_shared<HistoryList>(m_window, rightButtonBar->getButton("history_button"));
        m_historyList->clickedHistoryItem.connect(boost::bind(&SimpleUI::openLinkFromPopup, this,_1));
        m_historyList->deleteHistoryItem.connect(boost::bind(&tizen_browser::services::HistoryService::clearURLHistory, m_historyService.get(),_1));
        m_historyList->deleteHistoryItem.connect(boost::bind(&SimpleUI::hideHistory, this));

//        m_platformInputManager->rightPressed.connect(boost::bind(&HistoryList::rightPressed, m_historyList.get()));
//        m_platformInputManager->leftPressed.connect(boost::bind(&HistoryList::leftPressed, m_historyList.get()));
//        m_platformInputManager->enterPressed.connect(boost::bind(&HistoryList::enterPressed, m_historyList.get()));
    }
//    m_platformInputManager->returnPressed.disconnect_all_slots();
//    m_platformInputManager->returnPressed.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_historyList.get()));
//    hidePopup.connect(boost::bind(&SimpleUI::onReturnPressed, this, m_historyList.get()));
    m_historyList->addItems(m_historyService->getHistoryItems());
    m_historyList->showPopup();
}

void SimpleUI::disableHistoryButton(bool flag)
{
    BROWSER_LOGD("[%s:%d] flag:%d ", __PRETTY_FUNCTION__, __LINE__, flag);
    m_history->setEnabled(!flag);
    hidePopup();
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

void SimpleUI::settingsDeleteHistory()
{
    BROWSER_LOGD("[%s]: Deleting Hisory", __func__);
    SimplePopup *popup = SimplePopup::createPopup();
    popup->setTitle("Delete history");
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->setMessage("Are you sure you want to clear all browsing history?");
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteHistoryButton, this, _1, _2));
    popup->show();
}

void SimpleUI::settingsDeleteData()
{
    BROWSER_LOGD("[%s]", __func__);
    SimplePopup *popup = SimplePopup::createPopup();
    popup->setTitle("Delete data");
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->setMessage("Are you sure you want to clear all cookies and cache?");
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteDataButton, this, _1, _2));
    popup->show();

}

void SimpleUI::settingsDeleteFavorite()
{
    BROWSER_LOGD("[%s]", __func__);
    SimplePopup *popup = SimplePopup::createPopup();
    popup->setTitle("Delete favorite site");
    popup->addButton(OK);
    popup->addButton(CANCEL);
    popup->setMessage("Are you sure you want to clear all favorite site?");
    popup->buttonClicked.connect(boost::bind(&SimpleUI::onDeleteFavoriteButton, this, _1, _2));
    popup->show();
}

void SimpleUI::onDeleteHistoryButton(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if(button == OK){
        BROWSER_LOGD("[%s]: OK", __func__);
        m_historyService->clearAllHistory();
        m_history->setEnabled(false);
        webTitleBar->removeFavIcon();
        webTitleBar->show("History deleted");
        hidePopup();
    }
}

void SimpleUI::onDeleteDataButton(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if(button == OK){
        BROWSER_LOGD("[%s]: OK", __func__);
        m_webEngine->clearPrivateData();
        webTitleBar->removeFavIcon();
        webTitleBar->show("Data deleted");
        hidePopup();
    }
}

void SimpleUI::onDeleteFavoriteButton(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/)
{
    if(button == OK){
        BROWSER_LOGD("[%s]: OK", __func__);
        m_favoriteService->deleteAllBookmarks();
        bookmarkCheck();
        webTitleBar->removeFavIcon();
        webTitleBar->show("Favorites deleted");
        hidePopup();
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
        if (isHomePageActive() != true) {
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
    if (self->m_searchBox->is_visible())
        self->m_searchBox->hide();
    else if (!self->isHomePageActive() && !self->isErrorPageActive())
    {
        self->m_simpleURI->clearFocus();
        self->m_searchBox->show();
    }
}

void SimpleUI::addToBookmarks(void)
{
	if (m_favoriteService)
		m_favoriteService->addToBookmarks(m_webEngine->getURI(), m_webEngine->getTitle(), std::string(),
										  m_webEngine->getSnapshotData(373, 240),
										  m_webEngine->getFavicon());
}

void SimpleUI::deleteBookmark(void)
{
	if (m_favoriteService)
		m_favoriteService->deleteBookmark(m_webEngine->getURI());
}

}
}
