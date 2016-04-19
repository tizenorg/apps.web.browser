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
#include "browser_config.h"

#include <boost/any.hpp>
#include "BrowserLogger.h"
#include "Config.h"
#include <Ecore.h>
#include <Edje.h>
#include <Elementary.h>
#include <stdexcept>
#include <app.h>
#include <ewk_chromium.h>
#if PROFILE_MOBILE
#include <system_settings.h>
#include <app_common.h>
#endif

// for tests...
#include "Lifecycle.h"
#include "ServiceManager.h"
#include "BasicUI/AbstractMainWindow.h"

// Command line flags for engine
const char *engineCommandLineFlags[] = {
  "process-per-tab",
};

///\note Odroid platform modification
const std::string DEFAULT_URL = "";

static bool app_create(void * /*app_data*/)
{
    elm_config_accel_preference_set("opengl:depth24:stencil8");

    elm_config_focus_move_policy_set(ELM_FOCUS_MOVE_POLICY_CLICK);
    // Enabling focus
#if PROFILE_MOBILE
    elm_config_focus_highlight_enabled_set(EINA_FALSE);
#else
    elm_config_focus_highlight_enabled_set(EINA_TRUE);
#endif

    elm_config_cache_flush_enabled_set(boost::any_cast <Eina_Bool>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::CACHE_ENABLE_VALUE)));
    elm_config_cache_flush_interval_set(boost::any_cast <int>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::CACHE_INTERVAL_VALUE)));
    elm_config_cache_font_cache_size_set(boost::any_cast <int>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::CACHE_INTERVAL_VALUE)));
    elm_config_cache_image_cache_size_set(boost::any_cast <int>(tizen_browser::config::Config::getInstance().get(CONFIG_KEY::CACHE_IMAGE_VALUE)));

    for (auto flag: engineCommandLineFlags)
        ewk_chromium_append_command_line_flag(flag);

    /// \todo: clean casts, depends on ServiceManager
    std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
    std::dynamic_pointer_cast
    <
        tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));
    elm_app_base_scale_set(boost::any_cast<double>(tizen_browser::config::Config::getInstance().get("scale")));
    return true;
}

static void app_terminate(void */*app_data*/)
{
    BROWSER_LOGD("%s\n", __func__);
    std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
    std::dynamic_pointer_cast
    <
        tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));
    mainUi->destroyUI();
}

static void app_control(app_control_h app_control, void */*app_data*/){
    /* to test this functionality please use aul_test command on target:
     *  $aul_test org.tizen.browser __APP_SVC_URI__ <http://full.url.com/>
     */
    BROWSER_LOGD("%s\n", __func__);

    char *operation = NULL;
    char *request_uri = NULL;
    char *request_mime_type = NULL;

    if (app_control_get_operation(app_control, &operation) != APP_CONTROL_ERROR_NONE) {
        BROWSER_LOGD("get app_control operation failed");
        return;
    }

    if (app_control_get_uri(app_control, &request_uri) != APP_CONTROL_ERROR_NONE)
        BROWSER_LOGD("get app_control uri failed");

    if (app_control_get_mime(app_control, &request_mime_type) != APP_CONTROL_ERROR_NONE)
        BROWSER_LOGD("get app_control mime failed");


    BROWSER_LOGD("operation = [%s], request_uri = [%s], request_mime_type = [%s]"
            , operation, request_uri, request_mime_type);

    std::string uri = request_uri != NULL ? std::string(request_uri) : DEFAULT_URL;

    BROWSER_LOGD("[%s] uri=%s", __func__, uri.c_str());
    free(request_uri);
    free(request_mime_type);
    free(operation);

    std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
    std::dynamic_pointer_cast
    <
        tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));

    mainUi->exec(uri);

    evas_object_show(mainUi->getMainWindow().get());
    elm_win_activate(mainUi->getMainWindow().get());
}

static void app_pause(void *){
    BROWSER_LOGD("%s", __PRETTY_FUNCTION__);

    std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
    std::dynamic_pointer_cast
    <
        tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));
    mainUi->suspend();
}

static void app_resume(void *){
    BROWSER_LOGD("%s", __PRETTY_FUNCTION__);

    std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
    std::dynamic_pointer_cast
    <
        tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));
    mainUi->resume();
}

#if PROFILE_MOBILE
static void app_language_changed(app_event_info*, void *)
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   char *language;
   // Retrieve the current system language
   system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &language);
   if (language) {
        BROWSER_LOGD("[%s:%d] new lang: %s", __PRETTY_FUNCTION__, __LINE__, language);
        // Set the language in elementary
        elm_language_set(language);
        free(language);
   } else {
        BROWSER_LOGD("[%s:%d] Warning, failed to set new language!", __PRETTY_FUNCTION__, __LINE__);
   }
}
#endif

int main(int argc, char* argv[])try
{
    BEGIN()
    ewk_init();

//#if !defined(NDEBUG)
    //Initialization of logger module
    tizen_browser::logger::Logger::getInstance().init();
    tizen_browser::logger::Logger::getInstance().setLogTag("browser");
//#endif

	BROWSER_LOGD("BROWSER IS SAYING HELLO");
	BROWSER_LOGD("BROWSER TAG: %s",tizen_browser::logger::Logger::getInstance().getLogTag().c_str());
	BROWSER_LOGD("BROWSER REGISTERED LOGGERS COUNT: %d",tizen_browser::logger::Logger::getInstance().getLoggersCount());

    // TODO: Enable it after video black screen issue is resolved.
    // setenv("COREGL_FASTPATH", "1", 1);

    ui_app_lifecycle_callback_s ops;
    memset(&ops, 0x00, sizeof(ui_app_lifecycle_callback_s));

    ops.create = app_create;
    ops.terminate = app_terminate;
    ops.app_control = app_control;
    ops.pause = app_pause;
    ops.resume = app_resume;

#if PROFILE_MOBILE
    app_event_handler_h lang_changed_handler;
    ui_app_add_event_handler(&lang_changed_handler, APP_EVENT_LANGUAGE_CHANGED, app_language_changed, NULL);
#endif

    ui_app_main(argc, argv, &ops, NULL);

    ewk_shutdown();
    END()

} catch (std::exception & e)
{
    std::cerr << "UNHANDLED EXCEPTION " << e.what() << std::endl;
} catch (...)
{
    std::cerr << "UNHANDLED EXCEPTION" << std::endl;
}
