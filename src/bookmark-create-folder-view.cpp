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
#include "multiwindow-view.h"

bookmark_create_folder_view::bookmark_create_folder_view(
						Evas_Smart_Cb cb_func, void *cb_data, int parent_id)
:
	m_genlist(NULL)
	,m_back_button(NULL)
	,m_btn_save(NULL)
	,m_titlebar_btn_save(NULL)
	,m_titlebar_btn_back(NULL)
	,m_naviframe_item(NULL)
	,m_itc_title(NULL)
	,m_itc_folder(NULL)
	,m_folder_id(parent_id)
	,m_saved_id(0)
	,m_cb_func(cb_func)
	,m_cb_data(cb_data)
{
	BROWSER_LOGD("parent_id: %d", parent_id);
}

bookmark_create_folder_view::~bookmark_create_folder_view(void)
{
	BROWSER_LOGD("");

	evas_object_smart_callback_del(m_conformant, "virtualkeypad,state,on", __ime_show_cb);
	evas_object_smart_callback_del(m_conformant, "virtualkeypad,state,off", __ime_hide_cb);
	evas_object_smart_callback_del(m_conformant, "clipboard,state,on", __ime_show_cb);
	evas_object_smart_callback_del(m_conformant, "clipboard,state,off", __ime_hide_cb);

	evas_object_smart_callback_del(m_naviframe,
						"transition,finished",
						__naviframe_pop_finished_cb);
	if (m_itc_title)
		elm_genlist_item_class_free(m_itc_title);
	if (m_itc_folder)
		elm_genlist_item_class_free(m_itc_folder);
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
		elm_object_disabled_set(view_this->m_titlebar_btn_save, EINA_TRUE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
	} else {
		elm_object_disabled_set(view_this->m_btn_save, EINA_FALSE);
		elm_object_disabled_set(view_this->m_titlebar_btn_save, EINA_FALSE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
	}

	if (text)
		free(text);

	if (elm_object_focus_get(obj)) {
		if (elm_entry_is_empty(obj))
			elm_object_item_signal_emit(view_this->m_input_title_callback_data.it, "elm,state,eraser,hide", "");
		else
			elm_object_item_signal_emit(view_this->m_input_title_callback_data.it, "elm,state,eraser,show", "");
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
	if (!data)
		return;

	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;
	if (!elm_entry_is_empty(obj))
		elm_object_item_signal_emit(cp->m_input_title_callback_data.it, "elm,state,eraser,show", "");
	elm_object_item_signal_emit(cp->m_input_title_callback_data.it, "elm,state,rename,hide", "");
}

void bookmark_create_folder_view::__title_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;
	elm_object_item_signal_emit(cp->m_input_title_callback_data.it, "elm,state,eraser,hide", "");
	elm_object_item_signal_emit(cp->m_input_title_callback_data.it, "elm,state,rename,show", "");
}

void bookmark_create_folder_view::__title_entry_eraser_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;
	Evas_Object *entry = elm_object_item_part_content_get(cp->m_input_title_callback_data.it, "elm.icon.entry");
	elm_object_focus_set(entry, EINA_TRUE); // After button is clicked, entry should get focus again.
	elm_entry_entry_set(entry, "");
}

char *bookmark_create_folder_view::__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	bookmark_create_folder_view::menu_type type = callback_data->type;
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)callback_data->cp;

	if (!strcmp(part, "elm.text")) {
		switch (type) {
		case TITLE_INPUT_FIELD:
			break;
		case FOLDER_SELECT_MENU:
		{
			bookmark *bm = m_browser->get_bookmark();
			bookmark_item item;
			bm->get_item_by_id(cp->m_folder_id, &item);
			BROWSER_LOGD("[%s]", item.get_title());
			if ((item.get_id()) == bm->get_root_folder_id()) {
				return elm_entry_utf8_to_markup(BR_STRING_BOOKMARKS);
			} else {
				return elm_entry_utf8_to_markup(item.get_title());
			}
			break;
		}
		default:
			break;
		}
	}

	return NULL;
}

Evas_Object *bookmark_create_folder_view::__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	bookmark_create_folder_view::menu_type type = callback_data->type;
	bookmark_create_folder_view *view_this = (bookmark_create_folder_view *)callback_data->cp;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.edit")) {
		switch (type) {
		case FOLDER_SELECT_MENU:
		{
			BROWSER_LOGD("[%s] FOLDER_SELECT_MENU", part);
			Evas_Object *folder_icon = elm_icon_add(obj);
			if (folder_icon == NULL)
				break;
			elm_icon_standard_set(folder_icon, browser_img_dir"/I01_icon_folder.png");
			evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return folder_icon;
			break;
		}
		default:
			break;
		}
	} else if (!strcmp(part, "elm.icon.entry")) {
		Evas_Object *entry = elm_entry_add(obj);
		if (!entry)
			return NULL;
		elm_entry_single_line_set(entry, EINA_TRUE);
		elm_entry_scrollable_set(entry, EINA_TRUE);
		switch (type) {
		case TITLE_INPUT_FIELD:
		{
			BROWSER_LOGD("[%s] TITLE_INPUT_FIELD", part);
			elm_object_part_text_set(entry, "elm.guide", BR_STRING_ENTER_FOLDER_NAME);

			evas_object_smart_callback_add(entry,
							"changed", __title_entry_changed_cb, view_this);
			evas_object_smart_callback_add(entry,
							"preedit,changed", __title_entry_changed_cb, view_this);
			evas_object_smart_callback_add(entry,
						"activated", __title_entry_enter_key_cb, view_this);
			evas_object_smart_callback_add(entry,
						"focused", __title_entry_focused_cb, view_this);
			evas_object_smart_callback_add(entry,
						"unfocused", __title_entry_unfocused_cb, view_this);

			BROWSER_LOGD("m_input_title_string: %s", view_this->m_input_title_string.c_str());
			elm_entry_entry_set(entry, view_this->m_input_title_string.c_str());

			return entry;
			break;
		}
		default:
			break;
		}
	} else if (!strcmp(part, "elm.icon.eraser")) {
		Evas_Object *btn = elm_button_add(obj);
		elm_object_style_set(btn, "editfield_clear"); // Make "X" marked button by changing style.
		switch (type) {
		case TITLE_INPUT_FIELD:
			evas_object_smart_callback_add(btn, "clicked", __title_entry_eraser_clicked_cb, view_this);
			break;
		default:
			break;
		}
		return btn;
	}
	return NULL;
}

void bookmark_create_folder_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	genlist_callback_data *callback_data = (genlist_callback_data *)elm_object_item_data_get(it);
	//bookmark_create_folder_view *cp = (bookmark_create_folder_view *)callback_data->cp;
	bookmark_create_folder_view::menu_type type = callback_data->type;

	elm_genlist_item_selected_set(it, EINA_FALSE);

	switch (type) {
	case TITLE_INPUT_FIELD:
		BROWSER_LOGD("TITLE_INPUT_FIELD");
		break;
	case FOLDER_SELECT_MENU:
		BROWSER_LOGD("FOLDER_SELECT_MENU");
		//m_browser->create_bookmark_select_folder_view(cp->__select_folder_cb, cp, EINA_TRUE)->show();
		break;
	default:
		break;
	}
}

void bookmark_create_folder_view::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);

	EINA_SAFETY_ON_NULL_RETURN(cb_data);
	if (cb_data->type == TITLE_INPUT_FIELD)
		elm_object_item_signal_emit(it, "elm,state,top", "");
	else if (cb_data->type == FOLDER_SELECT_MENU)
		elm_object_item_signal_emit(it, "elm,state,bottom", "");
}

void bookmark_create_folder_view::__select_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)event_info;

	cp->m_folder_id = bookmark_item_data->get_id();
}

void bookmark_create_folder_view::__save_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	if ((cp->_save_folder()) == 0) {
		cp->show_msg_popup(BR_STRING_ALREADY_EXISTS);
		return;
	}

	if (cp->m_cb_func) {
		cp->m_cb_func(cp->m_cb_data, NULL, NULL);
	}

	cp->_back_to_previous_view();
}

void bookmark_create_folder_view::__back_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	cp->_back_to_previous_view();
}

void bookmark_create_folder_view::__naviframe_pop_finished_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	if (cp->m_naviframe_item != elm_naviframe_top_item_get(cp->m_naviframe))
		return;

	m_browser->delete_bookmark_select_folder_view();

	if (cp->m_input_title_callback_data.it != NULL) {
		Evas_Object *entry = elm_object_item_part_content_get(
						cp->m_input_title_callback_data.it, "elm.icon.entry");
		elm_object_focus_set(entry, EINA_TRUE);
	}
}

void bookmark_create_folder_view::__ime_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	EINA_SAFETY_ON_NULL_RETURN(data);
	Elm_Object_Item *it = (Elm_Object_Item*)data;
	elm_object_item_signal_emit(it, "elm,state,sip,shown", "");
}

void bookmark_create_folder_view::__ime_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	EINA_SAFETY_ON_NULL_RETURN(data);
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;
	elm_object_item_signal_emit(cp->m_naviframe_item, "elm,state,sip,hidden", "");
}

int bookmark_create_folder_view::_save_folder(void)
{
	BROWSER_LOGD("");
	int saved_folder_id;
	int ret = 0;
	ret = m_browser->get_bookmark()->save_folder(
						m_input_title_string.c_str(),
						&saved_folder_id,
						m_folder_id
						);

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

void bookmark_create_folder_view::_back_to_previous_view(void)
{
	BROWSER_LOGD("");
	if (elm_naviframe_bottom_item_get(m_naviframe) !=
						elm_naviframe_top_item_get(m_naviframe))
					elm_naviframe_item_pop(m_naviframe);
}

Evas_Object *bookmark_create_folder_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	// To use multiline textblock/entry/editfield in genlist, set height_for_width mode
	// then the item's height is calculated while the item's width fits to genlist width.
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, NULL);

	m_itc_title = elm_genlist_item_class_new();
	memset(m_itc_title, 0x00, sizeof(Elm_Genlist_Item_Class));

	m_itc_title->item_style = "dialogue/editfield";
	m_itc_title->func.content_get = __genlist_get_content_cb;
	m_itc_title->func.state_get = NULL;
	m_itc_title->func.del = NULL;

	m_input_title_callback_data.type = TITLE_INPUT_FIELD;
	m_input_title_callback_data.cp = (void *)this;

	m_input_title_callback_data.it = elm_genlist_item_append(genlist,
					m_itc_title, &m_input_title_callback_data, NULL,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
	elm_genlist_item_select_mode_set(m_input_title_callback_data.it,
						ELM_OBJECT_SELECT_MODE_NONE);

	m_itc_folder = elm_genlist_item_class_new();
	memset(m_itc_folder, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_itc_folder->item_style = "dialogue/1text.1icon.2";
	m_itc_folder->func.text_get = __genlist_get_text_cb;
	m_itc_folder->func.content_get = __genlist_get_content_cb;
	m_itc_folder->func.state_get = NULL;
	m_itc_folder->func.del = NULL;

	m_select_folder_callback_data.type = FOLDER_SELECT_MENU;
	m_select_folder_callback_data.cp = (void *)this;
	m_select_folder_callback_data.it = elm_genlist_item_append(genlist,
					m_itc_folder, &m_select_folder_callback_data, NULL,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
	return genlist;
}

void bookmark_create_folder_view::show()
{
	BROWSER_LOGD("");
	m_genlist = _create_genlist(m_naviframe);

	m_naviframe_item = elm_naviframe_item_push(m_naviframe,
						BR_STRING_CREATE_FOLDER, NULL, NULL, m_genlist, NULL);

	evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,on", __ime_show_cb, m_naviframe_item);
	evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,off", __ime_hide_cb, this);
	evas_object_smart_callback_add(m_conformant, "clipboard,state,on", __ime_show_cb, m_naviframe_item);
	evas_object_smart_callback_add(m_conformant, "clipboard,state,off", __ime_hide_cb, this);

	evas_object_smart_callback_add(m_naviframe,
					"transition,finished",
					__naviframe_pop_finished_cb, this);

	m_btn_save = elm_button_add(m_naviframe);
	if (!m_btn_save) return;
	elm_object_text_set(m_btn_save, BR_STRING_SAVE);
	evas_object_smart_callback_add(m_btn_save, "clicked", __save_btn_clicked_cb, this);

	elm_object_style_set(m_btn_save, "naviframe/toolbar/default");
	elm_object_item_part_content_set(m_naviframe_item, "toolbar_button1", m_btn_save);
	evas_object_show(m_btn_save);

	m_titlebar_btn_save = elm_button_add(m_naviframe);
	if (!m_titlebar_btn_save) return;
	elm_object_text_set(m_titlebar_btn_save, BR_STRING_SAVE);
	elm_object_style_set(m_titlebar_btn_save, "naviframe/toolbar/default");
	elm_object_item_part_content_set(m_naviframe_item, "title_toolbar_button1", m_titlebar_btn_save);
	evas_object_smart_callback_add(m_titlebar_btn_save, "clicked", __save_btn_clicked_cb, this);

	m_titlebar_btn_back = elm_button_add(m_naviframe);
	elm_object_style_set(m_titlebar_btn_back, "naviframe/back_btn/default");
	elm_object_item_part_content_set(m_naviframe_item, "title_prev_btn", m_titlebar_btn_back);
	evas_object_smart_callback_add(m_titlebar_btn_back, "clicked", __back_btn_clicked_cb, this);
}

