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

#ifndef WEBENGINESERVICE_H_
#define WEBENGINESERVICE_H_

#include <boost/noncopyable.hpp>
#include <string>
#include <Evas.h>
#include <memory>

#include "core/ServiceManager/ServiceFactory.h"
#include "core/ServiceManager/service_macros.h"

#include "core/AbstractWebEngine/AbstractWebEngine.h"
#include "core/AbstractWebEngine/TabIdTypedef.h"
#include "core/AbstractWebEngine/WebConfirmation.h"
#include "core/Tools/BrowserImage.h"

namespace tizen_browser {
namespace basic_webengine {
namespace webengine_service {

class WebView;

typedef std::shared_ptr<WebView> WebViewPtr;

class BROWSER_EXPORT WebEngineService : public AbstractWebEngine<Evas_Object>, boost::noncopyable
{
public:
    WebEngineService();
    virtual ~WebEngineService();
    virtual std::string getName() = 0;

    Evas_Object * getLayout() override;
    void init(void * guiParent) override;

    void setURI(const std::string &) override;
    std::string getURI(void) const override;

    std::string getTitle(void) const override;

    std::string getUserAgent(void) const override;
    void setUserAgent(const std::string& ua) override;

    void suspend(void) override;
    void resume(void) override;
    bool isSuspended(void) const override;

    void stopLoading(void) override;
    void reload(void) override;

    void back(void) override;
    void forward(void) override;

    bool isBackEnabled(void) const override;
    bool isForwardEnabled(void) const override;

    bool isLoading() const override;

    void clearCache() override;
    void clearCookies() override;
    void clearPrivateData() override;
    void clearPasswordData() override;
    void clearFormData() override;

    int tabsCount() const override;
    TabId currentTabId() const override;

    void destroyTabs() override;

    /**
     * Get TabContent collection filled with TabID and titles.
     * Without thumbnails.
     */
    std::vector<TabContentPtr> getTabContents() const override;

    /**
     * See AbstractWebEngine@addTab(const std::string&, const TabId*,
     * const boost::optional<int>, const std::string&, bool, bool)
     */
    TabId addTab(
            const std::string & uri = std::string(),
            const TabId* tabInitId = NULL,
            const boost::optional<int> tabId = boost::none,
            const std::string& title = std::string(),
            bool desktopMode = true,
            bool incognitoMode = false) override;
    Evas_Object* getTabView(TabId id) override;
    bool switchToTab(TabId) override;
    bool closeTab() override;
    bool closeTab(TabId) override;
    void confirmationResult(WebConfirmationPtr) override;

    /**
     * @brief Get snapshot of webpage
     *
     * @param width of snapshot
     * @param height of snapshot
     * @return Shared pointer to BrowserImage
     */
    std::shared_ptr<tizen_browser::tools::BrowserImage> getSnapshotData(int width, int height) override;

    std::shared_ptr<tizen_browser::tools::BrowserImage> getSnapshotData(TabId id, int width, int height, bool async) override;

    /**
     * @brief Get the state of private mode for a specific tab
     *
     * @param id of snapshot
     * @return state of private mode where:
     *     -1 is "Not set"
     *      0 is "False"
     *      1 is "True"
     */
    bool isPrivateMode(const TabId& id) override;


    /**
     * @brief Check if current tab has load error.
     *
     */
    bool isLoadError() const override;

     /**
     * \brief Sets Focus to URI entry.
     */
    void setFocus() override;

    /**
     * @brief Remove focus form URI
     */
    void clearFocus() override;

    /**
     * @brief check if URI is focused
     */
    bool hasFocus() const override;

    virtual int getZoomFactor() const override;

    /**
     * @brief check if autofit is enabled
     */

    virtual void setZoomFactor(int zoomFactor) override;


    /**
     * @brief Search string on searchOnWebsite
     *
     * @param string to search on searchOnWebsite
     * @param flags for search options
     */
    void searchOnWebsite(const std::string &, int flags) override;

    /**
     * @brief Get favicon of current page loaded
     */
    std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon() override;

    /**
     * @brief back or exit when back key is pressed
     */
    void backButtonClicked() override;

#if PROFILE_MOBILE
    void moreKeyPressed();
    void orientationChanged();
#endif

    void switchToMobileMode() override;
    void switchToDesktopMode() override;
    bool isDesktopMode() const override;

    void scrollView(const int& dx, const int& dy) override;

    void onTabIdCreated(int tabId) override;

#if PROFILE_MOBILE
    /**
     * @brief Searches for word in the current page.
     *
     * @param enabled The input word entered by user in Find on page entry.
     * @param forward If true, search forward, else search backward.
     * @param found_cb Callback function invoked when "text,found" event is triggered.
     * @param data User data.
     */
    void findWord(const char *word, Eina_Bool forward, Evas_Smart_Cb found_cb, void *data);
    /**
     * @brief Enable or disable touch events for current web view
     *
     * @param enabled True if touch event have to be enabled, false else.
     */
    void setTouchEvents(bool enabled) override;

    /**
     * @brief Get settings param.
     */
    bool getSettingsParam(WebEngineSettings param) override;

    /**
     * @brief Set bool settings param value.
     */
    void setSettingsParam(WebEngineSettings param, bool value) override;
#endif
private:
    // callbacks from WebView
    void _favIconChanged(std::shared_ptr<tizen_browser::tools::BrowserImage> bi);
    void _titleChanged(const std::string&, const std::string&);
    void _uriChanged(const std::string &);
    void _loadFinished();
    void _loadStarted();
    void _loadStop();
    void _loadError();
    void _forwardEnableChanged(bool);
    void _backwardEnableChanged(bool);
    void _loadProgress(double, TabId, bool);
    void _ready(TabId id);
    void _confirmationRequest(WebConfirmationPtr) ;
    void _IMEStateChanged(bool);
    void _snapshotCaptured(std::shared_ptr<tizen_browser::tools::BrowserImage> snapshot);
    void webViewClicked();
#if PROFILE_MOBILE
    int _getRotation();
    void setWebViewSettings(std::shared_ptr<WebView> webView);
#endif

    /**
     * disconnect signals from specified WebView
     * \param WebView
     */
    void disconnectSignals(WebViewPtr);

    void disconnectCurrentWebViewSignals() override;

    /**
     * connect signals of specified WebView
     * \param WebView
     */
    void connectSignals(WebViewPtr);

    int createTabId();

private:
    bool m_initialised;
    void* m_guiParent;
    bool m_stopped;

    // current TabId
    TabId m_currentTabId;
    // current WebView
    WebViewPtr m_currentWebView;

    // map of all tabs (WebViews)
    std::map<TabId, WebViewPtr > m_tabs;
    // Most recent tab list
    std::vector<TabId> m_mostRecentTab;
    int m_tabIdCreated;
#if PROFILE_MOBILE
    std::map<WebEngineSettings, bool>  m_settings;
#endif
};

} /* end of webengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */

#endif /* WEBENGINESERVICE_H_ */
