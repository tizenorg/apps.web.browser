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

/*
 * SimpleUI.h
 *
 *  Created on: Mar 18, 2014
 *      Author: k.szalilow
 */

#ifndef SIMPLEUI_H_
#define SIMPLEUI_H_

#include <Evas.h>

#include "AbstractMainWindow.h"
#include "AbstractService.h"
#include "AbstractFavoriteService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

// components
#include "AbstractWebEngine.h"
#include "MoreMenuUI.h"
#include "HistoryUI.h"
#include "SettingsUI.h"
#include "MainUI.h"
#include "TabUI.h"
#include "ButtonBar.h"
#include "HistoryService.h"
#include "BookmarkManagerUI.h"
#include "SimpleURI.h"
#include "SimpleScroller.h"
#include "WebTitleBar.h"
#include "PlatformInputManager.h"
#include "SessionStorage.h"
#include "SqlStorage.h"

// other
#include "Action.h"
#include "SimplePopup.h"
#include "WebConfirmation.h"
#include "ZoomList.h"
#include "TabList.h"
#include "BookmarksManager.h"
#include "Settings.h"
#include "HistoryList.h"
#include "NetworkErrorHandler.h"
#include "CustomPopup.h"
#include "Config.h"

namespace tizen_browser{
namespace base_ui{

template <>
void AbstractMainWindow<Evas_Object>::setMainWindow(Evas_Object * rawPtr)
{
    m_window = std::shared_ptr<Evas_Object>(rawPtr, evas_object_del);
}

class BROWSER_EXPORT SimpleUI : public AbstractMainWindow<Evas_Object>
{
public:
    SimpleUI(/*Evas_Object *window*/);
    virtual ~SimpleUI();
    virtual int exec(const std::string& url);
    virtual std::string getName();

    void destroyUI();
private:
    void createActions();
    void connectActions();

    void backEnable(bool enable);
    void forwardEnable(bool enable);
    void stopEnable(bool enable);
    void reloadEnable(bool enable);
    void addBookmarkEnable(bool enable);
    void removeBookmarkEnable(bool enable);
    void zoomEnable(bool enable);

    void settingsButtonEnable(bool enable);

    void loadFinished();
    void progressChanged(double progress);
    void loadStarted();

    void loadError();
    void setErrorButtons();
    bool isErrorPageActive();

    void bookmarkAdded();
    void bookmarkDeleted();

    bool isHomePageActive();
    void switchViewToHomePage();
    void updateBrowserView();
    void updateWebView();
    void updateURIBarView();
    void updateView();

    void openNewTab(const std::string &uri);
    void switchToTab(const tizen_browser::basic_webengine::TabId& tabId);
    void newTabClicked(const std::string &);
    void tabClicked(const tizen_browser::basic_webengine::TabId& tabId);
    void tabCreated();
    void tabClosed(const tizen_browser::basic_webengine::TabId& id);

    void bookmarkCheck();
    std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > getBookmarks(int folder_id = -1);
    std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > getBookmarkFolders(int folder_id);
    std::shared_ptr<services::HistoryItemVector> getHistory();
    std::shared_ptr<services::HistoryItemVector> getMostVisitedItems();
    void onBookmarkAdded(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem);

    void onBookmarkClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem);
    void onBookmarkDeleteClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem);
    void onBookmarkRemoved(const std::string& uri);

    void onHistoryAdded(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem);
    void onHistoryRemoved(const std::string& uri);
    void onOpenURLInNewTab(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem);
    void onMostVisitedTileClicked(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, int itemsNumber);
    void onClearHistoryClicked(const std::string&);

    void onMostVisitedClicked(const std::string&);
    void onBookmarkButtonClicked(const std::string&);
    void onBookmarkManagerButtonClicked(const std::string&);

    void handleConfirmationRequest(basic_webengine::WebConfirmationPtr webConfirmation);
    void authPopupButtonClicked(PopupButtons button, std::shared_ptr<PopupData> popupData);

    void onActionTriggered(const Action& action);

    void loadThemes();

    void setwvIMEStatus(bool status);

    sharedAction m_back;
    sharedAction m_forward;
    sharedAction m_stopLoading;
    sharedAction m_reload;
    sharedAction m_bookmark;
    sharedAction m_unbookmark;
    sharedAction m_tab;
    sharedAction m_share;
    sharedAction m_zoom_in;
    sharedAction m_showMoreMenu;
    sharedAction m_showBookmarkManagerUI;
    sharedAction m_settingPointerMode;
    sharedAction m_settingPrivateBrowsing;
    sharedAction m_settingDeleteHistory;
    sharedAction m_settingDeleteData;
    sharedAction m_settingDeleteFavorite;
    sharedAction m_mostvisited;
    sharedAction m_bookmarksvisited;
    sharedAction m_bookmarks_manager_Add_NewFolder;
    sharedAction m_bookmarks_manager_BookmarkBar;
    sharedAction m_bookmarks_manager_Folder1;
    sharedAction m_bookmarks_manager_Folder2;
    sharedAction m_bookmarks_manager_Folder3;

    /**
     * \brief filters URL before it is passed to WebEngine.
     *
     * This function should be connected with m_simpleURI->uriChanged.
     * it is a good place to check for special urls (like "about:home"),
     * filter forbidden addresses and to check set favorite icon.
     */
    void filterURL(const std::string& url);

    /**
     * Checks if correct tab is visible to user, and if not, it update browser view
     * @param id of tab that should be visible to user
     */
    void checkTabId(const tizen_browser::basic_webengine::TabId& id);

    /**
     * \brief check if url comming back from WebEngine should be passed to URI.
     *
     * For filtered addresses we need to hide real URI so the user would be confused.
     * and this is a back function that checks if address emited from browser should be changed.
     */
    void webEngineURLChanged(const std::string& url);
    void onmostHistoryvisitedClicked();
    void onBookmarkvisitedClicked();
     /**
     * @brief Adds current page to bookmarks.
     *
     */
    void addToBookmarks(int);
    /**
     * @brief Remove current page from bookmarks
     *
     * @param  ...
     * @return void
     */
    void deleteBookmark(void);

    /**
     * \brief shows Zoom showZoomMenu
     */
    void showZoomMenu();

    void zoomLevelChanged(int zoom_level);

    void showHistory();
    void hideHistory();
    void AddBookmarkPopup(std::string& str);
    void AddNewFolderPopup(std::string& str);
    void addBookmarkFolders();

    void newFolderBookmarkManager(const char* title, int by_user);
    void newFolderMoreMenu(const char* title, int by_user);
    void showTabUI();
    void closeTabUI(const std::string& str);
    void showMoreMenu();
    void closeMoreMenu(const std::string& str);
    void switchToMobileView();
    void switchToDesktopView();
    void showHistoryUI(const std::string& str);
    void closeHistoryUI(const std::string&);
    void showMainUI();
    void hideMainUI();
    void showURIBar();
    void hideURIBar();
    void hideWebView();
    void hideSettingsMenu();
    void showSettingsUI(const std::string&);
    void closeSettingsUI(const std::string&);

    void showProgressBar();
    void hideProgressBar();

    void closeBookmarkManagerMenu(std::string& str);
    void updateBookmarkManagerGenGrid(int folder_id);
    void showBookmarkManagerMenu();

    void showPopup(Evas_Object *content, char* btn1_text, char* btn2_text);

    void closeTab();
    void closeTab(const tizen_browser::basic_webengine::TabId& id);

    void settingsPointerModeSwitch(bool newState);
    void settingsPrivateModeSwitch(bool newState);
    void settingsDeleteData();
    void settingsDeleteFavorite();
    void settingsDeleteSelectedData(const std::string& str);
    void settingsResetMostVisited(const std::string& str);
    void settingsResetBrowser(const std::string& str);
    void onDeleteSelectedDataButton(PopupButtons button, std::shared_ptr<PopupData> popupData);
    void onDeleteMostVisitedButton(PopupButtons button, std::shared_ptr<PopupData> popupData);
    void onResetBrowserButton(PopupButtons button, std::shared_ptr<PopupData> popupData);
    void onDeleteDataButton(PopupButtons button, std::shared_ptr<PopupData> popupData);
    void onDeleteFavoriteButton(PopupButtons button, std::shared_ptr<PopupData> popupData);
    void tabLimitPopupButtonClicked(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/);
    void openLinkFromPopup(const std::string &);
    void disableHistoryButton(bool flag);

    void onNetworkError();
    void onNetworkConnected();
    void onNetErrorButtonPressed(PopupButtons, std::shared_ptr<PopupData>);

    void onReturnPressed(MenuButton *m);
    void onBackPressed();

    boost::signals2::signal<void ()> hidePopup;

    std::string edjePath(const std::string &);

    Evas_Object *m_mainLayout;
    Evas_Object *m_progressBar;
    Evas_Object *m_popup;
    Evas_Object *m_entry;
    Evas_Object *m_errorLayout;
    CustomPopup *m_ownPopup;
    SimpleScroller *m_scroller;

    std::shared_ptr<basic_webengine::AbstractWebEngine<Evas_Object>>  m_webEngine;
    std::shared_ptr<tizen_browser::base_ui::SimpleURI> m_simpleURI;
    std::shared_ptr<ButtonBar> leftButtonBar;
    std::shared_ptr<ButtonBar> webviewbar;

    std::shared_ptr<ButtonBar> rightButtonBar;
    std::shared_ptr<tizen_browser::interfaces::AbstractFavoriteService> m_favoriteService;
    std::shared_ptr<tizen_browser::services::HistoryService> m_historyService;
    std::shared_ptr<tizen_browser::base_ui::MoreMenuUI> m_moreMenuUI;
    std::shared_ptr<tizen_browser::base_ui::BookmarkManagerUI> m_bookmarkManagerUI;
    std::shared_ptr<tizen_browser::base_ui::MainUI> m_mainUI;
    std::shared_ptr<tizen_browser::base_ui::HistoryUI> m_historyUI;
    std::shared_ptr<tizen_browser::base_ui::SettingsUI> m_settingsUI;
    std::shared_ptr<tizen_browser::base_ui::TabUI> m_tabUI;
    std::shared_ptr<tizen_browser::base_ui::ZoomList> m_zoomList;
    std::shared_ptr<tizen_browser::base_ui::TabList> m_tabList;
    std::shared_ptr<tizen_browser::services::PlatformInputManager> m_platformInputManager;
    std::shared_ptr<tizen_browser::services::SessionStorage> m_sessionService;
    tizen_browser::Session::Session m_currentSession;
    std::shared_ptr<BookmarksManager> m_bookmarks_manager;
    std::shared_ptr<Settings> m_settings;
    std::shared_ptr<HistoryList> m_historyList;
    std::shared_ptr<WebTitleBar> webTitleBar;
    bool m_initialised;
    bool m_isHomePageActive;
    zoom_type m_currentZoom;
    int m_tabLimit;
    int m_favoritesLimit;
    bool m_wvIMEStatus;
    int m_curr_folder_id;
    // This context object is used to implicitly init internal ewk data used by opengl to create the first and
    // consecutive webviews in the application, otherwise we would encounter a crash after creating
    // the first web view
    Ewk_Context *m_ewkContext;

    std::vector<std::shared_ptr<tizen_browser::services::HistoryItem>> items_vector;
    std::unique_ptr<tizen_browser::basic_ui::NetworkErrorHandler> m_netErrorHandler;
    SimplePopup* m_networkErrorPopup;

    void searchWebPage(std::string &text, int flags);
    static void favicon_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);

};

}
}

#endif /* SIMPLEUI_H_ */
