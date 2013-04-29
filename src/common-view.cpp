/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
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

#include "common-view.h"

#include <Eina.h>
#include <Elementary.h>
#include <app_manager.h>
#include <eina_list.h>
#include <fcntl.h>
#include <string>
#include <ui-gadget.h>
#include <vconf.h>
#include <vconf-internal-nfc-keys.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"

Evas_Object *common_view::m_bg;
Evas_Object *common_view::m_naviframe;
Evas_Object *common_view::m_conformant;

#define CALLBACK_DATA_1	"cb1"
#define CALLBACK_DATA_2	"cb2"
#define CALLBACK_DATA_3	"cb3"

#define bludtooth_template_html_path browser_res_dir"/template/template_bluetooth_content_share.html"
#define bludtooth_sending_html_path browser_data_dir"/bluetooth_content_share.html"
#define BLUETOOTH_SENDING_HTML_A_HREF_REPLACING_KEY "a_href_needed"
#define BLUETOOTH_SENDING_HTML_URI_REPLACING_KEY "uri_needed"

struct _popup_callback {
   Evas_Smart_Cb func;
   void *func_data;
   void *user_data;
};

typedef struct _sns_data {
	char *pkg_name;
	char *service_name;
} sns_data;

static void __ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	BROWSER_LOGD("");
	if (!priv || !ug)
		return;

	Evas_Object *base = (Evas_Object*)ug_get_layout(ug);
	if (!base)
		return;

	Evas_Object *win = (Evas_Object *)ug_get_window();

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(win, base);
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

static Evas_Object *_get_popup(Evas_Object *sub_obj)
{
	Evas_Object *parent = elm_object_parent_widget_get(sub_obj);
	Evas_Object *popup = elm_popup_add(parent);
	Evas_Object *return_popup = NULL;
	while (parent) {
		const char *type = elm_object_widget_type_get(parent);
		if (type && !strcmp(type, elm_object_widget_type_get(popup))) {
			return_popup = parent;
			break;
		}
		parent = elm_object_parent_widget_get(parent);
	}

	evas_object_del(popup);

	return return_popup;
}

static void _destroy_cbs(Evas_Object *popup)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(popup);

	popup_callback *cb1 = (popup_callback *)evas_object_data_get(popup, CALLBACK_DATA_1);
	if (cb1)
		free(cb1);

	popup_callback *cb2 = (popup_callback *)evas_object_data_get(popup, CALLBACK_DATA_2);
	if (cb2)
		free(cb2);

	popup_callback *cb3 = (popup_callback *)evas_object_data_get(popup, CALLBACK_DATA_3);
	if (cb3)
		free(cb3);

	evas_object_data_set(popup, CALLBACK_DATA_1, NULL);
	evas_object_data_set(popup, CALLBACK_DATA_2, NULL);
	evas_object_data_set(popup, CALLBACK_DATA_3, NULL);
}

void common_view::destroy_popup(Evas_Object *sub_obj)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(sub_obj);

	Evas_Object *parent = elm_object_parent_widget_get(sub_obj);
	Evas_Object *popup = elm_popup_add(parent);
	while (parent) {
		const char *type = elm_object_widget_type_get(parent);
		if (type && !strcmp(type, elm_object_widget_type_get(popup))) {
			_destroy_cbs(parent);

			evas_object_del(parent);
			break;
		}
		parent = elm_object_parent_widget_get(parent);
	}

	evas_object_del(popup);
}

static void _free_sns_list(Eina_List *sns_list)
{
	EINA_SAFETY_ON_NULL_RETURN(sns_list);

	void *list_data = NULL;
	EINA_LIST_FREE(sns_list, list_data) {
		if (list_data) {
			sns_data *sd = (sns_data *)list_data;
			if (sd->service_name)
				free(sd->service_name);
			if (sd->pkg_name)
				free(sd->pkg_name);
		}
	}
}

static bool _get_capability_cb(const char *type, account_capability_state_e state, void *data)
{
	BROWSER_LOGD("type=[%s], state=[%d]", type, state);
	Eina_Bool *can_post = (Eina_Bool *)data;

	if (type && !strcmp(type, ACCOUNT_SUPPORTS_CAPABILITY_POST) && ACCOUNT_CAPABILITY_DISABLED != state)
		*can_post = EINA_TRUE;

	return true;
}

common_view::common_view(void)
:
	m_share_uri(NULL)
	,m_sns_list(NULL)
{
	BROWSER_LOGD("");
}

common_view::~common_view(void)
{
	BROWSER_LOGD("");

	_free_sns_list(m_sns_list);
	m_sns_list = NULL;

	eina_stringshare_del(m_share_uri);
}

bool common_view::_get_account_cb(account_h account, void *data)
{
	BROWSER_LOGD("");
	common_view *cv = (common_view *)data;

	int account_id = 0;
	Eina_Bool can_post = false;

	account_get_capability_all(account, _get_capability_cb, &can_post);
	if (!can_post)
		return false;

	account_get_account_id(account, &account_id);

	char *domain_name = NULL;
	account_get_domain_name(account, &domain_name);
	if (!domain_name || strlen(domain_name) == 0)
		return false;

	char *pkg_name = NULL;
	account_get_package_name(account, &pkg_name);
	BROWSER_LOGD("pkg_name [%s]", pkg_name);
	if (!pkg_name || strlen(pkg_name) == 0) {
		free(domain_name);
		return false;
	}

	sns_data *item = (sns_data *)malloc(sizeof(sns_data));
	memset(item, 0x00, sizeof(sns_data));

	item->pkg_name = strdup(pkg_name);
	free(pkg_name);

	item->service_name = strdup(domain_name);
	free(domain_name);

	cv->m_sns_list = eina_list_append(cv->m_sns_list, item);

	return true;
}

void common_view::__button1_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = (popup_callback *)data;
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_popup(obj);
}

void common_view::__button2_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = (popup_callback *)data;
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_popup(obj);
}

void common_view::__button3_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = (popup_callback *)data;
	common_view *cv = (common_view *)cb->user_data;

	if (cb->func)
		cb->func(cb->func_data, obj, event_info);

	cv->destroy_popup(obj);
}

void common_view::__share_message_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	common_view *cv = (common_view *)data;
	cv->launch_message(cv->m_share_uri);

	_free_sns_list(cv->m_sns_list);
	cv->m_sns_list = NULL;

	cv->destroy_popup(obj);
}

void common_view::__share_email_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	common_view *cv = (common_view *)data;
	cv->launch_email(cv->m_share_uri);

	_free_sns_list(cv->m_sns_list);
	cv->m_sns_list = NULL;

	cv->destroy_popup(obj);
}

void common_view::__sns_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	common_view *cv = (common_view *)data;

	_free_sns_list(cv->m_sns_list);
	cv->m_sns_list = NULL;
}

void common_view::__share_sns_cb(void *data, Evas_Object *obj, void *event_info)
{
	common_view *cv = (common_view *)data;
	Elm_Object_Item *selected_item = elm_list_selected_item_get(obj);
	const char *sns_name = elm_object_item_text_get(selected_item);
	BROWSER_LOGD("sns_name=[%s]", sns_name);

	char *pkg_name = NULL;
	Eina_List *l = NULL;
	void *list_data = NULL;
	EINA_LIST_FOREACH(cv->m_sns_list, l, list_data) {
		if (list_data) {
			sns_data *sd = (sns_data *)list_data;
			if (!strcmp(sd->service_name, sns_name)) {
				pkg_name = strdup(sd->pkg_name);
				break;
			}
		}
	}
	BROWSER_LOGD("pkg_name=[%s]", pkg_name);
	_free_sns_list(cv->m_sns_list);
	cv->m_sns_list = NULL;

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("service_create failed");
		free(pkg_name);
		return;
	}
	if (service_set_operation(service_handle, SERVICE_OPERATION_SEND_TEXT) < 0) {
		BROWSER_LOGE("service_set_operation failed");
		service_destroy(service_handle);
		free(pkg_name);
		return;
	}
	if (service_add_extra_data(service_handle, SERVICE_DATA_TEXT, cv->m_share_uri) < 0) {
		BROWSER_LOGE("Fail to set post data");
		service_destroy(service_handle);
		free(pkg_name);
		return;
	}
	if (service_set_package(service_handle, pkg_name) < 0) {
		BROWSER_LOGE("Fail to set SNS");
		service_destroy(service_handle);
		free(pkg_name);
		return;
	}
	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		free(pkg_name);
		return;
	}
	service_destroy(service_handle);
	free(pkg_name);

	cv->destroy_popup(obj);
}

void common_view::show_content_popup(const char *title, Evas_Object *content_obj, const char *button1_text, Evas_Smart_Cb button1_func, const char *button2_text, Evas_Smart_Cb button2_func, void *data, Eina_Bool share)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(content_obj);

	Evas_Object *popup = elm_popup_add(m_window);
	if (!popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return;
	}

	elm_object_style_set(popup, "min_menustyle");

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title)
		elm_object_part_text_set(popup, "title,text", title);

	if (share) {
		Evas_Object *content_layout = elm_layout_add(popup);
		if (!content_layout) {
			BROWSER_LOGE("elm_layout_add failed");
			return;
		}
		elm_layout_file_set(content_layout, browser_edj_dir"/browser-popup.edj", "share_popup");
		evas_object_size_hint_weight_set(content_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(content_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_part_content_set(content_layout, "elm.swallow.content", content_obj);

		elm_object_content_set(popup, content_layout);
	} else {
		elm_object_content_set(popup, content_obj);
	}

	evas_object_show(popup);

	Evas_Object *button1 = elm_button_add(popup);
	if (!button1) {
		BROWSER_LOGE("elm_button_add failed");
		return;
	}

	if (button1_text)
		elm_object_text_set(button1, button1_text);
	else
		elm_object_text_set(button1, BR_STRING_OK);

	elm_object_style_set(button1, "popup_button/default");
	elm_object_part_content_set(popup, "button1", button1);

	popup_callback *cb1 = (popup_callback *)malloc(sizeof(popup_callback));
	if (!cb1)
		return;

	memset(cb1, 0x00, sizeof(popup_callback));
	cb1->func = button1_func;
	cb1->func_data = data;
	cb1->user_data = this;

	evas_object_smart_callback_add(button1, "clicked", __button1_cb, cb1);
	evas_object_show(button1);

	evas_object_data_set(popup, CALLBACK_DATA_1, cb1);

	if (button2_text) {
		Evas_Object *button2 = elm_button_add(popup);
		if (!button2) {
			BROWSER_LOGE("elm_button_add failed");
			free(cb1);
			cb1 = NULL;
			return;
		}
		elm_object_text_set(button2, button2_text);
		elm_object_style_set(button2, "popup_button/default");
		elm_object_part_content_set(popup, "button2", button2);

		popup_callback *cb2 = (popup_callback *)malloc(sizeof(popup_callback));
		if (!cb2)
			return;

		memset(cb2, 0x00, sizeof(popup_callback));
		cb2->func = button2_func;
		cb2->func_data = data;
		cb2->user_data = this;

		evas_object_smart_callback_add(button2, "clicked", __button2_cb, cb2);
		evas_object_show(button2);

		evas_object_data_set(popup, CALLBACK_DATA_2, cb2);
	}
}

Evas_Object *common_view::show_msg_popup(const char *title, const char *msg, const char *button1_text, Evas_Smart_Cb button1_func, const char *button2_text, Evas_Smart_Cb button2_func, void *data, const char *button3_text, Evas_Smart_Cb button3_func)
{
	BROWSER_LOGD("title = [%s], msg = [%s]", title, msg);
	EINA_SAFETY_ON_NULL_RETURN_VAL(msg, NULL);

	if (m_browser->get_app_in_app_enable())
		return NULL;

	Evas_Object *popup = elm_popup_add(m_window);
	if (!popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title)
		elm_object_part_text_set(popup, "title,text", title);

	if (msg) {
		if (strlen(msg) > 200) {
			elm_object_style_set(popup, "min_menustyle");

			Evas_Object *content_layout = elm_layout_add(popup);
			if (!content_layout) {
				BROWSER_LOGE("elm_layout_add failed");
				return NULL;
			}
			elm_layout_file_set(content_layout, browser_edj_dir"/browser-popup.edj", "text_scroll_popup");
			evas_object_size_hint_weight_set(content_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(content_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_object_part_content_set(content_layout, "elm.swallow.content", content_layout);

			Evas_Object *scroller = elm_scroller_add(content_layout);
			elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

			Evas_Object *label = elm_label_add(scroller);
			elm_label_line_wrap_set(label, ELM_WRAP_CHAR);

			char *markup_msg = elm_entry_utf8_to_markup(msg);
			if (!markup_msg)
				return NULL;

			elm_object_text_set(label, markup_msg);

			free(markup_msg);

			elm_object_content_set(scroller, label);
			elm_object_part_content_set(content_layout, "elm.swallow.scroller", scroller);

			elm_object_content_set(popup, content_layout);
		} else
			elm_object_text_set(popup, msg);
	}
	evas_object_show(popup);

	Evas_Object *button1 = elm_button_add(popup);
	if (!button1) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}

	if (button1_text)
		elm_object_text_set(button1, button1_text);
	else
		elm_object_text_set(button1, BR_STRING_OK);

	elm_object_style_set(button1, "popup_button/default");
	elm_object_part_content_set(popup, "button1", button1);

	popup_callback *cb1 = (popup_callback *)malloc(sizeof(popup_callback));
	if (!cb1)
		return NULL;

	memset(cb1, 0x00, sizeof(popup_callback));
	cb1->func = button1_func;
	cb1->func_data = data;
	cb1->user_data = this;

	evas_object_smart_callback_add(button1, "clicked", __button1_cb, cb1);
	evas_object_show(button1);

	evas_object_data_set(popup, CALLBACK_DATA_1, cb1);

	popup_callback *cb2 = NULL;
	if (button2_text) {
		Evas_Object *button2 = elm_button_add(popup);
		if (!button2) {
			BROWSER_LOGE("elm_button_add failed");
			free(cb1);
			cb1 = NULL;
			return NULL;
		}
		elm_object_text_set(button2, button2_text);
		elm_object_style_set(button2, "popup_button/default");
		elm_object_part_content_set(popup, "button2", button2);

		cb2 = (popup_callback *)malloc(sizeof(popup_callback));
		if (!cb2)
			return NULL;

		memset(cb2, 0x00, sizeof(popup_callback));
		cb2->func = button2_func;
		cb2->func_data = data;
		cb2->user_data = this;

		evas_object_smart_callback_add(button2, "clicked", __button2_cb, cb2);
		evas_object_show(button2);

		evas_object_data_set(popup, CALLBACK_DATA_2, cb2);
	}

	if (button3_text) {
		Evas_Object *button3 = elm_button_add(popup);
		if (!button3) {
			BROWSER_LOGE("elm_button_add failed");
			free(cb1);
			cb1 = NULL;
			free(cb2);
			cb2 = NULL;

			return NULL;
		}
		elm_object_text_set(button3, button3_text);
		elm_object_style_set(button3, "popup_button/default");
		elm_object_part_content_set(popup, "button3", button3);

		popup_callback *cb3 = (popup_callback *)malloc(sizeof(popup_callback));
		if (!cb3)
			return NULL;

		memset(cb3, 0x00, sizeof(popup_callback));
		cb3->func = button3_func;
		cb3->func_data = data;
		cb3->user_data = this;

		evas_object_smart_callback_add(button3, "clicked", __button3_cb, cb3);
		evas_object_show(button3);

		evas_object_data_set(popup, CALLBACK_DATA_3, cb3);
	}

	return popup;
}

void common_view::show_msg_popup(const char *msg, int timeout)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(msg);

	Evas_Object *popup = elm_popup_add(m_window);
	if (!popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return;
	}

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, msg);
	elm_popup_timeout_set(popup, timeout);
	evas_object_show(popup);
}

void common_view::show_noti_popup(const char *msg, int timeout)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(msg);

	Evas_Object *notify = elm_notify_add(m_window);
	if (!notify) {
		BROWSER_LOGE("elm_notify_add failed");
		return;
	}
	elm_notify_align_set(notify, 0.5, 1.0);
	evas_object_size_hint_weight_set(notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(notify, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *notify_layout = elm_layout_add(notify);
	if (!notify_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return;
	}

	if (elm_win_rotation_get(m_window) % 180)
		elm_layout_theme_set(notify_layout, "standard", "selectioninfo", "horizontal/bottom_64");
	else
		elm_layout_theme_set(notify_layout, "standard", "selectioninfo", "vertical/bottom_64");
	evas_object_size_hint_weight_set(notify_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(notify_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(notify, notify_layout);
	edje_object_part_text_set(elm_layout_edje_get(notify_layout), "elm.text", msg);
	if (timeout > 0)
		elm_notify_timeout_set(notify, timeout);

	evas_object_show(notify);
}

void common_view::show_delete_popup(const char *title, const char *msg, const char *button1_text, Evas_Smart_Cb button1_func, const char *button2_text, Evas_Smart_Cb button2_func, void *data)
{
	BROWSER_LOGD("title = [%s], msg = [%s]", title, msg);
	EINA_SAFETY_ON_NULL_RETURN(msg);

	Evas_Object *popup = elm_popup_add(m_window);
	if (!popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return;
	}
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title)
		elm_object_part_text_set(popup, "title,text", title);

	if (msg)
		elm_object_text_set(popup, msg);
	evas_object_show(popup);

	Evas_Object *button1 = elm_button_add(popup);
	if (!button1) {
		BROWSER_LOGE("elm_button_add failed");
		return;
	}

	if (button1_text)
		elm_object_text_set(button1, button1_text);
	else
		elm_object_text_set(button1, BR_STRING_OK);

	elm_object_style_set(button1, "sweep/delete");
	elm_object_part_content_set(popup, "button1", button1);

	popup_callback *cb1 = (popup_callback *)malloc(sizeof(popup_callback));
	if (!cb1)
		return;

	memset(cb1, 0x00, sizeof(popup_callback));
	cb1->func = button1_func;
	cb1->func_data = data;
	cb1->user_data = this;

	evas_object_smart_callback_add(button1, "clicked", __button1_cb, cb1);
	evas_object_show(button1);

	evas_object_data_set(popup, CALLBACK_DATA_1, cb1);

	popup_callback *cb2 = NULL;
	if (button2_text) {
		Evas_Object *button2 = elm_button_add(popup);
		if (!button2) {
			BROWSER_LOGE("elm_button_add failed");
			free(cb1);
			cb1 = NULL;
			return;
		}
		elm_object_text_set(button2, button2_text);
		elm_object_style_set(button2, "popup_button/default");
		elm_object_part_content_set(popup, "button2", button2);

		cb2 = (popup_callback *)malloc(sizeof(popup_callback));
		if (!cb2)
			return;

		memset(cb2, 0x00, sizeof(popup_callback));
		cb2->func = button2_func;
		cb2->func_data = data;
		cb2->user_data = this;

		evas_object_smart_callback_add(button2, "clicked", __button2_cb, cb2);
		evas_object_show(button2);

		evas_object_data_set(popup, CALLBACK_DATA_2, cb2);
	}
}

Eina_Bool common_view::launch_tizenstore(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (service_set_operation(service_handle, SERVICE_OPERATION_VIEW) < 0) {
		BROWSER_LOGE("Fail to set service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_set_uri(service_handle, uri) < 0) {
		BROWSER_LOGE("Fail to set service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_set_app_id(service_handle, tizen_store) < 0) {
		BROWSER_LOGE("Fail to service_set_app_id");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_streaming_player(const char *uri, const char *cookie)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	bool vt_call_check = false;
	if (app_manager_is_running(sec_vt_app, &vt_call_check) < 0) {
		BROWSER_LOGE("Fail to get app running information");
		return EINA_FALSE;
	}

	if (vt_call_check) {
		show_msg_popup(NULL, BR_STRING_WARNING_VIDEO_PLAYER);
		return EINA_FALSE;
	}

	service_h service_handle = NULL;

	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("service handle is NULL");
		return EINA_FALSE;
	}

	if (service_add_extra_data(service_handle, "path", uri) < 0) {
		BROWSER_LOGE("Fail to set extra data");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (cookie) {
		if (service_add_extra_data(service_handle, "cookie", cookie) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	}

	if (service_set_package(service_handle, sec_streaming_player) < 0) {
		BROWSER_LOGE("Fail to set package");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_message(const char *uri, const char *receiver, Eina_Bool file_attach)
{
	BROWSER_LOGD("uri = [%s], receiver = [%s]", uri, receiver);

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (file_attach) {
		if (service_set_operation(service_handle, SERVICE_OPERATION_SEND) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		if (uri && (strlen(uri) > 0)) {
			if (service_add_extra_data(service_handle, "ATTACHFILE", uri)) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}
	} else {
		if (service_set_operation(service_handle, SERVICE_OPERATION_SEND_TEXT) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		if (uri && (strlen(uri) > 0)) {
			if (service_add_extra_data(service_handle, SERVICE_DATA_TEXT, uri) < 0) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}
	}

	if (receiver && (strlen(receiver) > 0)) {
		if (service_add_extra_data(service_handle, SERVICE_DATA_TO , receiver) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_email(const char *uri, Eina_Bool file_attach)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (file_attach) {
		if (service_set_operation(service_handle, SERVICE_OPERATION_SEND) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		if (service_set_uri(service_handle, uri) < 0) {
			BROWSER_LOGE("Fail to set uri");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	} else {
		if (service_set_operation(service_handle, SERVICE_OPERATION_SEND_TEXT) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		if (strstr(uri, mailto_scheme)) {
			if (service_add_extra_data(service_handle, SERVICE_DATA_TO, uri + strlen(mailto_scheme)) < 0) {
				BROWSER_LOGE("Fail to set mailto data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		} else {
			if (service_add_extra_data(service_handle, SERVICE_DATA_TEXT, uri) < 0) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}
	}

	if (service_set_app_id(service_handle, sec_email_app) < 0) {
		BROWSER_LOGE("Fail to service_set_app_id");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(m_window);
	if (service_set_window(service_handle, win_id) < 0) {
		BROWSER_LOGE("Fail to service_set_window");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_bluetooth(const char *file_path)
{
	BROWSER_LOGD("file_path = [%s]", file_path);
	EINA_SAFETY_ON_NULL_RETURN_VAL(file_path, EINA_FALSE);

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (service_set_operation(service_handle, SERVICE_OPERATION_SEND) < 0) {
		BROWSER_LOGE("Fail to set service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_set_uri(service_handle, file_path) < 0) {
		BROWSER_LOGE("Fail to set service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_set_app_id(service_handle, sec_bluetooth_app) < 0) {
		BROWSER_LOGE("Fail to service_set_app_id");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(m_window);
	if (service_set_window(service_handle, win_id) < 0) {
		BROWSER_LOGE("Fail to service_set_window");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_contact(const char *phone_number)
{
	BROWSER_LOGD("phone_number = [%s]", phone_number);
	EINA_SAFETY_ON_NULL_RETURN_VAL(phone_number, EINA_FALSE);

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(m_window);
	if (service_set_window(service_handle, win_id) < 0) {
		BROWSER_LOGE("Fail to service_set_window");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	service_set_app_id(service_handle, sec_contact_app);

	if (service_add_extra_data(service_handle, "type", "22")) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_add_extra_data(service_handle, "ct_num", phone_number)) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_Snote(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	struct ug_cbs cbs = {0, };
	cbs.layout_cb = __ug_layout_cb;
	cbs.result_cb = NULL;
	cbs.destroy_cb = __ug_destroy_cb;
	cbs.priv = (void *)this;

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (service_add_extra_data(service_handle, "type", "insert") < 0) {
		BROWSER_LOGE("Fail to add extra data for [type]");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_add_extra_data(service_handle, "text", uri) < 0) {
		BROWSER_LOGE("Fail to add extra data for [text]");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (!ug_create(NULL, sec_snote_ug, UG_MODE_FULLVIEW, service_handle, &cbs))
		BROWSER_LOGE("ug_create is failed.");

	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::launch_phone_call(const char *number, Eina_Bool is_vt_call)
{
	BROWSER_LOGD("uri = [%s]", number);
	EINA_SAFETY_ON_NULL_RETURN_VAL(number, EINA_FALSE);

	service_h service_handle = NULL;	
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (is_vt_call) {
		if (service_add_extra_data(service_handle, "KEY_CALL_TYPE", "MO") < 0) {
			BROWSER_LOGE("Fail to set extra data : KEY_CALL_TYPE");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		if (service_add_extra_data(service_handle, "number", number) < 0) {
			BROWSER_LOGE("Fail to set extra data : number");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		if (service_set_package(service_handle, sec_vt_app) < 0) {
			BROWSER_LOGE("Fail to set package");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	} else {
		if (service_set_operation(service_handle, SERVICE_OPERATION_CALL) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		std::string request_number = std::string(tel_scheme) + std::string(number);

		if (service_set_uri(service_handle, request_number.c_str()) < 0) {
			BROWSER_LOGE("Fail to set uri");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::_make_html_for_BT_send(const char *uri, const char *file_path)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(file_path, EINA_FALSE);

	FILE *file_read = NULL;
	FILE *file_write = NULL;

	char *raw_data = NULL;
	long raw_data_size = 0;
	long raw_data_uri_size = 0;

	file_read = fopen(bludtooth_template_html_path, "r");
	if (!file_read) {
		BROWSER_LOGE("failed to open template_bluetooth_content_share.html");
		return EINA_FALSE;
	}

	fseek(file_read, 0, SEEK_END);
	raw_data_size = ftell(file_read);
	rewind(file_read);

	raw_data_uri_size = strlen(uri);
	raw_data = (char *)malloc(sizeof(char) * (raw_data_size + raw_data_uri_size + 1));
	if (!raw_data) {
		BROWSER_LOGE("Failed to allocate memory to read files");
		fclose(file_read);
		return EINA_FALSE;
	}
	memset(raw_data, 0x00, (sizeof(char) * (raw_data_size + raw_data_uri_size + 1)));
	raw_data[raw_data_size + raw_data_uri_size] = '\n';

	size_t result = fread(raw_data, sizeof(char), (raw_data_size + raw_data_uri_size), file_read);
	fclose(file_read);
	if (result != (size_t)raw_data_size) {
		BROWSER_LOGE("Reading error, result[%d]", result);
		free (raw_data);
		raw_data = NULL;
		return EINA_FALSE;
	}

	/* convert to markup, ex) & -> &amp; */
	char *markup_converted_uri = elm_entry_utf8_to_markup(uri);
	std::string uri_string;
	std::string raw_data_string = std::string(raw_data);
	std::string::size_type pos = std::string::npos;

	if(!strlen(markup_converted_uri)) {
		BROWSER_LOGE("failed to convert uri to markup");
		uri_string = std::string(uri);
	} else {
		BROWSER_LOGD("markup_converted_uri[%s]", markup_converted_uri);
		uri_string = std::string(markup_converted_uri);
	}

	free (raw_data);
	raw_data = NULL;

	free(markup_converted_uri);
	markup_converted_uri = NULL;

	while ((pos = raw_data_string.find(BLUETOOTH_SENDING_HTML_A_HREF_REPLACING_KEY)) != std::string::npos)
		raw_data_string.replace(pos, strlen(BLUETOOTH_SENDING_HTML_A_HREF_REPLACING_KEY), uri_string);

	while ((pos = raw_data_string.find(BLUETOOTH_SENDING_HTML_URI_REPLACING_KEY)) != std::string::npos)
		raw_data_string.replace(pos, strlen(BLUETOOTH_SENDING_HTML_URI_REPLACING_KEY), uri_string);

	file_write = fopen(file_path, "w");
	if (!file_write) {
		BROWSER_LOGE("fopen failed");
		return EINA_FALSE;
	}

	fwrite(raw_data_string.c_str(), sizeof(char), raw_data_string.length(), file_write);
	fclose(file_write);

	return EINA_TRUE;
}

void common_view::__call_response_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = (popup_callback *)data;
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
	else if (!strncmp(number, wtai_wp_mc_scheme, strlen(wtai_wp_mc_scheme)))
		offset = strlen(wtai_wp_mc_scheme);
	else
		offset = strlen(wtai_wp_sd_scheme);

	cv->launch_phone_call(number + offset, vt_call);

	free(cb->user_data);
	free(cb);
}

void common_view::__call_response_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	popup_callback *cb = (popup_callback *)data;

	free(cb->user_data);
	free(cb);
}

void common_view::_show_tel_vtel_popup(const char *number)
{
	BROWSER_LOGD("number = [%s]", number);
	EINA_SAFETY_ON_NULL_RETURN(number);

	int offset = 0;
	if (!strncmp(number, tel_scheme, strlen(tel_scheme)))
		offset = strlen(tel_scheme);
	else if (!strncmp(number, vtel_scheme, strlen(vtel_scheme)))
		offset = strlen(vtel_scheme);
	else if (!strncmp(number, telto_scheme, strlen(telto_scheme)))
		offset = strlen(telto_scheme);
	else if (!strncmp(number, wtai_wp_mc_scheme, strlen(wtai_wp_mc_scheme)))
		offset = strlen(wtai_wp_mc_scheme);
	else
		offset = strlen(wtai_wp_sd_scheme);

	std::string msg = std::string(BR_STRING_CALL) + std::string(number + offset) + std::string("?");

	popup_callback *cb = (popup_callback *)malloc(sizeof(popup_callback));
	if (!cb)
		return;

	memset(cb, 0x00, sizeof(popup_callback));
	cb->func_data = this;
	cb->user_data = strdup(number);

	show_msg_popup(NULL, msg.c_str(), BR_STRING_OK, __call_response_ok_cb, BR_STRING_CANCEL, __call_response_cancel_cb, cb);
}

Eina_Bool common_view::_handle_intent_scheme(const char *uri)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	std::string parameter = std::string(uri);
	std::string designated_pkg;
	std::string extra_data;
	service_h service_handle = NULL;

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

	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (service_set_app_id(service_handle, designated_pkg.c_str()) < 0) {
		BROWSER_LOGE("Fail to service_set_app_id");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (extra_data.length()) {
		if (service_add_extra_data(service_handle, "param", extra_data.c_str())) {
			BROWSER_LOGE("service_add_extra_data is failed.");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::_handle_tizen_service_scheme(const char *uri)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	std::string parameter = std::string(uri);
	std::string designated_pkg;
	std::string application_ID;
	std::string key_string;
	std::string value_string;
	service_h service_handle = NULL;

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

	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (application_ID.length()) {
		if (service_set_app_id(service_handle, application_ID.c_str()) < 0) {
			BROWSER_LOGE("Fail to service_set_app_id");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	} else if (designated_pkg.length()){
		if (service_set_package(service_handle, designated_pkg.c_str()) < 0) {
			BROWSER_LOGE("Fail to service_set_package");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	} else {
		BROWSER_LOGE("Fail to get pkg name");
		service_destroy(service_handle);
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
					if (service_add_extra_data(service_handle, key_string.c_str(), value_string.c_str())) {
						BROWSER_LOGE("service_add_extra_data is failed.");
						service_destroy(service_handle);
						return EINA_FALSE;
					}
				}
			} else
				has_more_data = EINA_FALSE;
		} else
			has_more_data = EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::_handle_mailto_scheme(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to service_create");
		return EINA_FALSE;
	}

	if (service_set_operation(service_handle, SERVICE_OPERATION_SEND) < 0) {
		BROWSER_LOGE("Fail to service_set_operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_set_uri(service_handle, uri) < 0) {
		BROWSER_LOGE("Fail to service_set_uri");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to service_send_launch_request");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::_handle_unknown_scheme(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to service_create");
		return EINA_FALSE;
	}

	if (service_set_operation(service_handle, SERVICE_OPERATION_VIEW) < 0) {
		BROWSER_LOGE("Fail to service_set_operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_set_uri(service_handle, uri) < 0) {
		BROWSER_LOGE("Fail to service_set_uri");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to service_send_launch_request");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool common_view::handle_scheme(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	if (!strncmp(uri, http_scheme, strlen(http_scheme)))
		return EINA_FALSE;
	else if (!strncmp(uri, https_scheme, strlen(https_scheme)))
		return EINA_FALSE;
	else if (!strncmp(uri, file_scheme, strlen(file_scheme)))
		return EINA_FALSE;
	else if (!strncmp(uri, rtsp_scheme, strlen(rtsp_scheme))) {
		launch_streaming_player(uri);
		return EINA_TRUE;
	} else if (!strncmp(uri, tizenstore_scheme, strlen(tizenstore_scheme))) {
		launch_tizenstore(uri);
		return EINA_TRUE;
	} else if (!strncmp(uri, mailto_scheme, strlen(mailto_scheme))) {
		_handle_mailto_scheme(uri);
		return EINA_TRUE;
	} else if (!strncmp(uri, sms_scheme, strlen(sms_scheme)) || !strncmp(uri, smsto_scheme, strlen(smsto_scheme))) {
		std::string uri_str = std::string(uri);
		std::string msg_body;
		const char *delimeter = "?body=";
		if (uri_str.find(delimeter) != std::string::npos) {
			msg_body = uri_str.substr(uri_str.find(delimeter));
			uri_str = uri_str.substr(0, uri_str.length() - msg_body.length());
		}

		if (!msg_body.empty()) {
			msg_body = std::string(msg_body.c_str() + strlen(delimeter));
			char *temp = g_uri_unescape_string(msg_body.c_str(), NULL);
			msg_body = temp;
			free(temp);
		}

		int offset = 0;
		if (!strncmp(uri, sms_scheme, strlen(sms_scheme)))
			offset = strlen(sms_scheme);
		else
			offset = strlen(smsto_scheme);

		std::string receiver = std::string(uri_str.c_str() + offset);
		BROWSER_LOGD("receiver = [%s]", receiver.c_str());
		launch_message(msg_body.c_str(), receiver.c_str());
		return EINA_TRUE;
	} else if (!strncmp(uri, mms_scheme, strlen(mms_scheme)) || !strncmp(uri, mmsto_scheme, strlen(mmsto_scheme))) {
		std::string uri_str = std::string(uri);
		std::string msg_body;
		const char *delimeter1 = "?body=";
		const char *delimeter2 = "&body=";
		const char *subject_delimeter = "?subject=";

		if (uri_str.find(delimeter1) != std::string::npos || uri_str.find(delimeter2) != std::string::npos) {
			if (uri_str.find(delimeter1) != std::string::npos)
				msg_body = uri_str.substr(uri_str.find(delimeter1));
			else
				msg_body = uri_str.substr(uri_str.find(delimeter2));

			uri_str = uri_str.substr(0, uri_str.length() - msg_body.length());
		}
		if (uri_str.find(subject_delimeter) != std::string::npos)
			uri_str = uri_str.substr(0, uri_str.length() - uri_str.substr(uri_str.find(subject_delimeter)).length());
		if (!msg_body.empty()) {
			msg_body = std::string(msg_body.c_str() + strlen(delimeter1));
			char *temp = g_uri_unescape_string(msg_body.c_str(), NULL);
			msg_body = temp;
			free(temp);
		}

		int offset = 0;
		if (!strncmp(uri, mms_scheme, strlen(mms_scheme)))
			offset = strlen(mms_scheme);
		else
			offset = strlen(mmsto_scheme);

		std::string receiver = std::string(uri_str.c_str() + offset);
		BROWSER_LOGD("receiver = [%s]", receiver.c_str());

		launch_message(msg_body.c_str(), receiver.c_str());
		return EINA_TRUE;
	} else if (!strncmp(uri, wtai_wp_ap_scheme, strlen(wtai_wp_ap_scheme))) {
		std::string number = std::string(uri + strlen(wtai_wp_ap_scheme));
		if (number.find(";") != std::string::npos) {
			number = number.substr(0, number.length() - number.substr(number.find(";")).length());
		}

		BROWSER_LOGD("phone number = [%s]", number.c_str());
		launch_contact(number.c_str());
		return EINA_TRUE;
	} else if (!strncmp(uri, tel_scheme, strlen(tel_scheme)) || !strncmp(uri, telto_scheme, strlen(telto_scheme))
		|| !strncmp(uri, vtel_scheme, strlen(vtel_scheme)) || !strncmp(uri, wtai_wp_mc_scheme, strlen(wtai_wp_mc_scheme))
		|| !strncmp(uri, wtai_wp_sd_scheme, strlen(wtai_wp_sd_scheme))) {
		_show_tel_vtel_popup(uri);
		return EINA_TRUE;
	} else if (!strncmp(uri, tizen_service_scheme, strlen(tizen_service_scheme))) {
		_handle_tizen_service_scheme(uri);
		return EINA_TRUE;
	}
#if 0
	else if (!strncmp(uri, intent_scheme, strlen(intent_scheme))) {
		_handle_intent_scheme(uri);
		return EINA_TRUE;
	}
#endif
	else if (strstr(uri, ":"))
		return _handle_unknown_scheme(uri);

	return EINA_FALSE;
}


