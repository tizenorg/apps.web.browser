/*
 * Copyright 2013  Samsung Electronics Co., Ltd
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
 * Contact: Sangpyo Kim <sangpyo7.kim@samsung.com>
 *
 */

#include "bookmark-common-view.h"
#include "bookmark.h"
#include "bookmark-item.h"
#include "bookmark-add-view.h"
#include "bookmark-select-folder-view.h"
#include "bookmark-edit-view.h"
#include "bookmark-create-folder-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include <efl_extension.h>

#define bookmark_view_edj_path browser_edj_dir"/bookmark-view.edj"

#define FOLDER_NAME_ENTRY_MAX_COUNT 2048

static Elm_Entry_Filter_Limit_Size entry_limit_size = {0, FOLDER_NAME_ENTRY_MAX_COUNT};

bookmark_create_folder_view::bookmark_create_folder_view(Evas_Smart_Cb cb_func, void *cb_data, int parent_id)
	: m_genlist(NULL)
	, m_btn_save(NULL)
	, m_scroller(NULL)
	, m_main_layout(NULL)
	, m_box(NULL)
	, m_group_index(NULL)
	, m_naviframe_item(NULL)
	, m_folder_id(parent_id)
	, m_saved_id(0)
	, m_select_folder_cb_func(cb_func)
	, m_select_folder_cb_data(cb_data)
	, m_title_entry(NULL)
	, m_itc_folder(NULL)
	, m_state_index(0)
	, m_index(0)
	, m_contents_layout(NULL)
{
	BROWSER_LOGD("parent_id: %d", parent_id);
}

bookmark_create_folder_view::~bookmark_create_folder_view(void)
{
	BROWSER_LOGD("");

	evas_object_smart_callback_del(m_naviframe,
						"transition,finished",
						__naviframe_pop_finished_cb);

	if (m_title_entry)
		evas_object_smart_callback_del(m_title_entry, "changed", __title_entry_changed_cb);

	if (m_genlist) {
		elm_genlist_clear(m_genlist);
		evas_object_del(m_genlist);
		m_genlist = NULL;
	}
	if (m_scroller)
		evas_object_del(m_scroller);

	if (m_itc_folder)
		elm_genlist_item_class_free(m_itc_folder);

	if (m_group_index)
		evas_object_del(m_group_index);

	if (m_contents_layout) {
		evas_object_event_callback_del(m_contents_layout, EVAS_CALLBACK_RESIZE, __on_layout_resized_cb);
		evas_object_del(m_contents_layout);
	}
}

void bookmark_create_folder_view::__title_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	bookmark_create_folder_view *view_this = (bookmark_create_folder_view *)data;

	const char *title = elm_entry_entry_get(obj);

	Eina_Bool only_has_space = EINA_FALSE;
	unsigned int space_count = 0;
	if (title && strlen(title)) {
		for (unsigned int i = 0 ; i < strlen(title) ; i++) {
			if (title[i] == ' ')
				space_count++;
		}
		if (space_count == strlen(title))
			only_has_space = EINA_TRUE;
		view_this->m_input_title_string = title;
	}
	BROWSER_LOGD("m_input_title_string(%d)[%s]",
		strlen(view_this->m_input_title_string.c_str())
		,view_this->m_input_title_string.c_str());

	char *text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	if (!text || strlen(text) == 0 || !title || strlen(title) == 0
	    || only_has_space) {
		elm_object_disabled_set(view_this->m_btn_save, EINA_TRUE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
	} else {
		elm_object_disabled_set(view_this->m_btn_save, EINA_FALSE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
	}

	view_this->show_notification(text, obj, FOLDER_NAME_ENTRY_MAX_COUNT);

	if (text)
		free(text);

	Evas_Object *parent_layout = elm_object_parent_widget_get(obj);
	if (elm_object_focus_get(obj) && parent_layout) {
		if (elm_entry_is_empty(obj))
			elm_object_signal_emit(parent_layout, "elm,state,eraser,hide", "");
		else
			elm_object_signal_emit(parent_layout, "elm,state,eraser,show", "");
	}
}

void bookmark_create_folder_view::__title_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	const char *title = elm_entry_entry_get(obj);

	if (!title || strlen(title) == 0) {
		BROWSER_LOGD("title is empty");
		cp->show_msg_popup(BR_STRING_ENTER_TITLE);
		return;
	} else {
		if ((cp->_save_folder()) == 0) {
			cp->show_msg_popup(BR_STRING_ALREADY_EXISTS);
			return;
		}
		cp->_back_to_previous_view();
	}
}

void bookmark_create_folder_view::__title_entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	Evas_Object *parent_layout = elm_object_parent_widget_get(obj);
	if (parent_layout) {
		if (!elm_entry_is_empty(obj)) {
			elm_object_signal_emit(obj, "elm,state,eraser,show", "");
		}
		elm_object_signal_emit(obj, "elm,state,rename,hide", "");
	}
}

void bookmark_create_folder_view::__editfield_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *editfield = (Evas_Object *)data;

	if (!elm_entry_is_empty(obj) && elm_object_focus_get(obj))
		elm_object_signal_emit(editfield, "elm,action,show,button", "");
	else
		elm_object_signal_emit(editfield, "elm,action,hide,button", "");
}

void bookmark_create_folder_view::__editfield_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *entry = (Evas_Object *)data;

	elm_entry_entry_set(entry, "");
}

void bookmark_create_folder_view::__title_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	Evas_Object *parent_layout = elm_object_parent_widget_get(obj);
	if (parent_layout) {
		elm_object_signal_emit(parent_layout, "elm,state,eraser,hide", "");
		elm_object_signal_emit(parent_layout, "elm,state,rename,show", "");
	}
}

char *bookmark_create_folder_view::__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	_genlist_callback_data *callback_data = (_genlist_callback_data *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)callback_data->user_data;

	if (!strcmp(part, "elm.text.main.left")) {
		if (bookmark_item_data->get_id() == m_browser->get_bookmark()->get_root_folder_id()) {
			return strdup(BR_STRING_BOOKMARKS);
		} else {
			char *markup_title = elm_entry_utf8_to_markup(bookmark_item_data->get_title());
			if (markup_title) {
				char *title = strdup(markup_title);
				free(markup_title);
				return title;
			} else
				return strdup(bookmark_item_data->get_title());
		}
	}
	return NULL;
}

Evas_Object *bookmark_create_folder_view::__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
	RETV_MSG_IF(!obj, NULL, "obj is NULL");
	_genlist_callback_data *callback_data = (_genlist_callback_data *)data;
	RETV_MSG_IF(!callback_data->cp, NULL, "cp is NULL");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)callback_data->cp;

	Evas_Object *content = NULL;
	if (!strcmp(part, "elm.icon.left")) {
		content = elm_layout_add(obj);
		elm_layout_theme_set(content, "layout", "list/B/type.2", "default");
		Evas_Object *folder_icon = elm_icon_add(obj);
		if (folder_icon == NULL)
			return NULL;
		elm_image_file_set(folder_icon, bookmark_view_edj_path, "contacts_ic_folder.png");
		evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_layout_content_set(content, "elm.swallow.content", folder_icon);
		return folder_icon;
	}

	//Item_Data *id = data;
	int index = callback_data->index;
	Evas_Object *radio;
	Evas_Object *radio_main = (Evas_Object *)evas_object_data_get(obj, "radio_main");

	if (!strcmp(part, "elm.icon.right")) {
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "default/genlist");
		elm_radio_state_value_set(radio, index);
		elm_radio_group_add(radio, radio_main);
		if (index == cp->m_state_index)
			elm_radio_value_set(radio, cp->m_state_index);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_smart_callback_add(radio, "changed", NULL, callback_data);

		// If no divider, unregister access object
		//elm_access_object_unregister(radio);
		return radio;
	}

	return NULL;
}

void bookmark_create_folder_view::__timer_popup_expired_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	if (cp->m_genlist && cp->m_title_entry)
		elm_object_focus_set(cp->m_title_entry, EINA_TRUE);
}

void bookmark_create_folder_view::__save_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	if ((cp->_save_folder()) == 0) {
		cp->show_msg_popup(BR_STRING_ALREADY_EXISTS, 3, __timer_popup_expired_cb, data);
		return;
	}
	cp->_back_to_previous_view();
}

void bookmark_create_folder_view::__cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	cp->_back_to_previous_view();
}

void bookmark_create_folder_view::__naviframe_pop_finished_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	if (cp->m_naviframe_item != elm_naviframe_top_item_get(cp->m_naviframe))
		return;

	if (!m_browser->is_tts_enabled()) {
		if (cp->m_title_edit_field != NULL) {
			elm_object_focus_set(cp->m_title_edit_field, EINA_TRUE);
			elm_entry_cursor_end_set(cp->m_title_edit_field);
		}
	}
}

Eina_Bool bookmark_create_folder_view::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	if (!data) {
		BROWSER_LOGE("data is NULL");
		return EINA_FALSE;
	}
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	cp->_do_before_naviframe_pop();
	return EINA_TRUE;
}

int bookmark_create_folder_view::_save_folder(void)
{
	BROWSER_LOGD("");
	int saved_folder_id;
	int ret = 0;
	char *folder_title = elm_entry_markup_to_utf8(m_input_title_string.c_str());
	ret = m_browser->get_bookmark()->save_folder(
						folder_title,
						&saved_folder_id,
						m_folder_id
						);
	if (folder_title)
		free(folder_title);

	if (ret < 0) {
		BROWSER_LOGD("folder add/edit is failed");
		return -1;
	}
	if (ret == 0) {
		BROWSER_LOGD("same folder is already exist");
		return 0;
	}
	m_saved_id = saved_folder_id;
	return 1;
}

void bookmark_create_folder_view::_do_before_naviframe_pop(void)
{
	BROWSER_LOGD("");

	bookmark_item created_bookmark;
	m_bookmark->get_item_by_id(m_saved_id, &created_bookmark);

	if ((m_browser->is_bookmark_view_exist() || m_browser->get_add_bookmark_view_exist()) && m_select_folder_cb_func)
		m_select_folder_cb_func(m_select_folder_cb_data, NULL, &created_bookmark);
}

void bookmark_create_folder_view::_back_to_previous_view(void)
{
	BROWSER_LOGD("");

	if (elm_naviframe_bottom_item_get(m_naviframe) !=
						elm_naviframe_top_item_get(m_naviframe))
					elm_naviframe_item_pop(m_naviframe);
}

Evas_Object *bookmark_create_folder_view::_create_scroller(Evas_Object * parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");
	Evas_Object *scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);
	return scroller;
}

Evas_Object *bookmark_create_folder_view::_create_box(Evas_Object * parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");
	Evas_Object *box = NULL;
	box = elm_box_add(parent);
	RETV_MSG_IF(!box, NULL, "box is NULL");
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(box);
	return box;
}

void bookmark_create_folder_view::__group_index_lang_changed(void *data, Evas_Object * obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	elm_object_part_text_set(cp->m_group_index, "elm.text", BR_STRING_TARGET_FOLDER);
}

void bookmark_create_folder_view::_clear_genlist_item_data(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!genlist, "genlist is NULL");

	Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
	while (it) {
		elm_genlist_item_expanded_set(it, EINA_TRUE);
		_genlist_callback_data *item_data = (_genlist_callback_data *)elm_object_item_data_get(it);
		bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;
		delete bookmark_item_data;
		free(item_data);
		it = elm_genlist_item_next_get(it);
	}
}

void bookmark_create_folder_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	_genlist_callback_data *cb_data = (_genlist_callback_data *)elm_object_item_data_get(it);
	RET_MSG_IF(!cb_data->cp, "cp is NULL");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)cb_data->cp;
	int index = cb_data->index;
	BROWSER_LOGD("index: %d", index);
	Evas_Object *radio = elm_object_item_part_content_get(it, "elm.icon.right");
	cp->m_state_index = index;
	elm_radio_value_set(radio, cp->m_state_index);

	bookmark_item *bookmark_item_data = (bookmark_item *)cb_data->user_data;
	cp->m_folder_id = bookmark_item_data->get_id();
}

Eina_Bool bookmark_create_folder_view::_set_genlist_item_by_folder(Evas_Object *genlist, int folder_id, Elm_Object_Item *parent_it)
{
	BROWSER_LOGD("folder_id: %d", folder_id);
	std::vector<bookmark_item *> bookmark_list;
	Eina_Bool ret;

	if (folder_id < 0) {
		//for root folder only
		_genlist_callback_data *item_data = (_genlist_callback_data *)malloc(sizeof(_genlist_callback_data));
		if(!item_data)
			return EINA_FALSE;
		memset(item_data, 0x00, sizeof(_genlist_callback_data));

		bookmark_item *bookmark_item_data = new bookmark_item;
		/* deep copying by overloaded operator = */
		bookmark_item_data->set_id(0);
		bookmark_item_data->set_folder_flag(EINA_TRUE);
		bookmark_item_data->set_parent_id(-1);
		bookmark_item_data->set_title("Bookmarks");
		/* Folder item is found. get sub list */
		BROWSER_SECURE_LOGD("ROOT folder is %s(id: %d)",
				bookmark_item_data->get_title(),
				bookmark_item_data->get_id());
		item_data->cp = this;
		item_data->user_data = (void *)bookmark_item_data;
		item_data->index = m_index;
		m_index++;
		item_data->it = elm_genlist_item_append(genlist, m_itc_folder, item_data, parent_it,
				ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, this);
		return EINA_TRUE;
	}

	ret = m_bookmark->get_list_by_folder(folder_id, bookmark_list);
	if (ret == EINA_FALSE) {
		BROWSER_LOGE("get_list_by_folder is failed(folder id:%d)",folder_id);
		return EINA_FALSE;
	}

	for(unsigned int j = 0 ; j < bookmark_list.size() ; j++ ) {
		if (bookmark_list[j]->is_folder()) {
			_genlist_callback_data *item_data = (_genlist_callback_data *)malloc(sizeof(_genlist_callback_data));
			if(!item_data)
				return EINA_FALSE;
			memset(item_data, 0x00, sizeof(_genlist_callback_data));

			bookmark_item *bookmark_item_data = new bookmark_item;
			/* deep copying by overloaded operator = */
			*bookmark_item_data = *bookmark_list[j];
			/* Folder item is found. get sub list */
			BROWSER_SECURE_LOGD("Folder[%d] is %s(id: %d), index: %d\n", j,
					bookmark_list[j]->get_title(),
					bookmark_item_data->get_id(), m_index);
			item_data->cp = this;
			item_data->user_data = (void *)bookmark_item_data;
			item_data->index = m_index;
			m_index++;
			item_data->it = elm_genlist_item_append(genlist, m_itc_folder, item_data, parent_it,
					ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, this);
		}
	}
	m_bookmark->destroy_list(bookmark_list);
	return EINA_TRUE;
}

Eina_Bool bookmark_create_folder_view::_set_genlist_folder_tree(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!genlist, EINA_FALSE, "genlist is NULL");

	_clear_genlist_item_data(genlist);
	elm_genlist_clear(genlist);

	// Radio button
	Evas_Object *radio_main = elm_radio_add(genlist);
	elm_radio_state_value_set(radio_main, 0);
	elm_radio_value_set(radio_main, 0);
	evas_object_data_set(genlist, "radio_main", radio_main);

	m_itc_folder = elm_genlist_item_class_new();
	if(!m_itc_folder)
		return EINA_FALSE;
	m_itc_folder->item_style = "1line";
	m_itc_folder->func.text_get = __genlist_get_text_cb;
	m_itc_folder->func.content_get = __genlist_get_content_cb;
	m_itc_folder->func.state_get = NULL;
	m_itc_folder->func.del = NULL;

	int depth_count = 0;
	m_bookmark->get_folder_depth_count(&depth_count);
	BROWSER_LOGD("Final depth_count: %d", depth_count);
	depth_count = depth_count + 1; //increase count for virtual root folder

	for (int i=0 ; i <= depth_count ; i++) {
		BROWSER_LOGD("current depth: %d", i);
		if (i == 0 ) {
			BROWSER_LOGD("ROOT folder item is set", i);
			/* root folder */
			_set_genlist_item_by_folder(genlist, -1, NULL);
		} else {
			BROWSER_LOGD("SUB folder items are set", i);
			/* sub folder*/
			Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
			while (it) {
				Eina_Bool expanded = EINA_FALSE;
				expanded = elm_genlist_item_expanded_get(it);
				if (expanded == EINA_FALSE) {
					elm_genlist_item_expanded_set(it, EINA_TRUE);
					_genlist_callback_data *item_data = (_genlist_callback_data *)elm_object_item_data_get(it);
					bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;

					_set_genlist_item_by_folder(genlist, bookmark_item_data->get_id(), item_data->it);
				}

				_genlist_callback_data *cb_data = (_genlist_callback_data *)elm_object_item_data_get(it);
				bookmark_item *bookmark_item_data = (bookmark_item *)cb_data->user_data;
				if(bookmark_item_data->get_id() == m_folder_id) {
					int index = cb_data->index;
					BROWSER_LOGD("index: %d", index);
					Evas_Object *radio = elm_object_item_part_content_get(it, "elm.icon.1");
					m_state_index = index;
					elm_radio_value_set(radio, m_state_index);
				}

				it = elm_genlist_item_next_get(it);
			}
		}
	}

	return EINA_TRUE;
}

Evas_Object *bookmark_create_folder_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	_set_genlist_folder_tree(genlist);

	return genlist;
}

void bookmark_create_folder_view::__on_layout_resized_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	if(m_browser->get_browser_view()->is_landscape()) {
		BROWSER_LOGD("landscape");
		evas_object_size_hint_min_set(cp->m_contents_layout, 0, ELM_SCALE_SIZE(260));
		evas_object_size_hint_align_set(cp->m_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	} else {
		BROWSER_LOGD("portrait");
		evas_object_size_hint_min_set(cp->m_contents_layout, 0, ELM_SCALE_SIZE(520));
		evas_object_size_hint_align_set(cp->m_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	}
}

void bookmark_create_folder_view::_create_list_items(Evas_Object * parent)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!parent, "parent is NULL");

	Evas_Object *entry_layout = NULL;
	Evas_Object *entry = NULL;

	/* Title entry */
	Evas_Object *entry_bg_layout = elm_layout_add(parent);
	elm_layout_theme_set(entry_bg_layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(entry_bg_layout, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(entry_bg_layout, EVAS_HINT_FILL, 0.0);

	entry_layout = elm_layout_add(entry_bg_layout);
	elm_layout_theme_set(entry_layout, "layout", "editfield", "singleline");

	entry = elm_entry_add(entry_layout);
#if defined(HW_MORE_BACK_KEY)
	eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
#endif
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
	elm_object_domain_translatable_part_text_set(entry, "elm.guide", BROWSER_DOMAIN_NAME, "IDS_BR_BODY_FOLDER_NAME");
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	elm_entry_prediction_allow_set(entry, EINA_FALSE);
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &entry_limit_size);
	evas_object_smart_callback_add(entry, "changed", __editfield_changed_cb, entry_layout);
	elm_object_part_content_set(entry_layout, "elm.swallow.content", entry);
	evas_object_show(entry);
	evas_object_size_hint_weight_set(entry_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *button = elm_button_add(entry_layout);
	elm_object_style_set(button, "editfield_clear");
	evas_object_smart_callback_add(button, "clicked", __editfield_clear_button_clicked_cb, entry);
	elm_object_part_content_set(entry_layout, "elm.swallow.button", button);

	elm_object_signal_emit(entry_layout, "elm,state,top", "");
	elm_object_part_content_set(entry_bg_layout, "elm.swallow.content", entry_layout);
	Evas_Object *bg = elm_bg_add(entry_bg_layout);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_bg_color_set(bg, 255, 255, 255);
	elm_object_part_content_set(entry_bg_layout, "elm.swallow.bg", bg);

	evas_object_show(entry_bg_layout);
	elm_box_pack_end(m_box, entry_bg_layout);

	evas_object_smart_callback_add(entry, "changed", __title_entry_changed_cb, this);
	evas_object_smart_callback_add(entry, "preedit,changed", __title_entry_changed_cb, this);
	evas_object_smart_callback_add(entry, "activated", __title_entry_enter_key_cb, this);
	evas_object_smart_callback_add(entry, "focused", __title_entry_focused_cb, this);
	evas_object_smart_callback_add(entry, "unfocused", __title_entry_unfocused_cb, this);
	this->m_title_edit_field = entry;

	/* group title */
	m_group_index = elm_layout_add(m_box);
	RET_MSG_IF(!m_group_index, "elm_layout_add is failed");

	elm_object_focus_set(m_group_index, EINA_FALSE);
	elm_layout_theme_set(m_group_index, "genlist/item", "groupindex", "default");
	evas_object_size_hint_weight_set(m_group_index, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(m_group_index, EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(m_group_index, "elm.text.main", _("IDS_BR_BODY_LOCATION_M_INFORMATION"));

	evas_object_show(m_group_index);
	evas_object_smart_callback_add(m_group_index, "language,changed", __group_index_lang_changed, this);
	elm_box_pack_end(m_box, m_group_index);

	/* folder genlist*/
	m_contents_layout = elm_layout_add(m_box);
	elm_layout_file_set(m_contents_layout, bookmark_view_edj_path, "contents");
	elm_object_focus_set(m_contents_layout, EINA_FALSE);

	evas_object_size_hint_weight_set(m_contents_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_contents_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(m_contents_layout,  EVAS_CALLBACK_RESIZE, __on_layout_resized_cb, this);
	evas_object_show(m_contents_layout);

	elm_box_pack_end(m_box, m_contents_layout);

	m_genlist = _create_genlist(parent);
	evas_object_show(m_genlist);
	elm_layout_content_set(m_contents_layout, "elm.swallow.bookmarks", m_genlist);
	evas_object_show(m_contents_layout);
}

void bookmark_create_folder_view::show()
{
	BROWSER_LOGD("");

	m_main_layout= elm_layout_add(m_naviframe);
	RET_MSG_IF(!m_main_layout, "m_main_layout is NULL");
	elm_layout_theme_set(m_main_layout, "layout", "application", "noindicator");
	evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_scroller = _create_scroller(m_main_layout);
	RET_MSG_IF(!m_scroller, "m_scroller is NULL");
	elm_object_part_content_set(m_main_layout, "elm.swallow.content", m_scroller);

	m_box = _create_box(m_scroller);
	RET_MSG_IF(!m_box, "m_box is NULL");
	elm_object_content_set(m_scroller, m_box);
	_create_list_items(m_box);

	m_naviframe_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_SK3_CREATE_FOLDER", NULL, NULL, m_main_layout, NULL);
	elm_object_item_domain_text_translatable_set(m_naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);

	elm_naviframe_item_pop_cb_set(m_naviframe_item, __naviframe_pop_cb, this);

	evas_object_smart_callback_add(m_naviframe,
					"transition,finished",
					__naviframe_pop_finished_cb, this);
#if defined(HW_MORE_BACK_KEY)
	Evas_Object *btn_cancel = elm_button_add(m_naviframe);
	if (!btn_cancel) return;
	elm_object_style_set(btn_cancel, "naviframe/title_cancel");
	evas_object_smart_callback_add(btn_cancel, "clicked", __cancel_btn_clicked_cb, this);
	elm_object_item_part_content_set(m_naviframe_item, "title_left_btn", btn_cancel);

	m_btn_save = elm_button_add(m_naviframe);
	if (!m_btn_save) return;
	elm_object_style_set(m_btn_save, "naviframe/title_done");
	evas_object_smart_callback_add(m_btn_save, "clicked", __save_btn_clicked_cb, this);
	elm_object_item_part_content_set(m_naviframe_item, "title_right_btn", m_btn_save);
	elm_object_disabled_set(m_btn_save, EINA_TRUE);
	eext_object_event_callback_add(m_genlist, EEXT_CALLBACK_BACK, __cancel_btn_clicked_cb, this);
#else
	m_btn_save = elm_button_add(m_naviframe);
	if (!m_btn_save) return;
	elm_object_text_set(m_btn_save, BR_STRING_SAVE);
	elm_object_style_set(m_btn_save, "naviframe/toolbar/default");
	elm_object_item_part_content_set(m_naviframe_item, "toolbar_button1", m_btn_save);
	evas_object_smart_callback_add(m_btn_save, "clicked", __save_btn_clicked_cb, this);
	elm_object_disabled_set(m_btn_save, EINA_TRUE);
	evas_object_show(m_btn_save);
#endif
}

