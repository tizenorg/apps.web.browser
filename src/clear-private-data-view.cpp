/*
*  browser
*
* Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
*
* Contact: Junghwan Kang <junghwan.kang@samsung.com>
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

#include "clear-private-data-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "certificate-manager.h"
#include "history.h"
#include "webview.h"
#include "settings.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif
#include "platform-service.h"

#define browser_popup_edj_path browser_edj_dir"/browser-popup.edj"

Ecore_Thread *clear_private_data_view::m_current_thread;
Evas_Object *clear_private_data_view::m_popup_processing;
Evas_Object *clear_private_data_view::m_progressbar;

clear_private_data_view::clear_private_data_view(void)
	: m_genlist(NULL)
	, m_clear_button(NULL)
	, m_item_ic(NULL)
	, m_select_all_flag(EINA_FALSE)
	, m_process_popup_timer(NULL)
	, m_clear_selected_idler(NULL)
	, m_rotate_ctxpopup_idler(NULL)
{
	BROWSER_LOGD("");
}

clear_private_data_view::~clear_private_data_view(void)
{
	BROWSER_LOGD("");

	if (m_item_ic)
		elm_genlist_item_class_free(m_item_ic);
	if (m_popup_processing) {
		evas_object_del(m_popup_processing);
		m_popup_processing = NULL;
	}
	if (m_progressbar)
		m_progressbar = NULL;

	m_select_all_flag = EINA_FALSE;

	if (m_process_popup_timer)
		ecore_timer_del(m_process_popup_timer);
	m_process_popup_timer = NULL;

	if (m_clear_selected_idler)
		ecore_idler_del(m_clear_selected_idler);
	if (m_rotate_ctxpopup_idler)
		ecore_idler_del(m_rotate_ctxpopup_idler);
	evas_object_smart_callback_del(m_naviframe, "title,clicked", __title_clicked_cb);
}

void clear_private_data_view::on_rotate(void)
{
	if (m_rotate_ctxpopup_idler)
		ecore_idler_del(m_rotate_ctxpopup_idler);
	m_rotate_ctxpopup_idler = ecore_idler_add(__resize_idler_cb, this);
}

Eina_Bool clear_private_data_view::__resize_idler_cb(void *data)
{

	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL, "data is NULL");

	clear_private_data_view *cpdv = (clear_private_data_view *)data;
	cpdv->m_rotate_ctxpopup_idler = NULL;
	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool clear_private_data_view::show(void)
{
	BROWSER_LOGD("");

	if (m_current_thread)
		return EINA_FALSE;
	Evas_Object *genlist = NULL;

	genlist = _create_genlist(m_naviframe);
	if (!genlist) {
		BROWSER_LOGE("failed to _create_genlist");
		return EINA_FALSE;
	}
	evas_object_show(genlist);
	m_genlist = genlist;
	Elm_Object_Item *naviframe_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_HEADER_PERSONAL_DATA_ABB", NULL, NULL, m_genlist, NULL);
	elm_object_item_domain_text_translatable_set(naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);

#if defined(HW_MORE_BACK_KEY)
		eext_object_event_callback_add(m_genlist, EEXT_CALLBACK_BACK, __cancel_button_cb, this);
#endif

	Evas_Object *btn_cancel = elm_button_add(m_naviframe);
	if (!btn_cancel) return EINA_FALSE;
	elm_object_style_set(btn_cancel, "naviframe/title_cancel");
	evas_object_smart_callback_add(btn_cancel, "clicked", __cancel_button_cb, this);
	elm_object_item_part_content_set(naviframe_item, "title_left_btn", btn_cancel);

	m_clear_button = elm_button_add(m_naviframe);
	if (!m_clear_button) return EINA_FALSE;
	elm_object_style_set(m_clear_button, "naviframe/title_done");
	evas_object_smart_callback_add(m_clear_button, "clicked", __clear_button_cb, this);
	elm_object_item_part_content_set(naviframe_item, "title_right_btn", m_clear_button);

	elm_object_disabled_set(m_clear_button, EINA_TRUE);
	return EINA_TRUE;
}

Evas_Object *clear_private_data_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "language,changed", common_view::__genlist_lang_changed, NULL);


	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	if (!item_ic) {
		BROWSER_LOGE("elm_genlist_item_class_new failed");
		return NULL;
	}

	item_ic->item_style = "1line";
	item_ic->func.text_get = __genlist_label_get_cb;
	item_ic->func.content_get = __genlist_icon_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;
	m_item_ic = item_ic;

	m_select_all.user_data = (void *)this;
	m_select_all.type = CELAR_PRIVATE_DATA_VIEW_SELECT_ALL;
	m_select_all.is_checked = EINA_FALSE;

	m_clear_history_data.user_data = (void *)this;
	m_clear_history_data.type = CELAR_PRIVATE_DATA_VIEW_CLEAR_HISTORY;
	m_clear_history_data.is_checked = EINA_FALSE;

	m_clear_cache_data.user_data = (void *)this;
	m_clear_cache_data.type = CELAR_PRIVATE_DATA_VIEW_CLEAR_CACHE;
	m_clear_cache_data.is_checked = EINA_FALSE;

	m_clear_cookie_data.user_data = (void *)this;
	m_clear_cookie_data.type = CELAR_PRIVATE_DATA_VIEW_CLEAR_COOKIE;
	m_clear_cookie_data.is_checked = EINA_FALSE;

	m_clear_saved_password_data.user_data = (void *)this;
	m_clear_saved_password_data.type = CELAR_PRIVATE_DATA_VIEW_CLEAR_SAVED_PASSWORD;
	m_clear_saved_password_data.is_checked = EINA_FALSE;

	m_clear_form_data.user_data = (void *)this;
	m_clear_form_data.type = CELAR_PRIVATE_DATA_VIEW_CLEAR_FORM_DATA;
	m_clear_form_data.is_checked = EINA_FALSE;

	m_select_all.it = elm_genlist_item_append(genlist, m_item_ic, &m_select_all, NULL, ELM_GENLIST_ITEM_NONE,
													__genlist_item_selected_cb, &m_select_all);
	m_clear_history_data.it = elm_genlist_item_append(genlist, m_item_ic, &m_clear_history_data, NULL, ELM_GENLIST_ITEM_NONE,
													__genlist_item_selected_cb, &m_clear_history_data);
	m_clear_cache_data.it = elm_genlist_item_append(genlist, m_item_ic, &m_clear_cache_data, NULL, ELM_GENLIST_ITEM_NONE,
													__genlist_item_selected_cb, &m_clear_cache_data);
	m_clear_cookie_data.it = elm_genlist_item_append(genlist, m_item_ic, &m_clear_cookie_data, NULL, ELM_GENLIST_ITEM_NONE,
													__genlist_item_selected_cb, &m_clear_cookie_data);
	m_clear_saved_password_data.it = elm_genlist_item_append(genlist, m_item_ic, &m_clear_saved_password_data, NULL, ELM_GENLIST_ITEM_NONE,
															__genlist_item_selected_cb, &m_clear_saved_password_data);
	m_clear_form_data.it = elm_genlist_item_append(genlist, m_item_ic, &m_clear_form_data, NULL, ELM_GENLIST_ITEM_NONE,
													__genlist_item_selected_cb, &m_clear_form_data);
	return genlist;
}

void clear_private_data_view::_delete_private_data(void)
{
	BROWSER_LOGD("");
	Eina_Bool local_list[MAX_CLEAR_DATA_ITEMS];
	for (int i = 0; i < MAX_CLEAR_DATA_ITEMS; i++)
		local_list[i] = private_data_list[i];
	if (local_list[CELAR_PRIVATE_DATA_VIEW_CLEAR_HISTORY] == EINA_TRUE)
		m_browser->get_history()->delete_all();
	if (local_list[CELAR_PRIVATE_DATA_VIEW_CLEAR_CACHE] == EINA_TRUE) {
		webview_context::instance()->clear_private_data();
		certificate_manager::_delete_all();
	}
	if (local_list[CELAR_PRIVATE_DATA_VIEW_CLEAR_COOKIE] == EINA_TRUE) {
		webview_context::instance()->clear_cookies();
	}
	if (local_list[CELAR_PRIVATE_DATA_VIEW_CLEAR_SAVED_PASSWORD] == EINA_TRUE) {
		webview_context::instance()->clear_saved_ID_and_PW();
#if defined(WEB_LOGIN)
		webview_context::instance()->remove_all_fingerprint_login_datas();
		privacy_edit *pr_edit = m_browser->get_settings()->get_privacy_edit();
		if (webview_context::instance()->get_all_refreshed_password_data_list() == 0)
			pr_edit->enable_password_setting_menu(EINA_FALSE);
		else
			pr_edit->enable_password_setting_menu(EINA_TRUE);
#endif
	}
	if (local_list[CELAR_PRIVATE_DATA_VIEW_CLEAR_FORM_DATA] == EINA_TRUE)
		webview_context::instance()->clear_form_data();
}

char *clear_private_data_view::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
	RETV_MSG_IF(!part, NULL, "part is NULL");

	if (!strcmp(part, "elm.text.main.left")) {
		genlist_callback_data *callback_data = (genlist_callback_data *)data;
		menu_type type = callback_data->type;

		switch (type) {
		case CELAR_PRIVATE_DATA_VIEW_SELECT_ALL:
			return strdup(BR_STRING_SELECT_ALL);
		case CELAR_PRIVATE_DATA_VIEW_CLEAR_HISTORY:
			return strdup(BR_STRING_HISTORY);
		case CELAR_PRIVATE_DATA_VIEW_CLEAR_CACHE:
			return strdup(BR_STRING_CACHE);
		case CELAR_PRIVATE_DATA_VIEW_CLEAR_COOKIE:
			return strdup(BR_STRING_SETTINGS_COOKIES_AND_DATA);
		case CELAR_PRIVATE_DATA_VIEW_CLEAR_SAVED_PASSWORD:
			return strdup(BR_STRING_PASSWORDS);
		case CELAR_PRIVATE_DATA_VIEW_CLEAR_FORM_DATA:
			return strdup(BR_STRING_FORMDATA);
		case CELAR_PRIVATE_DATA_VIEW_MENU_END:
		case CELAR_PRIVATE_DATA_VIEW_MENU_UNKNOWN:
		default:
			break;
		}
	}

	return NULL;
}

Evas_Object *clear_private_data_view::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
	RETV_MSG_IF(!part, NULL, "part is NULL");

	if (!strcmp(part, "elm.icon.right")) {
		genlist_callback_data *callback_data = (genlist_callback_data *)data;
		clear_private_data_view *cpdv = (clear_private_data_view *)callback_data->user_data;

		Evas_Object *checkbox = NULL;
		checkbox = elm_check_add(obj);
		if (!checkbox) {
			BROWSER_LOGE("Failed to add clear_history_check");
			return NULL;
		}
		elm_check_state_pointer_set(checkbox, &(callback_data->is_checked));
		if (callback_data->type == CELAR_PRIVATE_DATA_VIEW_SELECT_ALL)
			evas_object_smart_callback_add(checkbox, "changed", __select_all_icon_clicked_cb, cpdv);
		else
			evas_object_smart_callback_add(checkbox, "changed", __checkbox_changed_cb, cpdv);
		evas_object_propagate_events_set(checkbox, EINA_FALSE);
		callback_data->checkbox = checkbox;

		return checkbox;
	}

	return NULL;
}

void clear_private_data_view::__genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);

	Evas_Object *checkbox = NULL;
	checkbox = elm_object_item_part_content_get(item, "elm.icon.right");

	Eina_Bool state = elm_check_state_get(checkbox);
	elm_check_state_set(checkbox, !state);

	genlist_callback_data *gcd = (genlist_callback_data *)data;
	clear_private_data_view *cpdv = (clear_private_data_view *)(gcd->user_data);
	RET_MSG_IF(!cpdv, "cpdv is NULL");

	if (gcd->type == CELAR_PRIVATE_DATA_VIEW_SELECT_ALL)
		__select_all_icon_clicked_cb((void *)cpdv, NULL, NULL);
	else
		__checkbox_changed_cb((void *)cpdv, NULL, NULL);
}

void clear_private_data_view::__select_all_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	clear_private_data_view *cpdv = (clear_private_data_view *)data;
	Eina_Bool state = cpdv->m_select_all_flag = !cpdv->m_select_all_flag;

	Elm_Object_Item *it = elm_genlist_first_item_get(cpdv->m_genlist);
	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data && cb_data->is_checked != state) {
			cb_data->is_checked = state;
			elm_genlist_item_update(it);
		}
		it = elm_genlist_item_next_get(it);
	}

	if (state)
		elm_object_disabled_set(cpdv->m_clear_button, EINA_FALSE);
	else
		elm_object_disabled_set(cpdv->m_clear_button, EINA_TRUE);

}

void clear_private_data_view::__checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	clear_private_data_view *cpdv = (clear_private_data_view *)data;
	Eina_Bool all_selected = EINA_TRUE;
	Elm_Object_Item *it = elm_genlist_first_item_get(cpdv->m_genlist);
	Elm_Object_Item *it_select_all = it;
	it = elm_genlist_item_next_get(it);   //To avoid the first "select All" option.
	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data) {
			if (cb_data->is_checked == EINA_FALSE) {
				all_selected = EINA_FALSE;
				break;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it_select_all);
	if (cb_data) {
		cb_data->is_checked = all_selected;
		elm_genlist_item_update(it_select_all);
		cpdv->m_select_all_flag = all_selected;
	}

	/* Set delete button status */
	it = elm_genlist_first_item_get(cpdv->m_genlist);
	while (it) {
		Evas_Object *ck = elm_object_item_part_content_get(it, "elm.icon.right");
		if (ck) {
			if (elm_check_state_get(ck) == EINA_TRUE) {
				elm_object_disabled_set(cpdv->m_clear_button, EINA_FALSE);
				return;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	elm_object_disabled_set(cpdv->m_clear_button, EINA_TRUE);
}

void clear_private_data_view::__cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	clear_private_data_view *cp = (clear_private_data_view*)data;
	cp->_back_to_previous_view();
	cp->m_select_all_flag = EINA_FALSE;

	Evas_Object *genlist = m_browser->get_settings()->get_privacy_edit()->get_genlist();
	if (genlist)
		evas_object_freeze_events_set(genlist, EINA_FALSE);
}

void clear_private_data_view::__clear_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	clear_private_data_view *cpdv = (clear_private_data_view*)data;

	if(m_popup_processing)
		return;
	elm_object_disabled_set(cpdv->m_clear_button, EINA_TRUE);

	evas_object_freeze_events_set(m_browser->get_settings()->get_privacy_edit()->get_genlist(), EINA_FALSE);
	for (int i =0 ; i < MAX_CLEAR_DATA_ITEMS ; i++)
		cpdv->private_data_list[i] = EINA_FALSE;
	Elm_Object_Item *it = elm_genlist_first_item_get(cpdv->m_genlist);
	it = elm_genlist_item_next_get(it);   //To avoid the first "select All" option.

	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data && cb_data->is_checked == EINA_TRUE) {
			if (cb_data->type == CELAR_PRIVATE_DATA_VIEW_CLEAR_HISTORY)
				cpdv->private_data_list[CELAR_PRIVATE_DATA_VIEW_CLEAR_HISTORY] = EINA_TRUE;
			else if (cb_data->type == CELAR_PRIVATE_DATA_VIEW_CLEAR_CACHE)
				cpdv->private_data_list[CELAR_PRIVATE_DATA_VIEW_CLEAR_CACHE] = EINA_TRUE;
			else if (cb_data->type == CELAR_PRIVATE_DATA_VIEW_CLEAR_COOKIE)
				cpdv->private_data_list[CELAR_PRIVATE_DATA_VIEW_CLEAR_COOKIE] = EINA_TRUE;
			else if (cb_data->type == CELAR_PRIVATE_DATA_VIEW_CLEAR_SAVED_PASSWORD)
				cpdv->private_data_list[CELAR_PRIVATE_DATA_VIEW_CLEAR_SAVED_PASSWORD] = EINA_TRUE;
			else if (cb_data->type == CELAR_PRIVATE_DATA_VIEW_CLEAR_FORM_DATA)
				cpdv->private_data_list[CELAR_PRIVATE_DATA_VIEW_CLEAR_FORM_DATA] = EINA_TRUE;
		}
		it = elm_genlist_item_next_get(it);
	}

	if (cpdv->m_clear_selected_idler)
		ecore_idler_del(cpdv->m_clear_selected_idler);
	cpdv->m_clear_selected_idler = ecore_idler_add(__clear_selected_idler_cb, data);
}

void clear_private_data_view::__delete_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	clear_private_data_view *cpdv = (clear_private_data_view*)data;

	cpdv->m_select_all_flag = EINA_FALSE;
	if (!m_current_thread) {
		_show_processing_popup(cpdv);
		m_current_thread = ecore_thread_run(_thread_start_clear_private_data, __thread_end_cb, __thread_cancel_cb, cpdv);
	}
	elm_object_disabled_set(cpdv->m_clear_button, EINA_FALSE);
}

void clear_private_data_view::_show_processing_popup(void *data)
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

#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(m_popup_processing, EEXT_CALLBACK_BACK, __show_noti_popup_cb, data);
#endif
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

void clear_private_data_view::_close_processing_popup(void)
{
	BROWSER_LOGD("");
	if (m_popup_processing)
		evas_object_del(m_popup_processing);
	m_popup_processing = NULL;
	m_progressbar = NULL;
}

void clear_private_data_view::__show_noti_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	clear_private_data_view *cpdv = (clear_private_data_view*)data;
	cpdv->show_noti_popup(BR_STRING_PROCESSING);
}

void clear_private_data_view::__delete_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	clear_private_data_view *cpdv = (clear_private_data_view*)data;
	elm_object_disabled_set(cpdv->m_clear_button, EINA_FALSE);
}

void clear_private_data_view::__title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *naviframe_item = (Elm_Object_Item *)event_info;
	Evas_Object *label = elm_object_item_part_content_get(naviframe_item, "elm.swallow.title");
	RET_MSG_IF(!label, "label is NULL");

	elm_label_slide_go(label);
}

void clear_private_data_view::_thread_start_clear_private_data(void *data, Ecore_Thread *th)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	clear_private_data_view *cpdv = (clear_private_data_view*)data;
	cpdv->_delete_private_data();
}

void clear_private_data_view::__thread_cancel_cb(void *data, Ecore_Thread *th)
{
	BROWSER_LOGD("");

	_close_processing_popup();
	m_current_thread = NULL;
}

void clear_private_data_view::__thread_end_cb(void *data, Ecore_Thread *th)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	clear_private_data_view *view = (clear_private_data_view *)data;

	if (view->m_process_popup_timer)
		ecore_timer_del(view->m_process_popup_timer);

	view->m_process_popup_timer = ecore_timer_add(0.3, view->__timer_expired_cb, data);
}

Eina_Bool clear_private_data_view::__timer_expired_cb(void *data)
{
	BROWSER_LOGD("");

	_close_processing_popup();
	m_current_thread = NULL;

	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL, "data is NULL");
	clear_private_data_view *view = (clear_private_data_view *)data;

	view->m_process_popup_timer = NULL;
	view->_back_to_previous_view();
	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool clear_private_data_view::__clear_selected_idler_cb(void *data)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(data == NULL, ECORE_CALLBACK_CANCEL, "data is NULL");

	clear_private_data_view *view = (clear_private_data_view *)data;

	view->m_clear_selected_idler = NULL;

	m_browser->get_browser_view()->show_delete_popup("IDS_BR_SK_DELETE",
													"IDS_BR_BODY_THE_SELECTED_PERSONAL_DATA_WILL_BE_DELETED",
													view->__delete_cancel_cb,
													"IDS_BR_SK_CANCEL",
													view->__delete_cancel_cb,
													"IDS_BR_SK_DELETE",
													view->__delete_ok_cb,
													view);
	return ECORE_CALLBACK_CANCEL;
}

void clear_private_data_view::_back_to_previous_view(void)
{
	BROWSER_LOGD("");

	if (elm_naviframe_bottom_item_get(m_naviframe) != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop(m_naviframe);
}
