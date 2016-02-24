/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contact: Kwangyong Choi <ky0.choi@samsung.com>
 *
 */

#include <app_control.h>
#include <app.h>
#include <app_extension.h>
#include <dd-display.h>
#include <Ecore_X.h>
#include <Elementary.h>
#include <Evas.h>
#include <device/display.h>
#include <device/callback.h>
#include <fcntl.h>
#include <string>
#include <storage.h>
#include <ui-gadget.h>
#include <notification/notification.h>
#include <vconf.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-object.h"
#include "browser-string.h"
#include "browser-view.h"
#include "platform-service.h"
#include "webview-list.h"
#include "webview.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#define MIN_FREE_STORAGE 5000000
#define MIN_INTERVAL_BETWEEN_MEMORY_NOTIFICATIONS 60

static Eina_Bool fullscreen_switch = EINA_FALSE;

static const char *support_mime_type[] = {"image/svg+xml", "text/html", "application/xml", NULL};

struct browser_data {
	Evas_Object *main_window;
	Evas_Object *screen_bg;
	browser *browser_instance;
	Eina_Bool on_shell_launch;
	time_t last_memory_notification;
};

static void _set_env(void)
{
	/* manual enabling of CoreGL fastpath */
	setenv("COREGL_FASTPATH", "1", 1);
	setenv("CAIRO_GL_COMPOSITOR", "msaa", 1);
	setenv("CAIRO_GL_LAZY_FLUSHING", "yes", 1);
}

static void _set_locale(void)
{
	char *r = NULL;

	char *lang_set = vconf_get_str(VCONFKEY_LANGSET);
	if (lang_set && strlen(lang_set) > 0)
		r = setlocale(LC_ALL, lang_set);

	BROWSER_LOGD("setlocale as [%s]", r);
	if (lang_set)
		free(lang_set);

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
	const char *profile = elm_config_profile_get();

	if (!strcmp(profile, "desktop"))
		elm_win_indicator_mode_set (obj, ELM_WIN_INDICATOR_HIDE);
	else
		elm_win_indicator_mode_set (obj, ELM_WIN_INDICATOR_SHOW);
}

static Evas_Object *_create_screen_bg(void *app_data)
{
	struct browser_data *ad = (struct browser_data *)app_data;

	Evas_Object* screen_bg = evas_object_rectangle_add(evas_object_evas_get(ad->main_window));
	evas_object_size_hint_weight_set(screen_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_color_set(screen_bg, 0, 0, 0, 0);
	evas_object_render_op_set(screen_bg, EVAS_RENDER_BLEND);
	elm_win_resize_object_add(ad->main_window, screen_bg);
	evas_object_show(screen_bg);

	if (ad->on_shell_launch == EINA_FALSE) {
		Evas_Object *conform = (Evas_Object *)app_get_preinitialized_conformant();
		if (conform != NULL)
			evas_object_del(conform);

		Evas_Object *bg = (Evas_Object *)app_get_preinitialized_background();
		if (bg != NULL)
			evas_object_del(bg);
	}

	return screen_bg;
}

void __platform_language_changed_cb(keynode_t *keynode, void *data)
{
	_set_locale();
}

static Evas_Object *_create_main_window(void *app_data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)app_data;
	Evas_Object *window = NULL;

	elm_app_base_scale_set(1.8);

	if (ad->on_shell_launch == EINA_FALSE)
		window = (Evas_Object *)app_get_preinitialized_window(BROWSER_PACKAGE_NAME);

	if (window == NULL)
		window = elm_win_add(NULL, BROWSER_PACKAGE_NAME, ELM_WIN_BASIC);

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
		elm_win_indicator_mode_set(window, ELM_WIN_INDICATOR_HIDE);

		// Synchronize Ecore_Animator with vxync.
		ecore_x_vsync_animator_tick_source_set(elm_win_xwindow_get(window));
		vconf_notify_key_changed(VCONFKEY_LANGSET, __platform_language_changed_cb, NULL);
	}

	return window;
}

static void _exit_popup_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del((Evas_Object *)data);
	elm_exit();
}

static void _exit_browser(Evas_Object *parent)
{
	BROWSER_LOGD("");

	Evas_Object *popup = NULL;
	popup = elm_popup_add(parent);
	elm_popup_align_set(popup, -1.0, 1.0);
	RET_MSG_IF(!popup, "popup is NULL");

	elm_object_part_text_set(popup, "title,text", BR_STRING_WARNING);
	elm_object_text_set(popup, BR_STRING_DISK_FULL);

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _exit_popup_button_cb, popup);
#endif

	Evas_Object *button = elm_button_add(popup);
	if (!button) {
		BROWSER_LOGE("elm_button_add failed");
		return;
	}

	elm_object_text_set(button, BR_STRING_OK);

	elm_object_style_set(button, "popup");
	elm_object_part_content_set(popup, "button1", button);
	evas_object_smart_callback_add(button, "clicked", _exit_popup_button_cb, popup);
	evas_object_show(popup);

	evas_object_show(parent);
	elm_win_activate(parent);
}

void __close_browser_low_internal_memory_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	elm_exit();
}

void __check_internal_storage(struct browser_data * sBrowserData)
{
	BROWSER_LOGD("");
	struct statvfs sVfs;
	int iRetVal;
	double free_size,required_size = MIN_FREE_STORAGE;
	iRetVal = storage_get_internal_memory_size(&sVfs);
	free_size = (double)(sVfs.f_bsize) * sVfs.f_bfree;
	BROWSER_LOGD("total size  : %lf, avail size : %lf, free size :%lf", (double)sVfs.f_frsize*sVfs.f_blocks, (double)sVfs.f_bsize*sVfs.f_bavail, (double)sVfs.f_bsize*sVfs.f_bfree);
	if (iRetVal< 0) {
		BROWSER_LOGE("Could not get available memory!");
	} else if (free_size < required_size){
		BROWSER_LOGD("Not enough memory!!!");
		sBrowserData->browser_instance->get_browser_view()->show_msg_popup(NULL, "BR_STRING_DISK_FULL", __close_browser_low_internal_memory_cb, "IDS_BR_SK_OK", __close_browser_low_internal_memory_cb);
	}
}

static void __br_rotate_cb(void *data, Evas_Object *obj, void *event)
{
	if (!data) {
		BROWSER_LOGE("data is NULL");
		return;
	}

	struct browser_data *ad = (struct browser_data *)data;
	int changed_ang = elm_win_rotation_get(ad->main_window);
	BROWSER_SECURE_LOGD("changed_ang[%d]", changed_ang);

	ad->browser_instance->rotate(changed_ang);
}

static void _register_rotate_callback(Evas_Object *main_window, void *user_data)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!main_window, "main_window is NULL");

	if (elm_win_wm_rotation_supported_get(main_window)) {
		const int rots[] = { 0, 90, 270};
		elm_win_wm_rotation_available_rotations_set(main_window, rots, (sizeof(rots) / sizeof(int)));
	}

	evas_object_smart_callback_add(main_window, "wm,rotation,changed", __br_rotate_cb, user_data);
}

static bool __br_app_create(void *app_data)
{
	BROWSER_LOGD("");

	elm_config_accel_preference_set("opengl:depth24:stencil8");

	struct browser_data *ad = (struct browser_data *)app_data;

	_set_locale();
	ad->main_window = _create_main_window(app_data);
	if (elm_config_access_get()) {
		char buf[256];
		snprintf(buf, sizeof(buf), "%s", BR_STRING_INTERNET);
		elm_access_force_say(buf);
	}
	ad->screen_bg = _create_screen_bg(ad);

	int low_memory = 0;
	vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &low_memory);
	if (low_memory > VCONFKEY_SYSMAN_LOW_MEMORY_SOFT_WARNING) {
		_exit_browser(ad->main_window);
		return false;
	}

	ad->browser_instance = new browser(ad->main_window);
	ad->browser_instance->set_screen_bg(ad->screen_bg);
	__check_internal_storage(ad);

	_register_rotate_callback(ad->main_window, ad);

	return true;
}

static void _suspend_manager(void *app_data, Eina_Bool suspend, Eina_Bool fullscreen_switch)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)app_data;
	Eina_Bool urlbar_show = ad->browser_instance->get_browser_view()->is_show_url_input_bar();

	// On app pause
	if(suspend){
		if(!urlbar_show && !fullscreen_switch){
			ad->browser_instance->set_full_screen_enable(EINA_TRUE);
			ad->browser_instance->get_browser_view()->set_keep_urlbar_on_resume(EINA_TRUE);
		}
	}

	// On app resume
	if(!suspend){
		if(!urlbar_show && !fullscreen_switch){
			ad->browser_instance->set_full_screen_enable(EINA_FALSE);
			ad->browser_instance->get_browser_view()->set_keep_urlbar_on_resume(EINA_FALSE);
		}
	}
}

static void __br_app_pause(void *app_data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)app_data;

	if (!ad->browser_instance)
		return;

	// Browser instance is suspended by power state callback.
	display_state_e state;
	device_display_get_state(&state);
	BROWSER_LOGD("state: %d", state);
	if (state != DISPLAY_STATE_SCREEN_OFF) {
		ad->browser_instance->pause();
		ug_pause();
	} else {
		ad->browser_instance->set_app_paused_by_display_off(EINA_TRUE);
		ad->browser_instance->deregister();
	}

	fullscreen_switch = ad->browser_instance->get_full_screen_enable();
	_suspend_manager(app_data, EINA_TRUE, fullscreen_switch);

	ad->browser_instance->set_app_paused(EINA_TRUE);
}

static void __br_app_resume(void *app_data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)app_data;
	if (!ad->browser_instance)
		return;

	if (elm_config_access_get()) {
		char buf[256];
		snprintf(buf, sizeof(buf), "%s", BR_STRING_INTERNET);
		elm_access_force_say(buf);
	}

	ad->browser_instance->get_browser_view()->set_notipopup_check(EINA_FALSE);

	_suspend_manager(app_data, EINA_FALSE, fullscreen_switch);

	ad->browser_instance->resume();
	ug_resume();

	display_set_image_enhance(ENHANCE_SCENARIO, 5);
}

static Eina_Bool _supported_mime_type(const char *mime_type)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!mime_type, EINA_FALSE, "mime_type is NULL");

	int index = 0;
	while (support_mime_type[index]) {
		if (!strcmp(support_mime_type[index], mime_type))
			return EINA_TRUE;
		index++;
	}

	return EINA_FALSE;
}

static void __br_app_reset(app_control_h app_control, void *app_data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)app_data;

	if (!ad->browser_instance)
		return;

	ad->browser_instance->get_browser_view()->pop_to_browser_view();
	ad->browser_instance->reset();
	ad->browser_instance->navi_frame_tree_focus_allow_set(EINA_TRUE);
	ad->browser_instance->get_browser_view()->set_notipopup_check(EINA_FALSE);

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

	BROWSER_LOGD("operation = [%s], request_uri = [%s], request_mime_type = [%s]", operation, request_uri, request_mime_type);

	display_set_image_enhance(ENHANCE_SCENARIO, 5);

	evas_object_show(ad->main_window);
	elm_win_activate(ad->main_window);

	char *referer = NULL;
	if (app_control_get_extra_data(app_control, "Referer", &referer) == APP_CONTROL_ERROR_NONE) {
		BROWSER_SECURE_LOGD("referer: %s", referer);
		ad->browser_instance->set_referer_header(referer);
		if (referer)
			free(referer);
	}

	char *homepage = NULL;
	if (app_control_get_extra_data(app_control, "homepage", &homepage) == APP_CONTROL_ERROR_NONE) {
		if (homepage) {
			ad->browser_instance->launch_homepage();
			free(homepage);
		}
	} else {
		if (_supported_mime_type(request_mime_type)) {
			ad->browser_instance->launch(request_uri);
		} else if (request_uri) {
			std::string uri = std::string(request_uri);
			if (!strncmp(request_uri, "/opt/", strlen("/opt/")))
				uri = std::string("file://") + uri;
			ad->browser_instance->launch(uri.c_str());
		} else {
			ad->browser_instance->launch(request_uri);
		}
	}

	char *search_keyword = NULL;
	if (app_control_get_extra_data(app_control, "search", &search_keyword) == APP_CONTROL_ERROR_NONE) {
		BROWSER_LOGD("search keyword launching");

		if (search_keyword) {
			ad->browser_instance->launch_for_searching(search_keyword);
			free(search_keyword);
		}
	}

	if ((operation && !strcmp(operation, "http://tizen.org/appcontrol/operation/search")) &&
		((app_control_get_extra_data(app_control, "http://tizen.org/appcontrol/data/keyword", &search_keyword) == APP_CONTROL_ERROR_NONE) ||
		(app_control_get_extra_data(app_control, "http://tizen.org/appcontrol/data/text", &search_keyword) == APP_CONTROL_ERROR_NONE))) {
		BROWSER_LOGD("http://tizen.org/appcontrol/data/keyword");

		if (search_keyword) {
			ad->browser_instance->launch_for_searching(search_keyword);
			free(search_keyword);
		}
	}

	if (request_uri)
		free(request_uri);
	if (request_mime_type)
		free(request_mime_type);
	if (operation)
		free(operation);

	__check_internal_storage(ad);
	ad->browser_instance->set_app_paused(EINA_FALSE);
}

static void __display_changed_cb(device_callback_e type, void *value, void *user_data)
{
	BROWSER_LOGD("%d, %p, %p", type, value, user_data);

	struct browser_data *ad = (struct browser_data *)user_data;

	if (!ad->browser_instance)
		return;

	display_state_e state = DISPLAY_STATE_NORMAL;
	if (value == (void *)0x0)
		state = DISPLAY_STATE_NORMAL;
	else if (value == (void *)0x1)
		state = DISPLAY_STATE_SCREEN_DIM;
	else if (value == (void *)0x2)
		state = DISPLAY_STATE_SCREEN_OFF;
	else
		BROWSER_LOGE("wrong display state");

	BROWSER_LOGD("DISPLAY_STATE_SCREEN state : [%d]", state);
	if (state == DISPLAY_STATE_SCREEN_OFF) {
		ad->browser_instance->set_app_paused_by_display_off(EINA_TRUE);
		ad->browser_instance->pause();
		ug_pause();
	} else if (state == DISPLAY_STATE_NORMAL) {
		if (ad->browser_instance->get_app_paused()) {
			BROWSER_LOGD("paused");
			return;
		}

		__br_app_resume(ad);
	}
}

static void __br_app_terminate(void *app_data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)app_data;

	if (ad->browser_instance)
		delete ad->browser_instance;

	evas_object_del(ad->main_window);

	device_remove_callback(DEVICE_CALLBACK_DISPLAY_STATE, __display_changed_cb);
	vconf_ignore_key_changed(VCONFKEY_LANGSET, __platform_language_changed_cb);
}

static void notify_low_memory(browser_data *ad)
{
	BROWSER_LOGD("");
	if(!ad)
		return;

	time_t current_time;
	time(&current_time);

	double offset = difftime(current_time, ad->last_memory_notification);
	if (offset < MIN_INTERVAL_BETWEEN_MEMORY_NOTIFICATIONS)
		return;
	ad->last_memory_notification = current_time;

	notification_h low_memory_notification_handle = notification_create(NOTIFICATION_TYPE_NOTI);

	if(!low_memory_notification_handle) {
		BROWSER_LOGE("notification_create failed.");
		return;
	}

	int err = NOTIFICATION_ERROR_NONE;

	notification_set_property(low_memory_notification_handle, NOTIFICATION_PROP_VOLATILE_DISPLAY);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("notification_set_property failed with err[%d]",err);
		goto CLEANUP;
	}

	err = notification_set_text(low_memory_notification_handle, NOTIFICATION_TEXT_TYPE_TITLE,
	                            gettext("IDS_BR_POP_NOT_ENOUGH_MEMORY"), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("notification_set_text failed with err[%d]",err);
		goto CLEANUP;
	}

	err = notification_set_text(low_memory_notification_handle, NOTIFICATION_TEXT_TYPE_CONTENT,
	                            gettext("IDS_BR_POP_NOT_ENOUGH_MEMORY_DELETE_SOME_ITEMS_AND_TRY_AGAIN"),
	                            NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("notification_set_text failed with err[%d]",err);
		goto CLEANUP;
	}

	err = notification_post(low_memory_notification_handle);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("notification_post failed with err[%d]",err);
		goto CLEANUP;
	}

CLEANUP:

	err = notification_free(low_memory_notification_handle);
	if (err != NOTIFICATION_ERROR_NONE)
		 BROWSER_LOGE("notification_free is failed with err[%d]", err);

	return;
}

static void __br_low_memory_cb(app_event_info_h event_info, void* data)
{
	BROWSER_LOGD("");
	struct browser_data *ad = (struct browser_data *)data;
	notify_low_memory(ad);
	if (!ad->browser_instance)
		return;
	ad->browser_instance->get_webview_list()->clean_up_webviews();
}

static void __br_lang_changed_cb(app_event_info_h event_info, void *data)
{
	BROWSER_LOGD("");
	char *lang_set = vconf_get_str(VCONFKEY_LANGSET);
	if (lang_set) {
		elm_language_set(lang_set);
		free(lang_set);
	}

	struct browser_data *ad = (struct browser_data *)data;
	if (!ad->browser_instance)
		return;

	ad->browser_instance->language_changed();

	return;
}

EXPORT_API int main(int argc, char *argv[])
{
	BROWSER_LOGD("");

	int ret = 0;
	app_event_handler_h low_memory_hanlder, lang_changed_handler;

	_set_env();

	ui_app_lifecycle_callback_s ops;
	memset(&ops, 0x00, sizeof(ui_app_lifecycle_callback_s));

	ops.create = __br_app_create;
	ops.terminate = __br_app_terminate;
	ops.pause = __br_app_pause;
	ops.resume = __br_app_resume;
	ops.app_control = __br_app_reset;

	struct browser_data ad;
	memset(&ad, 0x00, sizeof(struct browser_data));
	ad.on_shell_launch = EINA_TRUE;

#ifndef WEBKIT_EFL
	char** modified_argv = NULL;

	/**
	 * @important
	 *
	 * Chromium process model option "--process-per-tab" is used as
	 * a workaround for issue CBBROWSER-553
	 * "Blank screen is displayed when tap on stay/leave
	 * this page after changing setting of google page"
	 * (http://suprem.sec.samsung.net/jira/browse/CBBROWSER-553).
	 *
	 */
	//Workaround for passing the specific chromium-efl command line parameter
	//It should be added when app launched by icon
	if((argc > 2) && argv[2] && !strcmp(argv[2],"__AUL_STARTTIME__")){
		modified_argv = new(std::nothrow)char*[3];
		if (modified_argv)
		{
			modified_argv[0] = argv[0];
			modified_argv[1] = "--process-per-tab --js-flags=--optimize-for-size --turbo-asm";
			modified_argv[2] = NULL;
			argc = 2;
			argv = modified_argv;
		}
	}
	else if (argc > 0)
	{
		modified_argv = new(std::nothrow)char*[argc + 2];
		if (modified_argv)
		{
			memcpy(modified_argv, argv, sizeof(char*) * argc);
			modified_argv[argc] = "--process-per-tab";
			modified_argv[argc + 1] = NULL;
			argc += 1;
			argv = modified_argv;
		}
	}
#endif
	// In order to have selenium webdriver properly enabled, we need to
	// pass the appropriate parameters to EWK.
	ewk_set_arguments(argc, argv);

	ret = device_add_callback(DEVICE_CALLBACK_DISPLAY_STATE, __display_changed_cb, &ad);
	if (ret != 0)
		BROWSER_LOGE("device_add_callback failed!");

	ret = ui_app_add_event_handler(&low_memory_hanlder, APP_EVENT_LOW_MEMORY, __br_low_memory_cb, &ad);
	if (ret != APP_ERROR_NONE)
		BROWSER_LOGE("ui_app_add_event_handler failed!");

	ret = ui_app_add_event_handler(&lang_changed_handler, APP_EVENT_LANGUAGE_CHANGED, __br_lang_changed_cb, &ad);
	if (ret != APP_ERROR_NONE)
		BROWSER_LOGE("ui_app_add_event_handler failed!");

	ret = ui_app_main(argc, argv, &ops, &ad);

#ifndef WEBKIT_EFL
	delete[] modified_argv;
#endif
	return ret;
}
