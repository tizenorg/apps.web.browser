/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#include "browser-find-word.h"

Browser_Find_Word::Browser_Find_Word(Browser_View *browser_view)
:	m_find_word_index(0)
	,m_find_word_max_count(0)
	,m_webview(NULL)
	,m_find_word_entry_layout(NULL)
	,m_find_word_edit_field(NULL)
	,m_find_word_cancel_button(NULL)
	,m_find_word_prev_button(NULL)
	,m_find_word_next_button(NULL)
	,m_option_header_find_word_layout(NULL)
	,m_browser_view(browser_view)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!_create_layout())
		BROWSER_LOGE("_create_layout failed");
}

Browser_Find_Word::~Browser_Find_Word(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_find_word_entry_layout)
		evas_object_del(m_find_word_entry_layout);
	if (m_find_word_edit_field)
		evas_object_del(m_find_word_edit_field);
	if (m_find_word_cancel_button)
		evas_object_del(m_find_word_cancel_button);
	if (m_find_word_prev_button)
		evas_object_del(m_find_word_prev_button);
	if (m_find_word_next_button)
		evas_object_del(m_find_word_next_button);
	if (m_option_header_find_word_layout)
		evas_object_del(m_option_header_find_word_layout);
}

void Browser_Find_Word::init(Evas_Object *webview)
{
	BROWSER_LOGD("[%s]", __func__);
	m_webview = webview;
	Evas_Object *find_word_edit_field_entry = br_elm_editfield_entry_get(m_find_word_edit_field);
	elm_entry_entry_set(find_word_edit_field_entry, "");

	elm_object_focus_set(m_find_word_edit_field, EINA_TRUE);

	edje_object_part_text_set(elm_layout_edje_get(m_option_header_find_word_layout), "elm.index_text", "0/0");
}

void Browser_Find_Word::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	ewk_view_text_matches_unmark_all(webkit);
}

void Browser_Find_Word::find_word(const char *word, Find_Word_Direction direction)
{
	BROWSER_LOGD("word to find=[%s]", word);

	Evas_Object *webkit = elm_webview_webkit_get(m_webview);

	ewk_view_text_matches_unmark_all(webkit);
	m_find_word_max_count = ewk_view_text_matches_mark(webkit, word,
							EINA_FALSE, EINA_TRUE, 0);

	if (m_prev_searched_word.empty() || strcmp(m_prev_searched_word.c_str(), word))
		m_find_word_index = 0;

	m_prev_searched_word = std::string(word);

	if (direction == BROWSER_FIND_WORD_FORWARD) {
		if (ewk_view_text_search(elm_webview_webkit_get(m_webview),
					word, EINA_FALSE, EINA_TRUE, EINA_TRUE)) {
			m_find_word_index++;
			if (m_find_word_index > m_find_word_max_count)
				m_find_word_index = 1;
		}
	} else {
		if (ewk_view_text_search(elm_webview_webkit_get(m_webview),
					word, EINA_FALSE, EINA_FALSE, EINA_TRUE)) {
			m_find_word_index--;
			if (m_find_word_index <= 0)
				m_find_word_index = m_find_word_max_count;
		}
	}

	char index_text[100] = {0, };
	if (m_find_word_max_count < 100)
		sprintf(index_text, "%d/%d", m_find_word_index, m_find_word_max_count);
	else
		sprintf(index_text, "%d/-", m_find_word_index);

	_update_find_word_index_text(index_text);
}

void Browser_Find_Word::_update_find_word_index_text(const char *index_text)
{
	BROWSER_LOGD("[%s]", __func__);

	edje_object_part_text_set(elm_layout_edje_get(m_option_header_find_word_layout), "elm.index_text", index_text);
}

void Browser_Find_Word::__find_word_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Find_Word *find_word = (Browser_Find_Word *)data;
	Evas_Object *edit_field_entry = br_elm_editfield_entry_get(find_word->m_find_word_edit_field);
	const char *word_to_find = elm_entry_entry_get(edit_field_entry);
	if (!word_to_find || !strlen(word_to_find))
		return;

	find_word->find_word(word_to_find, Browser_Find_Word::BROWSER_FIND_WORD_FORWARD);

	elm_object_focus_set(edit_field_entry, EINA_FALSE);
	BROWSER_LOGD("find_word=[%s]", word_to_find);
}

Eina_Bool Browser_Find_Word::_create_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_option_header_find_word_layout = elm_layout_add(m_browser_view->m_navi_bar);
	if (!m_option_header_find_word_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_option_header_find_word_layout, BROWSER_EDJE_DIR"/browser-view-find-word-layout.edj",
		"browser-view/find_word_layout")) {
		BROWSER_LOGE("Can not set layout theme[browser-view/find_word_layout]\n");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_option_header_find_word_layout, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(m_option_header_find_word_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_option_header_find_word_layout);

	/* create url entry layout in url layout */
	m_find_word_entry_layout = elm_layout_add(m_browser_view->m_navi_bar);
	if (!m_find_word_entry_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_find_word_entry_layout, BROWSER_EDJE_DIR"/browser-view-find-word-layout.edj",
				"browser-view/find_word_editfield_layout")) {
		BROWSER_LOGE("browser-view/find_word_editfield_layout failed");
		return EINA_FALSE;
	}
	elm_object_part_content_set(m_option_header_find_word_layout, "elm.swallow.url", m_find_word_entry_layout);
	evas_object_show(m_find_word_entry_layout);

	edje_object_part_text_set(elm_layout_edje_get(m_option_header_find_word_layout), "title_text", BR_STRING_FIND_WORD);

	m_find_word_edit_field = br_elm_url_editfield_add(m_browser_view->m_navi_bar);
	if (!m_find_word_edit_field) {
		BROWSER_LOGE("elm_editfield_add failed");
		return EINA_FALSE;
	}

	elm_object_part_content_set(m_find_word_entry_layout, "elm.swallow.entry", m_find_word_edit_field);
	evas_object_show(m_find_word_edit_field);

	edje_object_signal_emit(elm_layout_edje_get(m_find_word_edit_field), "find_word,signal", "elm");

	br_elm_editfield_entry_single_line_set(m_find_word_edit_field, EINA_TRUE);
	br_elm_editfield_eraser_set(m_find_word_edit_field, EINA_TRUE);

	elm_object_signal_callback_add(m_find_word_edit_field, "elm,eraser,clicked", "elm",
						__find_word_erase_button_clicked_cb, this);

	Evas_Object *find_word_edit_field_entry = br_elm_editfield_entry_get(m_find_word_edit_field);
	elm_entry_entry_set(find_word_edit_field_entry, "");
	evas_object_smart_callback_add(find_word_edit_field_entry, "activated", __find_word_entry_enter_key_cb, this);
	elm_entry_input_panel_layout_set(find_word_edit_field_entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	ecore_imf_context_input_panel_event_callback_add((Ecore_IMF_Context *)elm_entry_imf_context_get(find_word_edit_field_entry),
			ECORE_IMF_INPUT_PANEL_STATE_EVENT, __find_word_entry_imf_event_cb, this);
	evas_object_show(m_find_word_edit_field);

	m_find_word_cancel_button = elm_button_add(m_browser_view->m_navi_bar);
	if (!m_find_word_cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_find_word_cancel_button, "text_only/style2");
	elm_object_text_set(m_find_word_cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(m_option_header_find_word_layout, "elm.swallow.cancel", m_find_word_cancel_button);
	evas_object_show(m_find_word_cancel_button);

	evas_object_smart_callback_add(m_find_word_cancel_button, "clicked", __find_word_cancel_button_clicked_cb, this);

	m_find_word_prev_button = elm_button_add(m_browser_view->m_navi_bar);
	if (!m_find_word_prev_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_find_word_prev_button, "browser/find_word_prev");
	elm_object_part_content_set(m_option_header_find_word_layout, "elm.swallow.find_word_prev", m_find_word_prev_button);
	evas_object_show(m_find_word_prev_button);
	evas_object_smart_callback_add(m_find_word_prev_button, "clicked", __find_word_prev_button_clicked_cb, this);

	m_find_word_next_button = elm_button_add(m_browser_view->m_navi_bar);
	if (!m_find_word_next_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_find_word_next_button, "browser/find_word_next");
	elm_object_part_content_set(m_option_header_find_word_layout, "elm.swallow.find_word_next", m_find_word_next_button);
	evas_object_show(m_find_word_next_button);
	evas_object_smart_callback_add(m_find_word_next_button, "clicked", __find_word_next_button_clicked_cb, this);

	const char *current_theme = elm_theme_get(NULL);
	if (current_theme && strstr(current_theme, "white")) {
	} else {
		edje_object_signal_emit(elm_layout_edje_get(m_find_word_entry_layout),
									"black_theme,signal", "");
	}

	return EINA_TRUE;
}

void Browser_Find_Word::__find_word_cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Find_Word *find_word = (Browser_Find_Word *)data;
	find_word->m_browser_view->_set_edit_mode(BR_NO_EDIT_MODE);
}

void Browser_Find_Word::__find_word_prev_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Find_Word *find_word = (Browser_Find_Word *)data;
	Evas_Object *edit_field_entry = br_elm_editfield_entry_get(find_word->m_find_word_edit_field);
	const char *word_to_find = elm_entry_entry_get(edit_field_entry);
	if (!word_to_find || !strlen(word_to_find))
		return;

	find_word->find_word(word_to_find, Browser_Find_Word::BROWSER_FIND_WORD_BACKWARD);
}

void Browser_Find_Word::__find_word_next_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Find_Word *find_word = (Browser_Find_Word *)data;
	Evas_Object *edit_field_entry = br_elm_editfield_entry_get(find_word->m_find_word_edit_field);
	const char *word_to_find = elm_entry_entry_get(edit_field_entry);
	if (!word_to_find || !strlen(word_to_find))
		return;

	find_word->find_word(word_to_find, Browser_Find_Word::BROWSER_FIND_WORD_FORWARD);
}

void Browser_Find_Word::__find_word_entry_imf_event_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	BROWSER_LOGD("value=%d", value);
	if (!data)
		return;

	Browser_Find_Word *find_word = (Browser_Find_Word *)data;
	if (value == ECORE_IMF_INPUT_PANEL_STATE_HIDE) {
		Evas_Object *find_word_editfield_entry = br_elm_editfield_entry_get(find_word->m_find_word_edit_field);
		elm_object_focus_set(find_word_editfield_entry, EINA_FALSE);
	}
}

void Browser_Find_Word::__find_word_erase_button_clicked_cb(void *data, Evas_Object *obj,
								const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Find_Word *find_word = (Browser_Find_Word *)data;
	edje_object_part_text_set(elm_layout_edje_get(find_word->m_option_header_find_word_layout), "elm.index_text", "0/0");
	Evas_Object *webkit = elm_webview_webkit_get(find_word->m_webview);
	ewk_view_text_matches_unmark_all(webkit);
}
