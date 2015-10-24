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
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 */

#include "find-on-page.h"

#include <Elementary.h>
#include <efl_extension.h>
#include <notification.h>
#include <string>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "webview.h"

#define find_on_page_edj_path browser_edj_dir"/find-on-page.edj"
#define FIND_WORD_MAX_COUNT	10000
#define FIND_ON_PAGE_ENTRY_STYLE "DEFAULT=' font_size=22 color=#000000 ellipsis=1'"


static void __keypad_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *entry = (Evas_Object *)data;
	elm_object_part_text_set(entry, "elm.guide", (std::string("<font_size=22>") + BR_STRING_FIND_ON_PAGE + "</font>").c_str());

	Evas_Object *access = elm_access_object_get(elm_entry_textblock_get(entry));
	elm_access_info_set(access, ELM_ACCESS_TYPE, BR_STRING_TEXT_FIELD_T);
}

find_on_page::find_on_page(Evas_Object *parent)
	: m_entry(NULL)
	, m_down_button(NULL)
	, m_up_button(NULL)
	, m_clear_icon_object(NULL)
	, m_total_count(0)
	, m_current_index(0)
	, m_input_word(NULL)
	, m_webview(NULL)
{
	BROWSER_LOGD("");
	elm_theme_extension_add(NULL, find_on_page_edj_path);

	m_main_layout = _create_main_layout(parent);
	_set_count(0, 0);

	vconf_notify_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, __font_size_changed_cb, this);
}

find_on_page::~find_on_page(void)
{
	BROWSER_LOGD("");

	Evas_Object *conformant = m_browser->get_browser_view()->get_conformant();
	evas_object_smart_callback_del(conformant, "virtualkeypad,state,on", __keypad_show_cb);
	evas_object_smart_callback_del(conformant, "clipboard,state,on", __keypad_show_cb);

	elm_theme_extension_del(NULL, find_on_page_edj_path);

	evas_object_smart_callback_del(m_down_button, "clicked", __down_clicked_cb);
	evas_object_smart_callback_del(m_up_button, "clicked", __up_clicked_cb);

	evas_object_smart_callback_del(m_entry, "activated", __enter_key_cb);
	evas_object_smart_callback_del(m_entry, "preedit,changed", __entry_changed_cb);
	evas_object_smart_callback_del(m_entry, "changed", __entry_changed_cb);

	evas_object_del(m_main_layout);

	eina_stringshare_del(m_input_word);
	vconf_ignore_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, __font_size_changed_cb);
}

void find_on_page::show_ime(void)
{
	BROWSER_LOGD("");

	elm_object_focus_set(m_entry, EINA_TRUE);
	elm_entry_cursor_end_set(m_entry);
}

void find_on_page::set_text(const char *text)
{
	BROWSER_SECURE_LOGD("%s", text);

	elm_entry_entry_set(m_entry, "");
	char *markup_text = elm_entry_utf8_to_markup(text);
	elm_entry_entry_append(m_entry, markup_text);
	if (markup_text)
		free(markup_text);
}

void find_on_page::clear_text(void)
{
	BROWSER_LOGD("");

	m_total_count = 0;
	m_current_index = 0;
	_set_count(0, 0);
	elm_entry_entry_set(m_entry, "");
}

void find_on_page::unset_focus(void)
{
	elm_object_focus_set(m_entry, EINA_FALSE);
}

void find_on_page::set_landscape_mode(Eina_Bool landscape)
{
	if (landscape) {
		elm_object_signal_emit(m_main_layout, "show,landscape-mode,signal", "");
	} else {
		elm_object_signal_emit(m_main_layout, "show,portrait-mode,signal", "");
	}
}

Evas_Object *find_on_page::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	// Find on page layout.
	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGD("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, find_on_page_edj_path, "find-on-page-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(layout, EEXT_CALLBACK_BACK, __back_clicked_cb, this);
#endif

	// Entry.
	Evas_Object *entry = elm_entry_add(layout);
	RETV_MSG_IF(!entry, NULL, "elm_entry_add failed.");

	Evas_Object *dim_area = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(layout), "elm.cross.trans");
	if (dim_area)
		edje_object_signal_callback_add(elm_layout_edje_get(layout), "mouse,down,1", "elm.cross.trans", __dim_area_clicked_cb, this);

	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_autocapital_type_set(entry, ELM_AUTOCAPITAL_TYPE_NONE);
	elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
	elm_entry_input_panel_enabled_set(entry, EINA_TRUE);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);
#if defined(HW_MORE_BACK_KEY)
	eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
#endif

	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_smart_callback_add(entry, "activated", __enter_key_cb, this);
	evas_object_smart_callback_add(entry, "preedit,changed", __entry_changed_cb, this);
	evas_object_smart_callback_add(entry, "changed", __entry_changed_cb, this);
	evas_object_smart_callback_add(entry, "language,changed", __entry_language_changed_cb, this);

	elm_object_part_text_set(entry, "elm.guide", (std::string("<font_size=22>") + BR_STRING_FIND_ON_PAGE + "</font>").c_str());

	Evas_Object *access = elm_access_object_get(elm_entry_textblock_get(entry));
	elm_access_info_set(access, ELM_ACCESS_TYPE, BR_STRING_TEXT_FIELD_T);
	elm_entry_text_style_user_push(entry, FIND_ON_PAGE_ENTRY_STYLE);

	Evas_Object *bt_clear = elm_object_part_content_get(m_entry, "elm.swallow.clear");
	elm_access_info_set(bt_clear, ELM_ACCESS_INFO, BR_STRING_CLEAR_ALL);

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	limit_filter_data.max_byte_count = 0;
	limit_filter_data.max_char_count = FIND_ON_PAGE_MAX_TEXT;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);

	elm_object_part_content_set(layout, "elm.swallow.entry", entry);

	// Keypad callbacks.
	Evas_Object *conformant = m_browser->get_browser_view()->get_conformant();
	evas_object_smart_callback_add(conformant, "virtualkeypad,state,on", __keypad_show_cb, entry);
	evas_object_smart_callback_add(conformant, "clipboard,state,on", __keypad_show_cb, entry);

	// Down button.
	Evas_Object *down_button = elm_button_add(layout);
	if (!down_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(down_button, "browser/fop/down");
	elm_access_info_set(down_button, ELM_ACCESS_INFO, BR_STRING_ACCESS_NEXT_BUT);
	evas_object_smart_callback_add(down_button, "clicked", __down_clicked_cb, this);
	elm_object_part_content_set(layout, "elm.swallow.down-button", down_button);

	// Up button.
	Evas_Object *up_button = elm_button_add(layout);
	if (!up_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(up_button, "browser/fop/up");
	elm_access_info_set(up_button, ELM_ACCESS_INFO, BR_STRING_ACCESS_PREV_BUT);
	evas_object_smart_callback_add(up_button, "clicked", __up_clicked_cb, this);
	elm_object_part_content_set(layout, "elm.swallow.up-button", up_button);

	m_entry = entry;
	m_down_button = down_button;
	m_up_button = up_button;

	return layout;
}

void find_on_page::__dim_area_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "fop  is NULL");
	find_on_page *fop = (find_on_page *)data;
	elm_object_signal_emit(fop->m_main_layout, "hide,cross,button,signal", "");
	elm_entry_entry_set(fop->m_entry, "");
}

void find_on_page::__clear_button_down_cb(void *data, Evas* evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Evas_Event_Key_Down *ev = (Evas_Event_Key_Down *)event_info;
	if (!ev)
		return;
	if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
		return;

	if ((!strcmp(ev->keyname, "Return")) || (!strcmp(ev->keyname, "KP_Enter")))
		__delete_clicked_cb(data, NULL, NULL, NULL);
}

void find_on_page::_set_count(int index, int total)
{
	BROWSER_LOGD("%d/%d", index, total);

	char text_buffer[16] = { 0, };
	int digit_count = 0;
	if (total > 999)
		digit_count = 4;
	else if (total > 99)
		digit_count = 3;
	else if (total > 9)
		digit_count = 2;
	else
		digit_count = 1;

	const char *elm_text = elm_entry_entry_get(m_entry);
	if (elm_text == NULL || strlen(elm_text) == 0) {
		// Show 0/0
		elm_object_signal_emit(m_main_layout, "digit_1,signal", "");
	} else {
		// Change count layout size.
		sprintf(text_buffer, "digit_%d,signal", digit_count);
		elm_object_signal_emit(m_main_layout, text_buffer, "");
	}

	sprintf(text_buffer, "%d/%d", index, total);
	elm_object_part_text_set(m_main_layout, "elm.text.count", text_buffer);

	if (total <= 1) {
		BROWSER_LOGD("total %d", total);
		_disable_up_button(EINA_TRUE);
		_disable_down_button(EINA_TRUE);
		return;
	}

	_disable_up_button(EINA_FALSE);
	_disable_down_button(EINA_FALSE);
}

void find_on_page::_disable_down_button(Eina_Bool disable)
{
	elm_object_disabled_set(m_down_button, disable);
}

void find_on_page::_disable_up_button(Eina_Bool disable)
{
	elm_object_disabled_set(m_up_button, disable);
}

void find_on_page::__text_found_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	find_on_page *fop = (find_on_page *)data;

	int match_count = *(int *)event_info;
	BROWSER_LOGD("match_count=%d", match_count);
	fop->m_total_count = match_count;
	if(fop->m_current_index == 0)
		fop->m_current_index = 1;

	if (match_count == -1 || match_count >= FIND_WORD_MAX_COUNT)
		fop->m_total_count = FIND_WORD_MAX_COUNT-1;
	else
		fop->m_total_count = match_count;

	if (match_count == 0)
		fop->m_current_index = 0;

	if(fop->m_current_index > fop->m_total_count)
		fop->m_current_index = fop->m_total_count;
	fop->_set_count(fop->m_current_index, fop->m_total_count);
}

void find_on_page::__entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	find_on_page *fop = (find_on_page *)data;

	RET_MSG_IF(!fop->m_webview, "fop->m_webview is NULL");

	fop->m_current_index = 1;

	const char *elm_text = elm_entry_entry_get(obj);
	char *text = elm_entry_markup_to_utf8(elm_text);

	if(!text)
		return;

	BROWSER_LOGD("text=[%s]", text);

	if (elm_text && strlen(elm_text))
		elm_object_signal_emit(fop->m_main_layout, "show,cross,button,signal", "");
	else
		elm_object_signal_emit(fop->m_main_layout, "hide,cross,button,signal", "");

	if (strlen(text) >= FIND_ON_PAGE_MAX_TEXT) {

		m_browser->get_browser_view()->show_notification(text, NULL, FIND_ON_PAGE_MAX_TEXT);

		char buf[FIND_ON_PAGE_MAX_TEXT + 1] = {0, };
		snprintf(buf, sizeof(buf) - 1, "%s", text);

		fop->set_text(buf);

		elm_entry_cursor_end_set(obj);
		free(text);
		return;
	}

	std::string input_uri_str;

	input_uri_str = std::string(text);
	unsigned int pos = input_uri_str.find("<preedit>");
	if (pos != std::string::npos)
		input_uri_str.erase(pos, strlen("<preedit>"));
	pos = input_uri_str.find("</preedit>");
	if (pos != std::string::npos)
		input_uri_str.erase(pos, strlen("</preedit>"));

	free(text);


	if ((!fop->m_input_word) || (input_uri_str.c_str() && strcmp(fop->m_input_word, input_uri_str.c_str()))) {
		eina_stringshare_replace(&fop->m_input_word, input_uri_str.c_str());
		fop->m_webview->find_word(input_uri_str.c_str(), EINA_TRUE, __text_found_cb, data);
	}
}

void find_on_page::__enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	// Unfocus the entry to hide IME when Done key is pressed.
	find_on_page *fop = (find_on_page *)data;
	if (!m_browser->is_keyboard_active())
		elm_object_focus_set(fop->m_entry, EINA_FALSE);
}

void find_on_page::__entry_language_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	// Update guide text for the language.
	find_on_page *fop = (find_on_page *)data;
	elm_object_part_text_set(fop->m_entry, "elm.guide", (std::string("<font_size=22>") + BR_STRING_URL_GUIDE_TEXT + "</font>").c_str());
}

void find_on_page::__delete_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	Evas_Object *entry = (Evas_Object *)data;
	elm_entry_entry_set(entry, "");
	elm_object_focus_set(entry, EINA_TRUE);
}

#if defined(HW_MORE_BACK_KEY)
void find_on_page::__back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	m_browser->get_browser_view()->show_url_bar();
}
#endif

void find_on_page::__down_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	find_on_page *fop = (find_on_page *)data;
	RET_MSG_IF(!fop->m_webview, "fop->m_webview is NULL");

	if (fop->m_total_count == 0)
		return;

	fop->m_current_index++;
	if (fop->m_current_index > fop->m_total_count)
		fop->m_current_index = 1;
	fop->m_webview->find_word(fop->m_input_word, EINA_TRUE, __text_found_cb, data);

	elm_object_focus_set(obj, EINA_TRUE);
}

void find_on_page::__up_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	find_on_page *fop = (find_on_page *)data;
	RET_MSG_IF(!fop->m_webview, "fop->m_webview is NULL");

	if (fop->m_total_count == 0)
		return;

	fop->m_current_index--;
	if (fop->m_current_index < 1)
		fop->m_current_index = fop->m_total_count;
	fop->m_webview->find_word(fop->m_input_word, EINA_FALSE, __text_found_cb, data);

	elm_object_focus_set(obj, EINA_TRUE);
}

void find_on_page::__font_size_changed_cb(keynode_t *keynode, void *data)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	find_on_page *fop = (find_on_page *)data;

	std::string font_size_tag = "DEFAULT='" + m_browser->get_browser_view()->get_font_size_tag() +"'";
	elm_entry_text_style_user_push(fop->m_entry, font_size_tag.c_str());
}
