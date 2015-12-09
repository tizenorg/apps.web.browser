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
 * WebKitEngineService.cpp
 *
 *  Created on: Apr 1, 2014
 *      Author: p.rafalski
 */

#include "browser_config.h"
#include "WebKitEngineService.h"

#include <Evas.h>
#include <memory>
#include <BrowserImage.h>
#include <app.h>

#include "AbstractWebEngine/TabId.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"
#include "WebView.h"

namespace tizen_browser {
namespace basic_webengine {
namespace webkitengine_service {

EXPORT_SERVICE(WebKitEngineService, "org.tizen.browser.webkitengineservice")

WebKitEngineService::WebKitEngineService()
    : m_initialised(false)
    , m_guiParent(nullptr)
    , m_stopped(false)
    , m_currentTabId(TabId::NONE)
{
    m_mostRecentTab.clear();
    m_tabs.clear();
    m_chronoTabs.clear();
    m_config.load("");

#if PROFILE_MOBILE
    // init settings
    m_settings[WebEngineSettings::PAGE_OVERVIEW] = boost::any_cast<bool>(m_config.get(CONFIG_KEY::WEB_ENGINE_PAGE_OVERVIEW));
    m_settings[WebEngineSettings::LOAD_IMAGES] = boost::any_cast<bool>(m_config.get(CONFIG_KEY::WEB_ENGINE_LOAD_IMAGES));
    m_settings[WebEngineSettings::ENABLE_JAVASCRIPT] = boost::any_cast<bool>(m_config.get(CONFIG_KEY::WEB_ENGINE_ENABLE_JAVASCRIPT));
    m_settings[WebEngineSettings::REMEMBER_FROM_DATA] = boost::any_cast<bool>(m_config.get(CONFIG_KEY::WEB_ENGINE_REMEMBER_FROM_DATA));
    m_settings[WebEngineSettings::REMEMBER_PASSWORDS] = boost::any_cast<bool>(m_config.get(CONFIG_KEY::WEB_ENGINE_REMEMBER_PASSWORDS));
    m_settings[WebEngineSettings::AUTOFILL_PROFILE_DATA] = boost::any_cast<bool>(m_config.get(CONFIG_KEY::WEB_ENGINE_AUTOFILL_PROFILE_DATA));
#endif
}

WebKitEngineService::~WebKitEngineService()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void WebKitEngineService::destroyTabs()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_tabs.clear();
    m_currentWebView.reset();
}

Evas_Object * WebKitEngineService::getLayout()
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->getLayout();
}

void WebKitEngineService::init(void * guiParent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_initialised) {
        m_guiParent = guiParent;
        m_initialised = true;
    }
}

void WebKitEngineService::connectSignals(std::shared_ptr<WebView> webView)
{
    M_ASSERT(webView);
    webView->favIconChanged.connect(boost::bind(&WebKitEngineService::_favIconChanged, this, _1));
    webView->titleChanged.connect(boost::bind(&WebKitEngineService::_titleChanged, this, _1, _2));
    webView->uriChanged.connect(boost::bind(&WebKitEngineService::_uriChanged, this, _1));
    webView->loadFinished.connect(boost::bind(&WebKitEngineService::_loadFinished, this));
    webView->loadStarted.connect(boost::bind(&WebKitEngineService::_loadStarted, this));
    webView->loadStop.connect(boost::bind(&WebKitEngineService::_loadStop, this));
    webView->loadProgress.connect(boost::bind(&WebKitEngineService::_loadProgress, this, _1));
    webView->loadError.connect(boost::bind(&WebKitEngineService::_loadError, this));
    webView->forwardEnableChanged.connect(boost::bind(&WebKitEngineService::_forwardEnableChanged, this, _1));
    webView->backwardEnableChanged.connect(boost::bind(&WebKitEngineService::_backwardEnableChanged, this, _1));
    webView->confirmationRequest.connect(boost::bind(&WebKitEngineService::_confirmationRequest, this, _1));
    webView->ewkViewClicked.connect(boost::bind(&WebKitEngineService::webViewClicked, this));
    webView->IMEStateChanged.connect(boost::bind(&WebKitEngineService::_IMEStateChanged, this, _1));
#ifdef HW_BACK_KEY
    webView->HWBackCalled.connect(boost::bind(&WebKitEngineService::_HWBackCalled, this));
#endif

}

void WebKitEngineService::disconnectSignals(std::shared_ptr<WebView> webView)
{
    M_ASSERT(webView);
    webView->favIconChanged.disconnect(boost::bind(&WebKitEngineService::_favIconChanged, this));
    webView->titleChanged.disconnect(boost::bind(&WebKitEngineService::_titleChanged, this, _1, _2));
    webView->uriChanged.disconnect(boost::bind(&WebKitEngineService::_uriChanged, this, _1));
    webView->loadFinished.disconnect(boost::bind(&WebKitEngineService::_loadFinished, this));
    webView->loadStarted.disconnect(boost::bind(&WebKitEngineService::_loadStarted, this));
    webView->loadStop.disconnect(boost::bind(&WebKitEngineService::_loadStop, this));
    webView->loadProgress.disconnect(boost::bind(&WebKitEngineService::_loadProgress, this, _1));
    webView->loadError.disconnect(boost::bind(&WebKitEngineService::_loadError, this));
    webView->forwardEnableChanged.disconnect(boost::bind(&WebKitEngineService::_forwardEnableChanged, this, _1));
    webView->backwardEnableChanged.disconnect(boost::bind(&WebKitEngineService::_backwardEnableChanged, this, _1));
    webView->confirmationRequest.disconnect(boost::bind(&WebKitEngineService::_confirmationRequest, this, _1));
    webView->ewkViewClicked.disconnect(boost::bind(&WebKitEngineService::webViewClicked, this));
    webView->IMEStateChanged.disconnect(boost::bind(&WebKitEngineService::_IMEStateChanged, this, _1));
#ifdef HW_BACK_KEY
    webView->HWBackCalled.disconnect(boost::bind(&WebKitEngineService::_HWBackCalled, this));
#endif

}

void WebKitEngineService::disconnectCurrentWebViewSignals()
{
    if(m_currentWebView.get())
        disconnectSignals(m_currentWebView);
}

int WebKitEngineService::createTabId()
{
    m_tabIdCreated = -1;
    AbstractWebEngine::createTabId();
    if(m_tabIdCreated == -1) {
        BROWSER_LOGE("%s generated tab id == -1", __PRETTY_FUNCTION__);
    }
    return m_tabIdCreated;
}

void WebKitEngineService::onTabIdCreated(int tabId)
{
    m_tabIdCreated= tabId;
}

void WebKitEngineService::setURI(const std::string & uri)
{
    BROWSER_LOGD("[%s:%d] uri=%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    M_ASSERT(m_currentWebView);
    m_stopped = false;
    m_currentWebView->setURI(uri);
}

std::string WebKitEngineService::getURI() const
{
    M_ASSERT(m_currentWebView);
    if(m_currentWebView)
        return m_currentWebView->getURI();
    else
        return std::string("");
}

bool WebKitEngineService::isLoadError() const
{
    return m_currentWebView->isLoadError();
}


std::string WebKitEngineService::getTitle() const
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

void WebKitEngineService::suspend()
{
    if(tabsCount()>0) {
        M_ASSERT(m_currentWebView);
        m_currentWebView->suspend();
    }
}

void WebKitEngineService::resume()
{
    if(tabsCount()>0) {
        M_ASSERT(m_currentWebView);
        m_currentWebView->resume();
    }
}

bool WebKitEngineService::isSuspended() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->isSuspended();
}

void WebKitEngineService::stopLoading(void)
{
    M_ASSERT(m_currentWebView);
    m_stopped = true;
    m_currentWebView->stopLoading();
}

void WebKitEngineService::reload(void)
{
    M_ASSERT(m_currentWebView);
    m_stopped = false;
    m_currentWebView->reload();
}

void WebKitEngineService::back(void)
{
    M_ASSERT(m_currentWebView);
    m_stopped = false;
    m_currentWebView->back();
}

void WebKitEngineService::forward(void)
{
    M_ASSERT(m_currentWebView);
    m_stopped = false;
    m_currentWebView->forward();
}

bool WebKitEngineService::isBackEnabled() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->isBackEnabled();
}

bool WebKitEngineService::isForwardEnabled() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->isForwardEnabled();
}

bool WebKitEngineService::isLoading() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->isBackEnabled();
}

void WebKitEngineService::_favIconChanged(std::shared_ptr<tizen_browser::tools::BrowserImage> bi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    favIconChanged(bi);
}

void WebKitEngineService::_titleChanged(const std::string& title, const std::string& tabId)
{
    titleChanged(title, tabId);
}

void WebKitEngineService::_uriChanged(const std::string & uri)
{
    uriChanged(uri);
}

void WebKitEngineService::_loadFinished()
{
    loadFinished();
}

void WebKitEngineService::_loadStarted()
{
    loadStarted();
}

void WebKitEngineService::_loadStop()
{
    loadStop();
}

void WebKitEngineService::_loadError()
{
    loadError();
}


void WebKitEngineService::_forwardEnableChanged(bool enable)
{
    forwardEnableChanged(enable);
}

void WebKitEngineService::_backwardEnableChanged(bool enable)
{
    backwardEnableChanged(enable);
}

void WebKitEngineService::_loadProgress(double d)
{
    loadProgress(d);
}

void WebKitEngineService::_confirmationRequest(WebConfirmationPtr c)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    confirmationRequest(c);
}

#ifdef HW_BACK_KEY
void WebKitEngineService::_HWBackCalled()
{
    HWBackCalled();
}
#endif

int WebKitEngineService::tabsCount() const
{
    return m_tabs.size();
}

TabId WebKitEngineService::currentTabId() const
{
    return m_currentTabId;
}

std::list<TabId> WebKitEngineService::listTabs() const
{
    return m_mostRecentTab;
}

std::vector<TabContentPtr> WebKitEngineService::getTabContents() const {
    std::vector<TabContentPtr> result;
    for(std::list<TabId>::const_iterator it = m_chronoTabs.begin(); it != m_chronoTabs.end(); ++it){
        WebViewPtr item = m_tabs.find(*it)->second;
        auto tabContent = std::make_shared<TabContent>(*it, item->getTitle());
        result.push_back(tabContent);
    }
    return result;
}

TabId WebKitEngineService::addTab(const std::string & uri,
        const TabId * tabInitId, const boost::optional<int> tabId,
        const std::string& title, bool desktopMode, bool incognitoMode)
{
    AbstractWebEngine::checkIfCreate();

    if (tabsCount() >= boost::any_cast<int>(m_config.get("TAB_LIMIT")))
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
    m_chronoTabs.push_front(newTabId);

    return newTabId;
}

Evas_Object* WebKitEngineService::getTabView(TabId id){
    return m_tabs[id]->getLayout();
}

bool WebKitEngineService::switchToTab(tizen_browser::basic_webengine::TabId newTabId)
{
    BROWSER_LOGD("[%s:%d] newTabId=%s", __PRETTY_FUNCTION__, __LINE__, newTabId.toString().c_str());

    // if there was any running WebView
    if (m_currentWebView) {
        disconnectSignals(m_currentWebView);
        suspend();
    }

    m_currentWebView = m_tabs[newTabId];
    m_currentTabId = newTabId;
    m_mostRecentTab.remove(newTabId);
    m_mostRecentTab.push_back(newTabId);

    connectSignals(m_currentWebView);
    resume();

    titleChanged(m_currentWebView->getTitle(), newTabId.toString());
    uriChanged(m_currentWebView->getURI());
    forwardEnableChanged(m_currentWebView->isForwardEnabled());
    backwardEnableChanged(m_currentWebView->isBackEnabled());
    favIconChanged(m_currentWebView->getFavicon());
    currentTabChanged(m_currentTabId);

    return true;
}

bool WebKitEngineService::closeTab()
{
    BROWSER_LOGD("[%s:%d] closing tab=%s", __PRETTY_FUNCTION__, __LINE__, m_currentTabId.toString().c_str());
    bool res = closeTab(m_currentTabId);
    return res;
}

bool WebKitEngineService::closeTab(TabId id) {
    BROWSER_LOGD("[%s:%d] closing tab=%s", __PRETTY_FUNCTION__, __LINE__, id.toString().c_str());
    BROWSER_LOGD("[%s:%d] NONE tab=%s", __PRETTY_FUNCTION__, __LINE__, TabId::NONE.toString().c_str());

    TabId closingTabId = id;
    bool res = true;
    if(closingTabId == TabId::NONE){
        return res;
    }
    m_tabs.erase(closingTabId);
    m_chronoTabs.remove(closingTabId);
    m_mostRecentTab.remove(closingTabId);
    if (m_tabs.size() == 0) {
        m_currentTabId = TabId::NONE;
    }
    else if (closingTabId == m_currentTabId && m_mostRecentTab.size()){
        res = switchToTab(m_mostRecentTab.back());
    }

    tabClosed(closingTabId);

    return res;
}

bool WebKitEngineService::nextTab()
{
    // Empty implementation, with random UUID as ID we cannot distinguish between previous and next tab. Functionality not used.
    return false;
}

bool WebKitEngineService::prevTab()
{
    // Empty implementation, with random UUID as ID we cannot distinguish between previous and next tab. Functionality not used.
    return false;
}

void WebKitEngineService::confirmationResult(WebConfirmationPtr c)
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

bool WebKitEngineService::isPrivateMode(const TabId& id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return m_tabs[id]->isPrivateMode();
}

std::shared_ptr<tizen_browser::tools::BrowserImage> WebKitEngineService::getSnapshotData(int width, int height)
{
    M_ASSERT(m_currentWebView);
    if(m_currentWebView)
        return m_currentWebView->captureSnapshot(width, height);
    else
        return std::make_shared<tizen_browser::tools::BrowserImage>();

}

std::shared_ptr<tizen_browser::tools::BrowserImage> WebKitEngineService::getSnapshotData(TabId id, int width, int height){
   return m_tabs[id]->captureSnapshot(width,height);
}

void WebKitEngineService::setFocus()
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->setFocus();
}

void WebKitEngineService::clearFocus()
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->clearFocus();
}

bool WebKitEngineService::hasFocus() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->hasFocus();
}


std::shared_ptr<tizen_browser::tools::BrowserImage> WebKitEngineService::getFavicon()
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

void WebKitEngineService::webViewClicked()
{
    AbstractWebEngine::webViewClicked();
}

#if PROFILE_MOBILE
void WebKitEngineService::setWebViewSettings(std::shared_ptr<WebView> webView) {
    webView->ewkSettingsAutoFittingSet(m_settings[WebEngineSettings::PAGE_OVERVIEW]);
    webView->ewkSettingsLoadsImagesSet(m_settings[WebEngineSettings::LOAD_IMAGES]);
    webView->ewkSettingsJavascriptEnabledSet(m_settings[WebEngineSettings::ENABLE_JAVASCRIPT]);
    webView->ewkSettingsFormCandidateDataEnabledSet(m_settings[WebEngineSettings::REMEMBER_FROM_DATA]);
    webView->ewkSettingsAutofillPasswordFormEnabledSet(m_settings[WebEngineSettings::REMEMBER_PASSWORDS]);
    webView->ewkSettingsFormProfileDataEnabledSet(m_settings[WebEngineSettings::AUTOFILL_PROFILE_DATA]);
}
#endif

int WebKitEngineService::getZoomFactor() const
{
    if(!m_currentWebView)
        return 0;
    return static_cast<int>(m_currentWebView->getZoomFactor()*100);

}

void WebKitEngineService::setZoomFactor(int zoomFactor)
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->setZoomFactor(0.01*zoomFactor);

}

void WebKitEngineService::clearCache()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); ++it){
            it->second->clearCache();
        }
}

void WebKitEngineService::clearCookies()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); ++it){
            it->second->clearCookies();
        }
}

void WebKitEngineService::clearPrivateData()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); ++it){
            it->second->clearPrivateData();
        }
}
void WebKitEngineService::clearPasswordData()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); ++it){
            it->second->clearPasswordData();
        }
}

void WebKitEngineService::clearFormData()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); ++it){
            it->second->clearFormData();
        }
}

void WebKitEngineService::searchOnWebsite(const std::string & searchString, int flags)
{
    m_currentWebView->searchOnWebsite(searchString, flags);
}

void WebKitEngineService::_IMEStateChanged(bool enable)
{
    IMEStateChanged(enable);
}

void WebKitEngineService::backButtonClicked()
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

void WebKitEngineService::switchToDesktopMode()
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->switchToDesktopMode();
}

void WebKitEngineService::switchToMobileMode()
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->switchToMobileMode();
}

bool WebKitEngineService::isDesktopMode() const
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->isDesktopMode();
}

void WebKitEngineService::scrollView(const int& dx, const int& dy)
{
    m_currentWebView->scrollView(dx, dy);
}

#if PROFILE_MOBILE
void WebKitEngineService::setTouchEvents(bool enabled)
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->setTouchEvents(enabled);
}

bool WebKitEngineService::getSettingsParam(WebEngineSettings param) {
    return m_settings.at(param);
}

void WebKitEngineService::setSettingsParam(WebEngineSettings param, bool value) {
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

} /* end of webkitengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */
