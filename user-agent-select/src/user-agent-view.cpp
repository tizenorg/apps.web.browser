/*
 *  ug-browser-user-agent-efl
 *
 * Copyright (c) 2012 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *              Sangpyo Kim <sangpyo7.kim@samsung.com>
 *              Junghwan Kang <junghwan.kang@samsung.com>
 *              Inbum Chang <ibchang@samsung.com>
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

#include <Elementary.h>
#include <Ecore_X.h>
#include <dlog.h>
#include <vconf-internal-keys.h>
#include "user-agent-view.h"
#include "user-agent-db.h"
#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

#define USERAGENT_KEY	VCONFKEY_BROWSER_BROWSER_USER_AGENT

#define user_agent_list_style "1line.2"

#ifdef _
#undef _
#endif
#define _(s)	dgettext(PKG_NAME, s)

User_Agent_View::User_Agent_View(Evas_Object *win, ui_gadget_h gadget)
:	m_main_layout(NULL),
	m_win(win),
	m_navi(NULL),
	m_genlist(NULL),
	m_radio_main(NULL),
	m_selected_user_agent_index(0),
	m_item_class(NULL),
	m_user_agent_db(NULL),
	m_gadget(gadget)
{
	SLOGD("[%s]", __func__);
}

User_Agent_View::~User_Agent_View(void)
{
	SLOGD("[%s]", __func__);
	if (m_user_agent_db)
		delete m_user_agent_db;

	if (m_item_class)
		elm_genlist_item_class_free(m_item_class);
}

char *User_Agent_View::_gl_label_get(void *data, Evas_Object *obj, const char *part)
{
	genlist_callback_data *callback_data = (genlist_callback_data *)data;

	SLOGD("%s [%s]", __func__, part);

	if (!strcmp(part, "elm.text.main.left")) {
		if (!callback_data->user_agent.empty())
			return strdup((const char*)callback_data->user_agent.c_str());
	} else if (!strcmp(part, "elm.text")) {
		if (!callback_data->user_agent.empty())
			return strdup((const char*)callback_data->user_agent.c_str());
	}

	return NULL;
}

Evas_Object *User_Agent_View::_gl_icon_get(void *data, Evas_Object *obj, const char *part)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	SLOGD("%s [%s]", __func__, part);

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	User_Agent_View *user_agent_view = (User_Agent_View *)(callback_data->user_data);
	int index = callback_data->self_index;
	Evas_Object *radio_main = user_agent_view->m_radio_main;


	if (!strcmp(part, "elm.icon.1")) {
		Evas_Object *radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, index);
		elm_radio_group_add(radio, radio_main);
		if (index == user_agent_view->m_selected_user_agent_index)
			elm_radio_value_set(radio, index);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_propagate_events_set(radio, EINA_TRUE);

		return radio;
	}
	return NULL;
}

Eina_Bool User_Agent_View::init(void)
{
	m_user_agent_db = new(nothrow) User_Agent_DB();
	if (!m_user_agent_db) {
		SLOGE("new User_Agent_DB failed");
		return EINA_FALSE;
	}

	return _create_layout();
}

Eina_Bool User_Agent_View::_create_layout(void)
{
	SLOGD("[%s]", __func__);
	m_navi = elm_naviframe_add(m_win);
	if (!m_navi) {
		SLOGE("elm_naviframe_add failed");
		return EINA_FALSE;
	}

#if defined(HW_MORE_BACK_KEY)
	// Add HW Key callbacks.
	elm_naviframe_prev_btn_auto_pushed_set(m_navi, EINA_FALSE);
	eext_object_event_callback_add(m_navi, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(m_navi, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);
#endif

	m_genlist = elm_genlist_add(m_navi);
	if (!m_genlist) {
		SLOGE("elm_naviframe_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_radio_main = elm_radio_add(m_genlist);
	if (!m_radio_main) {
		SLOGE("elm_radio_add failed");
		return EINA_FALSE;
	}
	elm_radio_state_value_set(m_radio_main, 0);
	elm_radio_value_set(m_radio_main, 0);

	if (m_user_agent_db->get_user_agent_list(m_user_agent_list) == EINA_FALSE) {
		SLOGE("get_user_agent_list failed");
		return EINA_FALSE;
	}

	m_item_class = elm_genlist_item_class_new();

	if (!m_item_class) {
		SLOGE("elm_genlist_item_class_new failed");
		return EINA_FALSE;
	}

	m_item_class->item_style = user_agent_list_style;
	m_item_class->func.text_get = _gl_label_get;
	m_item_class->func.content_get = _gl_icon_get;
	m_item_class->func.state_get = NULL;
	m_item_class->func.del = NULL;

	SLOGD("List Size: [%d]", m_user_agent_list.size());
	if (m_user_agent_list.size()) {
		int i = 0;
		char *current_user_agent = vconf_get_str(USERAGENT_KEY);

		for (std::map<std::string, std::string>::const_iterator iter = m_user_agent_list.begin();
		    iter != m_user_agent_list.end(); ++iter, i++) {

			genlist_callback_data *callback_data = new(nothrow) genlist_callback_data;
			if (!callback_data) {
				SLOGE("new(nothrow) genlist_callback_data failed");
				return EINA_FALSE;
			}
			m_item_callback_data_list.push_back(callback_data);

			callback_data->user_data = this;
			callback_data->user_agent = (*iter).first;
			callback_data->self_index = i + 1;
			callback_data->it = elm_genlist_item_append(m_genlist, m_item_class, callback_data,
						NULL, ELM_GENLIST_ITEM_NONE, __item_selected_cb, callback_data);

			SLOGD("List Item is Added[%d]", callback_data->self_index);

			if (current_user_agent && !callback_data->user_agent.empty()) {
				if (!strcmp(callback_data->user_agent.c_str(), current_user_agent)) {
					m_selected_user_agent_index = callback_data->self_index;
					elm_genlist_item_bring_in(callback_data->it, ELM_GENLIST_ITEM_SCROLLTO_IN);
				}
			}
		}

		if(current_user_agent)
			free(current_user_agent);
		current_user_agent = NULL;
	}
	evas_object_show(m_genlist);

	Evas_Object *back_button = elm_button_add(m_genlist);
	elm_object_text_set(back_button, "Back");
//	evas_object_smart_callback_add(back_button, "clicked", __back_button_cb, this);
	elm_object_style_set(back_button, "naviframe/back_btn/default");
	Elm_Object_Item *it = elm_naviframe_item_push(m_navi, "User agent", back_button, NULL, m_genlist, NULL);
	elm_object_item_domain_text_translatable_set(it, "ug-browser-user-agent-efl", EINA_TRUE);

	elm_naviframe_item_pop_cb_set(it, __back_button_cb, this);

	return EINA_TRUE;
}

void User_Agent_View::__item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	EINA_SAFETY_ON_NULL_RETURN(data);
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	int index = callback_data->self_index;
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;

	if (item) {
		User_Agent_View *user_agent_view = (User_Agent_View *)(callback_data->user_data);
		Evas_Object *radio = elm_object_item_part_content_get(item, "elm.icon.1");

		user_agent_view->m_selected_user_agent_index = index;
		elm_radio_value_set(radio, index);
		elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);

		user_agent_view->_set_user_agent();
	}
}

Eina_Bool User_Agent_View::__back_button_cb(void *data, Elm_Object_Item *it)
{
	if (!data)
		return EINA_FALSE;

	User_Agent_View *user_agent_view = (User_Agent_View *)data;

	if (user_agent_view->m_navi)
		evas_object_del(user_agent_view->m_navi);
	user_agent_view->m_navi = NULL;

	ug_destroy_me(user_agent_view->m_gadget);

	return EINA_FALSE;
}

void User_Agent_View::__set_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	User_Agent_View *user_agent_view = (User_Agent_View *)data;
	user_agent_view->_set_user_agent();
}

void User_Agent_View::_set_user_agent(void)
{
	unsigned int i = 0;
	for (i = 0 ; i < m_user_agent_list.size() ; i++) {
		if (m_item_callback_data_list[i]->self_index == m_selected_user_agent_index)
			break;
	}

	if (i >= m_user_agent_list.size()) {
		char *user_agent = vconf_get_str(USERAGENT_KEY);
		for (i = 0 ; i < m_user_agent_list.size() ; i++) {
			if (user_agent) {
				if(!strcmp(m_item_callback_data_list[i]->user_agent.c_str(), user_agent))
					break;
			}
		}
		if (user_agent)
			free(user_agent);
	}

	if (vconf_set_str(USERAGENT_KEY, m_item_callback_data_list[i]->user_agent.c_str()) < 0)
		SLOGE("vconf_set_str failed");
}
