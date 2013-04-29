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

#include "user-agent-view.h"

#include <vconf.h>

#include "browser-dlog.h"
#include "browser-string.h"
#include "user-agent-manager.h"

user_agent_view::user_agent_view(void)
:
	m_tizen_ua_check(NULL)
	,m_chrome_ua_check(NULL)
	,m_item_ic(NULL)
{
	BROWSER_LOGD("");

}

user_agent_view::~user_agent_view(void)
{
	BROWSER_LOGD("");

	if (m_item_ic)
		elm_genlist_item_class_free(m_item_ic);

	evas_object_smart_callback_del(m_naviframe, "title,clicked", NULL/*__title_clicked_cb*/);
}

Eina_Bool user_agent_view::show(void)
{
	BROWSER_LOGD("");

	Elm_Object_Item *navi_it = NULL;
	Evas_Object *layout = _create_main_layout(m_naviframe);
	if (!layout) {
		BROWSER_LOGE("Failed to make layout");
		return EINA_FALSE;
	}
	navi_it = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, layout, NULL);

	Evas_Object *label = elm_label_add(m_naviframe);
	if (!label) {
		BROWSER_LOGE("elm_label_add failed.");
		return EINA_FALSE;
	}
	elm_object_style_set(label, "naviframe_title");
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_wrap_width_set(label, EINA_TRUE);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_object_text_set(label, BR_STRING_USER_AGENT);
	evas_object_show(label);
	elm_object_item_part_content_set(navi_it, "elm.swallow.title", label);

	evas_object_smart_callback_add(m_naviframe, "title,clicked", __title_clicked_cb, this);

	return EINA_TRUE;
}

Evas_Object *user_agent_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Elm_Object_Item *it = NULL;
	Evas_Object *genlist = _create_genlist(parent);
	if (!genlist) {
		BROWSER_LOGE("Failed to make done button");
		return NULL;
	}

	return genlist;
}

Evas_Object *user_agent_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	if (!item_ic) {
		BROWSER_LOGE("elm_genlist_item_class_new failed");
		return NULL;
	}
	memset(item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));

	item_ic->item_style = "1text.1icon.2";
	item_ic->func.text_get = __label_get_cb;
	item_ic->func.content_get = __content_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;
	m_item_ic = item_ic;

	m_tizen_ua_item.type = TIZEN;
	m_tizen_ua_item.user_data = this;
	m_tizen_ua_item.it = elm_genlist_item_append(genlist, m_item_ic, &m_tizen_ua_item, NULL,
												ELM_GENLIST_ITEM_NONE, __item_selected_cb, &m_tizen_ua_item);
	m_chrome_ua_item.type = CHROME;
	m_chrome_ua_item.user_data = this;
	m_chrome_ua_item.it = elm_genlist_item_append(genlist, m_item_ic, &m_chrome_ua_item, NULL,
												ELM_GENLIST_ITEM_NONE, __item_selected_cb, &m_chrome_ua_item);
	return genlist;
}

Evas_Object *user_agent_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	user_agent_type type = callback_data->type;
	user_agent_view *uav = (user_agent_view *)(callback_data->user_data);

	if (!strcmp(part, "elm.icon")) {
		if (type == TIZEN) {
			uav->m_tizen_ua_check = elm_check_add(obj);
			if (!uav->m_tizen_ua_check) {
				BROWSER_LOGE("elm_check_add failed");
				return NULL;
			}
			char *user_agent = vconf_get_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT);
			BROWSER_LOGD("TIZEN - user_agent[%s]", user_agent);
			if (!user_agent || strncmp(user_agent, chrome_user_agent_title, strlen(chrome_user_agent_title))) {
				elm_check_state_set(uav->m_tizen_ua_check, EINA_TRUE);
				if (user_agent)
					free(user_agent);
			}

			evas_object_propagate_events_set(uav->m_tizen_ua_check, EINA_FALSE);
			evas_object_smart_callback_add(uav->m_tizen_ua_check, "changed", __check_changed_cb, callback_data);

			return uav->m_tizen_ua_check;
		} else if (type == CHROME) {
			uav->m_chrome_ua_check = elm_check_add(obj);
			if (!uav->m_chrome_ua_check) {
				BROWSER_LOGE("elm_check_add failed");
				return NULL;
			}
			char *user_agent = vconf_get_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT);
			BROWSER_LOGD("CHROME - user_agent[%s]", user_agent);
			if (user_agent) {
				if (!strcmp(user_agent, chrome_user_agent_title))
					elm_check_state_set(uav->m_chrome_ua_check, EINA_TRUE);

				free(user_agent);
			}

			evas_object_propagate_events_set(uav->m_chrome_ua_check, EINA_FALSE);
			evas_object_smart_callback_add(uav->m_chrome_ua_check, "changed", __check_changed_cb, callback_data);

			return uav->m_chrome_ua_check;
		}
	}

	return NULL;
}

char *user_agent_view::__label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	user_agent_type type = callback_data->type;
	BROWSER_LOGD("type[%d]", type);
	if (!strcmp(part, "elm.text")) {
		if (type == TIZEN)
			return strdup(tizen_user_agent_title);
		else if (type == CHROME)
			return strdup(chrome_user_agent_title);
	}

	return NULL;
}

void user_agent_view::__item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	user_agent_view *uav = (user_agent_view *)(callback_data->user_data);
	user_agent_type type = callback_data->type;
	BROWSER_LOGD("type[%d]", type);

	if (type == TIZEN) {
		elm_check_state_set(uav->m_tizen_ua_check, EINA_TRUE);
		elm_check_state_set(uav->m_chrome_ua_check, EINA_FALSE);
		if (vconf_set_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT, tizen_user_agent_title) < 0)
			BROWSER_LOGE("vconf_set_str failed");
	} else {
		elm_check_state_set(uav->m_tizen_ua_check, EINA_FALSE);
		elm_check_state_set(uav->m_chrome_ua_check, EINA_TRUE);
		if (vconf_set_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT, chrome_user_agent_title) < 0)
			BROWSER_LOGE("vconf_set_str failed");
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void user_agent_view::__check_changed_cb( void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	EINA_SAFETY_ON_NULL_RETURN(obj);

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	user_agent_view *uav = (user_agent_view *)(callback_data->user_data);
	user_agent_type type = callback_data->type;
	Eina_Bool state = elm_check_state_get(obj);

	if (state) {
		if (type == TIZEN) {
			elm_check_state_set(uav->m_tizen_ua_check, EINA_TRUE);
			elm_check_state_set(uav->m_chrome_ua_check, EINA_FALSE);
			if (vconf_set_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT, tizen_user_agent_title) < 0)
				BROWSER_LOGE("vconf_set_str failed");
		} else {
			elm_check_state_set(uav->m_tizen_ua_check, EINA_FALSE);
			elm_check_state_set(uav->m_chrome_ua_check, EINA_TRUE);
			if (vconf_set_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT, chrome_user_agent_title) < 0)
				BROWSER_LOGE("vconf_set_str failed");
		}
	} else {
		if (type == TIZEN) {
			if (!elm_check_state_get(uav->m_chrome_ua_check))
				elm_check_state_set(uav->m_tizen_ua_check, EINA_TRUE);
		} else {
			if (!elm_check_state_get(uav->m_tizen_ua_check))
				elm_check_state_set(uav->m_chrome_ua_check, EINA_TRUE);
		}
	}
}

void user_agent_view::__title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	Elm_Object_Item *naviframe_item = (Elm_Object_Item *)event_info;
	Evas_Object *label = elm_object_item_part_content_get(naviframe_item, "elm.swallow.title");
	EINA_SAFETY_ON_NULL_RETURN(label);

	elm_label_slide_go(label);
}

