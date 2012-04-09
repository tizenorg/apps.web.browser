/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#include "browser-picker-handler.h"
#include "browser-view.h"

#include <ui-gadget.h>

Browser_Picker_Handler::Browser_Picker_Handler(Browser_View *browser_view)
:	m_browser_view(browser_view)
	,m_webview(NULL)
	,m_picker_ug(NULL)
	,m_picker_layout(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Picker_Handler::~Browser_Picker_Handler(void)
{
	BROWSER_LOGD("[%s]", __func__);
	_destroy_options();

	if (m_picker_layout) {
		elm_object_part_content_unset(m_picker_layout, "elm.swallow.picker");
		evas_object_del(m_picker_layout);
	}

	if (m_picker_ug) {
		ug_destroy(m_picker_ug);
		m_picker_ug = NULL;
	}
}

void Browser_Picker_Handler::destroy_picker_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	_destroy_options();

	if (m_picker_layout) {
		elm_object_part_content_unset(m_picker_layout, "elm.swallow.picker");
		evas_object_del(m_picker_layout);
	}

	if (m_picker_ug) {
		ug_destroy(m_picker_ug);
		m_picker_ug = NULL;
	}
}

void Browser_Picker_Handler::init(Evas_Object *webview)
{
	BROWSER_LOGD("[%s]", __func__);
	deinit();

	m_webview = webview;
	memset(&m_selected_info, 0x00, sizeof(selected_info));

	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	evas_object_smart_callback_add(webkit, "one,single,tap", __one_single_tap_cb, this);
	evas_object_smart_callback_add(webkit, "inputmethod,changed", __input_method_changed_cb, this);
}

void Browser_Picker_Handler::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_webview) {
		Evas_Object *webkit = elm_webview_webkit_get(m_webview);

		_destroy_options();

		if (m_picker_layout) {
			elm_object_part_content_unset(m_picker_layout, "elm.swallow.picker");
			evas_object_del(m_picker_layout);
		}

		if (m_picker_ug) {
			ug_destroy(m_picker_ug);
			m_picker_ug = NULL;
		}

		evas_object_smart_callback_del(webkit, "one,single,tap", __one_single_tap_cb);
		evas_object_event_callback_del(m_win, EVAS_CALLBACK_RESIZE, __win_resize_cb);
		evas_object_smart_callback_del(webkit, "inputmethod,changed", __input_method_changed_cb);
	}
}

void Browser_Picker_Handler::_destroy_options(void)
{
	BROWSER_LOGD("[%s]", __func__);
	for (int i = 0 ; i < m_selected_info.option_number ; i++) {
		if (m_selected_info.option_list[i]) {
			free(m_selected_info.option_list[i]);
			m_selected_info.option_list[i] = NULL;
		}
	}

	memset(&m_selected_info, 0x00, sizeof(selected_info));
}

Eina_Bool Browser_Picker_Handler::_show_picker(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_picker_ug) {
		ug_destroy(m_picker_ug);
		m_picker_ug = NULL;
	}

	bundle *b = NULL;
	struct ug_cbs cbs = {0,};
	b = bundle_create();
	if (!b) {
		BROWSER_LOGE("bundle_create failed");
		return EINA_FALSE;
	}

	char bundle_buf[100] = {0, };
	sprintf(bundle_buf, "%d", m_selected_info.option_number);
	if (bundle_add(b, "Count", bundle_buf)) {
		BROWSER_LOGE("bundle_add failed");
		bundle_free(b);
		return EINA_FALSE;
	}

	char item_title[100] = {0, };
	for (int i = 0 ; i < m_selected_info.option_number ; i++) {
		sprintf(item_title, "%d", i);
		if (bundle_add(b, item_title, m_selected_info.option_list[i])) {
			BROWSER_LOGE("bundle_add failed");
			bundle_free(b);
			return EINA_FALSE;
		}
	}

	cbs.layout_cb = __picker_layout_cb;
	cbs.result_cb = __picker_result_cb;
	cbs.destroy_cb = __picker_destroy_cb;
	cbs.priv = (void *)this;

	m_picker_ug = ug_create(NULL, "picker-efl", UG_MODE_FRAMEVIEW, b, &cbs);
	if (!m_picker_ug) {
		BROWSER_LOGE("ug_create failed");
		return EINA_FALSE;
	}
	bundle_free(b);

	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	evas_object_smart_callback_call(webkit, "make,select,visible", (void*)(&m_selected_info.rect));

	evas_object_event_callback_del(m_win, EVAS_CALLBACK_RESIZE, __win_resize_cb);
	evas_object_event_callback_add(m_win, EVAS_CALLBACK_RESIZE, __win_resize_cb, this);

	bundle *option_bundle = NULL;
	option_bundle = bundle_create();
	if (!option_bundle) {
		BROWSER_LOGE("bundle_create failed");
		return EINA_FALSE;
	}

	BROWSER_LOGD("prev=%d, next=%d", m_selected_info.is_prev, m_selected_info.is_next);
	if (!m_selected_info.is_prev) {
		if (bundle_add(option_bundle, "PrevButton", "Disable")) {
			BROWSER_LOGE("bundle_add is failed.");
			bundle_free(option_bundle);
			return EINA_FALSE;
		}
	}
	if (!m_selected_info.is_next) {
		if (bundle_add(option_bundle, "NextButton", "Disable")) {
			BROWSER_LOGE("bundle_add is failed.");
			bundle_free(option_bundle);
			return EINA_FALSE;
		}
	}
	if (m_picker_ug) {
		if (ug_send_message(m_picker_ug, option_bundle))
			BROWSER_LOGE("ug_send_message is failed.\n");
	}
	bundle_free(option_bundle);

	return EINA_TRUE;
}

void Browser_Picker_Handler::__win_resize_cb(void* data, Evas* evas, Evas_Object* obj, void* ev)
{
	if (!data)
		return;

	Browser_Picker_Handler *picker_handler = (Browser_Picker_Handler *)data;

	int window_w = 0;
	int window_h = 0;
	evas_object_geometry_get(m_win, NULL, NULL, &window_w, &window_h);
	BROWSER_LOGD("window w=%d, h=%d", window_w, window_h);
	evas_object_resize(picker_handler->m_picker_layout, window_w, window_h);
}

void Browser_Picker_Handler::__picker_layout_cb(struct ui_gadget *ug, enum ug_mode mode, void *priv)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!ug || !priv)
		return;

	Browser_Picker_Handler *picker_handler = (Browser_Picker_Handler *)priv;

	if (picker_handler->m_picker_layout) {
		elm_object_part_content_unset(picker_handler->m_picker_layout, "elm.swallow.picker");
		evas_object_del(picker_handler->m_picker_layout);
	}

	picker_handler->m_picker_layout = elm_layout_add(m_navi_bar);
	if (!picker_handler->m_picker_layout) {
		BROWSER_LOGE("elm_layout_add");
		return;
	}
	elm_object_focus_allow_set(picker_handler->m_picker_layout, EINA_FALSE);

	if (!elm_layout_file_set(picker_handler->m_picker_layout, BROWSER_EDJE_DIR"/browser-picker-layout.edj",
										"picker_layout")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return;
	}
	evas_object_size_hint_weight_set(picker_handler->m_picker_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	int window_w = 0;
	int window_h = 0;
	evas_object_geometry_get(m_win, NULL, NULL, &window_w, &window_h);

	evas_object_resize(picker_handler->m_picker_layout, window_w, window_h);
	evas_object_move(picker_handler->m_picker_layout, 0, 0);
	evas_object_show(picker_handler->m_picker_layout);

	Evas_Object *base = (Evas_Object *)ug_get_layout(ug);
	if (!base) {
		BROWSER_LOGE("base is null");
		return;
	}

	switch (mode) {
		case UG_MODE_FRAMEVIEW:
			elm_object_part_content_set(picker_handler->m_picker_layout, "elm.swallow.picker", base);
			break;
		default:
			break;
	}
}

Eina_Bool Browser_Picker_Handler::_move_to_next_node(Eina_Bool is_next_node)
{
	BROWSER_LOGD("[%s]", __func__);

	_destroy_options();

	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	if (is_next_node) {
		m_selected_info.option_list = ewk_view_dropdown_get_next_options(webkit,
							&m_selected_info.option_number,
							&m_selected_info.current_option_index,
							&m_selected_info.rect,
							&m_selected_info.is_prev,
							&m_selected_info.is_next);
	} else {
		m_selected_info.option_list = ewk_view_dropdown_get_prev_options(webkit,
							&m_selected_info.option_number,
							&m_selected_info.current_option_index,
							&m_selected_info.rect,
							&m_selected_info.is_prev,
							&m_selected_info.is_next);
	}

	bundle *b = bundle_create();
	if (!b) {
		BROWSER_LOGE("bundle_create failed");
		return EINA_FALSE;
	}

	if (bundle_add(b, "Command", "Refill")) {
		BROWSER_LOGE("bundle_add failed");
		bundle_free(b);
		return EINA_FALSE;
	}
	char bundle_buf[100] = {0, };
	sprintf(bundle_buf, "%d", m_selected_info.option_number);
	if (bundle_add(b, "Count", bundle_buf)) {
		BROWSER_LOGE("bundle_add failed");
		bundle_free(b);
		return EINA_FALSE;
	}

	BROWSER_LOGD("prev=%d, next=%d", m_selected_info.is_prev, m_selected_info.is_next);
	if (!m_selected_info.is_prev) {
		if (bundle_add(b, "PrevButton", "Disable")) {
			BROWSER_LOGE("bundle_add is failed.");
			bundle_free(b);
			return EINA_FALSE;
		}
	} else {
		if (bundle_add(b, "PrevButton", "Enable")) {
			BROWSER_LOGE("bundle_add is failed.");
			bundle_free(b);
			return EINA_FALSE;
		}
	}

	if (!m_selected_info.is_next) {
		if (bundle_add(b, "NextButton", "Disable")) {
			BROWSER_LOGE("bundle_add is failed.");
			bundle_free(b);
			return EINA_FALSE;
		}
	} else {
		if (bundle_add(b, "NextButton", "Enable")) {
			BROWSER_LOGE("bundle_add is failed.");
			bundle_free(b);
			return EINA_FALSE;
		}
	}

	char item_title[100] = {0, };
	for (int i = 0 ; i < m_selected_info.option_number ; i++) {
		sprintf(item_title, "%d", i);
		if (bundle_add(b, item_title, m_selected_info.option_list[i])) {
			BROWSER_LOGE("bundle_add failed");
			bundle_free(b);
			return EINA_FALSE;
		}
	}

	if (m_picker_ug) {
		if (ug_send_message(m_picker_ug, b))
			BROWSER_LOGE("ug_send_message is failed.\n");
	}

	bundle_free(b);

	evas_object_smart_callback_call(webkit, "make,select,visible", (void*)(&m_selected_info.rect));
}

void Browser_Picker_Handler::__picker_result_cb(struct ui_gadget *ug, bundle *result, void *priv)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!ug || !priv || !result)
		return;

	Browser_Picker_Handler *picker_handler = (Browser_Picker_Handler *)priv;
	const char *response = bundle_get_val(result, "Result");
	if (response) {
		if (!strncmp(response, "Success", strlen("Success")))
			picker_handler->__picker_destroy_cb(ug, priv);
		else if (!strncmp(response, "Prev", strlen("Prev")))
			picker_handler->_move_to_next_node(EINA_FALSE);
		else if (!strncmp(response, "Next", strlen("Next")))
			picker_handler->_move_to_next_node(EINA_TRUE);
	}

	const char *index = bundle_get_val(result, "Index");
	if (index) {
		Evas_Object *webkit = elm_webview_webkit_get(picker_handler->m_webview);
		if (atoi(index)) {
			if (!ewk_page_dropdown_set_current_index(webkit, atoi(index)))
				BROWSER_LOGE("ewk_page_dropdown_set_current_index is failed.\n");
		}
	}
}

void Browser_Picker_Handler::__picker_destroy_cb(struct ui_gadget *ug, void *priv)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!ug || !priv)
		return;

	Browser_Picker_Handler *picker_handler = (Browser_Picker_Handler *)priv;

	if (ug_destroy(ug))
		BROWSER_LOGE("ug_destroy is failed.\n");

	picker_handler->m_picker_ug = NULL;

	elm_object_part_content_unset(picker_handler->m_picker_layout, "elm.swallow.picker");
	evas_object_del(picker_handler->m_picker_layout);
	picker_handler->m_picker_layout = NULL;
}

void Browser_Picker_Handler::__one_single_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data || !event_info)
		return;

	Evas_Point *position = (Evas_Point *)event_info;
	Browser_Picker_Handler *picker_handler = (Browser_Picker_Handler *)data;
	Evas_Object *webkit = elm_webview_webkit_get(picker_handler->m_webview);
	double zoom_rate = 1.0f;
	int webview_y = 0;
	evas_object_geometry_get(webkit, NULL, &webview_y, NULL, NULL);

	picker_handler->_destroy_options();

	if (ewk_view_zoom_cairo_scaling_get(webkit) == EINA_TRUE)
		zoom_rate = ewk_view_zoom_get(webkit);

	picker_handler->m_selected_info.clicked_x = position->x / zoom_rate;
	picker_handler->m_selected_info.clicked_y = (position->y - webview_y) / zoom_rate;

	picker_handler->m_selected_info.option_list = ewk_page_dropdown_get_options(webkit,
						picker_handler->m_selected_info.clicked_x,
						picker_handler->m_selected_info.clicked_y,
						&picker_handler->m_selected_info.option_number,
						&picker_handler->m_selected_info.current_option_index,
						&picker_handler->m_selected_info.rect,
						&picker_handler->m_selected_info.is_prev,
						&picker_handler->m_selected_info.is_next);

	if (picker_handler->m_selected_info.option_list) {
		if (!picker_handler->_show_picker())
			BROWSER_LOGE("_show_picker failed");
	}
}

void Browser_Picker_Handler::__input_method_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_Picker_Handler *picker_handler = (Browser_Picker_Handler *)data;
	Evas_Object *webkit = elm_webview_webkit_get(picker_handler->m_webview);
	int imh_type = ewk_view_imh_get(obj);
	BROWSER_LOGD("imh_type = %d", imh_type);
	if (imh_type == EWK_IMH_DATE) {
		const char *current_date = ewk_view_focused_input_element_value_get(webkit);
		BROWSER_LOGD("current_date=[%s]", current_date);

		if (!picker_handler->_show_calendar_picker(current_date, INPUT_TYPE_DATE))
			BROWSER_LOGE("_show_calendar_picker failed");

		if (current_date)
			free((char *)current_date);
	} else if (imh_type == EWK_IMH_TIME) {
		/* To do. */
	}
}

Eina_Bool Browser_Picker_Handler::_show_calendar_picker(const char *date, input_type type)
{
	BROWSER_LOGD("date=[%s], type=%d", date, type);

	if (m_picker_ug) {
		ug_destroy(m_picker_ug);
		m_picker_ug = NULL;
	}

	bundle *b = NULL;
	struct ug_cbs cbs = {0,};
	b = bundle_create();
	if (!b) {
		BROWSER_LOGE("bundle_create failed");
		return EINA_FALSE;
	}

	if (bundle_add(b, "date", date)) {
		BROWSER_LOGE("bundle_add failed");
		bundle_free(b);
		return EINA_FALSE;
	}

	cbs.layout_cb = __picker_layout_cb;
	cbs.destroy_cb = __picker_destroy_cb;
	if (type == INPUT_TYPE_DATE)
		cbs.result_cb = __calendar_picker_date_result_cb;
	cbs.priv = (void *)this;

	m_picker_ug = ug_create(NULL, "calendar-picker-efl", UG_MODE_FRAMEVIEW, b, &cbs);
	if (!m_picker_ug) {
		BROWSER_LOGE("ug_create failed");
		return EINA_FALSE;
	}
	bundle_free(b);

	return EINA_TRUE;
}

void Browser_Picker_Handler::__calendar_picker_date_result_cb(struct ui_gadget *ug,
								bundle *result, void *priv)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!ug || !priv || !result)
		return;

	Browser_Picker_Handler *picker_handler = (Browser_Picker_Handler *)priv;
	const char *value = bundle_get_val(result, "Result");
	if (value) {
		if (!strncmp(value, "Done", strlen("Done"))) {
			picker_handler->__picker_destroy_cb(ug, priv);
			return;
		}
	}
	const char *date = bundle_get_val(result, "Date");
	if (date) {
		Evas_Object *webkit = elm_webview_webkit_get(picker_handler->m_webview);
		if (!ewk_view_focused_input_element_value_set(webkit, date))
			BROWSER_LOGE("ewk_view_focused_input_element_value_set failed");
	}
}

