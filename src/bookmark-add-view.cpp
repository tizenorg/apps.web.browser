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

#include <efl_extension.h>
#include <string.h>

#include "bookmark.h"
#include "bookmark-item.h"
#include "bookmark-add-view.h"
#include "bookmark-select-folder-view.h"
#include "bookmark-edit-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "platform-service.h"
#include "history.h"
#include "webview.h"

#define bookmark_view_edj_path browser_edj_dir"/bookmark-view.edj"

bookmark_add_view::bookmark_add_view(const char *title, const char *uri, int folder_id_to_save, Eina_Bool edit_mode)
:
	m_scroller(NULL)
	, m_box(NULL)
	, m_genlist(NULL)
	, m_title_edit_field(NULL)
	, m_uri_edit_field(NULL)
	, m_btn_save(NULL)
	, m_btn_cancel(NULL)
	, m_bookmark_edit_title(NULL)
	, m_bookmark_edit_url(NULL)
	, m_naviframe_item(NULL)
	, m_bookmark_id(0)
{
	m_folder_id = m_browser->get_bookmark()->get_root_folder_id();
	m_origin_folder_id = folder_id_to_save;
	m_edit_mode = edit_mode;

	if (uri) {
		m_uri_string = uri;
		m_input_uri_string = uri;
		m_origin_uri_string = uri;
	}

	if (!m_edit_mode){
		BROWSER_SECURE_LOGD("[Add bookmark mode]Title[%s], URI[%s]", title, uri);
		if (title) {
			m_input_title_string = title;
			m_origin_title_string = title;
		}

		m_folder_id = folder_id_to_save;
	} else {
		BROWSER_SECURE_LOGD("[Edit bookmark mode]URI[%s]", uri);
		if (m_bookmark->get_id(uri, &m_bookmark_id)) {
			bookmark_item item;
			if (m_bookmark->get_item_by_id(m_bookmark_id, &item)) {
				if (item.get_uri()) {
					m_uri_string = uri;
					m_input_uri_string = uri;
				}
				if (item.get_title()) {
					m_input_title_string = item.get_title();
					m_origin_title_string = item.get_title();
				}
				m_folder_id = item.get_parent_id();
				//In edit mode the origin folder id will be the same as the folder the bookmark is created.
				m_origin_folder_id = m_folder_id;
			} else
				BROWSER_LOGD("get bookmark item is failed");
		} else {
			BROWSER_LOGD("get bookmark id is failed");
		}
	}
	m_entry_limit_size.max_char_count = 0;
	m_entry_limit_size.max_byte_count = TITLE_INPUT_ENTRY_MAX_COUNT;
}

bookmark_add_view::~bookmark_add_view(void)
{
	BROWSER_LOGD("");

	evas_object_smart_callback_del(m_naviframe,
						"transition,finished",
						__naviframe_pop_finished_cb);
	if (m_genlist) {
		elm_genlist_clear(m_genlist);
		evas_object_del(m_genlist);
	}

	if (m_bookmark_edit_title)
		free(m_bookmark_edit_title);
	if (m_bookmark_edit_url)
		free(m_bookmark_edit_url);
}

void bookmark_add_view::__entry_next_key_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");

	bookmark_add_view *cp = (bookmark_add_view *)data;
	if (cp->m_input_uri_callback_data.it != NULL) {
		Evas_Object *entry = elm_object_item_part_content_get(
						cp->m_input_uri_callback_data.it, "elm.icon.entry");
		elm_object_focus_set(entry, EINA_TRUE);
		elm_entry_cursor_end_set(entry);
	}
}

void bookmark_add_view::__title_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	platform_service ps;
	bookmark_add_view *cp = (bookmark_add_view *)data;

	const char *title = elm_entry_entry_get(obj);
	if(!title)
		return;
	cp->m_input_title_string = title;

	char *input_title_utf8 = elm_entry_markup_to_utf8(cp->m_input_title_string.c_str());
	if(!input_title_utf8){
		return;
	}
	char *input_uri_utf8 = elm_entry_markup_to_utf8(cp->m_input_uri_string.c_str());
	if(!input_uri_utf8){
		free(input_title_utf8);
		return;
	}
	BROWSER_SECURE_LOGD("m_input_title_string(%d)[%s]",
		strlen(cp->m_input_title_string.c_str())
		,cp->m_input_title_string.c_str());

	char *text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	if (!text || strlen(text) == 0 || cp->_is_empty(title) ||
			cp->_is_empty(cp->m_input_uri_string.c_str())) {
		elm_object_disabled_set(cp->m_btn_save, EINA_TRUE);
	} else if (cp->m_edit_mode && !strcmp(input_title_utf8, cp->m_origin_title_string.c_str())) {
		if (!strcmp(input_uri_utf8, cp->m_origin_uri_string.c_str()))
			if (cp->m_folder_id == cp->m_origin_folder_id)
				elm_object_disabled_set(cp->m_btn_save, EINA_TRUE);
	} else {
		elm_object_disabled_set(cp->m_btn_save, EINA_FALSE);
	}

	if (elm_object_focus_get(obj))
			cp->show_notification(text, obj, TITLE_INPUT_ENTRY_MAX_COUNT);

	if (text)
		free(text);
	if (input_title_utf8)
		free(input_title_utf8);
	if (input_uri_utf8)
		free(input_uri_utf8);

	if (elm_object_focus_get(obj) && cp->m_input_title_callback_data.it) {
		if (elm_entry_is_empty(obj))
			elm_object_item_signal_emit(cp->m_input_title_callback_data.it, "elm,state,eraser,hide", "");
		else
			elm_object_item_signal_emit(cp->m_input_title_callback_data.it, "elm,state,eraser,show", "");
	}
}

void bookmark_add_view::__entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	bookmark_add_view *cp = (bookmark_add_view *)data;
	if (cp->m_input_uri_callback_data.it != NULL) {
		//change focus to save  when done button is pressed
		if (elm_object_disabled_get(cp->m_btn_save)) {
			BROWSER_LOGD("save btn disable");
			elm_object_focus_set(cp->m_btn_cancel, EINA_TRUE);
		} else {
			BROWSER_LOGD("save btn enable");
			elm_object_focus_set(cp->m_btn_save, EINA_TRUE);
		}
	}
}

void bookmark_add_view::__title_entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	bookmark_add_view *cp = (bookmark_add_view *)data;
	if (cp->m_input_title_callback_data.it) {
		if (!elm_entry_is_empty(obj)) {
			elm_object_item_signal_emit(cp->m_input_title_callback_data.it, "elm,state,eraser,show", "");
		}
		elm_object_item_signal_emit(cp->m_input_title_callback_data.it, "elm,state,rename,hide", "");
	}
}

void bookmark_add_view::__title_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	bookmark_add_view *cp = (bookmark_add_view *)data;
	if (cp->m_input_title_callback_data.it) {
		elm_object_item_signal_emit(cp->m_input_title_callback_data.it, "elm,state,eraser,hide", "");
		elm_object_item_signal_emit(cp->m_input_title_callback_data.it, "elm,state,rename,show", "");
	}
}

void bookmark_add_view::__title_entry_eraser_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	bookmark_add_view *cp = (bookmark_add_view *)data;
	if (cp->m_input_title_callback_data.it) {
		Evas_Object *entry = elm_object_item_part_content_get(cp->m_input_title_callback_data.it, "elm.icon.entry");
		elm_object_focus_set(entry, EINA_TRUE); // After button is clicked, entry should get focus again.
		elm_entry_entry_set(entry, "");
	}
}

void bookmark_add_view::__uri_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	platform_service ps;
	bookmark_add_view *cp = (bookmark_add_view *)data;

	const char *uri = elm_entry_entry_get(obj);
	if(!uri)
		return;
	cp->m_input_uri_string = uri;

	char *input_title_utf8 = elm_entry_markup_to_utf8(cp->m_input_title_string.c_str());
	if(!input_title_utf8)
		return;
	char *input_uri_utf8 = elm_entry_markup_to_utf8(cp->m_input_uri_string.c_str());
	if(!input_uri_utf8){
		free(input_title_utf8);
		return;
	}
	BROWSER_SECURE_LOGD("m_input_uri_string(%d)[%s]", strlen(cp->m_input_uri_string.c_str()) ,cp->m_input_uri_string.c_str());

	char *text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	if (!text || strlen(text) == 0 || cp->_is_empty(uri) ||
			cp->_is_empty(cp->m_input_title_string.c_str())) {
		elm_object_disabled_set(cp->m_btn_save, EINA_TRUE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
	} else if (cp->m_edit_mode && !strcmp(input_uri_utf8, cp->m_origin_uri_string.c_str())) {
		if (!strcmp(input_title_utf8, cp->m_origin_title_string.c_str())) {
			if (cp->m_folder_id == cp->m_origin_folder_id) {
				elm_object_disabled_set(cp->m_btn_save, EINA_TRUE);
				elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
			}
		}
	} else {
		elm_object_disabled_set(cp->m_btn_save, EINA_FALSE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
	}

	if (elm_object_focus_get(obj))
		cp->show_notification(text, obj, TITLE_INPUT_ENTRY_MAX_COUNT);

	if (text)
		free(text);
	if (input_title_utf8)
		free(input_title_utf8);
	if (input_uri_utf8)
		free(input_uri_utf8);

	if (elm_object_focus_get(obj) && cp->m_input_uri_callback_data.it) {
		if (elm_entry_is_empty(obj))
			elm_object_item_signal_emit(cp->m_input_uri_callback_data.it, "elm,state,eraser,hide", "");
		else
			elm_object_item_signal_emit(cp->m_input_uri_callback_data.it, "elm,state,eraser,show", "");
	}
}

Eina_Bool bookmark_add_view::_is_empty(const char* entry_text)
{
	BROWSER_LOGD("");
	if (!entry_text)
		return EINA_TRUE;

	unsigned int len = strlen(entry_text);
	if (len == 0)
		return EINA_TRUE;

	unsigned int space_count = 0;
	for (unsigned int i = 0 ; i < len ; i++) {
		if (entry_text[i] == ' ')
			space_count++;
	}
	if (space_count == len)
		return EINA_TRUE;

	return EINA_FALSE;
}

void bookmark_add_view::__uri_entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	bookmark_add_view *cp = (bookmark_add_view *)data;

	if (cp->m_input_uri_callback_data.it) {
		if (!elm_entry_is_empty(obj)) {
			elm_object_item_signal_emit(cp->m_input_uri_callback_data.it, "elm,state,eraser,show", "");
			if (cp ->m_input_title_string.empty())
				elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
			else
				elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
		}
		elm_object_item_signal_emit(cp->m_input_uri_callback_data.it, "elm,state,rename,hide", "");
	}
}

void bookmark_add_view::__uri_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	bookmark_add_view *cp = (bookmark_add_view *)data;
	if (cp->m_input_uri_callback_data.it) {
		elm_object_item_signal_emit(cp->m_input_uri_callback_data.it, "elm,state,eraser,hide", "");
		elm_object_item_signal_emit(cp->m_input_uri_callback_data.it, "elm,state,rename,show", "");
	}
}

void bookmark_add_view::__uri_entry_eraser_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	bookmark_add_view *cp = (bookmark_add_view *)data;
	if (cp->m_input_uri_callback_data.it) {
		Evas_Object *entry = elm_object_item_part_content_get(cp->m_input_uri_callback_data.it, "elm.icon.entry");
		elm_object_focus_set(entry, EINA_TRUE); // After button is clicked, entry should get focus again.
		elm_entry_entry_set(entry, "");
	}
}

void bookmark_add_view::__editfield_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *editfield = (Evas_Object *)data;

	if (!elm_entry_is_empty(obj) && elm_object_focus_get(obj))
		elm_object_signal_emit(editfield, "elm,action,show,button", "");
	else
		elm_object_signal_emit(editfield, "elm,action,hide,button", "");
}

void bookmark_add_view::__entry_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *entry = (Evas_Object *)data;

	elm_entry_entry_set(entry, "");
}

char *bookmark_add_view::__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	RETV_MSG_IF(!data, NULL, "data is NULL");
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	bookmark_add_view::menu_type type = callback_data->type;
	bookmark_add_view *cp = (bookmark_add_view *)callback_data->cp;

	if (!strcmp(part, "elm.text") ||!strcmp(part, "elm.text.main.left") ||!strcmp(part, "elm.text.main") ) {
		switch (type) {
		case FOLDER_SELECT_MENU:
		{
			bookmark *bm = m_browser->get_bookmark();
			bookmark_item item;
			bm->get_item_by_id(cp->m_folder_id, &item);
			BROWSER_LOGD("[%s]", item.get_title());
			if ((item.get_id()) == bm->get_root_folder_id()) {
				return strdup(BR_STRING_BOOKMARKS);
			} else {
				return strdup(item.get_title());
			}
			break;
		}

		case FOLDER_GROUP_TITLE:
			return strdup(BR_STRING_LOCATION);

		default:
			break;
		}
	}
	return NULL;
}

char *bookmark_add_view::__genlist_get_title_cb(void *data, Evas_Object *obj, const char *part)
{
	RETV_MSG_IF(!data, NULL, "data is NULL");
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	bookmark_add_view::menu_type type = callback_data->type;

	switch (type) {
	case TITLE_INPUT_FIELD:
		return strdup(BR_STRING_BODY_TITLE);
	case URI_INPUT_FIELD:
		return strdup(BR_STRING_URL);
	default:
		break;
	}

	return NULL;
}

Evas_Object *bookmark_add_view::__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	RETV_MSG_IF(!data, NULL, "data is NULL");
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	bookmark_add_view *cp = (bookmark_add_view *)callback_data->cp;
	bookmark_add_view::menu_type type = callback_data->type;

	if (!strcmp(part, "elm.icon.1")) {
		switch (type) {
		case FOLDER_SELECT_MENU:
		{
			BROWSER_LOGD("[%s] FOLDER_SELECT_MENU", part);
			Evas_Object *content = elm_layout_add(obj);
			if (content == NULL)
				return NULL;
			elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
			Evas_Object *folder_icon = elm_icon_add(content);
			elm_image_file_set(folder_icon, bookmark_view_edj_path, "contacts_ic_folder.png");
			elm_layout_content_set(content, "elm.swallow.content", folder_icon);
			return content;
		}

		default:
			break;
		}
	}
	else if (!strcmp(part, "elm.icon.entry")) {
		Evas_Object *editfield = elm_layout_add(obj);
		elm_layout_theme_set(editfield, "layout", "editfield", "singleline");
		evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL, 0.0);
		evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND, 0.0);

		Evas_Object *entry = elm_entry_add(editfield);
		elm_entry_single_line_set(entry, EINA_TRUE);
		elm_entry_scrollable_set(entry, EINA_TRUE);

		if (!entry)
			return NULL;
#if defined(HW_MORE_BACK_KEY)
		eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
#endif
		elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_NEXT);
		elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
		Evas_Object *bt_clear = elm_object_part_content_get(entry, "elm.swallow.clear");
		elm_access_info_set(bt_clear, ELM_ACCESS_INFO, BR_STRING_CLEAR_ALL);
		switch (type) {
		case TITLE_INPUT_FIELD:
		{
			BROWSER_LOGD("[%s] TITLE_INPUT_FIELD", part);
			elm_object_part_text_set(entry, "elm.guide", BR_STRING_BODY_TITLE);
			evas_object_smart_callback_add(entry,
							"changed", __title_entry_changed_cb, cp);
			evas_object_smart_callback_add(entry,
							"preedit,changed", __title_entry_changed_cb, cp);
			evas_object_smart_callback_add(entry, "changed", __editfield_changed_cb, editfield);
			evas_object_smart_callback_add(entry,
						"activated", __entry_next_key_cb, cp);
			evas_object_smart_callback_add(entry,
						"focused", __title_entry_focused_cb, cp);
			evas_object_smart_callback_add(entry,
						"unfocused", __title_entry_unfocused_cb, cp);
			elm_entry_prediction_allow_set(entry, EINA_FALSE);

			BROWSER_LOGD("m_input_title_string: %s", cp->m_input_title_string.c_str());
			char *mark_up = elm_entry_utf8_to_markup(cp->m_input_title_string.c_str());
			RETV_MSG_IF(!mark_up, NULL, "elm_entry_utf8_to_markup failed");

			if (strlen(mark_up) == 0) {
				char *temp = elm_entry_utf8_to_markup(cp->m_input_uri_string.c_str()) ;
				RETV_MSG_IF(!temp, NULL, "elm_entry_utf8_to_markup failed");
				char *ptr = strstr(temp,"://");
				if (ptr != NULL)
					mark_up = ptr + 3;	// to remove protocols with :// and assign uri to the title
				else
					mark_up = temp;
				elm_entry_entry_set(entry, mark_up);
				if (temp)
					free(temp);
			} else {
				elm_entry_entry_set(entry, mark_up);
				if (mark_up)
					free(mark_up);
			}
			elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &(cp->m_entry_limit_size));

			elm_object_part_content_set(editfield, "elm.swallow.content", entry);
			Evas_Object *button = elm_button_add(editfield);
			elm_object_style_set(button, "editfield_clear");
			evas_object_smart_callback_add(button, "clicked", __entry_clear_button_clicked_cb, entry);

			elm_object_part_content_set(editfield, "elm.swallow.button", button);

			cp->m_title_edit_field = entry;
			return editfield;
			break;
		}
		case URI_INPUT_FIELD:
		{
			BROWSER_LOGD("[%s] URI_INPUT_FIELD", part);
			elm_object_part_text_set(entry, "elm.guide", BR_STRING_BODY_WEB_ADDRESS);
			elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

			evas_object_smart_callback_add(entry,
							"changed", __uri_entry_changed_cb, cp);
			evas_object_smart_callback_add(entry,
							"preedit,changed", __uri_entry_changed_cb, cp);
			evas_object_smart_callback_add(entry,
						"activated", __entry_enter_key_cb, cp);
			evas_object_smart_callback_add(entry, "changed", __editfield_changed_cb, editfield);
			evas_object_smart_callback_add(entry,
						"focused", __uri_entry_focused_cb, cp);
			evas_object_smart_callback_add(entry,
						"unfocused", __uri_entry_unfocused_cb, cp);
			elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_URL);
			elm_entry_prediction_allow_set(entry, EINA_FALSE);

			BROWSER_SECURE_LOGD("m_input_uri_string: %s", cp->m_input_uri_string.c_str());
			char *mark_up = elm_entry_utf8_to_markup(cp->m_input_uri_string.c_str());
			RETV_MSG_IF(!mark_up, NULL, "elm_entry_utf8_to_markup failed");

			elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &(cp->m_entry_limit_size));
			elm_entry_entry_set(entry, mark_up);

			elm_object_part_content_set(editfield, "elm.swallow.content", entry);
			Evas_Object *button = elm_button_add(editfield);
			elm_object_style_set(button, "editfield_clear");
			evas_object_smart_callback_add(button, "clicked", __entry_clear_button_clicked_cb, entry);

			elm_object_part_content_set(editfield, "elm.swallow.button", button);

			if (mark_up)
				free(mark_up);

			cp->m_uri_edit_field = entry;
			return editfield;
			break;
		}
		default:
			break;
		}
	}
	else if (!strcmp(part, "elm.icon.eraser")) {
		Evas_Object *btn = elm_button_add(obj);
		elm_object_style_set(btn, "editfield_clear"); // Make "X" marked button by changing style.
		switch (type) {
		case TITLE_INPUT_FIELD:
			evas_object_smart_callback_add(btn, "clicked", __title_entry_eraser_clicked_cb, cp);
			break;
		case URI_INPUT_FIELD:
			evas_object_smart_callback_add(btn, "clicked", __uri_entry_eraser_clicked_cb, cp);
			break;
		default:
			break;
		}
		return btn;
	}

	return NULL;
}

void bookmark_add_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	genlist_callback_data *callback_data = (genlist_callback_data *)elm_object_item_data_get(it);
	bookmark_add_view *cp = (bookmark_add_view *)callback_data->cp;
	bookmark_add_view::menu_type type = callback_data->type;

	elm_genlist_item_selected_set(it, EINA_FALSE);

	switch (type) {
	case FOLDER_SELECT_MENU:
		BROWSER_LOGD("FOLDER_SELECT_MENU");
		if (!m_browser->is_tts_enabled())
			elm_object_focus_set(cp->m_btn_save, EINA_TRUE);
		evas_object_freeze_events_set(cp->m_genlist, EINA_TRUE);
		m_browser->create_bookmark_select_folder_view(cp->__select_folder_cb, cp)->show();
		break;

	default:
		break;
	}
}

void bookmark_add_view::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	RET_MSG_IF(!event_info, "event_info is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	const Elm_Genlist_Item_Class *itc;
	itc = elm_genlist_item_item_class_get(it);
	if(itc && !strcmp(itc->item_style, "dialogue/separator"))
		return;

	genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
	RET_MSG_IF(!cb_data, "cb_data is NULL");

	if (cb_data->type == TITLE_INPUT_FIELD) {
		elm_object_item_signal_emit(it, "elm,state,top", "");
		elm_object_item_access_unregister(it);
	} else if (cb_data->type == URI_INPUT_FIELD) {
		elm_object_item_signal_emit(it, "elm,state,bottom", "");
		elm_object_item_access_unregister(it);
	}
}

void bookmark_add_view::__select_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	bookmark_add_view *cp = (bookmark_add_view *)data;
	bookmark_item *bookmark_item_data = NULL;
	evas_object_freeze_events_set(cp->m_genlist, EINA_FALSE);
	if (event_info)
		bookmark_item_data = (bookmark_item *)event_info;
	if (bookmark_item_data) {
		cp->m_folder_id = bookmark_item_data->get_id();
		if (cp->m_genlist)
			elm_genlist_item_update(cp->m_select_folder_callback_data.it);
	}
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
	cp->set_edtifield_focus_allow_set(EINA_TRUE);
	if (cp->m_title_edit_field)
		elm_object_focus_set(cp->m_title_edit_field,EINA_TRUE);

	// (disable | enable) save btn
	char *input_title_utf8 = elm_entry_markup_to_utf8(cp->m_input_title_string.c_str());
	if(!input_title_utf8)
		return;
	char *input_uri_utf8= elm_entry_markup_to_utf8(cp->m_input_uri_string.c_str());
	if(!input_uri_utf8){
		free(input_title_utf8);
		return;
	}
	if (cp->m_input_title_string.empty())
		elm_object_disabled_set(cp->m_btn_save, EINA_TRUE);
	else if (cp->m_input_uri_string.empty())
		elm_object_disabled_set(cp->m_btn_save, EINA_TRUE);
	else if (cp->m_edit_mode && cp->m_folder_id == cp->m_origin_folder_id && !strcmp(input_title_utf8, cp->m_origin_title_string.c_str()) && !strcmp(input_uri_utf8, cp->m_origin_uri_string.c_str()))
		elm_object_disabled_set(cp->m_btn_save, EINA_TRUE);
	else
		elm_object_disabled_set(cp->m_btn_save, EINA_FALSE);

	if (input_title_utf8)
		free(input_title_utf8);
	if (input_uri_utf8)
		free(input_uri_utf8);
}

void bookmark_add_view::__timer_popup_expired_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	bookmark_add_view *cp = (bookmark_add_view *)data;

	if (cp->m_genlist && cp->m_title_edit_field)
		elm_object_focus_set(cp->m_title_edit_field, EINA_TRUE);
}

void bookmark_add_view::__save_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_add_view *cp = (bookmark_add_view *)data;

	int ret = cp->_save_bookmark();
	if (ret == 0) {
		cp->show_msg_popup(BR_STRING_ALREADY_EXISTS, 3, __timer_popup_expired_cb, data);
		return;
	} else if (ret == -1) {
		if (!m_browser->get_bookmark()->get_memory_full())
			cp->show_noti_popup(BR_STRING_FAILED);
	} else {
		cp->show_noti_popup(BR_STRING_SAVED_TO_BOOKMARK);
	}
	cp->_back_to_previous_view();
}

void bookmark_add_view::__cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	bookmark_add_view *cp = (bookmark_add_view *)data;

	cp->_back_to_previous_view();
}

void bookmark_add_view::__naviframe_pop_finished_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	bookmark_add_view *cp = (bookmark_add_view *)data;

	if (cp->m_naviframe_item != elm_naviframe_top_item_get(cp->m_naviframe))
		return;

	if (!m_browser->is_tts_enabled()) {
		if (cp->m_input_title_callback_data.it != NULL) {
			Evas_Object *entry = elm_object_item_part_content_get(
						cp->m_input_title_callback_data.it, "elm.icon.entry");
			elm_object_focus_set(entry, EINA_TRUE);
			elm_entry_cursor_end_set(entry);
		}
	}

	m_browser->delete_bookmark_select_folder_view();
}

int bookmark_add_view::_save_bookmark(void)
{
	BROWSER_LOGD("");
	int saved_bookmark_id = -1;

	char *input_title_utf8 = elm_entry_markup_to_utf8(m_input_title_string.c_str());
	if (!input_title_utf8) {
		BROWSER_LOGD("input_title_utf8 is null");
		return -1;
	}
	char *input_uri_utf8= elm_entry_markup_to_utf8(m_input_uri_string.c_str());
	if (!input_uri_utf8) {
		BROWSER_LOGD("input_uri_utf8 is null");
		free(input_title_utf8);
		return -1;
	}

	int ret = 0;
	if (m_edit_mode) {
		if ((!strcmp(input_title_utf8, m_bookmark_edit_title)) &&
				(!strcmp(input_uri_utf8, m_bookmark_edit_url)) && (m_folder_id == m_origin_folder_id)) {
			free(input_uri_utf8);
			free(input_title_utf8);
			return 0;
		}

		if (strcmp(input_uri_utf8, m_bookmark_edit_url))
			ret = m_browser->get_bookmark()->update_bookmark_notify(m_bookmark_id,
					input_title_utf8,
					input_uri_utf8,
					m_folder_id,
					-1, EINA_TRUE, EINA_TRUE
					);
		else
			ret = m_browser->get_bookmark()->update_bookmark_notify(m_bookmark_id,
						input_title_utf8,
						input_uri_utf8,
						m_folder_id,
						-1, EINA_TRUE
						);

		if (m_browser->get_bookmark()->get_memory_full())
			m_browser->get_browser_view()->show_noti_popup(BR_STRING_DISK_FULL);


		/* get favicon from history if there is same URI */
		if (ret > 0) {
			if(m_folder_id != m_origin_folder_id)
				m_browser->get_bookmark()->set_last_sequence(m_bookmark_id);
			Evas_Object *favicon = m_browser->get_history()->get_history_favicon(input_uri_utf8);
			m_browser->get_bookmark()->set_favicon(m_bookmark_id, favicon);
			if (favicon)
				evas_object_del(favicon);

			Evas_Object *webicon = m_browser->get_history()->get_history_webicon(input_uri_utf8);
			m_browser->get_bookmark()->set_webicon(m_bookmark_id, webicon);
			if (webicon)
				evas_object_del(webicon);
		}
	} else {
		ret = m_browser->get_bookmark()->save_bookmark(
							input_title_utf8,
							input_uri_utf8,
							&saved_bookmark_id,
							m_folder_id
							);
		if (m_browser->get_bookmark()->get_memory_full())
			m_browser->get_browser_view()->show_noti_popup(BR_STRING_DISK_FULL);

		/* get favicon from history if there is same URI */
		if (ret > 0) {
			Evas_Object *favicon = m_browser->get_history()->get_history_favicon(input_uri_utf8);
			m_browser->get_bookmark()->set_favicon(saved_bookmark_id, favicon);
			m_bookmark_id = saved_bookmark_id;
			if (favicon)
				evas_object_del(favicon);

			Evas_Object *webicon = m_browser->get_history()->get_history_webicon(input_uri_utf8);
			m_browser->get_bookmark()->set_webicon(m_bookmark_id, webicon);
			if (webicon)
				evas_object_del(webicon);
		}
	}

	if (input_uri_utf8)
		free(input_uri_utf8);
	if (input_title_utf8)
		free(input_title_utf8);

	if (ret < 0) {
		BROWSER_LOGD("bookmark add/edit is failed");
		return -1;
	}
	if (ret == 0) {
		BROWSER_LOGD("same bookmark is already exist");
		return 0;
	}
	return 1;
}

void bookmark_add_view::_back_to_previous_view(void)
{
	BROWSER_LOGD("");

	if (elm_naviframe_bottom_item_get(m_naviframe) != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop(m_naviframe);
}

void bookmark_add_view::__genlist_free_cb(void *data, Evas *e, Evas_Object *genlist, void *ei)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!genlist, "genlist is NULL");
	RET_MSG_IF(!data, "data is NULL");
	bookmark_add_view *cp = (bookmark_add_view *)data;

	Elm_Genlist_Item_Class *itc_separator = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_separator");
	if (itc_separator) elm_genlist_item_class_free(itc_separator);

	Elm_Genlist_Item_Class *itc_title = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_title");
	if (itc_title) elm_genlist_item_class_free(itc_title);

	Elm_Genlist_Item_Class *itc_uri = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_uri");
	if (itc_uri) elm_genlist_item_class_free(itc_uri);

	Elm_Genlist_Item_Class *itc_group_title = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_group_title");
	if (itc_group_title) elm_genlist_item_class_free(itc_group_title);

	Elm_Genlist_Item_Class *itc_folder = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_folder");
	if (itc_folder) elm_genlist_item_class_free(itc_folder);

	Elm_Genlist_Item_Class *itc_qa = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_qa");
	if (itc_qa) elm_genlist_item_class_free(itc_qa);

	if (cp->m_genlist)
		cp->m_genlist = NULL;
}

Evas_Object *bookmark_add_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	RETV_MSG_IF(!genlist, NULL, "elm_genlist_add failed");
	elm_genlist_realization_mode_set(genlist, EINA_TRUE);
	// To use multiline textblock/entry/editfield in genlist, set height_for_width mode
	// then the item's height is calculated while the item's width fits to genlist width.
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, NULL);
	evas_object_smart_callback_add(genlist, "language,changed", common_view::__genlist_lang_changed, NULL);

	/* separator */
	Elm_Genlist_Item_Class *itc_separator = elm_genlist_item_class_new();
	if(!itc_separator)
		return NULL;
	itc_separator->item_style = "dialogue/separator";
	itc_separator->func.text_get = NULL;
	itc_separator->func.content_get = NULL;
	itc_separator->func.state_get = NULL;
	itc_separator->func.del = NULL;
	evas_object_data_set(genlist, "itc_separator", itc_separator);

	Elm_Object_Item *it = elm_genlist_item_append(genlist, itc_separator, NULL, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	Elm_Genlist_Item_Class *itc_title = elm_genlist_item_class_new();
	if(!itc_title)
		return NULL;
	itc_title->item_style = "entry.sub";
	itc_title->func.text_get = __genlist_get_title_cb;
	itc_title->func.content_get = __genlist_get_content_cb;
	itc_title->func.state_get = NULL;
	itc_title->func.del = NULL;
	evas_object_data_set(genlist, "itc_title", itc_title);

	m_input_title_callback_data.type = TITLE_INPUT_FIELD;
	m_input_title_callback_data.cp = (void *)this;
	m_input_title_callback_data.it = elm_genlist_item_append(genlist,
					itc_title, &m_input_title_callback_data, NULL,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);

	Elm_Genlist_Item_Class *itc_uri = elm_genlist_item_class_new();
	if(!itc_uri)
		return NULL;
	itc_uri->item_style = "entry.sub";
	itc_uri->func.text_get = __genlist_get_title_cb;
	itc_uri->func.content_get = __genlist_get_content_cb;
	itc_uri->func.state_get = NULL;
	itc_uri->func.del = NULL;
	evas_object_data_set(genlist, "itc_uri", itc_uri);

	m_input_uri_callback_data.type = URI_INPUT_FIELD;
	m_input_uri_callback_data.cp = (void *)this;
	m_input_uri_callback_data.it = elm_genlist_item_append(genlist,
					itc_uri, &m_input_uri_callback_data, NULL,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
	it = elm_genlist_item_append(genlist, itc_separator, NULL, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	Elm_Genlist_Item_Class *itc_group_title = elm_genlist_item_class_new();
	if(!itc_group_title)
		return NULL;
	itc_group_title->item_style = "groupindex";
	itc_group_title->func.text_get = __genlist_get_text_cb;
	itc_group_title->func.content_get = NULL;
	itc_group_title->func.state_get = NULL;
	itc_group_title->func.del = NULL;
	evas_object_data_set(genlist, "itc_group_title", itc_group_title);

	m_group_title_callback_data.type = FOLDER_GROUP_TITLE;
	m_group_title_callback_data.cp = (void *)this;
	m_group_title_callback_data.it = elm_genlist_item_append(genlist,
					itc_group_title, &m_group_title_callback_data, NULL,
					ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(m_group_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	Elm_Genlist_Item_Class *itc_folder = elm_genlist_item_class_new();
	if(!itc_folder)
		return NULL;
	itc_folder->item_style = "1line";
	itc_folder->func.text_get = __genlist_get_text_cb;
	itc_folder->func.content_get = __genlist_get_content_cb;
	itc_folder->func.state_get = NULL;
	itc_folder->func.del = NULL;
	evas_object_data_set(genlist, "itc_folder", itc_folder);

	m_select_folder_callback_data.type = FOLDER_SELECT_MENU;
	m_select_folder_callback_data.cp = (void *)this;
	m_select_folder_callback_data.it = elm_genlist_item_append(genlist,
					itc_folder, &m_select_folder_callback_data, NULL,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);

	evas_object_event_callback_add(genlist, EVAS_CALLBACK_FREE, __genlist_free_cb, this);

	return genlist;
}

void bookmark_add_view::set_edtifield_focus_allow_set(Eina_Bool enable)
{
	if (m_title_edit_field)
		elm_object_focus_allow_set(m_title_edit_field, enable);
	if (m_uri_edit_field)
		elm_object_focus_allow_set(m_uri_edit_field, enable);
}

void bookmark_add_view::back(void)
{
	BROWSER_LOGD("");
	_back_to_previous_view();
}

void bookmark_add_view::show()
{
	BROWSER_LOGD("");

	m_genlist = _create_genlist(m_naviframe);

	if (m_edit_mode) {
		m_naviframe_item = elm_naviframe_item_push(m_naviframe,
							"IDS_BR_HEADER_EDIT_BOOKMARK", NULL, NULL, m_genlist, NULL);//BR_STRING_EDIT
		elm_object_item_domain_text_translatable_set(m_naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);
		m_bookmark_edit_title = strdup(m_input_title_string.c_str());
		m_bookmark_edit_url = strdup(m_input_uri_string.c_str());
	} else {
		m_naviframe_item = elm_naviframe_item_push(m_naviframe,
					"IDS_BR_OPT_ADD_BOOKMARK", NULL, NULL, m_genlist, NULL);//BR_STRING_ADD_BOOKMARK
		elm_object_item_domain_text_translatable_set(m_naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);
	}

	evas_object_smart_callback_add(m_naviframe,
					"transition,finished",
					__naviframe_pop_finished_cb, this);
#if defined(HW_MORE_BACK_KEY)
	m_btn_cancel = elm_button_add(m_naviframe);
	if (!m_btn_cancel) return;
	elm_object_style_set(m_btn_cancel, "naviframe/title_cancel");
	//set_trans_text_to_object(m_btn_cancel, "IDS_BR_SK_CANCEL", NULL);
	evas_object_smart_callback_add(m_btn_cancel, "clicked", __cancel_btn_clicked_cb, this);
	elm_object_item_part_content_set(m_naviframe_item, "title_left_btn", m_btn_cancel);

	m_btn_save = elm_button_add(m_naviframe);
	if (!m_btn_save) return;
	elm_object_style_set(m_btn_save, "naviframe/title_done");
	//set_trans_text_to_object(m_btn_save, "IDS_BR_SK_SAVE", NULL);
	evas_object_smart_callback_add(m_btn_save, "clicked", __save_btn_clicked_cb, this);
	elm_object_item_part_content_set(m_naviframe_item, "title_right_btn", m_btn_save);
	if (m_edit_mode) {
		elm_object_disabled_set(m_btn_save, EINA_TRUE);
	}
	// Add HW key callback.
	eext_object_event_callback_add(m_genlist, EEXT_CALLBACK_BACK, __cancel_btn_clicked_cb, this);
#else
	m_btn_save = elm_button_add(m_naviframe);
	if (!m_btn_save) return;
	elm_object_text_set(m_btn_save, BR_STRING_SAVE);
	elm_object_style_set(m_btn_save, "naviframe/toolbar/default");
	elm_object_item_part_content_set(m_naviframe_item, "toolbar_button1", m_btn_save);
	evas_object_smart_callback_add(m_btn_save, "clicked", __save_btn_clicked_cb, this);
	if (m_edit_mode) {
		elm_object_disabled_set(m_btn_save, EINA_TRUE);
	}
	evas_object_show(m_btn_save);
#endif
}

