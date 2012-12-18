/*
 *  browser
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#define text_min_size 2
#define text_max_size 30

#include "browser-context-menu.h"
#include "browser-data-manager.h"
#include "browser-reader.h"
#include "browser-view.h"
#include "browser-window.h"

Browser_Reader::Browser_Reader(string reader_html, string base_url)
:	m_main_layout(NULL)
	,m_ewk_view(NULL)
	,m_parent_ewk_view(NULL)
	,m_text_small_button(NULL)
	,m_text_big_button(NULL)
	,m_cancel_button(NULL)
	,m_waiting_progress(NULL)
	,m_context_menu(NULL)
	,m_first_load(EINA_FALSE)
{
	BROWSER_LOGD("[%s]", __func__);
	m_reader_html = reader_html;
	m_reader_base_url = base_url;
}

Browser_Reader::~Browser_Reader(void)
{
	BROWSER_LOGD("[%s]", __func__);

	deinit();

	if (m_main_layout)
		evas_object_del(m_main_layout);
	if (m_ewk_view)
		evas_object_del(m_ewk_view);
	if (m_waiting_progress)
		evas_object_del(m_waiting_progress);
	if (m_parent_ewk_view)
		m_parent_ewk_view = NULL;
}

Eina_Bool Browser_Reader::init(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	m_parent_ewk_view = ewk_view;
	m_ewk_view = ewk_view_add(evas_object_evas_get(m_win));
	if (!m_ewk_view) {
		BROWSER_LOGE("ewk_view_add failed");
		return EINA_FALSE;
	}
	evas_object_color_set(m_ewk_view, 255, 255, 255, 255);
	evas_object_size_hint_weight_set(m_ewk_view, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_ewk_view, EVAS_HINT_FILL, EVAS_HINT_FILL);
#if 0
	evas_object_smart_callback_add(m_ewk_view, "load,committed",
					__did_commit_load_for_frame_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "load,nonemptylayout,finished",
					__did_commit_load_for_frame_cb, this);
#endif
	evas_object_smart_callback_add(m_ewk_view, "load,finished",
					__did_finish_progress_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "create,window",
					__create_new_window_cb, this);

	int font_size = BROWSER_READER_DEFAULT_FONT_SIZE;
	br_preference_get_int(READER_FONT_SIZE_KEY, &font_size);

	Ewk_Settings *setting = ewk_view_settings_get(m_ewk_view);
	ewk_settings_font_default_size_set(setting, font_size);

	if (!_create_main_layout()) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	Browser_View *browser_view = m_data_manager->get_browser_view();
	m_context_menu = browser_view->get_context_menu();
	if (!m_context_menu)
		BROWSER_LOGE("Unable to use context popup in reader view");

	m_context_menu->init(m_ewk_view);

	return EINA_TRUE;
}

Eina_Bool Browser_Reader::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);

	evas_object_smart_callback_del(m_ewk_view, "load,finished", __did_finish_progress_cb);
	evas_object_smart_callback_del(m_ewk_view, "create,window", __create_new_window_cb);

	/* Restore context popup of parent view */
	m_context_menu->init(m_parent_ewk_view);

	return EINA_TRUE;
}

void Browser_Reader::__create_new_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	*((Evas_Object **)event_info) = m_data_manager->get_browser_view()->get_focused_window()->m_ewk_view;
}

void Browser_Reader::__did_finish_progress_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Reader *reader = (Browser_Reader *)data;

	if (!reader->m_first_load)
		reader->m_first_load = EINA_TRUE;

	elm_object_part_content_unset(reader->m_main_layout, "elm.swallow.waiting_progress");
	if (reader->m_waiting_progress) {
		evas_object_del(reader->m_waiting_progress);
		reader->m_waiting_progress = NULL;
	}
}

void Browser_Reader::__did_commit_load_for_frame_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_Reader *reader = (Browser_Reader *)data;
	if (!reader->m_first_load)
		return;

	const char *uri = ewk_view_url_get(reader->m_ewk_view);
	m_data_manager->get_browser_view()->load_url(uri);

	if (!m_data_manager->get_browser_view()->show_article_reader(EINA_FALSE))
		BROWSER_LOGE("show_article_reader failed");
}

void Browser_Reader::__did_first_visually_non_empty_layout_for_frame_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Reader *browser_reader = (Browser_Reader *)data;

	elm_object_part_content_unset(browser_reader->m_main_layout, "elm.swallow.waiting_progress");
	if (browser_reader->m_waiting_progress) {
		evas_object_del(browser_reader->m_waiting_progress);
		browser_reader->m_waiting_progress = NULL;
	}
}

void Browser_Reader::__cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!m_data_manager->get_browser_view()->show_article_reader(EINA_FALSE))
		BROWSER_LOGE("show_article_reader failed");
}

void Browser_Reader::__text_smaller_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Browser_Reader *browser_reader = (Browser_Reader *)data;

	Ewk_Settings *setting = ewk_view_settings_get(browser_reader->m_ewk_view);
	int size = ewk_settings_font_default_size_get(setting);
	if (size > text_min_size)
		ewk_settings_font_default_size_set(setting, size - 1);
}

void Browser_Reader::__text_bigger_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Browser_Reader *browser_reader = (Browser_Reader *)data;

	Ewk_Settings *setting = ewk_view_settings_get(browser_reader->m_ewk_view);
	int size = ewk_settings_font_default_size_get(setting);
	if (size < text_max_size)
		ewk_settings_font_default_size_set(setting, size + 1);
}

Eina_Bool Browser_Reader::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_main_layout = elm_layout_add(m_navi_bar);
	if (!m_main_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}

	if (!elm_layout_file_set(m_main_layout, BROWSER_EDJE_DIR"/browser-reader.edj",
							"browser-reader/main_layout")) {
		BROWSER_LOGE("elm_layout_file_set failed", BROWSER_EDJE_DIR);
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_main_layout);

	/* Strangely, the event is passed via reader view to browser view without this code
	  * even set the event as false in edc. */
	evas_object_propagate_events_set(m_main_layout, EINA_FALSE);

	m_waiting_progress = elm_progressbar_add(m_navi_bar);
	if (!m_waiting_progress) {
		BROWSER_LOGE("elm_progressbar_add failed!");
		return EINA_FALSE;
	}

	elm_object_style_set(m_waiting_progress, "browser/loading_wheel");
	elm_progressbar_pulse(m_waiting_progress, EINA_TRUE);
	evas_object_size_hint_weight_set(m_waiting_progress, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_waiting_progress, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(m_main_layout, "elm.swallow.waiting_progress", m_waiting_progress);
	evas_object_show(m_waiting_progress);

	m_text_small_button = elm_button_add(m_main_layout);
	if (!m_text_small_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_text_small_button, "browser/close_button");
	elm_object_part_content_set(m_main_layout, "elm.swallow.text_small_button", m_text_small_button);
	evas_object_smart_callback_add(m_text_small_button, "clicked", __text_smaller_clicked_cb, this);
	evas_object_show(m_text_small_button);

	m_text_big_button = elm_button_add(m_main_layout);
	if (!m_text_big_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_text_big_button, "browser/close_button");
	elm_object_part_content_set(m_main_layout, "elm.swallow.text_big_button", m_text_big_button);
	evas_object_smart_callback_add(m_text_big_button, "clicked", __text_bigger_clicked_cb, this);
	evas_object_show(m_text_big_button);

	m_cancel_button = elm_button_add(m_main_layout);
	if (!m_cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_cancel_button, "browser/close_button");
	elm_object_part_content_set(m_main_layout, "elm.swallow.cancel_button", m_cancel_button);
	evas_object_smart_callback_add(m_cancel_button, "clicked", __cancel_button_clicked_cb, this);
	evas_object_show(m_cancel_button);

	elm_object_part_content_set(m_main_layout, "elm.swallow.reader_view", m_ewk_view);
	evas_object_show(m_ewk_view);

	ewk_view_html_contents_set(m_ewk_view, m_reader_html.c_str(), m_reader_base_url.c_str());

	return EINA_TRUE;
}

Evas_Object *Browser_Reader::get_main_layout(void)
{
	return m_main_layout;
}


