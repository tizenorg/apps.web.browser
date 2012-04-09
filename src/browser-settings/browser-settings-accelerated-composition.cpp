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

#include "browser-settings-accelerated-composition.h"

Browser_Settings_Accelerated_Composition::Browser_Settings_Accelerated_Composition(Browser_Settings_Main_View *main_view)
:
	m_genlist(NULL)
	,m_back_button(NULL)
	,m_accelerated_composition_check_button(NULL)
	,m_external_video_player_check_button(NULL)
	,m_main_view(main_view)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_Accelerated_Composition::~Browser_Settings_Accelerated_Composition(void)
{
	BROWSER_LOGD("[%s]", __func__);	
}

Eina_Bool Browser_Settings_Accelerated_Composition::init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!_create_main_layout()) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Browser_Settings_Accelerated_Composition::__accelerated_composition_check_button_changed_cb(void *data,
								Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Accelerated_Composition *acceleration_composition_view = (Browser_Settings_Accelerated_Composition *)data;

	const char *key = ACCELERATED_COMPOSITION_KEY;
	Eina_Bool state = elm_check_state_get(obj);

	if (vconf_set_bool(key, state) != 0)
		BROWSER_LOGE("Key: [%s], FAILED\n", key);

	if(!state) {
		/* Accelerated composition OFF, external video ON. external Video buttons must be disabled. CompositedRenderLayerBorders must be disabled.*/
		elm_check_state_set(acceleration_composition_view->m_external_video_player_check_button, state);
		if(vconf_set_bool(EXTERNAL_VIDEO_PLAYER_KEY, 1)!=0)
			BROWSER_LOGE("Key: %s, FAILED\n", EXTERNAL_VIDEO_PLAYER_KEY);
		elm_check_state_set(acceleration_composition_view->m_external_video_player_check_button, EINA_TRUE);
		elm_object_disabled_set(acceleration_composition_view->m_external_video_player_check_button, EINA_TRUE);
		elm_object_item_disabled_set(acceleration_composition_view->m_accelerated_external_video_player_callback_data.it, EINA_TRUE);
	} else {
		/* Accelerated composition ON , external video OFF */
		elm_object_disabled_set(acceleration_composition_view->m_external_video_player_check_button, EINA_FALSE);
		elm_object_item_disabled_set(acceleration_composition_view->m_accelerated_external_video_player_callback_data.it, EINA_FALSE);
	}
}

void Browser_Settings_Accelerated_Composition::__external_video_player_check_button_changed_cb(void *data,
								Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Accelerated_Composition *acceleration_composition_view = (Browser_Settings_Accelerated_Composition *)data;

	const char *key = EXTERNAL_VIDEO_PLAYER_KEY;
	Eina_Bool state = elm_check_state_get(obj);
	if (vconf_set_bool(key, state) != 0)
		BROWSER_LOGE("Key: [%s], FAILED\n", key);
}
Evas_Object *Browser_Settings_Accelerated_Composition::__genlist_icon_get(void *data,
						Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Accelerated_Composition::menu_type type = callback_data->type;
	Browser_Settings_Accelerated_Composition *acceleration_composition_view = (Browser_Settings_Accelerated_Composition *)callback_data->user_data;

	if (type == BR_ACCELERATED_COMPOSITION) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			acceleration_composition_view->m_accelerated_composition_check_button = elm_check_add(obj);
			if (!acceleration_composition_view->m_accelerated_composition_check_button) {
				BROWSER_LOGD("elm_check_add failed");
				return NULL;
			}
			elm_object_style_set(acceleration_composition_view->m_accelerated_composition_check_button, "on&off");
			evas_object_propagate_events_set(acceleration_composition_view->m_accelerated_composition_check_button, EINA_FALSE);

			int accelerated_composition = 1;
			if (vconf_get_bool(ACCELERATED_COMPOSITION_KEY, &accelerated_composition) < 0)
				BROWSER_LOGE("vconf_get_bool failed");
			elm_check_state_set(acceleration_composition_view->m_accelerated_composition_check_button, accelerated_composition);
			evas_object_smart_callback_add(acceleration_composition_view->m_accelerated_composition_check_button, "changed",
							__accelerated_composition_check_button_changed_cb, acceleration_composition_view);
			return acceleration_composition_view->m_accelerated_composition_check_button;
		}
	}
	else if (type == BR_EXTERNAL_VIDEO_PLAYER) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			acceleration_composition_view->m_external_video_player_check_button = elm_check_add(obj);
			if (!acceleration_composition_view->m_external_video_player_check_button) {
				BROWSER_LOGD("elm_check_add failed");
				return NULL;
			}
			elm_object_style_set(acceleration_composition_view->m_external_video_player_check_button, "on&off");
			evas_object_propagate_events_set(acceleration_composition_view->m_external_video_player_check_button, EINA_FALSE);

			int external_video_player = 1;
			if (vconf_get_bool(EXTERNAL_VIDEO_PLAYER_KEY, &external_video_player) < 0)
				BROWSER_LOGE("vconf_get_bool failed");
			elm_check_state_set(acceleration_composition_view->m_external_video_player_check_button, external_video_player);
			evas_object_smart_callback_add(acceleration_composition_view->m_external_video_player_check_button, "changed",
							__external_video_player_check_button_changed_cb, acceleration_composition_view);
			return acceleration_composition_view->m_external_video_player_check_button;
		}
	}

	return NULL;
}

char *Browser_Settings_Accelerated_Composition::__genlist_label_get(void *data,
					Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Accelerated_Composition::menu_type type = callback_data->type;
	if(type == BR_ACCELERATED_COMPOSITION) {
		if (!strncmp(part, "elm.text", strlen("elm.text"))) {
			return strdup(_("Accelerated composition"));
		}
	}
	else if(type == BR_EXTERNAL_VIDEO_PLAYER) {
		if (!strncmp(part, "elm.text", strlen("elm.text"))) {
			return strdup(_("External video player"));
		}
	}
}

void Browser_Settings_Accelerated_Composition::__item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Accelerated_Composition::menu_type type = callback_data->type;
	Browser_Settings_Accelerated_Composition *acceleration_composition_view = (Browser_Settings_Accelerated_Composition *)callback_data->user_data;

	if  (type == BR_ACCELERATED_COMPOSITION) {
		int value = elm_check_state_get(acceleration_composition_view->m_accelerated_composition_check_button);
		elm_check_state_set(acceleration_composition_view->m_accelerated_composition_check_button, !value);

		__accelerated_composition_check_button_changed_cb(acceleration_composition_view, acceleration_composition_view->m_accelerated_composition_check_button, NULL);

		elm_genlist_item_selected_set(acceleration_composition_view->m_accelerated_composition_callback_data.it, EINA_FALSE);
	}
	else if  (type == BR_EXTERNAL_VIDEO_PLAYER) {
		int value = elm_check_state_get(acceleration_composition_view->m_external_video_player_check_button);
		elm_check_state_set(acceleration_composition_view->m_external_video_player_check_button, !value);

		__external_video_player_check_button_changed_cb(acceleration_composition_view, acceleration_composition_view->m_external_video_player_check_button, NULL);

		elm_genlist_item_selected_set(acceleration_composition_view->m_accelerated_external_video_player_callback_data.it, EINA_FALSE);
	}
}

void Browser_Settings_Accelerated_Composition::__back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (elm_naviframe_bottom_item_get(m_navi_bar)
	    != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

Eina_Bool Browser_Settings_Accelerated_Composition::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_genlist = elm_genlist_add(m_navi_bar);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}

	m_seperator_item_class.item_style = "dialogue/seperator";
	m_seperator_item_class.func.text_get = NULL;
	m_seperator_item_class.func.content_get = NULL;
	m_seperator_item_class.func.del = NULL;
	m_seperator_item_class.func.state_get = NULL;

	Elm_Object_Item *it = elm_genlist_item_append(m_genlist, &m_seperator_item_class,
						NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_1_text_1_icon_item_class.item_style = "dialogue/1text.1icon";
	m_1_text_1_icon_item_class.func.text_get = __genlist_label_get;
	m_1_text_1_icon_item_class.func.content_get = __genlist_icon_get;
	m_1_text_1_icon_item_class.func.del = NULL;
	m_1_text_1_icon_item_class.func.state_get = NULL;

	m_accelerated_composition_callback_data.type = BR_ACCELERATED_COMPOSITION;
	m_accelerated_composition_callback_data.user_data = this;
	m_accelerated_composition_callback_data.it = elm_genlist_item_append(m_genlist, &m_1_text_1_icon_item_class, &m_accelerated_composition_callback_data,
							NULL, ELM_GENLIST_ITEM_NONE, __item_selected_cb, &m_accelerated_composition_callback_data);

	m_accelerated_external_video_player_callback_data.type = BR_EXTERNAL_VIDEO_PLAYER;
	m_accelerated_external_video_player_callback_data.user_data = this;
	m_accelerated_external_video_player_callback_data.it = elm_genlist_item_append(m_genlist, &m_1_text_1_icon_item_class, &m_accelerated_external_video_player_callback_data,
							NULL, ELM_GENLIST_ITEM_NONE, __item_selected_cb, &m_accelerated_external_video_player_callback_data);

	int acceleratedComposition = 1;
	if(vconf_get_bool(ACCELERATED_COMPOSITION_KEY, &acceleratedComposition) < 0)
		BROWSER_LOGE("Can not get [%s] value.\n", __func__, ACCELERATED_COMPOSITION_KEY);

	if(!acceleratedComposition)
		elm_object_item_disabled_set(m_accelerated_external_video_player_callback_data.it, EINA_TRUE);

	evas_object_show(m_genlist);

	m_back_button = elm_button_add(m_navi_bar);
	if (!m_back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_back_button, "browser/bookmark_controlbar_back");
	evas_object_show(m_back_button);
	evas_object_smart_callback_add(m_back_button, "clicked", __back_button_clicked_cb, this);

	Elm_Object_Item *navi_it = elm_naviframe_item_push(m_navi_bar, _("Accelerated composition"),
							m_back_button, NULL, m_genlist, "browser_titlebar");

	return EINA_TRUE;
}

