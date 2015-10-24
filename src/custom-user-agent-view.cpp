/*
 *  browser
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
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

#include "custom-user-agent-view.h"

#include <vconf.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "settings.h"
#include "webview.h"
#include "webview-list.h"
#include <efl_extension.h>
#include "platform-service.h"

#define CUSTOM_USER_AGENT_ENTRY_MAX_COUNT 4096

custom_user_agent_view::custom_user_agent_view(void)
:
	m_editfield_entry(NULL)
	,m_popup(NULL)
	,m_save_button(NULL)
	,m_cancel_button(NULL)
{
	BROWSER_LOGD("");
	memset(&m_editfield_data, 0x00, sizeof(m_editfield_data));
	m_entry_limit_size.max_char_count = 0;
	m_entry_limit_size.max_byte_count = CUSTOM_USER_AGENT_ENTRY_MAX_COUNT;
}

custom_user_agent_view::~custom_user_agent_view(void)
{
	BROWSER_LOGD("");

	if (m_editfield_data.item)
		elm_genlist_item_class_free(m_editfield_data.item);

	if (m_popup)
		evas_object_del(m_popup);
	m_popup = NULL;

//	evas_object_smart_callback_del(m_naviframe, "title,clicked", __title_clicked_cb);
}

Eina_Bool custom_user_agent_view::show(void)
{
	Elm_Object_Item *navi_it = NULL;
	Evas_Object *layout = _create_main_layout(m_naviframe);
	if (!layout) {
		BROWSER_LOGE("Failed to make layout");
		return EINA_FALSE;
	}
	navi_it = elm_naviframe_item_push(m_naviframe, BR_STRING_CUSTOM_USER_AGENT, NULL, NULL, layout, NULL);

	m_save_button = _create_title_text_btn(m_naviframe, BR_STR_ACBTN_DONE, __save_button_clicked_cb, this, EINA_FALSE);
	if (!m_save_button)
		return EINA_FALSE;
	elm_object_item_part_content_set(navi_it, "title_right_text_btn", m_save_button);

	elm_naviframe_item_pop_cb_set(navi_it, __back_button_clicked_cb, this);

	return EINA_TRUE;
}

Eina_Bool custom_user_agent_view::popup_edit_view_show(void)
{
	BROWSER_LOGD("");
	m_popup = brui_popup_add(m_browser->get_browser_view()->get_naviframe());
	RETV_MSG_IF(!m_popup, EINA_FALSE, "m_popup is NULL");

	elm_object_domain_translatable_part_text_set(m_popup, "title,text", BROWSER_DOMAIN_NAME, "IDS_BR_BODY_CUSTOM_USER_AGENT");

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(m_popup, EEXT_CALLBACK_BACK, __cancel_button_clicked_cb, this);
#endif
	evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* for margins */
	Evas_Object *layout = elm_layout_add(m_popup);
	elm_layout_file_set(layout,  browser_edj_dir"/browser-popup-lite.edj", "popup_input_text");
	elm_layout_theme_set(layout, "layout", "editfield", "singleline");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(m_popup, layout);

	m_entry_limit_size.max_char_count = 0;
	m_entry_limit_size.max_byte_count = USER_HOMEPAGE_ENTRY_MAX_COUNT;

	m_editfield_entry = elm_entry_add(layout);
	if (!m_editfield_entry) {
		BROWSER_LOGE("a_editfield_add is failed");
		return EINA_FALSE;
	}

	char *default_user_agent = (char *) m_browser->get_browser_view()->get_current_webview()->user_agent_get();
	if(!default_user_agent)
		return EINA_FALSE;

	elm_entry_markup_filter_append(m_editfield_entry, elm_entry_filter_limit_size, &m_entry_limit_size);
	evas_object_size_hint_weight_set(m_editfield_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_editfield_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	eext_entry_selection_back_event_allow_set(m_editfield_entry, EINA_TRUE);
	elm_entry_editable_set(m_editfield_entry, EINA_TRUE);
	elm_entry_cnp_mode_set(m_editfield_entry, ELM_CNP_MODE_PLAINTEXT);
	elm_entry_scrollable_set(m_editfield_entry, EINA_TRUE);
	elm_entry_autocapital_type_set(m_editfield_entry, ELM_AUTOCAPITAL_TYPE_NONE);
	elm_entry_prediction_allow_set(m_editfield_entry, EINA_FALSE);
	elm_entry_single_line_set(m_editfield_entry, EINA_TRUE);

#if defined(HW_MORE_BACK_KEY)
	eext_entry_selection_back_event_allow_set(m_editfield_entry, EINA_TRUE);
#endif

	if (strlen(default_user_agent))
		elm_object_text_set(m_editfield_entry, default_user_agent);
	else
		elm_object_part_text_set(m_editfield_entry, "elm.guide", BR_STRING_ENTER_URL);

	elm_entry_input_panel_layout_set(m_editfield_entry, ELM_INPUT_PANEL_LAYOUT_URL);
	elm_entry_input_panel_return_key_type_set(m_editfield_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	elm_entry_context_menu_clear(m_editfield_entry);
	elm_entry_context_menu_item_add(m_editfield_entry, "Clipboard", NULL, ELM_ICON_STANDARD, NULL, NULL);
	elm_object_part_content_set(layout, "elm.swallow.content" , m_editfield_entry);
	elm_entry_cursor_end_set(m_editfield_entry);

	if (!m_browser->is_keyboard_active())
		evas_object_smart_callback_add(m_editfield_entry, "activated", __popup_edit_save_button_clicked_cb, this);

	evas_object_smart_callback_add(m_editfield_entry, "changed", __popup_edit_entry_changed_cb, this);
	evas_object_smart_callback_add(m_editfield_entry, "preedit,changed", __popup_edit_entry_changed_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_popup);
	if (!cancel_button) {
		BROWSER_LOGE("elm_button_add for cancel_button is failed");
		return EINA_FALSE;
	}
	elm_object_domain_translatable_part_text_set(cancel_button, NULL, BROWSER_DOMAIN_NAME, "IDS_BR_SK_CANCEL");
	elm_object_style_set(cancel_button, "popup");
	elm_object_part_content_set(m_popup, "button1", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_button_clicked_cb, this);

	Evas_Object *ok_button = elm_button_add(m_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add for save_button is failed");
		return EINA_FALSE;
	}
	elm_object_domain_translatable_part_text_set(ok_button, NULL, BROWSER_DOMAIN_NAME, "IDS_BR_BUTTON_SET");
	elm_object_style_set(ok_button, "popup");
	elm_object_part_content_set(m_popup, "button2", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __popup_edit_save_button_clicked_cb, this);
	evas_object_show(m_popup);

	return EINA_TRUE;
}

void custom_user_agent_view::destroy_custom_user_agent_edit_popup_show(void)
{
	BROWSER_LOGD("");

	if (m_popup != NULL)
		evas_object_del(m_popup);
	m_popup = NULL;
}

Evas_Object *custom_user_agent_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = _create_genlist(parent);
	if (!genlist) {
		BROWSER_LOGE("Failed to make _create_genlist");
		return NULL;
	}

	m_editfield_data.it = elm_genlist_item_append(genlist, m_editfield_data.item, this, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(m_editfield_data.it, ELM_OBJECT_SELECT_MODE_NONE);

	return genlist;
}

Evas_Object *custom_user_agent_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	if(!item_ic)
		return NULL;
	item_ic->item_style = "entry.sub";
	item_ic->func.text_get = __text_get_cb;
	item_ic->func.content_get = __content_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

	m_editfield_data.item = item_ic;

	return genlist;
}

Evas_Object *custom_user_agent_view::_create_edit_field(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	char *default_user_agent = (char *) m_browser->get_browser_view()->get_current_webview()->user_agent_get();
	if (!default_user_agent) {
		BROWSER_LOGE("Failed to get user agent from VCONF with key [db/browser/user_agent]");
		return NULL;
	}

	Evas_Object *entry = elm_entry_add(parent);
	if (!entry) {
		BROWSER_LOGE("Failed to create entry.");
		free(default_user_agent);
		return NULL;
	}

	/* Not used in bleck theme */
	// elm_object_style_set(entry, "font_color_black");
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_prediction_allow_set(entry, EINA_FALSE);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_URL);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
#if defined(HW_MORE_BACK_KEY)
	eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
#endif

	if (default_user_agent && strlen(default_user_agent))
		elm_entry_entry_set(entry, default_user_agent);
	else
		elm_object_part_text_set(entry, "elm.guide", BR_STRING_ENTER_URL);

	evas_object_smart_callback_add(entry, "activated", __save_button_clicked_cb, this);
	evas_object_smart_callback_add(entry, "changed", __entry_changed_cb, this);
	evas_object_smart_callback_add(entry, "preedit,changed", __entry_changed_cb, this);
	evas_object_smart_callback_add(entry, "focused", __entry_focused_cb, this);
	evas_object_smart_callback_add(entry, "unfocused", __entry_unfocused_cb, this);

	free(default_user_agent);

	return entry;
}


/* callback functions */
char *custom_user_agent_view::__text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	if (!strcmp(part, "elm.text"))
		return strdup(BR_STRING_CUSTOM_USER_AGENT);

	return NULL;
}

Evas_Object *custom_user_agent_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	custom_user_agent_view *edit_view = (custom_user_agent_view *)data;

	if (!strcmp(part, "elm.icon.edit")) {
		Evas_Object *icon = elm_button_add(obj);
		if (!icon) {
			BROWSER_LOGE("Failed to elm_button_add.");
			return NULL;
		}
//		elm_object_style_set(icon, "minus");
		evas_object_propagate_events_set(icon, EINA_FALSE);
	} else if (!strcmp(part, "elm.icon.entry")) {
		edit_view->m_editfield_entry = edit_view->_create_edit_field(obj);
		return edit_view->m_editfield_entry;
	} else if (!strcmp(part, "elm.icon.eraser")) {
		Evas_Object *btn = elm_button_add(obj);
		if (!btn) {
			BROWSER_LOGE("Failed to elm_button_add.");
			return NULL;
		}
		elm_object_style_set(btn, "editfield_clear");
		evas_object_smart_callback_add(btn, "clicked", __eraser_clicked_cb, data);
		return btn;
	}

	return NULL;
}

void custom_user_agent_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = elm_genlist_selected_item_get(obj);
	if (it == NULL) return;
	elm_genlist_item_selected_set(it, EINA_FALSE);
}

void custom_user_agent_view::__save_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	custom_user_agent_view *edit_view = (custom_user_agent_view *)data;
	char *custom_user_agent = elm_entry_markup_to_utf8(elm_entry_entry_get(edit_view->m_editfield_entry));

	if (custom_user_agent)
		free(custom_user_agent);

	/* To avoid give focus to entry before it destroyed in steps of closing homepage edit view */
	evas_object_del(edit_view->m_editfield_entry);
	edit_view->m_editfield_entry = NULL;

	if (elm_naviframe_bottom_item_get(m_naviframe) != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop(m_naviframe);
}

Eina_Bool custom_user_agent_view::__back_button_clicked_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");

	custom_user_agent_view *edit_view = (custom_user_agent_view *)data;
	/* To avoid give focus to entry before it destroyed in steps of closing homepage edit view */
	evas_object_del(edit_view->m_editfield_entry);
	edit_view->m_editfield_entry = NULL;

	return EINA_TRUE;
}

void custom_user_agent_view::__cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	custom_user_agent_view *edit_view = (custom_user_agent_view *)data;
	edit_view->destroy_custom_user_agent_edit_popup_show();
}

void custom_user_agent_view::__eraser_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	custom_user_agent_view *edit_view = (custom_user_agent_view *)data;
	Evas_Object *entry = elm_object_item_part_content_get(edit_view->m_editfield_data.it, "elm.icon.entry");
	elm_object_focus_set(entry, EINA_TRUE);
	elm_entry_entry_set(entry, "");
}

void custom_user_agent_view::__title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *naviframe_item = (Elm_Object_Item *)event_info;
	Evas_Object *label = elm_object_item_part_content_get(naviframe_item, "elm.swallow.title");
	RET_MSG_IF(!label, "label is NULL");

	elm_label_slide_go(label);
}

void custom_user_agent_view::__entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	/* check that url is empty or not */
	custom_user_agent_view *edit_view = (custom_user_agent_view *)data;

	if (elm_object_focus_get(obj)) {
		if (elm_entry_is_empty(obj))
			elm_object_item_signal_emit(edit_view->m_editfield_data.it, "elm,state,eraser,hide", "");
		else
			elm_object_item_signal_emit(edit_view->m_editfield_data.it, "elm,state,eraser,show", "");
	}

	/* make save button deactivate unless uri is in editfield */
	char *user_agent = elm_entry_markup_to_utf8(elm_entry_entry_get(edit_view->m_editfield_entry));
	if (user_agent && strlen(user_agent)) {
		elm_object_disabled_set(edit_view->m_save_button, EINA_FALSE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
	} else {
		elm_object_disabled_set(edit_view->m_save_button, EINA_TRUE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
	}

	if (user_agent)
		free(user_agent);
}

void custom_user_agent_view::__entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	custom_user_agent_view *edit_view = (custom_user_agent_view *)data;

	if (!elm_entry_is_empty(obj))
		elm_object_item_signal_emit(edit_view->m_editfield_data.it, "elm,state,eraser,show", "");

	elm_object_item_signal_emit(edit_view->m_editfield_data.it, "elm,state,rename,hide", "");
}

void custom_user_agent_view::__entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	custom_user_agent_view *edit_view = (custom_user_agent_view *)data;

	elm_object_item_signal_emit(edit_view->m_editfield_data.it, "elm,state,eraser,hide", "");
	elm_object_item_signal_emit(edit_view->m_editfield_data.it, "elm,state,rename,show", "");
}

void custom_user_agent_view::__popup_edit_save_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	custom_user_agent_view *edit_view = (custom_user_agent_view *)data;

	Evas_Object *entry = edit_view->m_editfield_entry;
	char *custom_user_agent_uri = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));

	if (!custom_user_agent_uri) {
		BROWSER_LOGE("custom_user_agent_uri is NULL");
		return;
	}
	int count = m_browser->get_webview_list()->get_count();
	for (int i = 0 ; i < count ; i++){
		if(!m_browser->get_webview_list()->get_webview(i)){
		        if (custom_user_agent_uri)
		                free(custom_user_agent_uri);
			return;
		}
		m_browser->get_webview_list()->get_webview(i)->user_agent_set(custom_user_agent_uri);
	}
	m_browser->get_browser_view()->get_current_webview()->reload();

	if (custom_user_agent_uri)
		free(custom_user_agent_uri);

	/* To avoid give focus to entry before it destroyed in steps of closing homepage edit view */
	evas_object_del(edit_view->m_editfield_entry);
	edit_view->m_editfield_entry = NULL;

	edit_view->destroy_custom_user_agent_edit_popup_show();
}

void custom_user_agent_view::__popup_edit_entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	/* check that url is empty or not */
	custom_user_agent_view *edit_view = (custom_user_agent_view *)data;
	char *custom_user_agent_uri = elm_entry_markup_to_utf8(elm_entry_entry_get(edit_view->m_editfield_entry));

	/* make save button deactivate unless uri is in editfield */
	if (custom_user_agent_uri && strlen(custom_user_agent_uri)) {
		elm_object_disabled_set(edit_view->m_save_button, EINA_FALSE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
		edit_view->show_notification(custom_user_agent_uri, obj, CUSTOM_USER_AGENT_ENTRY_MAX_COUNT);
	} else {
		elm_object_disabled_set(edit_view->m_save_button, EINA_TRUE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
	}

	if (custom_user_agent_uri)
		free(custom_user_agent_uri);
}

