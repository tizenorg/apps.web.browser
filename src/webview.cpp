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

#include "webview.h"

#include <Elementary.h>
#include <bundle.h>
#include <string>
#include <syspopup_caller.h>

#include "authentication-manager.h"
#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "certificate-manager.h"
#include "custom-content-handler.h"
#include "custom-protocol-handler.h"
#include "download-manager.h"
#include "history.h"
#include "html5-feature-manager.h"
#include "multiwindow-view.h"
#include "network-manager.h"
#include "platform-service.h"
#include "preference.h"
#include "reader.h"
#include "uri-bar.h"
#include "user-agent-manager.h"
#include "webview-list.h"

static void _exit_browser(void)
{
	BROWSER_LOGD("");
	bundle *b = bundle_create();
	bundle_add(b, "_SYSPOPUP_TITLE_", "Warning");
	bundle_add(b, "_SYSPOPUP_CONTENT_", "Webkit initialization has been failed");
	syspopup_launch("syspopup-app", b);
	bundle_free(b);

	elm_exit();
}


#define FIND_WORD_MAX_COUNT	1000
#define certificate_crt_path	"/opt/usr/share/certs/ca-certificate.crt"

webview_context::webview_context(void)
{
	BROWSER_LOGD("");
	ewk_init();

	m_context = ewk_context_default_get();
	ewk_context_did_start_download_callback_set(m_context, __download_did_start_cb, this);

	ewk_context_cache_model_set(m_context, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);

	ewk_context_certificate_file_set(m_context, certificate_crt_path);

	_init_setting();
}

webview_context::~webview_context(void)
{
	BROWSER_LOGD("");
	ewk_shutdown();
}

void webview_context::__download_did_start_cb(const char *download_uri, void *data)
{
	BROWSER_LOGD("download_uri = [%s]", download_uri);
	EINA_SAFETY_ON_NULL_RETURN(download_uri);

	m_browser->get_download_manager()->launch_download_app(download_uri);
}

void webview_context::set_proxy_address(const char *proxy)
{
	BROWSER_LOGD("proxy = [%s]", proxy);
	EINA_SAFETY_ON_NULL_RETURN(proxy);
	ewk_context_proxy_uri_set(m_context, proxy);
}

const char *webview_context::get_proxy_address(void)
{
	return ewk_context_proxy_uri_get(m_context);
}

void webview_context::accept_cookie_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);
	Ewk_Cookie_Manager *cookie_manager = ewk_context_cookie_manager_get(m_context);
	if (enable)
		ewk_cookie_manager_accept_policy_set(cookie_manager, EWK_COOKIE_ACCEPT_POLICY_ALWAYS);
	else
		ewk_cookie_manager_accept_policy_set(cookie_manager, EWK_COOKIE_ACCEPT_POLICY_NEVER);
}

void webview_context::clear_cookies(void)
{
	Ewk_Cookie_Manager *cookie_manager = ewk_context_cookie_manager_get(m_context);
	ewk_cookie_manager_cookies_clear(cookie_manager);
}

void webview_context::clear_private_data(void)
{
	clear_memory_cache();

	ewk_context_web_indexed_database_delete_all(m_context);
	ewk_context_application_cache_delete_all(m_context);
	ewk_context_web_storage_delete_all(m_context);
	ewk_context_web_database_delete_all(m_context);
}

Evas_Object *webview_context::get_favicon(const char *uri)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, NULL);

	Evas_Object *favicon = ewk_context_icon_database_icon_object_add(m_context, uri, evas_object_evas_get(m_window));

	return favicon;
}

void webview_context::clear_memory_cache(void)
{
	ewk_context_cache_clear(m_context);
}

void webview_context::clear_form_data(void)
{
	ewk_context_form_candidate_data_clear(m_context);
}

void webview_context::clear_saved_ID_and_PW(void)
{
	ewk_context_form_password_data_clear(m_context);
}

void webview_context::notify_low_memory(void)
{
	ewk_context_notify_low_memory(m_context);
}

void webview_context::vibration_callbacks_set(Ewk_Vibration_Client_Vibrate_Cb vibrate, Ewk_Vibration_Client_Vibration_Cancel_Cb cancel, void *data)
{
	ewk_context_vibration_client_callbacks_set(m_context, vibrate, cancel, data);
}

void webview_context::_init_setting(void)
{
	BROWSER_LOGD("");
	accept_cookie_enabled_set(m_preference->get_accept_cookies_enabled());
#if defined(TEST_CODE)
	if (m_preference->get_demo_mode_enabled()) {
		memory_sampler_enabled_set(m_preference->get_memory_usage_profiling_enabled());
		memory_saving_mode_set(m_preference->get_low_memory_mode_enabled());
	}
#endif
}

#if defined(TEST_CODE)
void webview_context::memory_sampler_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	if (enable)
		ewk_context_memory_sampler_start(m_context, 0);
	else
		ewk_context_memory_sampler_stop(m_context);
}

void webview_context::memory_saving_mode_set(Eina_Bool enable)
{
	ewk_context_memory_saving_mode_set(m_context, enable);
}
#endif

typedef struct _webview_callback
{
   const char *event;
   Evas_Smart_Cb func;
   void *func_data;
} webview_callback;

webview::webview(Eina_Bool user_created)
:
	m_event_callbacks(NULL)
	,m_already_activated(EINA_FALSE)
	,m_is_user_created(user_created)
	,m_snapshot(NULL)
	,m_uri(NULL)
	,m_title(NULL)
	,m_reader_enabled(EINA_FALSE)
	,m_has_focus(EINA_FALSE)
{
	BROWSER_LOGD("");
	m_ewk_view = ewk_view_add(evas_object_evas_get(m_window));
	if (!m_ewk_view)
		_exit_browser();

	evas_object_color_set(m_ewk_view, 255, 255, 255, 255);
	evas_object_size_hint_weight_set(m_ewk_view, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_ewk_view, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_html5_manager = new html5_feature_manager(this);

	m_auth_manager = new authentication_manager(this);

	m_certi_manager = new certificate_manager(this);

	_init_setting();
}

webview::~webview(void)
{
	if (m_html5_manager)
		delete m_html5_manager;

	if (m_auth_manager)
		delete m_auth_manager;

	if (m_certi_manager)
		delete m_certi_manager;

	while (m_event_callbacks) {
		webview_callback *cb = (webview_callback *)eina_list_data_get(m_event_callbacks);
		m_event_callbacks = eina_list_remove(m_event_callbacks, cb);
		if (cb->event)
			eina_stringshare_del(cb->event);
		free(cb);
	}

	eina_stringshare_del(m_uri);
	eina_stringshare_del(m_title);

	if (m_snapshot)
		evas_object_del(m_snapshot);

	if (m_ewk_view) {
		evas_object_del(m_ewk_view);
		m_ewk_view = NULL;
	}
}

void webview::delete_ewk_view(void)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	eina_stringshare_replace(&m_uri, get_uri());
	eina_stringshare_replace(&m_title, get_title());

	evas_object_del(m_ewk_view);
	m_ewk_view = NULL;
}

void webview::orientation_send(int degree)
{
	if (degree == 270)
		ewk_view_orientation_send(m_ewk_view, -90);
	else
		ewk_view_orientation_send(m_ewk_view, degree);
}

void webview::load_uri(const char *uri)
{
	EINA_SAFETY_ON_NULL_RETURN(uri);
	if (!m_ewk_view) {
		m_ewk_view = ewk_view_add(evas_object_evas_get(m_window));

		evas_object_color_set(m_ewk_view, 255, 255, 255, 255);
		evas_object_size_hint_weight_set(m_ewk_view, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_ewk_view, EVAS_HINT_FILL, EVAS_HINT_FILL);

		_init_setting();
	}

	std::string scheme_uri = std::string(uri);

	if (!is_supported_scheme(uri))
		scheme_uri = std::string(http_scheme) + scheme_uri;

	BROWSER_LOGD("uri = [%s]", scheme_uri.c_str());

	ewk_view_url_set(m_ewk_view, scheme_uri.c_str());

	uri_bar *ub = m_browser->get_browser_view()->get_uri_bar();
	ub->set_uri(scheme_uri.c_str());

	if (m_browser->get_network_manager()->check_network_unlock() == EINA_TRUE) {
		m_browser->get_network_manager()->show_confirm_network_unlock_popup();
	}
}

void webview::load_stop(void)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ewk_view_stop(m_ewk_view);
}

void webview::reload(void)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ewk_view_reload(m_ewk_view);
}

const char *webview::get_uri(void)
{
	if (!m_ewk_view)
		return m_uri;

	return ewk_view_url_get(m_ewk_view);
}

const char *webview::get_title(void)
{
	if (!m_ewk_view)
		return m_title;

	return ewk_view_title_get(m_ewk_view);
}

double webview::get_progress(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, 0);

	return ewk_view_load_progress_get(m_ewk_view);
}

Evas_Object *webview::get_favicon(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, NULL);

	return ewk_context_icon_database_icon_object_add(ewk_view_context_get(m_ewk_view), get_uri(), evas_object_evas_get(m_ewk_view));
}

Eina_Bool webview::is_loading(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, EINA_FALSE);

	if (get_progress() == 0.0f || get_progress() == 1.0f)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

void webview::backward(void)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ewk_view_back(m_ewk_view);
}

void webview::forward(void)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ewk_view_forward(m_ewk_view);
}

Eina_Bool webview::backward_possible(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, EINA_FALSE);

	return ewk_view_back_possible(m_ewk_view);
}

Eina_Bool webview::forward_possible(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, EINA_FALSE);

	return ewk_view_forward_possible(m_ewk_view);
}

void webview::suspend(void)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ewk_view_suspend(m_ewk_view);
	ewk_view_visibility_set(m_ewk_view, EINA_FALSE);
}

void webview::resume(void)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ewk_view_resume(m_ewk_view);
	ewk_view_visibility_set(m_ewk_view, EINA_TRUE);
}

Eina_Bool webview::save_as_pdf(int w, int h, const char *pdf_file_path)
{
	BROWSER_LOGD("pdf_file_path=[%s]", pdf_file_path);
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(pdf_file_path, EINA_FALSE);

	ewk_view_contents_pdf_get(m_ewk_view, w, h, pdf_file_path);

	return EINA_TRUE;
}

Evas_Object *webview::capture_snapshot(int x, int y, int w, int h, float scale)
{
	BROWSER_LOGD("");

	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(scale, NULL);

	Eina_Rectangle snapshot_rect;
	snapshot_rect.x = x;
	snapshot_rect.y = y;
	snapshot_rect.w = w;
	snapshot_rect.h = h;

	Evas_Object *snapshot = NULL;
	snapshot = ewk_view_screenshot_contents_get(m_ewk_view, snapshot_rect, scale, evas_object_evas_get(m_ewk_view));
	if (!snapshot) {
		BROWSER_LOGE("ewk_view_screenshot_contents_get failed");

		snapshot = evas_object_rectangle_add(evas_object_evas_get(m_ewk_view));
		evas_object_resize(snapshot, w * scale, h * scale);
		evas_object_color_set(snapshot, 255, 255, 255, 255);
	} else {
		evas_object_image_filled_set(snapshot, EINA_TRUE);
	}
	evas_object_size_hint_min_set(snapshot, 1, 1);
	evas_object_size_hint_weight_set(snapshot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(snapshot, EVAS_HINT_FILL, EVAS_HINT_FILL);

	return snapshot;
}

void webview::attach_event(const char *event, Evas_Smart_Cb func, const void *data)
{
	BROWSER_LOGD("");

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	EINA_SAFETY_ON_NULL_RETURN(event);
	EINA_SAFETY_ON_NULL_RETURN(func);

	webview_callback *cb = (webview_callback *)malloc(sizeof(webview_callback));
	memset(cb, 0x00, sizeof(webview_callback));

	cb->event = eina_stringshare_add(event);
	cb->func = func;
	cb->func_data = (void *)data;
	m_event_callbacks = eina_list_append(m_event_callbacks, cb);
}

void webview::detach_event(const char *event, Evas_Smart_Cb func)
{
	BROWSER_LOGD("");

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	EINA_SAFETY_ON_NULL_RETURN(event);
	EINA_SAFETY_ON_NULL_RETURN(func);

	Eina_List *l;

	void *data;
	EINA_LIST_FOREACH(m_event_callbacks, l, data) {
		webview_callback *cb = (webview_callback *)data;
		if (cb->event && (!strcmp(cb->event, event)) && (cb->func == func)) {
			m_event_callbacks = eina_list_remove(m_event_callbacks, cb);
				eina_stringshare_del(cb->event);
			free(cb);

			break;
		}
	}
}

void webview::activate(void)
{
	BROWSER_LOGD("");

	Eina_Bool is_deleted_ewk_view = EINA_FALSE;
	if (!m_ewk_view) {
		m_ewk_view = ewk_view_add(evas_object_evas_get(m_window));

		evas_object_color_set(m_ewk_view, 255, 255, 255, 255);
		evas_object_size_hint_weight_set(m_ewk_view, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_ewk_view, EVAS_HINT_FILL, EVAS_HINT_FILL);

		_init_setting();

		is_deleted_ewk_view = EINA_TRUE;
	}

	if (m_already_activated) {
		BROWSER_LOGD("already activated");
		return;
	}

	evas_object_smart_callback_add(m_ewk_view, "load,started", __load_started_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "load,committed", __load_committed_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "load,error", __load_error_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "load,nonemptylayout,finished", __load_nonempty_layout_finished_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "title,changed", __title_changed_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "load,progress", __load_progress_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "load,finished", __load_finished_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "process,crashed", __process_crashed_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "create,window", __create_window_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "close,window", __close_window_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "icon,received", __icon_received_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "policy,navigation,decide", __policy_navigation_decide_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "policy,response,decide", __policy_response_decide_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "form,submit", __form_submit_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "edge,bottom", __edge_bottom_cb, this);

	enable_customize_contextmenu(EINA_TRUE);

	Eina_List *l;
	void *data;
	EINA_LIST_FOREACH(m_event_callbacks, l, data) {
		webview_callback *cb = (webview_callback *)data;
		BROWSER_LOGD("cb->event=[%s]", cb->event);
		evas_object_smart_callback_add(m_ewk_view, cb->event, cb->func, cb->func_data);
	}

	resume();

	m_already_activated = EINA_TRUE;

	if (is_deleted_ewk_view)
		load_uri(m_uri);
}

void webview::deactivate(void)
{
	BROWSER_LOGD("");

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	if (!m_already_activated) {
		BROWSER_LOGD("already deactivated");
		return;
	}

	evas_object_smart_callback_del(m_ewk_view, "load,started", __load_started_cb);
	evas_object_smart_callback_del(m_ewk_view, "load,committed", __load_committed_cb);
	evas_object_smart_callback_del(m_ewk_view, "load,error", __load_error_cb);
	evas_object_smart_callback_del(m_ewk_view, "load,nonemptylayout,finished", __load_nonempty_layout_finished_cb);
	evas_object_smart_callback_del(m_ewk_view, "title,changed", __title_changed_cb);
	evas_object_smart_callback_del(m_ewk_view, "load,progress", __load_progress_cb);
	evas_object_smart_callback_del(m_ewk_view, "load,finished", __load_finished_cb);

	evas_object_smart_callback_del(m_ewk_view, "process,crashed", __process_crashed_cb);

	evas_object_smart_callback_del(m_ewk_view, "create,window", __create_window_cb);
	evas_object_smart_callback_del(m_ewk_view, "close,window", __close_window_cb);

	evas_object_smart_callback_del(m_ewk_view, "icon,received", __icon_received_cb);

	evas_object_smart_callback_del(m_ewk_view, "policy,navigation,decide", __policy_navigation_decide_cb);
	evas_object_smart_callback_del(m_ewk_view, "policy,response,decide", __policy_response_decide_cb);

	evas_object_smart_callback_del(m_ewk_view, "form,submit", __form_submit_cb);

	evas_object_smart_callback_del(m_ewk_view, "edge,bottom", __edge_bottom_cb);

	enable_customize_contextmenu(EINA_FALSE);

	Eina_List *l;
	void *data;
	EINA_LIST_FOREACH(m_event_callbacks, l, data) {
		webview_callback *cb = (webview_callback *)data;
		if (cb) {
			BROWSER_LOGD("cb->event=[%s]", cb->event);
			evas_object_smart_callback_del(m_ewk_view, cb->event, cb->func);
		}
	}

	suspend();

	m_already_activated = EINA_FALSE;

	m_has_focus = evas_object_focus_get(m_ewk_view);
	BROWSER_LOGD("************ m_has_focus=%d", m_has_focus);
}

void webview::enable_customize_contextmenu(Eina_Bool enable)
{
	if (enable) {
		evas_object_smart_callback_add(m_ewk_view, "contextmenu,customize", __contextmenu_customize_cb, this);
		evas_object_smart_callback_add(m_ewk_view, "contextmenu,selected", __contextmenu_selected_cb, this);
	} else {
		evas_object_smart_callback_del(m_ewk_view, "contextmenu,customize", __contextmenu_customize_cb);
		evas_object_smart_callback_del(m_ewk_view, "contextmenu,selected", __contextmenu_selected_cb);
	}
}

void webview::replace_ewk_view(Evas_Object *ewk_view)
{
	BROWSER_LOGD("");

	EINA_SAFETY_ON_NULL_RETURN(ewk_view);

	if (m_ewk_view)
		evas_object_del(m_ewk_view);

	m_ewk_view = ewk_view;
}

void webview::auto_fitting_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_auto_fitting_set(settings, enable);
}

void webview::javascript_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_javascript_enabled_set(settings, enable);
}

void webview::auto_load_images_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_loads_images_automatically_set(settings, enable);
}

void webview::scripts_window_open_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_scripts_window_open_set(settings, enable);
}

void webview::text_encoding_type_set(const char *encoding_type)
{
	BROWSER_LOGD("encoding_type = [%s]", encoding_type);
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	EINA_SAFETY_ON_NULL_RETURN(encoding_type);

	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	if (!strncmp(encoding_type, PREF_VALUE_TEXT_ENCODING_AUTOMATIC, strlen(PREF_VALUE_TEXT_ENCODING_AUTOMATIC)))
		ewk_settings_uses_encoding_detector_set(settings, EINA_TRUE);
	else
		ewk_settings_default_encoding_set(settings, encoding_type);
}

void webview::save_ID_and_PW_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);

	ewk_settings_autofill_password_form_enabled_set(settings, enable);
}

void webview::remember_form_data_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);

	ewk_settings_form_candidate_data_enabled_set(settings, enable);
}

void webview::user_agent_set(const char *user_agent)
{
	BROWSER_LOGD("user agent = [%s]", user_agent);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	ewk_view_user_agent_set(m_ewk_view, user_agent);
}

const char *webview::user_agent_get(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, NULL);
	return ewk_view_user_agent_get(m_ewk_view);
}

void webview::ime_autofocus_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_show_ime_on_autofocus_set(settings, enable);
}

void webview::custom_http_header_set(const char *name, const char *value)
{
	BROWSER_LOGD("name = [%s], value = [%s]", name, value);
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	EINA_SAFETY_ON_NULL_RETURN(name);
	EINA_SAFETY_ON_NULL_RETURN(value);

	ewk_view_custom_header_add(m_ewk_view, name, value);
}

void webview::get_geometry(int *x, int *y, int *w, int *h)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	evas_object_geometry_get(m_ewk_view, x, y, w, h);
}

void webview::plugin_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_plugins_enabled_set(settings, enable);
}

void webview::motion_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	evas_object_smart_callback_call(m_ewk_view, "motion,enable", (void*)&enable);
}

void webview::link_mangnifier_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_link_magnifier_enabled_set(settings, enable);
}

void webview::scroll_size_get(int *w, int *h)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ewk_view_scroll_size_get(m_ewk_view, w, h);
}

void webview::scroll_position_get(int *x, int *y)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ewk_view_scroll_pos_get(m_ewk_view, x, y);
}

void webview::scroll_position_set(int x, int y)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ewk_view_scroll_set(m_ewk_view, x, y);
}

double webview::scale_get(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, 0.0);

	return ewk_view_scale_get(m_ewk_view);
}

void webview::content_size_get(int *w, int *h)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ewk_view_contents_size_get(m_ewk_view, w, h);
}

#if defined(TEST_CODE)
void webview::inspector_server_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	unsigned int port_num = 0;
	if (enable) {
		port_num = ewk_view_inspector_server_start(m_ewk_view, 0);
		char address[30] = {0, };
		BROWSER_LOGD("====== port_num = [%d]", port_num);
		sprintf(address, "%s%d", "192.168.129.3:", port_num);
		m_browser->get_browser_view()->show_msg_popup(NULL, address);
	} else
		ewk_view_inspector_server_stop(m_ewk_view);
}

void webview::recording_surface_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	ewk_view_recording_surface_enable_set(m_ewk_view, enable);
}

void webview::layer_borders_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_compositing_borders_visible_set(settings, enable);
}
#endif

void webview::_init_setting(void)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);

	ime_autofocus_set(EINA_FALSE);

	if (m_preference->get_view_level_type() == VIEW_LEVEL_TYPE_FIT_TO_WIDTH)
		auto_fitting_enabled_set(EINA_TRUE);
	else
		auto_fitting_enabled_set(EINA_FALSE);

	javascript_enabled_set(m_preference->get_javascript_enabled());
	auto_load_images_enabled_set(m_preference->get_display_images_enabled());
	/* If the block popup is enabled, the script window is disabled. (reverse) */
	scripts_window_open_enabled_set(!m_preference->get_block_popup_enabled());
	text_encoding_type_set(m_preference->get_text_encoding_type_str(m_preference->get_text_encoding_type_index()));
	save_ID_and_PW_enabled_set(m_preference->get_auto_save_id_password_enabled());
	remember_form_data_enabled_set(m_preference->get_auto_save_form_data_enabled());
	plugin_enabled_set(EINA_FALSE);

	motion_enabled_set(m_preference->get_tilt_zoom_enabled());
	link_mangnifier_enabled_set(EINA_TRUE);
#if !defined(TIZEN_PUBLIC)
#if defined(TEST_CODE)
	if (m_preference->get_demo_mode_enabled()) {
		recording_surface_enabled_set(m_preference->get_recording_surface_enabled());
		layer_borders_enabled_set(m_preference->get_composited_render_layer_borders_enabled());
		inspector_server_enabled_set(m_preference->get_remote_web_inspector_enabled());
		plugin_enabled_set(m_preference->get_plugins_enabled());
	} else {
		recording_surface_enabled_set(EINA_TRUE);
		layer_borders_enabled_set(EINA_FALSE);
	}

#endif /* TEST_CODE */
#endif /* TIZEN_PUBLIC */
	platform_service ps;
	char *lang = ps.get_system_language_set();
	char *region = ps.get_system_region_set();
	char accept_lang_region[6] = {0, };

	if (lang && strlen(lang)) {
		if (region && strlen(region))
			snprintf(accept_lang_region, sizeof(accept_lang_region), "%s-%s", lang, region);
		else
			snprintf(accept_lang_region, strlen(lang), "%s", lang);
	} else
		snprintf(accept_lang_region, strlen("en"), "en");

	if (lang)
		free(lang);

	if (region)
		free(region);

	custom_http_header_set("Accept-Language", accept_lang_region);

	const char *user_agent = NULL;
	if (m_preference->get_desktop_view_enabled())
		user_agent = m_browser->get_user_agent_manager()->get_desktop_user_agent();
	else
		user_agent = m_browser->get_user_agent_manager()->get_user_agent();

	BROWSER_LOGD("user agent = [%s]", user_agent);
	if (user_agent)
		user_agent_set((const char *)user_agent);
}

Evas_Object *webview::copy_snapshot(void)
{
	platform_service ps;
	return ps.copy_evas_image(m_snapshot);
}

void webview::private_browsing_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *setting = ewk_view_settings_get(m_ewk_view);
	ewk_settings_private_browsing_enabled_set(setting, enable);
}

Eina_Bool webview::private_browsing_enabled_get(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, EINA_FALSE);

	Ewk_Settings *setting = ewk_view_settings_get(m_ewk_view);
	return ewk_settings_private_browsing_enabled_get(setting);
}

void webview::font_default_size_set(int size)
{
	BROWSER_LOGD("size = [%d]", size);

	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	Ewk_Settings *setting = ewk_view_settings_get(m_ewk_view);
	ewk_settings_font_default_size_set(setting, size);
}

void webview::find_word(const char *word, Eina_Bool forward, Evas_Smart_Cb found_cb, void *data)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	EINA_SAFETY_ON_NULL_RETURN(word);

	detach_event("text,found", found_cb);
	attach_event("text,found", found_cb, data);

	// If the some events was attached during working time,
	// the webview should be added smart event.
	evas_object_smart_callback_del(m_ewk_view, "text,found", found_cb);
	evas_object_smart_callback_add(m_ewk_view, "text,found", found_cb, data);

	Ewk_Find_Options find_option = (Ewk_Find_Options)(EWK_FIND_OPTIONS_CASE_INSENSITIVE | EWK_FIND_OPTIONS_WRAP_AROUND
			| EWK_FIND_OPTIONS_SHOW_FIND_INDICATOR | EWK_FIND_OPTIONS_SHOW_HIGHLIGHT);

	if (!forward)
		find_option = (Ewk_Find_Options)(find_option | EWK_FIND_OPTIONS_BACKWARDS);

	ewk_view_text_find(m_ewk_view, word, find_option, FIND_WORD_MAX_COUNT);
}

void webview::find_word_clear(void)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	ewk_view_text_find_highlight_clear(m_ewk_view);
}

void webview::execute_script(const char *js_script, Ewk_View_Script_Execute_Callback cb, void *data)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	EINA_SAFETY_ON_NULL_RETURN(js_script);

	ewk_view_script_execute(m_ewk_view, js_script, cb, data);
}

void webview::html_contents_set(const char *html, const char *base_uri)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	EINA_SAFETY_ON_NULL_RETURN(html);

	ewk_view_html_contents_set(m_ewk_view, html, base_uri);
}

void webview::mht_contents_get(Ewk_Page_Contents_Context *context)
{
	EINA_SAFETY_ON_NULL_RETURN(m_ewk_view);
	EINA_SAFETY_ON_NULL_RETURN(context);

	ewk_view_page_contents_get(m_ewk_view, context);
}

void webview::__load_started_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	m_browser->get_browser_view()->get_uri_bar()->update_progress_bar(0.05f);
	m_browser->get_browser_view()->show_uri_input_bar(EINA_FALSE);

	if (m_browser->get_browser_view()->is_show_find_on_page())
		m_browser->get_browser_view()->show_find_on_page(EINA_FALSE);

	webview *wv = (webview *)data;
	wv->reader_enabled_set(EINA_FALSE);
	m_browser->get_browser_view()->get_uri_bar()->show_reader_icon(EINA_FALSE);
}

void webview::__load_committed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	uri_bar *ub = m_browser->get_browser_view()->get_uri_bar();
	ub->set_uri(wv->get_uri());

	wv->give_focus(EINA_FALSE);
}

void webview::__load_error_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Ewk_Error *error = (Ewk_Error *)event_info;
	int error_code = ewk_error_code_get(error);

	if (error_code == EWK_ERROR_NETWORK_STATUS_MALFORMED)
		m_browser->get_browser_view()->show_msg_popup(BR_STRING_LOAD_ERROR_TITLE, BR_STRING_LOAD_ERROR_MSG);
}

void webview::__load_nonempty_layout_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
}

void webview::__title_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
}

void webview::__load_progress_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	m_browser->get_browser_view()->get_uri_bar()->update_progress_bar(wv->get_progress());
}

static Eina_Bool __execute_js_cb(void *data)
{
	browser *br = (browser *)data;
	if (!br->get_browser_view()->get_reader())
		br->get_browser_view()->create_reader();

	br->get_browser_view()->get_reader()->execute_reader_js();

	return ECORE_CALLBACK_CANCEL;
}

void webview::__load_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;

	m_browser->get_browser_view()->get_uri_bar()->update_progress_bar(1.0f);

	if (wv->m_snapshot)
		evas_object_del(wv->m_snapshot);

	int w, h;
	wv->get_geometry(NULL, NULL, &w, &h);
	wv->m_snapshot = wv->capture_snapshot(0, 0, w, w * 0.3, 1.0);

	/* For initial access.
	    Initial Access : If user tries to access a specific webpage after just after launching time,
	    guide to set it as a homepage. */
	m_browsing_count++;

	// If private mode, do not save the history.
	int visit_count = 0;
	if (!wv->private_browsing_enabled_get())
		m_browser->get_history()->save_history(wv->get_title(), wv->get_uri(), wv->m_snapshot, &visit_count);

	if (visit_count >= 2 && m_browsing_count == 2)
		m_browser->get_browser_view()->show_prefered_homepage_confirm_popup();

	m_preference->set_last_visited_uri(wv->get_uri());

	ecore_idler_add(__execute_js_cb, m_browser);
}

static void _kill_browser_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	elm_exit();
}

void webview::__process_crashed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_browser_view()->show_msg_popup(NULL, BR_STRING_WEBPROCESS_CRASH, BR_STRING_OK, _kill_browser_cb);
}

void webview::__create_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;

	if (m_browser->get_webview_list()->get_count() >= BROWSER_WINDOW_MAX_SIZE) {
		*((Evas_Object **)event_info) = NULL;
		m_browser->get_browser_view()->show_msg_popup(NULL, BR_STRING_MAX_WINDOW_WARNING);
		return;
	}

	wv = m_browser->get_webview_list()->create_webview();
	m_browser->get_browser_view()->set_current_webview(wv);
	*((Evas_Object **)event_info) = wv->get_ewk_view();
}

Eina_Bool webview::__close_window_idler_cb(void *data)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	webview *replace_wv = m_browser->get_webview_list()->delete_webview(wv);
	if (replace_wv)
		m_browser->get_browser_view()->set_current_webview(replace_wv);

	return ECORE_CALLBACK_CANCEL;
}

/* The window should be deleted by idler. The webkit recommands to delete by idler. */
void webview::__close_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	ecore_idler_add(__close_window_idler_cb, data);
}

void webview::__icon_received_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
}

void webview::__policy_navigation_decide_cb(void *data, Evas_Object *obj, void *event_info)
{
	webview *wv = (webview *)data;
	Ewk_Policy_Decision *policy_decision = (Ewk_Policy_Decision *)event_info;
	const char *uri = ewk_policy_decision_url_get(policy_decision);
	BROWSER_LOGD("uri = [%s]", uri);

	const char *redirect_uri = wv->m_html5_manager->get_custom_protocol_handler()->get_protocol_from_uri(uri);
	if (redirect_uri) {
		wv->load_uri(redirect_uri);
		ewk_policy_decision_ignore(policy_decision);
		return;
	}

	if (m_browser->get_browser_view()->handle_scheme(uri))
		ewk_policy_decision_ignore(policy_decision);
	else
		ewk_policy_decision_use(policy_decision);
}

// FIXME : base uri may be get from webkit.
static char *_get_base_uri(const char *uri)
{
	if (!uri || !strlen(uri))
		return NULL;

	std::string uri_str = std::string(uri);
	int found = uri_str.rfind("/");
	if (found != std::string::npos) {
		std::string sub_str = uri_str.substr(0, found + 1);
		return strdup(sub_str.c_str());
	}

	return NULL;
}

void webview::__policy_response_decide_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	Ewk_Policy_Decision *policy_decision = (Ewk_Policy_Decision *)event_info;
	Ewk_Policy_Decision_Type policy_type = ewk_policy_decision_type_get(policy_decision);
	const char *uri = ewk_policy_decision_url_get(policy_decision);
	const char *cookie = ewk_policy_decision_cookie_get(policy_decision);
	const char *content_type = ewk_policy_decision_response_mime_get(policy_decision);

	const char *mime = ewk_policy_decision_response_mime_get(policy_decision);

	char *base_uri = _get_base_uri(uri);
	BROWSER_LOGD("base_uri=[%s]", base_uri);
	if (base_uri) {
		const char *redirect_uri = wv->m_html5_manager->get_custom_content_handler()->get_redirect_uri(uri, base_uri, mime);
		free(base_uri);
		BROWSER_LOGD("redirect_uri=[%s]", redirect_uri);
		if (redirect_uri && strlen(redirect_uri)) {
			ewk_policy_decision_ignore(policy_decision);
			wv->load_uri(redirect_uri);
			return;
		}
	}

	switch (policy_type) {
	case EWK_POLICY_DECISION_USE:
		BROWSER_LOGD("policy_use");
		ewk_policy_decision_use(policy_decision);
		break;

	case EWK_POLICY_DECISION_DOWNLOAD:
		BROWSER_LOGD("policy_download");
		ewk_policy_decision_suspend(policy_decision);

		m_browser->get_download_manager()->handle_download_request(uri, cookie, content_type);

		ewk_policy_decision_ignore(policy_decision);
		break;

	case EWK_POLICY_DECISION_IGNORE:
	default:
		BROWSER_LOGD("policy_ignore");
		ewk_policy_decision_ignore(policy_decision);
		break;
	}

	if (policy_type == EWK_POLICY_DECISION_DOWNLOAD) {
		if (!wv->backward_possible())
			ecore_idler_add(__close_window_idler_cb, data);
	}
}

void webview::__form_submit_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
}

void webview::__edge_bottom_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_browser_view()->show_jump_to_top_button(EINA_TRUE);
}

typedef enum {
	TEXT_ONLY = 0,
	INPUT_FIELD,
	TEXT_LINK,
	IMAGE_ONLY,
	IMAGE_LINK,
	UNKNOWN_MENU
} context_menu_type;

typedef enum _custom_context_menu_item_tag {
	CUSTOM_CONTEXT_MENU_ITEM_BASE_TAG = EWK_CONTEXT_MENU_ITEM_BASE_APPLICATION_TAG,
	CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_PAGE,
	CUSTOM_CONTEXT_MENU_ITEM_SHARE,
} custom_context_menu_item_tag;

static context_menu_type _get_menu_type(Ewk_Context_Menu *menu)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(menu, UNKNOWN_MENU);

	int count = ewk_context_menu_item_count(menu);
	Eina_Bool text = EINA_FALSE;
	Eina_Bool link = EINA_FALSE;
	Eina_Bool image = EINA_FALSE;
	for (int i = 0 ; i < count ; i++) {
		Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, i);
		Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_CLIPBOARD)
			return INPUT_FIELD;
		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY)
			text = EINA_TRUE;
		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW)
			link = EINA_TRUE;
		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD)
			image = EINA_TRUE;
	}

	if (text && !link)
		return TEXT_ONLY;
	if (link && !image)
		return TEXT_LINK;
	if (image && !link)
		return IMAGE_ONLY;
	if (image && link)
		return IMAGE_LINK;

	return UNKNOWN_MENU;
}

static void _customize_context_menu(Ewk_Context_Menu *menu)
{
	EINA_SAFETY_ON_NULL_RETURN(menu);

	int count = ewk_context_menu_item_count(menu);
	context_menu_type menu_type = _get_menu_type(menu);

	BROWSER_LOGD("menu_type=%d", menu_type);

	if (menu_type == UNKNOWN_MENU || menu_type == INPUT_FIELD)
		return;

	for (int i = 0 ; i < count ; i++) {
		Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
		Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
		ewk_context_menu_item_remove(menu, item);
	}

	if (menu_type == TEXT_ONLY) {
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY, BR_STRING_CTXMENU_COPY, true);
		ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_PAGE, BR_STRING_FIND_ON_PAGE, true);
	} else if (menu_type == TEXT_LINK) {
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, BR_STRING_CTXMENU_COPY_LINK_URL, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE, BR_STRING_CTXMENU_SELECTION_MODE, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DRAG, BR_STRING_CTXMENU_DRAG, true);
	} else if (menu_type == IMAGE_ONLY) {
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW, BR_STRING_CTXMENU_VIEW_IMAGE, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD, BR_STRING_CTXMENU_COPY_TO_CLIPBOARD, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK, BR_STRING_CTXMENU_SAVE_IMAGE, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE, BR_STRING_CTXMENU_SELECTION_MODE, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DRAG, BR_STRING_CTXMENU_DRAG, true);
	} else if (menu_type == IMAGE_LINK) {
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW, BR_STRING_CTXMENU_VIEW_IMAGE, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD, BR_STRING_CTXMENU_COPY_TO_CLIPBOARD, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK, BR_STRING_CTXMENU_SAVE_IMAGE, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, BR_STRING_CTXMENU_COPY_LINK_URL, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE, BR_STRING_CTXMENU_SELECTION_MODE, true);
		ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DRAG, BR_STRING_CTXMENU_DRAG, true);
	}
}

void webview::__contextmenu_customize_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Ewk_Context_Menu *menu = static_cast<Ewk_Context_Menu*>(event_info);
	_customize_context_menu(menu);
}

void webview::__contextmenu_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	webview *wv = (webview *)data;
	Ewk_Context_Menu_Item* item = static_cast<Ewk_Context_Menu_Item*>(event_info);
	Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
	const char *selected_text = ewk_view_text_selection_text_get(wv->m_ewk_view);
	const char *link_url = ewk_context_menu_item_link_url_get(item);
	const char *image_url = ewk_context_menu_item_image_url_get(item);
	const char *item_text = NULL;

	if (image_url && strlen(image_url))
		item_text = image_url;
	else if (link_url && strlen(link_url))
		item_text = link_url;
	else if (selected_text && strlen(selected_text))
		item_text = selected_text;

	if (tag == CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_PAGE) {
		m_browser->get_browser_view()->show_find_on_page(EINA_TRUE, item_text, wv);
	}
}

