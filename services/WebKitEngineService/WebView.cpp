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
 * WebView.cpp
 *
 *  Created on: Apr 1, 2014
 *      Author: p.rafalski
 */

#include "WebView.h"

#if defined(USE_EWEBKIT)
#include <ewk_chromium.h>
#endif

#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <Elementary.h>
#include <Evas.h>

#include "AbstractWebEngine/AbstractWebEngine.h"
#include "AbstractWebEngine/TabThumbCache.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"
#include "EflTools.h"
#include "GeneralTools.h"
#include "Tools/WorkQueue.h"
#include "ServiceManager.h"

#define certificate_crt_path CERTS_DIR
#define APPLICATION_NAME_FOR_USER_AGENT "SamsungBrowser/1.0"

using namespace tizen_browser::tools;

namespace tizen_browser {
namespace basic_webengine {
namespace webkitengine_service {

WebView::WebView(Evas_Object * obj, TabId tabId)
    : m_parent(obj)
    , m_tabId(tabId)
    , m_ewkView(NULL)
    , m_isLoading(false)
    , m_loadError(false)
{
    config.load("whatever");
}

WebView::~WebView()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    if (m_ewkView) {
        unregisterCallbacks();
    }
}

void WebView::init(Evas_Object * opener)
{
#if defined(USE_EWEBKIT)
    m_ewkView = ewk_view_add(evas_object_evas_get(m_parent));

    evas_object_data_set(m_ewkView, "_container", this);
    BROWSER_LOGD("%s:%d %s self=%p", __FILE__, __LINE__, __func__, this);

    evas_object_color_set(m_ewkView, 255, 255, 255, 255);
    evas_object_size_hint_weight_set(m_ewkView, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_ewkView, EVAS_HINT_FILL, EVAS_HINT_FILL);
    ewk_view_application_name_for_user_agent_set(m_ewkView, APPLICATION_NAME_FOR_USER_AGENT);
    //\todo: when value is other than 1.0, scroller is located improperly
//    ewk_view_device_pixel_ratio_set(m_ewkView, 1.0f);

#if PLATFORM(TIZEN)
    ewk_view_resume(m_ewkView);
#endif
    // set local storage, favion, cookies
    std::string webkit_path =  boost::any_cast <std::string> (config.get("webkit/dir"));

    if (m_ewkView)
    {
        Ewk_Context *context = ewk_view_context_get(m_ewkView);
        if (context)
        {
            ewk_context_cache_model_set(context, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);
            ewk_context_favicon_database_directory_set(context, (webkit_path + std::string("/favicon")).c_str());
            ewk_cookie_manager_persistent_storage_set(ewk_context_cookie_manager_get(context)
                                                     , (webkit_path + std::string("/cookies.db")).c_str()
                                                     , EWK_COOKIE_PERSISTENT_STORAGE_SQLITE);
        }
    }

///\note Odroid modification - not exists in WebKit API
//     ewk_cookie_manager_widget_cookie_directory_set(ewk_context_cookie_manager_get(context), webkit_path.c_str());
    setupEwkSettings();
    registerCallbacks();
#else
    m_ewkView = evas_object_rectangle_add(evas_object_evas_get(m_parent));
#endif
}

void WebView::registerCallbacks()
{
#if defined(USE_EWEBKIT)
    evas_object_smart_callback_add(m_ewkView, "load,started", __loadStarted, this);
    evas_object_smart_callback_add(m_ewkView, "load,stop", __loadStop, this);
    evas_object_smart_callback_add(m_ewkView, "load,finished", __loadFinished, this);
    evas_object_smart_callback_add(m_ewkView, "load,progress", __loadProgress, this);
    evas_object_smart_callback_add(m_ewkView, "load,error", __loadError, this);

    evas_object_smart_callback_add(m_ewkView, "title,changed", __titleChanged, this);
    evas_object_smart_callback_add(m_ewkView, "url,changed", __urlChanged, this);

    evas_object_smart_callback_add(m_ewkView, "back,forward,list,changed", __backForwardListChanged, this);

    evas_object_smart_callback_add(m_ewkView, "create,window", __newWindowRequest, this);
    evas_object_smart_callback_add(m_ewkView, "close,window", __closeWindowRequest, this);

    evas_object_smart_callback_add(m_ewkView, "geolocation,permission,request", __geolocationPermissionRequest, this);
    evas_object_smart_callback_add(m_ewkView, "usermedia,permission,request", __usermediaPermissionRequest, this);
    evas_object_smart_callback_add(m_ewkView, "notification,permission,request", __notificationPermissionRequest, this);
    evas_object_smart_callback_add(m_ewkView, "authentication,request", __authenticationRequest, this);
    evas_object_smart_callback_add(m_ewkView, "request,certificate,confirm", __requestCertificationConfirm, this);

    evas_object_event_callback_add(m_ewkView, EVAS_CALLBACK_MOUSE_DOWN, __setFocusToEwkView, this);
    evas_object_smart_callback_add(m_ewkView, "icon,received", __faviconChanged, this);

    evas_object_smart_callback_add(m_ewkView, "editorclient,ime,closed", __IMEClosed, this);
    evas_object_smart_callback_add(m_ewkView, "editorclient,ime,opened", __IMEOpened, this);
#endif
}

void WebView::unregisterCallbacks()
{
#if defined(USE_EWEBKIT)
    evas_object_smart_callback_del_full(m_ewkView, "load,started", __loadStarted, this);
    evas_object_smart_callback_del_full(m_ewkView, "load,stop", __loadStop, this);
    evas_object_smart_callback_del_full(m_ewkView, "load,finished", __loadFinished, this);
    evas_object_smart_callback_del_full(m_ewkView, "load,progress", __loadProgress, this);
    evas_object_smart_callback_del_full(m_ewkView, "load,error", __loadError, this);

    evas_object_smart_callback_del_full(m_ewkView, "title,changed", __titleChanged, this);
    evas_object_smart_callback_del_full(m_ewkView, "url,changed", __urlChanged, this);

    evas_object_smart_callback_del_full(m_ewkView, "back,forward,list,changed", __backForwardListChanged, this);

    evas_object_smart_callback_del_full(m_ewkView, "create,window", __newWindowRequest, this);
    evas_object_smart_callback_del_full(m_ewkView, "close,window", __closeWindowRequest, this);

    evas_object_smart_callback_del_full(m_ewkView, "geolocation,permission,request", __geolocationPermissionRequest, this);
    evas_object_smart_callback_del_full(m_ewkView, "usermedia,permission,request", __usermediaPermissionRequest, this);
    evas_object_smart_callback_del_full(m_ewkView, "notification,permission,request", __notificationPermissionRequest, this);
    evas_object_smart_callback_del_full(m_ewkView, "authentication,request", __authenticationRequest, this);
    evas_object_smart_callback_del_full(m_ewkView, "request,certificate,confirm", __requestCertificationConfirm, this);

    evas_object_event_callback_del(m_ewkView, EVAS_CALLBACK_MOUSE_DOWN, __setFocusToEwkView);
    evas_object_smart_callback_del_full(m_ewkView, "icon,received", __faviconChanged, this);

    evas_object_smart_callback_del_full(m_ewkView, "editorclient,ime,closed", __IMEClosed, this);
    evas_object_smart_callback_del_full(m_ewkView, "editorclient,ime,opened", __IMEOpened, this);
#endif
}

void WebView::setupEwkSettings()
{
#if defined(USE_EWEBKIT)
#if PLATFORM(TIZEN)
    Ewk_Settings * settings = ewk_view_settings_get(m_ewkView);
    ewk_settings_uses_keypad_without_user_action_set(settings, EINA_FALSE);
#endif
#endif
}

Evas_Object * WebView::getLayout()
{
    return m_ewkView;
}

void WebView::setURI(const std::string & uri)
{
    BROWSER_LOGD("%s:%d %s uri=%s", __FILE__, __LINE__, __func__, uri.c_str());
#if defined(USE_EWEBKIT)
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ewk_view_url_set(m_ewkView, uri.c_str());
    m_loadError = false;
#endif
}

std::string WebView::getURI(void)
{
#if defined(USE_EWEBKIT)
    BROWSER_LOGD("%s:%d %s uri=%s", __FILE__, __LINE__, __func__, ewk_view_url_get(m_ewkView));
    return tizen_browser::tools::fromChar(ewk_view_url_get(m_ewkView));
#else
    return std::string();
#endif
}

std::string WebView::getTitle(void)
{
    return m_title;
}

void WebView::stopLoading(void)
{
#if defined(USE_EWEBKIT)
    ewk_view_stop(m_ewkView);
#endif
    loadStop();
}

void WebView::reload(void)
{
#if defined(USE_EWEBKIT)
    if(m_loadError)
    {
        m_loadError = false;
        ewk_view_url_set(m_ewkView, ewk_view_url_get(m_ewkView));
    }
    else
        ewk_view_reload(m_ewkView);
#endif
}

void WebView::back(void)
{
#if defined(USE_EWEBKIT)
    m_loadError = false;
    ewk_view_back(m_ewkView);
#endif
}

void WebView::forward(void)
{
#if defined(USE_EWEBKIT)
    m_loadError = false;
    ewk_view_forward(m_ewkView);
#endif
}

bool WebView::isBackEnabled(void)
{
#if defined(USE_EWEBKIT)
    return ewk_view_back_possible(m_ewkView);
#else
    return false;
#endif
}

bool WebView::isForwardEnabled(void)
{
#if defined(USE_EWEBKIT)
    return ewk_view_forward_possible(m_ewkView);
#else
    return false;
#endif
}

bool WebView::isLoading()
{
    return m_isLoading;
}

bool WebView::isLoadError() const
{
    return m_loadError;
}

void WebView::setPrivateMode(bool state)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    M_ASSERT(m_ewkView);

#if defined(USE_EWEBKIT)
#if PLATFORM(TIZEN)
    Ewk_Settings * settings = ewk_view_settings_get(m_ewkView);
#else
    Ewk_Settings * settings = ewk_page_group_settings_get(ewk_view_page_group_get(m_ewkView));
#endif
    ewk_settings_private_browsing_enabled_set(settings, state);
    if (m_ewkView)
    {
        Ewk_Context *context = ewk_view_context_get(m_ewkView);
        if (context)
        {
            if(state)
            {
                 ewk_cookie_manager_accept_policy_set(ewk_context_cookie_manager_get(context), EWK_COOKIE_ACCEPT_POLICY_NEVER);
            }
            else
            {
                 ewk_cookie_manager_accept_policy_set(ewk_context_cookie_manager_get(context), EWK_COOKIE_ACCEPT_POLICY_ALWAYS);
            }
        }
    }
#endif
}

void WebView::confirmationResult(WebConfirmationPtr confirmation)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

#if defined(USE_EWEBKIT)
#if PLATFORM(TIZEN)
    switch(confirmation->getConfirmationType()) {
    case WebConfirmation::ConfirmationType::Geolocation: {
        Ewk_Geolocation_Permission_Request *request = m_confirmationGeolocationMap[confirmation];
        Eina_Bool result;
        if (confirmation->getResult() == WebConfirmation::ConfirmationResult::Confirmed)
            result = EINA_TRUE;
        else if (confirmation->getResult() == WebConfirmation::ConfirmationResult::Rejected)
            result = EINA_FALSE;
        else {
            BROWSER_LOGE("Wrong ConfirmationResult");
            break;
        }
        // set geolocation permission
        ewk_geolocation_permission_reply(request, result);
        ewk_view_resume(m_ewkView);

        // remove from map
        m_confirmationGeolocationMap.erase(confirmation);
        break;
    }
    case WebConfirmation::ConfirmationType::UserMedia: {
        Ewk_User_Media_Permission_Request *request = m_confirmationUserMediaMap[confirmation];
        Eina_Bool result;
        if (confirmation->getResult() == WebConfirmation::ConfirmationResult::Confirmed)
            result = EINA_TRUE;
        else if (confirmation->getResult() == WebConfirmation::ConfirmationResult::Rejected)
            result = EINA_FALSE;
        else {
            BROWSER_LOGE("Wrong ConfirmationResult");
            break;
        };

        // set usermedia permission
        ewk_user_media_permission_reply(request, result);
        ewk_view_resume(m_ewkView);

        // remove from map
        m_confirmationUserMediaMap.erase(confirmation);
        break;
    }
    case WebConfirmation::ConfirmationType::Notification: {
        Ewk_Notification_Permission_Request *request = m_confirmationNotificationMap[confirmation];
        Eina_Bool result;
        if (confirmation->getResult() == WebConfirmation::ConfirmationResult::Confirmed)
            result = EINA_TRUE;
        else if (confirmation->getResult() == WebConfirmation::ConfirmationResult::Rejected)
            result = EINA_FALSE;
        else {
            BROWSER_LOGE("Wrong ConfirmationResult");
            break;
        }

        // set notification permission
        ewk_notification_permission_reply(request, result);
        ewk_view_resume(m_ewkView);

        // remove from map
        m_confirmationNotificationMap.erase(confirmation);
        break;
    }
    case WebConfirmation::ConfirmationType::CertificateConfirmation: {
        CertificateConfirmationPtr cert = std::dynamic_pointer_cast<CertificateConfirmation, WebConfirmation>(confirmation);
        Ewk_Certificate_Policy_Decision *request = m_confirmationCertificatenMap[cert];
        Eina_Bool result;
        if (cert->getResult() == WebConfirmation::ConfirmationResult::Confirmed)
            result = EINA_TRUE;
        else if (cert->getResult() == WebConfirmation::ConfirmationResult::Rejected)
            result = EINA_FALSE;
        else {
            BROWSER_LOGE("Wrong ConfirmationResult");
            break;
        }

        // set certificate confirmation
        BROWSER_LOGE("NOT IMPLEMENTED: Certificate Confirmation handling!");

        // remove from map
        m_confirmationCertificatenMap.erase(cert);
        break;
    }
    case WebConfirmation::ConfirmationType::Authentication: {
        AuthenticationConfirmationPtr auth = std::dynamic_pointer_cast<AuthenticationConfirmation, WebConfirmation>(confirmation);
        Ewk_Auth_Request *request = m_confirmationAuthenticationMap[auth];
        if (auth->getResult() == WebConfirmation::ConfirmationResult::Confirmed) {
            BROWSER_LOGE("NOT IMPLEMENTED: Autenthication Request Confirmation handling!");
        } else if (auth->getResult() == WebConfirmation::ConfirmationResult::Rejected) {
            BROWSER_LOGE("NOT IMPLEMENTED: Autenthication Request Rejection handling!");
        } else {
            BROWSER_LOGE("Wrong ConfirmationResult");
            break;
        }

        // remove from map
        m_confirmationAuthenticationMap.erase(auth);
        break;
    }
    default:
        break;
    }
#else
   (void)confirmation;
#endif
#endif
}

std::shared_ptr<tizen_browser::tools::BrowserImage> WebView::captureSnapshot(int targetWidth, int targetHeight)
{
    BROWSER_LOGD("%s:%d %s NOT IMPLEMENTED, returning empty BrowserImage object", __FILE__, __LINE__, __func__);
    M_ASSERT(m_ewkView);
    M_ASSERT(targetWidth);
    M_ASSERT(targetHeight);
    Evas_Coord vw, vh;
    std::shared_ptr<tizen_browser::tools::BrowserImage> noImage = std::make_shared<tizen_browser::tools::BrowserImage>();
    evas_object_geometry_get(m_ewkView, NULL, NULL, &vw, &vh);
    if (vw == 0 || vh == 0)
        return noImage;

    Eina_Rectangle area;
    double snapshotProportions = (double)(targetWidth) /(double)(targetHeight);
    double webkitProportions = (double)(vw) /(double)(vh);
    if (webkitProportions >= snapshotProportions) {
        // centring position of screenshot
        area.x = (vw*getZoomFactor()/2) - (vh*getZoomFactor()*snapshotProportions/2);
        area.y = 0;
        area.w = vh*getZoomFactor()*snapshotProportions;
        area.h = vh*getZoomFactor();
    }
    else {
        area.x = 0;
        area.y = 0;
        area.w = vw*getZoomFactor();
        area.h = vw*getZoomFactor()/snapshotProportions;
    }
    if (area.w == 0 || area.h == 0)
        return noImage;

    double scaleW = (double)targetWidth / (double)(area.w);
    double scaleH = (double)targetHeight / (double)(area.h);

#if defined(USE_EWEBKIT)
#if PLATFORM(TIZEN)
    Evas_Object *snapshot = nullptr;
    // TODO use ewk_view_screenshot_contents_get_async API here
    if (snapshot)
        return tizen_browser::tools::EflTools::getBrowserImage(snapshot);
#endif
#endif

    return noImage;
}

#if defined(USE_EWEBKIT)
void WebView::__setFocusToEwkView(void * data, Evas * /* e */, Evas_Object * /* obj */, void * /* event_info */)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    WebView * self = reinterpret_cast<WebView *>(data);

    if(!self->hasFocus())
        self->ewkViewClicked();
}

void WebView::__newWindowRequest(void *data, Evas_Object *, void *out)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    WebView * self = reinterpret_cast<WebView *>(data);
    BROWSER_LOGD("%s:%d %s self=%p", __FILE__, __LINE__, __func__, self);
    BROWSER_LOGD("Window creating in tab: %s", self->getTabId().toString().c_str());
    std::shared_ptr<basic_webengine::AbstractWebEngine<Evas_Object>>  m_webEngine;
    m_webEngine = std::dynamic_pointer_cast
    <
        basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));
    M_ASSERT(m_webEngine);

    /// \todo: Choose newly created tab.
    TabId id = m_webEngine->addTab(std::string(), &self->getTabId());
    BROWSER_LOGD("Created tab: %s", id.toString().c_str());

    Evas_Object* tab_ewk_view = m_webEngine->getTabView(id);
    *static_cast<Evas_Object**>(out) = tab_ewk_view;
}

void WebView::__closeWindowRequest(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    WebView * self = reinterpret_cast<WebView *>(data);
    std::shared_ptr<AbstractWebEngine<Evas_Object>> m_webEngine =
                std::dynamic_pointer_cast
                <basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
                (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));
    m_webEngine->closeTab(self->getTabId());
}

void WebView::__loadStarted(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    WebView * self = reinterpret_cast<WebView *>(data);

    BROWSER_LOGD("%s:%d\n\t %s", __func__, __LINE__, ewk_view_url_get(self->m_ewkView));

    self->m_isLoading = true;
    self->loadStarted();
    tizen_browser::services::TabThumbCache* cache = tizen_browser::services::TabThumbCache::getInstance();
    cache->clearThumb(self->m_tabId);
}

void WebView::__loadStop(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    WebView * self = reinterpret_cast<WebView *>(data);
    self->m_isLoading = false;

    self->loadStop();
}

void WebView::__loadFinished(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    WebView * self = reinterpret_cast<WebView *>(data);
    self->m_isLoading = false;
    self->m_loadProgress = 1;

    self->loadFinished();
    self->loadProgress(self->m_loadProgress);
    tizen_browser::services::TabThumbCache* cache = tizen_browser::services::TabThumbCache::getInstance();
    cache->updateThumb(self->m_tabId);
}

void WebView::__loadProgress(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    WebView * self = reinterpret_cast<WebView *>(data);
    self->m_loadProgress = *(double *)event_info;

    self->loadProgress(self->m_loadProgress);
}

void WebView::__loadError(void* data, Evas_Object * obj, void* ewkError)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    WebView *self = reinterpret_cast<WebView*>(data);
    Ewk_Error *error = reinterpret_cast<Ewk_Error*>(ewkError);
    Ewk_Error_Type errorType = ewk_error_type_get(error);

    BROWSER_LOGD("[%s:%d] ewk_error_type: %d ",
                 __PRETTY_FUNCTION__, __LINE__, errorType);

    BROWSER_LOGD("[%s:%d] emiting signal ", __PRETTY_FUNCTION__, __LINE__);
    int errorCode = ewk_error_code_get(error);
    if(errorCode == EWK_ERROR_NETWORK_STATUS_CANCELLED)
    {
        BROWSER_LOGD("Stop signal emitted");
        BROWSER_LOGD("Error description: %s", ewk_error_description_get(error));
        evas_object_smart_callback_call(obj, "load,stop", NULL);
    }
    else
    {
        self->loadError();
        self->m_loadError=true;
    }
}

void WebView::__titleChanged(void * data, Evas_Object * obj, void * /* event_info */)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    WebView * self = reinterpret_cast<WebView *>(data);
    self->m_title = tizen_browser::tools::fromChar(ewk_view_title_get(obj));

    self->titleChanged(self->m_title);
}

void WebView::__urlChanged(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    WebView * self = reinterpret_cast<WebView *>(data);
    BROWSER_LOGD("URL changed for tab: %s", self->getTabId().toString().c_str());
    std::shared_ptr<basic_webengine::AbstractWebEngine<Evas_Object>> m_webEngine;
    m_webEngine = std::dynamic_pointer_cast<basic_webengine::AbstractWebEngine<Evas_Object>, tizen_browser::core::AbstractService>(
            tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));
    M_ASSERT(m_webEngine);
    self->uriChanged(tizen_browser::tools::fromChar(reinterpret_cast<const char *>(event_info)));
    self->tabIdChecker(self->m_tabId);
}

void WebView::__backForwardListChanged(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    WebView * self = reinterpret_cast<WebView *>(data);
    self->backwardEnableChanged(self->isBackEnabled());
    self->forwardEnableChanged(self->isForwardEnabled());
}

void WebView::__faviconChanged(void* data, Evas_Object*, void*)
{
    if(data)
    {
        WebView * self = static_cast<WebView *>(data);
        Evas_Object * favicon = ewk_context_icon_database_icon_object_add(ewk_view_context_get(self->m_ewkView), ewk_view_url_get(self->m_ewkView),evas_object_evas_get(self->m_ewkView));
        if (favicon) {
            BROWSER_LOGD("[%s:%d] Favicon received", __PRETTY_FUNCTION__, __LINE__);
            self->faviconImage = tizen_browser::tools::EflTools::getBrowserImage(favicon);
            evas_object_unref(favicon);
            self->favIconChanged(self->faviconImage);
        }
    }
}

void WebView::__IMEClosed(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s", __func__);
    WebView * self = reinterpret_cast<WebView *>(data);
    self->IMEStateChanged(false);
}

void WebView::__IMEOpened(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s", __func__);
    WebView * self = reinterpret_cast<WebView *>(data);
    self->IMEStateChanged(true);
}

std::string WebView::securityOriginToUri(const Ewk_Security_Origin *origin)
{
    std::string protocol = tizen_browser::tools::fromChar(ewk_security_origin_protocol_get(origin));
    std::string uri = tizen_browser::tools::fromChar(ewk_security_origin_host_get(origin));
    std::string url = (boost::format("%1%://%2%") % protocol % uri).str();
    return url;
}

void WebView::__geolocationPermissionRequest(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
#if PLATFORM(TIZEN)
    WebView * self = reinterpret_cast<WebView *>(data);

    Ewk_Geolocation_Permission_Request *request = reinterpret_cast<Ewk_Geolocation_Permission_Request *>(event_info);
    if (!request)
        return;

    // suspend webview
    ewk_view_suspend(self->m_ewkView);

    std::string url = WebView::securityOriginToUri(ewk_geolocation_permission_request_origin_get(request));

    ///\todo add translations
    std::string message = (boost::format("%1% Requests your location") % url).str();

    WebConfirmationPtr c = std::make_shared<WebConfirmation>(WebConfirmation::ConfirmationType::Geolocation, self->m_tabId, url, message);

    // store
    self->m_confirmationGeolocationMap[c] = request;

    self->cofirmationRequest(c);
#endif
}

void WebView::__usermediaPermissionRequest(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
#if PLATFORM(TIZEN)
    WebView * self = reinterpret_cast<WebView *>(data);

    Ewk_User_Media_Permission_Request *request = reinterpret_cast<Ewk_User_Media_Permission_Request *>(event_info);
    if (!request)
        return;

    // suspend webview
    ewk_view_suspend(self->m_ewkView);

    ///\todo add translations
    std::string message = "User media permission request";

    WebConfirmationPtr c = std::make_shared<WebConfirmation>(WebConfirmation::ConfirmationType::UserMedia, self->m_tabId, std::string(), message);

    // store
    self->m_confirmationUserMediaMap[c] = request;

    self->cofirmationRequest(c);
#endif
}

void WebView::__notificationPermissionRequest(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
#if PLATFORM(TIZEN)
    WebView * self = reinterpret_cast<WebView *>(data);

    Ewk_Notification_Permission_Request *request = reinterpret_cast<Ewk_Notification_Permission_Request *>(event_info);
    if (!request)
        return;

    // suspend webview
    ewk_view_suspend(self->m_ewkView);

    ///\todo add translations
    std::string message = (boost::format("%1% wants to display notifications") % self->getURI()).str();

    WebConfirmationPtr c = std::make_shared<WebConfirmation>(WebConfirmation::ConfirmationType::Notification, self->m_tabId, self->getURI(), message);

    // store
    self->m_confirmationNotificationMap[c] = request;

    self->cofirmationRequest(c);
#endif
}

void WebView::__authenticationRequest(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
#if PLATFORM(TIZEN)
    WebView * self = reinterpret_cast<WebView *>(data);

    Ewk_Auth_Request *request = reinterpret_cast<Ewk_Auth_Request *>(event_info);
    EINA_SAFETY_ON_NULL_RETURN(request);

    std::string url = self->getURI();
    std::string message = (boost::format("A username and password are being requested by %1%.") % url).str();

    AuthenticationConfirmationPtr c = std::make_shared<AuthenticationConfirmation>(self->m_tabId, url, message);

    self->m_confirmationAuthenticationMap[c] = request;

    self->cofirmationRequest(c);
#endif
}

void WebView::__requestCertificationConfirm(void * data , Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
#if PLATFORM(TIZEN)
    WebView * self = reinterpret_cast<WebView *>(data);

    Ewk_Certificate_Policy_Decision *request = reinterpret_cast<Ewk_Certificate_Policy_Decision *>(event_info);
    if (!request)
        return;

    // suspend webview
    ewk_view_suspend(self->m_ewkView);

    std::string url = self->getURI();

    ///\todo add translations
    std::string message = (boost::format("There are problems with the security certificate for this site.<br>%1%") % url).str();

    CertificateConfirmationPtr c = std::make_shared<CertificateConfirmation>(self->m_tabId, url, message);

    c->setResult(tizen_browser::basic_webengine::WebConfirmation::ConfirmationResult::Confirmed);

    // store
    self->m_confirmationCertificatenMap[c] = request;

    self->cofirmationRequest(c);
#endif
}
#endif

void WebView::setFocus()
{
    elm_object_focus_set(m_ewkView, EINA_TRUE);
}

void WebView::clearFocus()
{
    elm_object_focus_set(m_ewkView, EINA_FALSE);
}

bool WebView::hasFocus() const
{
    return elm_object_focus_get(m_ewkView) == EINA_TRUE ? true : false;
}

double WebView::getZoomFactor() const
{
    if(EINA_UNLIKELY(m_ewkView == NULL)){
        return 1.0;
    }

#if defined(USE_EWEBKIT)
    return ewk_view_page_zoom_get(m_ewkView);
#else
    return 1.0;
#endif
}

void WebView::setZoomFactor(double zoomFactor)
{
#if defined(USE_EWEBKIT)
    if(m_ewkView){
        //using zoomFactor = 0 sets zoom "fit to screen"

        ewk_view_page_zoom_set(m_ewkView, zoomFactor);
    }
#endif
}


const TabId& WebView::getTabId(){
    return m_tabId;
}


std::shared_ptr<tizen_browser::tools::BrowserImage> WebView::getFavicon() {
    BROWSER_LOGD("%s:%d, TabId: %s", __PRETTY_FUNCTION__, __LINE__, m_tabId.toString().c_str());
    M_ASSERT(m_ewkView);

#if defined(USE_EWEBKIT)
    if (faviconImage.get() == NULL) {

    if (m_ewkView)
    {
        Ewk_Context *context = ewk_view_context_get(m_ewkView);
        if (context)
        {
            Evas_Object * favicon = ewk_context_icon_database_icon_object_add(context, ewk_view_url_get(m_ewkView), evas_object_evas_get(m_ewkView));

#ifndef NDEBUG
                int w = 0, h = 0;
                evas_object_image_size_get(favicon, &w, &h);
                BROWSER_LOGD("[%s]: Info about favicon: w:%d h:%d, type: %s", __func__, w, h, evas_object_type_get(favicon));
#endif
                if (favicon) {
                    std::shared_ptr<tizen_browser::tools::BrowserImage>
                        image = tizen_browser::tools::EflTools::getBrowserImage(favicon);

                    evas_object_unref(favicon);

                    return image;
                }
            } else {
                return faviconImage;
            }
        }
    }
#endif

    BROWSER_LOGE("[%s:%d]: Returned favicon is empty!", __PRETTY_FUNCTION__, __LINE__);
    return std::make_shared<tizen_browser::tools::BrowserImage>();
}

void WebView::clearPrivateData()
{
    BROWSER_LOGD("Clearing private data");
#if defined(USE_EWEBKIT)
    if (m_ewkView)
    {
        Ewk_Context *context = ewk_view_context_get(m_ewkView);
        if (context)
        {
            ewk_context_web_storage_delete_all(context);
            ewk_cookie_manager_cookies_clear(ewk_context_cookie_manager_get(context));
        }
    }
#endif
}

void WebView::searchOnWebsite(const std::string & searchString, int flags)
{
    ///\todo: it should be "0" instead of "1024" for unlimited match count but it doesn't work properly in WebKit
    Eina_Bool result = ewk_view_text_find(m_ewkView, searchString.c_str(), static_cast<Ewk_Find_Options>(flags), 1024);
    BROWSER_LOGD("Ewk search; word: %s, result: %d", searchString.c_str(), result);
}

} /* namespace webkitengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */

