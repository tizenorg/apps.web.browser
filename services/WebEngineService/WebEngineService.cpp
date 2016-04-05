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

#include "browser_config.h"
#include "WebEngineService.h"

#include <Evas.h>
#include <memory>
#include <BrowserImage.h>
#include <app.h>

#include "AbstractWebEngine/TabId.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"
#include "Config/Config.h"
#include "WebView.h"

namespace tizen_browser {
namespace basic_webengine {
namespace webengine_service {

EXPORT_SERVICE(WebEngineService, "org.tizen.browser.webengineservice")

WebEngineService::WebEngineService()
    : m_initialised(false)
    , m_guiParent(nullptr)
    , m_stopped(false)
    , m_currentTabId(TabId::NONE)
    , m_tabIdCreated(-1)
{
    m_mostRecentTab.clear();
    m_tabs.clear();

#if PROFILE_MOBILE
    // init settings
    m_settings[WebEngineSettings::PAGE_OVERVIEW] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_PAGE_OVERVIEW));
    m_settings[WebEngineSettings::LOAD_IMAGES] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_LOAD_IMAGES));
    m_settings[WebEngineSettings::ENABLE_JAVASCRIPT] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_ENABLE_JAVASCRIPT));
    m_settings[WebEngineSettings::REMEMBER_FROM_DATA] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_REMEMBER_FROM_DATA));
    m_settings[WebEngineSettings::REMEMBER_PASSWORDS] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_REMEMBER_PASSWORDS));
    m_settings[WebEngineSettings::AUTOFILL_PROFILE_DATA] = boost::any_cast<bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::WEB_ENGINE_AUTOFILL_PROFILE_DATA));
#endif
}

WebEngineService::~WebEngineService()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void WebEngineService::destroyTabs()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_tabs.clear();
    m_currentWebView.reset();
}

Evas_Object * WebEngineService::getLayout()
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->getLayout();
}

void WebEngineService::init(void * guiParent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_initialised) {
        m_guiParent = guiParent;
        m_initialised = true;
    }
}

void WebEngineService::connectSignals(std::shared_ptr<WebView> webView)
{
    M_ASSERT(webView);
    webView->favIconChanged.connect(boost::bind(&WebEngineService::_favIconChanged, this, _1));
    webView->titleChanged.connect(boost::bind(&WebEngineService::_titleChanged, this, _1, _2));
    webView->uriChanged.connect(boost::bind(&WebEngineService::_uriChanged, this, _1));
    webView->downloadStarted.connect(boost::bind(&WebEngineService::_downloadStarted, this, _1));
    webView->loadFinished.connect(boost::bind(&WebEngineService::_loadFinished, this));
    webView->loadStarted.connect(boost::bind(&WebEngineService::_loadStarted, this));
    webView->loadStop.connect(boost::bind(&WebEngineService::_loadStop, this));
    webView->loadProgress.connect(boost::bind(&WebEngineService::_loadProgress, this, _1));
    webView->ready.connect(boost::bind(&WebEngineService::_ready, this, _1));
    webView->loadError.connect(boost::bind(&WebEngineService::_loadError, this));
    webView->forwardEnableChanged.connect(boost::bind(&WebEngineService::_forwardEnableChanged, this, _1));
    webView->backwardEnableChanged.connect(boost::bind(&WebEngineService::_backwardEnableChanged, this, _1));
    webView->confirmationRequest.connect(boost::bind(&WebEngineService::_confirmationRequest, this, _1));
    webView->ewkViewClicked.connect(boost::bind(&WebEngineService::webViewClicked, this));
    webView->IMEStateChanged.connect(boost::bind(&WebEngineService::_IMEStateChanged, this, _1));
    webView->snapshotCaptured.connect(boost::bind(&WebEngineService::_snapshotCaptured, this, _1));
    webView->redirectedWebPage.connect(boost::bind(&WebEngineService::_redirectedWebPage, this, _1, _2));
#if PROFILE_MOBILE
    webView->getRotation.connect(boost::bind(&WebEngineService::_getRotation, this));
#endif
}

void WebEngineService::disconnectSignals(std::shared_ptr<WebView> webView)
{
    M_ASSERT(webView);
    webView->favIconChanged.disconnect(boost::bind(&WebEngineService::_favIconChanged, this));
    webView->titleChanged.disconnect(boost::bind(&WebEngineService::_titleChanged, this, _1, _2));
    webView->uriChanged.disconnect(boost::bind(&WebEngineService::_uriChanged, this, _1));
    webView->loadFinished.disconnect(boost::bind(&WebEngineService::_loadFinished, this));
    webView->loadStarted.disconnect(boost::bind(&WebEngineService::_loadStarted, this));
    webView->loadStop.disconnect(boost::bind(&WebEngineService::_loadStop, this));
    webView->loadProgress.disconnect(boost::bind(&WebEngineService::_loadProgress, this, _1));
    webView->loadError.disconnect(boost::bind(&WebEngineService::_loadError, this));
    webView->forwardEnableChanged.disconnect(boost::bind(&WebEngineService::_forwardEnableChanged, this, _1));
    webView->backwardEnableChanged.disconnect(boost::bind(&WebEngineService::_backwardEnableChanged, this, _1));
    webView->confirmationRequest.disconnect(boost::bind(&WebEngineService::_confirmationRequest, this, _1));
    webView->ewkViewClicked.disconnect(boost::bind(&WebEngineService::webViewClicked, this));
    webView->IMEStateChanged.disconnect(boost::bind(&WebEngineService::_IMEStateChanged, this, _1));
    webView->redirectedWebPage.disconnect(boost::bind(&WebEngineService::_redirectedWebPage, this, _1, _2));
#if PROFILE_MOBILE
    webView->getRotation.disconnect(boost::bind(&WebEngineService::_getRotation, this));
#endif
}

void WebEngineService::disconnectCurrentWebViewSignals()
{
    if(m_currentWebView.get())
        disconnectSignals(m_currentWebView);
}

int WebEngineService::createTabId()
{
    m_tabIdCreated = -1;
    AbstractWebEngine::createTabId();
    if(m_tabIdCreated == -1) {
        BROWSER_LOGE("%s generated tab id == -1", __PRETTY_FUNCTION__);
    }
    return m_tabIdCreated;
}

void WebEngineService::onTabIdCreated(int tabId)
{
    m_tabIdCreated= tabId;
}

void WebEngineService::setURI(const std::string & uri)
{
    BROWSER_LOGD("[%s:%d] uri=%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    M_ASSERT(m_currentWebView);
    m_stopped = false;
    m_currentWebView->setURI(uri);
}

std::string WebEngineService::getURI() const
{
    M_ASSERT(m_currentWebView);
    if(m_currentWebView)
        return m_currentWebView->getURI();
    else
        return std::string("");
}

bool WebEngineService::isLoadError() const
{
    return m_currentWebView->isLoadError();
}


std::string WebEngineService::getTitle() const
{
    M_ASSERT(m_currentWebView);
    if (m_currentWebView) {
        if (m_stopped)
            return m_currentWebView->getURI();
        else
            return m_currentWebView->getTitle();
    } else
        return std::string("");
}

std::string WebEngineService::getUserAgent() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->getUserAgent();
}

void WebEngineService::setUserAgent(const std::string& ua)
{
    m_currentWebView->setUserAgent(ua);
}

void WebEngineService::suspend()
{
    if(tabsCount()>0) {
        M_ASSERT(m_currentWebView);
        m_currentWebView->suspend();
#if PROFILE_MOBILE
        unregisterHWKeyCallback();
#endif
    }
}

void WebEngineService::resume()
{
    if(tabsCount()>0) {
        M_ASSERT(m_currentWebView);
        m_currentWebView->resume();
#if PROFILE_MOBILE
        registerHWKeyCallback();
#endif
    }
}

bool WebEngineService::isSuspended() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->isSuspended();
}

void WebEngineService::stopLoading(void)
{
    M_ASSERT(m_currentWebView);
    m_stopped = true;
    m_currentWebView->stopLoading();
}

void WebEngineService::reload(void)
{
    M_ASSERT(m_currentWebView);
    m_stopped = false;
    m_currentWebView->reload();
}

void WebEngineService::back(void)
{
    M_ASSERT(m_currentWebView);
    m_stopped = false;
    m_currentWebView->back();
#if PROFILE_MOBILE
    closeFindOnPage();
#endif
}

void WebEngineService::forward(void)
{
    M_ASSERT(m_currentWebView);
    m_stopped = false;
    m_currentWebView->forward();
#if PROFILE_MOBILE
    closeFindOnPage();
#endif
}

bool WebEngineService::isBackEnabled() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->isBackEnabled();
}

bool WebEngineService::isForwardEnabled() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->isForwardEnabled();
}

bool WebEngineService::isLoading() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->isBackEnabled();
}

void WebEngineService::_favIconChanged(std::shared_ptr<tizen_browser::tools::BrowserImage> bi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    favIconChanged(bi);
}

void WebEngineService::_titleChanged(const std::string& title, const std::string& tabId)
{
    titleChanged(title, tabId);
}

void WebEngineService::_uriChanged(const std::string & uri)
{
    uriChanged(uri);
}

void WebEngineService::_downloadStarted(bool status)
{
    downloadStarted(status);
}

void WebEngineService::_loadFinished()
{
    loadFinished();
}

void WebEngineService::_loadStarted()
{
    loadStarted();
}

void WebEngineService::_loadStop()
{
    loadStop();
}

void WebEngineService::_loadError()
{
    loadError();
}

void WebEngineService::_forwardEnableChanged(bool enable)
{
    forwardEnableChanged(enable);
}

void WebEngineService::_backwardEnableChanged(bool enable)
{
    backwardEnableChanged(enable);
}

void WebEngineService::_loadProgress(double d)
{
    loadProgress(d);
}

void WebEngineService::_ready(TabId id)
{
    ready(id);
}

void WebEngineService::_confirmationRequest(WebConfirmationPtr c)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    confirmationRequest(c);
}

int WebEngineService::tabsCount() const
{
    return m_tabs.size();
}

TabId WebEngineService::currentTabId() const
{
    return m_currentTabId;
}

std::vector<TabContentPtr> WebEngineService::getTabContents() const {
    std::vector<TabContentPtr> result;
    for (auto const& tab : m_tabs) {
        auto tabContent = std::make_shared<TabContent>(tab.first, tab.second->getTitle());
        result.push_back(tabContent);
    }
    return result;
}

TabId WebEngineService::addTab(const std::string & uri,
        const TabId * tabInitId, const boost::optional<int> tabId,
        const std::string& title, bool desktopMode, bool incognitoMode)
{
    AbstractWebEngine::checkIfCreate();

    if (tabsCount() >= boost::any_cast<int>(tizen_browser::config::Config::getInstance().get("TAB_LIMIT")))
        return currentTabId();

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int newAdaptorId = -1;
    if(tabId) {
        newAdaptorId = *tabId;
    } else {
        // searching for next available tabId
        newAdaptorId = createTabId();
    }
    TabId newTabId(newAdaptorId);

    WebViewPtr p = std::make_shared<WebView>(reinterpret_cast<Evas_Object *>(m_guiParent), newTabId, title, incognitoMode);
    if (tabInitId)
        p->init(desktopMode, getTabView(*tabInitId));
    else
        p->init(desktopMode);

    m_tabs[newTabId] = p;

#if PROFILE_MOBILE
    setWebViewSettings(p);
#endif

    if (!uri.empty()) {
        p->setURI(uri);
    }

    AbstractWebEngine::tabCreated();

    return newTabId;
}

Evas_Object* WebEngineService::getTabView(TabId id){
    return m_tabs[id]->getLayout();
}

bool WebEngineService::switchToTab(tizen_browser::basic_webengine::TabId newTabId)
{
    BROWSER_LOGD("[%s:%d] newTabId=%s", __PRETTY_FUNCTION__, __LINE__, newTabId.toString().c_str());

    // if there was any running WebView
    if (m_currentWebView) {
        disconnectSignals(m_currentWebView);
        suspend();
    }

    m_currentWebView = m_tabs[newTabId];
    m_currentTabId = newTabId;
    m_mostRecentTab.erase(std::remove(m_mostRecentTab.begin(), m_mostRecentTab.end(), newTabId), m_mostRecentTab.end());
    m_mostRecentTab.push_back(newTabId);

    connectSignals(m_currentWebView);
    resume();

    titleChanged(m_currentWebView->getTitle(), newTabId.toString());
    uriChanged(m_currentWebView->getURI());
    forwardEnableChanged(m_currentWebView->isForwardEnabled());
    backwardEnableChanged(m_currentWebView->isBackEnabled());
    favIconChanged(m_currentWebView->getFavicon());
    currentTabChanged(m_currentTabId);
#if PROFILE_MOBILE
    m_currentWebView->orientationChanged();
#endif

    return true;
}

bool WebEngineService::closeTab()
{
    BROWSER_LOGD("[%s:%d] closing tab=%s", __PRETTY_FUNCTION__, __LINE__, m_currentTabId.toString().c_str());
    bool res = closeTab(m_currentTabId);
    return res;
}

bool WebEngineService::closeTab(TabId id) {
    BROWSER_LOGD("[%s:%d] closing tab=%s", __PRETTY_FUNCTION__, __LINE__, id.toString().c_str());

    TabId closingTabId = id;
    bool res = true;
    if(closingTabId == TabId::NONE){
        return res;
    }
    m_tabs.erase(closingTabId);
    m_mostRecentTab.erase(std::remove(m_mostRecentTab.begin(), m_mostRecentTab.end(), closingTabId), m_mostRecentTab.end());

    if (closingTabId == m_currentTabId) {
        m_currentWebView.reset();
    }
    if (m_tabs.size() == 0) {
        m_currentTabId = TabId::NONE;
    }
    else if (closingTabId == m_currentTabId && m_mostRecentTab.size()){
        res = switchToTab(m_mostRecentTab.back());
    }

    tabClosed(closingTabId);
    return res;
}

void WebEngineService::confirmationResult(WebConfirmationPtr c)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // tabId MUST be set
    M_ASSERT(c && c->getTabId() != TabId());

    // check if still exists
    if (m_tabs.find(c->getTabId()) == m_tabs.end()) {
        return;
    }

    m_tabs[c->getTabId()]->confirmationResult(c);
}

bool WebEngineService::isPrivateMode(const TabId& id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return m_tabs[id]->isPrivateMode();
}

std::shared_ptr<tizen_browser::tools::BrowserImage> WebEngineService::getSnapshotData(int width, int height)
{
    M_ASSERT(m_currentWebView);
    if(m_currentWebView)
        return m_currentWebView->captureSnapshot(width, height, false);
    else
        return std::make_shared<tizen_browser::tools::BrowserImage>();

}

std::shared_ptr<tizen_browser::tools::BrowserImage> WebEngineService::getSnapshotData(TabId id, int width, int height, bool async){
   return m_tabs[id]->captureSnapshot(width, height, async);
}

void WebEngineService::setFocus()
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->setFocus();
}

void WebEngineService::clearFocus()
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->clearFocus();
}

bool WebEngineService::hasFocus() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->hasFocus();
}


std::shared_ptr<tizen_browser::tools::BrowserImage> WebEngineService::getFavicon()
{
    M_ASSERT(m_currentWebView);
    if (m_currentWebView) {
        if (m_stopped)
            return std::make_shared<tizen_browser::tools::BrowserImage>();
        else
            return m_currentWebView->getFavicon();
    } else
        return std::make_shared<tizen_browser::tools::BrowserImage>();
}

void WebEngineService::webViewClicked()
{
    AbstractWebEngine::webViewClicked();
}

#if PROFILE_MOBILE
void WebEngineService::setWebViewSettings(std::shared_ptr<WebView> webView) {
    webView->ewkSettingsAutoFittingSet(m_settings[WebEngineSettings::PAGE_OVERVIEW]);
    webView->ewkSettingsLoadsImagesSet(m_settings[WebEngineSettings::LOAD_IMAGES]);
    webView->ewkSettingsJavascriptEnabledSet(m_settings[WebEngineSettings::ENABLE_JAVASCRIPT]);
    webView->ewkSettingsFormCandidateDataEnabledSet(m_settings[WebEngineSettings::REMEMBER_FROM_DATA]);
    webView->ewkSettingsAutofillPasswordFormEnabledSet(m_settings[WebEngineSettings::REMEMBER_PASSWORDS]);
    webView->ewkSettingsFormProfileDataEnabledSet(m_settings[WebEngineSettings::AUTOFILL_PROFILE_DATA]);
}

void WebEngineService::orientationChanged()
{
    if (m_currentWebView)
        m_currentWebView->orientationChanged();
}
#endif

int WebEngineService::getZoomFactor() const
{
    if(!m_currentWebView)
        return 0;
    return static_cast<int>(m_currentWebView->getZoomFactor()*100);

}

void WebEngineService::setZoomFactor(int zoomFactor)
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->setZoomFactor(0.01*zoomFactor);

}

void WebEngineService::clearCache()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); ++it){
            it->second->clearCache();
        }
}

void WebEngineService::clearCookies()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); ++it){
            it->second->clearCookies();
        }
}

void WebEngineService::clearPrivateData()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); ++it){
            it->second->clearPrivateData();
        }
}
void WebEngineService::clearPasswordData()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); ++it){
            it->second->clearPasswordData();
        }
}

void WebEngineService::clearFormData()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); ++it){
            it->second->clearFormData();
        }
}

void WebEngineService::searchOnWebsite(const std::string & searchString, int flags)
{
    m_currentWebView->searchOnWebsite(searchString, flags);
}

void WebEngineService::_IMEStateChanged(bool enable)
{
    IMEStateChanged(enable);
}

void WebEngineService::_snapshotCaptured(std::shared_ptr<tizen_browser::tools::BrowserImage> image)
{
    snapshotCaptured(image);
}

void WebEngineService::_redirectedWebPage(const std::string& oldUrl, const std::string& newUrl)
{
    redirectedWebPage(oldUrl, newUrl);
}

#if PROFILE_MOBILE
int WebEngineService::_getRotation()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<int> signal = getRotation();
    if (signal)
        return *signal;
    else
        return -1;
}

void WebEngineService::moreKeyPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_currentWebView);

    if (m_currentTabId == TabId::NONE || m_currentWebView->clearTextSelection())
        return;

    if (m_currentWebView->isFullScreen()) {
        m_currentWebView->exitFullScreen();
    }
}
#endif

void WebEngineService::backButtonClicked()
{
    M_ASSERT(m_currentWebView);

#if PROFILE_MOBILE
    if (m_currentWebView->clearTextSelection())
        return;

    if (m_currentWebView->isFullScreen()) {
        m_currentWebView->exitFullScreen();
        return;
    }
#endif

    if (isBackEnabled()) {
        m_currentWebView->back();
    } else if (m_currentWebView->isPrivateMode()) {
        closeTab();
        switchToWebPage();
    } else {
        ui_app_exit();
    }
}

void WebEngineService::switchToDesktopMode()
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->switchToDesktopMode();
}

void WebEngineService::switchToMobileMode()
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->switchToMobileMode();
}

bool WebEngineService::isDesktopMode() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->isDesktopMode();
}

void WebEngineService::scrollView(const int& dx, const int& dy)
{
    m_currentWebView->scrollView(dx, dy);
}

#if PROFILE_MOBILE
void WebEngineService::findWord(const char *word, Eina_Bool forward, Evas_Smart_Cb found_cb, void *data)
{
    m_currentWebView->findWord(word, forward, found_cb, data);
}

void WebEngineService::setTouchEvents(bool enabled)
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->setTouchEvents(enabled);
}

bool WebEngineService::getSettingsParam(WebEngineSettings param) {
    return m_settings.at(param);
}

void WebEngineService::setSettingsParam(WebEngineSettings param, bool value) {
    m_settings[param] = value;
    for(auto it = m_tabs.cbegin(); it != m_tabs.cend(); ++it) {
        switch (param) {
        case WebEngineSettings::PAGE_OVERVIEW:
            it->second->ewkSettingsAutoFittingSet(value);
            break;
        case WebEngineSettings::LOAD_IMAGES:
            it->second->ewkSettingsLoadsImagesSet(value);
            break;
        case WebEngineSettings::ENABLE_JAVASCRIPT:
            it->second->ewkSettingsJavascriptEnabledSet(value);
            break;
        case WebEngineSettings::REMEMBER_FROM_DATA:
            it->second->ewkSettingsFormCandidateDataEnabledSet(value);
            break;
        case WebEngineSettings::REMEMBER_PASSWORDS:
            it->second->ewkSettingsAutofillPasswordFormEnabledSet(value);
            break;
        case WebEngineSettings::AUTOFILL_PROFILE_DATA:
            it->second->ewkSettingsFormProfileDataEnabledSet(value);
            break;
        default:
            BROWSER_LOGD("[%s:%d] Warning unknown param value!", __PRETTY_FUNCTION__, __LINE__);
        }
    }
}
#endif

} /* end of webengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */
