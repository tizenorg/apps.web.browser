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

#ifndef WEBVIEW_H
#define WEBVIEW_H

#define BUILDING_EFL__
#include <EWebKit2.h>

#include <Evas.h>

#include "browser-object.h"

class html5_feature_manager;
class authentication_manager;
class certificate_manager;

class webview_context : public browser_object {
public:
	webview_context(void);
	~webview_context(void);

	void set_proxy_address(const char *proxy);
	const char *get_proxy_address(void);
	void accept_cookie_enabled_set(Eina_Bool enable);
	void clear_cookies(void);
	Evas_Object *get_favicon(const char *uri);
	void clear_memory_cache(void);
	void clear_form_data(void);
	void clear_saved_ID_and_PW(void);
	void notify_low_memory(void);
	void clear_private_data(void);
	void vibration_callbacks_set(Ewk_Vibration_Client_Vibrate_Cb vibrate, Ewk_Vibration_Client_Vibration_Cancel_Cb cancel, void *data);
#if defined(TEST_CODE)
	void memory_sampler_enabled_set(Eina_Bool enable);
	void memory_saving_mode_set(Eina_Bool enable);
#endif
private:
	void _init_setting(void);

	static void __download_did_start_cb(const char *download_uri, void *data);

	Ewk_Context *m_context;
};

class webview : public browser_object {
public:
	/* after create the webview, to receive smart event like "load,started",  you should call activate() */
	webview(Eina_Bool user_created = EINA_FALSE);
	~webview(void);

	Evas_Object *get_ewk_view(void) { return m_ewk_view; }
	void delete_ewk_view(void);
	void orientation_send(int degree);
	void load_uri(const char *uri);
	void load_stop(void);
	void reload(void);
	const char *get_uri(void);
	const char *get_title(void);
	double get_progress(void);
	Evas_Object *get_favicon(void);
	Eina_Bool is_loading(void);
	void backward(void);
	void forward(void);
	Eina_Bool backward_possible(void);
	Eina_Bool forward_possible(void);
	void suspend(void);
	void resume(void);
	Evas_Object *capture_snapshot(int x, int y, int w, int h, float scale);
	/* after create the webview, to receive smart event like "load,started",  you should call activate() */
	void activate(void);
	void deactivate(void);
	void replace_ewk_view(Evas_Object *ewk_view);

	void attach_event(const char *event, Evas_Smart_Cb func, const void *data);
	void detach_event(const char *event, Evas_Smart_Cb func);

	void find_word(const char *word, Eina_Bool forward, Evas_Smart_Cb found_cb, void *data);
	void find_word_clear(void);

	void auto_fitting_enabled_set(Eina_Bool enable);
	void javascript_enabled_set(Eina_Bool enable);
	void auto_load_images_enabled_set(Eina_Bool enable);
	void scripts_window_open_enabled_set(Eina_Bool enable);
	void text_encoding_type_set(const char *encoding_type);
	void save_ID_and_PW_enabled_set(Eina_Bool enable);
	void remember_form_data_enabled_set(Eina_Bool enable);
	void user_agent_set(const char *user_agent);
	const char *user_agent_get(void);
	void ime_autofocus_set(Eina_Bool enable);
	void custom_http_header_set(const char *name, const char *value);

	Eina_Bool save_as_pdf(int w, int h, const char *pdf_file_path);
	Eina_Bool is_user_created(void) { return m_is_user_created; }
	void set_user_created_flag(Eina_Bool user_created) { m_is_user_created = user_created; }
	void get_geometry(int *x, int *y, int *w, int *h);
	/* The return value of Evas_Object should be freed by caller. */
	Evas_Object *copy_snapshot(void);
	Evas_Object *get_snapshot(void) { return m_snapshot; }
	void private_browsing_enabled_set(Eina_Bool enable);
	Eina_Bool private_browsing_enabled_get(void);
	void plugin_enabled_set(Eina_Bool enable);
	void execute_script(const char *js_script, Ewk_View_Script_Execute_Callback cb, void *data);
	void html_contents_set(const char *html, const char *base_uri);
	Eina_Bool reader_enabled_get(void) { return m_reader_enabled; }
	void reader_enabled_set(Eina_Bool enabled) { m_reader_enabled = enabled; }
	void font_default_size_set(int size);
	void motion_enabled_set(Eina_Bool enable);
	void link_mangnifier_enabled_set(Eina_Bool enable);
	void scroll_position_get(int *x, int *y);
	void scroll_position_set(int x, int y);
	void scroll_size_get(int *w, int *h);
	double scale_get(void);
	void content_size_get(int *w, int *h);
	// FIXME : Workaround. The webview can't control the focus with other elementaries.
	// case: webview invokes ime -> app pause -> app resume, then the ime of webview doesn't invoke again.
	Eina_Bool has_focus(void) { return m_has_focus; }
	void give_focus(Eina_Bool focus = EINA_TRUE) { evas_object_focus_set(m_ewk_view, focus); }
	void enable_customize_contextmenu(Eina_Bool enable);
	void mht_contents_get(Ewk_View_MHTML_Data_Get_Callback callback, void *data);
#if defined(TEST_CODE)
	void inspector_server_enabled_set(Eina_Bool enable);
	void recording_surface_enabled_set(Eina_Bool enable);
	void layer_borders_enabled_set(Eina_Bool enable);
#endif /* TEST_CODE */
private:
	void _init_setting(void);

	static void __load_started_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_committed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_error_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_nonempty_layout_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_progress_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __process_crashed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __create_window_cb(void *data, Evas_Object *obj, void *event_info);
	static void __close_window_cb(void *data, Evas_Object *obj, void *event_info);
	static void __icon_received_cb(void *data, Evas_Object *obj, void *event_info);
	static void __policy_navigation_decide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __policy_response_decide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __form_submit_cb(void *data, Evas_Object *obj, void *event_info);
	static void __request_certi_cb(void *data, Evas_Object *obj, void *event_info);
	static void __contextmenu_customize_cb(void *data, Evas_Object *obj, void *event_info);
	static void __contextmenu_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edge_bottom_cb(void *data, Evas_Object *obj, void *event_info);

	static Eina_Bool __close_window_idler_cb(void *data);

	Evas_Object *m_ewk_view;
	Eina_List *m_event_callbacks;

	Eina_Bool m_already_activated;
	Eina_Bool m_is_user_created;
	html5_feature_manager *m_html5_manager;
	authentication_manager *m_auth_manager;
	certificate_manager *m_certi_manager;

	Evas_Object *m_snapshot;

	const char *m_uri;
	const char *m_title;
	Eina_Bool m_reader_enabled;
	// FIXME : Workaround. The webview can't control the focus with other elementaries.
	// case: webview invokes ime -> app pause -> app resume, then the ime of webview doesn't invoke again.
	Eina_Bool m_has_focus;
};

#endif /* WEBVIEW_H */

