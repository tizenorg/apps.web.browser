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

#ifndef __MAIN_VIEW_H__
#define __MAIN_VIEW_H__

#include "browser-object.h"
#include "main-toolbar.h"

typedef enum _main_view_scroll_mode {
	CONTENT_VIEW_SCROLL_ENABLE, // Entire screen
	CONTENT_VIEW_SCROLL_FIXED, // Entire screen - URL bar
	CONTENT_VIEW_SCROLL_FIXED_TOOLBAR // Entire screen - URL bar - Tool bar
} main_view_scroll_mode;

class main_view : public browser_object {
public:
	main_view(void);
	~main_view(void);

	void create_layout(Evas_Object *parent);
	Evas_Object *get_layout(void) { return m_main_layout; }
	void set_scroll_content(Evas_Object *scroll_content);
	void resize_scroll_content(void);
	void set_landscape_mode(Eina_Bool landscape);
	void set_scroll_mode(main_view_scroll_mode mode);
	void set_toolbar_scroller(void);
	main_view_scroll_mode get_scroll_mode(void) { return m_scroll_mode; }
	int get_scroll_position(void);
	void set_scroll_position(int new_y);
	void set_scroll_position_delta(int delta);
	void set_scroll_lock(Eina_Bool lock);
	void show_url_bar_animator(Eina_Bool show);
	void show_toolbar(Eina_Bool show);
	void show_url_bar(Eina_Bool show);
	Eina_Bool is_show_url_bar(void);
	Eina_Bool is_hide_url_bar(void);
	main_toolbar *get_main_toolbar(void) { return m_main_toolbar; }
	void set_is_flicked(Eina_Bool flicked) { m_is_flicked = flicked; }
	void deregister_callbacks(void);
	void register_callbacks(void);
	Evas_Object *get_main_layout(void) { return m_main_layout; }
	Evas_Object *get_main_scroller(void) { return m_scroller; }
private:
	void _set_toolbar_scroll_position(void);
	void _handle_mouse_down(int touch_y);
	void _handle_mouse_up(int touch_y);

	static Eina_Bool __resize_scroll_content_cb(void *data);
	static void __mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __multi_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __multi_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static Eina_Bool __move_url_bar_cb(void *data);
	static void __scroller_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
	static void __scroller_edge_top_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scroller_edge_bottom_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __show_url_bar_animator_cb(void *data);
	static Eina_Bool __hide_url_bar_animator_cb(void *data);
	static Eina_Bool __set_toolbar_scroll_position_cb(void *data);
	static void __delayed_toolbar_creation_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);

	Evas_Object *m_main_layout;
	Evas_Object *m_scroller;
	main_toolbar *m_main_toolbar;
	bool m_is_touched;
	Eina_Bool m_scroll_lock;
	Ecore_Animator *m_url_bar_animator;
	int m_last_touch_y;
	int m_toolbar_height;
	main_view_scroll_mode m_scroll_mode;
	Eina_Bool m_is_flicked;
};

#endif // __MAIN_VIEW_H__
