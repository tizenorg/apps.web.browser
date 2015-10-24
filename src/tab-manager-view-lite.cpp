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
 * Contact: Karthick R <karthick.r@samsung.com>
 *
 */

#include <Elementary.h>
#include <app.h>
#include <efl_extension.h>
#include "browser.h"
#include "browser-view.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "platform-service.h"
#include "tab-manager-view-lite.h"
#include "webview.h"
#include "webview-list.h"
#include "preference.h"

#define common_edj_path browser_edj_dir"/browser-common.edj"
#define multiwindow_view_edj_path browser_edj_dir"/multiwindow-view.edj"
#define tab_manager_view_lite_edj_path browser_edj_dir"/tab-manager-view-lite.edj"
#define tab_manager_view_gengrid_edj browser_edj_dir"/tab-manager-view-gengrid.edj"

tab_manager_view_lite::tab_manager_view_lite(void)
	: m_naviframe_item(NULL)
	, m_main_layout(NULL)
	, m_grid_layout(NULL)
	, m_more_popup(NULL)
	, m_theme(NULL)
	, m_gengrid(NULL)
	, m_multiwindow_gengrid_ic(NULL)
{
	BROWSER_LOGD("");
	m_theme = elm_theme_new();
	elm_theme_ref_set(m_theme, NULL);
	elm_theme_extension_add(m_theme, tab_manager_view_gengrid_edj);
}

tab_manager_view_lite::~tab_manager_view_lite(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0 ; i < m_tabview_item_list.size() ; i++)
		delete m_tabview_item_list[i];
	m_tabview_item_list.clear();

	if (m_multiwindow_gengrid_ic)
		elm_gengrid_item_class_free(m_multiwindow_gengrid_ic);

	_close_more_context_popup();

	elm_theme_extension_del(m_theme, tab_manager_view_gengrid_edj);
	elm_theme_free(m_theme);
}

void tab_manager_view_lite::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	tab_manager_view_lite *tbv = (tab_manager_view_lite *) data;
	if (!tbv->m_tabview_item_list.size()) { // when no item in tab manager , browser will get close on back button press
		elm_win_lower(m_window);
		return;
	}

	tbv->_exit_tab_manager_view_lite();
}

void tab_manager_view_lite::__more_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	tab_manager_view_lite *tbv = (tab_manager_view_lite *)data;
	tbv->_show_more_context_popup();
}

void tab_manager_view_lite::on_rotate(Eina_Bool landscape)
{
	BROWSER_LOGD("");

	RET_MSG_IF(!m_main_layout, "m_main_layout is NULL");
	RET_MSG_IF(!m_gengrid, "m_gengrid is NULL");

	Eina_List* list = elm_gengrid_realized_items_get(m_gengrid);
	Eina_List *l = NULL;
	void *data = NULL;
	EINA_LIST_FOREACH(list, l, data){
		tab_view_item_lite *item = (tab_view_item_lite *)elm_object_item_data_get((Elm_Object_Item*)data);
		Evas_Object *content_layout = item->m_container_layout;
		if(landscape){
			elm_object_signal_emit(content_layout, "landscape,signal", "");
		}else{
			elm_object_signal_emit(content_layout, "portrait,signal", "");
		}
	}

	if(landscape){
		elm_gengrid_item_size_set(m_gengrid, TAB_MANAGER_LANDSCAPE_ITEM_WIDTH, TAB_MANAGER_LANDSCAPE_ITEM_HEIGHT);
	} else {
		elm_gengrid_item_size_set(m_gengrid, TAB_MANAGER_ITEM_WIDTH, TAB_MANAGER_ITEM_HEIGHT);
	}
}

void tab_manager_view_lite::__rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	__resize_ctxpopup_cb(data);
}

Eina_Bool tab_manager_view_lite::__resize_ctxpopup_cb(void *data)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL, "data is NULL");
	tab_manager_view_lite *tbv = (tab_manager_view_lite *)data;

	if (!tbv->m_more_popup)
		return ECORE_CALLBACK_CANCEL;

	// Move ctxpopup to proper position.
	int x = 0;
	int y = 0;
	platform_service ps;
	ps.get_more_ctxpopup_position(&x, &y);
	evas_object_move(tbv->m_more_popup, x, y);

	return ECORE_CALLBACK_CANCEL;
}

void tab_manager_view_lite::__context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	tab_manager_view_lite *tbv = (tab_manager_view_lite *)data;
	tbv->_close_more_context_popup();
}

void tab_manager_view_lite::_show_more_context_popup(void) // need to implement
{
	BROWSER_LOGD("");

	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	m_more_popup = more_popup;
	elm_object_style_set(more_popup, "more/default");
	elm_ctxpopup_direction_priority_set(more_popup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", __context_popup_dismissed_cb, this);
	elm_ctxpopup_auto_hide_disabled_set(more_popup, EINA_TRUE);

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
#endif

	elm_ctxpopup_item_append(more_popup, BR_STRING_NEW_WINDOW, NULL, __add_new_tab_cb, this);
	Elm_Object_Item *close_all = elm_ctxpopup_item_append(more_popup, BR_STRING_CLOSE_ALL, NULL, __close_all_cb, this);
	if (!m_tabview_item_list.size())
		elm_object_item_disabled_set(close_all, EINA_TRUE);

	evas_object_smart_callback_add(elm_object_top_widget_get(m_more_popup), "rotation,changed", __rotate_ctxpopup_cb, this);
	__resize_more_ctxpopup_cb(this);

	evas_object_show(more_popup);

}

void tab_manager_view_lite::_close_more_context_popup(void)
{
	BROWSER_LOGD("");

	if (m_more_popup) {
		evas_object_smart_callback_del(elm_object_top_widget_get(m_more_popup), "rotation,changed", __rotate_ctxpopup_cb);
		evas_object_del(m_more_popup);
		m_more_popup = NULL;
	}
}

Eina_Bool tab_manager_view_lite::__resize_more_ctxpopup_cb(void *data)
{
	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL, "data is NULL");

	tab_manager_view_lite *tmv = (tab_manager_view_lite *)data;

	if (!tmv->m_more_popup)
		return ECORE_CALLBACK_CANCEL;

	BROWSER_LOGD("");

	// Move ctxpopup to proper position.
	int x = 0;
	int y = 0;
	platform_service ps;
	ps.get_more_ctxpopup_position(&x, &y);

	evas_object_move(tmv->m_more_popup, x, y);

	return ECORE_CALLBACK_CANCEL;
}

void tab_manager_view_lite::__add_new_tab_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	tab_manager_view_lite *tbv = (tab_manager_view_lite *)data;
	tbv->_close_more_context_popup();

	if (tbv->_add_new_webview())
		tbv->_exit_tab_manager_view_lite();
}

Eina_Bool tab_manager_view_lite::_add_new_webview(void)
{
	BROWSER_LOGD("");

	browser_view *bv = m_browser->get_browser_view();
	RETV_MSG_IF(!bv, EINA_FALSE, "browser view is NULL");

	if (m_browser->get_webview_list()->get_count() >= BROWSER_WINDOW_MAX_SIZE){
		bv->show_max_window_limit_reached_popup();
			return EINA_FALSE;
	}

		webview *wv = m_browser->get_webview_list()->create_renewed_webview(EINA_TRUE);
		bv->set_current_webview(wv);

		char *default_homepage = m_preference->get_homepage_uri();
		if (default_homepage && strlen(default_homepage))
			wv->load_uri(default_homepage);
		if (default_homepage)
			free(default_homepage);

	return EINA_TRUE;
}

void tab_manager_view_lite::_delete_webview(webview *deleted_wv)
{
	BROWSER_LOGD("");

	browser_view* bv = m_browser->get_browser_view();
	webview *current_wv = bv->get_current_webview();
	webview *replace_wv = m_browser->get_webview_list()->delete_webview(deleted_wv);
	if (replace_wv) {
		if (current_wv == deleted_wv) {
			bv->set_current_webview(replace_wv);
			for (unsigned int i = 0; i < m_tabview_item_list.size(); i++) {
#ifdef ENABLE_INCOGNITO_WINDOW
				if (m_tabview_item_list[i]) {
					const webview* wv = m_tabview_item_list[i]->get_webview();
					if (wv == replace_wv) {
						m_tabview_item_list[i]->set_current_tab_status(EINA_TRUE,
						                                               wv->is_incognito());
						break;
					}
#else
				if (m_tabview_item_list[i] && m_tabview_item_list[i]->get_webview() == replace_wv) {
					m_tabview_item_list[i]->set_current_tab_status(EINA_TRUE);
					break;
#endif
				}
			}
		}
	} else
		bv->set_current_webview(NULL);
}

void tab_manager_view_lite::_close_all_webviews()
{
	BROWSER_LOGD("");

	webview_list *wv_list = m_browser->get_webview_list();

	// Caution : When delete_webview() of webview_list, the get_webview_list() is also decreased.
	// So 'i--' is needed.
	for (unsigned int i = 0 ; i < wv_list->get_count() ; i++) {
		webview *wv = wv_list->get_webview(i);
		if (wv) {
			m_browser->get_webview_list()->delete_webview(wv);
			i--;
		}
	}
}

void tab_manager_view_lite::_exit_tab_manager_view_lite()
{
	BROWSER_LOGD("");

//Deregister MENU & BACK callbacks before popping the navi item, as tab manager view is deleted in
//browser view popped finished
#if defined(HW_MORE_BACK_KEY)
	if (m_main_layout) {
		eext_object_event_callback_del(m_main_layout, EEXT_CALLBACK_MORE, __more_cb);
		eext_object_event_callback_del(m_main_layout, EEXT_CALLBACK_BACK, __back_cb);
	}
#endif
	elm_naviframe_item_pop(m_naviframe);
}

void tab_manager_view_lite::show()
{
	BROWSER_LOGD("");

	m_main_layout = _create_main_layout(m_naviframe);
	if (  m_browser->get_webview_list()->get_count()  ) {
		m_gengrid = _create_gengrid(m_main_layout);
		elm_object_part_content_set(m_main_layout, "elm.swallow.grid", m_gengrid);
	} else {
		m_gengrid = NULL;
		elm_object_part_content_set(m_main_layout, "elm.swallow.grid",
						_create_no_contents_layout(m_main_layout));
	}
	_create_new_tab_button();
	m_naviframe_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_BODY_WINDOW_MANAGER", NULL, NULL, m_main_layout, NULL);
	elm_object_item_domain_text_translatable_set(m_naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);
#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_MORE, __more_cb, this);
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_BACK, __back_cb, this);
#endif


	Evas_Object *del_button = elm_button_add(m_main_layout);
	if (!del_button) {
		BROWSER_LOGE("elm_button_add failed");
	}
}

void tab_manager_view_lite::_create_new_tab_button()
{
	BROWSER_LOGD("");

	Evas_Object *new_tab_button = elm_button_add(m_main_layout);
	if (new_tab_button) {
		elm_object_domain_translatable_part_text_set(new_tab_button, NULL, BROWSER_DOMAIN_NAME,"IDS_BR_OPT_NEW_WINDOW");
		evas_object_smart_callback_add(new_tab_button, "clicked", __add_new_tab_cb, this);
		elm_layout_content_set(m_main_layout, "elm.swallow.new_tab_button", new_tab_button);
		evas_object_show(new_tab_button);

	}
}

Evas_Object * tab_manager_view_lite::_create_no_contents_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	Evas_Object *no_contents_layout = elm_layout_add(parent);
	if (no_contents_layout) {
		evas_object_size_hint_weight_set(no_contents_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(no_contents_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_layout_theme_set(no_contents_layout, "layout", "nocontents", "default");
		elm_object_domain_translatable_part_text_set(no_contents_layout, "elm.text", BROWSER_DOMAIN_NAME, "IDS_BR_BODY_NO_WINDOWS");
		elm_object_domain_translatable_part_text_set(no_contents_layout, "elm.help.text", BROWSER_DOMAIN_NAME, "IDS_BR_BODY_AFTER_YOU_VIEW_WEBPAGES_THEY_WILL_BE_SHOWN_HERE");
		elm_layout_signal_emit(no_contents_layout, "align.center", "elm");
	}
	return no_contents_layout;
}


Evas_Object *tab_manager_view_lite::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, tab_manager_view_lite_edj_path, "main-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	return layout;
}

Evas_Object *tab_manager_view_lite::_create_gengrid(Evas_Object *parent)
{
	RETV_MSG_IF(!parent, NULL, "parent is NULL");
	Evas_Object *gengrid = elm_gengrid_add(parent);
	if (!gengrid) {
		BROWSER_LOGE("elm_gengrid_add failed");
		return NULL;
	}
	elm_object_theme_set(gengrid, m_theme);

	if (m_browser->is_tts_enabled())
		elm_object_focus_allow_set(gengrid, EINA_FALSE);
	else
		elm_object_focus_allow_set(gengrid, EINA_TRUE);

	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	if (m_browser->get_browser_view()->is_landscape()) {
		elm_gengrid_item_size_set(gengrid, TAB_MANAGER_LANDSCAPE_ITEM_WIDTH, TAB_MANAGER_LANDSCAPE_ITEM_HEIGHT);
	} else {
		elm_gengrid_item_size_set(gengrid, TAB_MANAGER_ITEM_WIDTH, TAB_MANAGER_ITEM_HEIGHT);
	}
	elm_gengrid_align_set(gengrid, 0.5, 0.0);
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);

	m_multiwindow_gengrid_ic = elm_gengrid_item_class_new();
	if (m_multiwindow_gengrid_ic) {
		m_multiwindow_gengrid_ic->item_style = "tab_manager_grid";
		m_multiwindow_gengrid_ic->func.text_get = __get_text_cb;
		m_multiwindow_gengrid_ic->func.content_get = __get_content_cb;
		m_multiwindow_gengrid_ic->func.state_get = NULL;
		m_multiwindow_gengrid_ic->func.del = NULL;
	}

	int count = m_browser->get_webview_list()->get_count();
	webview *current_wv = m_browser->get_browser_view()->get_current_webview();
	for (int i = 0 ; i < count ; i++) {
		webview *wv = m_browser->get_webview_list()->get_webview(i);
		if (wv){
			tab_view_item_lite *item = new tab_view_item_lite(wv, __item_delete_cb, this, current_wv == wv);

			Elm_Object_Item *it = elm_gengrid_item_append(gengrid, m_multiwindow_gengrid_ic, item, __item_selected_cb, this);
			item->set_gengrid_item(it);
			m_tabview_item_list.push_back(item);
		}
	}
	elm_scroller_bounce_set(gengrid, EINA_FALSE, EINA_TRUE);
	return gengrid;
}

char *tab_manager_view_lite::__get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	tab_view_item_lite *item = (tab_view_item_lite *)data;
	if (!strcmp(part, "elm.text")) {
		if (item->get_title() && strlen(item->get_title())) {
			std::string title_str = std::string("<font_size=28><color=#FFFFFF>") + std::string(item->get_title()) + std::string("</color></font_size>");
			BROWSER_SECURE_LOGD("%s", title_str.c_str());
			return strdup(title_str.c_str());
		}
	}

	return NULL;
}

Evas_Object *tab_manager_view_lite::__get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	tab_view_item_lite *item = (tab_view_item_lite *)data;
	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *tt =  item->get_item_layout(obj);
		elm_layout_edje_get(item->m_container_layout);
		return tt;
	}
	return NULL;
}

void tab_manager_view_lite::__item_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	tab_view_item_lite *item = (tab_view_item_lite *)event_info;
	tab_manager_view_lite *tmv = (tab_manager_view_lite *)data;
	tmv->delete_tab(item->get_webview());
}

void tab_manager_view_lite::delete_tab(webview *deleted_wv)
{
	BROWSER_LOGD("");

	evas_object_freeze_events_set(m_gengrid, EINA_TRUE);
	if (deleted_wv)
		_delete_webview(deleted_wv);

	for (unsigned int i = 0; i < m_tabview_item_list.size(); i++) {
		if (m_tabview_item_list[i] && m_tabview_item_list[i]->get_webview() == deleted_wv) {
			tab_view_item_lite *item = m_tabview_item_list[i];
			item->delete_gengrid_item();
			delete item;
			m_tabview_item_list.erase(m_tabview_item_list.begin() + i);
			break;
		}
	}

	evas_object_freeze_events_set(m_gengrid, EINA_FALSE);
	if(!m_browser->get_webview_list()->get_count()) {
		m_gengrid = NULL;
		elm_object_part_content_set(m_main_layout, "elm.swallow.grid",
		                           _create_no_contents_layout(m_main_layout));
}

}
void tab_manager_view_lite::__close_all_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	tab_manager_view_lite *tmv = (tab_manager_view_lite *)data;

	tmv->_close_more_context_popup();

	elm_gengrid_clear(tmv->m_gengrid);

	tmv->_close_all_webviews();

	for (unsigned int i = 0 ; i < tmv->m_tabview_item_list.size() ; i++)
		delete tmv->m_tabview_item_list[i];
	tmv->m_tabview_item_list.clear();
	tmv->m_gengrid = NULL;
	elm_object_part_content_set(tmv->m_main_layout, "elm.swallow.grid",
					_create_no_contents_layout(tmv->m_main_layout));
}

void tab_manager_view_lite::__item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");
	RET_MSG_IF(!event_info, "event_info is NULL");

	tab_manager_view_lite *tmv = (tab_manager_view_lite *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	tab_view_item_lite *item = (tab_view_item_lite *)elm_object_item_data_get(it);

	webview *selected_wv = item->get_webview();

	/*
	 * Selected item will be removed, thus we should not exit the tab manager
	 * or set selected webview as the current webview.
	 */
	if (item->deleteButtonPressed())
		return;

	m_browser->get_browser_view()->set_current_webview(selected_wv);
	tmv->_exit_tab_manager_view_lite();
}

void tab_manager_view_lite::gengrid_item_access_disable()
{
	if (!m_browser->is_tts_enabled())
		return;

	Elm_Object_Item *it = elm_gengrid_first_item_get(m_gengrid);
	RET_MSG_IF(!it, "item is NULL");

	while (it) {
	//	elm_object_item_access_unregister(it);
		it = elm_gengrid_item_next_get(it);
	}

}
