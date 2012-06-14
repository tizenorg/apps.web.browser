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

extern "C" {
#include <Elementary.h>
}

#include "browser-utility.h"

static void __changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	void *erase = evas_object_data_get(layout, "eraser");
	BROWSER_LOGD("erase = %d", erase);
	if (!erase)
		return;

	if (elm_object_focus_get(layout)) {
		if (elm_entry_is_empty(obj))
			elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
		else
			elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");
	}
}

static void __focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	void *erase = evas_object_data_get(layout, "eraser");
	BROWSER_LOGD("erase = %d", erase);

	if (!elm_entry_is_empty(obj) && erase)
		elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");

	elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
}

static void __unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	if (elm_entry_is_empty(obj))
		elm_object_signal_emit(layout, "elm,state,guidetext,show", "elm");

	elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
}

static void __url_entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	void *erase = evas_object_data_get(layout, "eraser");
	BROWSER_LOGD("erase = %d", erase);

	if (!elm_entry_is_empty(obj) && erase)
		elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");

	elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");

	int *cursor_position = (int *)evas_object_data_get(obj, "cursor_position");
	BROWSER_LOGD("cursor_position = %d", cursor_position);
	elm_entry_cursor_pos_set(obj, (int)cursor_position);
}

static void __url_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	if (elm_entry_is_empty(obj))
		elm_object_signal_emit(layout, "elm,state,guidetext,show", "elm");

	elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");

	evas_object_data_set(obj, "cursor_position", (void *)elm_entry_cursor_pos_get(obj));

	BROWSER_LOGD("cursor_position = %d", elm_entry_cursor_pos_get(obj));
}

static void __eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *entry = (Evas_Object *)data;
	elm_entry_entry_set(entry, "");
}

Evas_Object *br_elm_url_editfield_add(Evas_Object *parent)
{
	Evas_Object *layout = elm_layout_add(parent);

	elm_layout_theme_set(layout, "layout", "browser-editfield", "default");

	Evas_Object *entry = elm_entry_add(parent);
	elm_object_part_content_set(layout, "elm.swallow.content", entry);

	evas_object_smart_callback_add(entry, "changed", __changed_cb, layout);
	evas_object_smart_callback_add(entry, "focused", __url_entry_focused_cb, layout);
	evas_object_smart_callback_add(entry, "unfocused", __url_entry_unfocused_cb, layout);
	elm_object_signal_callback_add(layout, "elm,eraser,clicked", "elm",
						__eraser_clicked_cb, entry);

	return layout;
}

Evas_Object *br_elm_editfield_add(Evas_Object *parent, Eina_Bool title)
{
	Evas_Object *layout = elm_layout_add(parent);
	if (title)
		elm_layout_theme_set(layout, "layout", "editfield", "title");
	else
		elm_layout_theme_set(layout, "layout", "editfield", "default");

	Evas_Object *entry = elm_entry_add(parent);
	elm_object_part_content_set(layout, "elm.swallow.content", entry);

	elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
	evas_object_data_set(layout, "eraser", (void *)EINA_TRUE);

	evas_object_smart_callback_add(entry, "changed", __changed_cb, layout);
	evas_object_smart_callback_add(entry, "focused", __focused_cb, layout);
	evas_object_smart_callback_add(entry, "unfocused", __unfocused_cb, layout);
	elm_object_signal_callback_add(layout, "elm,eraser,clicked", "elm",
						__eraser_clicked_cb, entry);

	return layout;
}

void br_elm_editfield_label_set(Evas_Object *obj, const char *label)
{
	elm_object_part_text_set(obj, "elm.text", label);
}

Evas_Object *br_elm_editfield_entry_get(Evas_Object *obj)
{
	return elm_object_part_content_get(obj, "elm.swallow.content");
}

void br_elm_editfield_guide_text_set(Evas_Object *obj, const char *text)
{
	elm_object_part_text_set(obj, "elm.guidetext", text);
}

void br_elm_editfield_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line)
{
	Evas_Object *entry = br_elm_editfield_entry_get(obj);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
}

void br_elm_editfield_eraser_set(Evas_Object *obj, Eina_Bool visible)
{
	evas_object_data_set(obj, "eraser", (void *)visible);

	if (visible)
		elm_object_signal_emit(obj, "elm,state,eraser,show", "elm");
	else
		elm_object_signal_emit(obj, "elm,state,eraser,hide", "elm");
}


static void __searchbar_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	if (elm_object_focus_get(layout)) {
		if (elm_entry_is_empty(obj))
			elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
		else
			elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");
	}
}

static void __searchbar_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");

	elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
	elm_object_signal_emit(layout, "cancel,in", "");
}

static void __searchbar_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	if (elm_entry_is_empty(obj))
		elm_object_signal_emit(layout, "elm,state,guidetext,show", "elm");
	elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
}

static void __searchbar_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *entry = (Evas_Object *)data;

	elm_entry_entry_set(entry, "");
}

static void __searchbar_bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *entry = (Evas_Object *)data;

	elm_object_focus_set(entry, EINA_TRUE);
}

static void __searchbar_cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *searchbar_layout = (Evas_Object *)data;
	Evas_Object *entry = elm_object_part_content_get(searchbar_layout, "elm.swallow.content");
	const char* text;
	evas_object_hide(obj);
	elm_object_signal_emit(searchbar_layout, "cancel,out", "");

	text = elm_entry_entry_get(entry);
	if (text != NULL && strlen(text) > 0)
		elm_entry_entry_set(entry, NULL);

	elm_object_focus_set(entry, EINA_FALSE);
}

static void __search_entry_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *entry = (Evas_Object *)data;
	if (!elm_object_focus_allow_get(entry)) {
		elm_object_focus_allow_set(entry, EINA_TRUE);
		elm_object_focus_set(entry, EINA_TRUE);
	}
}

Evas_Object *br_elm_searchbar_add(Evas_Object *parent)
{
	Evas_Object *searchbar_layout = elm_layout_add(parent);
	elm_layout_theme_set(searchbar_layout, "layout", "searchbar", "cancel_button");
	Evas_Object *entry = elm_entry_add(searchbar_layout);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_object_part_content_set(searchbar_layout, "elm.swallow.content", entry);
	elm_object_part_text_set(searchbar_layout, "elm.guidetext", BR_STRING_SEARCH);

	/* Workaround 
	  * When tab history view, the search entry has focus automatically.
	  * So the keypad is shown. So give focus manually. */
	edje_object_signal_callback_add(elm_layout_edje_get(searchbar_layout),
					"mouse,clicked,1", "elm.swallow.content", __search_entry_clicked_cb, entry);

	elm_object_focus_allow_set(entry, EINA_FALSE);

	Evas_Object *cancel_button = elm_button_add(searchbar_layout);
	elm_object_part_content_set(searchbar_layout, "button_cancel", cancel_button);
	elm_object_style_set(cancel_button, "searchbar/default");
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	evas_object_smart_callback_add(cancel_button, "clicked", __searchbar_cancel_clicked_cb, searchbar_layout);

	evas_object_smart_callback_add(entry, "changed", __searchbar_changed_cb, searchbar_layout);
	evas_object_smart_callback_add(entry, "focused", __searchbar_focused_cb, searchbar_layout);
	evas_object_smart_callback_add(entry, "unfocused", __searchbar_unfocused_cb, searchbar_layout);
	elm_object_signal_callback_add(searchbar_layout, "elm,eraser,clicked", "elm",
						__searchbar_eraser_clicked_cb, entry);
	elm_object_signal_callback_add(searchbar_layout, "elm,bg,clicked", "elm", __searchbar_bg_clicked_cb, entry);

	return searchbar_layout;
}

void br_elm_searchbar_text_set(Evas_Object *obj, const char *text)
{
	Evas_Object *entry = elm_object_part_content_get(obj, "elm.swallow.content");
	elm_object_text_set(entry, text);
}

char *br_elm_searchbar_text_get(Evas_Object *obj)
{
	Evas_Object *entry = elm_object_part_content_get(obj, "elm.swallow.content");
	const char *text = elm_object_text_get(entry);
	BROWSER_LOGD("search entry text=[%s]", text);

	if (text && strlen(text))
		return strdup(text);
	else
		return NULL;
}

Evas_Object *br_elm_searchbar_entry_get(Evas_Object *obj)
{
	return elm_object_part_content_get(obj, "elm.swallow.content");
}

