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

#include <Elementary.h>
#include <app.h>

#include "history-view.h"
#include "bookmark-add-view.h"
#include "browser.h"
#include "browser-view.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "history.h"
#include "webview.h"

#define MAX_DATE_LENTH	1024

history_view::history_view(void)
:
	m_item_ic(NULL)
	,m_date_ic(NULL)
	,m_current_sweep_item(NULL)
	,m_genlist(NULL)
	,m_naviframe_item(NULL)
	,m_is_bookmark_on_off_icon_clicked(EINA_FALSE)
	,m_clear_button(NULL)
	,m_main_layout(NULL)
{
	BROWSER_LOGD("");
}

history_view::~history_view(void)
{
	BROWSER_LOGD("");
	for (int i = 0 ; i < m_history_list.size() ; i++)
		delete m_history_list[i];
	m_history_list.clear();

	if (m_item_ic)
		elm_genlist_item_class_free(m_item_ic);

	if (m_date_ic)
		elm_genlist_item_class_free(m_date_ic);

	evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_cb);
}

void history_view::show(void)
{
	BROWSER_LOGD("");

	Evas_Object *layout = _create_main_layout(m_naviframe);
	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_cb, this);
	m_naviframe_item = elm_naviframe_item_push(m_naviframe, BR_STRING_HISTORY, NULL, NULL, layout, NULL);

	m_clear_button = elm_button_add(m_naviframe);
	if (!m_clear_button) {
		BROWSER_LOGE("Failed to make layout");
		return;
	}
	elm_object_text_set(m_clear_button, BR_STRING_CLEAR);
	evas_object_smart_callback_add(m_clear_button, "clicked", __clear_history_button_cb, this);

	elm_object_style_set(m_clear_button, "naviframe/toolbar/default");
	elm_object_item_part_content_set(m_naviframe_item, "toolbar_button1", m_clear_button);
	evas_object_show(m_clear_button);

	if (m_history_list.size() == 0)
		elm_object_disabled_set(m_clear_button, EINA_TRUE);
}

void history_view::__clear_history(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_history()->delete_all();

	Elm_Object_Item *it = elm_genlist_first_item_get(m_browser->get_history_view()->m_genlist);
	while (it) {
		Elm_Object_Item *tmp_it = elm_genlist_item_next_get(it);
		elm_object_item_del(it);
		it = tmp_it;
	}

	for(unsigned int i = 0 ; i < m_browser->get_history_view()->m_history_list.size(); i++ ) {
		if (m_browser->get_history_view()->m_history_list[i])
			delete m_browser->get_history_view()->m_history_list[i];
	}
	m_browser->get_history_view()->m_history_list.clear();

	if (m_browser->get_history_view()->m_history_list.size() == 0) {
		elm_object_disabled_set(m_browser->get_history_view()->m_clear_button, EINA_TRUE);
		evas_object_hide(m_browser->get_history_view()->m_genlist);
	}
}
void history_view::__clear_history_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	history_item *item = (history_item *)data;

	std::string msg = std::string(BR_STRING_CLEAR_HISTORY) + std::string("?");
	m_browser->get_history_view()->show_msg_popup("", msg.c_str(), BR_STRING_YES, __clear_history, BR_STRING_NO, NULL, item);
}

char *history_view::__genlist_date_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	history_item *item = (history_item *)data;

	BROWSER_LOGD("date=[%s]", item->get_date());
	if (!strcmp(part, "elm.text")) {
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
				return strdup(BR_STRING_HISTORY_LAST_7_DAYS);
			else if (day_gap <= 30)
				return strdup(BR_STRING_HISTORY_LAST_MONTH);
			else
				return strdup(BR_STRING_HISTORY_OLDER);
		}

		return strdup(BR_STRING_HISTORY_OLDER);
	}

	return NULL;
}

char *history_view::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	history_item *item = (history_item *)data;
	BROWSER_LOGD("title=[%s], uri=[%s]", item->get_title(), item->get_uri());

	if (!strcmp(part, "elm.text.1")) {
		// convert markup string because the string is cut after '#' in elementary.
		return elm_entry_utf8_to_markup(item->get_title());
	} else if (!strcmp(part, "elm.text.2"))
		return elm_entry_utf8_to_markup(item->get_uri());

	return NULL;
}

void history_view::__bookmark_on_off_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	history_item *item = (history_item *)data;

	m_browser->get_history_view()->m_is_bookmark_on_off_icon_clicked = EINA_TRUE;

	int bookmark_id = -1;
	if (m_browser->get_bookmark()->is_in_bookmark(item->get_uri())) {
		if (!elm_icon_file_set(obj, browser_img_dir"/I01_icon_bookmark_off.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.\n");
		}
		m_browser->get_bookmark()->delete_by_uri(item->get_uri());
	} else {
		if (!elm_icon_file_set(obj, browser_img_dir"/I01_icon_bookmark_on.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.\n");
		}
		m_browser->get_bookmark()->save_bookmark(item->get_title(), item->get_uri(), &bookmark_id);
#if defined(BROWSER_THUMBNAIL_VIEW)
		m_browser->get_bookmark()->set_thumbnail(bookmark_id,
							m_browser->get_history()->get_snapshot(item->get_uri()));
#endif
	}
	Elm_Object_Item *seleted_item = elm_genlist_selected_item_get(m_browser->get_history_view()->m_genlist);
	elm_genlist_item_selected_set(seleted_item, EINA_FALSE);
}

Evas_Object *history_view::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	history_item *item = (history_item *)data;
	BROWSER_LOGD("title=[%s], uri=[%s]", item->get_title(), item->get_uri());

	if (!strcmp(part, "elm.icon.1")) {
		Evas_Object *favicon = m_webview_context->get_favicon(item->get_uri());
		if (!favicon) {
			favicon = elm_icon_add(obj);
			elm_icon_file_set(favicon, browser_img_dir"/faviconDefault.png", NULL);
			evas_object_size_hint_aspect_set(favicon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		}

		return favicon;
	} else if (!strcmp(part, "elm.icon.2")) {
		Evas_Object *bookmark_icon = elm_icon_add(obj);
		if (!bookmark_icon)
			return NULL;

		if(m_browser->get_bookmark()->is_in_bookmark(item->get_uri())){
			if (!elm_icon_file_set(bookmark_icon, browser_img_dir"/I01_icon_bookmark_on.png", NULL)) {
				return NULL;
			}
		}else {
			if (!elm_icon_file_set(bookmark_icon, browser_img_dir"/I01_icon_bookmark_off.png", NULL)) {
				BROWSER_LOGE("elm_icon_file_set is failed.\n");
				return NULL;
			}
		}
		evas_object_size_hint_aspect_set(bookmark_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		evas_object_propagate_events_set(bookmark_icon, EINA_FALSE);
		evas_object_repeat_events_set(bookmark_icon, EINA_FALSE);

		evas_object_smart_callback_add(bookmark_icon, "clicked", __bookmark_on_off_icon_clicked_cb, item);

		return bookmark_icon;
	} else if (!strcmp(part, "elm.slide.swallow.1")) {
		Evas_Object *button = elm_button_add(obj);
		if (!button) {
			BROWSER_LOGD("elm_button_add() is failed.");
			return NULL;
		}
		elm_object_style_set(button, "sweep/multiline");
		elm_object_text_set(button, BR_STRING_ADD_TO_BOOKMARKS);
		evas_object_smart_callback_add(button, "clicked", __slide_add_to_bookmark_button_clicked_cb, item);

		return button;
	} else if (!strcmp(part, "elm.slide.swallow.2")) {
		Evas_Object *button = elm_button_add(obj);
		if (!button) {
			BROWSER_LOGD("elm_button_add() is failed.");
			return NULL;
		}
		elm_object_style_set(button, "sweep/multiline");
		elm_object_text_set(button, BR_STRING_DELETE);
		evas_object_smart_callback_add(button, "clicked", __slide_delete_button_clicked_cb, item);
		return button;
	}
	return NULL;
}

Evas_Object *history_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	memset(item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));

	item_ic->decorate_item_style = "mode/slide2";
	item_ic->item_style = "dialogue/2text.2icon.3";
	item_ic->decorate_all_item_style = "dialogue/edit";
	item_ic->func.text_get = __genlist_label_get_cb;
	item_ic->func.content_get = __genlist_icon_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

	Elm_Genlist_Item_Class *date_ic = elm_genlist_item_class_new();
	memset(date_ic, 0x00, sizeof(Elm_Genlist_Item_Class));

	date_ic->item_style = "dialogue/title";
	date_ic->decorate_item_style = NULL;
	date_ic->decorate_all_item_style = NULL;
	date_ic->func.text_get = __genlist_date_label_get_cb;
	date_ic->func.content_get = NULL;
	date_ic->func.state_get = NULL;
	date_ic->func.del = NULL;

	m_item_ic = item_ic;
	m_date_ic = date_ic;

	return genlist;
}

void history_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	history_item *item = (history_item *)data;
	if (m_browser->get_history_view()->m_is_bookmark_on_off_icon_clicked) {
		m_browser->get_history_view()->m_is_bookmark_on_off_icon_clicked = EINA_FALSE;
		Elm_Object_Item *selected_item = (Elm_Object_Item *)event_info;
		elm_genlist_item_selected_set(selected_item, EINA_FALSE);
		return;
	}
	m_browser->get_browser_view()->get_current_webview()->load_uri(item->get_uri());

	elm_naviframe_item_pop(m_naviframe);
}

Evas_Object *history_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	m_genlist = _create_genlist(parent);

	m_history_list = m_browser->get_history()->get_history_list();

	BROWSER_LOGD("history count=%d", m_history_list.size());

	int prev_year = 0;
	int prev_month = 0;
	int prev_day = 0;
	for (int i = 0 ; i < m_history_list.size() ; i++) {
		Elm_Object_Item *it = NULL;
		BROWSER_LOGD("m_history_list[i]->get_date()=[%s]", m_history_list[i]->get_date());

		if (m_history_list[i]->get_date() && strlen(m_history_list[i]->get_date())) {
			if (i == 0) {
				Elm_Object_Item *it = elm_genlist_item_append(m_genlist, m_date_ic,m_history_list[i], NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				if (m_history_list[i]->get_time_stamp_type() != m_history_list[i - 1]->get_time_stamp_type()) {
					Elm_Object_Item *it = elm_genlist_item_append(m_genlist, m_date_ic, m_history_list[i], NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
					elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
				}
			}
		}
		evas_object_smart_callback_add(m_genlist, "drag,start,right", __sweep_right_genlist_cb, this);
		evas_object_smart_callback_add(m_genlist, "drag,start,left", __sweep_left_genlist_cb, this);
		evas_object_smart_callback_add(m_genlist, "drag,start,up", __sweep_cancel_genlist_cb, this);
		evas_object_smart_callback_add(m_genlist, "drag,start,down", __sweep_cancel_genlist_cb, this);
		elm_genlist_item_append(m_genlist, m_item_ic, m_history_list[i], NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, m_history_list[i]);
	}

	return m_genlist;
}

void history_view::__naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	history_view*hv = (history_view *)data;
	if (hv->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		return;

	m_browser->delete_bookmark_add_view();
}

void history_view::__sweep_right_genlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Elm_Object_Item *it = (Elm_Object_Item *)elm_genlist_decorated_item_get(obj);
	if (it && (it != (Elm_Object_Item *)event_info)) {
		elm_genlist_item_decorate_mode_set(it, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DEFAULT);
	} else {
		BROWSER_LOGD("no decorated mode item");
	}

	if (!elm_genlist_decorate_mode_get(obj)) {
		elm_genlist_item_decorate_mode_set((Elm_Object_Item *)event_info, "slide", EINA_TRUE);
		elm_genlist_item_select_mode_set((Elm_Object_Item *)event_info, ELM_OBJECT_SELECT_MODE_NONE);
	}

	m_browser->get_history_view()->m_current_sweep_item = (Elm_Object_Item *)event_info;
}

void history_view::__sweep_left_genlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (elm_genlist_decorate_mode_get(obj))
		return;
	else {
		elm_genlist_item_decorate_mode_set((Elm_Object_Item *)event_info, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set((Elm_Object_Item *)event_info, ELM_OBJECT_SELECT_MODE_DEFAULT);
	}

	m_browser->get_history_view()->m_current_sweep_item = (Elm_Object_Item *)event_info;
}

void history_view::__sweep_cancel_genlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Elm_Object_Item *it = (Elm_Object_Item*)elm_genlist_decorated_item_get(obj);
	if (it) {
		elm_genlist_item_decorate_mode_set(it, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DEFAULT);

		if (it == m_browser->get_history_view()->m_current_sweep_item)
			m_browser->get_history_view()->m_current_sweep_item = NULL;
	}
}

void history_view::__slide_add_to_bookmark_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;
	history_item *item = (history_item *)data;
	m_browser->create_bookmark_add_view(item->get_title(), item->get_uri())->show();
}

void history_view::__slide_delete_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;
	history_item *item = (history_item *)data;
	m_browser->get_history_view()->show_msg_popup("", BR_STRING_CLEAR_ALL_HISTORY_DATA_Q, BR_STRING_YES, __delete_confirm_response_by_slide_button_cb, BR_STRING_NO, NULL, item);
}

void history_view::__delete_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	history_item *item = (history_item *)data;
	m_browser->get_history_view()->_delete_history_item_by_slide_button(item);
}

void history_view::_delete_date_only_label_genlist_item(void)
{
	BROWSER_LOGD("");

	Elm_Object_Item *it = elm_genlist_first_item_get(m_browser->get_history_view()->m_genlist);
	/* Remove date only label item */
	while (it) {
		if (elm_genlist_item_select_mode_get(it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			Elm_Object_Item *tmp_it = elm_genlist_item_next_get(it);
			if (!tmp_it || elm_genlist_item_select_mode_get(tmp_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
				tmp_it = it;
				it = elm_genlist_item_next_get(it);
				elm_object_item_del(tmp_it);
				continue;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	if (m_history_list.size() == 0)
		elm_object_disabled_set(m_clear_button, EINA_TRUE);
}

void history_view::_delete_history_item_by_slide_button(history_item *item)
{
	BROWSER_LOGD("");

	m_browser->get_history()->delete_history(item->get_uri());
	elm_object_item_del(m_browser->get_history_view()->m_current_sweep_item);

	Elm_Object_Item *it = elm_genlist_first_item_get(m_browser->get_history_view()->m_genlist);

	while (it) {
		history_item *item_data = NULL;
		item_data = (history_item *)elm_object_item_data_get(it);
		if (item_data && (elm_genlist_item_select_mode_get(it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
			if (item_data->get_uri()== item->get_uri()) {
				elm_object_item_del(it);
				break;
			}
		}
		it = elm_genlist_item_next_get(it);
	}

	for(unsigned int i = 0 ; i < m_browser->get_history_view()->m_history_list.size(); i++) {
		if (m_browser->get_history_view()->m_history_list[i]->get_uri()== item->get_uri()) {
			delete m_browser->get_history_view()->m_history_list[i];
			m_browser->get_history_view()->m_history_list.erase(m_browser->get_history_view()->m_history_list.begin() + i);
			break;
		}
	}

	m_browser->get_history_view()->_delete_date_only_label_genlist_item();
}
