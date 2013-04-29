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

#include "clear-private-data-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "history.h"
#include "webview.h"

clear_private_data_view::clear_private_data_view(void)
:
	m_content_box(NULL)
	,m_select_all_layout(NULL)
	,m_genlist(NULL)
	,m_delete_button(NULL)
	,m_item_ic(NULL)
{
	BROWSER_LOGD("");
}

clear_private_data_view::~clear_private_data_view(void)
{
	BROWSER_LOGD("");

	if (m_item_ic)
		elm_genlist_item_class_free(m_item_ic);

	if (m_select_all_item_ic)
		elm_genlist_item_class_free(m_select_all_item_ic);

	evas_object_smart_callback_del(m_naviframe, "title,clicked", __title_clicked_cb);
}

Eina_Bool clear_private_data_view::show(void)
{
	BROWSER_LOGD("");

	Elm_Object_Item *navi_it = NULL;
	m_content_box = _create_main_layout(m_naviframe);
	if (!m_content_box) {
		BROWSER_LOGE("Failed to make layout");
		return EINA_FALSE;
	}
	navi_it = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, m_content_box, NULL);

	m_delete_button = elm_button_add(m_naviframe);
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_delete_button, EINA_FALSE);

	elm_object_text_set(m_delete_button, BR_STRING_DELETE);
	evas_object_smart_callback_add(m_delete_button, "clicked", __delete_button_cb, this);

	elm_object_style_set(m_delete_button, "naviframe/toolbar/default");
	elm_object_item_part_content_set(navi_it, "toolbar_button1", m_delete_button);
	evas_object_show(m_delete_button);

	elm_object_disabled_set(m_delete_button, EINA_TRUE);

	Evas_Object *label = elm_label_add(m_naviframe);
	if (!label) {
		BROWSER_LOGE("elm_label_add failed.");
		return EINA_FALSE;
	}
	elm_object_style_set(label, "naviframe_title");
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_ALWAYS);
	elm_label_wrap_width_set(label, EINA_TRUE);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_object_text_set(label, BR_STRING_CLEAR_PERSONALISED_DATA);
	evas_object_show(label);
	elm_object_item_part_content_set(navi_it, "elm.swallow.title", label);

	evas_object_smart_callback_add(m_naviframe, "title,clicked", __title_clicked_cb, this);

	return EINA_TRUE;

}

Evas_Object *clear_private_data_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *content_box = elm_box_add(parent);
	if (!content_box) {
		BROWSER_LOGE("elm_box_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(content_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_select_all_layout = _create_select_all_layout(content_box);
	evas_object_show(m_select_all_layout);
	elm_box_pack_end(content_box, m_select_all_layout);

	m_genlist = _create_genlist(content_box);
	if (!m_genlist) {
		BROWSER_LOGE("Failed to make done button");
		return NULL;
	}
	evas_object_show(m_genlist);
	elm_box_pack_end(content_box, m_genlist);
	evas_object_show(content_box);

	return content_box;
}

Evas_Object *clear_private_data_view::_create_select_all_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("Failed to elm_layout_add");
		return NULL;
	}

	elm_layout_theme_set(layout, "genlist", "item", "select_all/default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_DOWN, __select_all_layout_down_cb, this);
	evas_object_show(layout);

	Evas_Object *check = elm_check_add(layout);
	if (!check) {
		BROWSER_LOGE("Failed to elm_check_add");
		return NULL;
	}

	evas_object_propagate_events_set(check, EINA_FALSE);
	evas_object_smart_callback_add(check, "changed", __select_all_chceckbox_changed_cb, this);
	elm_object_part_content_set(layout, "elm.icon", check);
	elm_object_part_text_set(layout, "elm.text", BR_STRING_SELECT_ALL);

	return layout;
}

Evas_Object *clear_private_data_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* Select all ic */
	Elm_Object_Item *it = NULL;
	Elm_Genlist_Item_Class *select_all_item_ic = elm_genlist_item_class_new();
	if (!select_all_item_ic) {
		BROWSER_LOGE("elm_genlist_item_class_new failed");
		return NULL;
	}
	memset(select_all_item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));
	select_all_item_ic->item_style = "selectall_check";
	select_all_item_ic->func.text_get = __genlist_label_get_cb;
	select_all_item_ic->func.content_get = __genlist_icon_get_cb;
	select_all_item_ic->func.state_get = NULL;
	select_all_item_ic->func.del = NULL;
	m_select_all_item_ic = select_all_item_ic;

	it = elm_genlist_item_append(genlist, select_all_item_ic, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	if (!item_ic) {
		BROWSER_LOGE("elm_genlist_item_class_new failed");
		return NULL;
	}
	memset(item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));

	item_ic->item_style = "1text.1icon.2";
	item_ic->decorate_all_item_style = "edit_default";
	item_ic->func.text_get = __genlist_label_get_cb;
	item_ic->func.content_get = __genlist_icon_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;
	m_item_ic = item_ic;

	m_clear_history_data.user_data = (void *)this;
	m_clear_history_data.type = CELAR_PRIVATE_DATA_VIEW_CLEAR_HISTORY;

	m_clear_cache_data.user_data = (void *)this;
	m_clear_cache_data.type = CELAR_PRIVATE_DATA_VIEW_CLEAR_CACHE;

	m_clear_cookie_data.user_data = (void *)this;
	m_clear_cookie_data.type = CELAR_PRIVATE_DATA_VIEW_CLEAR_COOKIE;

	m_clear_saved_password_data.user_data = (void *)this;
	m_clear_saved_password_data.type = CELAR_PRIVATE_DATA_VIEW_CLEAR_SAVED_PASSWORD;

	m_clear_history_data.it = elm_genlist_item_append(genlist, m_item_ic, &m_clear_history_data, NULL, ELM_GENLIST_ITEM_NONE,
													__genlist_item_selected_cb, &m_clear_history_data);
	m_clear_cache_data.it = elm_genlist_item_append(genlist, m_item_ic, &m_clear_cache_data, NULL, ELM_GENLIST_ITEM_NONE,
													__genlist_item_selected_cb, &m_clear_cache_data);
	m_clear_cookie_data.it = elm_genlist_item_append(genlist, m_item_ic, &m_clear_cookie_data, NULL, ELM_GENLIST_ITEM_NONE,
													__genlist_item_selected_cb, &m_clear_cookie_data);
	m_clear_saved_password_data.it = elm_genlist_item_append(genlist, m_item_ic, &m_clear_saved_password_data, NULL, ELM_GENLIST_ITEM_NONE,
															__genlist_item_selected_cb, &m_clear_saved_password_data);

	return genlist;
}

void clear_private_data_view::_delete_private_data(void)
{
	BROWSER_LOGD("");

	if (elm_check_state_get(m_clear_history_data.checkbox)) {
		m_browser->get_history()->delete_all();
	}
	if (elm_check_state_get(m_clear_cache_data.checkbox)) {
		Ewk_Context *ewk_context = ewk_context_default_get();
		ewk_context_cache_clear(ewk_context);
		ewk_context_web_indexed_database_delete_all(ewk_context);
		ewk_context_application_cache_delete_all(ewk_context);
		ewk_context_web_storage_delete_all(ewk_context);
		ewk_context_web_database_delete_all(ewk_context);
	}
	if (elm_check_state_get(m_clear_cookie_data.checkbox)) {
		Ewk_Context *ewk_context = ewk_context_default_get();
		ewk_cookie_manager_cookies_clear(ewk_context_cookie_manager_get(ewk_context));
	}
	if (elm_check_state_get(m_clear_saved_password_data.checkbox)) {
		/* Not yet */
	}
}

char *clear_private_data_view::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

	if (!strcmp(part, "elm.text")) {
		genlist_callback_data *callback_data = (genlist_callback_data *)data;
		clear_private_data_view *cpdv = (clear_private_data_view*)callback_data->user_data;
		menu_type type = callback_data->type;

		switch (type) {
		case CELAR_PRIVATE_DATA_VIEW_CLEAR_HISTORY:
			return strdup(BR_STRING_HISTORY);
		case CELAR_PRIVATE_DATA_VIEW_CLEAR_CACHE:
			return strdup(BR_STRING_CLEAR_CACHE);
		case CELAR_PRIVATE_DATA_VIEW_CLEAR_COOKIE:
			return strdup(BR_STRING_COOKIES);
		case CELAR_PRIVATE_DATA_VIEW_CLEAR_SAVED_PASSWORD:
			return strdup(BR_STRING_CLEAR_PASSWORDS);
		case CELAR_PRIVATE_DATA_VIEW_SELECT_ALL:
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
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
		genlist_callback_data *callback_data = (genlist_callback_data *)data;
		clear_private_data_view *cpdv = (clear_private_data_view *)callback_data->user_data;

		Evas_Object *checkbox = NULL;
		checkbox = elm_check_add(obj);
		if (!checkbox) {
			BROWSER_LOGE("Failed to add clear_history_check");
			return NULL;
		}
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
	EINA_SAFETY_ON_NULL_RETURN(data);
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);

	Evas_Object *checkbox = elm_object_item_part_content_get(item, "elm.icon");
	Eina_Bool state = elm_check_state_get(checkbox);
	elm_check_state_set(checkbox, !state);

	genlist_callback_data *gcd = (genlist_callback_data *)data;
	clear_private_data_view *cpdv = (clear_private_data_view *)(gcd->user_data);
	EINA_SAFETY_ON_NULL_RETURN(cpdv);

	__checkbox_changed_cb((void *)cpdv, NULL, NULL);
}

void clear_private_data_view::__select_all_chceckbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	EINA_SAFETY_ON_NULL_RETURN(obj);

	clear_private_data_view *cpdv = (clear_private_data_view *)data;
	Eina_Bool state = elm_check_state_get(obj);

	if (state)
		elm_object_disabled_set(cpdv->m_delete_button, EINA_FALSE);
	else
		elm_object_disabled_set(cpdv->m_delete_button, EINA_TRUE);

	Elm_Object_Item *it = elm_genlist_first_item_get(cpdv->m_genlist);
	while (it) {
		int index = (int)elm_object_item_data_get(it);
		Evas_Object *ck = elm_object_item_part_content_get(it, "elm.icon");
		if (ck)
			elm_check_state_set(ck, state);
		it = elm_genlist_item_next_get(it);
	}
}

void clear_private_data_view::__select_all_layout_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	EINA_SAFETY_ON_NULL_RETURN(obj);

	clear_private_data_view *cpdv = (clear_private_data_view *)data;

	Evas_Object *check = elm_object_part_content_get(cpdv->m_select_all_layout, "elm.icon");
	Eina_Bool state = elm_check_state_get(check);
	elm_check_state_set(check, !state);
	__select_all_chceckbox_changed_cb(data, check, NULL);
}

void clear_private_data_view::__checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	clear_private_data_view *cpdv = (clear_private_data_view *)data;
	/* Set select all checkbox */
	Evas_Object *select_all_check = elm_object_part_content_get(cpdv->m_select_all_layout, "elm.icon");
	Eina_Bool all_selected = EINA_TRUE;
	Elm_Object_Item *it = elm_genlist_first_item_get(cpdv->m_genlist);
	it = elm_genlist_item_next_get(it);
	while (it) {
		Evas_Object *ck = elm_object_item_part_content_get(it, "elm.icon");
		if (ck) {
			if (elm_check_state_get(ck) == EINA_FALSE) {
				all_selected = EINA_FALSE;
				break;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	elm_check_state_set(select_all_check, all_selected);

	/* Set delete button status */
	it = elm_genlist_first_item_get(cpdv->m_genlist);
	while (it) {
		Evas_Object *ck = elm_object_item_part_content_get(it, "elm.icon");
		if (ck) {
			if (elm_check_state_get(ck) == EINA_TRUE) {
				elm_object_disabled_set(cpdv->m_delete_button, EINA_FALSE);
				return;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	elm_object_disabled_set(cpdv->m_delete_button, EINA_TRUE);
}

void clear_private_data_view::__delete_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	clear_private_data_view *cpdv = (clear_private_data_view *)data;

	m_browser->get_browser_view()->show_msg_popup(NULL,
												BR_STRING_DELETE_Q,
												BR_STRING_YES,
												cpdv->__delete_ok_cb,
												BR_STRING_NO,
												NULL,
												cpdv);

}

void clear_private_data_view::__delete_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	clear_private_data_view *cpdv = (clear_private_data_view*)data;
	cpdv->_delete_private_data();

	elm_object_disabled_set(cpdv->m_delete_button, EINA_TRUE);

	if (elm_naviframe_bottom_item_get(m_naviframe) != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop(m_naviframe);
}

void clear_private_data_view::__title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	Elm_Object_Item *naviframe_item = (Elm_Object_Item *)event_info;
	Evas_Object *label = elm_object_item_part_content_get(naviframe_item, "elm.swallow.title");
	EINA_SAFETY_ON_NULL_RETURN(label);

	elm_label_slide_go(label);
}

