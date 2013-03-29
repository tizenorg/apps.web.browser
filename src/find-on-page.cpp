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

#include "find-on-page.h"

#include <Elementary.h>
#include <string>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-view.h"
#include "webview.h"

#define find_on_page_edj_path browser_edj_dir"/find-on-page.edj"

find_on_page::find_on_page(void)
:
	m_entry(NULL)
	,m_back_button(NULL)
	,m_entry_layout(NULL)
	,m_total_count(0)
	,m_current_index(0)
	,m_down_button(NULL)
	,m_up_button(NULL)
	,m_input_word(NULL)
	,m_webview(NULL)
{
	BROWSER_LOGD("");
	elm_theme_extension_add(NULL, find_on_page_edj_path);

	m_main_layout = _create_main_layout(m_window);
}

find_on_page::~find_on_page(void)
{
	BROWSER_LOGD("");

	elm_theme_extension_del(NULL, find_on_page_edj_path);

	eina_stringshare_del(m_input_word);
}

void find_on_page::show_ime(void)
{
	elm_object_focus_set(m_entry, EINA_TRUE);
}

void find_on_page::unfocus_entry(void)
{
	elm_object_focus_set(m_entry, EINA_FALSE);
	elm_object_focus_set(m_back_button, EINA_TRUE);
}

void find_on_page::set_text(const char *text)
{
	elm_entry_entry_set(m_entry, text);
	elm_entry_cursor_end_set(m_entry);
}

void find_on_page::clear_text(void)
{
	m_total_count = 0;
	m_current_index = 0;
	_set_count(0, 0);
	elm_entry_entry_set(m_entry, "");
}

Evas_Object *find_on_page::_create_main_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGD("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, find_on_page_edj_path, "find-on-page-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *entry_layout = _create_entry_layout(layout);
	elm_object_part_content_set(layout, "elm.swallow.entry_layout", entry_layout);

	Evas_Object *back_button = elm_button_add(layout);
	if (!back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(back_button, "browser/uri_input_back");
	evas_object_smart_callback_add(back_button, "clicked", __back_clicked_cb, this);
	elm_object_part_content_set(layout, "elm.swallow.back_button", back_button);

	Evas_Object *down_button = elm_button_add(layout);
	if (!down_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(down_button, "browser/find_down");
	evas_object_smart_callback_add(down_button, "clicked", __down_clicked_cb, this);
	// The down / up position of edc is changed, but just keep edc and change the source code.
	elm_object_part_content_set(layout, "elm.swallow.up_button", down_button);

	Evas_Object *up_button = elm_button_add(layout);
	if (!up_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(up_button, "browser/find_up");
	evas_object_smart_callback_add(up_button, "clicked", __up_clicked_cb, this);
	elm_object_part_content_set(layout, "elm.swallow.down_button", up_button);

	m_back_button = back_button;
	m_entry_layout = entry_layout;
	m_down_button = down_button;
	m_up_button = up_button;

	_set_count(0, 0);

	return layout;
}

void find_on_page::_set_count(int index, int total)
{
	char text_buffer[100] = {0, };
	sprintf(text_buffer, "%d/%d", index, total);
	edje_object_part_text_set(elm_layout_edje_get(m_entry_layout), "elm.text.count", text_buffer);

	_disable_down_button(EINA_FALSE);
	_disable_up_button(EINA_FALSE);

	if (index == 1 || total == 0)
		_disable_up_button(EINA_TRUE);
	if (index == total || total == 0)
		_disable_down_button(EINA_TRUE);
}

void find_on_page::_disable_down_button(Eina_Bool disable)
{
	elm_object_disabled_set(m_down_button, disable);
}

void find_on_page::_disable_up_button(Eina_Bool disable)
{
	elm_object_disabled_set(m_up_button, disable);
}

Evas_Object *find_on_page::_create_entry_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *entry_layout = elm_layout_add(parent);
	if (!entry_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(entry_layout, find_on_page_edj_path, "entry-layout");
	evas_object_size_hint_weight_set(entry_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *entry = elm_entry_add(entry_layout);
	if (!entry) {
		BROWSER_LOGE("elm_entry_add failed");
		return NULL;
	}

	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_prediction_allow_set(entry, EINA_FALSE);
	elm_entry_autocapital_type_set(entry, ELM_AUTOCAPITAL_TYPE_NONE);

	elm_entry_input_panel_enabled_set(entry, EINA_TRUE);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);

	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_smart_callback_add(entry, "activated", __enter_key_cb, this);
	evas_object_smart_callback_add(entry, "clicked", __entry_clicked_cb, this);
	evas_object_smart_callback_add(entry, "changed", __entry_changed_cb, this);

	/* 'ellipsis=1' is not supported by elm_entry_text_style_user_push, but just specify it */
	elm_entry_text_style_user_push(entry, uri_entry_style);

	evas_object_show(entry);

	elm_object_part_content_set(entry_layout, "elm.swallow.entry", entry);

	edje_object_signal_callback_add(elm_layout_edje_get(entry_layout), "mouse,clicked,1", "delete_icon,touch_area", __delete_clicked_cb, entry);

	m_entry = entry;

	return entry_layout;

}

void find_on_page::__text_found_cb(void *data, Evas_Object *obj, void *event_info)
{
	find_on_page *fop = (find_on_page *)data;

	unsigned int match_count = *(unsigned int *)event_info;
	BROWSER_LOGD("match_count=%d", match_count);
	fop->m_total_count = match_count;
	if (match_count == 0)
		fop->m_current_index = 0;

	fop->_set_count(fop->m_current_index, fop->m_total_count);
}

void find_on_page::__entry_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (!m_browser->get_browser_view()->is_ime_on()) {
		elm_entry_cursor_end_set(obj);
		elm_object_focus_set(obj, EINA_FALSE);
		elm_object_focus_set(obj, EINA_TRUE);
	}
}

void find_on_page::__entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	find_on_page *fop = (find_on_page *)data;

	EINA_SAFETY_ON_NULL_RETURN(fop->m_webview);

	fop->m_current_index = 1;

	const char *text = elm_entry_entry_get(obj);
	BROWSER_LOGD("text=[%s]", text);
	eina_stringshare_replace(&fop->m_input_word, text);
	fop->m_webview->find_word(text, EINA_TRUE, __text_found_cb, data);
}

void find_on_page::__enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
}

void find_on_page::__delete_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	elm_entry_entry_set((Evas_Object *)data, "");
}

void find_on_page::__back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	m_browser->get_browser_view()->show_find_on_page(EINA_FALSE);
}

void find_on_page::__down_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	find_on_page *fop = (find_on_page *)data;
	EINA_SAFETY_ON_NULL_RETURN(fop->m_webview);

	if (fop->m_total_count == 0)
		return;

	fop->m_current_index++;
	fop->m_webview->find_word(fop->m_input_word, EINA_TRUE, __text_found_cb, data);
}

void find_on_page::__up_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	find_on_page *fop = (find_on_page *)data;
	EINA_SAFETY_ON_NULL_RETURN(fop->m_webview);

	if (fop->m_total_count == 0)
		return;

	fop->m_current_index--;
	fop->m_webview->find_word(fop->m_input_word, EINA_FALSE, __text_found_cb, data);
}

