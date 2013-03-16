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

#include "most-visited.h"

#include <string>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "history.h"
#include "history-item.h"
#include "most-visited-item.h"
#include "platform-service.h"
#include "preference.h"
#include "uri-bar.h"
#include "uri-input-bar.h"
#include "webview.h"

#define most_visited_item_w	((320 + 40) * efl_scale)
#define most_visited_item_h	((220 + 52 + 40) * efl_scale)

#define most_visited_edj_path browser_edj_dir"/most-visited.edj"

most_visited::most_visited(void)
:
	m_gengrid(NULL)
	,m_gengrid_ic(NULL)
{
	BROWSER_LOGD("");
	elm_theme_extension_add(NULL, most_visited_edj_path);

	m_main_layout = _create_main_layout(m_window);
}

most_visited::~most_visited(void)
{
	BROWSER_LOGD("");
	for (int i = 0 ; i < m_most_visited_item_list.size() ; i++)
		delete m_most_visited_item_list[i];
	m_most_visited_item_list.clear();

	if (m_gengrid_ic)
		elm_gengrid_item_class_free(m_gengrid_ic);

	if (m_main_layout)
		evas_object_del(m_main_layout);

	elm_theme_extension_add(NULL, most_visited_edj_path);
}

char *most_visited::__get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

	most_visited_item *item = (most_visited_item *)data;
	if (!strcmp(part, "elm.text")) {
		if (item->get_title() && strlen(item->get_title())) {
			std::string title_str = std::string("<font_size=28><color=#343432>") + std::string(item->get_title()) + std::string("</color></font_size>");
			return strdup(title_str.c_str());
		}
	}

	return NULL;
}

Evas_Object *most_visited::__get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

	most_visited_item *item = (most_visited_item *)data;
	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *layout = item->copy_layout();
		return layout;
	}

	return NULL;
}

typedef struct _option_callback {
	Elm_Object_Item *selected_it;
	void *user_data;
	most_visited_item *item;
} option_callback;

void most_visited::__private_open_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	option_callback *oc = (option_callback *)data;
	if (oc) {
		most_visited *mv = (most_visited *)oc->user_data;
		most_visited_item *selected_mvi = (most_visited_item *)oc->item;

		m_browser->get_browser_view()->get_uri_bar()->set_private_mode(EINA_TRUE);

		webview *wv = m_browser->get_browser_view()->get_current_webview();
		wv->load_uri(selected_mvi->get_uri());

		free(oc);
	}

	m_browser->get_browser_view()->destroy_popup(obj);
}

void most_visited::__set_as_homepage_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	option_callback *oc = (option_callback *)data;
	if (oc) {
		most_visited_item *selected_mvi = (most_visited_item *)oc->item;
		m_preference->set_user_homagepage(selected_mvi->get_uri());
		m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);

		free(oc);
	}

	m_browser->get_browser_view()->destroy_popup(obj);
}

void most_visited::__delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	option_callback *oc = (option_callback *)data;
	if (oc) {
		most_visited *mv = (most_visited *)oc->user_data;
		most_visited_item *selected_mvi = (most_visited_item *)oc->item;
		most_visited_item *last_mvi = mv->m_most_visited_item_list[mv->m_most_visited_item_list.size() - 1];

		history_item last_hi(last_mvi->get_title(), last_mvi->get_uri(), NULL, last_mvi->get_visit_count());
		history_item selected_hi(selected_mvi->get_title(), selected_mvi->get_uri(), NULL, selected_mvi->get_visit_count());

		// Get next most visited site.
		history_item *next_history_item = m_browser->get_history()->get_next_history_order_by_visit_count(&last_hi);
		BROWSER_LOGD("next_history_item=[%d]", next_history_item);
		m_browser->get_history()->reset_history_visit_count(&selected_hi);

		if (next_history_item)
			BROWSER_LOGD("next history title=[%s], uri=[%s]", next_history_item->get_title(), next_history_item->get_uri());

		if (oc->selected_it) {
			int index = 0;
			Elm_Object_Item *it = elm_gengrid_first_item_get(mv->m_gengrid);
			while (it) {
				if (oc->selected_it == it)
					break;
			
				it = elm_gengrid_item_next_get(it);
				index++;
			}

			elm_object_item_del(oc->selected_it);

			delete mv->m_most_visited_item_list[index];
			mv->m_most_visited_item_list.erase(mv->m_most_visited_item_list.begin() + index);
		}

		if (next_history_item) {
			most_visited_item *next_mi = new most_visited_item(next_history_item->copy_snapshot(), next_history_item->get_title(), next_history_item->get_uri(), next_history_item->get_visit_count());
			elm_gengrid_item_append(mv->m_gengrid, mv->m_gengrid_ic, next_mi, __item_clicked_cb, next_mi);
			mv->m_most_visited_item_list.push_back(next_mi);
			delete next_history_item;
		} else {
			elm_gengrid_item_append(mv->m_gengrid, mv->m_gengrid_ic, NULL, NULL, NULL);
		}

		free(oc);
	}

	m_browser->get_browser_view()->destroy_popup(obj);
}

void most_visited::__option_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	option_callback *oc = (option_callback *)data;
	if (oc)
		free(oc);
}

void most_visited::_show_option_popup(most_visited_item *item, Elm_Object_Item *selected_it)
{
	Evas_Object *content_list = elm_list_add(m_window);
	if (!content_list) {
		BROWSER_LOGE("elm_list_add failed");
	}
	elm_list_mode_set(content_list, ELM_LIST_EXPAND);

	option_callback *oc = (option_callback *)malloc(sizeof(option_callback));
	if (oc) {
		oc->selected_it = selected_it;
		oc->user_data = this;
		oc->item = item;
	}

	elm_list_item_append(content_list, BR_STRING_PRIVATE_BROWSER, NULL, NULL, __private_open_cb, oc);
	elm_list_item_append(content_list, BR_STRING_SET_AS_HOMEPAGE, NULL, NULL, __set_as_homepage_cb, oc);
	elm_list_item_append(content_list, BR_STRING_DELETE, NULL, NULL, __delete_cb, oc);

	m_browser->get_browser_view()->show_content_popup(NULL, content_list, BR_STRING_CANCEL, __option_popup_cancel_cb, NULL, NULL, oc);
}

void most_visited::__item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	most_visited *mv = (most_visited *)data;
	Elm_Object_Item *selected_it = (Elm_Object_Item *)event_info;
	int index = 0;
	Elm_Object_Item *it = elm_gengrid_first_item_get(obj);
	while (it) {
		if (selected_it == it)
			break;

		it = elm_gengrid_item_next_get(it);
		index++;
	}
	BROWSER_LOGD("selected index = %d", index);

	// Issue : longpress on most visited item during ime, the ime is shown shortly when load uri.
	// So unfocus the entry to hide ime.
	m_browser->get_browser_view()->get_uri_input_bar()->unfocus_entry();

	if (index < mv->m_most_visited_item_list.size()) {
		mv->_show_option_popup(mv->m_most_visited_item_list[index], selected_it);
	}
}

void most_visited::__gengrid_scroll_cb(void *data, Evas_Object *obj, void *event_info)
{
	m_browser->get_browser_view()->get_uri_input_bar()->unfocus_entry();
}

Evas_Object *most_visited::_create_gengrid(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *gengrid = elm_gengrid_add(parent);
	if (!gengrid) {
		BROWSER_LOGE("elm_gengrid_add failed");
		return NULL;		
	}
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_gengrid_item_size_set(gengrid, most_visited_item_w, most_visited_item_h);
	elm_gengrid_align_set(gengrid, 0.5, 0.0);
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);

	m_gengrid_ic = elm_gengrid_item_class_new();
	m_gengrid_ic->item_style = "browser_gridtext";
	m_gengrid_ic->func.text_get = __get_text_cb;
	m_gengrid_ic->func.content_get = __get_content_cb;
	m_gengrid_ic->func.state_get = NULL;
	m_gengrid_ic->func.del = NULL;

	evas_object_smart_callback_add(gengrid, "longpressed", __item_longpressed_cb, this);
	evas_object_smart_callback_add(gengrid, "scroll", __gengrid_scroll_cb, this);

	return gengrid;
}

void most_visited::__item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	most_visited_item *item = (most_visited_item *)data;
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	wv->load_uri(item->get_uri());
}

Evas_Object *most_visited::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	std::vector<history_item *> history_list;
	history_list = m_browser->get_history()->get_histories_order_by_visit_count(most_visited_item_count);
	if (history_list.size() == 0)
		return NULL;

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, most_visited_edj_path, "most-visited-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *gengrid = _create_gengrid(layout);
	if (!gengrid) {
		BROWSER_LOGE("elm_gengrid_add failed");
		return NULL;		
	}
	elm_object_part_content_set(layout, "elm.swallow.gengrid", gengrid);

	for (int i = 0 ; i < history_list.size() ; i++) {
		BROWSER_LOGD("title=[%s], history=[%s], count=[%d]", history_list[i]->get_title(), history_list[i]->get_uri(), history_list[i]->get_visit_count());
		most_visited_item *item = new most_visited_item(history_list[i]->copy_snapshot(), history_list[i]->get_title(), history_list[i]->get_uri(), history_list[i]->get_visit_count());
		m_most_visited_item_list.push_back(item);

		elm_gengrid_item_append(gengrid, m_gengrid_ic, item, __item_clicked_cb, item);
	}
	for (int i = history_list.size() ; i < most_visited_item_count ; i++)
		elm_gengrid_item_append(gengrid, m_gengrid_ic, NULL, NULL, NULL);

	for (int i = 0 ; i < history_list.size() ; i++)
		delete history_list[i];
	history_list.clear();

	m_gengrid = gengrid;

	return layout;
}

