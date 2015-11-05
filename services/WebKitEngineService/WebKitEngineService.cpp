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
#include "AbstractWebEngine/TabThumbCache.h"

#include <Evas.h>
#include <memory>
#include <BrowserImage.h>
#include <app.h>

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
}

WebKitEngineService::~WebKitEngineService()
{
}

Evas_Object * WebKitEngineService::getLayout()
{
    M_ASSERT(m_currentWebView);
    return m_currentWebView->getLayout();
}

void WebKitEngineService::init(void * guiParent)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
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
    webView->cofirmationRequest.connect(boost::bind(&WebKitEngineService::_confirmationRequest, this, _1));
    webView->ewkViewClicked.connect(boost::bind(&WebKitEngineService::webViewClicked, this));
    webView->IMEStateChanged.connect(boost::bind(&WebKitEngineService::_IMEStateChanged, this, _1));
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
    webView->cofirmationRequest.disconnect(boost::bind(&WebKitEngineService::_confirmationRequest, this, _1));
    webView->ewkViewClicked.disconnect(boost::bind(&WebKitEngineService::webViewClicked, this));
    webView->IMEStateChanged.disconnect(boost::bind(&WebKitEngineService::_IMEStateChanged, this, _1));
}

void WebKitEngineService::disconnectCurrentWebViewSignals()
{
    if(m_currentWebView.get())
        disconnectSignals(m_currentWebView);
}

void WebKitEngineService::setURI(const std::string & uri)
{
    BROWSER_LOGD("%s:%d %s uri=%s", __FILE__, __LINE__, __func__, uri.c_str());
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
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    confirmationRequest(c);
}

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

std::vector<std::shared_ptr<TabContent> > WebKitEngineService::getTabContents() const {
    std::vector<std::shared_ptr<TabContent>> result;
    for(std::list<TabId>::const_iterator it = m_chronoTabs.begin(); it != m_chronoTabs.end(); it++){
        WebViewPtr item = m_tabs.find(*it)->second;

        tizen_browser::services::TabThumbCache* thumbCache = tizen_browser::services::TabThumbCache::getInstance();
        std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail = thumbCache->getThumb(*it);

        auto tabContent = std::make_shared<TabContent>(*it, item->getTitle(), thumbnail);
        result.push_back(tabContent);
    }
    return result;
}

TabId WebKitEngineService::addTab(const std::string & uri, const TabId * openerId, const std::string& title, bool desktopMode, bool incognitoMode)
{
    AbstractWebEngine::checkIfCreate();

    config::DefaultConfig config;
    config.load("");

    if (tabsCount() >= boost::any_cast<int>(config.get("TAB_LIMIT")))
        return currentTabId();

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    // searching for next available tabId
    TabId newTabId;
    WebViewPtr p = std::make_shared<WebView>(reinterpret_cast<Evas_Object *>(m_guiParent), newTabId, title, incognitoMode);
    if (openerId)
        p->init(desktopMode, getTabView(*openerId));
    else
        p->init(desktopMode);

    m_tabs[newTabId] = p;

    if (!uri.empty()) {
        p->setURI(uri);
    }

    switchToTab(newTabId);
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
    BROWSER_LOGD("%s:%d %s closing tab=%s", __FILE__, __LINE__, __func__, m_currentTabId.toString().c_str());
    bool res = closeTab(m_currentTabId);
    return res;
}

bool WebKitEngineService::closeTab(TabId id) {
    BROWSER_LOGD("%s:%d %s closing tab=%s", __FILE__, __LINE__, __func__, id.toString().c_str());
    BROWSER_LOGD("%s:%d %s NONE tab=%s", __FILE__, __LINE__, __func__, TabId::NONE.toString().c_str());

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
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
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
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
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

void WebKitEngineService::clearPrivateData()
{
    for(std::map<TabId, WebViewPtr>::const_iterator it = m_tabs.begin(); it != m_tabs.end(); it++){
            it->second->clearPrivateData();
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
    if (isBackEnabled()) {
        m_currentWebView->back();
    } else if (m_currentWebView->isPrivateMode()) {
        closeTab();
        switchToWebPage();
    } else {
        app_efl_exit();
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

void WebKitEngineService::setTouchEvents(bool enabled)
{
    M_ASSERT(m_currentWebView);
    m_currentWebView->setTouchEvents(enabled);
}

} /* end of webkitengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */
