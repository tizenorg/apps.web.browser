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

#include <app.h>

#include "browser-dlog.h"
#include "browser-string.h"
#include "homepage-edit-view.h"
#include "platform-service.h"
#include "preference.h"
#include "setting-view.h"

homepage_edit_view::homepage_edit_view(void)
:
	m_item_ic(NULL)
	,m_editfield_layout(NULL)
	,m_save_button(NULL)
{
	BROWSER_LOGD("");
}

homepage_edit_view::~homepage_edit_view(void)
{
	BROWSER_LOGD("");

	if (m_item_ic)
		elm_genlist_item_class_free(m_item_ic);

	evas_object_smart_callback_del(m_naviframe, "title,clicked", __title_clicked_cb);

	evas_object_del(m_editfield_layout);
}

Eina_Bool homepage_edit_view::show(void)
{
	Elm_Object_Item *navi_it = NULL;
	Evas_Object *layout = _create_main_layout(m_naviframe);
	if (!layout) {
		BROWSER_LOGE("Failed to make layout");
		return EINA_FALSE;
	}
	navi_it = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, layout, NULL);

	m_save_button = elm_button_add(m_naviframe);
	if (!m_save_button) {
		BROWSER_LOGE("Failed to make done button");
		return EINA_FALSE;
	}
	elm_object_style_set(m_save_button, "naviframe/toolbar/default");
	elm_object_text_set(m_save_button, BR_STRING_SAVE);
	evas_object_smart_callback_add(m_save_button, "clicked", __save_button_clicked_cb, this);
	elm_object_item_part_content_set(navi_it, "toolbar_button1", m_save_button);

	Evas_Object *back_button = elm_object_item_part_content_get(navi_it, "prev_btn");
	evas_object_smart_callback_add(back_button, "clicked", __cancel_button_clicked_cb, this);

	Evas_Object *label = elm_label_add(m_naviframe);
	if (!label) {
		BROWSER_LOGE("elm_label_add failed.");
		return EINA_FALSE;
	}
	elm_object_style_set(label, "naviframe_title");
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_wrap_width_set(label, EINA_TRUE);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_object_text_set(label, BR_STRING_HOMEPAGE);
	evas_object_show(label);
	elm_object_item_part_content_set(navi_it, "elm.swallow.title", label);

	evas_object_smart_callback_add(m_naviframe, "title,clicked", __title_clicked_cb, this);

	return EINA_TRUE;
}

Evas_Object *homepage_edit_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Elm_Object_Item *it = NULL;
	Evas_Object *genlist = _create_genlist(parent);
	if (!genlist) {
		BROWSER_LOGE("Failed to make genlist");
		return NULL;
	}

	it = elm_genlist_item_append(genlist, m_item_ic, this, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, NULL);
	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_NONE);

	return genlist;
}

Evas_Object *homepage_edit_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	memset(item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));

	item_ic->item_style = "dialogue/1icon";
	item_ic->func.text_get = NULL;
	item_ic->func.content_get = __content_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;
	m_item_ic = item_ic;

	return genlist;
}

Evas_Object *homepage_edit_view::_create_edit_field(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	const char *user_homepage_url = m_preference->get_user_homagepage();
	platform_service ps;
	Evas_Object *entry = NULL;
	Evas_Object *editfield_layout = ps.editfield_add(parent, EINA_TRUE);
	ps.editfield_entry_single_line_set(editfield_layout, EINA_TRUE);

	entry = ps.editfield_entry_get(editfield_layout);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_URL);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	evas_object_smart_callback_add(entry, "activated", __save_button_clicked_cb, this);
	evas_object_smart_callback_add(entry, "changed", __entry_changed_cb, this);
	evas_object_smart_callback_add(entry, "preedit,changed", __entry_changed_cb, this);

	evas_object_show(editfield_layout);

	if (user_homepage_url && strlen(user_homepage_url))
		elm_entry_entry_set(entry, user_homepage_url);
	else
		elm_object_part_text_set(editfield_layout, "elm.guidetext", BR_STRING_ENTER_URL);

	ps.editfield_label_set(editfield_layout, BR_STRING_URL);
	ps.editfield_eraser_set(editfield_layout, EINA_TRUE);

	return editfield_layout;
}


/* callback functions */
Evas_Object *homepage_edit_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);

	if (!data)
		return NULL;

	homepage_edit_view *edit_view = (homepage_edit_view *)data;

	if (!strcmp(part, "elm.icon")) {
		edit_view->m_editfield_layout = edit_view->_create_edit_field(m_naviframe);
		return edit_view->m_editfield_layout;
	}

	return NULL;
}

void homepage_edit_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = elm_genlist_selected_item_get(obj);
	if (it == NULL) return;
	elm_genlist_item_selected_set(it, EINA_FALSE);
}

void homepage_edit_view::__save_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	EINA_SAFETY_ON_NULL_RETURN(obj);

	platform_service ps;
	homepage_edit_view *edit_view = NULL;
	edit_view = (homepage_edit_view *)data;

	Evas_Object *entry = ps.editfield_entry_get(edit_view->m_editfield_layout);
	char *homepage = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));

	if (!homepage) {
		BROWSER_LOGE("homepage is NULL");
		return;
	} else {
		BROWSER_LOGD("User homepage is set as [%s]", homepage);
		m_preference->set_user_homagepage((const char *)homepage);
	}

	if (homepage)
		free(homepage);

	/* To avoid give focus to entry before it destroyed in steps of closing homepage edit view */
	evas_object_del(entry);

	if (elm_naviframe_bottom_item_get(m_naviframe)
	    != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop(m_naviframe);
}

void homepage_edit_view::__cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	platform_service ps;
	homepage_edit_view *edit_view = (homepage_edit_view *)data;
	Evas_Object *entry = ps.editfield_entry_get(edit_view->m_editfield_layout);

	/* To avoid give focus to entry before it destroyed in steps of closing homepage edit view */
	evas_object_del(entry);

	if (elm_naviframe_bottom_item_get(m_naviframe)
	    != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop(m_naviframe);
}

void homepage_edit_view::__title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	Elm_Object_Item *naviframe_item = (Elm_Object_Item *)event_info;
	Evas_Object *label = elm_object_item_part_content_get(naviframe_item, "elm.swallow.title");
	EINA_SAFETY_ON_NULL_RETURN(label);

	elm_label_slide_go(label);
}

void homepage_edit_view::__entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	/* check that url is empty or not */
	platform_service ps;
	homepage_edit_view *edit_view = (homepage_edit_view *)data;
	Evas_Object *layout = edit_view->m_editfield_layout;
	Evas_Object *entry = ps.editfield_entry_get(edit_view->m_editfield_layout);
	char *homepage = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));

	/* make save button deactivate unless uri is in editfield */
	if (homepage && strlen(homepage)) {
		elm_object_disabled_set(edit_view->m_save_button, EINA_FALSE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
	} else {
		elm_object_disabled_set(edit_view->m_save_button, EINA_TRUE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
	}

	if (homepage)
		free(homepage);
}

