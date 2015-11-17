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
#include "app_common.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"
#include "EflTools.h"
#include "GeneralTools.h"
#include "Tools/WorkQueue.h"
#include "ServiceManager.h"

#define certificate_crt_path CERTS_DIR
#if MERGE_ME
#define APPLICATION_NAME_FOR_USER_AGENT "SamsungBrowser/1.0"
#else
#define APPLICATION_NAME_FOR_USER_AGENT "Mozilla/5.0 (X11; SMART-TV; Linux) AppleWebkit/538.1 (KHTML, like Gecko) Safari/538.1"
#endif

//TODO: temporary user agent for mobile display, change to proper one
#define APPLICATION_NAME_FOR_USER_AGENT_MOBILE "Mozilla/5.0 (Linux; Tizen 3.0; SAMSUNG SM-Z130H) AppleWebKit/538.1 (KHTML, like Gecko) SamsungBrowser/1.0 Mobile Safari/538.1"

using namespace tizen_browser::tools;

namespace tizen_browser {
namespace basic_webengine {
namespace webkitengine_service {

const std::string WebView::COOKIES_PATH = "cookies";

WebView::WebView(Evas_Object * obj, TabId tabId, const std::string& title, bool incognitoMode)
    : m_parent(obj)
    , m_tabId(tabId)
    , m_ewkView(nullptr)
    , m_ewkContext(nullptr)
    , m_title(title)
    , m_isLoading(false)
    , m_loadError(false)
    , m_suspended(false)
    , m_private(incognitoMode)
{
    config.load("whatever");
}

WebView::~WebView()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    if (m_ewkView) {
        unregisterCallbacks();
    }

    ewk_context_unref(m_ewkContext);
}

void WebView::init(bool desktopMode, Evas_Object*)
{
#if defined(USE_EWEBKIT)
    m_ewkView = m_private ? ewk_view_add_in_incognito_mode(evas_object_evas_get(m_parent)) :
                            ewk_view_add_with_context(evas_object_evas_get(m_parent), ewk_context_new());

    m_ewkContext = ewk_view_context_get(m_ewkView);
    if (m_ewkContext)
        m_private ? ewk_cookie_manager_accept_policy_set(ewk_context_cookie_manager_get(m_ewkContext), EWK_COOKIE_ACCEPT_POLICY_NEVER) :
                    ewk_cookie_manager_accept_policy_set(ewk_context_cookie_manager_get(m_ewkContext), EWK_COOKIE_ACCEPT_POLICY_ALWAYS);

    evas_object_data_set(m_ewkView, "_container", this);
    BROWSER_LOGD("%s:%d %s self=%p", __FILE__, __LINE__, __func__, this);

    evas_object_color_set(m_ewkView, 255, 255, 255, 255);
    evas_object_size_hint_weight_set(m_ewkView, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_ewkView, EVAS_HINT_FILL, EVAS_HINT_FILL);
    if (desktopMode) {
        switchToDesktopMode();
    } else {
        switchToMobileMode();
    }
    //\todo: when value is other than 1.0, scroller is located improperly
//    ewk_view_device_pixel_ratio_set(m_ewkView, 1.0f);

#if PLATFORM(TIZEN)
    ewk_view_resume(m_ewkView);
#endif

    ewk_context_cache_model_set(m_ewkContext, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);
    std::string path = app_get_data_path() + COOKIES_PATH;
    ewk_cookie_manager_persistent_storage_set(ewk_context_cookie_manager_get(m_ewkContext),  path.c_str(), EWK_COOKIE_PERSISTENT_STORAGE_SQLITE);

    setupEwkSettings();
    registerCallbacks();
    resume();
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
    evas_object_smart_callback_add(m_ewkView, "authentication,challenge", __authenticationChallenge, this);
    evas_object_smart_callback_add(m_ewkView, "request,certificate,confirm", __requestCertificationConfirm, this);

    evas_object_event_callback_add(m_ewkView, EVAS_CALLBACK_MOUSE_DOWN, __setFocusToEwkView, this);
    evas_object_smart_callback_add(m_ewkView, "icon,received", __faviconChanged, this);

    evas_object_smart_callback_add(m_ewkView, "editorclient,ime,closed", __IMEClosed, this);
    evas_object_smart_callback_add(m_ewkView, "editorclient,ime,opened", __IMEOpened, this);

#if PROFILE_MOBILE
    evas_object_smart_callback_add(m_ewkView, "contextmenu,customize", __contextmenu_customize_cb, this);
#endif
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
    evas_object_smart_callback_del_full(m_ewkView, "authentication,challenge", __authenticationChallenge, this);
    evas_object_smart_callback_del_full(m_ewkView, "request,certificate,confirm", __requestCertificationConfirm, this);

    evas_object_event_callback_del(m_ewkView, EVAS_CALLBACK_MOUSE_DOWN, __setFocusToEwkView);
    evas_object_smart_callback_del_full(m_ewkView, "icon,received", __faviconChanged, this);

    evas_object_smart_callback_del_full(m_ewkView, "editorclient,ime,closed", __IMEClosed, this);
    evas_object_smart_callback_del_full(m_ewkView, "editorclient,ime,opened", __IMEOpened, this);

#if PROFILE_MOBILE
    evas_object_smart_callback_del_full(m_ewkView, "contextmenu,customize", __contextmenu_customize_cb,this);
#endif
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
    return fromChar(ewk_view_url_get(m_ewkView));
#else
    return std::string();
#endif
}

std::string WebView::getTitle(void)
{
    return m_title;
}

void WebView::suspend()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    M_ASSERT(m_ewkView);

    ewk_view_suspend(m_ewkView);
    m_suspended = true;
}

void WebView::resume()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    M_ASSERT(m_ewkView);

    ewk_view_resume(m_ewkView);
    m_suspended = false;
}

void WebView::stopLoading(void)
{
#if defined(USE_EWEBKIT)
    m_isLoading = false;
    ewk_view_stop(m_ewkView);
#endif
    loadStop();
}

void WebView::reload(void)
{
#if defined(USE_EWEBKIT)
    m_isLoading = true;
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
        //FIXME: https://bugs.tizen.org/jira/browse/TT-229
        CertificateConfirmationPtr cert = std::dynamic_pointer_cast<CertificateConfirmation, WebConfirmation>(confirmation);

        // The below line doesn't serve any purpose now, but it may become
        // relevant when implementing https://bugs.tizen.org/jira/browse/TT-229
        // Ewk_Certificate_Policy_Decision *request = m_confirmationCertificatenMap[cert];

        if (cert->getResult() == WebConfirmation::ConfirmationResult::Confirmed)
            //FIXME: do something
            BROWSER_LOGE("NOT IMPLEMENTED: Certificate Confirmation handling!");
        else if (cert->getResult() == WebConfirmation::ConfirmationResult::Rejected)
            //FIXME: do something else
            BROWSER_LOGE("NOT IMPLEMENTED: Certificate Confirmation handling!");
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
        Ewk_Auth_Challenge *request = m_confirmationAuthenticationMap[auth];

        if (auth->getResult() == WebConfirmation::ConfirmationResult::Confirmed) {
            ewk_auth_challenge_credential_use(request, auth->getLogin().c_str(), auth->getPassword().c_str());
        } else if (auth->getResult() == WebConfirmation::ConfirmationResult::Rejected) {
            ewk_auth_challenge_credential_cancel(request);
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

std::shared_ptr<BrowserImage> WebView::captureSnapshot(int targetWidth, int targetHeight)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    M_ASSERT(m_ewkView);
    M_ASSERT(targetWidth);
    M_ASSERT(targetHeight);
    Evas_Coord vw, vh;
    std::shared_ptr<BrowserImage> noImage = std::make_shared<BrowserImage>();
    evas_object_geometry_get(m_ewkView, nullptr, nullptr, &vw, &vh);
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


    BROWSER_LOGD("[%s:%d] Before snapshot (screenshot) - look at the time of taking snapshot below",__func__, __LINE__);
#if defined(USE_EWEBKIT)
#if PLATFORM(TIZEN)
    Evas_Object *snapshot = ewk_view_screenshot_contents_get( m_ewkView, area, 1.0, evas_object_evas_get(m_ewkView));
    BROWSER_LOGD("[%s:%d] Snapshot (screenshot) catched, evas pointer: %p",__func__, __LINE__, snapshot);
    if (snapshot)
        return EflTools::getBrowserImage(snapshot);
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
    TabId id;
    if (m_webEngine->currentTabId() != (id = m_webEngine->addTab(std::string(), &self->getTabId()))) {
        BROWSER_LOGD("Created tab: %s", id.toString().c_str());
        Evas_Object* tab_ewk_view = m_webEngine->getTabView(id);
        *static_cast<Evas_Object**>(out) = tab_ewk_view;
    }

    // switch to a new tab
    m_webEngine->switchToTab(id);
    m_webEngine->windowCreated();
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
}

void WebView::__loadProgress(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    WebView * self = reinterpret_cast<WebView *>(data);
    if (!self->isLoading())
        return;
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
        evas_object_smart_callback_call(obj, "load,stop", nullptr);
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
    self->m_title = fromChar(ewk_view_title_get(obj));
    self->titleChanged(self->m_title, self->getTabId().toString());
}

void WebView::__urlChanged(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    WebView * self = reinterpret_cast<WebView *>(data);
    BROWSER_LOGD("URL changed for tab: %s", self->getTabId().toString().c_str());
    self->uriChanged(self->getURI());
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
        if (favicon && self->isLoading()) {
            BROWSER_LOGD("[%s:%d] Favicon received", __PRETTY_FUNCTION__, __LINE__);
            self->faviconImage = EflTools::getBrowserImage(favicon);
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
    std::string protocol = fromChar(ewk_security_origin_protocol_get(origin));
    std::string uri = fromChar(ewk_security_origin_host_get(origin));
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

    self->confirmationRequest(c);
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

    self->confirmationRequest(c);
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

    self->confirmationRequest(c);
#endif
}

void WebView::__authenticationChallenge(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
#if PLATFORM(TIZEN)
    WebView * self = reinterpret_cast<WebView *>(data);

    Ewk_Auth_Challenge *request = reinterpret_cast<Ewk_Auth_Challenge *>(event_info);
    EINA_SAFETY_ON_NULL_RETURN(request);

    const char* realm = ewk_auth_challenge_realm_get(request);
    const char* auth_url = ewk_auth_challenge_url_get(request);
    if (!realm || !auth_url)
        BROWSER_LOGE("realm or url NULL");
    ewk_auth_challenge_suspend(request);

    std::string url = self->getURI();
    std::string message = (boost::format("A username and password are being requested by %1%.") % url).str();

    AuthenticationConfirmationPtr c = std::make_shared<AuthenticationConfirmation>(self->m_tabId, url, message);

    self->m_confirmationAuthenticationMap[c] = request;

    self->confirmationRequest(c);
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

    self->confirmationRequest(c);
#endif
}
#endif

#if PROFILE_MOBILE
context_menu_type WebView::_get_menu_type(Ewk_Context_Menu *menu)
{
    int count = ewk_context_menu_item_count(menu);
    bool text = false;
    bool link = false;
    bool image = false;
    bool email_address = false;
    bool call_number = false;
    bool selection_mode = false;
    for (int i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, i);
        Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
        BROWSER_LOGD("tag=%d", tag);
        if (tag == EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE)
            selection_mode = EINA_TRUE;
        if (tag == EWK_CONTEXT_MENU_ITEM_TAG_CLIPBOARD)
            return INPUT_FIELD;
        if (tag == EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB)
            text = EINA_TRUE;
        if (tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW || tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD)
            link = EINA_TRUE;
        if (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD)
            image = EINA_TRUE;
    }

    if (email_address && selection_mode)
	return EMAIL_SCHEME;
    if (call_number && selection_mode)
	return TEL_SCHEME;
    if (text && !link)
	return TEXT_ONLY;
    if (link && !image)
	return TEXT_LINK;
    if (image && !link)
	return IMAGE_ONLY;
    if(selection_mode && image && link)
	return TEXT_IMAGE_LINK;
    if (image && link)
	return IMAGE_LINK;

    return UNKNOWN_MENU;
}

void WebView::_show_context_menu_text_link(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("");

    int count = ewk_context_menu_item_count(menu);
    Ewk_Context_Menu_Item_Tag originalCtxItem[count];

    for (int i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item * item = ewk_context_menu_nth_item_get(menu, 0);
        Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

        originalCtxItem[i] = tag;
        ewk_context_menu_item_remove(menu, item);
    }

    /* Open */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, "Open", true);
        break;
        }
    }
    /* Open in new window */
    for (int i = 0 ; i < count ; i++) {
	if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW) {
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, "Open in new tab", true);
		break;
	}
    }
    /* Copy link text */
    for (int i = 0 ; i < count ; i++) {
	if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD) {
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, "Copy link text", true);
		break;
	}
    }
    /* Copy link address */
    for (int i = 0 ; i < count ; i++) {
	if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD) {
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, "Copy link address", true);
		break;
	}
    }
    /* Share Link */
    for (int i = 0 ; i < count ; i++) {
	if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_SMART_LINKS) {
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_SMART_LINKS, "Share", true);
		break;
	}
    }
    /* Download link */
    for (int i = 0 ; i < count ; i++) {
	if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK) {
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK, "Download Link", true);
		break;
	}
    }
}

void WebView::_show_context_menu_text_only(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("");

    int count = ewk_context_menu_item_count(menu);
    Ewk_Context_Menu_Item_Tag originalCtxItem[count];

    const char *selected_text = ewk_view_text_selection_text_get(m_ewkView);
    Eina_Bool text_selected = EINA_FALSE;
    if (selected_text && strlen(selected_text) > 0)
        text_selected = EINA_TRUE;

    for (int  i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
        Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

        originalCtxItem[i] = tag;
        ewk_context_menu_item_remove(menu, item);
    }
    /* Select all */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL, "Select all", true);
            break;
	}
    }

    /* Copy */
    if (text_selected == EINA_TRUE) {
        for (int i = 0 ; i < count ; i++) {
            if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY) {
                ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY, "Copy", true);
                break;
            }
        }
    }

/* Share*/
    if (text_selected == EINA_TRUE) {
        for (int i = 0 ; i < count ; i++) {
            if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_SMART_LINKS) {
                ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_SMART_LINKS, "Share", true);
                break;
            }
        }
    }
}

void WebView::_show_context_menu_image_only(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("");

    int count = ewk_context_menu_item_count(menu);
    Ewk_Context_Menu_Item_Tag originalCtxItem[count];

    for (int i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
        Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
        originalCtxItem[i] = tag;
        ewk_context_menu_item_remove(menu, item);
    }

    /* Open in current Tab */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW, "Open", true);
            break;
        }
    }
    /* Open in New Tab */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW, "Open in new tab", true);
            break;
        }
    }
    /* Copy image */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD, "Copy image", true);
            break;
        }
    }
    /* Save image */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK, "Save image", true);
            break;
        }
    }
}

void WebView::_show_context_menu_image_link(Ewk_Context_Menu *menu)
{
    BROWSER_LOGD("");

    int count = ewk_context_menu_item_count(menu);
   Ewk_Context_Menu_Item_Tag originalCtxItem[count];

    for (int i = 0 ; i < count ; i++) {
        Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
        Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

        originalCtxItem[i] = tag;
        ewk_context_menu_item_remove(menu, item);
    }

    /* Open in current window */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, "Open", true);
            break;
        }
    }
    /* Open in new window */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, "Open in new tab", true);
            break;
        }
    }
    /* Save link */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK, "Save link", true);
            break;
        }
    }
    /* Copy link */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, "Copy link", true);
            break;
        }
    }
    /* Save image */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK, "Save image", true);
	    break;
        }
    }
    /* copy image */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD, "Copy image", true);
            break;
        }
    }
    /* View image */
    for (int i = 0 ; i < count ; i++) {
        if (originalCtxItem[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW) {
            ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW, "View image", true);
             break;
        }
    }
}

void WebView::_customize_context_menu(Ewk_Context_Menu *menu)
{
    context_menu_type menu_type = _get_menu_type(menu);
    BROWSER_LOGD("menu_type=%d", menu_type);

    if (menu_type == UNKNOWN_MENU || menu_type == INPUT_FIELD)
        return;

    switch (menu_type) {
        case TEXT_ONLY:
            _show_context_menu_text_only(menu);
        break;

        case TEXT_LINK:
            _show_context_menu_text_link(menu);
        break;

        case IMAGE_ONLY:
            _show_context_menu_image_only(menu);
        break;

        case IMAGE_LINK:
            _show_context_menu_image_link(menu);
        break;

        default:
        return;
        break;
    }
}

void WebView::__contextmenu_customize_cb(void *data, Evas_Object * /* obj */, void *event_info)
{
    BROWSER_LOGD("");

    Ewk_Context_Menu *menu = reinterpret_cast<Ewk_Context_Menu*>(event_info);
    WebView * self = reinterpret_cast<WebView *>(data);

    self->_customize_context_menu(menu);
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
    if(EINA_UNLIKELY(m_ewkView == nullptr)) {
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
    if(m_ewkView) {
        //using zoomFactor = 0 sets zoom "fit to screen"

        if(zoomFactor != getZoomFactor()) 
            ewk_view_page_zoom_set(m_ewkView, zoomFactor);
    }
#endif
}

void WebView::scrollView(const int& dx, const int& dy)
{
    ewk_view_scroll_by(m_ewkView, dx, dy);
}

#if PROFILE_MOBILE
void WebView::setTouchEvents(bool enabled) {
    ewk_view_touch_events_enabled_set(m_ewkView, enabled);
}
#endif

const TabId& WebView::getTabId() {
    return m_tabId;
}


std::shared_ptr<BrowserImage> WebView::getFavicon()
{
    BROWSER_LOGD("%s:%d, TabId: %s", __PRETTY_FUNCTION__, __LINE__, m_tabId.toString().c_str());

    Evas_Object * favicon = ewk_context_icon_database_icon_object_add(ewk_view_context_get(m_ewkView), ewk_view_url_get(m_ewkView),evas_object_evas_get(m_ewkView));
    faviconImage = EflTools::getBrowserImage(favicon);
    evas_object_unref(favicon);

    if(faviconImage.get())
        return faviconImage;

    BROWSER_LOGD("[%s:%d] Returned favicon is empty!",  __PRETTY_FUNCTION__, __LINE__);
    return std::make_shared<BrowserImage>();
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

void WebView::switchToDesktopMode() {
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    ewk_view_user_agent_set(m_ewkView, APPLICATION_NAME_FOR_USER_AGENT);
    m_desktopMode = true;
}

void WebView::switchToMobileMode() {
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    ewk_view_user_agent_set(m_ewkView, APPLICATION_NAME_FOR_USER_AGENT_MOBILE);
    m_desktopMode = false;
}

bool WebView::isDesktopMode() const {
    return m_desktopMode;
}

} /* namespace webkitengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */

