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
#if defined(BROWSER_TAG)
#include "add-tag-view.h"
#endif
#include "bookmark-edit-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "multiwindow-view.h"
#include "platform-service.h"

bookmark_add_view::bookmark_add_view(
					const char *title, const char *uri, int folder_id_to_save, Eina_Bool edit_mode)
:
	m_genlist(NULL)
	,m_back_button(NULL)
	,m_title_edit_field(NULL)
	,m_uri_edit_field(NULL)
	,m_btn_save(NULL)
	,m_titlebar_btn_save(NULL)
	,m_titlebar_btn_back(NULL)
	,m_naviframe_item(NULL)
	,m_itc_title(NULL)
	,m_itc_uri(NULL)
	,m_itc_folder(NULL)
	,m_folder_id(root_folder_id)
	,m_bookmark_id(0)
#if defined(BROWSER_TAG)
	,m_itc_tag(NULL)
#endif
{
	m_edit_mode = edit_mode;
	if (!m_edit_mode) {
		BROWSER_LOGD("[Add bookmark mode]Title[%s], URI[%s]", title, uri);
		if (uri) {
			m_uri_string = uri;
			m_input_uri_string = uri;
		}
		if (title)
			m_input_title_string = title;
		m_folder_id = folder_id_to_save;
#if defined(BROWSER_TAG)
		m_tags_string.clear();
#endif
	} else {
		BROWSER_LOGD("[Edit bookmark mode]URI[%s]", uri);
		bookmark bookmark_instance;
		if (bookmark_instance.get_id(uri, &m_bookmark_id)) {
			bookmark_item item;
			if (bookmark_instance.get_item_by_id(m_bookmark_id, &item)) {
				m_uri_string = item.get_uri();
				m_input_uri_string = item.get_uri();
				m_input_title_string = item.get_title();
				m_folder_id = item.get_parent_id();
#if defined(BROWSER_TAG)
				if (item.get_tag1() && (strlen(item.get_tag1()) > 0))
					m_tags_string = item.get_tag1();
				if (item.get_tag2() && (strlen(item.get_tag2()) > 0)) {
					m_tags_string += ",";
					m_tags_string += item.get_tag2();
				}
				if (item.get_tag3() && (strlen(item.get_tag3()) > 0)) {
					m_tags_string += ",";
					m_tags_string += item.get_tag3();
				}
				if (item.get_tag4() && (strlen(item.get_tag4()) > 0)) {
					m_tags_string += ",";
					m_tags_string += item.get_tag4();
				}
#endif
			} else
				BROWSER_LOGD("get bookmark item is failed");
		} else {
			BROWSER_LOGD("get bookmark id is failed");
		}
	}
}

bookmark_add_view::~bookmark_add_view(void)
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
	if (m_itc_uri)
		elm_genlist_item_class_free(m_itc_uri);
	if (m_itc_separator)
		elm_genlist_item_class_free(m_itc_separator);
	if (m_itc_folder)
		elm_genlist_item_class_free(m_itc_folder);
#if defined(BROWSER_TAG)
	if (m_itc_tag)
		elm_genlist_item_class_free(m_itc_tag);
#endif
}

void bookmark_add_view::__title_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	platform_service ps;
	bookmark_add_view *view_this = (bookmark_add_view *)data;

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
	    || view_this->m_input_uri_string.empty()
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

void bookmark_add_view::__title_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	bookmark_add_view *view_this = (bookmark_add_view *)data;

	const char *title = elm_entry_entry_get(obj);

	if (!title || strlen(title) == 0) {
		BROWSER_LOGD("title is empty");
		view_this->show_msg_popup(BR_STRING_ENTER_BOOKMARK_NAME);
		return;
	} else {
		if ((view_this->_save_bookmark()) == 0) {
			view_this->show_msg_popup(BR_STRING_ALREADY_EXISTS);
			return;
		}
		view_this->_back_to_previous_view();
	}
}

void bookmark_add_view::__uri_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	platform_service ps;
	bookmark_add_view *cp = (bookmark_add_view *)data;

	const char *uri = elm_entry_entry_get(obj);

	Eina_Bool only_has_space = EINA_FALSE;
	unsigned int space_count = 0;
	if (uri && strlen(uri)) {
		for (unsigned int i = 0 ; i < strlen(uri) ; i++) {
			if (uri[i] == ' ')
				space_count++;
		}
		if (space_count == strlen(uri))
			only_has_space = EINA_TRUE;
		cp->m_input_uri_string = uri;
	}
	BROWSER_LOGD("m_input_uri_string(%d)[%s]",
		strlen(cp->m_input_uri_string.c_str())
		,cp->m_input_uri_string.c_str());

	char *text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	if (!text || strlen(text) == 0 || !uri || strlen(uri) == 0
	    || cp->m_input_title_string.empty()
	    || only_has_space) {
		elm_object_disabled_set(cp->m_btn_save, EINA_TRUE);
		elm_object_disabled_set(cp->m_titlebar_btn_save, EINA_TRUE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
	} else {
		elm_object_disabled_set(cp->m_btn_save, EINA_FALSE);
		elm_object_disabled_set(cp->m_titlebar_btn_save, EINA_FALSE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
	}

	if (text)
		free(text);
}

void bookmark_add_view::__uri_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;
#if 0
	bookmark_add_view *view_this = (bookmark_add_view *)data;

	const char *uri = elm_entry_entry_get(obj);
	if (!uri || strlen(uri) == 0) {
		BROWSER_LOGD("uri is empty");
		view_this->show_msg_popup(BR_STRING_ENTER_URL);
		return;
	} else {
		if ((view_this->_save_bookmark()) == 0) {
			view_this->show_msg_popup(BR_STRING_ALREADY_EXISTS);
			return;
		}
		view_this->_back_to_previous_view();
	}
#endif
}

char *bookmark_add_view::__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	bookmark_add_view::menu_type type = callback_data->type;
	bookmark_add_view *cp = (bookmark_add_view *)callback_data->cp;

	if (!strcmp(part, "elm.text")) {
		switch (type) {
		case TITLE_INPUT_FIELD:
			break;
		case FOLDER_SELECT_MENU:
		{
			bookmark *bm = cp->m_browser->get_bookmark();
			bookmark_item item;
			bm->get_item_by_id(cp->m_folder_id, &item);
			BROWSER_LOGD("[%s]", item.get_title());
			if ((item.get_parent_id()) == 0) {
				return elm_entry_utf8_to_markup(BR_STRING_MOBILE);
			} else {
				return elm_entry_utf8_to_markup(item.get_title());
			}
			break;
		}
#if defined(BROWSER_TAG)
		case TAG_SELECT_MENU:
			if (cp->m_tags_string.empty())
				return elm_entry_utf8_to_markup(BR_STRING_TAP_TO_ADD_TAG);
			else
				return elm_entry_utf8_to_markup(cp->m_tags_string.c_str());
			break;
#endif
		default:
			break;
		}
	}

	return NULL;
}

Evas_Object *bookmark_add_view::__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	bookmark_add_view::menu_type type = callback_data->type;
	bookmark_add_view *view_this = (bookmark_add_view *)callback_data->cp;
	platform_service ps;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.edit")) {
		switch (type) {
		case TITLE_INPUT_FIELD:
		{
			BROWSER_LOGD("[%s] TITLE_INPUT_FIELD", part);
			view_this->m_title_edit_field = ps.editfield_add(obj, EINA_FALSE);
			if (!view_this->m_title_edit_field)
				return NULL;
			ps.editfield_entry_single_line_set(view_this->m_title_edit_field, EINA_TRUE);
			ps.editfield_guide_text_set(view_this->m_title_edit_field, BR_STRING_ENTER_BOOKMARK_NAME);

			Evas_Object *entry = ps.editfield_entry_get(view_this->m_title_edit_field);
			evas_object_smart_callback_add(entry,
							"changed", __title_entry_changed_cb, view_this);
			evas_object_smart_callback_add(entry,
							"preedit,changed", __title_entry_changed_cb, view_this);
			evas_object_smart_callback_add(entry,
						"activated", __title_entry_enter_key_cb, view_this);

			BROWSER_LOGD("m_input_title_string: %s", view_this->m_input_title_string.c_str());
			elm_entry_entry_set(entry, view_this->m_input_title_string.c_str());

			if (elm_entry_is_empty(entry))
				elm_object_signal_emit(view_this->m_title_edit_field, "elm,state,guidetext,show", "elm");

			return view_this->m_title_edit_field;
			break;
		}
		case URI_INPUT_FIELD:
		{
			BROWSER_LOGD("[%s] URI_INPUT_FIELD", part);
			view_this->m_uri_edit_field = ps.editfield_add(obj, EINA_FALSE);
			if (!view_this->m_uri_edit_field)
				return NULL;
			ps.editfield_entry_single_line_set(view_this->m_uri_edit_field, EINA_TRUE);
			ps.editfield_guide_text_set(view_this->m_uri_edit_field, BR_STRING_URL);

			Evas_Object *entry = ps.editfield_entry_get(view_this->m_uri_edit_field);
			evas_object_smart_callback_add(entry,
							"changed", __uri_entry_changed_cb, view_this);
			evas_object_smart_callback_add(entry,
							"preedit,changed", __uri_entry_changed_cb, view_this);
			evas_object_smart_callback_add(entry,
						"activated", __uri_entry_enter_key_cb, view_this);
			elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_URL);
			elm_entry_prediction_allow_set(entry, EINA_FALSE);

			BROWSER_LOGD("m_input_uri_string: %s", view_this->m_input_uri_string.c_str());
			elm_entry_entry_set(entry, view_this->m_input_uri_string.c_str());

			if (elm_entry_is_empty(entry))
				elm_object_signal_emit(view_this->m_uri_edit_field, "elm,state,guidetext,show", "elm");

			return view_this->m_uri_edit_field;
			break;
		}
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
#if defined(BROWSER_TAG)
		case TAG_SELECT_MENU:
		{
			BROWSER_LOGD("[%s] TAG_SELECT_MENU", part);
			Evas_Object *folder_icon = elm_icon_add(obj);
			if (folder_icon == NULL)
				break;
			elm_icon_standard_set(folder_icon, browser_img_dir"/I01_icon_tag.png");
			evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return folder_icon;
			break;
		}
#endif
		default:
			break;
		}
	}
	return NULL;
}

void bookmark_add_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	genlist_callback_data *callback_data = (genlist_callback_data *)elm_object_item_data_get(it);
	bookmark_add_view *cp = (bookmark_add_view *)callback_data->cp;
	bookmark_add_view::menu_type type = callback_data->type;

	elm_genlist_item_selected_set(it, EINA_FALSE);

	switch (type) {
	case TITLE_INPUT_FIELD:
		BROWSER_LOGD("TITLE_INPUT_FIELD");
		break;
	case FOLDER_SELECT_MENU:
		BROWSER_LOGD("FOLDER_SELECT_MENU");
		m_browser->create_bookmark_select_folder_view(cp->__select_folder_cb, cp, EINA_TRUE)->show();
		break;
#if defined(BROWSER_TAG)
	case TAG_SELECT_MENU:
	{
		BROWSER_LOGD("TAG_SELECT_MENU");
		m_browser->get_add_tag_view(cp->__edit_bookmark_done_cb, cp, cp->m_tags_string.c_str())->show();
		break;
	}
#endif
	default:
		break;
	}
}

void bookmark_add_view::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);

	EINA_SAFETY_ON_NULL_RETURN(cb_data);
	if (cb_data->type == TITLE_INPUT_FIELD)
		elm_object_item_signal_emit(it, "elm,state,top", "");
	else if (cb_data->type == URI_INPUT_FIELD)
		elm_object_item_signal_emit(it, "elm,state,bottom", "");
}

void bookmark_add_view::__select_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	bookmark_add_view *cp = (bookmark_add_view *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)event_info;

	cp->m_folder_id = bookmark_item_data->get_id();
}

void bookmark_add_view::__save_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_add_view *view_this = (bookmark_add_view *)data;

	if ((view_this->_save_bookmark()) == 0) {
		view_this->show_msg_popup(BR_STRING_ALREADY_EXISTS);
		return;
	}
	view_this->_back_to_previous_view();
	if (view_this->m_browser->get_bookmark_edit_view())
		view_this->m_browser->get_bookmark_edit_view()->refresh();
}

void bookmark_add_view::__back_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	bookmark_add_view *view_this = (bookmark_add_view *)data;

	view_this->_back_to_previous_view();
}

void bookmark_add_view::__naviframe_pop_finished_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	bookmark_add_view *cp = (bookmark_add_view *)data;

	if (cp->m_naviframe_item != elm_naviframe_top_item_get(cp->m_naviframe))
		return;

	m_browser->delete_bookmark_select_folder_view();
#if defined(BROWSER_TAG)
	m_browser->delete_add_tag_view();
#endif
}

void bookmark_add_view::__ime_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	EINA_SAFETY_ON_NULL_RETURN(data);
	Elm_Object_Item *it = (Elm_Object_Item*)data;
	elm_object_item_signal_emit(it, "elm,state,sip,shown", "");
}

void bookmark_add_view::__ime_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	EINA_SAFETY_ON_NULL_RETURN(data);
	bookmark_add_view *cp = (bookmark_add_view *)data;
	elm_object_item_signal_emit(cp->m_naviframe_item, "elm,state,sip,hidden", "");
}

int bookmark_add_view::_save_bookmark(void)
{
	BROWSER_LOGD("");
	int saved_bookmark_id;

#if defined(BROWSER_TAG)
	char *tag = NULL;
	char *tag1 = NULL;
	char *tag2 = NULL;
	char *tag3 = NULL;
	char *tag4 = NULL;
	std::vector<char *> tag_list;

	int len = m_tags_string.size();
	BROWSER_LOGD("m_tags_string(%d) :%s", len, m_tags_string.c_str());
	int max_tag_count = 0;
	m_browser->get_bookmark()->get_max_tag_count_for_a_bookmark_item(&max_tag_count);
	if (len > 0){
		char *buffer = (char *)malloc(len + 1);
		if (!buffer)
			return -1;
		strcpy(buffer, m_tags_string.c_str());
		int i = 0;
		tag = strtok(buffer, ",");
		if (tag) {
			tag_list.push_back(strdup(tag));
			BROWSER_LOGD("tag =%s, tag_list[%d]=%s", tag, i, tag_list[i]);
			for (i = 1 ; i < max_tag_count ; i++) {
				tag = strtok(NULL, ",");
				if (!tag)
					break;
				tag_list.push_back(strdup(tag));
				BROWSER_LOGD("tag =%s, tag_list[%d]=%s", tag, i, tag_list[i]);
			}
		}
		free(buffer);

		if ((tag_list.size() > 0) && tag_list[0])
			tag1 = tag_list[0];
		if ((tag_list.size() > 1) && tag_list[1])
			tag2 = tag_list[1];
		if ((tag_list.size() > 2) && tag_list[2])
			tag3 = tag_list[2];
		if ((tag_list.size() > 3) && tag_list[3])
			tag4 = tag_list[3];
	}
#endif
	int ret = 0;
	if (m_edit_mode) {
		ret = m_browser->get_bookmark()->update_bookmark(m_bookmark_id,
					m_input_title_string.c_str(),
					m_input_uri_string.c_str(),
					m_folder_id,
					-1
#if defined(BROWSER_TAG)
					, tag1, tag2, tag3, tag4
#endif
					);
	} else {
		ret = m_browser->get_bookmark()->save_bookmark(
							m_input_title_string.c_str(),
							m_input_uri_string.c_str(),
							&saved_bookmark_id,
							m_folder_id
#if defined(BROWSER_TAG)
							, tag1, tag2, tag3, tag4
#endif
							);
	}

#if defined(BROWSER_TAG)
	for(unsigned int j = 0 ; j < tag_list.size() ; j++) {
		if (tag_list[j])
			free(tag_list[j]);
	}
	tag_list.clear();
#endif
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
	if (elm_naviframe_bottom_item_get(m_naviframe) !=
						elm_naviframe_top_item_get(m_naviframe))
					elm_naviframe_item_pop(m_naviframe);
}

Evas_Object *bookmark_add_view::_create_genlist(Evas_Object *parent)
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
	m_itc_title->item_style = "dialogue/1icon";
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

	m_itc_uri = elm_genlist_item_class_new();
	memset(m_itc_uri, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_itc_uri->item_style = "dialogue/1icon";
	m_itc_uri->func.content_get = __genlist_get_content_cb;
	m_itc_uri->func.state_get = NULL;
	m_itc_uri->func.del = NULL;

	m_input_uri_callback_data.type = URI_INPUT_FIELD;
	m_input_uri_callback_data.cp = (void *)this;

	m_input_uri_callback_data.it = elm_genlist_item_append(genlist,
					m_itc_uri, &m_input_uri_callback_data, NULL,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
	elm_genlist_item_select_mode_set(m_input_uri_callback_data.it,
						ELM_OBJECT_SELECT_MODE_NONE);

	/* separator */
	m_itc_separator = elm_genlist_item_class_new();
	memset(m_itc_separator, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_itc_separator->item_style = "dialogue/separator";
	m_itc_separator->func.text_get = NULL;
	m_itc_separator->func.content_get = NULL;
	m_itc_separator->func.state_get = NULL;
	m_itc_separator->func.del = NULL;
	Elm_Object_Item *it = elm_genlist_item_append(genlist, m_itc_separator, NULL, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

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
#if defined(BROWSER_TAG)
	m_itc_tag = elm_genlist_item_class_new();
	memset(m_itc_tag, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_itc_tag->item_style = "dialogue/multiline/1text.1icon";
	m_itc_tag->func.text_get = __genlist_get_text_cb;
	m_itc_tag->func.content_get = __genlist_get_content_cb;
	m_itc_tag->func.state_get = NULL;
	m_itc_tag->func.del = NULL;

	m_select_tag_callback_data.type = TAG_SELECT_MENU;
	m_select_tag_callback_data.cp = (void *)this;
	m_select_tag_callback_data.it = elm_genlist_item_append(genlist,
					m_itc_tag, &m_select_tag_callback_data, NULL,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
#endif
	return genlist;
}

void bookmark_add_view::show()
{
	BROWSER_LOGD("");
	m_genlist = _create_genlist(m_naviframe);

	m_naviframe_item = elm_naviframe_item_push(m_naviframe,
						BR_STRING_EDIT, NULL, NULL, m_genlist, NULL);

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

#if defined(BROWSER_TAG)
void bookmark_add_view::__edit_bookmark_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	bookmark_add_view *cp = (bookmark_add_view *)data;
	std::vector<char *> tag_list = *((std::vector<char *> *)event_info);

	std::string tag_str;
	if (tag_list.size()) {
		tag_str = std::string(tag_list[0]);
		for (unsigned int i = 1 ; i < tag_list.size() ; i++) {
			tag_str = tag_str + "," + std::string(tag_list[i]);
		}
		cp->m_tags_string = tag_str;

		for (unsigned int i = 0 ; i < tag_list.size() ; i++)
			free(tag_list[i]);
	}
}
#endif
