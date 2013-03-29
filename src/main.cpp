/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Ecore_X.h>
#include <Elementary.h>
#include <Evas.h>

#include <app.h>
#include <bundle.h>
#include <string>
#include <syspopup_caller.h>
#include <ui-gadget.h>
#include <vconf.h>

#include "browser.h"
#include "browser-object.h"
#include "browser-dlog.h"
#if defined(MDM)
#include "mdm-manager.h"
#endif
#include "webview-list.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#define BROWSER_PACKAGE_NAME	"browser"

static const char *support_mime_type[] = {"image/svg+xml", "text/html", "application/xml", NULL};

struct browser_data {
	Evas_Object *main_window;
	browser *browser_instance;
};

static void _set_env(void)
{
#if !defined(TIZEN_PUBLIC)
	/* manual enabling of CoreGL fastpath */
	setenv("COREGL_FASTPATH", "1", 1);
#endif
	setenv("CAIRO_GL_COMPOSITOR", "msaa", 1);
	setenv("CAIRO_GL_LAZY_FLUSHING", "yes", 1);

	/* set image cache suze */
	if (setenv("ELM_IMAGE_CACHE", "0", 1))
		BROWSER_LOGD("ELM_IMAGE_CACHE is set to 1MB");
}

static void _set_locale(void)
{
	char *r = setlocale(LC_ALL, "");
	/* if locale is not set properly, try again to set as language base */
	if (!r) {
	    char *lang_set = vconf_get_str(VCONFKEY_LANGSET);
	    if (lang_set && strlen(lang_set) > 0)
		r = setlocale(LC_ALL, lang_set);

	    BROWSER_LOGE("setlocale as [%s]", r);
	    if (lang_set)
		free(lang_set);
	}
	r = bindtextdomain(BROWSER_PACKAGE_NAME, browser_locale_dir);
	if (!r)
		BROWSER_LOGE("bindtextdomain failed");

	r = textdomain(BROWSER_PACKAGE_NAME);
	if (!r)
		BROWSER_LOGE("textdomain failed");
}

static void __br_destroy(void)
{
	BROWSER_LOGD("");
	elm_exit();
}

static void __main_window_del_cb(void *data, Evas_Object *obj, void *event)
{
	BROWSER_LOGD("");
	__br_destroy();
}

static void __profile_changed_cb(void *data, Evas_Object *obj, void *event)
{
	struct browser_data *ad = (struct browser_data *)data;
	if (ad->browser_instance->get_app_in_app_enable())
		return;

	const char *profile = elm_config_profile_get();

	if (!strcmp(profile, "desktop"))
		elm_win_indicator_mode_set (obj, ELM_WIN_INDICATOR_HIDE);
	else
		elm_win_indicator_mode_set (obj, ELM_WIN_INDICATOR_SHOW);
}

static Evas_Object *_create_main_window(void *app_data)
{
	BROWSER_LOGD("");
	Evas_Object *window = elm_win_add(NULL, BROWSER_PACKAGE_NAME, ELM_WIN_BASIC);
	if (window) {
		int width = 0;
		int height = 0;
		elm_win_title_set(window, BROWSER_PACKAGE_NAME);
		elm_win_borderless_set(window, EINA_TRUE);
		elm_win_conformant_set(window, EINA_TRUE);
		evas_object_smart_callback_add(window, "delete,request", __main_window_del_cb, app_data);
		evas_object_smart_callback_add(window, "profile,changed", __profile_changed_cb, app_data);
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &width, &height);
		evas_object_resize(window, width, height);
		elm_win_indicator_mode_set(window, ELM_WIN_INDICATOR_SHOW);
	}

	return window;
}

static void __br_rotate_cb(void *data, Evas_Object *obj, void *event)
{
	if (!data) {
		BROWSER_LOGE("data is NULL");
		return;
	}
	struct browser_data *ad = (struct browser_data *)data;
	int changed_ang = elm_win_rotation_get(ad->main_window);
	BROWSER_LOGD("changed_ang[%d]", changed_ang);

	if (changed_ang == -1)
		return;
	else
		ad->browser_instance->rotate(changed_ang);
}

static void _exit_browser(void)
{
	BROWSER_LOGD("");
	bundle *b = bundle_create();
	if (!b)
		return;

	bundle_add(b, "_SYSPOPUP_TITLE_", "Warning");
	bundle_add(b, "_SYSPOPUP_CONTENT_", "Low memory, Can't launch browser. Kill other applications");
	syspopup_launch("syspopup-app", b);
	bundle_free(b);

	elm_exit();
}

static bool __br_app_create(void *app_data)
{
	BROWSER_LOGD("");

	struct browser_data *ad = (struct browser_data *)app_data;

	int low_memory = 0;
	vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &low_memory);
	if (low_memory > VCONFKEY_SYSMAN_LOW_MEMORY_SOFT_WARNING)
		_exit_browser();

	elm_config_preferred_engine_set("opengl_x11");

	_set_locale();

	ad->main_window = _create_main_window(app_data);

	ad->browser_instance = new browser(ad->main_window);

	if (elm_win_wm_rotation_supported_get(ad->main_window))
	{
		const int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->main_window, rots, 4);
	}
	evas_object_smart_callback_add(ad->main_window, "wm,rotation,changed", __br_rotate_cb, ad);

	return true;
}

static void __br_app_pause(void *app_data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)app_data;
	ad->browser_instance->pause();
	ug_pause();
}

static void __br_app_resume(void *app_data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)app_data;
#if defined(MDM)
	ad->browser_instance->get_mdm_manager()->check_mdm_policy();
#endif
	ad->browser_instance->resume();
	ug_resume();
}

static Eina_Bool _supported_mime_type(const char *mime_type)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(mime_type, EINA_FALSE);

	int index = 0;
	while (support_mime_type[index]) {
		if (!strcmp(support_mime_type[index], mime_type))
			return EINA_TRUE;
		index++;
	}

	return EINA_FALSE;
}

#if defined(WEBCLIP)
static char *_get_uri_from_webclip_content_info(const char *content_info_str)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(content_info_str, NULL);
	// "x=149&y=165&scale=2.000000&devicewidth=720&url=http://m....com/ "
	int len = strlen(content_info_str);
	char *buffer;

	buffer = (char *)malloc(len + 1);
	if (!buffer)
		return NULL;
	memset(buffer, 0x00, sizeof(char) * (len + 1));
	strcpy(buffer, content_info_str);

	char *label = NULL;
	char *value = NULL;

	/* Get first label data : x*/
	label = strtok(buffer, "=");
	value = strtok(NULL, "&");
	BROWSER_LOGD("label =%s, value = %s\n", label, value);

	/* run skipping y , scale, devicewidth*/
	for (int i = 0 ; i < 3 ; i++) {
		label = strtok(NULL, "=");
		value = strtok(NULL, "&");
		BROWSER_LOGD("label =%s, value = %s\n", label, value);
	}

	/* url */
	label = strtok(NULL, "=");
	value = strtok(NULL, "\0");
	if (!value) {
		free(buffer);
		return NULL;
	}
	char *url = strdup(value);
	BROWSER_LOGD("label =%s, value = %s\n", label, value);

	free(buffer);

	return url;
}
#endif

static void __br_app_reset(service_h service, void *app_data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)app_data;

#if defined(MDM)
	ad->browser_instance->get_mdm_manager()->check_mdm_policy();
#endif

	char *operation = NULL;
	char *request_uri = NULL;
	char *request_mime_type = NULL;

	if (service_get_operation(service, &operation) != SERVICE_ERROR_NONE) {
		BROWSER_LOGD("get service operation failed");
		return;
	}

	if (service_get_uri(service, &request_uri) != SERVICE_ERROR_NONE)
		BROWSER_LOGD("get service uri failed");

	if (service_get_mime(service, &request_mime_type) != SERVICE_ERROR_NONE)
		BROWSER_LOGD("get service mime failed");

	BROWSER_LOGD("operation = [%s], request_uri = [%s], request_mime_type = [%s]", operation, request_uri, request_mime_type);

#if defined(WEBCLIP)
	char *webclip_content_info = NULL;
	if (service_get_extra_data(service, "webclip", &webclip_content_info) != SERVICE_ERROR_NONE)
		BROWSER_LOGD("get service webclip uri failed");

	if (webclip_content_info && !request_uri)
		request_uri = _get_uri_from_webclip_content_info(webclip_content_info);

	BROWSER_LOGD("request_uri : %s ", request_uri);
	if (webclip_content_info)
		free(webclip_content_info);
#endif

	if (_supported_mime_type(request_mime_type))
		ad->browser_instance->launch(request_uri);
	else if (request_uri) {
		std::string uri = std::string(request_uri);
		if (!strncmp(request_uri, "/opt/", strlen("/opt/")))
			uri = std::string("file://") + uri;

		ad->browser_instance->launch(uri.c_str());
	} else
		ad->browser_instance->launch(request_uri);

	if (request_uri)
		free(request_uri);
	if (request_mime_type)
		free(request_mime_type);
	if (operation)
		free(operation);

	char *miniapp = NULL;
	if (service_get_extra_data(service, "http://tizen.org/appcontrol/data/miniapp", &miniapp) != SERVICE_ERROR_NONE)
		BROWSER_LOGD("http://tizen.org/appcontrol/data/miniapp failed");

	BROWSER_LOGD("miniapp=[%s]", miniapp);
	if (miniapp && !strcmp(miniapp, "on")) {
		ad->browser_instance->set_app_in_app_enable(EINA_TRUE);
	}

	evas_object_show(ad->main_window);
	elm_win_activate(ad->main_window);
}

static void __br_app_terminate(void *app_data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)app_data;

	evas_object_del(ad->main_window);

	delete ad->browser_instance;
}

static void __br_low_memory_cb(void* data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)data;
	ad->browser_instance->get_webview_list()->clean_up_webviews();
}

static void __br_low_battery_cb(void* data)
{
	BROWSER_LOGD("");
	return;
}

static void __br_lang_changed_cb(void *data)
{
	BROWSER_LOGD("");
	return;
}

static void __br_region_changed_cb(void *data)
{
	BROWSER_LOGD("");
	return;
}

EXPORT_API int main(int argc, char *argv[])
{
	_set_env();

	app_event_callback_s ops;
	memset(&ops, 0x00, sizeof(app_event_callback_s));

	ops.create = __br_app_create;
	ops.terminate = __br_app_terminate;
	ops.pause = __br_app_pause;
	ops.resume = __br_app_resume;
	ops.service = __br_app_reset;
	ops.low_memory = __br_low_memory_cb;
	ops.low_battery = __br_low_battery_cb;
	ops.device_orientation = NULL;
	ops.language_changed = __br_lang_changed_cb;
	ops.region_format_changed = __br_region_changed_cb;

	struct browser_data ad;
	memset(&ad, 0x00, sizeof(struct browser_data));

	int ret = app_efl_main(&argc, &argv, &ops, &ad);

	return ret;
}

