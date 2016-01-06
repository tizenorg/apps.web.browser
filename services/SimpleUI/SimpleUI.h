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
#if PROFILE_MOBILE
#include "FindOnPageUI.h"
#include "SettingsUI_mob.h"
#include "TextPopup_mob.h"
#include "ContentPopup_mob.h"
#else
#include "SettingsUI.h"
#endif
#include "QuickAccess.h"
#include "TabUI.h"
#include "ZoomUI.h"
#include "HistoryService.h"
#include "TabServiceTypedef.h"
#include "BookmarkDetailsUI.h"
#include "BookmarkFlowUI.h"
#include "BookmarkManagerUI.h"
#include "PlatformInputManager.h"
#include "SessionStorage.h"
#include "StorageService.h"

// other
#include "Action.h"
#include "InputPopup.h"
#include "SimplePopup.h"
#include "WebConfirmation.h"
#include "ViewManager.h"
#include "CertificateContents.h"
#include "MenuButton.h"

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
    void titleChanged(const std::string& title, const std::string& tabId);
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
    void windowCreated();

#if PROFILE_MOBILE
    void openNewTab(const std::string &uri, const std::string& title =
            std::string(), const boost::optional<int> adaptorId = boost::none,
            bool desktopMode = false, bool incognitoMode = false);
#else
    void openNewTab(const std::string &uri, const std::string& title =
            std::string(), const boost::optional<int> adaptorId = boost::none,
            bool desktopMode = true, bool incognitoMode = false);
#endif

    void switchToTab(const tizen_browser::basic_webengine::TabId& tabId);
    void newTabClicked();
    void tabClicked(const tizen_browser::basic_webengine::TabId& tabId);
    void closeTabsClicked(const tizen_browser::basic_webengine::TabId& tabId);
    bool isIncognito(const tizen_browser::basic_webengine::TabId& tabId);
    void tabCreated();
    bool checkIfCreate();
    void tabClosed(const tizen_browser::basic_webengine::TabId& id);

    std::shared_ptr<services::HistoryItemVector> getHistory();
    std::shared_ptr<services::HistoryItemVector> getMostVisitedItems();

    //UI signal handling functions
    void onBookmarkAdded(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem);

    void onBookmarkClicked(std::shared_ptr<tizen_browser::services::BookmarkItem> bookmarkItem);
    void onNewFolderClicked();
    void onNewFolderPopupClick(const std::string& folder_name);
#if PROFILE_MOBILE
    void onEditFolderClicked(const std::string& folder_name);
    void onDeleteFolderClicked(const std::string& folder_name);
    void onRemoveFoldersClicked(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem>> items);
    void onEditFolderPopupClicked(const std::string& newName);
    void onDeleteFolderPopupClicked(PopupButtons button);
#endif
    void onBookmarkRemoved(const std::string& uri);

    void onHistoryRemoved(const std::string& uri);
    void onOpenURLInNewTab(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, bool desktopMode);
    /**
     * @brief Handles 'openUrlInNewTab' signals. Uses QuickAccess to indicate
     * desktop/mobile mode.
     * TODO: desktop mode should be checked in WebView or QuickAcces (depends
     * on which view is active)
     */
    void onOpenURLInNewTab(const std::string& url);
    void onOpenURLInNewTab(const std::string& url, const std::string& title, bool desktopMode);
    void onMostVisitedTileClicked(std::shared_ptr<tizen_browser::services::HistoryItem> historyItem, int itemsNumber);
    void onClearHistoryAllClicked();
    void onDeleteHistoryItems(std::shared_ptr<const std::vector<int>> itemIds);

    void onMostVisitedClicked();
    void onBookmarkButtonClicked();

    /**
     * @brief Handles 'generateThumb' signals.
     */
    void onGenerateThumb(basic_webengine::TabId tabId);
    void onSnapshotCaptured(std::shared_ptr<tizen_browser::tools::BrowserImage> snapshot);
    void onCreateTabId();

    void handleConfirmationRequest(basic_webengine::WebConfirmationPtr webConfirmation);
    void authPopupButtonClicked(PopupButtons button, std::shared_ptr<PopupData> popupData);
    void certPopupButtonClicked(PopupButtons button, std::shared_ptr<PopupData> popupData);

    void onActionTriggered(const Action& action);
    void onMouseClick();
    void onRedKeyPressed();
    void onYellowKeyPressed();
#if PROFILE_MOBILE
    void onMenuButtonPressed();
#endif
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
    void addBookmark(BookmarkUpdate bookmark_update);

    /**
     * @brief Edits currents page bookmark
     *
     */
    void editBookmark(BookmarkUpdate bookmark_update);

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
    void showCertificatePopup();
    void showBookmarkFlowUI(bool state);
#if PROFILE_MOBILE
    void closeBookmarkFlowUI();

    void showFindOnPageUI();
    void findWord(const struct FindData& fdata);
    void closeFindOnPageUI();

    void registerHWKeyCallback();
    void unregisterHWKeyCallback();

    void onRotateClockwisePressed();
    void onRotateCounterClockwisePressed();
    void onRotation();
#endif
    void closeBookmarkDetailsUI();
    void closeBookmarkManagerUI();
    void showBookmarkManagerUI();
    void onBookmarkCustomFolderClicked(int);
    void onBookmarkAllFolderClicked();
    void onBookmarkSpecialFolderClicked();

    void showPopup(interfaces::AbstractPopup* popup);
    void dismissPopup(interfaces::AbstractPopup* popup);

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

    typedef CertificateContents::CurrentTabCertificateData TabCertificateData;
    TabCertificateData* getCertificateDataForTab(const tizen_browser::basic_webengine::TabId& tabId);

    void setCertificatePem(const char* pem);
    void loadHostCertInfoFromDB();
    void saveCertificateInfo(const std::string&, const std::string&, CertificateContents::HOST_TYPE type);
    void deleteAllSavedCertificates(void);
    void updateCertificateInfo(const std::string&, const std::string&, CertificateContents::HOST_TYPE type);

    std::string edjePath(const std::string &);

    std::vector<interfaces::AbstractPopup*> m_popupVector;

    std::shared_ptr<WebPageUI> m_webPageUI;
    std::shared_ptr<basic_webengine::AbstractWebEngine<Evas_Object>>  m_webEngine;
    std::shared_ptr<interfaces::AbstractFavoriteService> m_favoriteService;
    std::shared_ptr<services::HistoryService> m_historyService;
    services::TabServicePtr m_tabService;
    std::shared_ptr<MoreMenuUI> m_moreMenuUI;
    std::shared_ptr<BookmarkDetailsUI> m_bookmarkDetailsUI;
#if PROFILE_MOBILE
    std::shared_ptr<BookmarkFlowUI> m_bookmarkFlowUI;
    std::shared_ptr<FindOnPageUI> m_findOnPageUI;
#endif
    std::shared_ptr<CertificateContents> m_certificateContents;
    std::shared_ptr<BookmarkManagerUI> m_bookmarkManagerUI;
    std::shared_ptr<QuickAccess> m_quickAccess;
    std::shared_ptr<HistoryUI> m_historyUI;
    std::shared_ptr<SettingsUI> m_settingsUI;
    std::shared_ptr<TabUI> m_tabUI;
    std::shared_ptr<services::PlatformInputManager> m_platformInputManager;
    std::shared_ptr<services::StorageService> m_storageService;
    storage::Session m_currentSession;
    std::shared_ptr<tizen_browser::base_ui::ZoomUI> m_zoomUI;
    bool m_initialised;
    int m_tabLimit;
    int m_favoritesLimit;
    bool m_wvIMEStatus;
    std::string m_folder_name;

    std::map<tizen_browser::basic_webengine::TabId, TabCertificateData> m_tabCertificateMap;

    //helper object used to view management
    ViewManager m_viewManager;
    Evas_Object *main_window;
#if PROFILE_MOBILE
    int angle = 0;
#endif
};

}
}

#endif /* SIMPLEUI_H_ */
