/*
 * Copyright 2014  Samsung Electronics Co., Ltd
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
 * Contact: Mahesh Domakuntla <mahesh.d@samsung.com>
 *
 */

#include "tab-view-item-lite.h"

#include <Elementary.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "platform-service.h"
#include "webview.h"
#include "browser-view.h"

#define tab_manager_view_edj_path browser_edj_dir"/tab-manager-view-lite.edj"

tab_view_item_lite::tab_view_item_lite(webview *wv, Evas_Smart_Cb delete_cb, void *data, Eina_Bool is_current_tab)
	: m_container_layout(NULL)
	, m_favicon(NULL)
	, m_layout(NULL)
	, m_is_current_tab(is_current_tab)
	, m_gengrid_item(NULL)
	, m_delete_button_pressed(false)
{
	BROWSER_SECURE_LOGD("title = [%s], uri = [%s]", wv->get_title(), wv->get_uri());

	if (wv->get_title() && strlen(wv->get_title()) > 0)
		m_title = eina_stringshare_add(wv->get_title());
	else if (wv->get_uri() && strlen(wv->get_uri()) > 0)
		m_title = eina_stringshare_add(wv->get_uri());
	else
		m_title = eina_stringshare_add(blank_page);

	m_uri = eina_stringshare_add(wv->get_uri());
	m_wv = wv;

	m_delete_cb = delete_cb;
	m_cb_data = data;

}

tab_view_item_lite::~tab_view_item_lite(void)
{
	BROWSER_LOGD("");
	if (m_title){
		eina_stringshare_del(m_title);
		m_title = NULL;
	}

	if (m_uri){
		eina_stringshare_del(m_uri);
		m_uri = NULL;
	}
}

void tab_view_item_lite::update_title(void)
{
	BROWSER_LOGD("");

	if (m_wv) {
		const char* title = m_wv->get_title();
		if (title && strlen(title) > 0)
			eina_stringshare_replace(&m_title, title);
	}
}

Evas_Object *tab_view_item_lite::get_item_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, tab_manager_view_edj_path, "tab-item");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_layout = layout;

	Evas_Object *content_layout = elm_layout_add(layout);
	elm_layout_file_set(content_layout, tab_manager_view_edj_path, "tab-layout");

	evas_object_size_hint_weight_set(content_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(content_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	if (m_browser->get_browser_view()->is_landscape())
		elm_object_signal_emit(content_layout, "landscape,signal", "");
	else
		elm_object_signal_emit(content_layout, "portrait,signal", "");

	Evas_Object *favicon = NULL;
	char *uri = NULL;
	if (m_uri) {
		uri = strdup(m_uri);
		favicon = webview_context::instance()->get_favicon(m_uri);
	}

	edje_object_part_text_set(elm_layout_edje_get(content_layout), "elm.swallow.title_text", m_title);
	edje_object_part_text_set(elm_layout_edje_get(content_layout), "elm.swallow.uri_text", uri);
	if (uri) {
		free(uri);
		uri = NULL;
	}
	if (!favicon) {
		favicon = elm_icon_add(content_layout);
		elm_image_file_set(favicon, tab_manager_view_edj_path, "I01_icon_default_internet.png");
	}
	elm_object_part_content_set(content_layout, "elm.swallow.favicon", favicon);
	Evas_Object *delete_button = elm_button_add(content_layout);
	elm_object_style_set(delete_button, "multiwindow_item/delete");
	evas_object_smart_callback_add(delete_button, "clicked", __delete_item_cb, this);
	evas_object_smart_callback_add(delete_button, "pressed", __pressed_delete_cb, this);
	evas_object_smart_callback_add(delete_button, "unpressed", __unpressed_delete_cb, this);
	elm_object_part_content_set(content_layout, "elm.swallow.delete_button", delete_button);
	evas_object_propagate_events_set(delete_button, EINA_FALSE);

	m_container_layout = content_layout;
#ifdef ENABLE_INCOGNITO_WINDOW
	set_current_tab_status(m_is_current_tab, get_webview()->is_incognito());
#else
	set_current_tab_status(m_is_current_tab);
#endif

	elm_object_part_content_set(layout, "elm.swallow.content", content_layout);
	return layout;
}

#ifdef ENABLE_INCOGNITO_WINDOW
void tab_view_item_lite::set_current_tab_status(Eina_Bool is_current_tab, Eina_Bool is_incognito)
#else
void tab_view_item_lite::set_current_tab_status(Eina_Bool is_current_tab)
#endif
{
	m_is_current_tab = is_current_tab;

	RET_MSG_IF(!m_container_layout, "m_container_layout is NULL");
	Evas_Object *eobject = elm_layout_edje_get(m_container_layout);

	if (is_current_tab) {
		edje_object_signal_emit(eobject, "selected_item,signal", "");
	} else {
#ifdef ENABLE_INCOGNITO_WINDOW
		if (is_incognito)
#else
		webview *current_webview = m_browser->get_browser_view()->get_current_webview();
		if (current_webview && current_webview->private_browsing_enabled_get())
#endif
			edje_object_signal_emit(eobject, "secret_item,signal", "");
		else
			edje_object_signal_emit(eobject, "unselected_item,signal", "");
	}
}

void tab_view_item_lite::__delete_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	tab_view_item_lite *tvi = (tab_view_item_lite*)data;

	tvi->m_delete_cb(tvi->m_cb_data, obj, tvi);
}

void tab_view_item_lite::__pressed_delete_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	BROWSER_LOGD("");

	tab_view_item_lite *tvi = static_cast<tab_view_item_lite*>(data);
	tvi->m_delete_button_pressed = true;
}

void tab_view_item_lite::__unpressed_delete_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	BROWSER_LOGD("");

	tab_view_item_lite *tvi = static_cast<tab_view_item_lite*>(data);
	tvi->m_delete_button_pressed = false;
}

void tab_view_item_lite::delete_gengrid_item(void)
{
	BROWSER_LOGD("");

	if (m_gengrid_item) {
		elm_object_item_del(m_gengrid_item);
		m_gengrid_item = NULL;
	}
}
