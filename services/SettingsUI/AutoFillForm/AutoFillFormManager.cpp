/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contact: Junghwan Kang <junghwan.kang@samsung.com>
 *
 */

#include "AutoFillFormManager.h"
#include "BrowserLogger.h"

#include <regex.h>
#include "browser-string.h"
#include "AutoFillFormItem.h"
#include "AutoFillFormListView.h"
#include "AutoFillFormComposeView.h"

#define USER_HOMEPAGE_ENTRY_MAX_COUNT 4096
#define HOMEPAGE_URLEXPR "((https?|ftp|gopher|telnet|file|notes|ms-help):((//)|(\\\\))[\\w\\d:#@%/;$()~_?+-=\\.&]+)"


auto_fill_form_manager::auto_fill_form_manager(void)
:
	m_list_view(NULL),
	m_composer(NULL)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	m_auto_fill_form_item_list = load_entire_item_list();
	ewk_context_form_autofill_profile_changed_callback_set(profiles_updated_cb, this);
}

auto_fill_form_manager::~auto_fill_form_manager(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	if (m_list_view)
		delete m_list_view;
	m_list_view = NULL;

	if(m_composer)
		delete m_composer;
	m_composer = NULL;
	ewk_context_form_autofill_profile_changed_callback_set(NULL, this);
}

void auto_fill_form_manager::profiles_updated_cb(void* data)
{
	auto_fill_form_manager *formManager = (auto_fill_form_manager*)data;
	formManager->refresh_items_view();
}

void auto_fill_form_manager::refresh_items_view()
{
	load_entire_item_list();
	if (m_list_view)
		m_list_view->refresh_view();
}

Eina_Bool auto_fill_form_manager::save_auto_fill_form_item(auto_fill_form_item_data *item_data)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RETV_MSG_IF(!item_data, EINA_FALSE, "key is NULL");
	RETV_MSG_IF(!item_data->name, EINA_FALSE, "key is NULL");

	return EINA_TRUE;
}

Eina_Bool auto_fill_form_manager::delete_auto_fill_form_item(auto_fill_form_item *item)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RETV_MSG_IF(!item, EINA_FALSE, "item is NULL");

	for (unsigned int i = 0; i < m_auto_fill_form_item_list.size(); i++) {
		if (m_auto_fill_form_item_list[i]->get_profile_id() == item->get_profile_id()) {

			Ewk_Context *ewk_context = ewk_context_default_get();
			if (ewk_context_form_autofill_profile_remove(ewk_context, item->get_profile_id()) == EINA_FALSE)
				return EINA_FALSE;

			m_auto_fill_form_item_list.erase(m_auto_fill_form_item_list.begin() + i);
		}
	}

	return EINA_TRUE;
}

Eina_Bool auto_fill_form_manager::delete_all_auto_fill_form_items(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	m_auto_fill_form_item_list.clear();

	Ewk_Context *ewk_context = ewk_context_default_get();
	Eina_List *entire_item_list = ewk_context_form_autofill_profile_get_all(ewk_context);

	Eina_List *list = NULL;
	void *item_data = NULL;

	EINA_LIST_FOREACH(entire_item_list, list, item_data) {
		if (item_data) {
			Ewk_Autofill_Profile *profile = (Ewk_Autofill_Profile *)item_data;
			ewk_context_form_autofill_profile_remove(ewk_context, ewk_autofill_profile_id_get(profile));
		}
	}

	return EINA_TRUE;
}

unsigned int auto_fill_form_manager::get_auto_fill_form_item_count(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	return m_auto_fill_form_item_list.size();
}

auto_fill_form_item *auto_fill_form_manager::create_new_auto_fill_form_item(Ewk_Autofill_Profile *profile)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	auto_fill_form_item *item = NULL;
	if (!profile)
		item = new auto_fill_form_item(NULL);
	else {
		auto_fill_form_item_data *item_data = (auto_fill_form_item_data *)malloc(sizeof(auto_fill_form_item_data));
		if (!item_data) {
			BROWSER_LOGE("Malloc failed to get item_data");
			return NULL;
		}
		memset(item_data, 0x00, sizeof(auto_fill_form_item_data));
		item_data->profile_id = ewk_autofill_profile_id_get(profile);
		item_data->name = ewk_autofill_profile_data_get(profile, EWK_PROFILE_NAME);
		item_data->company = ewk_autofill_profile_data_get(profile, EWK_PROFILE_COMPANY);
		item_data->primary_address = ewk_autofill_profile_data_get(profile, EWK_PROFILE_ADDRESS1);
		item_data->secondary_address = ewk_autofill_profile_data_get(profile, EWK_PROFILE_ADDRESS2);
		item_data->city_town = ewk_autofill_profile_data_get(profile, EWK_PROFILE_CITY_TOWN);
		item_data->state_province_region = ewk_autofill_profile_data_get(profile, EWK_PROFILE_STATE_PROVINCE_REGION);
		item_data->post_code = ewk_autofill_profile_data_get(profile, EWK_PROFILE_ZIPCODE);
		item_data->country = ewk_autofill_profile_data_get(profile, EWK_PROFILE_COUNTRY);
		item_data->phone_number = ewk_autofill_profile_data_get(profile, EWK_PROFILE_PHONE);
		item_data->email_address = ewk_autofill_profile_data_get(profile, EWK_PROFILE_EMAIL);
		item_data->activation = false;
		item_data->compose_mode = profile_edit;

		item = new auto_fill_form_item(item_data);
		free(item_data);
	}

	return item;
}

auto_fill_form_list_view *auto_fill_form_manager::show_list_view(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	if (m_list_view)
		delete m_list_view;

	m_list_view = new auto_fill_form_list_view(this);
	m_list_view->show();

	return m_list_view;
}

auto_fill_form_compose_view *auto_fill_form_manager::show_composer(auto_fill_form_item *item)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	if (m_composer)
		delete m_composer;

	m_composer = new auto_fill_form_compose_view(this, item);
	m_composer->show();

	return m_composer;
}

Eina_Bool auto_fill_form_manager::delete_list_view(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	if (m_list_view)
		delete m_list_view;
	m_list_view = NULL;

	return EINA_TRUE;
}

Eina_Bool auto_fill_form_manager::delete_composer(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	if (m_composer)
		delete m_composer;
	m_composer = NULL;

	return EINA_TRUE;
}

void auto_fill_form_manager::see_all_data(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	for (unsigned int i = 0; i < m_auto_fill_form_item_list.size(); i++) {
		BROWSER_LOGD("m_auto_fill_form_item_list[%d] item - start", i);
		BROWSER_LOGD("************************************************************************************");
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].id[%d]", i, m_auto_fill_form_item_list[i]->get_profile_id());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_name[%s]", i, m_auto_fill_form_item_list[i]->get_name());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_company[%s]", i, m_auto_fill_form_item_list[i]->get_company());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_primary_address[%s]", i, m_auto_fill_form_item_list[i]->get_primary_address());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_secondary_address[%s]", i, m_auto_fill_form_item_list[i]->get_secondary_address2());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_city_town[%s]", i, m_auto_fill_form_item_list[i]->get_city_town());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_state_province_region[%s]", i, m_auto_fill_form_item_list[i]->get_state_province());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_post_code[%s]", i, m_auto_fill_form_item_list[i]->get_post_code());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_country_region[%s]", i, m_auto_fill_form_item_list[i]->get_country());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_phone_number[%s]", i, m_auto_fill_form_item_list[i]->get_phone_number());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_email_address[%s]", i, m_auto_fill_form_item_list[i]->get_email_address());
		BROWSER_LOGD("m_auto_fill_form_item_list[%d].m_activation[%d]", i, m_auto_fill_form_item_list[i]->get_activation());
		BROWSER_LOGD("************************************************************************************");
		BROWSER_LOGD("m_auto_fill_form_item_list[%d] item - end", i);
	}

	return;
}

std::vector<auto_fill_form_item *> auto_fill_form_manager::load_entire_item_list(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	m_auto_fill_form_item_list.clear();

	Ewk_Context *ewk_context = ewk_context_default_get();
	Eina_List *entire_item_list = ewk_context_form_autofill_profile_get_all(ewk_context);

	Eina_List *list = NULL;
	void *item_data = NULL;

	EINA_LIST_FOREACH(entire_item_list, list, item_data) {
		if (item_data) {
			Ewk_Autofill_Profile *profile = (Ewk_Autofill_Profile *)item_data;
			auto_fill_form_item *item = create_new_auto_fill_form_item(profile);
			if (item)
				m_auto_fill_form_item_list.push_back(item);
		}
	}

	return m_auto_fill_form_item_list;
}

Eina_Bool auto_fill_form_manager::add_item_to_list(auto_fill_form_item *item)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RETV_MSG_IF(!item, EINA_FALSE, "item is NULL");

	m_auto_fill_form_item_list.push_back(item);
	m_list_view->refresh_view();

	return EINA_TRUE;;
}
