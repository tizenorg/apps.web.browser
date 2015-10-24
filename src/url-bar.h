/*
 * Copyright 2014  Samsung Electronics Co., Ltd
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

#ifndef __URL_BAR_H__
#define __URL_BAR_H__

#include "browser-object.h"
#include "more-menu-manager.h"

class url_bar : public browser_object {
public:
	url_bar(Evas_Object *parent);
	~url_bar(void);

	Evas_Object *get_layout(void) { return m_main_layout; }
	void set_tab_count(int count);
	void set_text(const char *text, Eina_Bool update_icon = EINA_TRUE);
	void update_star_icon(void);
	void show_star_icon(Eina_Bool show);
	void show_secure_icon(Eina_Bool show);
	void update_secure_icon(void);
	void update_fav_icon(void);
	void show_fav_icon(Eina_Bool show);
	void show_reload_cancel_icon(Eina_Bool show);
	void set_loading_status(Eina_Bool loading);
	void enable_private_mode(Eina_Bool enable);
	void set_landscape_mode(Eina_Bool landscape);
	void set_fixed_mode(Eina_Bool mode);
	void set_focus_on_text(void);
	void set_highlight_on_text(void);
	void focus_allow_set(Eina_Bool show);
	void enable_tab_manager_button(Eina_Bool enable);
	void language_changed(void);
	more_menu_manager *get_more_menu_manager(void) { return m_more_menu_manager; }
	Eina_Bool is_loading_status(void) { return m_loading; }
private:
	void _create_main_layout(Evas_Object *parent);
	void _create_text_layout(Evas_Object *parent);
	void _show_tab_manager(void);
#if defined(HW_MORE_BACK_KEY)
	static void __back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __menu_clicked_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static void __text_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
	static void __access_item_clicked_cb(void *data, Evas_Object *obj, Elm_Object_Item *item);
	static void __uri_box_key_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __tab_manager_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __star_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __secure_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __reload_cancel_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __font_size_changed_cb(keynode_t *keynode, void *data);
	static Eina_Bool __tab_manager_clicked_idler_cb(void *data);
	static void __access_tab_manager_clicked_cb(void *data, Evas_Object *obj, Elm_Object_Item *item);

	more_menu_manager *m_more_menu_manager;
	Evas_Object *m_main_layout;
	Evas_Object *m_text_layout;
	Evas_Object *m_entry;
	Evas_Object *m_tab_manager_button;
	Evas_Object *m_secure_icon;
	Evas_Object *m_star_icon;
	Evas_Object *m_uri_box_object;
	Evas_Object *m_tab_manager_access;
	Evas_Object *m_reload_cancel_icon;
	Eina_Bool m_fixed_mode;
	Eina_Bool m_loading;
	Eina_Bool m_is_show_star_icon;
};

#endif // __URL_BAR_H__
