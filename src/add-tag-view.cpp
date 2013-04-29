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

#include "add-tag-view.h"

#include <Elementary.h>
#include <string>

#include "bookmark.h"
#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "scrap.h"

#define add_tag_view_edj_path browser_edj_dir"/add-tag-view.edj"

add_tag_view::add_tag_view(Evas_Smart_Cb done_cb, void *done_cb_data, const char *tag)
:
	m_item_ic(NULL)
	,m_main_layout(NULL)
	,m_genlist(NULL)
	,m_tagbar_layout(NULL)
	,m_tag_entry(NULL)
	,m_multibutton_entry(NULL)
	,m_multibutton_entry_clicked_item(NULL)
	,m_done_cb(done_cb)
	,m_done_cb_data(done_cb_data)
	,m_init_tag(NULL)
{
	BROWSER_LOGD("tag=[%s]", tag);

	m_main_layout = _create_main_layout(m_naviframe);

	if (tag && strlen(tag)) {
		m_init_tag = eina_stringshare_add(tag);

		if (m_init_tag) {
			scrap scrap_instance;
			std::vector<char *> tag_list = scrap_instance.extract_tag_list(m_init_tag);
			for (int i = 0 ; i < tag_list.size() ; i++) {
				_add_tag_to_multibutton_entry(tag_list[i]);
				free(tag_list[i]);
			}
		}
	}
}

add_tag_view::~add_tag_view(void)
{
	BROWSER_LOGD("");

	for (int i = 0 ; i < m_tag_list.size() ; i++)
		free(m_tag_list[i]);
	m_tag_list.clear();

	for (int i = 0 ; i < m_tag_item_list.size() ; i++)
		free(m_tag_item_list[i]);

	if (m_item_ic)
		elm_genlist_item_class_free(m_item_ic);

	evas_object_smart_callback_del(m_conformant, "virtualkeypad,state,on", __ime_show_cb);
	evas_object_smart_callback_del(m_conformant, "virtualkeypad,state,off", __ime_hide_cb);
	evas_object_smart_callback_del(m_conformant, "clipboard,state,on", __ime_show_cb);
	evas_object_smart_callback_del(m_conformant, "clipboard,state,off", __ime_hide_cb);

	eina_stringshare_del(m_init_tag);
}

void add_tag_view::show(void)
{
	Elm_Object_Item *naviframe_item = elm_naviframe_item_push(m_naviframe, BR_STRING_ADD_TAG, NULL, NULL, m_main_layout, NULL);

	Evas_Object *done_button = elm_button_add(m_naviframe);
	if (!done_button) {
		BROWSER_LOGE("elm_button_add failed");
		return;
	}
	elm_object_text_set(done_button, BR_STRING_DONE);
	evas_object_smart_callback_add(done_button, "clicked", __done_cb, this);

	elm_object_style_set(done_button, "naviframe/toolbar/default");
	elm_object_item_part_content_set(naviframe_item, "toolbar_button1", done_button);

	Evas_Object *title_done_button = elm_button_add(m_naviframe);
	if (!title_done_button) {
		BROWSER_LOGE("elm_button_add failed");
		return;
	}
	elm_object_text_set(title_done_button, BR_STRING_DONE);
	elm_object_style_set(title_done_button, "naviframe/toolbar/default");

	elm_object_item_part_content_set(naviframe_item, "title_toolbar_button1", title_done_button);
	evas_object_smart_callback_add(title_done_button, "clicked", __done_cb, this);

	Evas_Object *back_button = elm_button_add(m_naviframe);
	if (!back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return;
	}
	elm_object_style_set(back_button, "naviframe/back_btn/default");
	elm_object_item_part_content_set(naviframe_item, "title_prev_btn", back_button);
	evas_object_smart_callback_add(back_button, "clicked", __back_button_cb, this);

	evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,on", __ime_show_cb, naviframe_item);
	evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,off", __ime_hide_cb, naviframe_item);
	evas_object_smart_callback_add(m_conformant, "clipboard,state,on", __ime_show_cb, naviframe_item);
	evas_object_smart_callback_add(m_conformant, "clipboard,state,off", __ime_hide_cb, naviframe_item);
}

void add_tag_view::__ime_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)data;
	elm_object_item_signal_emit(it, "elm,state,sip,shown", "");
}

void add_tag_view::__ime_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)data;
	elm_object_item_signal_emit(it, "elm,state,sip,hidden", "");
}

void add_tag_view::__back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_naviframe_item_pop(m_naviframe);
}

Evas_Object *add_tag_view::_create_main_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, add_tag_view_edj_path, "main-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *content_layout = elm_layout_add(layout);
	if (!content_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}

	elm_layout_theme_set(content_layout, "layout", "application", "searchbar_base");
	elm_object_signal_emit(content_layout, "elm,state,show,searchbar", "elm");

	Evas_Object *tagbar_layout = _create_tagbar_layout(content_layout);
	if (!tagbar_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_object_part_content_set(content_layout, "searchbar", tagbar_layout);

	std::vector<char *> bookmark_tag_list;
	bookmark bookmark_instance;
	if (bookmark_instance.get_tag_list(bookmark_tag_list)) {
		for (int i = 0 ; i < bookmark_tag_list.size() ; i++) {
			BROWSER_LOGD("tag[%d]=[%s]", i, bookmark_tag_list[i]);
			m_tag_list.push_back(bookmark_tag_list[i]);
		}
	}

	std::vector<char *> scrap_tag_list;
	scrap scrap_instance;
	scrap_tag_list = scrap_instance.get_tag_list();

	for (int i = 0 ; i < scrap_tag_list.size() ; i++) {
		BROWSER_LOGD("scrap tag[%d]=[%s]", i, scrap_tag_list[i]);
		Eina_Bool duplicated = EINA_FALSE;
		for (int j = 0 ; j < m_tag_list.size() ; j++) {
			if (!strcmp(m_tag_list[j], scrap_tag_list[i]))
				duplicated = EINA_TRUE;
		}

		if (!duplicated)
			m_tag_list.push_back(strdup(scrap_tag_list[i]));
	}

	for (int i = 0 ; i < scrap_tag_list.size() ; i++)
		free(scrap_tag_list[i]);

	if (m_tag_list.size() == 0) {
		Evas_Object *empty_layout = elm_layout_add(layout);
		if (!empty_layout) {
			BROWSER_LOGE("elm_layout_add");
			return NULL;
		}
		elm_layout_theme_set(empty_layout, "layout", "nocontents", "search");
		elm_object_part_text_set (empty_layout, "elm.text", BR_STRING_NO_TAG);
		elm_object_part_content_set(content_layout, "elm.swallow.content", empty_layout);
	} else {
		Evas_Object *genlist = _create_genlist(layout);
		if (!genlist) {
			BROWSER_LOGE("_create_genlist failed");
			return NULL;
		}

		for (int i = 0 ; i < m_tag_list.size() ; i++) {
			tag_item *item = (tag_item *)malloc(sizeof(tag_item));
			if (!item)
				return NULL;
			memset(item, 0x00, sizeof(tag_item));
			item->tag = m_tag_list[i];
			BROWSER_LOGD("*********** item->tag=[%s]", item->tag);
			item->is_checked = EINA_FALSE;
			item->data = this;
			elm_genlist_item_append(genlist, m_item_ic, item, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, item);

			m_tag_item_list.push_back(item);
		}

		elm_object_part_content_set(content_layout, "elm.swallow.content", genlist);
		m_genlist = genlist;
	}
	elm_object_part_content_set(layout, "elm.swallow.content", content_layout);

	m_tagbar_layout = tagbar_layout;

	return layout;
}

void add_tag_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	tag_item *item = (tag_item *)data;

	item->is_checked = !(item->is_checked);

	__check_changed_cb(item, NULL, NULL);

	elm_genlist_item_update(it);
	elm_genlist_item_selected_set(it, EINA_FALSE);
}

Evas_Object *add_tag_view::_create_multibutton_entry(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *multibutton_entry = elm_multibuttonentry_add(parent);
	if (!multibutton_entry) {
		BROWSER_LOGE("elm_multibuttonentry_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(multibutton_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(multibutton_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_multibuttonentry_editable_set(multibutton_entry, EINA_FALSE);

	return multibutton_entry;
}

void add_tag_view::__done_cb(void *data, Evas_Object *obj, void *event_info)
{
	add_tag_view *atv = (add_tag_view *)data;

	Elm_Object_Item *it = elm_multibuttonentry_first_item_get(atv->m_multibutton_entry);
	std::vector<char *> tag_list;

	while (it) {
		const char *tag = elm_object_item_text_get(it);
		BROWSER_LOGD("tag=[%s]", tag);
		tag_list.push_back(strdup(tag));

		it = elm_multibuttonentry_item_next_get(it);
	}

	if (atv->m_done_cb) {
		atv->m_done_cb(atv->m_done_cb_data, NULL, &tag_list);
	}

	__back_button_cb(data, NULL, NULL);
}


static void _tag_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *tagbar_layout = (Evas_Object *)data;

	if (elm_object_focus_get(obj)) {
		if (elm_entry_is_empty(obj)) {
			BROWSER_LOGD("elm,state,eraser,hide");
			elm_object_signal_emit(tagbar_layout, "elm,state,eraser,hide", "elm");
		} else {
			BROWSER_LOGD("text=[%s] - elm,state,eraser,show", elm_entry_entry_get(obj));
			elm_object_signal_emit(tagbar_layout, "elm,state,eraser,show", "elm");
		}
	}

	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(tagbar_layout, "elm,state,guidetext,hide", "elm");
	else {
		if (!elm_object_focus_get(obj))
			elm_object_signal_emit(tagbar_layout, "elm,state,guidetext,show", "elm");
		else
			elm_object_signal_emit(tagbar_layout, "elm,state,guidetext,hide", "elm");
	}
}

static void _tag_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *tagbar_layout = (Evas_Object *)data;

	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(tagbar_layout, "elm,state,eraser,show", "elm");
	elm_object_signal_emit(tagbar_layout, "elm,state,guidetext,hide", "elm");
	elm_object_signal_emit(tagbar_layout, "cancel,in", "");
}

static void _tag_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *tagbar_layout = (Evas_Object *)data;

	if (elm_entry_is_empty(obj))
		elm_object_signal_emit(tagbar_layout, "elm,state,guidetext,show", "elm");

	elm_object_signal_emit(tagbar_layout, "elm,state,eraser,hide", "elm");
}

static void _tag_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	Evas_Object *entry = (Evas_Object *)data;
	elm_entry_entry_set(entry, "");
}

static void _tag_bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");

	elm_object_focus_set(obj, EINA_TRUE);
}

void add_tag_view::_delete_multibutton_entry_item(Elm_Object_Item *item)
{
	EINA_SAFETY_ON_NULL_RETURN(item);
	const char *item_text = elm_object_item_text_get(item);
	for (int i = 0 ; i < m_tag_item_list.size() ; i++) {
		if (!strcmp(item_text, m_tag_item_list[i]->tag)) {
			m_tag_item_list[i]->is_checked = EINA_FALSE;
			elm_genlist_realized_items_update(m_genlist);
			break;
		}
	}

	elm_object_item_del(item);

	if (_get_multibutton_entry_count() == 0) {
		elm_object_part_content_unset(m_main_layout, "elm.swallow.tag_button");
		evas_object_del(m_multibutton_entry);
		m_multibutton_entry = NULL;
	}
}

void add_tag_view::__tag_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	add_tag_view *atv = (add_tag_view *)data;

	atv->destroy_popup(obj);

	if (atv->m_multibutton_entry_clicked_item) {
		atv->_delete_multibutton_entry_item(atv->m_multibutton_entry_clicked_item);
		atv->m_multibutton_entry_clicked_item = NULL;
	}
}

void add_tag_view::__tag_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	add_tag_view *atv = (add_tag_view *)data;

	atv->destroy_popup(obj);

	if (atv->m_multibutton_entry_clicked_item) {
		const char *tag = elm_object_item_text_get(atv->m_multibutton_entry_clicked_item);
		elm_entry_entry_set(atv->m_tag_entry, tag);
		elm_object_focus_set(atv->m_tag_entry, EINA_TRUE);
		elm_entry_cursor_end_set(atv->m_tag_entry);

		atv->_delete_multibutton_entry_item(atv->m_multibutton_entry_clicked_item);
		atv->m_multibutton_entry_clicked_item = NULL;
	}
}

void add_tag_view::_show_tag_clicked_popup(Elm_Object_Item *item, Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN(item);
	EINA_SAFETY_ON_NULL_RETURN(parent);

	const char *tag = elm_object_item_text_get(item);

	Evas_Object *content_list = elm_list_add(parent);
	elm_list_mode_set(content_list, ELM_LIST_EXPAND);
	elm_list_item_append(content_list, BR_STRING_DELETE, NULL, NULL, __tag_delete_cb, this);
	elm_list_item_append(content_list, BR_STRING_EDIT, NULL, NULL, __tag_edit_cb, this);

	show_content_popup(tag, content_list, BR_STRING_CLOSE);
}

void add_tag_view::_tag_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	add_tag_view *atv = (add_tag_view *)data;

	atv->m_multibutton_entry_clicked_item = item;

	atv->_show_tag_clicked_popup(item, m_naviframe);
}

int add_tag_view::_get_multibutton_entry_count(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_multibutton_entry, 0);

	int item_count = 0;
	Elm_Object_Item *it = elm_multibuttonentry_first_item_get(m_multibutton_entry);
	while (it) {
		item_count++;
		it = elm_multibuttonentry_item_next_get(it);
	}

	return item_count;
}

Eina_Bool add_tag_view::_add_tag_to_multibutton_entry(const char *tag)
{
	BROWSER_LOGD("tag=[%s]", tag);
	EINA_SAFETY_ON_NULL_RETURN_VAL(tag, EINA_FALSE);
	if (strstr(tag, ",")) {
		show_msg_popup(NULL, BR_STRING_SPECIAL_CHARACTOR_WARNING);
		return EINA_FALSE;
	}

	if (_get_multibutton_entry_count() >= BROWSER_MAX_TAG_COUNT) {
		show_msg_popup(NULL, BR_STRING_WARNING_OVER_TAG_LIMIT);
		return EINA_FALSE;
	}

	if (!m_multibutton_entry) {
		m_multibutton_entry = _create_multibutton_entry(m_main_layout);
		elm_object_part_content_set(m_main_layout, "elm.swallow.tag_button", m_multibutton_entry);
	}

	Elm_Object_Item *it = elm_multibuttonentry_first_item_get(m_multibutton_entry);
	while (it) {
		const char *item_text = elm_object_item_text_get(it);
		if (!strcmp(item_text, tag)) {
			show_msg_popup(NULL, BR_STRING_ALREADY_EXISTS);
			return EINA_FALSE;
		}

		it = elm_multibuttonentry_item_next_get(it);
	}

	elm_multibuttonentry_item_append(m_multibutton_entry, tag, _tag_clicked_cb, this);

	for (int i = 0 ; i < m_tag_item_list.size() ; i++) {
		if (!strcmp(tag, m_tag_item_list[i]->tag)) {
			m_tag_item_list[i]->is_checked = EINA_TRUE;
			elm_genlist_realized_items_update(m_genlist);
			break;
		}
	}

	return EINA_TRUE;
}

void add_tag_view::__tag_add_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	add_tag_view *atv = (add_tag_view *)data;

	const char* text = elm_entry_entry_get(atv->m_tag_entry);
	BROWSER_LOGD("text=[%s]", text);
	if (text != NULL && strlen(text) > 0) {
		atv->_add_tag_to_multibutton_entry(text);
		elm_entry_entry_set(atv->m_tag_entry, NULL);
	}
}

Evas_Object *add_tag_view::_create_tagbar_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, add_tag_view_edj_path, "tagbar-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *entry = elm_entry_add(layout);
	if (!entry) {
		BROWSER_LOGE("elm_entry_add failed");
		return NULL;
	}
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_object_part_content_set(layout, "elm.swallow.content", entry);
	elm_object_part_text_set(layout, "elm.guidetext", BR_STRING_TAP_TO_INPUT_TAG);

	evas_object_smart_callback_add(entry, "changed", _tag_changed_cb, layout);
	evas_object_smart_callback_add(entry, "focused", _tag_focused_cb, layout);
	evas_object_smart_callback_add(entry, "unfocused", _tag_unfocused_cb, layout);
	elm_object_signal_callback_add(layout, "elm,eraser,clicked", "elm", _tag_eraser_clicked_cb, entry);
	elm_object_signal_callback_add(layout, "elm,bg,clicked", "elm", _tag_bg_clicked_cb, entry);

	Evas_Object *add_button = elm_button_add(layout);
	if (!add_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_part_content_set(layout, "button_cancel", add_button);
	elm_object_style_set(add_button, "searchbar/default");
	elm_object_text_set(add_button, BR_STRING_ADD);

	evas_object_smart_callback_add(add_button, "clicked", __tag_add_clicked_cb, this);

	m_tag_entry = entry;

	return layout;
}

Evas_Object *add_tag_view::_create_genlist(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	memset(item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));
	item_ic->item_style = "2text.1icon";
	item_ic->decorate_item_style = NULL;
	item_ic->decorate_all_item_style = NULL;
	item_ic->func.text_get = __genlist_label_get_cb;
	item_ic->func.content_get = __genlist_content_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

	m_item_ic = item_ic;

	return genlist;
}

char *add_tag_view::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	tag_item *item = (tag_item *)data;

	if (!strcmp(part, "elm.text.1")) {
		return elm_entry_utf8_to_markup(item->tag);
	}

	return NULL;
}

void add_tag_view::_delete_tag_from_multibutton_entry(const char *tag)
{
	BROWSER_LOGD("tag=[%s]", tag);
	EINA_SAFETY_ON_NULL_RETURN(tag);
	Elm_Object_Item *it = elm_multibuttonentry_first_item_get(m_multibutton_entry);
	while (it) {
		const char *item_text = elm_object_item_text_get(it);
		if (!strcmp(item_text, tag)) {
			_delete_multibutton_entry_item(it);
			return;
		}

		it = elm_multibuttonentry_item_next_get(it);
	}
}

void add_tag_view::__check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	tag_item *item = (tag_item *)data;
	add_tag_view *atv = (add_tag_view *)(item->data);
	BROWSER_LOGD("tag=[%s], is_checked=[%d]", item->tag, item->is_checked);

	if (item->is_checked) {
		if (!atv->_add_tag_to_multibutton_entry(item->tag))
			item->is_checked = EINA_FALSE;
	} else
		atv->_delete_tag_from_multibutton_entry(item->tag);
}

Evas_Object *add_tag_view::__genlist_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	tag_item *item = (tag_item *)data;

	if (!strcmp(part, "elm.icon")) {
		Evas_Object *check = elm_check_add(obj);
		if (!check)
			return NULL;

		elm_check_state_pointer_set(check, &(item->is_checked));
		elm_check_state_set(check, item->is_checked);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_smart_callback_add(check, "changed", __check_changed_cb, data);

		return check;
	}

	return NULL;
}

