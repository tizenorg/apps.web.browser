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

#include "browser-personal-data-manager.h"

Browser_Personal_Data_Manager::Browser_Personal_Data_Manager(void)
:	m_webview(NULL)
	,m_popup(NULL)
{
	BROWSER_LOGD("[%s]", __func__);

	m_personal_data_db = new(nothrow) Browser_Personal_Data_DB;
	if (!m_personal_data_db) {
		BROWSER_LOGE("new Browser_Personal_Data_DB failed");
	}
}

Browser_Personal_Data_Manager::~Browser_Personal_Data_Manager(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_popup)
		evas_object_del(m_popup);

	if (m_personal_data_db)
		delete m_personal_data_db;
}

Eina_Bool Browser_Personal_Data_Manager::clear_personal_data(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_personal_data_db) {
		return m_personal_data_db->clear_personal_data();
	}

	return EINA_FALSE;
}

personal_data_save_mode Browser_Personal_Data_Manager::_get_save_mode(void)
{
	BROWSER_LOGD("[%s]", __func__);
	char *save_mode = vconf_get_str(AUTO_SAVE_ID_PASSWORD_KEY);
	if (!save_mode) {
		BROWSER_LOGE("vconf_get_str failed");
		return SAVE_PERSONAL_DATA_ALWAYS_ASK;
	}

	personal_data_save_mode mode;
	if (!strncmp(save_mode, ALWAYS_ASK, strlen(ALWAYS_ASK))) {
		BROWSER_LOGD("SAVE_PERSONAL_DATA_ALWAYS_ASK");
		mode = SAVE_PERSONAL_DATA_ALWAYS_ASK;
	} else if (!strncmp(save_mode, ALWAYS_ON, strlen(ALWAYS_ON))) {
		BROWSER_LOGD("SAVE_PERSONAL_DATA_ON");
		mode = SAVE_PERSONAL_DATA_ON;
	} else {
		BROWSER_LOGD("SAVE_PERSONAL_DATA_OFF");
		mode = SAVE_PERSONAL_DATA_OFF;
	}

	free(save_mode);

	return mode;
}

Eina_Bool Browser_Personal_Data_Manager::set_personal_data(const char *url)
{
	if (_get_save_mode() == SAVE_PERSONAL_DATA_OFF || !m_personal_data_db) {
		BROWSER_LOGE("SAVE_PERSONAL_DATA_OFF");
		return EINA_FALSE;
	}

	std::string user_name;
	std::string password;
	std::string domain = get_domain_name(url);
	if (!m_personal_data_db->get_personal_data(user_name, password, domain)) {
		BROWSER_LOGE("get_personal_data failed");
		return EINA_FALSE;
	}

	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	if (!ewk_view_autofill_personal_data(webkit, user_name.c_str(), password.c_str()))
		BROWSER_LOGE("ewk_view_autofill_personal_data failed");
}

Eina_Bool Browser_Personal_Data_Manager::_save_personal_data(std::string user_name,
						std::string password, std::string url)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_personal_data_db)
		return m_personal_data_db->save_personal_data(url, user_name, password);

	return EINA_FALSE;
}

int Browser_Personal_Data_Manager::_show_ask_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_popup)
		evas_object_del(m_popup);

	m_popup = elm_popup_add(m_navi_bar);
	if (!m_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return 1;
	}

	evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	std::string confirm_msg = std::string("Remember private data?");
	elm_object_text_set(m_popup, confirm_msg.c_str());

	evas_object_show(m_popup);

	evas_object_focus_set(m_popup, EINA_TRUE);

	return 1;
}

void Browser_Personal_Data_Manager::__submit_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Personal_Data_Manager *manager = (Browser_Personal_Data_Manager *)data;
	const char **personal_data = (const char **)event_info;
	std::string user_name = personal_data[0];
	std::string password = personal_data[1];
	std::string url = manager->get_domain_name(personal_data[2]);
	BROWSER_LOGD("user_name=[%s], password=[%s], url=[%s]", user_name.c_str(), password.c_str(), url.c_str());

	personal_data_save_mode save_mode = manager->_get_save_mode();
	if (save_mode == SAVE_PERSONAL_DATA_OFF) {
		BROWSER_LOGD("SAVE_PERSONAL_DATA_OFF");
		return;
	} else if (save_mode == SAVE_PERSONAL_DATA_ON) {
		BROWSER_LOGD("SAVE_PERSONAL_DATA_ON");
		if (!manager->_save_personal_data(user_name, password, url))
			BROWSER_LOGE("_save_personal_data failed");
	} else {
			if (!manager->_save_personal_data(user_name, password, url))
				BROWSER_LOGE("_save_personal_data failed");
		evas_object_del(manager->m_popup);
		manager->m_popup = NULL;
	}
}

void Browser_Personal_Data_Manager::init(Evas_Object *webview)
{
	BROWSER_LOGD("[%s]", __func__);
	deinit();

	m_webview = webview;	
	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	evas_object_smart_callback_add(ewk_view_frame_main_get(webkit),
				"submit,clicked", __submit_clicked_cb, this);
}

void Browser_Personal_Data_Manager::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_webview) {
		Evas_Object *webkit = elm_webview_webkit_get(m_webview);
		evas_object_smart_callback_del(ewk_view_frame_main_get(webkit),
					"submit,clicked", __submit_clicked_cb);
	}
}

