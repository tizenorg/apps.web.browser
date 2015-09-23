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

class BROWSER_EXPORT WebPageUI : public tizen_browser::core::AbstractService, public tizen_browser::interfaces::AbstractUIComponent {
public:
    WebPageUI();
    virtual ~WebPageUI();
    virtual std::string getName();
    virtual void init(Evas_Object* parent);
    virtual Evas_Object* getContent();
    virtual void showUI();
    virtual void hideUI();
    void loadStarted();
    void progressChanged(double progress);
    void loadFinished();
    bool isErrorPageActive();
    bool isHomePageActive() { return m_homePageActive; }
    void switchViewToErrorPage();
    void switchViewToWebPage(Evas_Object* content, const std::string uri);
    void switchViewToQuickAccess(Evas_Object* content);
    URIEntry& getURIEntry() const { return *m_URIEntry.get(); }
    void setTabsNumber(int tabs);
    void setBackButtonEnabled(bool enabled) { m_back->setEnabled(enabled); }
    void setForwardButtonEnabled(bool enabled) { m_forward->setEnabled(enabled); }
    void setReloadButtonEnabled(bool enabled) { m_reload->setEnabled(enabled); }
    void setStopButtonEnabled(bool enabled) { m_stopLoading->setEnabled(enabled); }
    void setMoreMenuButtonEnabled(bool enabled) { m_showMoreMenu->setEnabled(enabled); }

    boost::signals2::signal<void ()> backPage;
    boost::signals2::signal<void ()> forwardPage;
    boost::signals2::signal<void ()> stopLoadingPage;
    boost::signals2::signal<void ()> reloadPage;
    boost::signals2::signal<void ()> showTabUI;
    boost::signals2::signal<void ()> showMoreMenu;

    static void faviconClicked(void* data, Evas_Object* obj, const char* emission, const char* source);
private:
    void createLayout();
    void createErrorLayout();
    void createActions();
    void connectActions();
    void showProgressBar();
    void hideProgressBar();
    void hideWebView();
    void setErrorButtons();
    void setMainContent(Evas_Object* content);
    void updateURIBar(const std::string& uri);
    std::string edjePath(const std::string& file);

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
    Evas_Object* m_progressBar;
    std::unique_ptr<ButtonBar> m_leftButtonBar;
    std::unique_ptr<ButtonBar> m_rightButtonBar;
    std::unique_ptr<URIEntry> m_URIEntry;
    bool m_homePageActive;

    sharedAction m_back;
    sharedAction m_forward;
    sharedAction m_stopLoading;
    sharedAction m_reload;
    sharedAction m_tab;
    sharedAction m_showMoreMenu;
};


}   // namespace tizen_browser
}   // namespace base_ui

#endif // WEBPAGEUI_H
