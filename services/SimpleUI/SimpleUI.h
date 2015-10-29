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
#include "WebPageUI.h"
#include "AbstractWebEngine.h"
#include "MoreMenuUI.h"
#include "HistoryUI.h"
#include "SettingsUI.h"
#include "QuickAccess.h"
#include "TabUI.h"
#include "ZoomUI.h"
#include "HistoryService.h"
#include "BookmarkManagerUI.h"
#include "PlatformInputManager.h"
#include "SessionStorage.h"
#include "SqlStorage.h"

// other
#include "Action.h"
#include "SimplePopup.h"
#include "WebConfirmation.h"
#include "BookmarksManager.h"
#include "Config.h"
#include "ViewManager.h"

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
    void suspend() {m_webEngine->suspend();}
    void resume() {m_webEngine->resume();}

    void destroyUI();
private:
    // setup functions
    void loadUIServices();
    void connectUISignals();
    void loadModelServices();
    void initModelServices();
    void initUIServices();
    void connectModelSignals();
    void restoreLastSession();
    Evas_Object* createWebLayout(Evas_Object* parent);
    Evas_Object* createErrorLayout(Evas_Object* parent);

    void forwardEnable(bool enable);
    void stopEnable(bool enable);
    void reloadEnable(bool enable);

    void loadStopped();
    void loadFinished();
    void progressChanged(double progress);
    void loadStarted();

    void loadError();
    void setErrorButtons();

    void bookmarkAdded();
    void bookmarkDeleted();

    void showQuickAccess();
    void switchViewToQuickAccess();
    void switchViewToIncognitoPage();
    void switchViewToWebPage();
    void updateView();

    void openNewTab(const std::string &uri, bool desktopMode = true, bool incognitoMode = false);
    void switchToTab(const tizen_browser::basic_webengine::TabId& tabId);
    void newTabClicked();
    void tabClicked(const tizen_browser::basic_webengine::TabId& tabId);
    void closeTabsClicked(const tizen_browser::basic_webengine::TabId& tabId);
    void tabCreated();
    bool checkIfCreate();
    void tabClosed(const tizen_browser::basic_webengine::TabId& id);

    std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > getBookmarks(int folder_id = -1);
    std::shared_ptr<services::HistoryItemVector> getHistory();
    std::shared_ptr<services::HistoryItemVector> getMostVisitedItems();

    //UI signal handling functions
    void onBookmarkAdded(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem);

    void onBookmarkClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem);
    void onBookmarkRemoved(const std::string& uri);

    void onHistoryRemoved(const std::string& uri);
    void onOpenURLInNewTab(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, bool desktopMode);
    /**
     * @brief Handles 'openUrlInNewTab' signals. Uses QuickAccess to indicate desktop/mobile mode.
     */
    void onOpenURLInNewTab(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem);
    void onMostVisitedTileClicked(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, int itemsNumber);
    void onClearHistoryClicked();

    void onMostVisitedClicked();
    void onBookmarkButtonClicked();

    void handleConfirmationRequest(basic_webengine::WebConfirmationPtr webConfirmation);
    void authPopupButtonClicked(PopupButtons button, std::shared_ptr<PopupData> popupData);

    void onActionTriggered(const Action& action);
    void onMouseClick();
    void onRedKeyPressed();
    void setwvIMEStatus(bool status);

    sharedAction m_showBookmarkManagerUI;

    /**
     * \brief filters URL before it is passed to WebEngine.
     *
     * This function should be connected with m_simpleURI->uriChanged.
     * it is a good place to check for special urls (like "about:home"),
     * filter forbidden addresses and to check set favorite icon.
     */
    void filterURL(const std::string& url);

    // // on uri entry widget "changed,user" signal
    void onURLEntryEditedByUser(const std::shared_ptr<std::string> editedUrlPtr);
    // on uri entry widget "changed" signal
    void onURLEntryEdited();

    /**
     * \brief check if url comming back from WebEngine should be passed to URI.
     *
     * For filtered addresses we need to hide real URI so the user would be confused.
     * and this is a back function that checks if address emited from browser should be changed.
     */
    void webEngineURLChanged(const std::string url);
    void onmostHistoryvisitedClicked();
    void onBookmarkvisitedClicked();
    /**
     * @brief Check if the current page exists as a bookmark.
     *
     */
    bool checkBookmark();
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
     * @brief show Zoom Menu
     */
    void showZoomUI();
    void closeZoomUI();
    void setZoomFactor(int level);
    int getZoomFactor();
    void scrollView(const int& dx, const int& dy);

    void showTabUI();
    void closeTabUI();
    void showMoreMenu();
    void closeMoreMenu();
    void switchToMobileMode();
    void switchToDesktopMode();
    void showHistoryUI();
    void closeHistoryUI();
    void showSettingsUI();
    void closeSettingsUI();
    void closeBookmarkManagerUI();
    void showBookmarkManagerUI();
    void showPopup(Evas_Object *content, char* btn1_text, char* btn2_text);

    void closeTab();
    void closeTab(const tizen_browser::basic_webengine::TabId& id);

    void settingsDeleteSelectedData(const std::string& str);
    void settingsResetMostVisited();
    void settingsResetBrowser();
    void onDeleteSelectedDataButton(const std::string &dataText);
    void onDeleteMostVisitedButton(std::shared_ptr<PopupData> popupData);
    void onResetBrowserButton(PopupButtons button, std::shared_ptr<PopupData> popupData);
    void tabLimitPopupButtonClicked(PopupButtons button, std::shared_ptr< PopupData > /*popupData*/);
    int tabsCount();

    void onReturnPressed(MenuButton *m);
    void onBackPressed();
    void onEscapePressed();

    void searchWebPage(std::string &text, int flags);

    std::string edjePath(const std::string &);

    Evas_Object *m_popup;

    std::shared_ptr<WebPageUI> m_webPageUI;
    std::shared_ptr<basic_webengine::AbstractWebEngine<Evas_Object>>  m_webEngine;
    std::shared_ptr<interfaces::AbstractFavoriteService> m_favoriteService;
    std::shared_ptr<services::HistoryService> m_historyService;
    std::shared_ptr<MoreMenuUI> m_moreMenuUI;
    std::shared_ptr<BookmarkManagerUI> m_bookmarkManagerUI;
    std::shared_ptr<QuickAccess> m_quickAccess;
    std::shared_ptr<HistoryUI> m_historyUI;
    std::shared_ptr<SettingsUI> m_settingsUI;
    std::shared_ptr<TabUI> m_tabUI;
    std::shared_ptr<services::PlatformInputManager> m_platformInputManager;
    std::shared_ptr<services::SessionStorage> m_sessionService;
    Session::Session m_currentSession;
    std::shared_ptr<tizen_browser::base_ui::ZoomUI> m_zoomUI;
    std::shared_ptr<BookmarksManager> m_bookmarks_manager;
    bool m_initialised;
    int m_tabLimit;
    int m_favoritesLimit;
    bool m_wvIMEStatus;

    //helper object used to view management
    ViewManager* m_viewManager;

    // This context object is used to implicitly init internal ewk data used by opengl to create the first and
    // consecutive webviews in the application, otherwise we would encounter a crash after creating
    // the first web view
    Ewk_Context *m_ewkContext;
};

}
}

#endif /* SIMPLEUI_H_ */
