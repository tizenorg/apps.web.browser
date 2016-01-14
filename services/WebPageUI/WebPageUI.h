/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#ifndef WEBPAGEUI_H
#define WEBPAGEUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>
#include "AbstractService.h"
#include "AbstractUIComponent.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "ButtonBar.h"
#include "URIEntry.h"

namespace tizen_browser {
namespace base_ui {

class WebPageUIStatesManager;
enum class WPUState;
typedef std::shared_ptr<WebPageUIStatesManager> WPUStatesManagerPtr;
typedef std::shared_ptr<const WebPageUIStatesManager> WPUStatesManagerPtrConst;
class UrlHistoryList;
typedef std::shared_ptr<UrlHistoryList> UrlHistoryPtr;

class BROWSER_EXPORT WebPageUI : public tizen_browser::core::AbstractService, public tizen_browser::interfaces::AbstractUIComponent {
public:
    WebPageUI();
    virtual ~WebPageUI();
    virtual std::string getName();
    virtual void init(Evas_Object* parent);
    virtual Evas_Object* getContent();
    UrlHistoryPtr getUrlHistoryList();
    virtual void showUI();
    virtual void hideUI();
    void loadStopped();
    void loadStarted();
    void progressChanged(double progress);
    void loadFinished();
    WPUStatesManagerPtrConst getStatesMgr() {return m_statesMgr;}
    /**
     * @param state The state to compare
     * @returns True if manager's state equals to given state
     */
    bool stateEquals(WPUState state) const;
    /**
     * @param states The states to compare
     * @returns True if one of the given states equals to the manager's state
     */
    bool stateEquals(std::initializer_list<WPUState> states) const;
    bool isWebPageUIvisible() { return m_WebPageUIvisible; }
    void toIncognito(bool);
    void switchViewToErrorPage();
    void switchViewToWebPage(Evas_Object* content, const std::string uri, const std::string title);
    void switchViewToIncognitoPage();
    void switchViewToQuickAccess(Evas_Object* content);
    URIEntry& getURIEntry() const { return *m_URIEntry.get(); }
    void setPageTitle(const std::string& title);
    void setTabsNumber(int tabs);
    void setBackButtonEnabled(bool enabled) { m_back->setEnabled(enabled); }
    void setForwardButtonEnabled(bool enabled) { m_forward->setEnabled(enabled); }
    void setReloadButtonEnabled(bool enabled) { m_reload->setEnabled(enabled); }
    void setStopButtonEnabled(bool enabled) { m_stopLoading->setEnabled(enabled); }
    void setMoreMenuButtonEnabled(bool enabled) { m_showMoreMenu->setEnabled(enabled); }
    void lockWebview();
    void lockUrlHistoryList();
    void unlockUrlHistoryList();
    void onRedKeyPressed();
    void onYellowKeyPressed();
#if PROFILE_MOBILE
    void mobileEntryFocused();
    void mobileEntryUnfocused();
#endif

    boost::signals2::signal<void ()> backPage;
    boost::signals2::signal<void ()> forwardPage;
    boost::signals2::signal<void ()> stopLoadingPage;
    boost::signals2::signal<void ()> reloadPage;
    boost::signals2::signal<void ()> showTabUI;
#if PROFILE_MOBILE
    boost::signals2::signal<void ()> hideMoreMenu;
#endif
    boost::signals2::signal<void ()> showMoreMenu;
    boost::signals2::signal<void ()> hideQuickAccess;
    boost::signals2::signal<void ()> showQuickAccess;
    boost::signals2::signal<void ()> bookmarkManagerClicked;
    boost::signals2::signal<void ()> showZoomNavigation;
    boost::signals2::signal<void (bool enabled)> setWebViewTouchEvents;

private:
    static void faviconClicked(void* data, Evas_Object* obj, const char* emission, const char* source);
    static Eina_Bool _cb_down_pressed_on_urlbar(void *data, Evas_Object *obj, Evas_Object *src, Evas_Callback_Type type, void *event_info);
    static void _bookmark_manager_clicked(void * data, Evas_Object *, void *);
#if PROFILE_MOBILE && GESTURE
    static Evas_Event_Flags _gesture_move(void *data, void *event_info);
    static void _geasture_finished(void *data, Evas_Object *obj, const char *emission, const char *source);
#endif

    void createLayout();
    void createErrorLayout();
    void createPrivateLayout();
    void createActions();
    void connectActions();
    void showProgressBar();
    void hideProgressBar();
    void hideWebView();
    void setErrorButtons();
    void setPrivateButtons();
    void setMainContent(Evas_Object* content);
    void updateURIBar(const std::string& uri);
    std::string edjePath(const std::string& file);
    void refreshFocusChain();
#if PROFILE_MOBILE && GESTURE
    void geastureUp();
    void geastureDown();
#endif

    // wrappers to call singal as a reaction to other signal
    void backPageConnect() { backPage(); }
    void forwardPageConnect() { forwardPage(); }
    void stopLoadingPageConnect() { stopLoadingPage(); }
    void reloadPageConnect() { reloadPage(); }
    void showTabUIConnect();
    void showMoreMenuConnect();

    Evas_Object* m_parent;
    Evas_Object* m_mainLayout;
    Evas_Object* m_errorLayout;
    Evas_Object* m_privateLayout;
    Evas_Object* m_progressBar;
    Evas_Object* m_bookmarkManagerButton;

    std::unique_ptr<ButtonBar> m_leftButtonBar;
    std::unique_ptr<ButtonBar> m_rightButtonBar;
    std::unique_ptr<URIEntry> m_URIEntry;
    WPUStatesManagerPtr m_statesMgr;
    UrlHistoryPtr m_urlHistoryList;
    bool m_webviewLocked;
    bool m_WebPageUIvisible;

    sharedAction m_back;
    sharedAction m_forward;
    sharedAction m_stopLoading;
    sharedAction m_reload;
    sharedAction m_tab;
    sharedAction m_showMoreMenu;

#if PROFILE_MOBILE && GESTURE
    Evas_Object* m_geastureLayer;
    bool m_uriBarHidden;
    static const int SINGLE_FINGER = 1;
    static const int SWIPE_MOMENTUM_TRESHOLD = 500;
#endif
};


}   // namespace tizen_browser
}   // namespace base_ui

#endif // WEBPAGEUI_H
