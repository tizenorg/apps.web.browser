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

#ifndef WEBVIEW_H_
#define WEBVIEW_H_

#include <boost/signals2/signal.hpp>
#include <string>
#include <Evas.h>

#if defined(USE_EWEBKIT)
//#include <EWebKit2.h>
#include <ewk_chromium.h>
#endif

#include "browser_config.h"
#include "Config/Config.h"
#include "BrowserImage.h"
#include "AbstractWebEngine/TabId.h"
#include "AbstractWebEngine/WebConfirmation.h"

namespace tizen_browser {
namespace basic_webengine {
namespace webkitengine_service {

class WebView
{
public:
    WebView(Evas_Object *, TabId);
    virtual ~WebView();
    void init(bool desktopView = true, Evas_Object * opener = NULL);


    void setURI(const std::string &);
    std::string getURI(void);

    std::string getTitle(void);

    void stopLoading(void);
    void reload(void);

    void back(void);
    void forward(void);

    bool isBackEnabled(void);
    bool isForwardEnabled(void);

    bool isLoading();
    bool isLoadError() const;

    Evas_Object * getLayout();

    void confirmationResult(WebConfirmationPtr);

    void setPrivateMode(bool);

    std::shared_ptr<tizen_browser::tools::BrowserImage> captureSnapshot(int width, int height);
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

    /**
     * @brief get current real zoom factor from webkit
     */
    double getZoomFactor() const;

    /**
     * @brief set zoom factor of website
     */
    void setZoomFactor(double zoomFactor);


    void clearPrivateData();

    /**
     * @return tab id
     */
    const TabId& getTabId();

    /**
     * @brief Search string on searchOnWebsite
     *
     * @param string to search on searchOnWebsite
     * @param flags for search options
     */
    void searchOnWebsite(const std::string &, int);

    /**
     * @brief Change user agent to desktop type
     */
    void switchToDesktopView();

    /**
     * @brief Change user agent to mobile type
     */
    void switchToMobileView();

    /**
     * @brief Get favicon of URL
     */
    std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon();

// signals
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::tools::BrowserImage>)> favIconChanged;
    boost::signals2::signal<void (const std::string &)> titleChanged;
    boost::signals2::signal<void (const std::string &)> uriChanged;
    boost::signals2::signal<void (TabId)> tabIdChecker;

    boost::signals2::signal<void ()> loadFinished;
    boost::signals2::signal<void ()> loadStarted;
    boost::signals2::signal<void ()> loadStop;
    boost::signals2::signal<void ()> loadError;
    boost::signals2::signal<void (double)> loadProgress;

    boost::signals2::signal<void (bool)> forwardEnableChanged;
    boost::signals2::signal<void (bool)> backwardEnableChanged;

    boost::signals2::signal<void (WebConfirmationPtr)> cofirmationRequest;

    boost::signals2::signal<void ()> ewkViewClicked;

    boost::signals2::signal<void (bool)> IMEStateChanged;

private:
    void registerCallbacks();
    void unregisterCallbacks();
    void setupEwkSettings();

#if defined(USE_EWEBKIT)
    static std::string securityOriginToUri(const Ewk_Security_Origin *);
    static void __setFocusToEwkView(void * data, Evas * e, Evas_Object * obj, void * event_info);
    static void __newWindowRequest(void * data, Evas_Object *, void *out);
    static void __closeWindowRequest(void * data, Evas_Object *, void *);
#endif

    // Load
    static void __loadStarted(void * data, Evas_Object * obj, void * event_info);
    static void __loadStop(void * data, Evas_Object * obj, void * event_info);
    static void __loadFinished(void * data, Evas_Object * obj, void * event_info);
    static void __loadProgress(void * data, Evas_Object * obj, void * event_info);
    static void __loadError(void* data, Evas_Object* obj, void *ewkError);

    static void __titleChanged(void * data, Evas_Object * obj, void * event_info);
    static void __urlChanged(void * data, Evas_Object * obj, void * event_info);

    static void __backForwardListChanged(void * data, Evas_Object * obj, void * event_info);

    // Favicon - from database
    static void __faviconChanged(void* data, Evas_Object*, void*);

    static void __IMEClosed(void * data, Evas_Object *obj, void *event_info);
    static void __IMEOpened(void * data, Evas_Object *obj, void *event_info);

    // confirmation requests
    static void __geolocationPermissionRequest(void * data, Evas_Object * obj, void * event_info);
    static void __usermediaPermissionRequest(void * data, Evas_Object * obj, void * event_info);
    static void __notificationPermissionRequest(void * data, Evas_Object * obj, void * event_info);
    static void __authenticationRequest(void * data, Evas_Object * obj, void * event_info);
    static void __requestCertificationConfirm(void * data, Evas_Object * obj, void * event_info);

    static void scriptLinkSearchCallback(Evas_Object *o, const char *value, void *data);
private:
    Evas_Object * m_parent;
    TabId m_tabId;
    Evas_Object * m_ewkView;
    // ewk context of this web view
    Ewk_Context * m_ewkContext;
    std::string m_title;
    std::shared_ptr<tizen_browser::tools::BrowserImage> faviconImage;
    bool m_isLoading;
    double m_loadProgress;
    bool m_loadError;



    config::DefaultConfig config;

#if defined(USE_EWEBKIT)
#if PLATFORM(TIZEN)
    std::map<WebConfirmationPtr, Ewk_Geolocation_Permission_Request *> m_confirmationGeolocationMap;
    std::map<WebConfirmationPtr, Ewk_User_Media_Permission_Request *> m_confirmationUserMediaMap;
    std::map<WebConfirmationPtr, Ewk_Notification_Permission_Request *> m_confirmationNotificationMap;
    std::map<CertificateConfirmationPtr, Ewk_Certificate_Policy_Decision *> m_confirmationCertificatenMap;
    std::map<AuthenticationConfirmationPtr, Ewk_Auth_Request *> m_confirmationAuthenticationMap;
#endif
#endif
};

} /* namespace webkitengine_service */
} /* end of basic_webengine */
} /* end of tizen_browser */
#endif /* WEBVIEW_H_ */
