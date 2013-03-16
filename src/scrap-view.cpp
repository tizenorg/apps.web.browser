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

#include "scrap-view.h"

#include <Elementary.h>
#include <algorithm>
#include <string>

#include "add-tag-view.h"
#include "browser.h"
#include "browser-view.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "webview.h"

#define scrap_view_edj_path browser_edj_dir"/scrap-view.edj"

#define MAX_DATE_LENTH	1024

scrap_view::scrap_view(void)
:
	m_item_ic(NULL)
	,m_group_title_ic(NULL)
	,m_genlist(NULL)
	,m_naviframe_item(NULL)
	,m_main_layout(NULL)
	,m_tag_genlist(NULL)
	,m_tag_group_index_ic(NULL)
	,m_tag_item_ic(NULL)
	,m_more_button(NULL)
	,m_search_button(NULL)
	,m_toolbar_delete_button(NULL)
	,m_title_select_all_button(NULL)
	,m_searchbar_layout(NULL)
	,m_search_genlist(NULL)
	,m_search_genlist_layout(NULL)
	,m_no_contents_layout(NULL)
	,m_select_all_flag(EINA_FALSE)
	,m_unset_genlist(NULL)
	,m_search_item_ic(NULL)
{
	BROWSER_LOGD("");
	elm_theme_extension_add(NULL, scrap_view_edj_path);
}

scrap_view::~scrap_view(void)
{
	BROWSER_LOGD("");

	if (m_scrap_list.size())
		elm_object_part_content_unset(m_main_layout, "elm.swallow.content");

	if (m_genlist)
		evas_object_del(m_genlist);
	if (m_tag_genlist)
		evas_object_del(m_tag_genlist);
	if (m_toolbar_delete_button)
		evas_object_del(m_toolbar_delete_button);
	if (m_title_select_all_button)
		evas_object_del(m_title_select_all_button);
	if (m_search_genlist_layout)
		evas_object_del(m_search_genlist_layout);

	for (int i = 0 ; i < m_scrap_list.size() ; i++)
		delete m_scrap_list[i];

	for (int i = 0 ; i < m_tag_list.size() ; i++)
		free(m_tag_list[i]);

	for (int i = 0 ; i < m_genlist_item_list.size() ; i++)
		free(m_genlist_item_list[i]);

	if (m_item_ic)
		elm_genlist_item_class_free(m_item_ic);
	if (m_group_title_ic)
		elm_genlist_item_class_free(m_group_title_ic);
	if (m_tag_group_index_ic)
		elm_genlist_item_class_free(m_tag_group_index_ic);
	if (m_tag_item_ic)
		elm_genlist_item_class_free(m_tag_item_ic);
	if (m_search_item_ic)
		elm_genlist_item_class_free(m_search_item_ic);

	evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_cb);

	evas_object_smart_callback_del(m_conformant, "virtualkeypad,state,on", __ime_show_cb);
	evas_object_smart_callback_del(m_conformant, "virtualkeypad,state,off", __ime_hide_cb);
	evas_object_smart_callback_del(m_conformant, "clipboard,state,on", __ime_show_cb);
	evas_object_smart_callback_del(m_conformant, "clipboard,state,off", __ime_hide_cb);

	elm_theme_extension_del(NULL, scrap_view_edj_path);
}

void scrap_view::show(void)
{
	m_main_layout = _create_main_layout(m_naviframe);

	Evas_Object *back_button = elm_button_add(m_naviframe);
	if (back_button) {
		elm_object_style_set(back_button, "naviframe/end_btn/default");
		evas_object_smart_callback_add(back_button, "clicked", __back_button_cb, this);
	}
	m_naviframe_item = elm_naviframe_item_push(m_naviframe, BR_STRING_SCRAPBOOK, back_button, NULL, m_main_layout, "browser-scrap-view");

	m_more_button = elm_button_add(m_naviframe);
	if (m_more_button) {
		elm_object_style_set(m_more_button, "naviframe/more/default");
		evas_object_smart_callback_add(m_more_button, "clicked", __more_cb, this);
		elm_object_item_part_content_set(m_naviframe_item, "toolbar_more_btn", m_more_button);
	}

	m_search_button = elm_button_add(m_naviframe);
	if (m_search_button) {
		elm_object_style_set(m_search_button, "naviframe/toolbar/default");
		elm_object_text_set(m_search_button, BR_STRING_SEARCH);
		evas_object_smart_callback_add(m_search_button, "clicked", __search_cb, this);
		elm_object_item_part_content_set(m_naviframe_item, "toolbar_button2", m_search_button);
	}

	if (m_scrap_list.size() == 0) {
		elm_object_disabled_set(m_more_button, EINA_TRUE);
		elm_object_disabled_set(m_search_button, EINA_TRUE);
	}

	Evas_Object *title_back_button = elm_button_add(m_naviframe);
	if (!title_back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return;
	}
	elm_object_style_set(title_back_button, "naviframe/back_btn/default");
	elm_object_item_part_content_set(m_naviframe_item, "title_prev_btn", title_back_button);
	evas_object_smart_callback_add(title_back_button, "clicked", __search_back_button_cb, this);

	m_searchbar_layout = _create_search_bar(m_naviframe);
	elm_object_item_part_content_set(m_naviframe_item, "title_toolbar_button1", m_searchbar_layout);

	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_cb, this);

	evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,on", __ime_show_cb, m_naviframe_item);
	evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,off", __ime_hide_cb, this);
	evas_object_smart_callback_add(m_conformant, "clipboard,state,on", __ime_show_cb, m_naviframe_item);
	evas_object_smart_callback_add(m_conformant, "clipboard,state,off", __ime_hide_cb, this);
}

void scrap_view::__ime_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)data;
	elm_object_item_signal_emit(it, "elm,state,sip,shown", "");
}

void scrap_view::__ime_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view *sv = (scrap_view *)data;

	Evas_Object *content = elm_object_part_content_get(sv->m_main_layout, "elm.swallow.content");
	if (content == sv->m_search_genlist_layout)
		elm_object_part_content_set(sv->m_main_layout, "elm.swallow.content", sv->m_unset_genlist);

	if (sv->m_search_genlist_layout) {
		elm_object_part_content_unset(sv->m_search_genlist_layout, "elm.swallow.content");
		if (sv->m_search_genlist) {
			evas_object_del(sv->m_search_genlist);
			sv->m_search_genlist = NULL;
		}
		if (sv->m_no_contents_layout) {
			evas_object_del(sv->m_no_contents_layout);
			sv->m_no_contents_layout = NULL;
		}
		evas_object_del(sv->m_search_genlist_layout);
		sv->m_search_genlist_layout;
		sv->m_search_genlist_layout = NULL;
	}

	elm_object_item_signal_emit(sv->m_naviframe_item, "elm,state,sip,hidden", "");
}

void scrap_view::__back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view *sv = (scrap_view *)data;
	if (!sv->_is_tag_view()) {
		if (elm_genlist_decorate_mode_get(sv->m_genlist)) {
			sv->_set_edit_mode(EINA_FALSE);
			return;
		}
	}

	/* This is because, the ime is invoked shortly in below scenario.
	* Search scrap -> Press back button of search bar -> Press back button in scrap view -> browser view, but the ime is invoked shortly.
	*/
	if (sv->m_searchbar_layout) {
		Evas_Object *search_entry = elm_object_part_content_get(sv->m_searchbar_layout, "elm.swallow.content");
		elm_object_focus_allow_set(search_entry, EINA_FALSE);
	}

	elm_naviframe_item_pop(m_naviframe);
}

Eina_Bool scrap_view::_is_tag_view(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_main_layout, EINA_FALSE);
	if (m_tag_genlist != elm_object_part_content_get(m_main_layout, "elm.swallow.content"))
		return EINA_FALSE;

	return EINA_TRUE;
}

void scrap_view::__view_by_tag_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view *sv = (scrap_view *)data;
	sv->_change_tag_view(EINA_TRUE);

	evas_object_del(obj);
}

void scrap_view::__view_by_date_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view *sv = (scrap_view *)data;
	sv->_change_tag_view(EINA_FALSE);

	evas_object_del(obj);
}

void scrap_view::__more_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view *sv = (scrap_view *)data;
	sv->_set_edit_mode(EINA_TRUE);

	evas_object_del(obj);
}

void scrap_view::__search_keyword_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view *sv = (scrap_view *)data;
	Evas_Object *searchbar_layout = sv->m_searchbar_layout;

	if (elm_object_focus_get(searchbar_layout)) {
		if (elm_entry_is_empty(obj))
			elm_object_signal_emit(searchbar_layout, "elm,state,eraser,hide", "elm");
		else
			elm_object_signal_emit(searchbar_layout, "elm,state,eraser,show", "elm");
	}
	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(searchbar_layout, "elm,state,guidetext,hide", "elm");

	const char *input_text = elm_entry_entry_get(obj);
	BROWSER_LOGD("input_text=[%s]", input_text);
	sv->_search_scrap(input_text);
}

Evas_Object *scrap_view::_create_search_genlist(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	memset(item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));
//	item_ic->item_style = "dialogue/2text.1icon.2";
	item_ic->item_style = "2text.1icon.4.tb";
	item_ic->decorate_item_style = NULL;
	item_ic->decorate_all_item_style = NULL;
	item_ic->func.text_get = __search_genlist_label_get_cb;
	item_ic->func.content_get = __genlist_content_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	m_search_item_ic = item_ic;

	return genlist;
}

char *scrap_view::__search_genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);
	scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)data;
	scrap_item *item = genlist_item->item;

	scrap_view *sv = m_browser->get_scrap_view();
	Evas_Object *search_entry = elm_object_part_content_get(sv->m_searchbar_layout, "elm.swallow.content");
	const char *keyword = elm_entry_entry_get(search_entry);

	if (!strcmp(part, "elm.text.1")) {
		std::string source_string;
		if (!item->get_title() || strlen(item->get_title()) == 0)
			source_string = std::string(item->get_uri());
		else
			source_string = std::string(item->get_title());

		std::string::size_type pos = std::string::npos;
		if((keyword && strlen(keyword)) && (pos = source_string.find(keyword)) != std::string::npos)
			source_string = source_string.substr(0, pos) + std::string("<color=#0000ffff>") + std::string(keyword) + std::string("</color>") + source_string.substr(pos + strlen(keyword), source_string.length());

		return strdup(source_string.c_str());
	} else if (!strcmp(part, "elm.text.2")) {
		scrap scrap_instance;
		std::vector<char *> tag_list = scrap_instance.get_tag_list(item);
		if (tag_list.size() == 0) {
			std::string source_string = std::string(item->get_uri());

			std::string::size_type pos = std::string::npos;
			if((keyword && strlen(keyword)) && (pos = source_string.find(keyword)) != std::string::npos)
				source_string = source_string.substr(0, pos) + std::string("<color=#0000ffff>") + std::string(keyword) + std::string("</color>") + source_string.substr(pos + strlen(keyword), source_string.length());

			return strdup(source_string.c_str());
		} else {
			std::string tag_str = std::string(tag_list[0]);
			for (int i = 1 ; i < tag_list.size() ; i++) {
				tag_str = tag_str + ", " + std::string(tag_list[i]);
			}

			for (int i = 0 ; i < tag_list.size() ; i++)
				free(tag_list[i]);

			std::string source_string = tag_str;

			std::string::size_type pos = std::string::npos;
			if((keyword && strlen(keyword)) && (pos = source_string.find(keyword)) != std::string::npos)
				source_string = source_string.substr(0, pos) + std::string("<color=#0000ffff>") + std::string(keyword) + std::string("</color>") + source_string.substr(pos + strlen(keyword), source_string.length());

			return strdup(source_string.c_str());
		}
	}

	return NULL;
}

Evas_Object *scrap_view::_create_search_genlist_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, scrap_view_edj_path, "search-genlist-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	return layout;
}

void scrap_view::_search_scrap(const char *keyword)
{
	if (!m_search_genlist_layout) {
		m_search_genlist_layout = _create_search_genlist_layout(m_naviframe);
	}

	if (!keyword || strlen(keyword) == 0) {
		Evas_Object *content = elm_object_part_content_get(m_main_layout, "elm.swallow.content");
		if (content == m_search_genlist_layout) {
			elm_object_part_content_unset(m_main_layout, "elm.swallow.content");
			elm_object_part_content_set(m_main_layout, "elm.swallow.content", m_unset_genlist);
		}
		if (m_search_genlist) {
			evas_object_del(m_search_genlist);
			m_search_genlist = NULL;
		}
		if (m_no_contents_layout) {
			evas_object_del(m_no_contents_layout);
			m_no_contents_layout = NULL;
		}
		if (m_search_genlist_layout) {
			evas_object_del(m_search_genlist_layout);
			m_search_genlist_layout = NULL;
		}

		return;
	}

	if (m_search_genlist)
		elm_genlist_clear(m_search_genlist);

	int count = 0;
	Elm_Object_Item *it = elm_genlist_first_item_get(m_genlist);
	while (it) {
		if (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)elm_object_item_data_get(it);
			scrap_item *item = genlist_item->item;
			if ((item->get_title() && strstr(item->get_title(), keyword)) || (item->get_tag() && strstr(item->get_tag(), keyword))) {
				if (!m_search_genlist) {
					m_search_genlist = _create_search_genlist(m_search_genlist_layout);
				}

				count++;
				elm_genlist_item_append(m_search_genlist, m_search_item_ic, genlist_item, NULL, ELM_GENLIST_ITEM_NONE, __search_genlist_item_clicked_cb, genlist_item);
			}
		}
		it = elm_genlist_item_next_get(it);
	}

	if (count == 0) {
		if (!m_no_contents_layout) {
			m_no_contents_layout = elm_layout_add(m_search_genlist_layout);
			if (m_no_contents_layout) {
				elm_layout_theme_set(m_no_contents_layout, "layout", "nocontents", "search");
				elm_object_part_text_set(m_no_contents_layout, "elm.text", BR_STRING_NO_SEARCH_RESULT);
			}
		}
		elm_object_part_content_unset(m_search_genlist_layout, "elm.swallow.content");
		elm_object_part_content_set(m_search_genlist_layout, "elm.swallow.content", m_no_contents_layout);
		if (m_search_genlist) {
			evas_object_del(m_search_genlist);
			m_search_genlist = NULL;
		}
	} else {
		Evas_Object *content = elm_object_part_content_get(m_search_genlist_layout, "elm.swallow.content");
		if (content != m_search_genlist) {
			if (m_no_contents_layout) {
				evas_object_del(m_no_contents_layout);
				m_no_contents_layout = NULL;
			}
			elm_object_part_content_set(m_search_genlist_layout, "elm.swallow.content", m_search_genlist);
		}
	}

	Evas_Object *content = elm_object_part_content_get(m_main_layout, "elm.swallow.content");
	if (content != m_search_genlist_layout) {
		m_unset_genlist = elm_object_part_content_unset(m_main_layout, "elm.swallow.content");
		evas_object_hide(m_unset_genlist);
		elm_object_part_content_set(m_main_layout, "elm.swallow.content", m_search_genlist_layout);
	}
}

static void _focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *searchbar_layout = (Evas_Object *)data;

	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(searchbar_layout, "elm,state,eraser,show", "elm");
	elm_object_signal_emit(searchbar_layout, "elm,state,guidetext,hide", "elm");
	elm_object_signal_emit(searchbar_layout, "cancel,in", "");
}

static void _unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *searchbar_layout = (Evas_Object *)data;

	if (elm_entry_is_empty(obj))
		elm_object_signal_emit(searchbar_layout, "elm,state,guidetext,show", "elm");

	elm_object_signal_emit(searchbar_layout, "elm,state,eraser,hide", "elm");
}

static void _eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	elm_entry_entry_set((Evas_Object *)data, "");
}

static void _bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	elm_object_focus_set((Evas_Object *)data, EINA_TRUE);
}

Evas_Object *scrap_view::_create_search_bar(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *searchbar_layout = elm_layout_add(parent);
	if (!searchbar_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_theme_set(searchbar_layout, "layout", "searchbar", "default");

	Evas_Object *entry = elm_entry_add(searchbar_layout);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_smart_callback_add(entry, "changed", __search_keyword_changed_cb, this);
	evas_object_smart_callback_add(entry, "focused", _focused_cb, searchbar_layout);
	evas_object_smart_callback_add(entry, "unfocused", _unfocused_cb, searchbar_layout);
	elm_object_part_content_set(searchbar_layout, "elm.swallow.content", entry);
	elm_object_part_text_set(searchbar_layout, "elm.guidetext", BR_STRING_SEARCH);
	elm_object_signal_callback_add(searchbar_layout, "elm,eraser,clicked", "elm", _eraser_clicked_cb, entry);
	elm_object_signal_callback_add(searchbar_layout, "elm,bg,clicked", "elm", _bg_clicked_cb, entry);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	evas_object_size_hint_weight_set(searchbar_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(searchbar_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	return searchbar_layout;
}

void scrap_view::_set_edit_mode(Eina_Bool edit_mode)
{
	if (_is_tag_view())
		return;

	if (edit_mode) {
		elm_object_style_set(m_bg, "edit_mode");
		elm_genlist_decorate_mode_set(m_genlist, EINA_TRUE);

		if (!m_toolbar_delete_button) {
			m_toolbar_delete_button = elm_button_add(m_naviframe);
			if (m_toolbar_delete_button) {
				elm_object_style_set(m_toolbar_delete_button, "naviframe/toolbar/default");
				elm_object_text_set(m_toolbar_delete_button, BR_STRING_DELETE);
				evas_object_smart_callback_add(m_toolbar_delete_button, "clicked", __toolbar_delete_cb, this);
			}
		}
		Evas_Object *button = elm_object_item_part_content_unset(m_naviframe_item, "toolbar_button2");
		evas_object_hide(button);
		elm_object_item_part_content_set(m_naviframe_item, "toolbar_button2", m_toolbar_delete_button);

		Evas_Object *more_button = elm_object_item_part_content_unset(m_naviframe_item, "toolbar_more_btn");
		evas_object_hide(more_button);

		if (!m_title_select_all_button) {
			m_title_select_all_button = elm_button_add(m_naviframe);
			if (m_title_select_all_button) {
				elm_object_style_set(m_title_select_all_button, "naviframe/title_icon");
				Evas_Object *icon = elm_icon_add(m_naviframe);
				if (!icon) {
					BROWSER_LOGD("elm_icon_add is failed");
					return;
				}
				elm_icon_standard_set(icon, browser_img_dir"/00_icon_edit.png");
				evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
				elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
				elm_object_part_content_set(m_title_select_all_button, "icon", icon);
				evas_object_smart_callback_add(m_title_select_all_button, "clicked", __select_all_cb, this);
			}
		}

		elm_object_item_part_content_set(m_naviframe_item, "title_right_btn", m_title_select_all_button);
	} else {
		elm_object_style_set(m_bg, "default");
		elm_genlist_decorate_mode_set(m_genlist, EINA_FALSE);
		for (int i = 0 ; i < m_genlist_item_list.size() ; i++)
			m_genlist_item_list[i]->is_checked = EINA_FALSE;

		Evas_Object *button = elm_object_item_part_content_unset(m_naviframe_item, "toolbar_button2");
		evas_object_hide(button);
		elm_object_item_part_content_set(m_naviframe_item, "toolbar_button2", m_search_button);

		elm_object_item_part_content_set(m_naviframe_item, "toolbar_more_btn", m_more_button);

		button = elm_object_item_part_content_unset(m_naviframe_item, "title_right_btn");
		evas_object_hide(button);
	}
}

void scrap_view::__select_all_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view *sv = (scrap_view *)data;
	sv->m_select_all_flag = !(sv->m_select_all_flag);
	Elm_Object_Item *it = elm_genlist_first_item_get(sv->m_genlist);
	while (it) {
		scrap_view_genlist_item *item_data = (scrap_view_genlist_item *)elm_object_item_data_get(it);
		item_data->is_checked = sv->m_select_all_flag;
		elm_genlist_item_update(it);
		it = elm_genlist_item_next_get(it);
	}
}

void scrap_view::__toolbar_delete_confirm_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view *sv = (scrap_view *)data;

	Elm_Object_Item *it = elm_genlist_first_item_get(sv->m_genlist);
	while (it) {
		Evas_Object *genlist = sv->m_genlist;
		scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)elm_object_item_data_get(it);
		if (genlist_item->is_checked && elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			Elm_Object_Item *next_it = NULL;
			Elm_Object_Item *prev_it = NULL;
			next_it = elm_genlist_item_next_get(it);
			prev_it = elm_genlist_item_prev_get(it);

			if (prev_it && elm_genlist_item_select_mode_get(prev_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
				if (!next_it || (next_it && elm_genlist_item_select_mode_get(next_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
					elm_object_item_del(prev_it);
				else
					elm_object_item_data_set(prev_it, elm_object_item_data_get(next_it));
			}

			Elm_Object_Item *tmp_it = elm_genlist_item_next_get(it);
			elm_object_item_del(it);
			it = tmp_it;

			scrap_item *item = genlist_item->item;

			scrap scrap_instance;
			scrap_instance.delete_scrap(item->get_saved_file_path());

			for (int i = 0 ; i < sv->m_scrap_list.size() ; i++) {
				if (item == sv->m_scrap_list[i]) {
					delete sv->m_scrap_list[i];
					sv->m_scrap_list.erase(sv->m_scrap_list.begin() + i);
					break;
				}
			}

			elm_genlist_realized_items_update(genlist);

			if (sv->m_scrap_list.size() == 0) {
				Evas_Object *content = elm_object_part_content_unset(sv->m_main_layout, "elm.swallow.content");

				evas_object_hide(content);

				Evas_Object *no_contents_layout = elm_layout_add(sv->m_main_layout);
				if (no_contents_layout) {
					evas_object_size_hint_weight_set(no_contents_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
					elm_layout_theme_set(no_contents_layout, "layout", "nocontents", "text");
					elm_object_part_text_set(no_contents_layout, "elm.text", BR_STRING_NO_ITEMS);
					elm_object_part_content_set(sv->m_main_layout, "elm.swallow.content", no_contents_layout);
				}

				elm_object_disabled_set(sv->m_more_button, EINA_TRUE);
				elm_object_disabled_set(sv->m_search_button, EINA_TRUE);

				sv->_set_edit_mode(EINA_FALSE);
			}
		} else
			it = elm_genlist_item_next_get(it);
	}

}

void scrap_view::__toolbar_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	m_browser->get_scrap_view()->show_delete_popup(NULL, BR_STRING_DELETE_Q, BR_STRING_DELETE, __toolbar_delete_confirm_cb, BR_STRING_CANCEL, NULL, data);
}

static void _context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
}

void scrap_view::__more_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view *sv = (scrap_view *)data;

	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", _context_popup_dismissed_cb, NULL);

#if defined(BROWSER_TAG)
	if (!sv->_is_tag_view()) {
		elm_ctxpopup_item_append(more_popup, BR_STRING_VIEW_BY_TAG, NULL, __view_by_tag_cb, data);
		elm_ctxpopup_item_append(more_popup, BR_STRING_DELETE, NULL, __more_delete_cb, data);
	} else
		elm_ctxpopup_item_append(more_popup, BR_STRING_VIEW_BY_DATE, NULL, __view_by_date_cb, data);
#else
	elm_ctxpopup_item_append(more_popup, BR_STRING_DELETE, NULL, __more_delete_cb, data);
#endif

	int x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	evas_object_move(more_popup, x + w / 2, y + h / 2);
	evas_object_show(more_popup);
}

void scrap_view::__search_back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view *sv = (scrap_view *)data;
	Evas_Object *search_entry = elm_object_part_content_get(sv->m_searchbar_layout, "elm.swallow.content");
	elm_entry_entry_set(search_entry, "");
	elm_object_focus_set(search_entry, EINA_FALSE);

	elm_object_focus_set(sv->m_search_button, EINA_TRUE);
}

void scrap_view::__search_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	scrap_view *sv = (scrap_view *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)elm_genlist_decorated_item_get(sv->m_genlist);
	if (it) {
		elm_genlist_item_decorate_mode_set(it, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DEFAULT);
	}

	elm_object_item_signal_emit(sv->m_naviframe_item, "elm,state,sip,shown", "");
	Evas_Object *search_entry = elm_object_part_content_get(sv->m_searchbar_layout, "elm.swallow.content");
	elm_object_focus_set(search_entry, EINA_TRUE);
}

void scrap_view::__naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	scrap_view *sv = (scrap_view *)data;
	if (sv->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		return;
#if defined(BROWSER_TAG)
	m_browser->delete_add_tag_view();
#endif
}

void scrap_view::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	scrap_view *sv = (scrap_view *)data;

	BROWSER_LOGD("");
	if (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		Elm_Object_Item *next_it = NULL;
		Elm_Object_Item *prev_it = NULL;
		next_it = elm_genlist_item_next_get(it);
		prev_it = elm_genlist_item_prev_get(it);

		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && (next_it && elm_genlist_item_select_mode_get(next_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
			elm_object_item_signal_emit(it, "elm,state,top", "");
			return;
		}
	}

	if (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		Elm_Object_Item *next_it = NULL;
		Elm_Object_Item *prev_it = NULL;
		next_it = elm_genlist_item_next_get(it);
		prev_it = elm_genlist_item_prev_get(it);
		if (next_it && prev_it) {
			if ((elm_genlist_item_select_mode_get(next_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
			    && (elm_genlist_item_select_mode_get(prev_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
				elm_object_item_signal_emit(it, "elm,state,center", "");
				return;
			}
		}
	}

	if (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		Elm_Object_Item *next_it = NULL;
		Elm_Object_Item *prev_it = NULL;
		next_it = elm_genlist_item_next_get(it);
		prev_it = elm_genlist_item_prev_get(it);

		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && (next_it && elm_genlist_item_select_mode_get(next_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
			elm_object_item_signal_emit(it, "elm,state,bottom", "");
			return;
		}
		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && !next_it) {
			elm_object_item_signal_emit(it, "elm,state,bottom", "");
			return;
		}
	}

	elm_object_item_signal_emit(it, "elm,state,default", "");
}

Evas_Object *scrap_view::_create_main_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, scrap_view_edj_path, "main-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_genlist = _create_genlist(layout);
	if (m_genlist) {
		elm_object_part_content_set(layout, "elm.swallow.content", m_genlist);
	} else {
		Evas_Object *no_contents_layout = elm_layout_add(layout);
		if (no_contents_layout) {
			evas_object_size_hint_weight_set(no_contents_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_layout_theme_set(no_contents_layout, "layout", "nocontents", "text");
			elm_object_part_text_set(no_contents_layout, "elm.text", BR_STRING_NO_ITEMS);
		}
		elm_object_part_content_set(layout, "elm.swallow.content", no_contents_layout);
	}

	return layout;
}

Evas_Object *scrap_view::_create_genlist(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
//	evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, this);

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	memset(item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));
//	item_ic->item_style = "dialogue/2text.1icon.2";
	item_ic->item_style = "2text.1icon.4";
	item_ic->decorate_item_style = "mode/slide2";
	item_ic->decorate_all_item_style = "edit_default";
	item_ic->func.text_get = __genlist_label_get_cb;
	item_ic->func.content_get = __genlist_content_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

	Elm_Genlist_Item_Class *group_title_ic = elm_genlist_item_class_new();
	memset(group_title_ic, 0x00, sizeof(Elm_Genlist_Item_Class));
//	group_title_ic->item_style = "dialogue/grouptitle";
	group_title_ic->item_style = "groupindex";
	group_title_ic->decorate_item_style = NULL;
	group_title_ic->decorate_all_item_style = "edit_default";
	group_title_ic->func.text_get = __genlist_label_get_cb;
	group_title_ic->func.content_get = __genlist_content_get_cb;
	group_title_ic->func.state_get = NULL;
	group_title_ic->func.del = NULL;

	evas_object_smart_callback_add(genlist, "drag,start,right", __sweep_right_cb, this);
	evas_object_smart_callback_add(genlist, "drag,start,left", __sweep_left_cb, this);

	scrap scrap_instance;
	m_scrap_list = scrap_instance.get_scrap_list();
	if (m_scrap_list.size() == 0) {
		evas_object_del(genlist);
		return NULL;
	}

	for (int i = 0 ; i < m_scrap_list.size() ; i++) {
		if (i == 0) {
			scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)malloc(sizeof(scrap_view_genlist_item));
			genlist_item->item = m_scrap_list[i];
			genlist_item->is_checked = EINA_FALSE;
			genlist_item->is_tag_index = EINA_TRUE;
			m_genlist_item_list.push_back(genlist_item);

			Elm_Object_Item *it = elm_genlist_item_append(genlist, group_title_ic, genlist_item, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

			genlist_item->it = it;
		} else {
			if (m_scrap_list[i]->get_time_stamp_type() != m_scrap_list[i - 1]->get_time_stamp_type()) {
				scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)malloc(sizeof(scrap_view_genlist_item));
				genlist_item->item = m_scrap_list[i];
				genlist_item->is_checked = EINA_FALSE;
				genlist_item->is_tag_index = EINA_TRUE;
				m_genlist_item_list.push_back(genlist_item);

				Elm_Object_Item *it = elm_genlist_item_append(genlist, group_title_ic, genlist_item, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

				genlist_item->it = it;
			}
		}
		scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)malloc(sizeof(scrap_view_genlist_item));
		genlist_item->item = m_scrap_list[i];
		genlist_item->is_checked = EINA_FALSE;
		genlist_item->is_tag_index = EINA_FALSE;
		m_genlist_item_list.push_back(genlist_item);

		genlist_item->it = elm_genlist_item_append(genlist, item_ic, genlist_item, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, genlist_item);
	}

	m_item_ic = item_ic;
	m_group_title_ic = group_title_ic;

	return genlist;
}

void scrap_view::_change_tag_view(Eina_Bool tag_view)
{
	if (tag_view) {
		if (!m_tag_genlist)
			m_tag_genlist = _create_tag_genlist(m_main_layout);

		Evas_Object *content = elm_object_part_content_unset(m_main_layout, "elm.swallow.content");
		if (content)
			evas_object_hide(content);
		elm_object_part_content_set(m_main_layout, "elm.swallow.content", m_tag_genlist);
	} else {
		Evas_Object *content = elm_object_part_content_unset(m_main_layout, "elm.swallow.content");
		if (content)
			evas_object_hide(content);
		elm_object_part_content_set(m_main_layout, "elm.swallow.content", m_genlist);

		if (m_tag_genlist) {
			evas_object_del(m_tag_genlist);
			m_tag_genlist = 0;

			if (m_tag_group_index_ic) {
				elm_genlist_item_class_free(m_tag_group_index_ic);
				m_tag_group_index_ic = NULL;
			}
			if (m_tag_item_ic) {
				elm_genlist_item_class_free(m_tag_item_ic);
				m_tag_item_ic = NULL;
			}

			for (int i = 0 ; i < m_tag_list.size() ; i++)
				free(m_tag_list[i]);
			m_tag_list.clear();
		}
	}
}

static bool _comparison_func(const char *c1, const char *c2)
{
    return strcmp(c1, c2) < 0;
}

Evas_Object *scrap_view::_create_tag_genlist(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_object_style_set(genlist, "handler");

	Elm_Genlist_Item_Class *group_index_ic = elm_genlist_item_class_new();
	memset(group_index_ic, 0x00, sizeof(Elm_Genlist_Item_Class));
	group_index_ic->item_style = "groupindex";
	group_index_ic->decorate_item_style = NULL;
	group_index_ic->decorate_all_item_style = NULL;
	group_index_ic->func.text_get = __tag_genlist_label_get_cb;
	group_index_ic->func.content_get = __tag_genlist_content_get_cb;
	group_index_ic->func.state_get = NULL;
	group_index_ic->func.del = NULL;

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	memset(item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));
	item_ic->item_style = "2text.1icon.4";
	item_ic->decorate_item_style = NULL;
	item_ic->decorate_all_item_style = NULL;
	item_ic->func.text_get = __tag_genlist_label_get_cb;
	item_ic->func.content_get = __tag_genlist_content_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

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

	std::sort(m_tag_list.begin(), m_tag_list.end(), _comparison_func);

	for (int i = 0 ; i < m_tag_list.size() ; i++) {
		BROWSER_LOGD("m_tag_list[%d]=[%s]", i, m_tag_list[i]);
		Elm_Object_Item *it = elm_genlist_item_append(genlist, group_index_ic, m_tag_list[i], NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		for (int j = 0 ; j < m_scrap_list.size() ; j++) {
			std::vector<char *> item_tag_list = scrap_instance.extract_tag_list(m_scrap_list[j]->get_tag());
			for (int k = 0 ; k < item_tag_list.size() ; k++) {
				if (!strcmp(m_tag_list[i], item_tag_list[k])) {
					elm_genlist_item_append(genlist, item_ic, m_scrap_list[j], NULL, ELM_GENLIST_ITEM_NONE, __tag_genlist_item_clicked_cb, m_scrap_list[j]);
				}

				free(item_tag_list[k]);
			}
		}
	}

	Eina_Bool untaged = EINA_FALSE;
	for (int i = 0 ; i < m_scrap_list.size() ; i++) {
		const char *item_tag = m_scrap_list[i]->get_tag();
		if (!item_tag || strlen(item_tag) == 0) {
			if (!untaged) {
				Elm_Object_Item *it = elm_genlist_item_append(genlist, group_index_ic, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
				untaged = EINA_TRUE;
			}
			elm_genlist_item_append(genlist, item_ic, m_scrap_list[i], NULL, ELM_GENLIST_ITEM_NONE, __tag_genlist_item_clicked_cb, m_scrap_list[i]);
		}
	}

	m_tag_item_ic = item_ic;
	m_tag_group_index_ic = group_index_ic;

	return genlist;
}

char *scrap_view::__tag_genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);

	if (!strcmp(part, "elm.text")) {
		char *tag = (char *)data;
		if (!tag)
			return strdup(BR_STRING_UNTAGGED);
		else
			return strdup(tag);
	} else if (!strcmp(part, "elm.text.1")) {
		scrap_item *item = (scrap_item *)data;
		if (!item->get_title() || strlen(item->get_title()) == 0)
			return strdup(item->get_uri());
		else
			return strdup(item->get_title());
	} else if (!strcmp(part, "elm.text.2")) {
		scrap_item *item = (scrap_item *)data;

		scrap scrap_instance;
		std::vector<char *> tag_list = scrap_instance.get_tag_list(item);
		if (tag_list.size() == 0)
			return strdup(item->get_uri());
		else {
			std::string tag_str = std::string(tag_list[0]);
			for (int i = 1 ; i < tag_list.size() ; i++) {
				tag_str = tag_str + ", " + std::string(tag_list[i]);
			}

			for (int i = 0 ; i < tag_list.size() ; i++)
				free(tag_list[i]);

			return strdup(tag_str.c_str());
		}
	}

	return NULL;
}

void scrap_view::__sweep_right_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Elm_Object_Item *sweep_it = (Elm_Object_Item *)event_info;
	Elm_Object_Item *it = (Elm_Object_Item *)elm_genlist_decorated_item_get(obj);
	if (it && it != sweep_it) {
		elm_genlist_item_decorate_mode_set(it, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DEFAULT);
	}

	if (!elm_genlist_decorate_mode_get(obj)) {
		elm_genlist_item_decorate_mode_set(sweep_it, "slide", EINA_TRUE);
		elm_genlist_item_select_mode_set(sweep_it, ELM_OBJECT_SELECT_MODE_NONE);
	}
}

void scrap_view::__sweep_left_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Elm_Object_Item *sweep_it = (Elm_Object_Item *)event_info;

	if (elm_genlist_decorate_mode_get(obj))
		return;

	elm_genlist_item_decorate_mode_set(sweep_it, "slide", EINA_FALSE);
	elm_genlist_item_select_mode_set(sweep_it, ELM_OBJECT_SELECT_MODE_DEFAULT);
}

void scrap_view::__tag_genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	scrap_item *item = (scrap_item *)data;
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	wv->load_uri(item->get_saved_file_path());

	elm_naviframe_item_pop(m_naviframe);
}

void scrap_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)data;
	scrap_item *item = genlist_item->item;

	if (elm_genlist_decorate_mode_get(obj)) {
		genlist_item->is_checked = !(genlist_item->is_checked);
		elm_genlist_item_update((Elm_Object_Item *)event_info);
		elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
		return;
	}

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	wv->load_uri(item->get_saved_file_path());

	elm_naviframe_item_pop(m_naviframe);
}

void scrap_view::__search_genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)data;
	scrap_item *item = genlist_item->item;
	scrap_view *sv = m_browser->get_scrap_view();

	Evas_Object *search_entry = elm_object_part_content_get(sv->m_searchbar_layout, "elm.swallow.content");
	elm_entry_entry_set(search_entry, "");
	elm_object_focus_set(search_entry, EINA_FALSE);

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	wv->load_uri(item->get_saved_file_path());

	elm_naviframe_item_pop(m_naviframe);
}

char *scrap_view::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);
	scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)data;
	scrap_item *item = genlist_item->item;

	if (!strcmp(part, "elm.text.1")) {
		if (!item->get_title() || strlen(item->get_title()) == 0)
			return strdup(item->get_uri());
		else
			return strdup(item->get_title());
	} else if (!strcmp(part, "elm.text.2")) {
		scrap scrap_instance;
		std::vector<char *> tag_list = scrap_instance.get_tag_list(item);
		if (tag_list.size() == 0)
			return strdup(item->get_uri());
		else {
			std::string tag_str = std::string(tag_list[0]);
			for (int i = 1 ; i < tag_list.size() ; i++) {
				tag_str = tag_str + ", " + std::string(tag_list[i]);
			}

			for (int i = 0 ; i < tag_list.size() ; i++)
				free(tag_list[i]);

			return strdup(tag_str.c_str());
		}
	} else if (!strcmp(part, "elm.text")) {
		time_t current_time;
		struct tm *current_time_info;
		time(&current_time);
		current_time_info = localtime(&current_time);

		int current_year = current_time_info->tm_year;
		int current_yday = current_time_info->tm_yday;

		struct tm item_time_info;
		memset(&item_time_info, 0x00, sizeof(struct tm));
		item_time_info.tm_year = item->get_year() - 1900;
		item_time_info.tm_mon = item->get_month() - 1;
		item_time_info.tm_mday = item->get_day();
		time_t item_time = mktime(&item_time_info);
		struct tm *ptr_item_time = localtime(&item_time);

		int item_year = ptr_item_time->tm_year;
		int item_yday = ptr_item_time->tm_yday;

		BROWSER_LOGD("current_year=%d, current_yday=%d, item_year=%d, item_yday=%d", current_year, current_yday, item_year, item_yday);

		if (current_year == item_year) {
			int day_gap = current_yday - item_yday;
			if (day_gap == 0)
				return strdup(BR_STRING_HISTORY_TODAY);
			else if (day_gap == 1)
				return strdup(BR_STRING_HISTORY_YESTERDAY);
			else if (day_gap <= 7)
				return strdup(BR_STRING_HISTORY_LAST_WEEK);
			else if (day_gap <= 30)
				return strdup(BR_STRING_HISTORY_LAST_MONTH);
			else
				return strdup(BR_STRING_HISTORY_OLDER);
		}

		return strdup(BR_STRING_HISTORY_OLDER);
	}

	return NULL;
}

void scrap_view::__edit_scrap_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_item *item = (scrap_item *)data;
	std::vector<char *> tag_list = *((std::vector<char *> *)event_info);
	scrap scrap_instance;

	std::string tag_str;
	if (tag_list.size()) {
		tag_str = std::string(tag_list[0]);
		for (int i = 1 ; i < tag_list.size() ; i++) {
			tag_str = tag_str + "," + std::string(tag_list[i]);
		}
		scrap_instance.update_tag(item, tag_str.c_str());

		item->set_tag(tag_str.c_str());

		for (int i = 0 ; i < tag_list.size() ; i++)
			free(tag_list[i]);
	} else {
		scrap_instance.update_tag(item, tag_str.c_str());

		item->set_tag(tag_str.c_str());
	}
}

void scrap_view::__edit_tag_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	scrap_item *item = (scrap_item *)data;

	Evas_Object *genlist = m_browser->get_scrap_view()->m_genlist;
	Elm_Object_Item *it = (Elm_Object_Item *)elm_genlist_decorated_item_get(genlist);
	if (it) {
		elm_genlist_item_decorate_mode_set(it, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DEFAULT);
	}
#if defined(BROWSER_TAG)
	m_browser->get_add_tag_view(__edit_scrap_done_cb, item, item->get_tag())->show();
#endif
}

void scrap_view::__delete_confirm_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = m_browser->get_scrap_view()->m_genlist;
	Elm_Object_Item *sweep_it = (Elm_Object_Item *)elm_genlist_decorated_item_get(genlist);

	Elm_Object_Item *next_it = NULL;
	Elm_Object_Item *prev_it = NULL;
	next_it = elm_genlist_item_next_get(sweep_it);
	prev_it = elm_genlist_item_prev_get(sweep_it);

	if (prev_it && elm_genlist_item_select_mode_get(prev_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		if (!next_it || (next_it && elm_genlist_item_select_mode_get(next_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
			elm_object_item_del(prev_it);
		else
			elm_object_item_data_set(prev_it, elm_object_item_data_get(next_it));
	}

	elm_object_item_del(sweep_it);

	scrap_item *item = (scrap_item *)data;

	scrap scrap_instance;
	scrap_instance.delete_scrap(item->get_saved_file_path());

	for (int i = 0 ; i < m_browser->get_scrap_view()->m_scrap_list.size() ; i++) {
		if (item == m_browser->get_scrap_view()->m_scrap_list[i]) {
			delete m_browser->get_scrap_view()->m_scrap_list[i];
			m_browser->get_scrap_view()->m_scrap_list.erase(m_browser->get_scrap_view()->m_scrap_list.begin() + i);
			break;
		}
	}

	elm_genlist_realized_items_update(genlist);

	scrap_view *sv = m_browser->get_scrap_view();
	if (sv->m_scrap_list.size() == 0) {
		Evas_Object *content = elm_object_part_content_unset(sv->m_main_layout, "elm.swallow.content");

		evas_object_hide(content);

		Evas_Object *no_contents_layout = elm_layout_add(sv->m_main_layout);
		if (no_contents_layout) {
			evas_object_size_hint_weight_set(no_contents_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_layout_theme_set(no_contents_layout, "layout", "nocontents", "text");
			elm_object_part_text_set(no_contents_layout, "elm.text", BR_STRING_NO_ITEMS);
			elm_object_part_content_set(sv->m_main_layout, "elm.swallow.content", no_contents_layout);
		}

		elm_object_disabled_set(sv->m_more_button, EINA_TRUE);
		elm_object_disabled_set(sv->m_search_button, EINA_TRUE);
	}
}

void scrap_view::__delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_item *item = (scrap_item *)data;
	m_browser->get_scrap_view()->show_delete_popup(NULL, BR_STRING_DELETE_Q, BR_STRING_DELETE, __delete_confirm_cb, BR_STRING_CANCEL, NULL, item);
}

void scrap_view::__edit_checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)data;
	scrap_item *item = genlist_item->item;
	BROWSER_LOGD("check=[%d]", genlist_item->is_checked);
	if (genlist_item->is_tag_index) {
		Elm_Object_Item *it = elm_genlist_item_next_get(genlist_item->it);
		while (it && elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			scrap_view_genlist_item *item_data = (scrap_view_genlist_item *)elm_object_item_data_get(it);
			item_data->is_checked = genlist_item->is_checked;
			elm_genlist_item_update(it);
			it = elm_genlist_item_next_get(it);
		}
	}
}

Evas_Object *scrap_view::__tag_genlist_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);
	scrap_item *item = (scrap_item *)data;

	if (!strcmp(part, "elm.icon")) {
		Evas_Object *favicon = NULL;
		favicon = m_webview_context->get_favicon(item->get_uri());
		if (!favicon) {
			favicon = elm_icon_add(obj);
			elm_icon_standard_set(favicon, browser_img_dir"/faviconDefault.png");
			evas_object_size_hint_aspect_set(favicon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		}

		return favicon;
	}

	return NULL;
}

Evas_Object *scrap_view::__genlist_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);
	scrap_view_genlist_item *genlist_item = (scrap_view_genlist_item *)data;
	scrap_item *item = genlist_item->item;

	if (elm_genlist_decorate_mode_get(obj)) {
		if (!strcmp(part, "elm.edit.icon.1")) {
			Evas_Object *checkbox = elm_check_add(obj);
			if (checkbox) {
				elm_check_state_pointer_set(checkbox, &(genlist_item->is_checked));
				evas_object_propagate_events_set(checkbox, EINA_FALSE);
				evas_object_smart_callback_add(checkbox, "changed", __edit_checkbox_changed_cb, genlist_item);
			}
			return checkbox;
		}
	}

	if (!strcmp(part, "elm.icon")) {
		Evas_Object *favicon = NULL;
		favicon = m_webview_context->get_favicon(item->get_uri());
		if (!favicon) {
			favicon = elm_icon_add(obj);
			elm_icon_standard_set(favicon, browser_img_dir"/faviconDefault.png");
			evas_object_size_hint_aspect_set(favicon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		}

		return favicon;
	}  else if (!strcmp(part, "elm.slide.swallow.1")) {
		Evas_Object *button = elm_button_add(obj);
		if (!button) {
			BROWSER_LOGD("elm_button_add() is failed.");
			return NULL;
		}
		elm_object_style_set(button, "sweep/multiline");
		elm_object_text_set(button, BR_STRING_EDIT_TAGS);
		evas_object_smart_callback_add(button, "clicked", __edit_tag_cb, item);

		return button;
	}  else if (!strcmp(part, "elm.slide.swallow.2")) {
		Evas_Object *button = elm_button_add(obj);
		if (!button) {
			BROWSER_LOGD("elm_button_add() is failed.");
			return NULL;
		}
		elm_object_style_set(button, "sweep/delete");
		elm_object_text_set(button, BR_STRING_DELETE);
		evas_object_smart_callback_add(button, "clicked", __delete_cb, item);

		return button;
	}

	return NULL;
}

