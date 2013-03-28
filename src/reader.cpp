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

#include "reader.h"

#include <app.h>
#include <string>
#include <vector>

#include "add-tag-view.h"
#include "browser.h"
#include "browser-dlog.h"
#include "browser-object.h"
#include "browser-string.h"
#include "browser-view.h"
#include "common-view.h"
#include "preference.h"
#include "scrap.h"
#include "uri-bar.h"
#include "webview.h"

#define reader_edj_path browser_edj_dir"/reader.edj"
#define reader_js_path browser_res_dir"/js/reader.js"
#define recognizearticle_js_path browser_res_dir"/js/recognizearticle.js"

#define PRINT_PDF_W	210
#define PRINT_PDF_H	297

#define pdf_file_path	browser_data_dir"/print.pdf"
#define print_pkg_name	"org.tizen.mobileprint"
#define print_service_name	"http://tizen.org/appcontrol/operation/PRINT"
#define print_files_type	"service_print_files_type"

static void _context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	evas_object_del(obj);
}

reader::reader(void)
:
	m_webview(NULL)
	,m_reader_html(NULL)
	,m_main_layout(NULL)
	,m_scrap_tag(NULL)
	,m_small_font_button(NULL)
	,m_large_font_button(NULL)
{
	BROWSER_LOGD("");
	elm_theme_extension_add(NULL, reader_edj_path);
}

reader::~reader(void)
{
	BROWSER_LOGD("");
	eina_stringshare_del(m_reader_html);

	if (m_scrap_tag)
		free(m_scrap_tag);

	if (m_webview)
		delete m_webview;
	if (m_main_layout)
		evas_object_del(m_main_layout);

	elm_theme_extension_del(NULL, reader_edj_path);
}

Eina_Bool reader::execute_reader_js(void)
{
	BROWSER_LOGD("");
	const char *uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
	if (!uri || !strlen(uri))
		return EINA_FALSE;

	FILE *file = 0;
	long size = 0;
	file = fopen(reader_js_path, "r");
	if (file) {
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		rewind(file);
		if (!size) {
			BROWSER_LOGD("ftell size is 0. unable to create with the size");
			fclose(file);
			return EINA_FALSE;
		}

		size++;

		char *reader_script = (char *)malloc(sizeof(char) * size);
		if (!reader_script) {
			BROWSER_LOGD("Failed to get memory for reader texts");
			fclose(file);
			return EINA_FALSE;
		}
		memset(reader_script, 0x00, sizeof(char) * size);
		size_t result = fread(reader_script, 1, size - 1, file);

		if (result != (size_t)(size - 1)) {
			BROWSER_LOGE("Reading error, result[%d] and reader_script[%d]", result, size - 1);
			free (reader_script);
			reader_script = NULL;
			return EINA_FALSE;
		}

		BROWSER_LOGD("size=[%ld]", size);

		m_browser->get_browser_view()->get_current_webview()->execute_script(reader_script, __execute_reader_js_cb, this);

		fclose(file);
		if (reader_script)
			free(reader_script);

		return EINA_TRUE;
	} else {
		BROWSER_LOGE("fopen failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool reader::execute_recognizearticle_js(void)
{
	BROWSER_LOGD("");
	const char *uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
	if (!uri || !strlen(uri))
		return EINA_FALSE;

	FILE *file = 0;
	long size = 0;
	file = fopen(recognizearticle_js_path, "r");
	if (file) {
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		rewind(file);
		if (!size) {
			BROWSER_LOGD("ftell size is 0. unable to create with the size");
			fclose(file);
			return EINA_FALSE;
		}

		size++;

		char *recognizearticle_script = (char *)malloc(sizeof(char) * size);
		if (!recognizearticle_script) {
			BROWSER_LOGD("Failed to get memory for recognizearticle texts");
			fclose(file);
			return EINA_FALSE;
		}
		memset(recognizearticle_script, 0x00, sizeof(char) * size);
		size_t result = fread(recognizearticle_script, 1, size - 1, file);

		if (result != (size_t)(size - 1)) {
			BROWSER_LOGE("Reading error, result[%d] and recognizearticle_script[%d]", result, size - 1);
			free (recognizearticle_script);
			recognizearticle_script = NULL;
			fclose(file);
			return EINA_FALSE;
		}

		BROWSER_LOGD("size=[%ld]", size);

		m_browser->get_browser_view()->get_current_webview()->execute_script(recognizearticle_script, __execute_recognizearticle_js_cb, this);

		fclose(file);
		if (recognizearticle_script)
			free(recognizearticle_script);

		return EINA_TRUE;
	} else {
		BROWSER_LOGE("fopen failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

// When user clicks a link in reader view.
void reader::__load_commited_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_browser_view()->show_reader(EINA_FALSE);
	reader *rd = (reader *)data;
	const char *uri = rd->m_webview->get_uri();
	BROWSER_LOGD("uri=[%s]", uri);
	m_browser->get_browser_view()->get_current_webview()->load_uri(uri);
}

void reader::__load_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	evas_object_smart_callback_add(obj, "load,committed", __load_commited_cb, data);
}

void reader::__ime_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	reader *rd = (reader *)data;
	rd->show_toolbar(EINA_FALSE);
}

void reader::__ime_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	reader *rd = (reader *)data;
	if (!m_browser->get_browser_view()->is_show_find_on_page())
		rd->show_toolbar(EINA_TRUE);
}

void reader::show_toolbar(Eina_Bool show)
{
	if (show)
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,toolbar,signal", "");
	else
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,toolbar,signal", "");
}

Evas_Object *reader::get_layout(void)
{
	if (m_main_layout)
		return m_main_layout;

	m_main_layout = _create_main_layout(m_window);

	m_webview = new webview();
	Evas_Object *ewk_view = m_webview->get_ewk_view();

	m_webview->enable_customize_contextmenu(EINA_TRUE);

	evas_object_smart_callback_add(ewk_view, "load,finished", __load_finished_cb, this);

	evas_object_size_hint_weight_set(ewk_view, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_webview->font_default_size_set(m_preference->get_reader_font_size());
	m_webview->html_contents_set(m_reader_html, m_browser->get_browser_view()->get_current_webview()->get_uri());

	elm_object_part_content_set(m_main_layout, "elm.swallow.webview", ewk_view);

	evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,on", __ime_show_cb, this);
	evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,off", __ime_hide_cb, this);

	evas_object_smart_callback_add(m_conformant, "clipboard,state,on", __ime_show_cb, this);
	evas_object_smart_callback_add(m_conformant, "clipboard,state,off", __ime_hide_cb, this);

	return m_main_layout;
}

void reader::delete_layout(void)
{
	evas_object_smart_callback_del(m_conformant, "virtualkeypad,state,on", __ime_show_cb);
	evas_object_smart_callback_del(m_conformant, "virtualkeypad,state,off", __ime_hide_cb);

	evas_object_smart_callback_del(m_conformant, "clipboard,state,on", __ime_show_cb);
	evas_object_smart_callback_del(m_conformant, "clipboard,state,off", __ime_hide_cb);

	if (m_main_layout) {
		elm_object_part_content_unset(m_main_layout, "elm.swallow.webview");
		evas_object_del(m_main_layout);
		m_main_layout = NULL;
	}

	if (m_webview) {
		delete m_webview;
		m_webview = NULL;
	}
}

void reader::__execute_reader_js_cb(Evas_Object *obj, const char *javascript_result, void *data)
{
	BROWSER_LOGD("javascript_result=[%s]", javascript_result);

	reader *rd = (reader *)data;

	if (!m_browser->get_browser_view()->get_current_webview() || !javascript_result)
		return;

	if (javascript_result && strncmp(javascript_result, "undefined", strlen("undefined"))) {
		if (!rd->m_reader_html)
			rd->m_reader_html = eina_stringshare_add(javascript_result);
		else
			eina_stringshare_replace(&rd->m_reader_html, javascript_result);
		m_browser->get_browser_view()->get_uri_bar()->show_reader_icon(EINA_TRUE);
		m_browser->get_browser_view()->get_current_webview()->reader_enabled_set(EINA_TRUE);
	} else {
		eina_stringshare_del(rd->m_reader_html);
		m_browser->get_browser_view()->get_uri_bar()->show_reader_icon(EINA_FALSE);
		m_browser->get_browser_view()->get_current_webview()->reader_enabled_set(EINA_FALSE);
	}
}

void reader::__execute_recognizearticle_js_cb(Evas_Object *obj, const char *javascript_result, void *data)
{
	BROWSER_LOGD("javascript_result=[%s]", javascript_result);

	reader *rd = (reader *)data;

	if (!m_browser->get_browser_view()->get_current_webview() || !javascript_result)
		return;

	if (javascript_result && strncmp(javascript_result, "false", strlen("false"))) {
		rd->execute_reader_js();
	} else {
		eina_stringshare_del(rd->m_reader_html);
		m_browser->get_browser_view()->get_uri_bar()->show_reader_icon(EINA_FALSE);
		m_browser->get_browser_view()->get_current_webview()->reader_enabled_set(EINA_FALSE);
	}
}

Evas_Object *reader::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, reader_edj_path, "reader-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *toolbar = _create_toolbar_layout(layout);
	elm_object_part_content_set(layout, "elm.swallow.toolbar", toolbar);

	return layout;
}

Evas_Object *reader::_create_toolbar_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, reader_edj_path, "toolbar-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *more_button = elm_button_add(layout);
	if (!more_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(more_button, "browser/toolbar_menu");
	elm_object_part_content_set(layout, "elm.swallow.more_button", more_button);
	evas_object_smart_callback_add(more_button, "clicked", __more_cb, this);

	m_small_font_button = elm_button_add(layout);
	if (!m_small_font_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_small_font_button, "browser/small_font");
	elm_object_part_content_set(layout, "elm.swallow.small_font_button", m_small_font_button);
	evas_object_smart_callback_add(m_small_font_button, "clicked", __small_font_cb, this);

	m_large_font_button = elm_button_add(layout);
	if (!m_large_font_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_large_font_button, "browser/large_font");
	elm_object_part_content_set(layout, "elm.swallow.large_font_button", m_large_font_button);
	evas_object_smart_callback_add(m_large_font_button, "clicked", __large_font_cb, this);

	Evas_Object *back_button = elm_button_add(layout);
	if (!back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(back_button, "naviframe/back_btn/default");
	elm_object_part_content_set(layout, "elm.swallow.back_button", back_button);
	evas_object_smart_callback_add(back_button, "clicked", __back_cb, this);

	return layout;
}

void reader::__small_font_cb(void *data, Evas_Object *obj, void *event_info)
{
	reader *rd = (reader *)data;
	int font_size = m_preference->get_reader_font_size();
	if (font_size > 9) {
		m_preference->set_reader_font_size(font_size - 1);
		rd->m_webview->font_default_size_set(font_size - 1);
		elm_object_disabled_set(rd->m_large_font_button, EINA_FALSE);
	} else
		elm_object_disabled_set(rd->m_small_font_button, EINA_TRUE);
}

void reader::__large_font_cb(void *data, Evas_Object *obj, void *event_info)
{
	reader *rd = (reader *)data;
	int font_size = m_preference->get_reader_font_size();
	if (font_size < 30) {
		m_preference->set_reader_font_size(font_size + 1);
		rd->m_webview->font_default_size_set(font_size + 1);
		elm_object_disabled_set(rd->m_small_font_button, EINA_FALSE);
	} else
		elm_object_disabled_set(rd->m_large_font_button, EINA_TRUE);
}

void reader::__print_cb(void *data, Evas_Object *obj, void *event_info)
{
	reader *rd = (reader *)data;

	rd->m_webview->save_as_pdf(PRINT_PDF_W, PRINT_PDF_H, pdf_file_path);

	service_h service;

	service_create(&service);
	service_set_operation(service, print_service_name);
	service_set_package(service, print_pkg_name);

	service_set_uri(service, pdf_file_path);
	service_add_extra_data(service, print_files_type, "WEB");
	service_send_launch_request(service, NULL, NULL);
	service_destroy(service);

	evas_object_del(obj);
}

void reader::__mht_contents_get_cb(Evas_Object *ewk_view, const char *data, void *user_data)
{
	EINA_SAFETY_ON_NULL_RETURN(data);
	reader *rd = (reader *)user_data;

	scrap scrap_instance;

	BROWSER_LOGD("rd->m_webview->get_title()=[%s]", rd->m_webview->get_title());
	BROWSER_LOGD("rd->m_webview->get_uri()=[%s]", rd->m_webview->get_uri());

	const char *title = rd->m_webview->get_title();
	char *file_path = NULL;
	if (title && strlen(title))
		file_path = scrap_instance.save(rd->m_webview->get_title(), rd->m_webview->get_uri(), data, rd->m_scrap_tag);
	else
		file_path = scrap_instance.save(rd->m_webview->get_uri(), rd->m_webview->get_uri(), data, rd->m_scrap_tag);

	if (file_path)
		free(file_path);

	if (rd->m_scrap_tag) {
		free(rd->m_scrap_tag);
		rd->m_scrap_tag = NULL;
	}

	m_browser->get_browser_view()->show_noti_popup(BR_STRING_SAVED);
}

void reader::__add_scrap_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	reader *rd = (reader *)data;
	std::vector<char *> tag_list = *((std::vector<char *> *)event_info);

	std::string tag_str;
	if (tag_list.size() > 0)
		tag_str = tag_list[0];

	for (int i = 1 ; i < tag_list.size() ; i++) {
		tag_str = tag_str + std::string(",") + std::string(tag_list[i]);
		BROWSER_LOGD("[%d]=[%s]", i, tag_list[i]);
	}

	if (rd->m_scrap_tag) {
		free(rd->m_scrap_tag);
		rd->m_scrap_tag = NULL;
	}

	if (tag_list.size() > 0)
		rd->m_scrap_tag = strdup(tag_str.c_str());

	rd->m_webview->mht_contents_get(__mht_contents_get_cb, rd);

	for (int i = 1 ; i < tag_list.size() ; i++)
		delete tag_list[i];

	m_browser->get_browser_view()->show_noti_popup(BR_STRING_SAVED);
}

void reader::__add_to_scrap_cb(void *data, Evas_Object *obj, void *event_info)
{
	reader *rd = (reader *)data;
#if defined(BROWSER_TAG)
	m_browser->get_add_tag_view(__add_scrap_done_cb, rd, NULL)->show();
#else
	rd->m_webview->mht_contents_get(__mht_contents_get_cb, rd);
#endif

	evas_object_del(obj);
}

void reader::_show_more_context_popup(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(parent);

	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", _context_popup_dismissed_cb, NULL);

	elm_ctxpopup_item_append(more_popup, BR_STRING_ADD_TO_SCRAPBOOK, NULL, __add_to_scrap_cb, this);
	elm_ctxpopup_item_append(more_popup, BR_STRING_PRINT, NULL, __print_cb, this);

	int x, y, w, h;
	evas_object_geometry_get(parent, &x, &y, &w, &h);
	evas_object_move(more_popup, x + (w / 2), y + (h /2));
	evas_object_show(more_popup);
}

void reader::__more_cb(void *data, Evas_Object *obj, void *event_info)
{
	reader *rd = (reader *)data;
	rd->_show_more_context_popup(obj);
}

void reader::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	m_browser->get_browser_view()->show_reader(EINA_FALSE);
}

