/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
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

#ifndef SETTING_VIEW_H
#define SETTING_VIEW_H

#include <ui-gadget.h>

#include "browser-object.h"
#include "clear-private-data-view.h"
#include "common-view.h"
#include "homepage-edit-view.h"
#include "text-encoding-type-view.h"
#include "website-setting-view.h"
#include "user-agent-view.h"
#if defined(TEST_CODE)
#include "custom-user-agent-view.h"
#endif

class setting_view : public browser_object, public common_view {
public:
	typedef enum _menu_type
	{
		BR_HOMEPAGE_TITLE = 0,
		BR_HOMEPAGE_MENU,
		BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE,
		BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE,
		BR_HOMEPAGE_SUBMENU_EMPTY_PAGE,
		BR_HOMEPAGE_SUBMENU_CURRENT_PAGE,
		BR_CONTENT_TITLE,
		BR_CONTENT_MENU_DEFAULT_VIEW_LEVEL,
		BR_CONTENT_SUBMENU_FIT_TO_WIDTH,
		BR_CONTENT_SUBMENU_READABLE,
		BR_CONTENT_MENU_RUN_JAVASCRIPT,
		BR_CONTENT_MENU_DISPLAY_IMAGES,
		BR_CONTENT_MENU_BLOCK_POPUP,
		BR_CONTENT_MENU_ENCODING_TYPE,
		BR_PRIVACY_TITLE,
		BR_PRIVATE_MENU_CLEAR_CACHE,
		BR_PRIVATE_MENU_CLEAR_HISTORY,
		BR_PRIVATE_MENU_SHOW_SECURITY_WARNINGS,
		BR_PRIVACY_MENU_ACCEPT_COOKIES,
		BR_PRIVATE_MENU_CLEAR_ALL_COOKIE_DATA,
		BR_PRIVACY_MENU_REMEMBER_FORM_DATA,
		BR_PRIVACY_CLEAR_FORM_DATA,
		BR_PRIVACY_MENU_REMEMBER_PASSWORDS,
		BR_PRIVACY_CLEAR_PASSWORDS,
		BR_PRIVACY_WEBSITE_SETTING,
		BR_PRIVACY_SUBMENU_ALWAYS_ASK,
		BR_PRIVACY_SUBMENU_ALWAYS_ON,
		BR_PRIVACY_SUBMENU_ALWAYS_OFF,
		BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA,
		BR_PRIVACY_MENU_ENABLE_LOCATION,
		BR_PRIVACY_MENU_CLEAR_LOCATION_ACCESS,
		BR_MENU_CERTIFICATES,
		BR_MENU_ABOUT_BROWSER,
		BR_MENU_RESET_TO_DEFAULT,
		BR_DEBUG_TITLE,
		BR_MENU_USER_AGENT,
#if defined(TEST_CODE)
		BR_MENU_USER_CUSTOM_USER_AGENT,
#endif
		BR_MENU_UNKNOWN
	} menu_type;

	setting_view(void);
	~setting_view(void);

	void show(void);
	clear_private_data_view *get_clear_private_data_view(void);
	homepage_edit_view *get_homepage_edit_view(void);
	text_encoding_type_view *get_text_encoding_type_view(void);
	website_setting_view *get_website_setting_view(void);
	user_agent_view *get_user_agent_view(void);
#if defined(TEST_CODE)
	custom_user_agent_view *get_custom_user_agent_view(void);
#endif

private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Evas_Object *_create_genlist(Evas_Object *parent);
	void _delete_clear_private_data_view(void);
	void _delete_homepage_edit_view(void);
	void _delete_text_encoding_type_view(void);
	void _delete_website_setting_view(void);
	void _delete_user_agent_view(void);
#if defined(TEST_CODE)
	void _delete_custom_user_agent_view(void);
#endif
	void _reset_to_default_setting(void);
	Eina_Bool _call_user_agent(void);

	static char *__text_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info);

	static void __expandable_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __homepage_sub_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __plugin_sub_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __default_view_level_sub_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __on_off_check_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	/* callback functions for on&off buttons */
	static void __run_javascript_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __show_images_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __block_popup_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __show_security_warnings_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __accept_cookies_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __enable_location_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __auto_save_id_pass_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __auto_save_form_data_check_changed_cb(void *data, Evas_Object *obj, void *event_info);

	/* callback functions for popup choose */
	static void __clear_cache_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_history_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_all_cookie_data_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_form_data_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_passwords_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_location_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __reset_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_response_cb(void *data, Evas_Object *obj, void *event_info);

	/* callback functions for user agent ug action */
	static void __ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv);
	static void __ug_destroy_cb(ui_gadget_h ug, void *priv);

	Elm_Genlist_Item_Class *m_category_title_item_class;
	Elm_Genlist_Item_Class *m_2_text_item_class;
	Elm_Genlist_Item_Class *m_2_text_3_item_class;
	Elm_Genlist_Item_Class *m_2_text_2_item_class;
	Elm_Genlist_Item_Class *m_1_text_1_icon_item_class;
	Elm_Genlist_Item_Class *m_1_text_item_class;
	Elm_Genlist_Item_Class *m_2_text_1_icon_item_class;
	Elm_Genlist_Item_Class *m_radio_text_item_class;
	Elm_Genlist_Item_Class *m_seperator_item_class;
	Elm_Genlist_Item_Class *m_seperator_with_bottom_line_item_class;

	/* Homepage */
	genlist_callback_data m_homepage_title_callback_data;
	genlist_callback_data m_homepage_item_callback_data;
	genlist_callback_data m_recently_visited_item_callback_data;
	genlist_callback_data m_user_homepage_item_callback_data;
	genlist_callback_data m_current_page_item_callback_data;
	genlist_callback_data m_empty_page_item_callback_data;

	/* Display */
	genlist_callback_data m_display_title_callback_data;
	genlist_callback_data m_defailt_view_level_item_callback_data;
	genlist_callback_data m_fit_to_width_item_callback_data;
	genlist_callback_data m_readable_item_callback_data;

	/* Content */
	genlist_callback_data m_content_title_callback_data;
	genlist_callback_data m_run_javascript_item_callback_data;
	genlist_callback_data m_display_images_item_callback_data;
	genlist_callback_data m_block_popup_item_callback_data;

	/* Privacy */
	genlist_callback_data m_privacy_title_callback_data;
	genlist_callback_data m_clear_cache_item_callback_data;
	genlist_callback_data m_clear_history_item_callback_data;
	genlist_callback_data m_show_security_warnings_item_callback_data;
	genlist_callback_data m_accept_cookies_item_callback_data;
	genlist_callback_data m_clear_all_cookies_data_item_callback_data;
	genlist_callback_data m_clear_private_data;
	genlist_callback_data m_clear_form_data_callback_data;
	genlist_callback_data m_auto_save_form_data_callback_data;
#if 0
	genlist_callback_data m_auto_save_always_ask_item_callback_data;
	genlist_callback_data m_auto_save_always_on_item_callback_data;
	genlist_callback_data m_auto_save_always_off_item_callback_data;
#endif
	genlist_callback_data m_clear_passwords_callback_data;
	genlist_callback_data m_encoding_type_callback_data;
	genlist_callback_data m_auto_save_item_callback_data;
	genlist_callback_data m_website_setting_callback_data;
	genlist_callback_data m_clear_private_data_item_callback_data;
	genlist_callback_data m_enable_location_callback_data;
	genlist_callback_data m_clear_location_access_callback_data;

	/* Others */
	genlist_callback_data m_about_browser_item_callback_data;
	genlist_callback_data m_reset_item_callback_data;

	/* Debug */
	genlist_callback_data m_debug_title_callback_data;
	genlist_callback_data m_user_agent_item_callback_data;
#if defined(TEST_CODE)
	genlist_callback_data m_custom_user_agent_item_callback_data;
#endif
	Evas_Object *m_genlist;

	Evas_Object *m_homepage_radio_group;
	Evas_Object *m_default_view_level_radio_group;
	Evas_Object *m_auto_save_id_pass_radio_group;
	Evas_Object *m_run_javascript_check;
	Evas_Object *m_display_images_check;
	Evas_Object *m_block_popup_check;
	Evas_Object *m_clear_cache_confirm_popup;
	Evas_Object *m_clear_passwords_confirm_popup;
	Evas_Object *m_auto_save_id_pass_check;
	Evas_Object *m_clear_form_data_confirm_popup;
	Evas_Object *m_auto_save_form_data_check;
	Evas_Object *m_clear_history_confirm_popup;
	Evas_Object *m_accept_cookies_check;
	Evas_Object *m_clear_all_cookies_data_confirm_popup;
	Evas_Object *m_enable_location_check;
	Evas_Object *m_clear_location_confirm_popup;
	Evas_Object *m_reset_confirm_popup;
	Evas_Object *m_show_security_warnings_check;

	Elm_Object_Item *m_navi_it;
	Elm_Object_Item *m_naviframe_item;
	clear_private_data_view *m_clear_private_data_view;
	text_encoding_type_view *m_text_encoding_type_view;
	website_setting_view *m_website_setting_view;
	homepage_edit_view *m_homepage_edit_view;
	user_agent_view *m_user_agent_view;
#if defined(TEST_CODE)
	custom_user_agent_view *m_custom_user_agent_view;
#endif
};

#endif /* SETTING_VIEW_H */

