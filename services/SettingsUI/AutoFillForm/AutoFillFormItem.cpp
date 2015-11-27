#include "AutoFillFormManager.h"
#include "AutoFillFormItem.h"
#include "BrowserLogger.h"

//#include <efl_extension.h>
#include <regex.h>
#include "browser-string.h"

auto_fill_form_item::auto_fill_form_item(auto_fill_form_item_data *item_data)
{
	BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

	memset(&m_item_data, 0x00, sizeof(m_item_data));
	m_item_data.profile_id = -1; //The id starts with 0 from webkit

	if (item_data) {
		m_item_data.profile_id= item_data->profile_id;
		m_item_data.name = item_data->name;
		m_item_data.company = item_data->company;
		m_item_data.primary_address = item_data->primary_address;
		m_item_data.secondary_address = item_data->secondary_address;
		m_item_data.city_town = item_data->city_town;
		m_item_data.state_province_region = item_data->state_province_region;
		m_item_data.post_code = item_data->post_code;
		m_item_data.country = item_data->country;
		m_item_data.phone_number = item_data->phone_number;
		m_item_data.email_address = item_data->email_address;
		m_item_data.activation = item_data->activation;
		m_item_data.compose_mode = item_data->compose_mode;
	}
}

auto_fill_form_item::~auto_fill_form_item(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
}

profile_save_errorcode auto_fill_form_item::save_item(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	Ewk_Autofill_Profile *profile = ewk_autofill_profile_new();
	if (!profile) {
		BROWSER_LOGE("Failed to ewk_autofill_profile_new");
		return profile_create_failed;
	}

	if (m_item_data.name)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_NAME, m_item_data.name);

	if (m_item_data.company)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_COMPANY, m_item_data.company);

	if (m_item_data.primary_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS1, m_item_data.primary_address);

	if (m_item_data.secondary_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS2, m_item_data.secondary_address);

	if (m_item_data.city_town)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_CITY_TOWN, m_item_data.city_town);

	if (m_item_data.state_province_region)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_STATE_PROVINCE_REGION, m_item_data.state_province_region);

	if (m_item_data.post_code)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ZIPCODE, m_item_data.post_code);

	if (m_item_data.country)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_COUNTRY, m_item_data.country);

	if (m_item_data.phone_number)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_PHONE, m_item_data.phone_number);

	if (m_item_data.email_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_EMAIL, m_item_data.email_address);

	Ewk_Context *ewk_context = ewk_context_default_get();
	if (ewk_context_form_autofill_profile_add(ewk_context, profile) == EINA_FALSE) {
		BROWSER_LOGE("Failed to ewk_context_form_autofill_profile_add");
		ewk_autofill_profile_delete(profile);
		return duplicate_profile;
	}
	m_item_data.profile_id = ewk_autofill_profile_id_get(profile);
	ewk_autofill_profile_delete(profile);

	return save_error_none;
}

profile_edit_errorcode auto_fill_form_item::update_item(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	/* Find profile with id */
	Ewk_Context *ewk_context = ewk_context_default_get();
	Ewk_Autofill_Profile *profile = ewk_context_form_autofill_profile_get(ewk_context, m_item_data.profile_id);
	if (!profile) {
		BROWSER_LOGE("Failed to ewk_context_form_autofill_profile_get with ID [%d]", m_item_data.profile_id);
		return profile_edit_failed;
	}

	if (m_item_data.name)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_NAME, m_item_data.name);
	if (m_item_data.company)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_COMPANY, m_item_data.company);
	if (m_item_data.primary_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS1, m_item_data.primary_address);
	if (m_item_data.secondary_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS2, m_item_data.secondary_address);
	if (m_item_data.city_town)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_CITY_TOWN, m_item_data.city_town);
	if (m_item_data.state_province_region)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_STATE_PROVINCE_REGION, m_item_data.state_province_region);
	if (m_item_data.post_code)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ZIPCODE, m_item_data.post_code);
	if (m_item_data.country)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_COUNTRY, m_item_data.country);
	if (m_item_data.phone_number)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_PHONE, m_item_data.phone_number);
	if (m_item_data.email_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_EMAIL, m_item_data.email_address);

	if (ewk_context_form_autofill_profile_set(ewk_context, m_item_data.profile_id, profile) == EINA_FALSE) {
		BROWSER_LOGE("Failed to ewk_context_form_autofill_profile_set with ID [%d]", m_item_data.profile_id);
		return profile_already_exist;
	}

	return update_error_none;
}
