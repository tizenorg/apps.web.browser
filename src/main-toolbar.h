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

#ifndef __MAIN_TOOLBAR_H__
#define __MAIN_TOOLBAR_H__

#include "browser-object.h"

class main_toolbar : public browser_object {
public:
	main_toolbar(void);
	~main_toolbar(void);

	void create_layout(Evas_Object *parent);
	Evas_Object *get_scroller(void) { return m_toolbar_scroller; }
	Evas_Object *get_layout(void) { return m_main_layout; }
	void set_scroll_position(int y);
	void enable_forward_button(Eina_Bool enable);
	void enable_backward_button(Eina_Bool enable);
	void set_focus_on_backward_button(void);
	void set_focus_on_forward_button(void);
	void set_focus_on_home_button(void);
	void set_landscape_mode(Eina_Bool landscape);
	void enable_fixed_toolbar(Eina_Bool enable);
	void enable_toolbar_event(Eina_Bool enable);

	Eina_Bool has_layout() { return m_main_layout ? EINA_TRUE : EINA_FALSE; }
private:
	static void __backward_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __forward_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __home_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __bookmark_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __language_changed_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_main_layout;
	Evas_Object *m_toolbar_scroller;

	// Toolbar buttons.
	Evas_Object *m_backward_button;
	Evas_Object *m_forward_button;
	Evas_Object *m_home_button;
	Evas_Object *m_saved_page_button;
	Evas_Object *m_bookmark_button;
	Eina_Bool m_is_fixed_mode;
};

#endif // __MAIN_TOOLBAR_H__
