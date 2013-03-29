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

#include "uri-input-bar.h"

#include <Elementary.h>
#include <regex.h>
#include <string>
#include "browser.h"
#include "browser-dlog.h"
#include "browser-view.h"
#include "preference.h"
#include "uri-bar.h"
#include "webview.h"

#define uri_input_bar_edj_path browser_edj_dir"/uri-input-bar.edj"

#define URLEXPR "((https?|ftp|gopher|telnet|file|notes|ms-help):((//)|(\\\\))+[\w\d:#@%/;$()~_?\+-=\\\.&]*)"

static Eina_Bool _is_regular_express(const char *uri)
{
	BROWSER_LOGD("uri=[%s]", uri);

	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);
	regex_t regex;
	if (regcomp(&regex, URLEXPR, REG_EXTENDED | REG_ICASE) != 0) {
		BROWSER_LOGD("regcomp failed");
		return EINA_FALSE;
	}

	if (regexec(&regex, uri, 0, NULL, REG_NOTEOL) == 0) {
		BROWSER_LOGD("url expression");
		return EINA_TRUE;
	}

	if (!strcmp(uri, blank_page))
		return EINA_TRUE;

	int len = strlen(uri);
	if (*uri != '.' && *(uri + len - 1) != '.' && strstr(uri, ".")) {
		BROWSER_LOGD("url tmp expression");
		return EINA_TRUE;
	}

	return EINA_FALSE;
}

uri_input_bar::uri_input_bar(Evas_Object *parent)
:
	m_uri_entry(NULL)
	,m_back_button(NULL)
{
	BROWSER_LOGD("");
	elm_theme_extension_add(NULL, uri_input_bar_edj_path);

	m_main_layout = _create_main_layout(parent);
}

uri_input_bar::~uri_input_bar(void)
{
	BROWSER_LOGD("");

	elm_theme_extension_del(NULL, uri_input_bar_edj_path);
}

void uri_input_bar::set_uri(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN(uri);
	char *markup = elm_entry_utf8_to_markup(uri);

	elm_entry_entry_set(m_uri_entry, markup);

	free(markup);
}

void uri_input_bar::show_ime(void)
{
	const char *init_text = m_browser->get_browser_view()->get_current_webview()->get_uri();
	BROWSER_LOGD("init_text=[%s]", init_text);

	set_uri(init_text);

	elm_entry_context_menu_disabled_set(m_uri_entry, EINA_TRUE);

	const char *input_uri = elm_entry_entry_get(m_uri_entry);
	if (input_uri && strlen(input_uri))
		elm_entry_select_all(m_uri_entry);
	elm_entry_cursor_end_set(m_uri_entry);

	elm_object_focus_set(m_uri_entry, EINA_TRUE);
}

void uri_input_bar::unfocus_entry(void)
{
	elm_object_focus_set(m_uri_entry, EINA_FALSE);
	elm_object_focus_set(m_back_button, EINA_TRUE);
}

static void _dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
}

void uri_input_bar::__google_engine_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	m_preference->set_search_engine_type(0);

	evas_object_del(obj);
}

void uri_input_bar::__yahoo_engine_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	m_preference->set_search_engine_type(1);

	evas_object_del(obj);
}

void uri_input_bar::__bing_engine_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	m_preference->set_search_engine_type(2);

	evas_object_del(obj);
}

void uri_input_bar::_show_search_context_popup(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN(parent);
	Evas_Object *context_popup = elm_ctxpopup_add(m_window);
	if (!context_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	elm_ctxpopup_horizontal_set(context_popup, EINA_TRUE);
	elm_ctxpopup_direction_priority_set(context_popup, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_DOWN);
	evas_object_smart_callback_add(context_popup,"dismissed", _dismissed_cb, NULL);

	Evas_Object *icon = elm_image_add(context_popup);
	if (!icon) {
		BROWSER_LOGE("elm_image_add");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_CP_icon_google.png", NULL);
	elm_ctxpopup_item_append(context_popup, NULL, icon, __google_engine_selected_cb, this);

	icon = elm_image_add(context_popup);
	if (!icon) {
		BROWSER_LOGE("elm_image_add");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_CP_icon_yahoo.png", NULL);
	elm_ctxpopup_item_append(context_popup, NULL, icon, __yahoo_engine_selected_cb, this);

	icon = elm_image_add(context_popup);
	if (!icon) {
		BROWSER_LOGE("elm_image_add");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_CP_icon_bing.png", NULL);
	elm_ctxpopup_item_append(context_popup, NULL, icon, __bing_engine_selected_cb, this);

	Evas_Coord x, y, w, h;
	evas_object_geometry_get(parent, &x, &y, &w, &h);
	evas_object_move(context_popup, x + (w / 2), y + (h /2));
	evas_object_show(context_popup);
}

void uri_input_bar::__search_engine_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	uri_input_bar *uib = (uri_input_bar *)data;
	uib->_show_search_context_popup(obj);
}

void uri_input_bar::__back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_browser_view()->show_uri_input_bar(EINA_FALSE);
}

Evas_Object *uri_input_bar::_create_main_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGD("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, uri_input_bar_edj_path, "uri-input-bar-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *entry_layout = _create_entry_layout(layout);
	elm_object_part_content_set(layout, "elm.swallow.entry_layout", entry_layout);

	Evas_Object *circle_button = elm_button_add(layout);
	if (!circle_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}

	elm_object_style_set(circle_button, "browser/uri_input_search");

	evas_object_smart_callback_add(circle_button, "clicked", __search_engine_clicked_cb, this);
	elm_object_part_content_set(layout, "elm.swallow.circle_button", circle_button);

	Evas_Object *back_button = elm_button_add(layout);
	if (!back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(back_button, "browser/uri_input_back");
	evas_object_smart_callback_add(back_button, "clicked", __back_clicked_cb, NULL);
	elm_object_part_content_set(layout, "elm.swallow.back_button", back_button);

	m_back_button = back_button;

	return layout;
}

void uri_input_bar::__delete_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	elm_entry_entry_set((Evas_Object *)data, "");
}

void uri_input_bar::__enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *input_uri = elm_entry_entry_get(obj);
	if (!input_uri || strlen(input_uri) == 0)
		return;

	char *utf8 = elm_entry_markup_to_utf8(input_uri);
	if (!utf8 || strlen(utf8) == 0)
		return;

	char *uri = strdup(utf8);
	if (!uri) {
		free(utf8);
		return;
	}

	BROWSER_LOGD("input_uri=[%s]", utf8);
	m_browser->get_browser_view()->show_uri_input_bar(EINA_FALSE);

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (_is_regular_express(uri)) {
		wv->load_uri(uri);
	} else {
		int search_engine_type = m_preference->get_search_engine_type();
		if (search_engine_type == 0) {
			std::string query_uri = std::string(google_query_uri) + std::string(uri);
			wv->load_uri(query_uri.c_str());
		} else if (search_engine_type == 1) {
			std::string query_uri = std::string(yahoo_query_uri) + std::string(uri);
			wv->load_uri(query_uri.c_str());
		} else if (search_engine_type == 2) {
			std::string query_uri = std::string(bing_query_uri_prefix) + std::string(uri) + std::string(bing_query_uri_postfix);
			wv->load_uri(query_uri.c_str());
		}
	}

	free(uri);
	free(utf8);
}

void uri_input_bar::__entry_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	// To invoke keypad after hiding.
	elm_entry_context_menu_disabled_set(obj, EINA_FALSE);

	if (!m_browser->get_browser_view()->is_ime_on()) {
		elm_object_focus_set(obj, EINA_FALSE);
		elm_object_focus_set(obj, EINA_TRUE);
	}
}

void uri_input_bar::__entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	uri_input_bar *uib = (uri_input_bar *)data;
	const char *input_uri = elm_entry_entry_get(obj);
	BROWSER_LOGD("input_uri=[%s]", input_uri);
	if (_is_regular_express(input_uri))
		elm_entry_input_panel_return_key_type_set(uib->m_uri_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_GO);
	else
		elm_entry_input_panel_return_key_type_set(uib->m_uri_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);
}

Evas_Object *uri_input_bar::_create_entry_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *entry_layout = elm_layout_add(parent);
	if (!entry_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(entry_layout, uri_input_bar_edj_path, "entry-layout");
	evas_object_size_hint_weight_set(entry_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *uri_entry = elm_entry_add(entry_layout);
	if (!uri_entry) {
		BROWSER_LOGE("elm_entry_add failed");
		return NULL;
	}

	elm_entry_single_line_set(uri_entry, EINA_TRUE);
	elm_entry_scrollable_set(uri_entry, EINA_TRUE);

	elm_entry_input_panel_enabled_set(uri_entry, EINA_TRUE);
	elm_entry_input_panel_layout_set(uri_entry, ELM_INPUT_PANEL_LAYOUT_URL);
	elm_entry_input_panel_return_key_type_set(uri_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_GO);

	evas_object_size_hint_weight_set(uri_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_smart_callback_add(uri_entry, "activated", __enter_key_cb, this);
	evas_object_smart_callback_add(uri_entry, "clicked", __entry_clicked_cb, this);
	evas_object_smart_callback_add(uri_entry, "changed", __entry_changed_cb, this);

	/* 'ellipsis=1' is not supported by elm_entry_text_style_user_push, but just specify it */
	elm_entry_text_style_user_push(uri_entry, uri_entry_style);

	evas_object_show(uri_entry);

	elm_object_part_content_set(entry_layout, "elm.swallow.entry", uri_entry);

	edje_object_signal_callback_add(elm_layout_edje_get(entry_layout), "mouse,clicked,1", "delete_icon,touch_area", __delete_clicked_cb, uri_entry);

	m_uri_entry = uri_entry;

	return entry_layout;

}

