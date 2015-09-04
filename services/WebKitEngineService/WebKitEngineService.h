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

#ifndef WEBKITENGINESERVICE_H_
#define WEBKITENGINESERVICE_H_

#include <boost/noncopyable.hpp>
#include <string>
#include <Evas.h>
#include <memory>

#include "ServiceFactory.h"
#include "service_macros.h"

#include "AbstractWebEngine/AbstractWebEngine.h"
#include "AbstractWebEngine/TabId.h"
#include "AbstractWebEngine/WebConfirmation.h"
#include "BrowserImage.h"

namespace tizen_browser {
namespace basic_webengine {
namespace webkitengine_service {

class WebView;

typedef std::shared_ptr<WebView> WebViewPtr;

class BROWSER_EXPORT WebKitEngineService: public AbstractWebEngine<Evas_Object>, boost::noncopyable
{
public:
    WebKitEngineService();
    virtual ~WebKitEngineService();
    virtual std::string getName();

    Evas_Object * getLayout();
    void init(void * guiParent);

    void setURI(const std::string &);
    std::string getURI(void) const;

    std::string getTitle(void) const;

    void stopLoading(void);
    void reload(void);

    void back(void);
    void forward(void);

    bool isBackEnabled(void) const;
    bool isForwardEnabled(void) const;

    bool isLoading() const;

    void clearPrivateData();

    int tabsCount() const;
    TabId currentTabId() const;
    std::list<TabId> listTabs() const;
    std::vector<std::shared_ptr<TabContent> > getTabContents() const;

    /**
     * Adds new tab. If uri is specified setURI(uri).
     * If it first tab in WebEngine switch to this tab.
     * @param uri
     * @return TabId of created tab
     */
    TabId addTab(const std::string & uri = std::string(), const TabId * openerId = NULL, bool desktopMode = true);
    Evas_Object* getTabView(TabId id);
    bool switchToTab(TabId);
    bool closeTab();
    bool closeTab(TabId);

    /**
     * Change tab to next created, dummy implementation.
     * @return true if changed successfully, false otherwise
     */
    bool nextTab();
    /**
     * Change tab to previous created, dummy implementation.
     * @return true if changed successfully, false otherwise
     */
    bool prevTab();

    void confirmationResult(WebConfirmationPtr);

    /**
     * @brief Get snapshot of webpage
     *
     * @param width of snapshot
     * @param height of snapshot
     * @return Shared pointer to BrowserImage
     */
    std::shared_ptr<tizen_browser::tools::BrowserImage> getSnapshotData(int width, int height);

    std::shared_ptr<tizen_browser::tools::BrowserImage> getSnapshotData(TabId id, int width, int height);

    void setPrivateMode(bool);
    bool isPrivateMode() const;


    /**
     * @brief Check if current tab has load error.
     *
     */
    bool isLoadError() const;

     /**
     * \brief Sets Focus to URI entry.
     */
    void setFocus();

    /**
     * @brief Remove focus form URI
     */
    void clearFocus();

    /**
     * @brief check if URI is focused
     */
    bool hasFocus() const;

    virtual int getZoomFactor() const;

    /**
     * @brief check if autofit is enabled
     */

    virtual void setZoomFactor(int zoomFactor);


    /**
     * @brief Search string on searchOnWebsite
     *
     * @param string to search on searchOnWebsite
     * @param flags for search options
     */
    void searchOnWebsite(const std::string &, int flags);

    /**
     * @brief Get favicon of current page loaded
     */
    std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon();

    /**
     * @brief back or exit when back key is pressed
     */
    void backButtonClicked() const;

    void switchToMobileMode();
    void switchToDesktopMode();
    bool isDesktopMode() const;

    void scrollView(const int& dx, const int& dy);

private:
    // callbacks from WebView
    void _favIconChanged(std::shared_ptr<tizen_browser::tools::BrowserImage> bi);
    void _titleChanged(const std::string &);
    void _uriChanged(const std::string &);
    void _uriChangedOnTab(TabId);
    void _loadFinished();
    void _loadStarted();
    void _loadStop();
    void _loadError();
    void _forwardEnableChanged(bool);
    void _backwardEnableChanged(bool);
    void _loadProgress(double);
    void _confirmationRequest(WebConfirmationPtr) ;
    void _IMEStateChanged(bool);
    void webViewClicked();

    /**
     * disconnect signals from specified WebView
     * \param WebView
     */
    void disconnectSignals(WebViewPtr);

    void disconnectCurrentWebViewSignals();

    /**
     * connect signals of specified WebView
     * \param WebView
     */
    void connectSignals(WebViewPtr);

private:
    bool m_initialised;
    bool m_privateMode;
    void * m_guiParent;

    // current TabId
    TabId m_currentTabId;
    // current WebView
    WebViewPtr m_currentWebView;

    // map of all tabs (WebViews)
    std::map<TabId, WebViewPtr > m_tabs;
    // Most recent tab list
    std::list<TabId> m_mostRecentTab;
    // recently added tabs first
    std::list<TabId> m_chronoTabs;
};

} /* end of webkitengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */

#endif /* WEBKITENGINESERVICE_H_ */
