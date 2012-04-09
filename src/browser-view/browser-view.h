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

#ifndef BROWSER_VIEW_H
#define BROWSER_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"
class Browser_Bookmark_DB;
class Browser_Class;
class Browser_Context_Menu;
class Browser_Exscheme_Handler;
class Browser_Find_Word;
class Browser_Multi_Window_View;
class Browser_Personal_Data_Manager;
class Browser_Picker_Handler;
class Browser_Predictive_History;
class Browser_Settings_Class;
class Browser_Window;

/* edit mode state check */
typedef enum _edit_mode {
	BR_NO_EDIT_MODE 	= 0,
	BR_URL_ENTRY_EDIT_MODE,
	BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF,
	BR_FIND_WORD_MODE
} edit_mode;

class Browser_View : public Browser_Common_View {
	friend class Browser_Class;
	friend class Browser_Context_Menu;
	friend class Browser_Find_Word;
	friend class Browser_Multi_Window_View;
	friend class Browser_Predictive_History;
public:
	Browser_View(Evas_Object *win, Evas_Object *navi_bar, Evas_Object *bg,
						Evas_Object *layout, Browser_Class *browser);
	~Browser_View(void);

	Eina_Bool init(void);
	void launch(const char *url);
	void set_focused_window(Browser_Window *window, Eina_Bool show_most_visited_sites = EINA_TRUE);
	Browser_Window *get_focused_window(void) { return m_focused_window; }
	Evas_Object *get_focused_webview(void);
	void load_url(const char *url);
	string get_title(Browser_Window *window);
	string get_title(void);
	string get_url(void);
	string get_url(Browser_Window *window);
	void return_to_browser_view(Eina_Bool saved_most_visited_sites_item = EINA_FALSE);
	void unset_navigationbar_title_object(Eina_Bool is_unset);

	void pause(void);
	void resume(void);
	void reset(void);

	void suspend_webview(Evas_Object *webview);
	void resume_webview(Evas_Object *webview);

	void set_edit_mode(edit_mode mode) { m_edit_mode = mode; }
	Evas_Object *get_favicon(const char *url);
	void delete_non_user_created_windows(void);
	void init_personal_data_manager(Evas_Object *webview);
	void deinit_personal_data_manager(void);
	Browser_Personal_Data_Manager *get_personal_data_manager(void) { return m_personal_data_manager; }

	void stop_and_reload(void);

	typedef struct _html5_video_data {
		const char* path;
		const char* cookie;
	} html5_video_data;
private:
	typedef enum _homepage_mode {
		BR_START_MODE_MOST_VISITED_SITES	= 0,
		BR_START_MODE_RECENTLY_VISITED_SITE,
		BR_START_MODE_CUSTOMIZED_URL,
		BR_START_MODE_EMPTY_PAGE,
		BR_START_MODE_UNKOWN
	} homepage_mode;

	/* ewk view event callback functions. */
	static void __uri_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_started_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_progress_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_nonempty_layout_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __create_webview_cb(void *data, Evas_Object *obj, void *event_info);
	static void __window_close_cb(void *data, Evas_Object *obj, void *event_info);
	static void __html_boundary_reached_cb(void *data, Evas_Object *obj, void *event_info);
	static void __html5_video_request_cb(void *data, Evas_Object *obj, void *event_info);
	static void __vibrator_vibrate_cb(void *data, Evas_Object *obj, void *event_info);
	static void __vibrator_cancel_cb(void *data, Evas_Object *obj, void *event_info);

	/* imf event callback functions */
	static void __url_entry_imf_event_cb(void *data, Ecore_IMF_Context *ctx, int value);

	/* edje object event callback functions */
	static void __url_entry_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
	static void __refresh_button_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

	/* evas event callback functions */
	static void __url_entry_focus_out_cb(void *data, Evas *e, void *event_info);

	/* ewk view evas object event callback functions */
	static void __ewk_view_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *ev);
	static void __ewk_view_mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *ev);
	static void __ewk_view_multi_down_cb(void *data, Evas *evas, Evas_Object *obj, void *ev);
	static void __ewk_view_mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *ev);
	static void __scroller_edge_bottom_cb(void *data, Evas_Object *obj, void *event_info);

	/* idler callback functions */
	static Eina_Bool __webview_layout_resize_idler_cb(void *data);
	static Eina_Bool __close_window_idler_cb(void *data);
	static Eina_Bool __scroller_bring_in_idler_cb(void *data);

	/* evas object smart callback functions */
	static void __ewk_view_edge_top_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ewk_view_scroll_down_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ewk_view_scroll_up_cb(void *data, Evas_Object *obj, void *event_info);

	static void __scoller_resize_cb(void* data, Evas* evas, Evas_Object* obj, void* ev);
	static void __url_layout_mouse_down_cb(void *data, Evas* evas, Evas_Object *obj,
										void *event_info);

	/* elementary event callback functions */
	static void __title_back_button_clicked_cb(void *data , Evas_Object *obj, void *event_info);
	static void __url_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __go_to_bookmark_cb(void *data, Evas_Object *obj, void *event_info);
	static void __backward_cb(void *data, Evas_Object *obj, void *event_info);
	static void __forward_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_to_home_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_bookmark_cb(void *data, Evas_Object *obj, void *event_info);
	static void __multi_window_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scroller_scroll_cb(void *data, Evas_Object *obj, void *event_info);
	static void __download_manager_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_context_popup_dismissed_cb(void *data, Evas_Object *obj,
									void *event_info);
	static void __url_entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __internet_settings_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_cb(void *data, Evas_Object *obj, void *event_info);

	static void __zoom_in_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __zoom_out_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __zoom_button_timeout_cb(void *data);
	Eina_Bool _create_zoom_buttons(void);

	static void __expand_option_header_cb(void *data, Evas_Object *obj, void *event_info);
	static void __share_cb(void *data, Evas_Object *obj, void *event_info);
	static void __option_header_url_layout_mouse_down_cb(void *data, Evas* evas,
								Evas_Object *obj, void *event_info);
	static void __add_to_home_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_to_home_done_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_to_send_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info);
	static void __dim_area_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

	/* elm transit callback functions */
	static void __new_window_transit_finished_cb(void *data, Elm_Transit *transit);

	/* normal member functions */
	Eina_Bool _create_main_layout(void);
	Evas_Object *_create_url_layout(void);
	Evas_Object *_create_option_header_url_layout(void);
	Evas_Object *_create_control_bar(void);
	Eina_Bool _show_more_context_popup(void);
	void _stop_loading(void);
	void _reload(void);
	void _set_navigationbar_title(const char *title);
	void _set_url_entry(const char *url, Eina_Bool set_secrue_icon = EINA_TRUE);
	Eina_Bool _is_option_header_expanded(void);
	void _set_controlbar_back_forward_status(void);
	void _navigationbar_title_clicked(void);
	void _load_start(void);
	void _load_finished(void);
	void _set_multi_window_controlbar_text(int count);
	/* get activated url entry, if the option header is expanded, return url entry in option header */
	Evas_Object *_get_activated_url_entry(void);
	void _set_edit_mode(edit_mode mode);
	edit_mode _get_edit_mode(void) { return m_edit_mode; }
	homepage_mode _get_homepage_mode(void) { return m_homepage_mode; }
	/* set homepage from homepage vconf */
	void _set_homepage_mode(void);
	void _set_secure_icon(void);
	Eina_Bool _set_favicon(void);
	Eina_Bool _call_download_manager(void);
	Eina_Bool _call_internet_settings(void);
	void _pop_other_views(void);
	Eina_Bool _show_new_window_effect(Evas_Object *current_ewk_view,
							Evas_Object *new_ewk_view);
	Eina_Bool _is_loading(void);
	Eina_Bool _search_keyword_from_search_engine(const char *keyword);
	Eina_Bool _call_html5_video_streaming_player(const char *url, const char *cookie);
	void _destroy_more_context_popup(void);
	void _enable_browser_scroller_scroll(void);
	void _enable_webview_scroll(void);
	void _navigationbar_visible_set_signal(Eina_Bool visible);
	void _navigationbar_visible_set(Eina_Bool visible);
	Eina_Bool _navigationbar_visible_get(void);
	Evas_Object *_create_add_to_home_control_bar(void);

	void _jump_to_top(void);
	void _hide_scroller_url_layout(void);
	static Eina_Bool _activate_url_entry_idler_cb(void *data);

	Evas_Object *m_main_layout;
	Evas_Object *m_scroller;
	Evas_Object *m_content_box;
	Evas_Object *m_dummy_loading_progressbar;
	Evas_Object *m_conformant;
	Evas_Object *m_title_back_button;

	/* url layout member variables */
	Evas_Object *m_url_layout;
	Evas_Object *m_url_entry_layout;
	Evas_Object *m_url_edit_field;
	Evas_Object *m_cancel_button;
	Evas_Object *m_url_progressbar;
	Evas_Object *m_url_progresswheel;

	Evas_Object *m_option_header_layout;
	/* url layout which is inserted to navigation bar option header member variables */
	Evas_Object *m_option_header_url_layout;
	Evas_Object *m_option_header_url_entry_layout;
	Evas_Object *m_option_header_url_edit_field;
	Evas_Object *m_option_header_cancel_button;
	Evas_Object *m_option_header_url_progressbar;
	Evas_Object *m_option_header_url_progresswheel;

	/* control bar member variables */
	Evas_Object *m_control_bar;
	Elm_Object_Item *m_backward_button;
	Elm_Object_Item *m_forward_button;
	Elm_Object_Item *m_add_bookmark_button;
	Elm_Object_Item *m_more_button;
	Elm_Object_Item *m_multi_window_button;
	Elm_Object_Item *m_share_controlbar_button;

	Evas_Object *m_add_to_home_control_bar;

	/* state check */
	edit_mode m_edit_mode;
	homepage_mode m_homepage_mode;

	Browser_Window *m_focused_window;
	Eina_Bool m_is_scrolling;
	int m_scroller_region_y;
	Evas_Object *m_more_context_popup;
	Browser_Predictive_History *m_predictive_history;
	Browser_Settings_Class *m_browser_settings;

	Elm_Transit *m_new_window_transit;
	Browser_Window *m_created_new_window;

	std::string m_last_visited_url;

	Elm_Object_Item *m_navi_it;
	Browser_Find_Word *m_find_word;

	Eina_Bool m_is_scroll_up;
	Eina_Bool m_is_multi_touch;

	Browser_Context_Menu *m_context_menu;
	Browser_Exscheme_Handler *m_exscheme_handler;
	Browser_Personal_Data_Manager *m_personal_data_manager;
	Browser_Picker_Handler *m_picker_handler;

	Evas_Object *m_zoom_in_button;
	Evas_Object *m_zoom_out_button;
	Ecore_Timer *m_zoom_button_timer;
};
#endif /* BROWSER_VIEW_H */

