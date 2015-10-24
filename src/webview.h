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
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 */

#ifndef WEBVIEW_H
#define WEBVIEW_H

#define BUILDING_EFL__
#ifdef WEBKIT_EFL
#include <EWebKit_internal.h>
#include <EWebKit.h>
#else
#include <ewk_chromium.h>
#endif

#include <Evas.h>
#include <libsoup/soup.h>

#include "browser-object.h"
#include "certificate-manager.h"

class html5_feature_manager;
class certificate_manager;

typedef enum _context_menu_type {
	TEXT_ONLY = 0,
	INPUT_FIELD,
	TEXT_LINK,
	IMAGE_ONLY,
	IMAGE_LINK,
	EMAIL_SCHEME,
	TEL_SCHEME,
	TEXT_IMAGE_LINK,
	UNKNOWN_MENU
} context_menu_type;

typedef enum _custom_context_menu_item_tag {
	CUSTOM_CONTEXT_MENU_ITEM_BASE_TAG = EWK_CONTEXT_MENU_ITEM_BASE_APPLICATION_TAG,
	CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_PAGE,
	CUSTOM_CONTEXT_MENU_ITEM_SHARE,
	CUSTOM_CONTEXT_MENU_ITEM_SEARCH,
	CUSTOM_CONTEXT_MENU_ITEM_SAVE_TO_KEEPIT,
	CUSTOM_CONTEXT_MENU_ITEM_CALL,
	CUSTOM_CONTEXT_MENU_ITEM_SEND_MESSAGE,
	CUSTOM_CONTEXT_MENU_ITEM_SEND_EMAIL,
	CUSTOM_CONTEXT_MENU_ITEM_SEND_ADD_TO_CONTACT,
} custom_context_menu_item_tag;

typedef struct _reader_info {
	char *title;
	char *image_url;
	char *description;
	const char *html;
} reader_info;

struct permission_info {
	Evas_Object *popup;
	Ewk_User_Media_Permission_Request *permission_request;
};

// Ratio of thumbnail.
#define THUMBNAIL_WIDTH 10
#define THUMBNAIL_HEIGHT 9

const int HTTP_NO_CONTENT = 204;
const char *const EMPTY_URL = "about:blank";

class webview_context : public browser_object {
public:

	static webview_context* instance()
	{
		if (m_instance == NULL)
		{
			m_instance = new (std::nothrow) webview_context();
		}
		return m_instance;
	}
	static void cleanup()
	{
		delete m_instance;
	}

	~webview_context(void);

	void network_session_changed(void);
	Evas_Object *get_favicon(const char *uri);
	void clear_memory_cache(void);
	void clear_cookies(void);
	void clear_form_data(void);
	void clear_saved_ID_and_PW(void);
	void clear_private_data(void);
	void vibration_callbacks_set(Ewk_Vibration_Client_Vibrate_Cb vibrate,
		Ewk_Vibration_Client_Vibration_Cancel_Cb cancel, void *data);
private:
	webview_context(void);
	static webview_context *m_instance;
	Ewk_Context *m_context;
};

class webview : public browser_object {
public:

	/* after create the webview, to receive smart event like "load,started",  you should call activate() */
#ifdef ENABLE_INCOGNITO_WINDOW
	webview(Eina_Bool user_created = EINA_FALSE, Eina_Bool is_incognito = EINA_FALSE);
#else
	webview(Eina_Bool user_created = EINA_FALSE);
#endif
	webview(const char *title, const char *uri, Eina_Bool incognito, int sync_id, Eina_Bool create_ewk_view = EINA_FALSE);

	~webview(void);
	void delete_all_idler(void);

	Evas_Object *get_ewk_view(void) { return m_ewk_view; }
	void delete_ewk_view(void);
	void orientation_send(int degree);
	void load_uri(const char *uri);
	void load_low_mem_uri(const char *reload_uri);
	void load_stop(void);
	void reload(void);
	const char *get_uri(void);
	const char *get_title(void);
	void update_uri(void);
	void update_title(void);
	double get_progress(void);
	Evas_Object *get_favicon(void);
	Eina_Bool is_loading(void);
	void backward(void);
	void forward(void);
	Eina_Bool backward_possible(void);
	Eina_Bool forward_possible(void);
	void suspend(void);
	void resume(void);
	void set_page_visibility_state(Eina_Bool visible);

	/* after create the webview, to receive smart event like "load,started",  you should call activate() */
	void activate(void);
	void deactivate(void);

	void attach_event(const char *event, Evas_Smart_Cb func, const void *data);
	void detach_event(const char *event, Evas_Smart_Cb func);

	void find_word(const char *word, Eina_Bool forward, Evas_Smart_Cb found_cb, void *data);
	void uses_keypad_without_user_action_set(Eina_Bool enabled);
	void auto_fitting_enabled_set(Eina_Bool enable);
	void javascript_enabled_set(Eina_Bool enable);
	void auto_load_images_enabled_set(Eina_Bool enable);
	void auto_fill_forms_set(Eina_Bool enabled);
	void save_ID_and_PW_enabled_set(Eina_Bool enable);
	void remember_form_data_enabled_set(Eina_Bool enable);
	void user_agent_set(const char *user_agent);
	const char *user_agent_get(void);
	Eina_Bool is_user_created(void) const { return m_is_user_created; }
	void set_user_created_flag(Eina_Bool user_created) { m_is_user_created = user_created; }
	void get_geometry(int *x, int *y, int *w, int *h);
	/* The return value of Evas_Object should be freed by caller. */
	void private_browsing_enabled_set(Eina_Bool enable);
	Eina_Bool private_browsing_enabled_get(void);
	void motion_enabled_set(Eina_Bool enable);
	void scroll_position_get(int *x, int *y);
	void scroll_position_set(int x, int y);
	double scale_get(void);
	void content_size_get(int *w, int *h);
	// FIXME : Workaround. The webview can't control the focus with other elementaries.
	// case: webview invokes ime -> app pause -> app resume, then the ime of webview doesn't invoke again.
	Eina_Bool has_focus(void) { return m_has_focus; }
	void give_focus(Eina_Bool focus = EINA_TRUE);
	void show_certificate_status_popup(Eina_Bool ask);
	void enable_customize_contextmenu(Eina_Bool enable);
	void sync_id_set(int id) { m_sync_id = id; }
	int sync_id_get(void) { return m_sync_id; }
	void set_parent_webview(webview *parent_wv) { m_parent_webview = parent_wv; }
	webview *get_parent_webview(void) { return m_parent_webview; }
	const char *get_init_url(void) { return m_init_url; }
	const char *get_url_for_bookmark_update(void) { return m_url_for_bookmark_update; }
	void exit_fullscreen_mode(void);
	Eina_Bool is_data_scheme(void) { return m_is_data_scheme; }
	Eina_Bool is_file_scheme(void) { return m_is_file_scheme; }
	int get_modified(void);
	void set_request_uri(const char *uri);
	char *get_request_uri(void);
	Eina_Bool is_suspended(void) { return m_is_suspended; }
	unsigned int get_navigation_count(void) { return m_navigation_count; }
	Eina_Bool get_text_selection_mode_clear(void) { return ewk_view_text_selection_clear(m_ewk_view); }
	Eina_Bool is_empty_page_check(void);
	void reset_ewk_view(void);
	Eina_Bool is_error_page(void) { return m_is_error_page; }
	Eina_Bool get_url_bar_shown(void) { return m_is_url_bar_shown; }
	void set_url_bar_shown(Eina_Bool shown);
	void set_requested_webview(Eina_Bool requested) { m_requested_webview = requested; }
	Eina_Bool get_requested_webview(void) { return m_requested_webview; }
	void accept_cookie_enabled_set(Evas_Object *ewk_view, Eina_Bool enable);
	void clear_cookies(Evas_Object *ewk_view);
#ifdef ENABLE_INCOGNITO_WINDOW
	Eina_Bool is_incognito(void) const { return m_incognito; };
#endif
//#if defined(BROWSER_TIZEN_LITE)
  certificate_manager* get_certificate_manager(void) { return m_certi_manager; };
//#endif

private:
	void _set_init_url(const char *url);
	void _init_settings(void);
	void update_settings(void);
	void _create_webview_managers(void);
	void _delete_webview_managers(void);
	context_menu_type _get_menu_type(Ewk_Context_Menu *menu);
	void _customize_context_menu(Ewk_Context_Menu *menu);
	void _set_url_for_bookmark_update(const char *url);
	void _register_alive_callback(void);
	void _unregister_alive_callback(void);
	void _create_ewk_view(void);
	void _init_bookmark_thumbnail(void);
	void _set_bookmark_favicon(void *data, int bookmark_id, Eina_Bool is_update_URI);
	void _set_bookmark_thumbnail(void *data, int bookmark_id, Eina_Bool is_update_URI);
	void _set_bookmark_webicon(void *data, int bookmark_id, Eina_Bool is_update_URI);
	void _show_context_menu_text_only(Ewk_Context_Menu *menu);
	void _show_context_menu_text_link(Ewk_Context_Menu *menu);
	void _show_context_menu_image_only(Ewk_Context_Menu *menu);
	void _show_context_menu_image_link(Ewk_Context_Menu *menu);
	void _show_context_menu_email_address(Ewk_Context_Menu *menu);
	void _show_context_menu_call_number(Ewk_Context_Menu *menu);
	void _show_context_menu_text_image_link(Ewk_Context_Menu *menu);

	static void __load_started_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __load_started_timer_cb(void *data);
	static void __load_committed_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __load_committed_timer_cb(void *data);
	static void __load_error_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_nonempty_layout_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __url_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_progress_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __load_progress_timer_cb(void *data);
	static void __load_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __load_finished_timer_cb(void *data);
	static void __process_crashed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __create_window_cb(void *data, Evas_Object *obj, void *event_info);
	static void __close_window_cb(void *data, Evas_Object *obj, void *event_info);
	static void __icon_received_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __icon_received_timer_cb(void *data);
	static void __policy_navigation_decide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __policy_response_decide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __form_submit_cb(void *data, Evas_Object *obj, void *event_info);
	static void __contextmenu_customize_cb(void *data, Evas_Object *obj, void *event_info);
	static void __contextmenu_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_forward_list_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __mouse_down_cb(void *data, Evas* evas, Evas_Object *obj, void *event_info);
	static void __urlbar_scroll_cb(void *data, Evas_Object *obj, void *event_info);
	static void __flick_canceled_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __hide_progress_cb(void *data);
	static void __rotate_prepared_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __close_window_idler_cb(void *data);
	static Eina_Bool __finish_timer_idler_cb(void *data);
	static Eina_Bool __sync_webview_cb(void *data);
	static void __download_website_icon_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data);
	Eina_Bool _download_website_icon(const char *icon_data);
	static void __set_certificate_data_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __delete_webview_idler_cb(void *data);
    static void __fullscreen_enter_cb(void *data, Evas_Object *obj, void *event_info);
    static void __fullscreen_exit_cb(void *data, Evas_Object *obj, void *event_info);
    static void __download_request_cb(const char *download_uri, void *data);
	static void __popup_close(webview *wv);
	static void __usermedia_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __usermedia_ok_cb(void *data, Evas_Object *obj, void *event_info);
	static void __usermedia_permission_request_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_ewk_view;
	Eina_List *m_event_callbacks;

	Eina_Bool m_already_activated;
	Eina_Bool m_is_user_created;

	html5_feature_manager *m_html5_manager;
	certificate_manager *m_certi_manager;

	Evas_Object *m_snapshot;
	Evas_Object *m_thumbnail;

	Evas_Object *m_loading_error_confirm_popup;

	const char *m_uri;
	const char *m_title;
	// FIXME : Workaround. The webview can't control the focus with other elementaries.
	// case: webview invokes ime -> app pause -> app resume, then the ime of webview doesn't invoke again.
	Eina_Bool m_has_focus;
	int m_sync_id;
	Eina_Bool m_incognito;
	webview *m_parent_webview;
	Ecore_Timer *m_load_finish_timer;
	const char *m_init_url;
	char *m_request_uri;
	unsigned int m_first_navigator_count;
	const char *m_url_for_bookmark_update;
	Eina_Bool m_is_data_scheme;
	Eina_Bool m_is_file_scheme;

	Eina_Bool m_is_backward;
	double m_progress;
	Eina_Bool m_is_download_url;
	unsigned int m_navigation_count;
	Eina_Bool m_is_error_page;
	Evas_Object *m_webicon;
	Eina_Bool m_is_suspended;
	Ecore_Idler *m_sync_webview_idler;
	Ecore_Idler *m_load_started_idler;
	Ecore_Idler *m_load_committed_idler;
	Ecore_Timer *m_load_finished_idler;
	Eina_Bool m_is_url_bar_shown;
	permission_info m_permission_info;
	int m_new_window_count;
	int m_current_orientation;
	int m_status_code;

	Eina_Bool m_requested_webview;
};

#endif /* WEBVIEW_H */
