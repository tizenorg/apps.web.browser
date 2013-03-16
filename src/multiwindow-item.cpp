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

#include "multiwindow-item.h"

#include <Elementary.h>
#include "browser-dlog.h"
#include "multiwindow-view.h"

multiwindow_item::multiwindow_item(Evas_Object *snapshot, const char *title, const char *uri, Evas_Smart_Cb clicked_cb, Evas_Smart_Cb delete_cb, void *data)
:
	m_is_shrink(EINA_FALSE)
	,m_layout(NULL)
	,m_snapshot(snapshot)
{
	BROWSER_LOGD("title = [%s], uri = [%s]", title, uri);
	m_title = eina_stringshare_add(title);
	m_uri = eina_stringshare_add(uri);

	m_clicked_cb = clicked_cb;
	m_delete_cb = delete_cb;
	m_cb_data = data;

	m_layout = _create_layout(m_window);
}

multiwindow_item::~multiwindow_item(void)
{
	BROWSER_LOGD("");

	if (m_layout) {
		elm_object_part_content_unset(m_layout, "elm.swallow.snapshot");
		if (m_snapshot)
			evas_object_hide(m_snapshot);
		evas_object_del(m_layout);
	}

	eina_stringshare_del(m_title);
	eina_stringshare_del(m_uri);
}

void multiwindow_item::shrink_layout(Eina_Bool shrink)
{
	BROWSER_LOGD("shrink=[%d]", shrink);
	m_is_shrink = shrink;

	if (shrink) {
		edje_object_signal_emit(elm_layout_edje_get(m_layout), "shrink,layout,signal", "");
	}
#if !defined(MULTIWINDOW_PINCH_EFFECT_DISABLE)
	else {
		edje_object_signal_emit(elm_layout_edje_get(m_layout), "expand,layout,signal", "");
	}
#endif
}

void multiwindow_item::show_tilting_effect(void)
{
	if (!is_shrink())
		edje_object_signal_emit(elm_layout_edje_get(m_layout), "tilting,snapshot,signal", "");
}

void multiwindow_item::show_spring_effect(void)
{
	if (!is_shrink())
		edje_object_signal_emit(elm_layout_edje_get(m_layout), "spring,snapshot,signal", "");
}

void multiwindow_item::show_delete_button(Eina_Bool show)
{
	BROWSER_LOGD("show=[%d]", show);
	if (show)
		edje_object_signal_emit(elm_layout_edje_get(m_layout), "show,delete_button,signal", "");
	else
		edje_object_signal_emit(elm_layout_edje_get(m_layout), "hide,delete_button,signal", "");
}

void multiwindow_item::__item_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	multiwindow_item *mi = (multiwindow_item *)data;
	mi->m_clicked_cb(mi->m_cb_data, obj, mi);
}

void multiwindow_item::__delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	multiwindow_item *mi = (multiwindow_item *)data;
	mi->m_delete_cb(mi->m_cb_data, obj, mi);
}

Evas_Object *multiwindow_item::_create_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, multiwindow_view_edj_path, "multiwindow-item");

	evas_object_size_hint_weight_set(layout, 0.0, 0.0);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(layout, "elm.swallow.snapshot", m_snapshot);
	edje_object_part_text_set(elm_layout_edje_get(layout), "title_text", m_title);

	Evas_Object *delete_button = elm_button_add(layout);
	if (!delete_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(delete_button, "multiwindow_item/delete");
	evas_object_smart_callback_add(delete_button, "clicked", __delete_cb, this);

	elm_object_part_content_set(layout, "elm.swallow.delete_button", delete_button);

	edje_object_signal_callback_add(elm_layout_edje_get(layout), "mouse,clicked,1", "title_bg", __item_clicked_cb, this);
	edje_object_signal_callback_add(elm_layout_edje_get(layout), "mouse,clicked,1", "elm.swallow.snapshot", __item_clicked_cb, this);

	return layout;
}
