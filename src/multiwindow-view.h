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

#ifndef MULTIWINDOW_VIEW_H
#define MULTIWINDOW_VIEW_H

#include <Elementary.h>
#include <Evas.h>
#include <vector>
#include "browser-object.h"
#include "common-view.h"

#define multiwindow_item_snapshot_w	(630 * efl_scale)
#define multiwindow_item_snapshot_h	(208 * efl_scale)

#define multiwindow_view_edj_path browser_edj_dir"/multiwindow-view.edj"
#define multiwindow_panes_edj_path browser_edj_dir"/multiwindow-panes.edj"

#define FLICK_MOMENTUM_THRESHOLD	1500

class multiwindow_item;
class multiwindow_view : public browser_object, public common_view {
public:
	// If the bookmark view need to be shown, init_bookmark may be EINA_TRUE.
	multiwindow_view(Eina_Bool init_bookmark = EINA_FALSE);
	~multiwindow_view(void);

	void show(void);
	Eina_Bool is_multiwindow_showing(void) { return m_is_multiwindow_showing; }
	// Switch multiwindow view <-> bookmark view
	void change_view(Eina_Bool show_multiwindow_view);
	void close_multiwindow_view(void);
private:
	Evas_Object *_create_gesture_layer(Evas_Object *parent);
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_multiwindow_layout(Evas_Object *parent);
	Evas_Object *_create_toolbar_layout(Evas_Object *parent);
	void _show_close_effect(void);
	void _show_more_context_popup(Evas_Object *parent);
	void _show_open_effect(void);
	void _show_max_effect(void);
	void _show_min_effect(void);
	Eina_Bool _is_effect_running(void);
	Eina_Bool _is_draging(void) { return m_is_draging; }
	Evas_Object *_create_padding_layout(Evas_Object *parent, int index);
	void _show_renew_homepage_confirm_popup(void);
	void _renew_homepage(void);

	static void __renew_homepage_ok_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
	static void __close_all_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __close_effect_animator_cb(void *data);
	static void __handle_press_cb(void *data, Evas_Object *obj, void *event_info);
	static void __handle_unpress_cb(void *data, Evas_Object *obj, void *event_info);
	static void __index_1_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __index_2_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __layout_resize_cb(void* data, Evas* evas, Evas_Object* obj, void* ev);
	static void __more_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __min_effect_animator_cb(void *data);
	static Eina_Bool __max_effect_animator_cb(void *data);
	static Eina_Bool __open_effect_animator_cb(void *data);
	static void __plus_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scroller_edge_top_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scroller_edge_bottom_cb(void *data, Evas_Object *obj, void *event_info);
	static void __transit_move_finished_cb(void *data, Elm_Transit *transit);

	static Evas_Event_Flags __gesture_zoom_move(void *data, void *event_info);
	static Evas_Event_Flags __gesture_zoom_start(void *data, void *event_info);
	static Evas_Event_Flags __gesture_zoom_end(void *data, void *event_info);

	static Evas_Event_Flags __gesture_momentum_start(void *data, void *event_info);
	static Evas_Event_Flags __gesture_momentum_move(void *data, void *event_info);

	static void __hide_padding_layout_finished_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

	std::vector<multiwindow_item *> m_multiwindow_item_list;
	Evas_Object *m_main_layout;
	Evas_Object *m_toolbar_layout;
	Evas_Object *m_gesture_layer;
	Evas_Object *m_panes;
	Evas_Object *m_scroller;
	Evas_Object *m_container_layout;

	Eina_Bool m_scroll_tilting;
	Eina_Bool m_scroll_spring;
	int m_selected_index;

	Ecore_Animator *m_open_effect_animator;
	Ecore_Animator *m_max_effect_animator;
	Ecore_Animator *m_close_effect_animator;
	Ecore_Animator *m_min_effect_animator;

	Eina_Bool m_is_draging;
	double m_prev_size;
	Eina_Bool m_is_multiwindow_showing;
	Eina_Bool m_is_zooming;

	Evas_Object *m_plus_button;

	std::vector<Evas_Object *> m_padding_layout_list;
	double m_handle_rate;
	Evas_Object *m_never_show_checkbox;
	Elm_Transit *m_move_transit;
};

#endif /* MULTIWINDOW_VIEW_H */

