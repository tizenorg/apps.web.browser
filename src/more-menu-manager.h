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
 * Contact: Jiwon Lee <jiwonear.lee@samsung.com>
 *
 */

#ifndef MORE_MENU_MANAGER_H
#define MORE_MENU_MANAGER_H

#include <Evas.h>
#include <vconf.h>

#include "browser-object.h"
#include "common-view.h"
#include "webview.h"

typedef enum _uri_bar_more_menu_type
{
	more_menu_normal = 0,
	more_menu_epmty_webview,
	more_menu_unknown
} uri_bar_more_menu_type;


class more_menu_manager : public browser_object {
public:
	more_menu_manager(void);
	~more_menu_manager(void);
	void hide_more_context_popup(void);
	void show_more_menu(void);
	Eina_Bool is_show_more_menu(void);
	void set_private_mode(Eina_Bool enable, Eina_Bool update_history);
	void enable_browser_view_focus(Eina_Bool enable);

private:
	static Eina_Bool __resize_more_ctxpopup_cb(void *data);
	static void __rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_popup_back_cb(void *data, Evas_Object *obj, void *event_info);
	Evas_Object *_create_more_context_popup();
	static void __new_window_cb(void *data, Evas_Object *obj, void *event_info);
#ifdef ENABLE_INCOGNITO_WINDOW
	static void __new_incognito_window_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static void __share_cb(void *data, Evas_Object *obj, void *event_info);
	static void __settings_popup_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_popup_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_on_page_cb(void *data, Evas_Object *obj, void *event_info);
	static void __private_mode_allow_ask_popup_language_changed(void *data, Evas_Object *obj, void *event_info);
	static void __private_mode_allow_no_cb(void *data, Evas_Object *obj, void *event_info);
	static void __private_mode_allow_allow_cb(void *data, Evas_Object *obj, void *event_info);
	void _private_mode_allow_ask_popup(void);
	static void __private_on_off_cb(void *data, Evas_Object *obj, void *event_info);
	static void __setting_cb(void *data, Evas_Object *obj, void *event_info);

	Eina_Bool _append_normal_mode_more_context_popup(Evas_Object *more_popup);
	Eina_Bool _append_empty_page_mode_more_context_popup(Evas_Object *more_popup);
	Eina_Bool _append_more_context_popup(Evas_Object *more_popup, uri_bar_more_menu_type type);
	void _show_more_context_popup();

	Evas_Object *m_more_context_popup;
	Eina_Bool m_is_dismissed_by_back_key;
	Eina_Bool m_is_minimize_popup_shown;
	char *m_scrap_tag;
	Evas_Object *m_private_mode_allow_ask_popup;
	Evas_Object *m_private_mode_allow_ask_check;
};

#endif /* MORE_MENU_MANAGER_H */
