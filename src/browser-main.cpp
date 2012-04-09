/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

extern "C" {
#include <ITapiSat.h>
}

#include <sys/utsname.h>

#include "browser-class.h"
#include "browser-config.h"

using namespace std;

struct browser_data {
	Evas_Object *main_win;
	Evas_Object *bg;
	Evas_Object *main_layout;
	Evas_Object *navi_bar;
	Elm_Theme *browser_theme;

	Browser_Class *browser_instance;
};

static void __br_set_env(void)
{
	/* enable gl */
	if (!getenv("ELM_ENGINE")) {
		if (setenv("ELM_ENGINE", "gl", 1))
			BROWSER_LOGD("ELM_ENGINE's value is overwrited");
	}

	/* set image cache suze */
	if (setenv("ELM_IMAGE_CACHE", "0", 1))
		BROWSER_LOGD("ELM_IMAGE_CACHE is set to 1MB");

	/* This is because of showing navigation bar more quickly. */
	if (setenv("ELM_BRING_IN_SCROLL_FRICTION", "0.2", 1))
		BROWSER_LOGD("ELM_BRING_IN_SCROLL_FRICTION is set");

	if (setenv("ELM_PAGE_SCROLL_FRICTION", "0.4", 1))
		BROWSER_LOGD("ELM_PAGE_SCROLL_FRICTION is set");

	if (setenv("ELM_THUMBSCROLL_BOUNCE_FRICTION", "0.2", 1))
		BROWSER_LOGD("ELM_THUMBSCROLL_BOUNCE_FRICTION is set");
}

static void __br_destroy(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	elm_exit();
}

static void __main_win_del_cb(void *data, Evas_Object *obj, void *event)
{
	BROWSER_LOGD("<< window delete callback [%d]>>", appcore_measure_time());
}

static Evas_Object *__create_main_win(void *app_data)
{
	Evas_Object *window = elm_win_add(NULL, BROWSER_PACKAGE_NAME, ELM_WIN_BASIC);
	if (window) {
		int w;
		int h;
		elm_win_title_set(window, BROWSER_PACKAGE_NAME);
		elm_win_borderless_set(window, EINA_TRUE);
		elm_win_conformant_set(window, EINA_TRUE);
		evas_object_smart_callback_add(window, "delete,request",
						__main_win_del_cb, app_data);
		ecore_x_window_size_get(ecore_x_window_root_first_get(),
					&w, &h);
		evas_object_resize(window, w, h);
		elm_win_indicator_mode_set(window, ELM_WIN_INDICATOR_SHOW);
		evas_object_show(window);
	}

	return window;
}

static Evas_Object *__create_bg(Evas_Object *win)
{
	Evas_Object *bg;
	bg = elm_bg_add(win);
	if (!bg)
		return NULL;

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg);

	return bg;
}

static Evas_Object *__create_main_layout(Evas_Object *win)
{
	Evas_Object *layout;
	layout = elm_layout_add(win);
	if (!layout)
		return NULL;

	if (!elm_layout_theme_set(layout, "layout", "application", "default"))
		BROWSER_LOGE("elm_layout_theme_set is failed.\n");

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, layout);
	edje_object_signal_emit(elm_layout_edje_get(layout), "elm,state,show,indicator", "elm");
	evas_object_show(layout);

	return layout;
}

static Evas_Object *__create_navi_bar(void *app_data)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *navi_bar;
	struct browser_data *ad = (struct browser_data *)app_data;
	navi_bar = elm_naviframe_add(ad->main_layout);
	if (navi_bar) {
		elm_object_theme_set(navi_bar, ad->browser_theme);
		elm_object_style_set(navi_bar, "browser");
		elm_object_part_content_set(ad->main_layout, "elm.swallow.content", navi_bar);
		evas_object_show(navi_bar);
	}

	return navi_bar;
}

static Eina_Bool __process_app_service(bundle *b, void *app_data)
{
	BROWSER_LOGD("[%s]", __func__);
	struct browser_data *ad = (struct browser_data *)app_data;

	const char *operation = appsvc_get_operation(b);
	const char *request_uri = appsvc_get_uri(b);
	const char *request_mime_type = appsvc_get_mime(b);

	if (!operation && !request_uri && !request_mime_type) {
		BROWSER_LOGD("Not app svc");
		return EINA_FALSE;
	}

	BROWSER_LOGD("operation[%s], request_uri[%s], request_mime_type[%s]", operation, request_uri, request_mime_type);

	std::string full_path;

	if (request_mime_type) {
		if (!strncmp(request_mime_type, "http.uri", strlen("http.uri"))
		    || !strncmp(request_mime_type, "file.uri", strlen("file.uri"))) {
		    	if (request_uri)
				full_path = std::string(request_uri);
		} else if (!strncmp(request_mime_type, "application/x-shockwave-flash", strlen("application/x-shockwave-flash"))
			|| !strncmp(request_mime_type, "image/svg+xml", strlen("image/svg+xml"))
			|| !strncmp(request_mime_type, "text/html", strlen("text/html"))
			|| !strncmp(request_mime_type, "application/xml", strlen("application/xml"))) {
			if (request_uri)
				full_path = "file://" + std::string(request_uri);
		}
	} else if (request_uri) {
		full_path = std::string(request_uri);
	}

	BROWSER_LOGD("url=[%s]", full_path.c_str());

	if (ad->main_win)
		elm_win_activate(ad->main_win);

	ad->browser_instance->launch(full_path.c_str(), EINA_TRUE);

	return EINA_TRUE;
}

static void __br_load_url(bundle *b, void *app_data)
{
	BROWSER_LOGD("[%s]", __func__);
	struct browser_data *ad = (struct browser_data *)app_data;

	if (__process_app_service(b, app_data)) {
		BROWSER_LOGD("app service");
		return;
	}

	std::string full_path;
	const char *mime_type = bundle_get_val(b, AUL_K_MIME_TYPE);
	const char *content_url = bundle_get_val(b, AUL_K_MIME_CONTENT);
	const char *search_keyword = bundle_get_val(b, "search_keyword");
	const char *goto_url = bundle_get_val(b, "goto_url");
	const char *url = bundle_get_val(b, "url");
	if (mime_type && content_url) {
		BROWSER_LOGD("mime type=[%s], url=[%s]", mime_type, content_url);
		if (!strcmp(mime_type, "http.uri") || !strcmp(mime_type, "file.uri")) {
			full_path = content_url;
		} else if (!strcmp(mime_type, "application/x-shockwave-flash")
			|| !strcmp(mime_type, "image/svg+xml")
			|| !strcmp(mime_type, "text/html")
			|| !strcmp(mime_type, "application/xml")) {
			full_path = "file://" + std::string(content_url);
		}
	} else if (search_keyword) {
		BROWSER_LOGD("search_keyword=[%s]", search_keyword);
		if (search_keyword) {
			char *search_url = vconf_get_str(SEARCHURL_KEY);
			if (search_url) {
				full_path = std::string(search_url) + std::string(search_keyword);
				free(search_url);
			}
		}
	} else if (goto_url || url) {
		BROWSER_LOGD("goto_url=[%s], url=[%s]", goto_url, url);
		if (goto_url)
			full_path = goto_url;
		else
			full_path = url;
	}
	BROWSER_LOGD("url=[%s]", full_path.c_str());

	if (ad->main_win)
		elm_win_activate(ad->main_win);

	ad->browser_instance->launch(full_path.c_str(), EINA_TRUE);
}

static Eina_Bool __br_keydown_event(void *data, int type, void *event)
{
	BROWSER_LOGD("[%s]", __func__);
	Ecore_Event_Key *ev = (Ecore_Event_Key *)event;

	if (!strcmp(ev->keyname, KEY_END)) {
		__br_destroy(data);
	}

	return EXIT_FAILURE;
}

static int __br_lang_changed_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	/* To do */
	return 0;
}

static int __br_low_memory_cb(void* data)
{
	BROWSER_LOGD("[%s]", __func__);
	/* To do */
	struct browser_data *ad = (struct browser_data *)data;

	if (ad && ad->browser_instance)
		ad->browser_instance->clean_up_windows();

	return 0;
}

static void __br_register_system_event(void *app_data)
{
	struct browser_data *ad = (struct browser_data *)app_data;

	/* add system event callback */
	if (0 != appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, __br_lang_changed_cb, ad))
		BROWSER_LOGE("appcore_set_event_callback is failed.\n");

	if (0 != appcore_set_event_callback(APPCORE_EVENT_LOW_MEMORY, __br_low_memory_cb, ad))
		BROWSER_LOGE("appcore_set_event_callback is failed.\n");

	ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, __br_keydown_event, ad);
}

static int __br_app_create(void *app_data)
{
	BROWSER_LOGD("<< theme extenstion [%d]>>", appcore_measure_time());
	struct browser_data *ad = (struct browser_data *)app_data;

	ad->browser_theme = elm_theme_new();
	elm_theme_ref_set(ad->browser_theme, NULL);
	elm_theme_extension_add(ad->browser_theme, BROWSER_NAVIFRAME_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_CONTROLBAR_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_BUTTON_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_PROGRESSBAR_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_URL_LAYOUT_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_PREDICTIVE_HISTORY_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_SETTINGS_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_MOST_VISITED_SITES_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_MOST_VISITED_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_BOOKMARK_THEME);
	elm_theme_extension_add(ad->browser_theme, BROWSER_FIND_WORD_LAYOUT_THEME);

	BROWSER_LOGD("<< create main window [%d]>>", appcore_measure_time());
	ad->main_win = __create_main_win(ad);
	if (!ad->main_win) {
		BROWSER_LOGE("fail to create window");
		return -1;
	}

	BROWSER_LOGD("<< create background [%d]>>", appcore_measure_time());
	ad->bg = __create_bg(ad->main_win);
	if (!ad->bg) {
		BROWSER_LOGE("fail to create bg");
		return -1;
	}

	BROWSER_LOGD("<< create layout main [%d]>>", appcore_measure_time());
	ad->main_layout = __create_main_layout(ad->main_win);
	if (!ad->main_layout) {
		BROWSER_LOGE("fail to create main layout");
		return -1;
	}

	ad->navi_bar = __create_navi_bar(ad);
	if (!ad->navi_bar) {
		BROWSER_LOGE("fail to create navi bar");
		return -1;
	}

	/* create browser instance & init */
	ad->browser_instance = new(nothrow) Browser_Class(ad->main_win, ad->navi_bar, ad->bg, ad->main_layout);
	if (!ad->browser_instance) {
		BROWSER_LOGE("fail to Browser_Class");
		return -1;
	}
	if (ad->browser_instance->init() == EINA_FALSE) {
		BROWSER_LOGE("fail to browser init");
		return -1;
	}

	/* init internationalization */
	int ret = appcore_set_i18n(BROWSER_PACKAGE_NAME, BROWSER_LOCALE_DIR);
	if (ret) {
		BROWSER_LOGE("fail to appcore_set_i18n");
		return -1;
	}

	__br_register_system_event(ad);

	return 0;
}

/* GCF test requirement */
static void __send_termination_event_to_tapi(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int ret = TAPI_API_SUCCESS;
	int request_id = -1;
	TelSatEventDownloadReqInfo_t event_data;
	event_data.eventDownloadType = TAPI_EVENT_SAT_DW_TYPE_BROWSER_TERMINATION;
	event_data.u.browserTerminationEventReqInfo.browserTerminationCause = TAPI_SAT_BROWSER_TERMINATED_BY_USER;

	tel_init();
	tel_register_app_name((char*)"org.tizen.browser");
	tel_download_sat_event(&event_data, &request_id);
	if(ret != TAPI_API_SUCCESS && ret != TAPI_API_SAT_EVENT_NOT_REQUIRED_BY_USIM)
		BROWSER_LOGE("failed to tel_download_sat_event");

	tel_deinit();
}

static int __br_app_terminate(void *app_data)
{
	BROWSER_LOGD("[%s]", __func__);
	struct browser_data *ad = (struct browser_data *)app_data;

	elm_theme_extension_del(ad->browser_theme, BROWSER_NAVIFRAME_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_CONTROLBAR_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_BUTTON_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_URL_LAYOUT_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_PROGRESSBAR_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_PREDICTIVE_HISTORY_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_SETTINGS_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_MOST_VISITED_SITES_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_MOST_VISITED_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_BOOKMARK_THEME);
	elm_theme_extension_del(ad->browser_theme, BROWSER_FIND_WORD_LAYOUT_THEME);
	elm_theme_free(ad->browser_theme);

	/* GCF test requirement */
	__send_termination_event_to_tapi();

	if (ad->main_win);
		evas_object_del(ad->main_win);

	if (ad->browser_instance)
		delete ad->browser_instance;

	BROWSER_LOGD("<< __br_app_terminate ends [%d]>>", appcore_measure_time());
	return 0;
}

static int __br_app_pause(void *app_data)
{
	BROWSER_LOGD("[%s]", __func__);
	struct browser_data *ad = (struct browser_data *)app_data;

	if (!ad || !ad->browser_instance)
		return 0;

	ad->browser_instance->pause();

	return 0;
}

static int __br_app_resume(void *app_data)
{
	BROWSER_LOGD("[%s]", __func__);
	struct browser_data *ad = (struct browser_data *)app_data;

	if (!ad || !ad->browser_instance)
		return 0;

	ad->browser_instance->resume();

	return 0;
}

static int __br_app_reset(bundle *b, void *app_data)
{
	struct browser_data *ad = (struct browser_data *)app_data;
	BROWSER_LOGD("[%s]", __func__);
	ad->browser_instance->reset();

	__br_load_url(b, app_data);

	return 0;
}

int main(int argc, char *argv[])
{
	__br_set_env();

	appcore_measure_start();

	struct appcore_ops ops;

	ops.create = __br_app_create;
	ops.terminate = __br_app_terminate;
	ops.pause = __br_app_pause;
	ops.resume = __br_app_resume;
	ops.reset = __br_app_reset;

	struct browser_data ad;
	memset(&ad, 0x0, sizeof(struct browser_data));

	ops.data = &ad;

	int ret = appcore_efl_main(BROWSER_PACKAGE_NAME, &argc, &argv, &ops);

	return ret;
}

