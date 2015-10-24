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
 * Contact: Jihye Song <jihye3.song@samsung.com>
 *
 */

#include <Elementary.h>
#include <app.h>
#include <efl_extension.h>
#include <set>

#include "history-view.h"
#include "bookmark-add-view.h"
#include "browser.h"
#include "browser-view.h"
#include "bookmark-view.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "history.h"
#include "platform-service.h"
#include "webview.h"
#include "webview-list.h"

#define MAX_DATE_LENTH	1024
#define ITEM_COUNT_APPEND 20
#define history_view_edj_path browser_edj_dir"/history-view.edj"
#define bookmark_view_edj_path browser_edj_dir"/bookmark-view.edj"
#define common_edj_path browser_edj_dir"/browser-common.edj"
#define browser_genlist_edj_path browser_edj_dir"/browser-genlist.edj"
#define BR_TEMP_STRING_STATE "State"	//TODO : Need to modify after corresponding string available
Ecore_Thread *history_view::m_current_thread;
std::vector<history_item *> history_view::m_item_vector;

history_view::history_view(void)
	: m_naviframe_item(NULL)
	, m_main_layout(NULL)
	, m_genlist(NULL)
	, m_popup_processing(NULL)
	, m_progressbar(NULL)
	, m_item_ic(NULL)
	, m_date_ic(NULL)
	, m_long_pressed_popup_genlist_item(NULL)
	, m_btn_delete(NULL)
	, m_select_all_layout(NULL)
	, m_show_toast_message(EINA_FALSE)
	, m_delete_popup(NULL)
	, m_title_select_all_button(NULL)
	, m_toolbar_item(NULL)
	, m_current_selected_item(NULL)
	, m_popup(NULL)
	, m_selected_item(NULL)
	, m_more_popup(NULL)
	, m_edit_mode(EINA_FALSE)
	, m_clear_history_popup(NULL)
	, m_tabbar(NULL)
	, m_history(NULL)
	, m_bookmarks(NULL)
	, m_today_parent(NULL)
	, m_yesterday_parent(NULL)
	, m_lastweek_parent(NULL)
	, m_lastmonth_parent(NULL)
	, m_older_parent(NULL)
	, m_parent(NULL)
	, m_ahead_parent(NULL)
	, m_append_timer(NULL)
	, m_count_checked_item(0)
	, m_list_item_count(0)
{
	BROWSER_LOGD("");

	m_browser->register_history_listener(this);

	if (system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_TIME_CHANGED,
			__system_time_changed_cb, this) != SYSTEM_SETTINGS_ERROR_NONE)
	{
		BROWSER_LOGD("Failed to register system time changed callback!");
	}
}

history_view::~history_view(void)
{
	BROWSER_LOGD("");
	if(m_current_thread) {
		if (ecore_thread_cancel(m_current_thread))
			BROWSER_LOGE("Thread has been cancelled");
		else
			BROWSER_LOGE("Thread is pending for cancellation");
	}
	m_browser->unregister_history_listener(this);
	for (unsigned int i = 0 ; i < m_history_list.size() ; i++)
		delete m_history_list[i];
	m_history_list.clear();

	m_today_history_list.clear();

	m_yesterday_history_list.clear();

	m_lastweek_history_list.clear();

	m_lastmonth_history_list.clear();

	m_older_history_list.clear();

	m_current_history_list.clear();

	m_ahead_history_list.clear();

	_close_more_context_popup();

	if (m_delete_popup) {
		clear_popups();
		m_delete_popup = NULL;
	}

	if (m_append_timer) {
		ecore_timer_del(m_append_timer);
		m_append_timer = NULL;
	}

	if (m_item_ic)
		elm_genlist_item_class_free(m_item_ic);

	if (m_date_ic)
		elm_genlist_item_class_free(m_date_ic);

	if (m_title_select_all_button)
		evas_object_del(m_title_select_all_button);

	if (m_toolbar_item)
		elm_object_item_del(m_toolbar_item);

	if (m_popup)
		evas_object_del(m_popup);

	if (m_popup_processing)
		evas_object_del(m_popup_processing);

	if (m_progressbar)
		evas_object_del(m_progressbar);

	if (m_no_contents_scroller) {
		Evas_Object *no_contents_layout = elm_object_content_get(m_no_contents_scroller);
		if (no_contents_layout)
			evas_object_del(no_contents_layout);
		evas_object_del(m_no_contents_scroller);
	}

	if (m_bookmarks)
		evas_object_del(m_bookmarks);

	if (m_history)
		evas_object_del(m_history);

	if (m_tabbar)
		evas_object_del(m_tabbar);

	evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_finished_cb);

	if (system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_TIME_CHANGED) != SYSTEM_SETTINGS_ERROR_NONE)
		BROWSER_LOGD("Failed to unregister system time changed callback!");
}

void history_view::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	history_view *hv = (history_view *)data;
	if (hv->m_edit_mode) {
		hv->_set_edit_mode(EINA_FALSE);
		hv->_update_genlist_item_fields();
		return;
	}
	if (m_browser->get_history()->m_get_thread()) {
		if ( ecore_thread_cancel(m_browser->get_history()->m_get_thread()))
			BROWSER_LOGE("Thread has been cancelled");
		else
			BROWSER_LOGE("Thread is pending for cancellation");

		return;
	}

	elm_naviframe_item_pop(m_naviframe);
}

void history_view::__more_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	history_view *hv = (history_view *)data;
	if (hv->m_edit_mode)
		return;
	RET_MSG_IF(hv->m_popup_processing || hv->m_clear_history_popup, "Clear history is in progress or history clear popup is about to form");

	if (hv->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe)) {
		BROWSER_LOGD("history view is not on top");
		return;
	}

	if (hv->m_history_list.size())
		hv->_show_more_context_popup();
}

void history_view::__rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	__resize_history_ctxpopup_cb(data);
}

Eina_Bool history_view::__resize_history_ctxpopup_cb(void *data)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL, "data is NULL");
	history_view *hv = (history_view *)data;

	if (!hv->m_more_popup)
		return ECORE_CALLBACK_CANCEL;

	// Move history ctxpopup to proper position.
	int x = 0;
	int y = 0;
	platform_service ps;
	ps.get_more_ctxpopup_position(&x, &y);
	evas_object_move(hv->m_more_popup, x, y);

	return ECORE_CALLBACK_CANCEL;
}

void history_view::hide_history_popups()
{
	BROWSER_LOGD("");
	if(m_current_thread) {
		if (ecore_thread_cancel(m_current_thread))
			BROWSER_LOGE("Thread has been cancelled");
		else
			BROWSER_LOGE("Thread is pending for cancellation");
	}
	if (m_browser->get_history()->m_get_thread()) {
		if ( ecore_thread_cancel(m_browser->get_history()->m_get_thread()))
			BROWSER_LOGE("Thread has been cancelled");
		else
			BROWSER_LOGE("Thread is pending for cancellation");
	}
	_close_more_context_popup();
}

void history_view::__history_cleared(Eina_Bool is_cancelled)
{
	BROWSER_LOGD("");
	_close_processing_popup();
	if (is_cancelled)
		_rebuild_list();
	else {
		_create_show_no_contents();

		for (unsigned int i = 0 ; i < m_history_list.size() ; i++)
			delete m_history_list[i];

		m_history_list.clear();
	}
}

void history_view::__context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	history_view *hv = (history_view *)data;

	hv->_close_more_context_popup();
}

void history_view::_show_more_context_popup(void)
{
	BROWSER_LOGD("");
	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	elm_object_style_set(more_popup, "more/default");
	elm_ctxpopup_direction_priority_set(more_popup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", __context_popup_dismissed_cb, this);
	elm_ctxpopup_auto_hide_disabled_set(more_popup, EINA_TRUE);
	evas_object_smart_callback_add(elm_object_top_widget_get(more_popup), "rotation,changed", __rotate_ctxpopup_cb, this);
	m_more_popup = more_popup;

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
#endif

	brui_ctxpopup_item_append(more_popup, BR_STRING_DELETE,
			__more_delete_cb, common_edj_path,
			"I01_more_popup_icon_delete.png", this);

	__resize_history_ctxpopup_cb(this);

	evas_object_show(more_popup);
}

void history_view::_close_more_context_popup(void)
{
	BROWSER_LOGD("");
	if (m_more_popup) {
		evas_object_smart_callback_del(elm_object_top_widget_get(m_more_popup), "rotation,changed", __rotate_ctxpopup_cb);
		evas_object_del(m_more_popup);
		m_more_popup = NULL;
	}
}

void history_view::_add_tabbar()
{
	BROWSER_LOGD("");
	RET_MSG_IF(!m_main_layout, "m_main_layout is NULL");

	m_first_tab_selected = true;
	m_tabbar = _create_tabbar(m_main_layout);
	m_bookmarks = elm_toolbar_item_append(m_tabbar, NULL, BR_STRING_BOOKMARKS, __bookmark_clicked_cb, this);
	m_history = elm_toolbar_item_append(m_tabbar, NULL, BR_STRING_HISTORY, NULL, NULL);
	elm_toolbar_item_selected_set(m_history, EINA_TRUE);
	elm_object_part_content_set(m_main_layout, "elm.swallow.tabbar", m_tabbar);
}

void history_view::show()
{
	BROWSER_LOGD("");

	m_main_layout = _create_main_layout(m_naviframe, bookmark_view_edj_path);
	m_genlist = _create_genlist(m_main_layout);
	if (m_genlist) {
		_add_tabbar();
		elm_object_part_content_set(m_main_layout, "elm.swallow.contents", m_genlist);
	} else {
		_create_show_no_contents();
	}

	m_naviframe_item = elm_naviframe_item_push(m_naviframe,
			"IDS_BR_BODY_BOOKMARKS", NULL, NULL, m_main_layout, NULL);
	elm_object_item_domain_text_translatable_set(m_naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);
	evas_object_smart_callback_add(m_naviframe, "transition,finished",
			__naviframe_pop_finished_cb, this);

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_MORE, __more_cb, this);
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_BACK, __back_cb, this);
#endif
}

void history_view::_show_processing_popup(void)
{
	BROWSER_LOGD("");

	if (m_popup_processing) {
		evas_object_del(m_popup_processing);
		m_popup_processing = NULL;
	}

	m_popup_processing = brui_popup_add(m_naviframe);
	RET_MSG_IF(!m_popup_processing, "m_popup_processing is NULL");

	Evas_Object *pgbar_outer_layout = elm_layout_add(m_popup_processing);
	elm_layout_file_set(pgbar_outer_layout, browser_edj_dir"/browser-popup-lite.edj", "processing_popup");
	evas_object_size_hint_weight_set(pgbar_outer_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_progressbar = elm_progressbar_add(m_popup_processing);
	elm_progressbar_pulse(m_progressbar, EINA_TRUE);
	elm_object_style_set(m_progressbar, "pending");

	elm_progressbar_horizontal_set(m_progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(m_progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(m_progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(m_progressbar);

	elm_object_part_content_set(pgbar_outer_layout, "elm.swallow.content", m_progressbar);
	elm_object_content_set(m_popup_processing, pgbar_outer_layout);
	evas_object_show(m_popup_processing);
}

void history_view::_close_processing_popup(void)
{
	BROWSER_LOGD("");

	if (m_popup_processing) {
		evas_object_del(m_popup_processing);
		m_popup_processing = NULL;
	}
	m_progressbar = NULL;
}

void history_view::_close_clear_history_popup(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	history_view *hv = (history_view *)data;
	if (hv->m_clear_history_popup) {
		evas_object_del(hv->m_clear_history_popup);
		hv->m_clear_history_popup = NULL;
	}
}

void history_view::__more_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	history_view *hv = (history_view *)data;
	hv->_set_edit_mode(EINA_TRUE);

	hv->_close_more_context_popup();

	hv->_update_genlist_item_fields();
}

void history_view::_update_genlist_after_deletion(void* data) {
	browser *bv = (browser*)data;
	history_view *hv = bv->get_history_view();

	for (unsigned int i=0 ;i<m_item_vector.size();i++){
		if (m_item_vector[i]->get_it_data())
			elm_object_item_del(m_item_vector[i]->get_it_data());
		hv->_delete_history_item(m_item_vector[i]);
	}
	// To delete parent items after child list is null
	hv->_delete_date_only_label_genlist_item();

	if (hv->m_count_checked_item > 0)
		hv->_set_selected_title();
	else
		 hv->_unset_selected_title();

	if (hv->m_toolbar_item != NULL){
		elm_object_item_disabled_set(hv->m_toolbar_item, EINA_TRUE);
	}
	elm_access_say(BR_STRING_DELETED);
	hv->_set_edit_mode(EINA_FALSE);
}

void history_view::_update_genlist_item_fields() {
	Elm_Object_Item *it = elm_genlist_first_item_get(this->m_genlist);
	it = elm_genlist_item_next_get(it);
	while (it) {
		if (elm_genlist_item_type_get(it) != ELM_GENLIST_ITEM_GROUP)
			elm_genlist_item_fields_update(it, "elm.icon.2", ELM_GENLIST_ITEM_FIELD_CONTENT);
		it = elm_genlist_item_next_get(it);
	}
}

void history_view::_thread_start_clear_items_from_history_db(void* data, Ecore_Thread *th) {
	BROWSER_LOGD("Thread Started");
	browser *bv = (browser*)data;
	history_view *hv = bv->get_history_view();
	m_item_vector.clear();
	for(unsigned int i = 0 ; i < hv->m_history_list.size(); i++) {
		if (ecore_thread_check(m_current_thread)) {
			BROWSER_LOGE("Cancelling the thread");
			m_current_thread = NULL;
			return;
		}
		if (hv->m_history_list[i]->get_is_checked_data()) {
			m_item_vector.push_back(hv->m_history_list[i]);
			m_browser->get_history()->delete_history(hv->m_history_list[i]->get_uri());
			hv->m_count_checked_item--;
		}
	}
}

void history_view::__thread_cancel_cb(void* data, Ecore_Thread *th) {
	BROWSER_LOGD("Thread Canceled");
	browser *bv = (browser*)data;
	history_view *hv = bv->get_history_view();
	hv->_update_genlist_after_deletion(data);
	hv->_close_processing_popup();
	m_current_thread = NULL;
}

void history_view::__thread_end_cb(void* data, Ecore_Thread *th)
{
	BROWSER_LOGD("Thread Ended");

	browser *bv = (browser*)data;
	history_view *hv = bv->get_history_view();
	hv->_set_edit_mode(EINA_FALSE);
	hv->_update_genlist_item_fields();
	hv->_update_genlist_after_deletion(data);
	hv->_close_processing_popup();

	if (hv->m_show_toast_message) {
		hv->show_noti_popup(BR_STRING_BROWSING_HISTORY_IS_DELETED);
		hv->m_show_toast_message = EINA_FALSE;
	}
	m_current_thread = NULL;
}

void history_view::__toolbar_delete_confirm_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	history_view *hv = (history_view*)data;
	if (hv->m_delete_popup)
		hv->m_delete_popup = NULL;
	hv->_show_processing_popup();

	m_current_thread = ecore_thread_run(_thread_start_clear_items_from_history_db, __thread_end_cb, __thread_cancel_cb,(void *)m_browser);
	if (!m_current_thread) {
		BROWSER_LOGD("Thread Creation Failed");
		hv->_close_processing_popup();
	}
}

void history_view::__toolbar_delete_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	history_view *hv = (history_view*)data;
	if (hv->m_delete_popup)
		hv->m_delete_popup = NULL;
}

void history_view::select_all_items()
{
	BROWSER_LOGD("");
	m_count_checked_item = 0;
	for (unsigned int i = 0 ; i < m_today_history_list.size(); i++) {
		m_today_history_list[i]->set_checked_data(EINA_TRUE);
		m_count_checked_item++;
	}

	for (unsigned int i = 0 ; i < m_yesterday_history_list.size(); i++) {
		m_yesterday_history_list[i]->set_checked_data(EINA_TRUE);
		m_count_checked_item++;
	}

	for (unsigned int i = 0 ; i < m_lastweek_history_list.size(); i++) {
		m_lastweek_history_list[i]->set_checked_data(EINA_TRUE);
		m_count_checked_item++;
	}

	for (unsigned int i = 0 ; i < m_lastmonth_history_list.size(); i++) {
		m_lastmonth_history_list[i]->set_checked_data(EINA_TRUE);
		m_count_checked_item++;
	}

	for (unsigned int i = 0 ; i < m_older_history_list.size(); i++) {
		m_older_history_list[i]->set_checked_data(EINA_TRUE);
		m_count_checked_item++;
	}

	for (unsigned int i = 0 ; i < m_ahead_history_list.size(); i++) {
		m_ahead_history_list[i]->set_checked_data(EINA_TRUE);
		m_count_checked_item++;
	}

}

void history_view::unselect_all_items()
{
	BROWSER_LOGD("");
	for (unsigned int i = 0 ; i < m_today_history_list.size(); i++) {
		m_today_history_list[i]->set_checked_data(EINA_FALSE);
		m_count_checked_item--;
	}

	for (unsigned int i = 0 ; i < m_yesterday_history_list.size(); i++) {
		m_yesterday_history_list[i]->set_checked_data(EINA_FALSE);
		m_count_checked_item--;
	}

	for (unsigned int i = 0 ; i < m_lastweek_history_list.size(); i++) {
		m_lastweek_history_list[i]->set_checked_data(EINA_FALSE);
		m_count_checked_item--;
	}

	for (unsigned int i = 0 ; i < m_lastmonth_history_list.size(); i++) {
		m_lastmonth_history_list[i]->set_checked_data(EINA_FALSE);
		m_count_checked_item--;
	}

	for (unsigned int i = 0 ; i < m_older_history_list.size(); i++) {
		m_older_history_list[i]->set_checked_data(EINA_FALSE);
		m_count_checked_item--;
	}

	for (unsigned int i = 0 ; i < m_ahead_history_list.size(); i++) {
		m_ahead_history_list[i]->set_checked_data(EINA_FALSE);
		m_count_checked_item--;
	}

}

void history_view::_set_selected_title()
{
	BROWSER_LOGD("");
	std::string sub_title;
	char label_count[1024] = {'\0', };
	char *text = NULL;
	int len ;

	snprintf(label_count, sizeof(label_count), "%d", (m_count_checked_item));
	len = strlen(label_count) + strlen(BR_STRING_SELECTED) + 1;
	text = (char *)malloc(len * sizeof(char));

	if (!text)
		return ;

	memset(text, 0x00, len);
	snprintf(text, len-1, _("IDS_BR_HEADER_PD_SELECTED_ABB"), m_count_checked_item);
	sub_title.append(text);
	elm_object_item_part_text_set(m_naviframe_item, "elm.text.title", sub_title.c_str());
	free(text);
}

void history_view::_unset_selected_title()
{
	BROWSER_LOGD(" ");

	_set_selected_title();
}


void history_view::__select_all_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info, Eina_Bool checkbox_flow_switch)
{
	BROWSER_LOGD("");
	history_view *hv = (history_view *)data;

	if (hv->m_count_checked_item == hv->m_history_list.size()) {
		if(checkbox_flow_switch){
			Evas_Object *ck_all = elm_object_part_content_get(hv->m_select_all_layout, "elm.icon");
			if (ck_all) elm_check_state_set(ck_all, EINA_FALSE);
		}
		hv->unselect_all_items();
	} else {
		if(checkbox_flow_switch){
			Evas_Object *ck_all = elm_object_part_content_get(hv->m_select_all_layout, "elm.icon");
			if (ck_all) elm_check_state_set(ck_all, EINA_TRUE);
		}
		hv->select_all_items();
	}

	hv->_update_genlist_item_fields();

	if (hv->m_count_checked_item) {
		elm_object_disabled_set(hv->m_btn_delete, EINA_FALSE);
		hv->_set_selected_title();
	} else {
		elm_object_disabled_set(hv->m_btn_delete, EINA_TRUE);
		hv->_unset_selected_title();
	}
}

void history_view::__toolbar_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	history_view *hv = (history_view*)data;

	if (hv->m_delete_popup)
		return;
	if (hv->m_count_checked_item != hv->m_history_list.size()) {
		hv->m_show_toast_message = EINA_TRUE;
		__toolbar_delete_confirm_cb(data, obj, event_info);
	} else
		hv->m_delete_popup = hv->show_msg_popup("IDS_BR_SK_DELETE", BR_STRING_ALL_BROWSING_HISTORY_WILLBE_DELETED, __toolbar_delete_cancel_cb, "IDS_BR_SK_CANCEL", __toolbar_delete_cancel_cb , "IDS_BR_SK_DELETE", __toolbar_delete_confirm_cb, data);
}

void history_view::_set_edit_mode(Eina_Bool edit_mode)
{
	BROWSER_LOGD("");
	m_edit_mode = edit_mode;
	if (edit_mode) {
		m_count_checked_item = 0;

		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "set,edit,mode,signal", "");
		_set_select_all_layout();
		Evas_Object *btn_cancel = elm_button_add(m_naviframe);
		if (!btn_cancel) return;
		elm_object_style_set(btn_cancel, "naviframe/title_cancel");
		evas_object_smart_callback_add(btn_cancel, "clicked", __back_cb, this);
		elm_object_item_part_content_set(m_naviframe_item, "title_left_btn", btn_cancel);

		m_btn_delete = elm_button_add(m_naviframe);
		if (!m_btn_delete) return;
		elm_object_style_set(m_btn_delete, "naviframe/title_done");
		evas_object_smart_callback_add(m_btn_delete, "clicked", __toolbar_delete_cb, this);
		elm_object_item_part_content_set(m_naviframe_item, "title_right_btn", m_btn_delete);
		elm_object_disabled_set(m_btn_delete, EINA_TRUE);
		_set_selected_title();
	} else {
		elm_layout_text_set(m_naviframe, NULL, BR_STRING_BOOKMARKS);
		elm_object_style_set(get_elm_bg(), "default");
		if (m_history_list.size() > 0)
			for (unsigned int i = 0 ; i < m_history_list.size() ; i++)
				m_history_list[i]->set_checked_data(EINA_FALSE);

		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "set,default,mode,signal", "");
		Evas_Object *button_cancel = elm_object_item_part_content_unset(m_naviframe_item, "title_left_btn");
		evas_object_hide(button_cancel);
		Evas_Object *button_save = elm_object_item_part_content_unset(m_naviframe_item, "title_right_btn");
		evas_object_hide(button_save);
	}

	if (m_history_list.size())
		elm_genlist_realized_items_update(m_genlist);

}

void history_view::_set_select_all_layout(void)
{
	BROWSER_LOGD("");
	if (m_select_all_layout) {
		evas_object_del(m_select_all_layout);
		m_select_all_layout = NULL;
	}

	m_select_all_layout = elm_layout_add(m_main_layout);
	if (!m_select_all_layout) {
		BROWSER_LOGD("elm_layout_add is failed");
		return;
	}

	elm_object_focus_allow_set(m_select_all_layout, EINA_TRUE);

	elm_layout_theme_set(m_select_all_layout, "genlist", "item", "select_all/default");
	evas_object_size_hint_weight_set(m_select_all_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_select_all_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *check = elm_check_add(m_select_all_layout);
	evas_object_propagate_events_set(check, EINA_FALSE);
	elm_object_part_content_set(m_select_all_layout, "elm.icon", check);
	elm_object_part_text_set(m_select_all_layout, "elm.text.main", BR_STRING_SELECT_ALL);
	evas_object_show(m_select_all_layout);
	elm_object_part_content_set(m_main_layout, "elm.swallow.selectall_layout", m_select_all_layout);

	evas_object_event_callback_add(m_select_all_layout, EVAS_CALLBACK_MOUSE_DOWN, __select_all_key_down_cb, this);
	evas_object_event_callback_add(check, EVAS_CALLBACK_MOUSE_DOWN, __select_all_key_down_checkbox_cb, this);
}

void history_view::__select_all_key_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	history_view *cp = (history_view *)data;

	cp->__select_all_btn_clicked_cb(cp, obj, event_info, EINA_TRUE);
}

void history_view::__select_all_key_down_checkbox_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	history_view *cp = (history_view *)data;

	cp->__select_all_btn_clicked_cb(cp, obj, event_info, EINA_FALSE);
}

void history_view::__clear_history_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	history_view *hv = (history_view *)data;

	hv->_close_more_context_popup();

	hv->m_clear_history_popup = hv->show_msg_popup("IDS_BR_BODY_CLEAR_HISTORY",
				BR_STRING_HISTORY_WILLBE_CLEARED,
				_close_clear_history_popup,
				"IDS_BR_SK_CANCEL",
				_close_clear_history_popup,
				"IDS_BR_SK_CLEAR",
				__popup_clear_btn_clicked_cb,
				data);
	evas_object_del(obj);
}

void history_view::__popup_clear_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	history_view *hv = (history_view *)data;
	hv->_show_processing_popup();
	if(hv->m_clear_history_popup)
		hv->m_clear_history_popup = NULL;

	if (!m_browser->get_history()->delete_all(hv, NULL, NULL))
		hv->_close_processing_popup();
}

char *history_view::__genlist_date_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("");
	if (!strcmp(part, "elm.text.main")) {
	ItemGroupData *itemData = (ItemGroupData *)data;
		Time_stamp_type day_status = itemData->timestamp;

		if (day_status == HISTORY_TODAY)
			return strdup(BR_STRING_HISTORY_TODAY);
		else if (day_status == HISTORY_YESTERDAY)
			return strdup(BR_STRING_HISTORY_YESTERDAY);
		else if (day_status == HISTORY_OLDER)
			return strdup(BR_STRING_HISTORY_OLDER);
		else if (day_status == HISTORY_LAST_7_DAYS)
			return strdup(BR_STRING_HISTORY_LAST_7_DAYS);
		else if (day_status == HISTORY_LAST_MONTH)
			return strdup(BR_STRING_HISTORY_LAST_MONTH);
		else
			return strdup(BR_STRING_AHEAD);
	}

	return NULL;
}

char *history_view::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	RETV_MSG_IF(!data,NULL, "data is NULL");
	history_item *item = (history_item *)data;
	BROWSER_SECURE_LOGD("title=[%s], uri=[%s]", item->get_title(), item->get_uri());
	if (!strcmp(part, "elm.text.main.left.top")) {
		// convert markup string because the string is cut after '#' in elementary.
		return elm_entry_utf8_to_markup(item->get_title());
	} else if (!strcmp(part, "elm.text.sub.left.bottom"))
		return elm_entry_utf8_to_markup(item->get_uri());

	return NULL;
}

void history_view::__edit_checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	history_view *hv = m_browser->get_history_view();
	history_item *genlist_item = (history_item *)data;
	genlist_item->set_checked_data(elm_check_state_get(obj));
	BROWSER_LOGD("check=[%d]", genlist_item->get_is_checked_data());

	if (genlist_item->get_is_checked_data())
		hv->m_count_checked_item++;
	else
		hv->m_count_checked_item--;

	if (hv->m_count_checked_item) {
		elm_object_disabled_set(hv->m_btn_delete, EINA_FALSE);
		Evas_Object *ck_all = elm_object_part_content_get(hv->m_select_all_layout, "elm.icon");
		if (ck_all) {
			if (hv->m_count_checked_item == hv->m_history_list.size())
				elm_check_state_set(ck_all, EINA_TRUE);
			else
				elm_check_state_set(ck_all, EINA_FALSE);
		}
		hv->_set_selected_title();
	}
	else {
		elm_object_disabled_set(hv->m_btn_delete, EINA_TRUE);
		Evas_Object *ck_all = elm_object_part_content_get(hv->m_select_all_layout, "elm.icon");
		if (ck_all) elm_check_state_set(ck_all, EINA_FALSE);
		hv->_unset_selected_title();
	}
}

Evas_Object *history_view::__genlist_date_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("");
	history_view *hv = m_browser->get_history_view();
	Elm_Object_Item *parent = NULL;
	ItemGroupData *itemData = (ItemGroupData *) data;
	Time_stamp_type day_status = HISTORY_OLDER;
	if (itemData)
		day_status = itemData->timestamp;

	switch(day_status) {
		case HISTORY_TODAY:
		{
			parent = hv->m_today_parent;
			break;
		}
		case HISTORY_YESTERDAY:
		{
			parent = hv->m_yesterday_parent;
			break;
		}
		case HISTORY_LAST_7_DAYS:
		{
			parent = hv->m_lastweek_parent;
			break;
		}
		case HISTORY_LAST_MONTH:
		{
			parent = hv->m_lastmonth_parent;
			break;
		}
		case HISTORY_OLDER:
		{
			parent = hv->m_older_parent;
			break;
		}
		case HISTORY_NEXT_DAYS:
		{
			parent = hv->m_ahead_parent;
			break;
		}
		default:
			break;
	}

	Eina_Bool expanded = EINA_FALSE;
	if (itemData)
		expanded = itemData->expanded;

	if (!strcmp(part, "elm.icon")) {
		Evas_Object *arrow_layout = elm_layout_add(obj);
		elm_layout_file_set(arrow_layout, history_view_edj_path, "arrow-layout");
		Evas_Object *arrow = elm_icon_add(obj);
		if (arrow) {
			if (expanded)
				edje_object_signal_emit(elm_layout_edje_get(arrow_layout), "state,expanded,signal", "");
			else
				edje_object_signal_emit(elm_layout_edje_get(arrow_layout), "state,contracted,signal", "");

			return arrow_layout;
		}
		return NULL;
	}
	return NULL;
}

void history_view::__genlist_date_del_cb(void *data, Evas_Object *obj){
	BROWSER_LOGD("");
	if (data)
		delete (ItemGroupData *)data;
}

Evas_Object *history_view::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	RETV_MSG_IF(!data,NULL,"data is NULL");
	history_item *item = (history_item *)data;
	history_view *hv = m_browser->get_history_view();
	BROWSER_SECURE_LOGD("title=[%s], uri=[%s]", item->get_title(), item->get_uri());

	if (!strcmp(part, "elm.icon.1")) {
		Evas_Object *content = elm_layout_add(obj);
		elm_layout_theme_set(content, "layout", "list/B/type.2", "default");
		Evas_Object *favicon = item->copy_favicon();
		Evas_Object *favicon_layout = elm_layout_add(obj);

		if (!favicon) {
			favicon = elm_icon_add(obj);
			elm_image_file_set(favicon, history_view_edj_path, "I01_history_favicon.png");
			evas_object_size_hint_aspect_set(favicon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			elm_layout_file_set(favicon_layout, history_view_edj_path, "history-default-icon-layout");
			elm_object_part_content_set(favicon_layout, "elm.swallow.favicon", favicon);
			elm_layout_content_set(content, "elm.swallow.content", favicon_layout);
			return content;
		}
		elm_layout_file_set(favicon_layout, history_view_edj_path, "favicon-layout");
		elm_object_part_content_set(favicon_layout, "elm.swallow.favicon", favicon);
		elm_layout_content_set(content, "elm.swallow.content", favicon_layout);
		return content;

	} else if (!strcmp(part, "elm.icon.right")) {
		Evas_Object *star_layout = elm_layout_add(obj);
		elm_layout_file_set(star_layout, history_view_edj_path, "star-layout");
		Evas_Object *star = elm_icon_add(obj);
		if (star) {
			if(m_browser->get_bookmark()->is_in_bookmark(item->get_uri())) {
				elm_object_part_content_set(star_layout, "elm.icon.image", star);
				return star_layout;
			}
		}

		 return NULL;
	} else if (!strcmp(part, "elm.icon.2")) {
		if (hv->m_edit_mode) {
			Evas_Object *checkboxlayout = elm_layout_add(obj);
			elm_layout_theme_set(checkboxlayout, "layout", "list/C/type.2", "default");
			if (!checkboxlayout) {
				BROWSER_LOGE("elm_layout_add failed");
				return NULL;
			}
			Evas_Object *check = elm_check_add(obj);
			if (check == NULL) {
				BROWSER_LOGD("elm_check_add is failed");
				return NULL;
			}
			evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_layout_content_set(checkboxlayout, "elm.swallow.content", check);

			if (item->get_is_checked_data()) {
					elm_check_state_set(check, EINA_TRUE);
			}
			evas_object_propagate_events_set(check, EINA_FALSE);
			evas_object_smart_callback_add(check, "changed", __edit_checkbox_changed_cb, item);
			return checkboxlayout;
		}
		return NULL;
	}
	return NULL;
}

void history_view::__genlist_headitem_clicked_cb(void * data,Evas_Object * obj,void * event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");
	history_view *hv = m_browser->get_history_view();
	hv->m_parent = (Elm_Object_Item *)event_info;

	ItemGroupData *itemData = (ItemGroupData *)elm_object_item_data_get(hv->m_parent);
	Eina_Bool expanded = itemData->expanded;
	Evas_Object *item_access_obj = elm_object_item_access_object_get(hv->m_parent);
	elm_genlist_item_selected_set(hv->m_parent, EINA_FALSE);

	Evas_Object *arrow_layout = elm_object_item_part_content_get(hv->m_parent, "elm.icon");

	Time_stamp_type day_status = itemData->timestamp;
	switch(day_status) {
		case HISTORY_TODAY:
		{
			hv->m_current_history_list = hv->get_today_list();
			break;
		}
		case HISTORY_YESTERDAY:
		{
			hv->m_current_history_list = hv->get_yesterday_list();
			break;
		}
		case HISTORY_LAST_7_DAYS:
		{
			hv->m_current_history_list = hv->get_lastweek_list();
			break;
		}
		case HISTORY_LAST_MONTH:
		{
			hv->m_current_history_list = hv->get_lastmonth_list();
			break;
		}
		case HISTORY_OLDER:
		{
			hv->m_current_history_list = hv->get_older_list();
			break;
		}
		case HISTORY_NEXT_DAYS:
		{
			hv->m_current_history_list = hv->get_ahead_list();
			break;
		}
		default:
			break;
	}
	if (expanded == EINA_FALSE) {
		itemData->expanded = EINA_TRUE;

		if (item_access_obj)
			elm_access_info_set(item_access_obj, ELM_ACCESS_TYPE, BR_STRING_DOUBLE_TAP_TO_CLOSE_THE_LIST_T);

		if (hv->m_append_timer) {
			ecore_timer_del(hv->m_append_timer);
			hv->m_append_timer = NULL;
			hv->m_list_item_count = 0;
		}

		hv->m_append_timer = ecore_timer_add(0.01, __items_append_timer_cb , hv);

	} else {
		if (hv->m_append_timer) {
			ecore_timer_del(hv->m_append_timer);
			hv->m_append_timer = NULL;
		}

		for (unsigned int i = 0 ; i < hv->m_current_history_list.size() ; i++) {
			hv->m_current_history_list[i]->set_it_data(NULL);
		}

		itemData->expanded = EINA_FALSE;
		if (item_access_obj)
			elm_access_info_set(item_access_obj, ELM_ACCESS_TYPE, BR_STRING_DOUBLE_TAP_TO_OPEN_THE_LIST_T);
		elm_genlist_item_subitems_clear(hv->m_parent);
		hv->m_list_item_count = 0;
	}
	elm_genlist_item_update(hv->m_parent);
}

Eina_Bool history_view::__items_append_timer_cb(void *data)
{
	BROWSER_LOGD("");

	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL, "data is NULL");
	history_view *hv = (history_view*)data;

	if (!hv->m_current_history_list.size() && hv->m_append_timer) {
		hv->m_append_timer = NULL;
		hv->m_list_item_count = 0;
		return ECORE_CALLBACK_CANCEL;
	}

	std::vector<history_item *> history_list = hv->m_current_history_list;
	unsigned int item_count = 0;

	for (unsigned int i = hv->m_list_item_count ; i < history_list.size() ; i++, item_count++) {
		if (item_count == ITEM_COUNT_APPEND) {
			return ECORE_CALLBACK_RENEW;
		} else {
			Elm_Object_Item *it = elm_genlist_item_append(hv->m_genlist, hv->m_item_ic, history_list[i], hv->m_parent, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, history_list[i]);
			history_list[i]->set_it_data(it);
			hv->m_list_item_count++;
			if (hv->m_list_item_count == history_list.size()) {
				hv->m_current_history_list.clear();
				hv->m_list_item_count = 0;
				hv->m_append_timer = NULL;
				return ECORE_CALLBACK_CANCEL;
			}
		}
	}

	return ECORE_CALLBACK_CANCEL;
}

void history_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	history_item *item = (history_item *)elm_object_item_data_get(it);

	history_view *hv = m_browser->get_history_view() ;

	if (hv->m_edit_mode) {
		elm_genlist_item_selected_set(it, EINA_FALSE);

		Evas_Object *checkboxlayout = NULL;
		Evas_Object *checkbox = NULL;
		checkboxlayout = elm_object_item_part_content_get(it, "elm.icon.2");
		checkbox = elm_object_part_content_get(checkboxlayout,"elm.swallow.content");

		Eina_Bool state = elm_check_state_get(checkbox);
		elm_check_state_set(checkbox, !state);
		__edit_checkbox_changed_cb(item, checkbox, NULL);
		return;
	}
	evas_object_freeze_events_set(m_browser->get_history_view()->m_genlist, EINA_FALSE);
	if (m_browser->get_history_view()->m_popup) {
		Elm_Object_Item *selected_item = (Elm_Object_Item *)event_info;
		elm_genlist_item_selected_set(selected_item, EINA_FALSE);
		return;
	}

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (!wv) {
		wv = m_browser->get_webview_list()->create_webview(EINA_TRUE);
		wv->set_request_uri(item->get_uri());
		m_browser->get_browser_view()->set_current_webview(wv);
	}

	wv->load_uri(item->get_uri());

	elm_naviframe_item_pop_to(elm_naviframe_bottom_item_get(m_naviframe));
}

void history_view::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *item = (Elm_Object_Item*)event_info;

	if ( elm_genlist_item_type_get(item) == ELM_GENLIST_ITEM_GROUP) {
		ItemGroupData *itemData = (ItemGroupData *)elm_object_item_data_get(item);
		Eina_Bool expanded = itemData->expanded;
		Evas_Object *access = elm_object_item_access_object_get(item);
		if (access) {
			if (expanded)
				elm_access_info_set(access , ELM_ACCESS_TYPE, BR_STRING_DOUBLE_TAP_TO_CLOSE_THE_LIST_T);
			else
				elm_access_info_set(access , ELM_ACCESS_TYPE, BR_STRING_DOUBLE_TAP_TO_OPEN_THE_LIST_T);
		 }
	} else {
		Evas_Object *access = elm_object_item_access_object_get(item);
		if (access)
			elm_access_info_set(access , ELM_ACCESS_TYPE, BR_STRING_ACCESS_DOUBLE_TAP_TO_OPEN_WEBPAGE_T);
	}

}

void history_view::_populate_genlist(Evas_Object *genlist, history_item *item, Time_stamp_type time_stamp )
{
	BROWSER_LOGD("");
	RET_MSG_IF(!item, "item is NULL");

	Elm_Object_Item * *parent_item = NULL;
	std::vector<history_item *> *group_item_list = NULL;

	switch (time_stamp) {
		case HISTORY_TODAY:
		{
			parent_item = &m_today_parent;
			group_item_list = &m_today_history_list;
			break;
		}
		case HISTORY_YESTERDAY:
		{
			parent_item = &m_yesterday_parent;
			group_item_list = &m_yesterday_history_list;
			break;
		}
		case HISTORY_LAST_7_DAYS:
		{
			parent_item = &m_lastweek_parent;
			group_item_list = &m_lastweek_history_list;
			break;
		}
		case HISTORY_LAST_MONTH:
		{
			parent_item = &m_lastmonth_parent;
			group_item_list = &m_lastmonth_history_list;
			break;

		}
		case HISTORY_OLDER:
		{
			parent_item = &m_older_parent;
			group_item_list = &m_older_history_list;
			break;
		}
		case HISTORY_NEXT_DAYS:
		{
			parent_item = &m_ahead_parent;
			group_item_list = &m_ahead_history_list;
			break;
		}
		default:
			break;
	}

	if (*parent_item == NULL) {
		ItemGroupData *itemData = new ItemGroupData;
		itemData->timestamp = time_stamp;
		itemData->expanded = EINA_FALSE;
		*parent_item = elm_genlist_item_append(genlist, m_date_ic,(void *)itemData , NULL, ELM_GENLIST_ITEM_GROUP, __genlist_headitem_clicked_cb, (void *)itemData);
	}
	if(*parent_item)
		group_item_list->push_back(item);
}

Evas_Object *history_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	evas_object_smart_callback_add(genlist, "language,changed", common_view::__genlist_lang_changed, NULL);
	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	if(item_ic) {
		item_ic->item_style = "2line.top";
		item_ic->decorate_all_item_style = "edit_default";
		item_ic->func.text_get = __genlist_label_get_cb;
		item_ic->func.content_get = __genlist_icon_get_cb;
		item_ic->func.state_get = NULL;
		item_ic->func.del = NULL;
	}

	Elm_Genlist_Item_Class *date_ic = elm_genlist_item_class_new();

	if (date_ic) {
		date_ic->item_style = "groupindex.sub";
		date_ic->decorate_item_style = NULL;
		date_ic->decorate_all_item_style = "edit_default";
		date_ic->func.text_get = __genlist_date_label_get_cb;
		date_ic->func.content_get = __genlist_date_icon_get_cb;
		date_ic->func.state_get = NULL;
		date_ic->func.del = __genlist_date_del_cb;
	}

	m_history_list = m_browser->get_history()->get_history_list();

	m_item_ic = item_ic;
	m_date_ic = date_ic;

	if (m_history_list.size() == 0) {
		evas_object_del(genlist);
		return NULL;
	}

	for (unsigned int i = 0 ; i < m_history_list.size() ; i++) {
		Time_stamp_type time_stamp = m_history_list[i]->get_time_stamp_type();
		_populate_genlist(genlist, m_history_list[i], time_stamp);
	}

	evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, NULL);

	return genlist;
}

char *history_view::__long_press_popup_genlist_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	RETV_MSG_IF(!data, NULL, "data is NULL");
	const char* ret_val = '\0';
	ret_val = (const char*) data;
	return strdup(ret_val);
}

void history_view::__genlist_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	history_view *hv = (history_view *)data;
	if (hv->m_edit_mode == EINA_TRUE)
		return;

	hv->m_current_selected_item = (Elm_Object_Item *)event_info;
	if (!hv->m_current_selected_item || elm_object_item_data_get(hv->m_current_selected_item) == NULL) {
		BROWSER_LOGE("item_data is NULL");
		return;
	}

	if (elm_genlist_item_type_get(hv->m_current_selected_item) == ELM_GENLIST_ITEM_GROUP)
		return;

	Evas_Object *popup = brui_popup_add(m_window);
	RET_MSG_IF(!popup, "popup is NULL");

	hv->m_popup = popup;
	hv->m_selected_item = (history_item *)elm_object_item_data_get(hv->m_current_selected_item);
	RET_MSG_IF(!hv->m_selected_item, "Selected item is NULL");

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(popup, "block,clicked", __cancel_btn_clicked_cb, hv);

#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __cancel_btn_clicked_cb, hv);
#else
	cancel_btn = elm_button_add(popup);
	elm_object_style_set(cancel_btn, "popup");
	elm_object_text_set(cancel_btn, BR_STRING_CANCEL);
	elm_object_part_content_set(popup, "button1", cancel_btn);
	evas_object_smart_callback_add(cancel_btn, "clicked", __cancel_btn_clicked_cb, hv);
#endif
	const char *uri = hv->m_selected_item->get_uri();

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "title,text", hv->m_selected_item->get_title());

	/* box */
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	Evas_Object *box = elm_box_add(popup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	hv->m_long_pressed_popup_genlist_item = itc;

	/* genlist */
	Evas_Object *genlist = elm_genlist_add(box);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	if (itc){
		itc->item_style = "default";
		itc->func.text_get = __long_press_popup_genlist_text_get_cb;
		itc->func.content_get = NULL;
		itc->func.state_get = NULL;
		itc->func.del = NULL;
	}
	elm_genlist_item_append(genlist, itc, (void *) BR_STRING_SHARE, NULL, ELM_GENLIST_ITEM_NONE, __popup_share_button_clicked_cb, hv);
	if (uri && strlen(uri) > 0 && !m_browser->get_bookmark()->is_in_bookmark(uri))
		elm_genlist_item_append(genlist, itc, (void *) BR_STRING_ADD_BOOKMARK, NULL, ELM_GENLIST_ITEM_NONE, __popup_add_bookmark_button_clicked_cb, hv);

	if(hv->m_selected_item->get_uri() != NULL && strlen(hv->m_selected_item->get_uri()) > 0)
		elm_genlist_item_append(genlist, itc, (void *) BR_STRING_SET_AS_HOMEPAGE, NULL, ELM_GENLIST_ITEM_NONE, __popup_set_homepage_clicked_cb, hv);
	elm_genlist_item_append(genlist, itc, (void *) BR_STRING_DELETE, NULL, ELM_GENLIST_ITEM_NONE, __popup_delete_button_clicked_cb, hv);
	evas_object_show(genlist);
	elm_box_pack_end(box, genlist);
	evas_object_size_hint_min_set(box, -1, LONG_PRESS_LIST_POPUP_MIN_HEIGHT);
	elm_object_content_set(popup, box);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __longpress_popup_del_cb, hv);
	evas_object_show(popup);
	evas_object_freeze_events_set(hv->m_genlist, EINA_TRUE);
	elm_genlist_item_selected_set( hv->m_current_selected_item, EINA_FALSE);

	elm_genlist_item_class_free(hv->m_long_pressed_popup_genlist_item);
	hv->m_long_pressed_popup_genlist_item = NULL;
}

void history_view::__longpress_popup_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	history_view *hv = (history_view *)data;
	evas_object_freeze_events_set(hv->m_genlist, EINA_FALSE);
}

void history_view::__cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	history_view *hv = (history_view *)data;
	if (hv->m_popup) {
		evas_object_del(hv->m_popup);
		hv->m_popup = NULL;
	}
	evas_object_freeze_events_set(hv->m_genlist, EINA_FALSE);
}

void history_view::__bookmark_clicked_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	history_view *hv = (history_view *)data;
	if (hv->m_first_tab_selected) {		// ommit first callback called by default selection, not user click
		hv->m_first_tab_selected = false;
		return;
	}

	edje_object_signal_emit(elm_layout_edje_get(hv->m_main_layout), "play,touch_sound,signal", "");
	elm_naviframe_item_pop(m_naviframe);
}

void history_view::_create_show_no_contents()
{
	BROWSER_LOGD("");

	if (m_genlist)
		evas_object_del(m_genlist);

	_add_tabbar();
	m_no_contents_scroller = _create_no_content(m_main_layout, BR_STRING_NO_HISTORY, BR_STRING_HISTORY_NO_CONTENT_HELP_TEXT);
	elm_object_part_content_set(m_main_layout, "elm.swallow.contents", m_no_contents_scroller);
}

void history_view::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	history_view*hv = (history_view *)data;
	if (hv->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		return;

	m_browser->delete_bookmark_add_view();
}

void history_view::__popup_share_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;
	history_view *hv = (history_view *)data;
	if (hv->m_popup) {
		evas_object_del(hv->m_popup);
		hv->m_popup = NULL;
	}

	if(hv->m_selected_item)
		m_browser->get_browser_view()->share(hv->m_selected_item->get_uri(), hv->m_selected_item->get_title());
}

void history_view::__popup_add_bookmark_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	history_view *hv = (history_view *)data;
	if (hv->m_popup) {
		evas_object_del(hv->m_popup);
		hv->m_popup = NULL;
	}

	m_browser->create_bookmark_add_view(hv->m_selected_item->get_title(), hv->m_selected_item->get_uri(),m_browser->get_bookmark()->get_root_folder_id(), EINA_FALSE)->show();
}

void history_view::__popup_set_homepage_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	history_view *hv = (history_view *)data;
	if (hv->m_popup) {
		evas_object_del(hv->m_popup);
		hv->m_popup = NULL;
	}
	const char *uri = hv->m_selected_item->get_uri();
	RET_MSG_IF(!(uri && strlen(uri) > 0), "Invalid uri");

	m_preference->set_user_homagepage(uri);
	m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);
	hv->show_noti_popup(BR_STRING_SETTING_SAVED);
}

void history_view::__popup_delete_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	history_view *hv = (history_view *)data;
	if (hv->m_popup) {
		evas_object_del(hv->m_popup);
		hv->m_popup = NULL;
	}

	if(hv->m_selected_item)
		m_browser->get_history_view()->show_delete_popup("IDS_BR_OPT_DELETE_HISTORY", "IDS_BR_POP_DELETEHISTORYQUESTION", NULL, "IDS_BR_SK_CANCEL", NULL, "IDS_BR_SK_DELETE", __delete_confirm_response_by_popup_button_cb, hv->m_selected_item);
}

void history_view::__delete_confirm_response_by_popup_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	history_item *item = (history_item *)data;
	elm_object_item_del(item->get_it_data());
	m_browser->get_history()->delete_history(item->get_uri());
	m_browser->get_history_view()->_delete_history_item(item);
	m_browser->get_history_view()->_delete_date_only_label_genlist_item();
}

void history_view::_delete_date_only_label_genlist_item(void)
{
	BROWSER_LOGD("");

	if (m_today_history_list.size() == 0 && m_today_parent) {
		elm_object_item_del(m_today_parent);
		m_today_parent = NULL;
	}
	if (m_yesterday_history_list.size() == 0 && m_yesterday_parent) {
		elm_object_item_del(m_yesterday_parent);
		m_yesterday_parent = NULL;
	}
	if (m_lastweek_history_list.size() == 0 && m_lastweek_parent){
		elm_object_item_del(m_lastweek_parent);
		m_lastweek_parent = NULL;
	}
	if (m_lastmonth_history_list.size() == 0 && m_lastmonth_parent) {
		elm_object_item_del(m_lastmonth_parent);
		m_lastmonth_parent = NULL;
	}
	if (m_older_history_list.size() == 0 && m_older_parent) {
		elm_object_item_del(m_older_parent);
		m_older_parent = NULL;
	}

	if (m_ahead_history_list.size() == 0 && m_ahead_parent) {
		elm_object_item_del(m_ahead_parent);
		m_ahead_parent = NULL;
	}

	if (elm_genlist_items_count(m_genlist) == 0) {
		_create_show_no_contents();
	}
}

void history_view::_delete_history_item(history_item *item)
{
	BROWSER_LOGD("");
	Time_stamp_type time_stamp = item->get_time_stamp_type();
	for(unsigned int i = 0 ; i < m_history_list.size(); i++) {
		if (m_history_list[i] == item) {
			BROWSER_LOGD("FOUND %d",i);
			delete m_history_list[i];
			m_history_list.erase(m_history_list.begin() + i);
			break;
		}
	}
	switch(time_stamp) {
		case HISTORY_TODAY:
		{
			BROWSER_LOGD("TODAY");
			for (unsigned int i = 0 ; i < m_today_history_list.size(); i++) {
				if (m_today_history_list[i] == item) {
					m_today_history_list.erase(m_today_history_list.begin() + i);
					break;
				}
			}
			break;
		}
		case HISTORY_YESTERDAY:
		{
			BROWSER_LOGD("YESTERDAY");
			for (unsigned int i = 0 ; i < m_yesterday_history_list.size(); i++) {
				if (m_yesterday_history_list[i] == item) {
					m_yesterday_history_list.erase(m_yesterday_history_list.begin() + i);
					break;
				}
			}
			break;
		}
		case HISTORY_LAST_7_DAYS:
		{
			BROWSER_LOGD("LAST 7 DAYS");
			for (unsigned int i = 0 ; i < m_lastweek_history_list.size(); i++) {
				if (m_lastweek_history_list[i] == item) {
					m_lastweek_history_list.erase(m_lastweek_history_list.begin() + i);
					break;
				}
			}
			break;
		}
		case HISTORY_LAST_MONTH:
		{
				BROWSER_LOGD("LAST MONTH");
				for (unsigned int i = 0 ; i < m_lastmonth_history_list.size(); i++) {
					if (m_lastmonth_history_list[i] == item) {
						m_lastmonth_history_list.erase(m_lastmonth_history_list.begin() + i);
						break;
					}
				}
			break;
		}
		case HISTORY_OLDER:
		{
			BROWSER_LOGD("OLDER");
			for (unsigned int i = 0 ; i < m_older_history_list.size(); i++) {
				if (m_older_history_list[i] == item) {
					m_older_history_list.erase(m_older_history_list.begin() + i);
					break;
				}
			}
			break;
		}
		case HISTORY_NEXT_DAYS:
		{
			BROWSER_LOGD("ahead");
			for (unsigned int i = 0 ; i < m_ahead_history_list.size(); i++) {
				if (m_ahead_history_list[i] == item) {
					m_ahead_history_list.erase(m_ahead_history_list.begin() + i);
					break;
				}
			}
			break;
		}
		default:
			break;
	}
}

void history_view::_rebuild_list(void)
{
	BROWSER_LOGD("");
	if(m_genlist)
		elm_genlist_clear(m_genlist);

	for (unsigned int i = 0 ; i < m_history_list.size() ; i++)
		delete m_history_list[i];

	m_history_list.clear();
	m_today_history_list.clear();
	m_yesterday_history_list.clear();
	m_lastweek_history_list.clear();
	m_lastmonth_history_list.clear();
	m_older_history_list.clear();
	m_ahead_history_list.clear();
	m_today_parent = NULL;
	m_yesterday_parent = NULL;
	m_lastweek_parent = NULL;
	m_lastmonth_parent = NULL;
	m_older_parent = NULL;
	m_ahead_parent = NULL;

	m_history_list = m_browser->get_history()->get_history_list();

	if (m_history_list.size() == 0)
		_create_show_no_contents();
	else {
		for (unsigned int i = 0 ; i < m_history_list.size() ; i++) {
			Time_stamp_type time_stamp = m_history_list[i]->get_time_stamp_type();
			_populate_genlist(m_genlist, m_history_list[i], time_stamp);
		}
	}
}

void history_view::__system_time_changed_cb(system_settings_key_e key, void* data)
{
	if (SYSTEM_SETTINGS_KEY_TIME_CHANGED == key)
	{
		BROWSER_LOGD("System setting key ok!");
		history_view* hv = static_cast<history_view*>(data);
		if (hv)
		{
			std::set<int> ids;

			BROWSER_LOGD("Storing checked ids...");
			// Store checked ids...
			for(int i = 0 ; i < hv->m_history_list.size(); ++i)
			{
				if (hv->m_history_list[i]->get_is_checked_data())
				{
					ids.insert(hv->m_history_list[i]->get_id());
				}
			}

			BROWSER_LOGD("Rebuilding lists...");
			// Rebuild lists since system time has changed but
			// timestamps have remained the same
			hv->_rebuild_list();

			BROWSER_LOGD("Restoring checked ids...");
			// Restore checked ids...
			for(int i = 0 ; i < hv->m_history_list.size(); ++i)
			{
				if (ids.find(hv->m_history_list[i]->get_id()) != ids.end())
				{
					hv->m_history_list[i]->set_checked_data(EINA_TRUE);
				}
			}
		}
	}
	else
	{
		BROWSER_LOGD("Wrong system setting key!");
	}
}
