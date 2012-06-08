/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#ifndef BROWSER_SETTINGS_MAIN_VIEW_H
#define BROWSER_SETTINGS_MAIN_VIEW_H

#include "browser-config.h"
#include "browser-common-view.h"

class Browser_Settings_Edit_Homepage_View;
class Browser_Settings_Clear_Data_View;
class Browser_Settings_Plugin_View;

class Browser_Settings_Accelerated_Composition;
class Browser_Settings_User_Agent_View;

class Browser_Settings_Main_View : public Browser_Common_View {
public:
	typedef enum _menu_type
	{
		BR_HOMEPAGE_TITLE = 0,
		BR_HOMEPAGE_MENU,
		BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES,
		BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE,
		BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE,
		BR_HOMEPAGE_SUBMENU_EMPTY_PAGE,
		BR_DISPLAY_TITLE = 10,
		BR_DISPLAY_MENU_DEFAULT_VIEW_LEVEL,
		BR_DISPLAY_SUBMENU_FIT_TO_WIDTH,
		BR_DISPLAY_SUBMENU_READABLE,
		BR_DISPLAY_SUBMENU_LANDSCAPE_VIEW,
		BR_CONTENT_TITLE = 20,
		BR_CONTENT_MENU_RUN_JAVASCRIPT,
		BR_CONTENT_MENU_DISPLAY_IMAGES,
		BR_CONTENT_MENU_BLOCK_POPUP,
		BR_PRIVACY_TITLE = 30,
		BR_PRIVACY_MENU_ACCEPT_COOKIES,
		BR_PRIVACY_MENU_AUTOSAVE_ID_PASSWORD,
		BR_PRIVACY_SUBMENU_ALWAYS_ASK,
		BR_PRIVACY_SUBMENU_ALWAYS_ON,
		BR_PRIVACY_SUBMENU_ALWAYS_OFF,
		BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA,
		BR_STORAGE_TITLE = 40,
		BR_STORAGE_MENU_DEFAULT_STORAGE,
		BR_STORAGE_SUBMENU_PHONE,
		BR_STORAGE_SUBMENU_MEMORY_CARD,
		BR_SEARCH_TITLE = 50,
		BR_SEARCH_MENU_SEARCH_ENGINE,
		BR_SEARCH_SUBMENU_GOOGLE,
		BR_SEARCH_SUBMENU_YAHOO,
		BR_SEARCH_SUBMENU_NAVER,
		BR_SEARCH_MENU_CASE_SENSITIVE,
		BR_MENU_READER = 60,
		BR_MENU_PLUGINS,
		BR_MENU_CERTIFICATES,
		BR_MENU_ABOUT_BROWSER,
		BR_MENU_RESET_TO_DEFAULT,
		BR_DEBUG_TITLE = 70,
		BR_MENU_DEBUG_MODE,
		BR_MENU_USER_AGENT,
		BR_MENU_ACCELERATED_COMPOSITION,
		BR_MENU_UNKNOWN
	}menu_type;

	Browser_Settings_Main_View(void);
	~Browser_Settings_Main_View(void);

	Eina_Bool init(void);
	Evas_Object *get_genlist(void) { return m_genlist; }
private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Eina_Bool _create_main_layout(void);
	Evas_Object *_create_content_genlist(void);
	Eina_Bool _call_user_agent(void);
	Eina_Bool _show_reset_confirm_popup(void);
	void _reset_settings(void);

	/* genlist callback functions */
	static Evas_Object *__genlist_icon_get(void *data, Evas_Object *obj, const char *part);
	static char *__genlist_label_get(void *data, Evas_Object *obj, const char *part);

	/* vconf changed callback functions */
	static void __mmc_key_changed_cb(keynode_t *keynode, void *data);

	/* elementary event callback functions */
	static void __back_button_clicked_cb(void *data, Evas_Object* obj,
								void* event_info);
	static void __expandable_icon_clicked_cb(void *data, Evas_Object *obj,
								void *event_info);
	static void __on_off_check_clicked_cb(void *data, Evas_Object *obj,
								void *event_info);
	static void __homepage_sub_item_clicked_cb(void *data, Evas_Object *obj,
								void *event_info);
	static void __default_view_level_sub_item_clicked_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __auto_save_id_pass_sub_item_clicked_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __default_storage_sub_item_clicked_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __run_javascript_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __display_images_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __block_popup_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __accept_cookies_check_changed_cb(void *data,
						Evas_Object *obj, void *event_info);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __reset_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info);

	Evas_Object *m_genlist;
	Evas_Object *m_back_button;
	Elm_Genlist_Item_Class m_category_title_item_class;
	Elm_Genlist_Item_Class m_2_text_item_class;
	Elm_Genlist_Item_Class m_1_text_1_icon_item_class;
	Elm_Genlist_Item_Class m_1_text_item_class;
	Elm_Genlist_Item_Class m_radio_text_item_class;
	Elm_Genlist_Item_Class m_seperator_item_class;

	/* Homepage */
	genlist_callback_data m_homepage_title_callback_data;
	genlist_callback_data m_homepage_item_callback_data;
	genlist_callback_data m_most_visited_item_callback_data;
	genlist_callback_data m_recently_visited_item_callback_data;
	genlist_callback_data m_user_homepage_item_callback_data;
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
	genlist_callback_data m_accept_cookies_item_callback_data;
	genlist_callback_data m_auto_save_item_callback_data;
	genlist_callback_data m_auto_save_always_ask_item_callback_data;
	genlist_callback_data m_auto_save_always_on_item_callback_data;
	genlist_callback_data m_auto_save_always_off_item_callback_data;
	genlist_callback_data m_clear_private_data_item_callback_data;

	/* Storage */
	genlist_callback_data m_storage_title_callback_data;
	genlist_callback_data m_default_storage_item_callback_data;
	genlist_callback_data m_default_storage_phone_item_callback_data;
	genlist_callback_data m_default_storage_mmc_item_callback_data;

	/* Others */
	genlist_callback_data m_plugins_item_callback_data;
	genlist_callback_data m_reset_item_callback_data;

	/* Debug */
	genlist_callback_data m_debug_title_callback_data;
	genlist_callback_data m_user_agent_item_callback_data;
	genlist_callback_data m_accelerated_composition_item_callback_data;
	Evas_Object *m_homepage_radio_group;
	Evas_Object *m_default_view_level_radio_group;
	Evas_Object *m_auto_save_id_pass_radio_group;
	Evas_Object *m_default_storage_radio_group;
	Evas_Object *m_default_storage_mmc_radio_button;
	Evas_Object *m_run_javascript_check;
	Evas_Object *m_display_images_check;
	Evas_Object *m_block_popup_check;
	Evas_Object *m_accept_cookies_check;

	Evas_Object *m_reset_confirm_popup;
	Elm_Object_Item *m_navi_it;

	Browser_Settings_Edit_Homepage_View *m_edit_homepage_view;
	Browser_Settings_Clear_Data_View *m_clear_data_view;
	Browser_Settings_Plugin_View *m_plugin_view;
	Browser_Settings_Accelerated_Composition *m_accelerated_composition_view;

	Browser_Settings_User_Agent_View *m_user_agent_view;
};

#endif /* BROWSER_SETTINGS_MAIN_VIEW_H */

