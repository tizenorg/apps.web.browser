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

#include "common-view.h"

#include <Ecore_X.h>
#include <Eina.h>
#include <Elementary.h>
#include <app_control.h>
#include <app_manager.h>
#include <app_control_internal.h>
#include <gio/gio.h>
#include <eina_list.h>
#include <fcntl.h>
#include <notification.h>
#include <string>
#include <ui-gadget.h>
#include <utilX.h>
#include <vconf.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "platform-service.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

Evas_Object *common_view::m_bg;
Evas_Object *common_view::m_naviframe;
Evas_Object *common_view::m_conformant;
Evas_Object *common_view::m_toast_popup;
int common_view::m_view_count;

#define CALLBACK_DATA_0	"cb0"
#define CALLBACK_DATA_1	"cb1"
#define CALLBACK_DATA_2	"cb2"
#define CALLBACK_DATA_3	"cb3"

#define bludtooth_template_html_path browser_res_dir"/template/template_bluetooth_content_share.html"
#define bludtooth_sending_html_path browser_data_dir"/bluetooth_content_share.html"
#define BLUETOOTH_SENDING_HTML_A_HREF_REPLACING_KEY "a_href_needed"
#define BLUETOOTH_SENDING_HTML_URI_REPLACING_KEY "uri_needed"

#define FONT_SIZE_SMALL_GENLIST_H 112
#define FONT_SIZE_NORMAL_GENLIST_H 96
#define FONT_SIZE_LARGE_GENLIST_H 112
#define FONT_SIZE_HUGE_GENLIST_H 112
#define FONT_SIZE_GIANT_GENLIST_H 136

#define ITEM_COUNT_FOR_VT 10
#define ITEM_COUNT_FOR_LN 4
#define ITEM_COUNT_FOR_VT_GIANT_FONT 6
#define ITEM_COUNT_FOR_LN_GIANT_FONT 3

#define WINDOW_TOP_MARGIN 63
#define WINDOW_BOTTOM_MARGIN 61
#define POPUP_TOP_SIZE 69
#define POPUP_BOTTOM_SIZE 69

struct popup_callback
{
public:
	popup_callback(Evas_Smart_Cb cb = NULL, void* fdata = NULL, void* udata = NULL) :
		func(cb),
		func_data(fdata),
		user_data(udata)
	{}

	~popup_callback(){}

	Evas_Smart_Cb func;
	void *func_data;
	void *user_data;

private:
	// Don't copy me
	popup_callback(const popup_callback&);
	popup_callback& operator=(const popup_callback&);
};

struct scheme_handling_popup_callback
{
public:
	scheme_handling_popup_callback(char *u = NULL, void* udata = NULL) :
		uri(u),
		user_data(udata)
	{}

	~scheme_handling_popup_callback(){}

	char *uri;
	void *user_data;

private:
	// Don't copy me
	scheme_handling_popup_callback(const scheme_handling_popup_callback&);
	scheme_handling_popup_callback& operator=(const scheme_handling_popup_callback&);
};

static void __ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	BROWSER_LOGD("");
	if (!priv || !ug)
		return;

	Evas_Object *base = (Evas_Object*)ug_get_layout(ug);
	if (!base)
		return;

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(base);
		break;
	default:
		break;
	}
}

static void __ug_destroy_cb(ui_gadget_h ug, void *priv)
{
	BROWSER_LOGD("");

	ug_destroy(ug);
}

static void _destroy_cbs(Evas_Object *popup)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!popup, "popup is NULL");

	popup_callback *cb0 = static_cast<popup_callback*>
		(evas_object_data_get(popup, CALLBACK_DATA_0));
	delete cb0;

	popup_callback *cb1 = static_cast<popup_callback*>
		(evas_object_data_get(popup, CALLBACK_DATA_1));
	delete cb1;

	popup_callback *cb2 = static_cast<popup_callback*>
		(evas_object_data_get(popup, CALLBACK_DATA_2));
	delete cb2;

	popup_callback *cb3 = static_cast<popup_callback*>
		(evas_object_data_get(popup, CALLBACK_DATA_3));
	delete cb3;

	evas_object_data_set(popup, CALLBACK_DATA_0, NULL);
	evas_object_data_set(popup, CALLBACK_DATA_1, NULL);
	evas_object_data_set(popup, CALLBACK_DATA_2, NULL);
	evas_object_data_set(popup, CALLBACK_DATA_3, NULL);
}

void common_view::destroy_popup(Evas_Object *sub_obj)
{
	BROWSER_LOGD("");

	_destroy_cbs(m_popup);

	if (m_popup) {
		evas_object_del(m_popup);
		m_popup = NULL;
	}
}

void common_view::destroy_msg_popup(void)
{
	BROWSER_LOGD("");

	_destroy_cbs(m_msg_popup);

	if(m_msg_popup) {
		evas_object_del(m_msg_popup);
		m_msg_popup = NULL;
	}
}

void common_view::destroy_delete_popup(void)
{
	BROWSER_LOGD("");

	_destroy_cbs(m_popup_confirm);

	if(m_popup_confirm) {
		evas_object_del(m_popup_confirm);
		m_popup_confirm = NULL;
	}
}

void common_view::destroy_content_popup(void)
{
	BROWSER_LOGD("");

	_destroy_cbs(m_content_popup);

	if(m_content_popup) {
		evas_object_del(m_content_popup);
		m_content_popup = NULL;
	}
}

common_view::common_view(void)
:
	m_share_uri(NULL)
	, m_popup(NULL)
	, m_msg_popup(NULL)
	, m_popup_confirm(NULL)
	, m_content_popup(NULL)
	, m_context_popup(NULL)
	, m_popup_list(NULL)
	, m_box_of_popup_content(NULL)
	, m_genlist_of_popup_content(NULL)
	, m_app_in_app_popup_window(NULL)
	, m_popup_processing(NULL)
{
	BROWSER_LOGD("");
	m_view_count++;
	if (m_view_count > 1)
		get_elm_bg();
	m_browser->register_common_view(this);
}

common_view::~common_view(void)
{
	BROWSER_LOGD("");

	m_browser->unregister_common_view(this);
	m_view_count--;

	eina_stringshare_del(m_share_uri);

	if (m_popup)
		evas_object_del(m_popup);
	if (m_popup_processing)
		evas_object_del(m_popup_processing);

	clear_popups();
}

Evas_Object *common_view::get_elm_bg(void)
{
	RETV_MSG_IF(!m_window, NULL, "m_window is NULL");

	if (!m_bg) {
		BROWSER_LOGD("create elm bg");
		m_bg = elm_bg_add(m_window);
		if (!m_bg)
			return NULL;

		evas_object_size_hint_weight_set(m_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(m_window, m_bg);
		evas_object_lower(m_bg);
		evas_object_show(m_bg);
	}

	return m_bg;
}

void common_view::on_pause(void)
{
	BROWSER_LOGD("");
	hide_common_view_popups();
}

void common_view::on_rotate(int degree)
{
	BROWSER_LOGD("");
	if (m_box_of_popup_content) {
		int popup_height = genlist_popup_calculate_height(m_genlist_of_popup_content);
		evas_object_size_hint_min_set(m_box_of_popup_content, 0, popup_height);
	}
}

void common_view::__button1_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_popup(obj);
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

void common_view::__button2_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_popup(obj);
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

void common_view::__button3_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_popup(obj);
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

void common_view::__msg_button1_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_msg_popup();
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

void common_view::__msg_button2_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_msg_popup();
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

void common_view::__msg_button3_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_msg_popup();
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

void common_view::__delete_button1_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_delete_popup();
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

void common_view::__delete_button2_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_delete_popup();
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

void common_view::__content_button1_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_content_popup();
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

void common_view::__content_button2_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_content_popup();
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

#if defined(HW_MORE_BACK_KEY)
void common_view::__popup_hw_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;
	RET_MSG_IF(!cv, "common_view is NULL");

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	_destroy_cbs(obj);

	evas_object_del(obj);

	if (cv->m_popup) {
		evas_object_del(cv->m_popup);
		cv->m_popup = NULL;
	}
	cv->m_box_of_popup_content = NULL;
	cv->m_genlist_of_popup_content = NULL;
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}
#endif

#if defined(HW_MORE_BACK_KEY)
void common_view::__popup_timeout_hw_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;
	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	_destroy_cbs(obj);
	evas_object_del(obj);
	if(m_toast_popup){
		evas_object_del(m_toast_popup);
		m_toast_popup = NULL;

	}
	if(cv->m_popup)
		cv->m_popup = NULL;

	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}
#endif

int common_view::genlist_popup_calculate_height(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	int height = 0;
	int item_h = 0;
	double scale = elm_config_scale_get() / elm_app_base_scale_get();

	unsigned int genlist_item_count = 0;
	if (genlist)
		genlist_item_count = elm_genlist_items_count(genlist);

	int rotate_angle = elm_win_rotation_get(m_window);

	if (0 == rotate_angle || 180 == rotate_angle) {
		item_h = (int)(FONT_SIZE_NORMAL_GENLIST_H * scale);
		height = item_h * ((genlist_item_count > ITEM_COUNT_FOR_VT) ? (ITEM_COUNT_FOR_VT + 0.5) : genlist_item_count);
	} 
	else {
		/* landscape mode*/
		item_h = (int)(FONT_SIZE_NORMAL_GENLIST_H * scale);
		height = item_h * ((genlist_item_count > ITEM_COUNT_FOR_LN) ? (ITEM_COUNT_FOR_LN + 0.5) : genlist_item_count);

	}
	height = height * scale;

	return height;
}

int common_view::genlist_popup_calculate_height_with_margin(Evas_Object *genlist, popup_shape shape)
{
	BROWSER_LOGD("");
	int height = 0;
	int item_h = 0;
	double scale = elm_config_scale_get() / elm_app_base_scale_get();

	unsigned int genlist_item_count = 0;
	if (genlist)
		genlist_item_count = elm_genlist_items_count(genlist);

	int x,y,w,h = 0;
	evas_object_geometry_get(m_window, &x, &y, &w, &h);

	unsigned int max_height = 0;

	if (shape == popup_with_only_top)
		max_height = h - WINDOW_TOP_MARGIN - WINDOW_BOTTOM_MARGIN - POPUP_TOP_SIZE;
	else if (shape == popup_with_only_bottom)
		max_height = h - WINDOW_TOP_MARGIN - WINDOW_BOTTOM_MARGIN - POPUP_BOTTOM_SIZE;
	else if (shape == popup_with_top_bottom)
		max_height = h - WINDOW_TOP_MARGIN - WINDOW_BOTTOM_MARGIN - POPUP_TOP_SIZE - POPUP_BOTTOM_SIZE;

	item_h = (int)(FONT_SIZE_NORMAL_GENLIST_H);
	height =  ((max_height > item_h * genlist_item_count ) ? item_h *genlist_item_count : max_height);
	height = height * scale;

	return height;
}

void common_view::share(const char *uri, const char *title)
{
	BROWSER_SECURE_LOGD("uri=[%s], title=[%s]", uri, title);
	RET_MSG_IF(!uri, "uri is NULL");

	eina_stringshare_replace(&m_share_uri, uri);

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return;
	}

	if (app_control_set_operation(app_control, "http://tizen.org/appcontrol/operation/share_text") < 0) {
		BROWSER_LOGE("Fail to set app_control operation");
		app_control_destroy(app_control);
		return;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(m_window);
	if (app_control_set_window(app_control, win_id) < 0) {
		BROWSER_LOGE("Fail to app_control_set_window");
		app_control_destroy(app_control);
		return;
	}

	if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_TEXT, uri) < 0) {
		BROWSER_LOGE("Fail to set extra data : APP_CONTROL_DATA_TEXT");
		app_control_destroy(app_control);
		return;
	}

	if (title && strlen(title) > 0) {
		if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_SUBJECT, title) < 0) {
			BROWSER_LOGE("Fail to set extra data : APP_CONTROL_DATA_SUBJECT");
			app_control_destroy(app_control);
			return;
		}
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return;
	}

	app_control_destroy(app_control);
}

Evas_Object *common_view::show_content_popup(Evas_Object *popup, const char *title_text_id, Evas_Object *content_obj,
											Evas_Smart_Cb popup_destroy_func,
											const char *button1_text_id, Evas_Smart_Cb button1_func,
											const char *button2_text_id, Evas_Smart_Cb button2_func,
											void *data, Eina_Bool share)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!popup, NULL, "popup is NULL");
	RETV_MSG_IF(!content_obj, NULL, "content_obj is NULL");

	m_content_popup = popup;
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title_text_id)
		set_trans_text_to_object(popup, title_text_id, "title,text");
	evas_object_event_callback_add(popup,  EVAS_CALLBACK_FREE, __content_popup_free_cb, this);

	if (m_browser->is_tts_enabled())
		elm_object_tree_focus_allow_set(m_naviframe, EINA_FALSE);
#if defined(HW_MORE_BACK_KEY)
	popup_callback *back_key_cb_data = new(std::nothrow)
		popup_callback(popup_destroy_func, data, this);
	if (!back_key_cb_data)
		return NULL;

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_hw_back_cb, back_key_cb_data);
	evas_object_data_set(popup, CALLBACK_DATA_0, back_key_cb_data);
#endif
	if (!button1_text_id && !button1_func && !button2_text_id && !button2_func)
		return NULL;

	Evas_Object *button1 = elm_button_add(popup);
	if (!button1) {
		BROWSER_LOGE("elm_button_add failed");
#if defined(HW_MORE_BACK_KEY)
		delete back_key_cb_data;
#endif
		return NULL;
	}

	if (button1_text_id)
		set_trans_text_to_object(button1, button1_text_id, NULL);
	else
		set_trans_text_to_object(button1, "IDS_BR_SK_OK", NULL);

	elm_object_style_set(button1, "popup");
	elm_object_part_content_set(popup, "button1", button1);
	if (!m_browser->is_tts_enabled())
		elm_object_focus_set(button1, EINA_TRUE);

	popup_callback *cb1 = new(std::nothrow)
		popup_callback(button1_func, data, this);
	if (!cb1) {
#if defined(HW_MORE_BACK_KEY)
		delete back_key_cb_data;
#endif
		return NULL;
	}

	evas_object_smart_callback_add(button1, "clicked", __content_button1_cb, cb1);
	evas_object_show(button1);

	evas_object_data_set(popup, CALLBACK_DATA_1, cb1);

	if (button2_text_id) {
		Evas_Object *button2 = elm_button_add(popup);
		if (!button2) {
			BROWSER_LOGE("elm_button_add failed");
#if defined(HW_MORE_BACK_KEY)
			delete back_key_cb_data;
#endif
			delete cb1;
			return NULL;
		}
		set_trans_text_to_object(button2, button2_text_id, NULL);
		elm_object_style_set(button2, "popup");
		elm_object_part_content_set(popup, "button2", button2);

		popup_callback *cb2 = new(std::nothrow)
			popup_callback(button2_func, data, this);
		if (!cb2) {
#if defined(HW_MORE_BACK_KEY)
			delete back_key_cb_data;
#endif
			delete cb1;
			return NULL;
		}

		evas_object_smart_callback_add(button2, "clicked", __content_button2_cb, cb2);
		evas_object_show(button2);

		evas_object_data_set(popup, CALLBACK_DATA_2, cb2);
	}
	evas_object_show(popup);
	return popup;
}

Evas_Object *common_view::show_msg_popup(const char *title_text_id, const char *msg_text_id,
											Evas_Smart_Cb popup_destroy_func,
											const char *button1_text_id, Evas_Smart_Cb button1_func,
											const char *button2_text_id, Evas_Smart_Cb button2_func,
											void *data,
											const char *button3_text_id, Evas_Smart_Cb button3_func)
{
	BROWSER_LOGD("title = [%s], msg = [%s]", title_text_id, msg_text_id);
	RETV_MSG_IF(!msg_text_id,NULL, "msg_text_id is NULL");

	Evas_Object *popup;

	/* popup */
	popup = brui_popup_add(m_window);
	m_msg_popup = popup;
	evas_object_event_callback_add(popup,  EVAS_CALLBACK_FREE, __msg_popup_free_cb, this);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	set_trans_text_to_object(popup, msg_text_id, NULL);
	if (title_text_id)
		set_trans_text_to_object(popup, title_text_id, "title,text");

#if defined(HW_MORE_BACK_KEY)
	popup_callback *back_key_cb_data = new(std::nothrow)
		popup_callback(popup_destroy_func, data, this);
	if (!back_key_cb_data)
		return NULL;

	BROWSER_LOGD("popup[%p]", popup);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_hw_back_cb, back_key_cb_data);
	evas_object_data_set(popup, CALLBACK_DATA_0, back_key_cb_data);
#endif

	Evas_Object *button1 = elm_button_add(popup);
	if (!button1) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}

	if (button1_text_id)
		set_trans_text_to_object(button1, button1_text_id, NULL);
	else
		set_trans_text_to_object(button1, "IDS_BR_SK_OK", NULL);
	elm_object_style_set(button1, "popup");
	elm_object_part_content_set(popup, "button1", button1);
	if (!m_browser->is_tts_enabled())
		elm_object_focus_set(button1, EINA_TRUE);

	popup_callback *cb1 = new(std::nothrow)
		popup_callback(button1_func, data, this);
	if (!cb1) {
#if defined(HW_MORE_BACK_KEY)
		delete back_key_cb_data;
#endif
		return NULL;
	}

	evas_object_smart_callback_add(button1, "clicked", __msg_button1_cb, cb1);
	evas_object_show(button1);

	evas_object_data_set(popup, CALLBACK_DATA_1, cb1);

	popup_callback *cb2 = NULL;
	if (button2_text_id) {
		Evas_Object *button2 = elm_button_add(popup);
		if (!button2) {
			BROWSER_LOGE("elm_button_add failed");
#if defined(HW_MORE_BACK_KEY)
			delete back_key_cb_data;
#endif
			delete cb1;
			return NULL;
		}
		set_trans_text_to_object(button2, button2_text_id, NULL);
		elm_object_style_set(button2, "popup");
		elm_object_part_content_set(popup, "button2", button2);

		cb2 = new(std::nothrow) popup_callback(button2_func, data, this);
		if (!cb2)
			return NULL;

		evas_object_smart_callback_add(button2, "clicked", __msg_button2_cb, cb2);
		evas_object_show(button2);

		evas_object_data_set(popup, CALLBACK_DATA_2, cb2);
	}
	if (button3_text_id) {
		Evas_Object *button3 = elm_button_add(popup);
		if (!button3) {
			BROWSER_LOGE("elm_button_add failed");
#if defined(HW_MORE_BACK_KEY)
			delete back_key_cb_data;
#endif
			delete cb1;
			delete cb2;
			return NULL;
		}
		set_trans_text_to_object(button3, button3_text_id, NULL);
		elm_object_style_set(button3, "popup");
		elm_object_part_content_set(popup, "button3", button3);

		popup_callback *cb3 = new(std::nothrow)
			popup_callback(button3_func, data, this);
		if (!cb3)
			return NULL;

		evas_object_smart_callback_add(button3, "clicked", __msg_button3_cb, cb3);
		evas_object_show(button3);

		evas_object_data_set(popup, CALLBACK_DATA_3, cb3);
	}
	evas_object_show(popup);
	return popup;
}

Elm_Object_Item *common_view::add_item_to_popup(Evas_Object *popup,
			const char *label_id, Evas_Object *icon, Evas_Smart_Cb func, const void *data)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!popup, NULL, "popup is NULL");
	RETV_MSG_IF(!label_id, NULL, "label_id is NULL");

	Elm_Object_Item *it = NULL;
	it = elm_popup_item_append(popup, NULL, icon, func, data);
	if (it) {
		set_trans_text_to_object_item(it, label_id, NULL);
		return it;
	}
	return NULL;
}

void common_view::show_msg_popup(const char *msg, int timeout, Evas_Smart_Cb func, void *data, Eina_Bool has_focus)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!msg, "msg is NULL");
	Evas_Object *popup = brui_popup_add(m_window);
	RET_MSG_IF(!popup, "popup is NULL");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, (std::string("<align=center>") + msg + "</align>").c_str());

#if defined(HW_MORE_BACK_KEY)
	popup_callback *back_key_cb_data = new(std::nothrow)
		popup_callback(func, data, this);
	if (!back_key_cb_data)
		return;

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_timeout_hw_back_cb, back_key_cb_data);
	evas_object_data_set(popup, CALLBACK_DATA_0, back_key_cb_data);
#endif

	elm_popup_timeout_set(popup, timeout);

	evas_object_smart_callback_add(popup, "timeout", __popup_timeout_hw_back_cb, back_key_cb_data);

	evas_object_show(popup);
	m_popup = popup;
}

void common_view::show_noti_popup(const char *msg)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!msg, "msg is NULL");

	notification_status_message_post(msg);
	elm_access_say(msg);
}

void common_view::show_delete_popup(const char *title_text_id, const char *msg_text_id,
										Evas_Smart_Cb popup_destroy_func,
										const char *button1_text_id, Evas_Smart_Cb button1_func,
										const char *button2_text_id, Evas_Smart_Cb button2_func,
										void *data)
{
	BROWSER_LOGD("title = [%s], msg = [%s]", title_text_id, msg_text_id);
	RET_MSG_IF(!msg_text_id, "msg_text_id is NULL");

	Evas_Object *popup;
	/* popup */
	popup = brui_popup_add(m_window);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_event_callback_add(popup,  EVAS_CALLBACK_FREE, __popup_free_cb, this);
	if (msg_text_id)
		set_trans_text_to_object(popup, msg_text_id, NULL);
	if (title_text_id)
		set_trans_text_to_object(popup, title_text_id, "title,text");

	m_popup_confirm = popup;

#if defined(HW_MORE_BACK_KEY)
	popup_callback *back_key_cb_data = new(std::nothrow)
		popup_callback(popup_destroy_func, data, this);
	if (!back_key_cb_data)
		return;
	BROWSER_LOGD("popup[%p]", popup);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __popup_hw_back_cb, back_key_cb_data);
	evas_object_data_set(popup, CALLBACK_DATA_0, back_key_cb_data);
#endif

	Evas_Object *button1 = elm_button_add(popup);
	if (!button1) {
		BROWSER_LOGE("elm_button_add failed");
#if defined(HW_MORE_BACK_KEY)
		delete back_key_cb_data;
#endif
		return;
	}

	if (button1_text_id)
		set_trans_text_to_object(button1, button1_text_id, NULL);
	else
		set_trans_text_to_object(button1, "IDS_BR_SK_OK", NULL);
	elm_object_style_set(button1, "popup");
	elm_object_part_content_set(popup, "button1", button1);
	if (!m_browser->is_tts_enabled())
		elm_object_focus_set(button1, EINA_TRUE);

	popup_callback *cb1 = new(std::nothrow)
		popup_callback(button1_func, data, this);
	if (!cb1) {
#if defined(HW_MORE_BACK_KEY)
		delete back_key_cb_data;
#endif
		return;
	}

	evas_object_smart_callback_add(button1, "clicked", __delete_button1_cb, cb1);
	evas_object_show(button1);

	evas_object_data_set(popup, CALLBACK_DATA_1, cb1);

	if (button2_text_id) {
		Evas_Object *button2 = elm_button_add(popup);
		if (!button2) {
			BROWSER_LOGE("elm_button_add failed");
#if defined(HW_MORE_BACK_KEY)
			delete back_key_cb_data;
#endif
			delete cb1;
			return;
		}
		set_trans_text_to_object(button2, button2_text_id, NULL);
		//elm_object_style_set(button2, "sweep/delete");
		elm_object_style_set(button2, "popup");
		elm_object_part_content_set(popup, "button2", button2);

		popup_callback *cb2 = new(std::nothrow)
			popup_callback(button2_func, data, this);
		if (!cb2) {
#if defined(HW_MORE_BACK_KEY)
			delete back_key_cb_data;
#endif
			delete cb1;
			return;
		}

		evas_object_smart_callback_add(button2, "clicked", __delete_button2_cb, cb2);
		evas_object_show(button2);

		evas_object_data_set(popup, CALLBACK_DATA_2, cb2);
	}
	evas_object_show(popup);
}

Eina_Bool common_view::launch_tizenstore(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (!app_control) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) < 0) {
		BROWSER_LOGE("Fail to set app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_uri(app_control, uri) < 0) {
		BROWSER_LOGE("Fail to set uri operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_app_id(app_control, tizen_store) < 0) {
		BROWSER_LOGE("Fail to app_control_set_app_id");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_streaming_player(const char *uri, const char *cookie)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	bool vt_call_check = false;
	if (app_manager_is_running(sec_vt_app, &vt_call_check) < 0) {
		BROWSER_LOGE("Fail to get app running information");
		return EINA_FALSE;
	}

	if (vt_call_check) {
		show_msg_popup(NULL, BR_STRING_WARNING_VIDEO_PLAYER);
		return EINA_FALSE;
	}

	app_control_h app_control = NULL;

	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (!app_control) {
		BROWSER_LOGE("app_control handle is NULL");
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) < 0) {
		BROWSER_LOGE("app_control_set_operation failed");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_mime(app_control, "video/") < 0) {
		BROWSER_LOGE("Fail to app_control_set_mime");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_add_extra_data(app_control, "path", uri) < 0) {
		BROWSER_LOGE("Fail to set extra data");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (cookie) {
		if (app_control_add_extra_data(app_control, "cookie", cookie) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_audio_player(const char *uri, const char *cookie)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	app_control_h app_control = NULL;

	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (!app_control) {
		BROWSER_LOGE("app_control handle is NULL");
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) < 0) {
		BROWSER_LOGE("app_control_set_operation failed");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_mime(app_control, "audio/") < 0) {
		BROWSER_LOGE("Fail to app_control_set_mime");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (cookie) {
		if (app_control_add_extra_data(app_control, "cookie", cookie) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_message(const char *uri, const char *receiver, Eina_Bool file_attach)
{
	BROWSER_SECURE_LOGD("uri = [%s], receiver = [%s]", uri, receiver);

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_COMPOSE) < 0) {
		BROWSER_LOGE("Fail to set app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (uri && strlen(uri) > 0) {
		if (app_control_set_uri(app_control, uri) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}

	if (receiver && strlen(receiver)) {
		if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_TO, receiver) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_bluetooth(const char *file_path)
{
	BROWSER_SECURE_LOGD("file_path = [%s]", file_path);
	RETV_MSG_IF(!file_path, EINA_FALSE, "file_path is NULL");

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_SEND) < 0) {
		BROWSER_LOGE("Fail to set app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_uri(app_control, file_path) < 0) {
		BROWSER_LOGE("Fail to set app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_app_id(app_control, sec_bluetooth_app) < 0) {
		BROWSER_LOGE("Fail to app_control_set_app_id");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(m_window);
	if (app_control_set_window(app_control, win_id) < 0) {
		BROWSER_LOGE("Fail to app_control_set_window");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_contact(const char *phone_number, const char *email_address)
{
	BROWSER_LOGD("phone_number = [%s]", phone_number);

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(m_window);
	if (app_control_set_window(app_control, win_id) < 0) {
		BROWSER_LOGE("Fail to app_control_set_window");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	if (app_control_set_operation(app_control, "http://tizen.org/appcontrol/operation/add") < 0) {
		BROWSER_LOGE("Fail to app_control_set_operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	if (app_control_set_mime(app_control, "application/vnd.tizen.contact") < 0) {
		BROWSER_LOGE("Fail to app_control_set_mime");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (phone_number && strlen(phone_number)) {
		if (app_control_add_extra_data(app_control, "http://tizen.org/appcontrol/data/phone", phone_number) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}
	if (email_address && strlen(email_address)) {
		if (app_control_add_extra_data(app_control, "http://tizen.org/appcontrol/data/email", email_address) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}
	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_dialer(const char *phone_number)
{
	BROWSER_LOGD("phone_number = [%s]", phone_number);
	RETV_MSG_IF(!phone_number, EINA_FALSE, "phone_number is NULL");

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(m_window);
	if (app_control_set_window(app_control, win_id) < 0) {
		BROWSER_LOGE("Fail to app_control_set_window");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_DIAL) < 0) {
		BROWSER_LOGE("Fail to app_control_set_operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	char *decoded_number = g_uri_unescape_string(phone_number, NULL);
	if (app_control_set_uri(app_control, decoded_number) < 0) {
		BROWSER_LOGE("app_control_add_extra_data is failed.");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	free(decoded_number);

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	return EINA_TRUE;
}


Eina_Bool common_view::launch_nfc(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_SEND_TEXT) < 0) {
		BROWSER_LOGE("Fail to set app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_add_extra_data(app_control, APP_CONTROL_DATA_TEXT, uri) < 0) {
		BROWSER_LOGE("Fail to add extra data for [uri]");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_app_id(app_control, sec_nfc_app) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_Snote(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	struct ug_cbs cbs = {0, };
	cbs.layout_cb = __ug_layout_cb;
	cbs.result_cb = NULL;
	cbs.destroy_cb = __ug_destroy_cb;
	cbs.priv = (void *)this;

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (app_control_add_extra_data(app_control, "type", "insert") < 0) {
		BROWSER_LOGE("Fail to add extra data for [type]");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_add_extra_data(app_control, "text", uri) < 0) {
		BROWSER_LOGE("Fail to add extra data for [text]");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (!ug_create(NULL, sec_snote_ug, UG_MODE_FULLVIEW, app_control, &cbs))
		BROWSER_LOGE("ug_create is failed.");

	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_phone_call(const char *number, Eina_Bool is_vt_call)
{
	BROWSER_SECURE_LOGD("uri = [%s]", number);
	RETV_MSG_IF(!number, EINA_FALSE, "number is NULL");

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (is_vt_call) {
		if (app_control_add_extra_data(app_control, "KEY_CALL_TYPE", "MO") < 0) {
			BROWSER_LOGE("Fail to set extra data : KEY_CALL_TYPE");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		if (app_control_add_extra_data(app_control, "number", number) < 0) {
			BROWSER_LOGE("Fail to set extra data : number");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		if (app_control_set_app_id(app_control, sec_vt_app) < 0) {
			BROWSER_LOGE("Fail to set package");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	} else {
		if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_CALL) < 0) {
			BROWSER_LOGE("Fail to set app_control operation");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		std::string request_number = std::string(tel_scheme) + std::string(number);

		if (app_control_set_uri(app_control, request_number.c_str()) < 0) {
			BROWSER_LOGE("Fail to set uri");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_cms_svc(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (strlen(uri)) {
		if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) < 0) {
			BROWSER_LOGE("Fail to app_control_set_mime");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		if (app_control_set_mime(app_control, cms_mime_type) < 0) {
			BROWSER_LOGE("Fail to app_control_set_mime");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		if (app_control_set_uri(app_control, uri) < 0) {
			BROWSER_LOGE("Fail to app_control_set_uri");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		if (app_control_set_app_id(app_control, "3vtmFKbnKM.CmsSvc") < 0) {
			BROWSER_LOGE("Fail to app_control_set_app_id");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
			BROWSER_LOGE("Fail to launch app_control operation");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}

	app_control_destroy(app_control);

	show_noti_popup(BR_STRING_DOWNLOADING_ING);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_sdp_svc(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (strlen(uri)) {
		if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) < 0) {
			BROWSER_LOGE("Fail to app_control_set_mime");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		if (app_control_set_mime(app_control, "application/sdp") < 0) {
			BROWSER_LOGE("Fail to app_control_set_mime");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		if (app_control_set_uri(app_control, uri) < 0) {
			BROWSER_LOGE("Fail to app_control_set_uri");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		if (app_control_set_app_id(app_control, sec_streaming_player) < 0) {
			BROWSER_LOGE("Fail to app_control_set_app_id");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}

		if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
			BROWSER_LOGE("Fail to launch app_control operation");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}

	app_control_destroy(app_control);

	return EINA_TRUE;
}

void common_view::hide_common_view_popups(void)
{
	BROWSER_LOGD("");
	/* Need to re-design following popup policy to keep popups from resume/pause+ing */
	/* So far, in now, it's best to block here if it's fixed from function caller side */
	return;
}

void common_view::clear_popups(void)
{
	BROWSER_LOGD("");
	if (m_popup_confirm) {
		evas_object_del(m_popup_confirm);
		m_popup_confirm = NULL;
	}
	if (m_msg_popup) {
		evas_object_del(m_msg_popup);
		m_msg_popup = NULL;
	}
	if (m_popup_list) {
		evas_object_del(m_popup_list);
		m_popup_list = NULL;
	}
	if (m_content_popup) {
		evas_object_del(m_content_popup);
		m_content_popup = NULL;
	}
	if (m_context_popup) {
		evas_object_del(m_context_popup);
		m_context_popup = NULL;
	}
}

void common_view::show_notification(const char* data, Evas_Object *obj, unsigned int length)
{
	RET_MSG_IF(!data, "data is NULL");
	if (strlen(data) >= length) {
		BROWSER_SECURE_LOGD("Max Length Reached = %d", strlen(data));
		char *text = NULL;
		const char *fmt = BR_STRING_MAXIMUM_CHARACTER_WARNING;

		if (-1 == asprintf(&text, fmt, length)) return;

		this->show_noti_popup(text);
		elm_access_say(text);
		if (obj)
			elm_entry_cursor_end_set(obj);
		free(text);
	}
}

void common_view::show_processing_popup(void)
{
	BROWSER_LOGD("");

	if (m_popup_processing)
		evas_object_del(m_popup_processing);

	m_popup_processing = brui_popup_add(m_naviframe);
	RET_MSG_IF(!m_popup_processing, "m_popup_processing is NULL");

	Evas_Object *pgbar_outer_layout = elm_layout_add(m_popup_processing);
	elm_layout_file_set(pgbar_outer_layout, browser_edj_dir"/browser-popup.edj", "simple_message_popup");
	evas_object_size_hint_weight_set(pgbar_outer_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *progressbar = elm_progressbar_add(m_popup_processing);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	elm_object_style_set(progressbar, "pending_list");

	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progressbar);

	elm_object_part_content_set(pgbar_outer_layout, "elm.swallow.content", progressbar);
	elm_object_content_set(m_popup_processing, pgbar_outer_layout);
	evas_object_show(m_popup_processing);
}

void common_view::close_processing_popup(void)
{
	BROWSER_LOGD("");

	if (m_popup_processing) {
		evas_object_del(m_popup_processing);
		m_popup_processing = NULL;
	}
}

void common_view::__popup_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	common_view *cp = (common_view *)data;
	cp->m_popup_confirm = NULL;
}

void common_view::__msg_popup_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	common_view *cp = (common_view *)data;
	cp->m_msg_popup = NULL;
}


void common_view::__list_popup_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	common_view *cp = (common_view *)data;
	cp->m_popup_list = NULL;
}

void common_view::__content_popup_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	common_view *cp = (common_view *)data;
	cp->m_content_popup = NULL;
}

void common_view::__call_response_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);
	common_view *cv = (common_view *)cb->func_data;
	char *number = (char *)cb->user_data;

	int offset = 0;
	Eina_Bool vt_call = EINA_FALSE;
	if (!strncmp(number, tel_scheme, strlen(tel_scheme)))
		offset = strlen(tel_scheme);
	else if (!strncmp(number, vtel_scheme, strlen(vtel_scheme))) {
		offset = strlen(vtel_scheme);
		vt_call = EINA_TRUE;
	} else if (!strncmp(number, telto_scheme, strlen(telto_scheme)))
		offset = strlen(telto_scheme);
	else if (!strncmp(number, callto_scheme, strlen(callto_scheme)))
		offset = strlen(callto_scheme);
	else if (!strncmp(number, wtai_wp_mc_scheme, strlen(wtai_wp_mc_scheme)))
		offset = strlen(wtai_wp_mc_scheme);
	else
		offset = strlen(wtai_wp_sd_scheme);

	cv->launch_phone_call(number + offset, vt_call);

	free(cb->user_data);
	delete cb;
}

void common_view::__call_response_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = static_cast<popup_callback*>(data);

	free(cb->user_data);
	delete cb;
}

void common_view::__genlist_lang_changed(void *data, Evas_Object * obj, void *event_info)
{
	elm_genlist_realized_items_update(obj);
}

void common_view::_show_tel_vtel_popup(const char *number)
{
	BROWSER_LOGD("number = [%s]", number);
	RET_MSG_IF(!number, "number is NULL");

	int offset = 0;
	if (!strncmp(number, tel_scheme, strlen(tel_scheme)))
		offset = strlen(tel_scheme);
	else if (!strncmp(number, vtel_scheme, strlen(vtel_scheme)))
		offset = strlen(vtel_scheme);
	else if (!strncmp(number, telto_scheme, strlen(telto_scheme)))
		offset = strlen(telto_scheme);
	else if (!strncmp(number, callto_scheme, strlen(callto_scheme)))
		offset = strlen(callto_scheme);
	else if (!strncmp(number, wtai_wp_mc_scheme, strlen(wtai_wp_mc_scheme)))
		offset = strlen(wtai_wp_mc_scheme);
	else
		offset = strlen(wtai_wp_sd_scheme);

	std::string msg = std::string(BR_STRING_CALL) + std::string(number + offset) + std::string("?");

	popup_callback *cb = new(std::nothrow)
		popup_callback(NULL, this, strdup(number));
	if (!cb)
		return;

	show_msg_popup(NULL, msg.c_str(), __call_response_cancel_cb, "IDS_BR_SK_CANCEL", __call_response_cancel_cb, "IDS_BR_SK_OK", __call_response_ok_cb, cb);
}

Eina_Bool common_view::_handle_intent_scheme(const char *uri)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	std::string parameter = std::string(uri);
	std::string designated_pkg;
	std::string extra_data;
	app_control_h app_control = NULL;

	size_t pkgname_start_pos = 0;
	size_t pkgname_end_pos = 0;

	if (parameter.find("package=") != std::string::npos) {
		pkgname_start_pos = parameter.find("package=") + strlen("package=");
		pkgname_end_pos = parameter.find(";", pkgname_start_pos);
		designated_pkg = parameter.substr(pkgname_start_pos, pkgname_end_pos - pkgname_start_pos);
	}

	if (!designated_pkg.length()) {
		BROWSER_LOGE("Failed to get pkg name from intent scheme");
		return EINA_FALSE;
	}

	/* parse parameter */
	size_t extra_data_start_pos = 0;
	size_t extra_data_end_pos = 0;

	if (parameter.find("param=") != std::string::npos) {
		extra_data_start_pos = parameter.find("param=") + strlen("param=");
		extra_data_end_pos = parameter.find(";", extra_data_start_pos);
		extra_data = parameter.substr(extra_data_start_pos, extra_data_end_pos - extra_data_start_pos);
	}

	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (!app_control) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (app_control_set_app_id(app_control, designated_pkg.c_str()) < 0) {
		BROWSER_LOGE("Fail to app_control_set_app_id");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (extra_data.length()) {
		if (app_control_add_extra_data(app_control, "param", extra_data.c_str()) < 0) {
			BROWSER_LOGE("app_control_add_extra_data is failed.");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::_handle_tizen_service_scheme(const char *uri)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	std::string parameter = std::string(uri);
	std::string designated_pkg;
	std::string application_ID;
	std::string key_string;
	std::string value_string;
	app_control_h app_control = NULL;

	/* parse pkg name */
	size_t pkgname_start_pos = 0;
	size_t pkgname_end_pos = 0;

	if (parameter.find("package=") != std::string::npos) {
		pkgname_start_pos = parameter.find("package=") + strlen("package=");
		pkgname_end_pos = parameter.find(";", pkgname_start_pos);
		designated_pkg = parameter.substr(pkgname_start_pos, pkgname_end_pos - pkgname_start_pos);
	} else if (parameter.find("AppID=") != std::string::npos) {
		pkgname_start_pos = parameter.find("AppID=") + strlen("AppID=");
		pkgname_end_pos = parameter.find(";", pkgname_start_pos);
		application_ID = parameter.substr(pkgname_start_pos, pkgname_end_pos - pkgname_start_pos);
	} else {
		BROWSER_LOGE("Failed to parse pkg name from tizen service scheme");
		return EINA_FALSE;
	}

	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (!app_control) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (application_ID.length()) {
		if (app_control_set_app_id(app_control, application_ID.c_str()) < 0) {
			BROWSER_LOGE("Fail to app_control_set_app_id");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	} else if (designated_pkg.length()){
		if (app_control_set_app_id(app_control, designated_pkg.c_str()) < 0) {
			BROWSER_LOGE("Fail to app_control_set_app_id");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	} else {
		BROWSER_LOGE("Fail to get pkg name");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	/* parse parameter */
	int key_string_start_pos = 0;
	int key_string_end_pos = 0;
	int value_string_start_pos = 0;
	int value_string_end_pos = 0;

	Eina_Bool has_more_data = EINA_TRUE;

	while (has_more_data == EINA_TRUE) {
		key_string_start_pos = 0;
		key_string_end_pos = 0;
		value_string_start_pos = 0;
		value_string_end_pos = 0;
		key_string.clear();
		value_string.clear();

		if (parameter.find("key=") != std::string::npos) {
			if (parameter.find("value=") != std::string::npos) {
				key_string_start_pos = parameter.find("key=") + strlen("key=");
				key_string_end_pos = parameter.find(",", key_string_start_pos);
				if ((key_string_start_pos >= strlen("key=")) && (key_string_end_pos >= 0)) {
					key_string = parameter.substr(key_string_start_pos, key_string_end_pos - key_string_start_pos);
					parameter.erase(key_string_start_pos - strlen("key="), key_string_end_pos - key_string_start_pos + strlen("key="));
				} else
					has_more_data = EINA_FALSE;

				value_string_start_pos = parameter.find("value=") + strlen("value=");
				value_string_end_pos = parameter.find(";", value_string_start_pos);
				if ((value_string_start_pos >= strlen("value=")) && (value_string_end_pos >= 0)) {
					value_string = parameter.substr(value_string_start_pos, value_string_end_pos - value_string_start_pos);
					parameter.erase(value_string_start_pos - strlen("value="), value_string_end_pos - value_string_start_pos + strlen("value="));
				} else
					has_more_data = EINA_FALSE;

				if (key_string.length() && value_string.length()) {
					if (app_control_add_extra_data(app_control, key_string.c_str(), value_string.c_str()) < 0) {
						BROWSER_LOGE("app_control_add_extra_data is failed.");
						app_control_destroy(app_control);
						return EINA_FALSE;
					}
				}
			} else
				has_more_data = EINA_FALSE;
		} else
			has_more_data = EINA_FALSE;
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::_handle_mailto_scheme(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to app_control_create");
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_COMPOSE) < 0) {
		BROWSER_LOGE("Fail to app_control_set_operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_uri(app_control, uri) < 0) {
		BROWSER_LOGE("Fail to app_control_set_uri");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to app_control_send_launch_request");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::_handle_unknown_scheme(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to app_control_create");
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) < 0) {
		BROWSER_LOGE("Fail to app_control_set_operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_uri(app_control, uri) < 0) {
		BROWSER_LOGE("Fail to app_control_set_uri");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to app_control_send_launch_request");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool common_view::handle_scheme(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	if (!strncmp(uri, http_scheme, strlen(http_scheme)))
		return EINA_FALSE;
	else if (!strncmp(uri, https_scheme, strlen(https_scheme)))
		return EINA_FALSE;
	else if (!strncmp(uri, file_scheme, strlen(file_scheme)))
		return EINA_FALSE;
	else if (!strncmp(uri, rtsp_scheme, strlen(rtsp_scheme))) {
		// WebKit will handle RTSP stream.
		return EINA_TRUE;
	} else if (!strncmp(uri, tizenstore_scheme, strlen(tizenstore_scheme))) {
		launch_tizenstore(uri);
		return EINA_TRUE;
	} else if (!strncmp(uri, mailto_scheme, strlen(mailto_scheme))) {
		_handle_mailto_scheme(uri);
		return EINA_TRUE;
	} else if (!strncmp(uri, sms_scheme, strlen(sms_scheme)) || !strncmp(uri, smsto_scheme, strlen(smsto_scheme))) {
		launch_message(uri, NULL);
		return EINA_TRUE;
	} else if (!strncmp(uri, tel_scheme, strlen(tel_scheme)) || !strncmp(uri, telto_scheme, strlen(telto_scheme))) {
		launch_dialer(uri);
		return EINA_TRUE;
	} else if (!strncmp(uri, tizen_service_scheme, strlen(tizen_service_scheme))) {
		_handle_tizen_service_scheme(uri);
		return EINA_TRUE;
	} else if (!strncmp(uri, callto_scheme, strlen(callto_scheme))) {
		std::string request_uri = std::string(tel_scheme) + std::string(uri + strlen(callto_scheme));
		launch_dialer(request_uri.c_str());
		return EINA_TRUE;
	} else if (!strncmp(uri, vtel_scheme, strlen(vtel_scheme))) {
		std::string request_uri = std::string(tel_scheme) + std::string(uri + strlen(vtel_scheme));
		launch_dialer(request_uri.c_str());
		return EINA_TRUE;
	} else if (!strncmp(uri, wtai_wp_ap_scheme, strlen(wtai_wp_ap_scheme))) {
		std::string number = std::string(uri + strlen(wtai_wp_ap_scheme));
		if (number.find(";") != std::string::npos) {
			number = number.substr(0, number.length() - number.substr(number.find(";")).length());
		}
		BROWSER_LOGD("phone number = [%s]", number.c_str());
		launch_contact(number.c_str());
		return EINA_TRUE;
	} else if (!strncmp(uri, wtai_wp_mc_scheme, strlen(wtai_wp_mc_scheme))) {
		std::string request_uri = std::string(tel_scheme) + std::string(uri + strlen(wtai_wp_mc_scheme));
		launch_dialer(request_uri.c_str());
		return EINA_TRUE;
	} else if (!strncmp(uri, wtai_wp_sd_scheme, strlen(wtai_wp_sd_scheme))) {
		std::string request_uri = std::string(tel_scheme) + std::string(uri + strlen(wtai_wp_sd_scheme));
		launch_dialer(request_uri.c_str());
		return EINA_TRUE;
	} else if (strstr(uri, ":"))
		return _handle_unknown_scheme(uri);

	return EINA_FALSE;
}

Eina_Bool common_view::_is_file_scheme(const char* uri)
{
	if (!strncmp(uri, file_scheme, strlen(file_scheme)))
		return EINA_TRUE;

	return EINA_FALSE;
}

void common_view::set_trans_text_to_object(Evas_Object *obj, const char *text_id, const char* part)
{
	RET_MSG_IF(text_id == NULL, "text_id is NULL");
	RET_MSG_IF(obj == NULL, "obj is NULL");
	const char *domain = NULL;

	if(strstr(text_id, "IDS_COM"))
		domain = SYSTEM_DOMAIN_NAME;
	else
		domain = BROWSER_DOMAIN_NAME;

	char *mark_up = elm_entry_utf8_to_markup(text_id);
	elm_object_domain_translatable_part_text_set(obj, part, domain, mark_up);

	if(mark_up)
		free(mark_up);
}

void common_view::set_trans_text_to_object_item(Elm_Object_Item *it, const char *text_id, const char* part)
{
	RET_MSG_IF(text_id == NULL, "text_id is NULL");
	RET_MSG_IF(it == NULL, "item is NULL");
	const char *domain = NULL;

	if(strstr(text_id, "IDS_COM"))
		domain = SYSTEM_DOMAIN_NAME;
	else
		domain = BROWSER_DOMAIN_NAME;

	elm_object_item_domain_translatable_part_text_set(it, part, domain, text_id);
}

char *common_view::get_text_from_ID(const char *ID)
{
	RETV_MSG_IF(!ID, NULL, "text id is NULL");
	char *str = NULL;

	if(strstr(ID, "IDS_COM"))
		str = dgettext("sys_string", ID);
	else
		str = gettext(ID);

	return str;
}

Evas_Object *common_view::_create_title_icon_btn(Evas_Object *parent, Evas_Smart_Cb func, const char *icon_path, const char *icon_group, void *data)
{
	Evas_Object *ic;
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	elm_object_style_set(btn, "naviframe/title_icon");
	ic = elm_image_add(parent);
	elm_image_file_set(ic, icon_path, icon_group);
	elm_object_part_content_set(btn, "icon", ic);
	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

Evas_Object *common_view::_create_title_text_btn(Evas_Object *parent, const char *text_id, Evas_Smart_Cb func, void *data)
{
	RETV_MSG_IF(!parent, NULL, "parent is NULL");
	RETV_MSG_IF(!text_id, NULL, "text_id is NULL");
	RETV_MSG_IF(!strlen(text_id), NULL, "text_id has no length");

	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	elm_object_style_set(btn, "naviframe/title_text");

	set_trans_text_to_object(btn, text_id, NULL);

	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

Evas_Object *common_view::_create_title_text_btn(Evas_Object *parent, const char *text_id, Evas_Smart_Cb func, void *data, Eina_Bool is_left)
{
	RETV_MSG_IF(!parent, NULL, "parent is NULL");
	RETV_MSG_IF(!text_id, NULL, "text_id is NULL");
	RETV_MSG_IF(!strlen(text_id), NULL, "text_id has no length");

	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	if (is_left)
		elm_object_style_set(btn, "naviframe/title_text_left");
	else
		elm_object_style_set(btn, "naviframe/title_text_right");

	set_trans_text_to_object(btn, text_id, NULL);

	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

Evas_Object* common_view::_create_box(Evas_Object* parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *box = NULL;
	box = elm_box_add(parent);
	RETV_MSG_IF(!(box), NULL, "box is NULL");
	elm_object_focus_set(box, EINA_FALSE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_clear(box);
	evas_object_show(box);
	return box;
}

Evas_Object* common_view::_create_tabbar(Evas_Object* parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *toolbar = elm_toolbar_add(parent);
	RETV_MSG_IF(!(toolbar), NULL, "toolbar is NULL");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	evas_object_size_hint_weight_set(toolbar, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(toolbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_show(toolbar);
	return  toolbar;
}

Evas_Object* common_view::_create_main_layout(Evas_Object* parent, const char *file)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *layout = elm_layout_add(parent);
	RETV_MSG_IF(!layout, NULL, "layout is NULL");
	elm_layout_file_set(layout, file, "main-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_focus_set(layout, EINA_FALSE);
	evas_object_show(layout);

	return layout;
}

Evas_Object *common_view::_create_no_content(Evas_Object *parent, const char *text, const char *text_help)
{
	BROWSER_LOGD("");
	if (parent == NULL) {
		BROWSER_LOGD("parent is NULL");
		return NULL;
	}

	Evas_Object *scroller = elm_scroller_add(parent);
	if (!scroller) {
		BROWSER_LOGE("elm_scroller_add failed");
		return NULL;
	}
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	evas_object_show(scroller);

	Evas_Object *no_contents_layout = elm_layout_add(parent);
	if (!no_contents_layout) {
		BROWSER_LOGD("elm_layout_add is failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(no_contents_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(no_contents_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_layout_theme_set(no_contents_layout, "layout", "nocontents", "default");
	elm_object_focus_set(no_contents_layout, EINA_FALSE);
	elm_object_domain_translatable_part_text_set(no_contents_layout, "elm.text", BROWSER_DOMAIN_NAME, text);
	if (text_help)
		elm_object_domain_translatable_part_text_set(no_contents_layout, "elm.help.text", BROWSER_DOMAIN_NAME, text_help);
	elm_layout_signal_emit(no_contents_layout, "align.center", "elm");

	Evas_Object *no_content_access = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(no_contents_layout), "elm.text");
	Evas_Object *no_content_access_object = elm_access_object_register(no_content_access, no_contents_layout);
	elm_object_focus_custom_chain_append(no_contents_layout, no_content_access_object, NULL);
	elm_access_info_set(no_content_access_object, ELM_ACCESS_INFO, text);

	elm_object_content_set(scroller, no_contents_layout);
	return scroller;
}

std::string common_view::get_font_size_tag(void)
{
	return "font_size=22";
}

void common_view::show_email_scheme_popup(const char *uri)
{
	RET_MSG_IF(!uri, "uri is NULL");
	RET_MSG_IF(strlen(uri) == 0, "uri has no length");

	Evas_Object *popup = NULL;
	popup = brui_popup_add(m_window);
	RET_MSG_IF(!popup, "popup is NULL");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_event_callback_add(popup,  EVAS_CALLBACK_FREE, __popup_freed_cb, this);
	elm_object_part_text_set(popup, "title,text", uri + strlen(mailto_scheme));

	scheme_handling_popup_callback *cb = new(std::nothrow)
		scheme_handling_popup_callback(strdup(uri), this);
	RET_MSG_IF(cb == NULL, "Failed to allocate");

	evas_object_smart_callback_add(popup, "block,clicked", __scheme_popup_cancel_cb, cb);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __scheme_popup_cancel_cb, cb);

	elm_popup_item_append(popup, BR_STRING_SEND_EMAIL, NULL, __scheme_popup_email_selected_cb, cb);
	elm_popup_item_append(popup, BR_STRING_ADD_TO_CONTACT, NULL, __scheme_popup_contact_selected_cb, cb);
	elm_popup_item_append(popup, BR_STRING_COPY, NULL, __scheme_popup_copy_selected_cb, cb);

	m_context_popup = popup;
	evas_object_show(popup);
}

void common_view::show_phone_number_scheme_popup(const char *uri)
{
	RET_MSG_IF(!uri, "uri is NULL");
	RET_MSG_IF(strlen(uri) == 0, "uri has no length");

	Evas_Object *popup = NULL;
	popup = brui_popup_add(m_window);
	RET_MSG_IF(!popup, "popup is NULL");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_event_callback_add(popup,  EVAS_CALLBACK_FREE, NULL, this);
	elm_object_part_text_set(popup, "title,text", uri + strlen(tel_scheme));

	scheme_handling_popup_callback *cb = new(std::nothrow)
		scheme_handling_popup_callback(strdup(uri), this);
	RET_MSG_IF(cb == NULL, "Failed to allocate");

	evas_object_smart_callback_add(popup, "block,clicked", __scheme_popup_cancel_cb, cb);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __scheme_popup_cancel_cb, cb);

	elm_popup_item_append(popup, BR_STRING_CALL, NULL, __scheme_popup_call_selected_cb, cb);
	elm_popup_item_append(popup, BR_STRING_SEND_MESSAGE, NULL, __scheme_popup_message_selected_cb, cb);
	elm_popup_item_append(popup, BR_STRING_ADD_TO_CONTACT, NULL, __scheme_popup_contact_selected_cb, cb);
	elm_popup_item_append(popup, BR_STRING_COPY, NULL, __scheme_popup_copy_selected_cb, cb);

	m_context_popup = popup;
	evas_object_show(popup);
}

void common_view::__scheme_popup_email_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!obj, "obj is NULL");
	RET_MSG_IF(!data, "data is NULL");

	if (obj != NULL) {
		evas_object_del(obj);
		obj = NULL;
	}

	scheme_handling_popup_callback *cb = static_cast<scheme_handling_popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;
	char *uri = cb->uri;

	if (uri && strlen(uri) > 0 && !strncmp(uri, mailto_scheme, strlen(mailto_scheme)))
		cv->handle_scheme((const char *)uri);

	free(uri);
	delete cb;
}

void common_view::__scheme_popup_message_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!obj, "obj is NULL");
	RET_MSG_IF(!data, "data is NULL");

	if (obj != NULL) {
		evas_object_del(obj);
		obj = NULL;
	}

	scheme_handling_popup_callback *cb = static_cast<scheme_handling_popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;
	char *uri = cb->uri;

	if (uri && strlen(uri) > 0 && !strncmp(uri, tel_scheme, strlen(tel_scheme))) {
			std::string::size_type pos = std::string::npos;
			std::string source = std::string(uri);
			while ((pos = source.find(tel_scheme)) != std::string::npos)
				source.replace(pos, strlen(tel_scheme), sms_scheme);
			cv->handle_scheme(source.c_str());
	}

	free(uri);
	delete cb;
}

void common_view::__scheme_popup_call_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!obj, "obj is NULL");
	RET_MSG_IF(!data, "data is NULL");

	if (obj != NULL) {
		evas_object_del(obj);
		obj = NULL;
	}

	scheme_handling_popup_callback *cb = static_cast<scheme_handling_popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;
	char *uri = cb->uri;

	if (uri && strlen(uri) > 0 && !strncmp(uri, tel_scheme, strlen(tel_scheme)))
		cv->handle_scheme((const char *)uri);

	free(uri);
	delete cb;
}

void common_view::__scheme_popup_contact_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!obj, "obj is NULL");
	RET_MSG_IF(!data, "data is NULL");

	if (obj != NULL) {
		evas_object_del(obj);
		obj = NULL;
	}

	scheme_handling_popup_callback *cb = static_cast<scheme_handling_popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;
	char *uri = cb->uri;

	if (uri && strlen(uri) > 0 && !strncmp(uri, tel_scheme, strlen(tel_scheme)))
		cv->launch_contact(uri + strlen(tel_scheme));
	else if (uri && strlen(uri) > 0 && !strncmp(uri, mailto_scheme, strlen(mailto_scheme))) {
		size_t source_end_pos = 0;
		std::string source = std::string(uri);

		if (source.find("?") != std::string::npos) {
			source_end_pos = source.find("?");
			source = source.substr(0, source_end_pos);
		}
		cv->launch_contact(NULL, source.c_str() + strlen(mailto_scheme));
	}

	free(uri);
	delete cb;
}

void common_view::__scheme_popup_copy_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!obj, "obj is NULL");
	RET_MSG_IF(!data, "data is NULL");

	scheme_handling_popup_callback *cb = static_cast<scheme_handling_popup_callback*>(data);
	char *uri = cb->uri;

	if (uri && strlen(uri) > 0 && !strncmp(uri, tel_scheme, strlen(tel_scheme)) && strlen(uri) > strlen(tel_scheme))
		elm_cnp_selection_set(m_window, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT, uri + strlen(tel_scheme), strlen(uri) - strlen(tel_scheme));
	else if (uri && strlen(uri) > 0 && !strncmp(uri, mailto_scheme, strlen(mailto_scheme)) && strlen(uri) > strlen(mailto_scheme))
		elm_cnp_selection_set(m_window, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT, uri + strlen(mailto_scheme), strlen(uri) - strlen(mailto_scheme));

	if (obj != NULL) {
		evas_object_del(obj);
		obj = NULL;
	}
}

void common_view::__scheme_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!obj, "obj is NULL");

	if (obj != NULL) {
		evas_object_del(obj);
		obj = NULL;
	}

	RET_MSG_IF(!data, "data is NULL");
	scheme_handling_popup_callback *cb = static_cast<scheme_handling_popup_callback*>(data);
	common_view *cv = (common_view *)cb->user_data;
	cv->m_context_popup = NULL;

	free(cb->uri);
	delete cb;
}

void common_view::__popup_freed_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
}
