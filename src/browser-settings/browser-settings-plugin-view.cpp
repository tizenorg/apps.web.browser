/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#include "browser-settings-plugin-view.h"

Browser_Settings_Plugin_View::Browser_Settings_Plugin_View(Browser_Settings_Main_View *main_view)
:
	m_genlist(NULL)
	,m_back_button(NULL)
	,m_run_plugins_check_button(NULL)
	,m_main_view(main_view)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_Plugin_View::~Browser_Settings_Plugin_View(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_Settings_Plugin_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!_create_main_layout()) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Browser_Settings_Plugin_View::__run_plugins_check_button_changed_cb(void *data,
								Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Plugin_View *plugin_view = (Browser_Settings_Plugin_View *)data;

	const char *key = RUN_PLUGINS_KEY;
	Eina_Bool state = elm_check_state_get(obj);
	if (vconf_set_bool(key, state) != 0)
		BROWSER_LOGE("Key: [%s], FAILED\n", key);
}

Evas_Object *Browser_Settings_Plugin_View::__genlist_icon_get(void *data,
						Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Plugin_View::menu_type type = callback_data->type;
	Browser_Settings_Plugin_View *plugin_view = (Browser_Settings_Plugin_View *)callback_data->user_data;
	if (type == BR_RUN_PLUGINS) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			plugin_view->m_run_plugins_check_button = elm_check_add(obj);
			if (!plugin_view->m_run_plugins_check_button) {
				BROWSER_LOGD("elm_check_add failed");
				return NULL;
			}
			elm_object_style_set(plugin_view->m_run_plugins_check_button, "on&off");
			evas_object_propagate_events_set(plugin_view->m_run_plugins_check_button, EINA_FALSE);

			int run_plugins = 1;
			if (vconf_get_bool(RUN_PLUGINS_KEY, &run_plugins) < 0)
				BROWSER_LOGE("vconf_get_bool failed");
			elm_check_state_set(plugin_view->m_run_plugins_check_button, run_plugins);
			evas_object_smart_callback_add(plugin_view->m_run_plugins_check_button, "changed",
							__run_plugins_check_button_changed_cb, plugin_view);
			return plugin_view->m_run_plugins_check_button;
		}
	}

	return NULL;
}

char *Browser_Settings_Plugin_View::__genlist_label_get(void *data,
					Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Plugin_View::menu_type type = callback_data->type;
	if (type == BR_RUN_PLUGINS) {
		if (!strncmp(part, "elm.text", strlen("elm.text"))) {
			return strdup(BR_STRING_ENABLE_PLUGINS);
		}
	}
}

void Browser_Settings_Plugin_View::__item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Plugin_View::menu_type type = callback_data->type;
	Browser_Settings_Plugin_View *plugin_view = (Browser_Settings_Plugin_View *)callback_data->user_data;

	if (type == BR_RUN_PLUGINS) {
		int value = elm_check_state_get(plugin_view->m_run_plugins_check_button);
		elm_check_state_set(plugin_view->m_run_plugins_check_button, !value);

		__run_plugins_check_button_changed_cb(plugin_view, plugin_view->m_run_plugins_check_button, NULL);

		elm_genlist_item_selected_set(plugin_view->m_run_plugins_callback_data.it, EINA_FALSE);
	}
}

void Browser_Settings_Plugin_View::__back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (elm_naviframe_bottom_item_get(m_navi_bar)
	    != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

Eina_Bool Browser_Settings_Plugin_View::_create_main_layout(void)
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

	m_run_plugins_callback_data.type = BR_RUN_PLUGINS;
	m_run_plugins_callback_data.user_data = this;
	m_run_plugins_callback_data.it = elm_genlist_item_append(m_genlist, &m_1_text_1_icon_item_class, &m_run_plugins_callback_data,
							NULL, ELM_GENLIST_ITEM_NONE, __item_selected_cb, &m_run_plugins_callback_data);

	evas_object_show(m_genlist);

	m_back_button = elm_button_add(m_genlist);
	if (!m_back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_back_button, "browser/bookmark_controlbar_back");
	evas_object_show(m_back_button);
	evas_object_smart_callback_add(m_back_button, "clicked", __back_button_clicked_cb, this);

	Elm_Object_Item *navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_PLUGINS,
							m_back_button, NULL, m_genlist, "browser_titlebar");

	return EINA_TRUE;
}

