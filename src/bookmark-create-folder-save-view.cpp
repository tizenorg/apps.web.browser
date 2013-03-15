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
#include "bookmark-create-folder-save-view.h"
#include "bookmark-edit-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "multiwindow-view.h"
#include "platform-service.h"

bookmark_create_folder_save_view::bookmark_create_folder_save_view(Evas_Smart_Cb cb_func, void *cb_data, int folder_id_to_save)
:
	m_genlist(NULL)
	,m_back_button(NULL)
	,m_title_edit_field(NULL)
	,m_btn_save(NULL)
	,m_titlebar_btn_save(NULL)
	,m_titlebar_btn_back(NULL)
	,m_naviframe_item(NULL)
	,m_itc_title(NULL)
	,m_cb_func(cb_func)
	,m_cb_data(cb_data)
{
	BROWSER_LOGD("");
	m_input_title_string.clear();
	m_folder_id = folder_id_to_save;
}

bookmark_create_folder_save_view::~bookmark_create_folder_save_view(void)
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
}

void bookmark_create_folder_save_view::__title_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	platform_service ps;
	bookmark_create_folder_save_view *view_this = (bookmark_create_folder_save_view *)data;
	//Evas_Object *entry = ps.editfield_entry_get(view_this->m_title_edit_field);

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
		else
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
}

void bookmark_create_folder_save_view::__title_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	bookmark_create_folder_save_view *cp = (bookmark_create_folder_save_view *)data;

	const char *title = elm_entry_entry_get(obj);

	if (!title || strlen(title) == 0) {
		BROWSER_LOGD("title is empty");
		cp->show_msg_popup(BR_STRING_ENTER_FOLDER_NAME);
		return;
	} else {
		if ((cp->_save_folder()) == 0) {
			cp->show_msg_popup(BR_STRING_ALREADY_EXISTS);
			return;
		}
		cp->_back_to_previous_view();
		cp->_back_to_previous_view();
		if (cp->m_browser->get_bookmark_select_folder_view())
			cp->m_browser->get_bookmark_select_folder_view()->refresh();
		if (cp->m_browser->get_bookmark_edit_view())
			cp->m_browser->get_bookmark_edit_view()->refresh();
	}
}

Evas_Object *bookmark_create_folder_save_view::__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	bookmark_create_folder_save_view *view_this = (bookmark_create_folder_save_view *)callback_data->cp;
	platform_service ps;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.edit")) {
		view_this->m_title_edit_field = ps.editfield_add(obj, EINA_TRUE);
		if (!view_this->m_title_edit_field)
			return NULL;
		ps.editfield_entry_single_line_set(view_this->m_title_edit_field, EINA_TRUE);
		ps.editfield_label_set(view_this->m_title_edit_field, BR_STRING_FOLDER_NAME);
		ps.editfield_guide_text_set(view_this->m_title_edit_field, BR_STRING_ENTER_FOLDER_NAME);

		Evas_Object *entry = ps.editfield_entry_get(view_this->m_title_edit_field);
		evas_object_smart_callback_add(entry,
						"changed", __title_entry_changed_cb, view_this);
		evas_object_smart_callback_add(entry,
						"preedit,changed", __title_entry_changed_cb, view_this);
		evas_object_smart_callback_add(entry,
					"activated", __title_entry_enter_key_cb, view_this);

		BROWSER_LOGD("m_input_title_string: %s", view_this->m_input_title_string.c_str());
		elm_entry_entry_set(entry, view_this->m_input_title_string.c_str());
		view_this->m_input_title_string.clear();

		if (elm_entry_is_empty(entry))
			elm_object_signal_emit(view_this->m_title_edit_field, "elm,state,guidetext,show", "elm");

		return view_this->m_title_edit_field;
	}
	return NULL;
}

void bookmark_create_folder_save_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!event_info)
		return;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	elm_genlist_item_selected_set(it, EINA_FALSE);
}

void bookmark_create_folder_save_view::__save_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_create_folder_save_view *cp = (bookmark_create_folder_save_view *)data;

	if ((cp->_save_folder()) == 0) {
		cp->show_msg_popup(BR_STRING_ALREADY_EXISTS);
		return;
	}

	if (cp->m_cb_func) {
		cp->m_cb_func(cp->m_cb_data, NULL, NULL);
	}

	cp->_back_to_previous_view();
	cp->_back_to_previous_view();
}

void bookmark_create_folder_save_view::__back_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_create_folder_save_view *view_this = (bookmark_create_folder_save_view *)data;

	view_this->_back_to_previous_view();
}

void bookmark_create_folder_save_view::__naviframe_pop_finished_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_create_folder_save_view *cp = (bookmark_create_folder_save_view *)data;

	if (cp->m_naviframe_item != elm_naviframe_top_item_get(cp->m_naviframe))
		return;
}

void bookmark_create_folder_save_view::__ime_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)data;
	elm_object_item_signal_emit(it, "elm,state,sip,shown", "");
}

void bookmark_create_folder_save_view::__ime_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)data;
	elm_object_item_signal_emit(it, "elm,state,sip,hidden", "");
}

int bookmark_create_folder_save_view::_save_folder(void)
{
	BROWSER_LOGD("");

	int saved_folder_id;
	BROWSER_LOGD("m_input_title_string: %s", m_input_title_string.c_str());
	int ret = m_browser->get_bookmark()->save_folder(
							m_input_title_string.c_str(),
							&saved_folder_id,
							m_folder_id);
	if (ret < 0) {
		BROWSER_LOGD("folder add is failed");
		return -1;
	}
	if (ret == 0) {
		BROWSER_LOGD("same folder is already exist");
		return 0;
	}

	return 1;
}

void bookmark_create_folder_save_view::_back_to_previous_view(void)
{
	BROWSER_LOGD("");
	if (elm_naviframe_bottom_item_get(m_naviframe) !=
						elm_naviframe_top_item_get(m_naviframe))
					elm_naviframe_item_pop(m_naviframe);
}

Evas_Object *bookmark_create_folder_save_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	m_itc_title = elm_genlist_item_class_new();
	memset(m_itc_title, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_itc_title->item_style = "dialogue/1icon";
	m_itc_title->func.content_get = __genlist_get_content_cb;
	m_itc_title->func.state_get = NULL;
	m_itc_title->func.del = NULL;

	m_input_title_callback_data.cp = (void *)this;

	m_input_title_callback_data.it = elm_genlist_item_append(genlist,
					m_itc_title, &m_input_title_callback_data, NULL,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
	elm_genlist_item_select_mode_set(m_input_title_callback_data.it,
						ELM_OBJECT_SELECT_MODE_NONE);

	return genlist;
}

void bookmark_create_folder_save_view::show()
{
	BROWSER_LOGD("");
	m_genlist = _create_genlist(m_naviframe);

	m_naviframe_item = elm_naviframe_item_push(m_naviframe,
						BR_STRING_CREATE_FOLDER, NULL, NULL, m_genlist, NULL);

	evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,on", __ime_show_cb, m_naviframe_item);
	evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,off", __ime_hide_cb, m_naviframe_item);
	evas_object_smart_callback_add(m_conformant, "clipboard,state,on", __ime_show_cb, m_naviframe_item);
	evas_object_smart_callback_add(m_conformant, "clipboard,state,off", __ime_hide_cb, m_naviframe_item);

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
