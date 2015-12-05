/*
 * AutoFillFormItem.h
 *
 *  Created on: Nov 16, 2015
 *      Author: youngj
 */

#ifndef AUTOFILLFORMITEM_H_
#define AUTOFILLFORMITEM_H_

#include "common.h"

class auto_fill_form_item {
public:
	auto_fill_form_item(auto_fill_form_item_data *item_data);
	~auto_fill_form_item(void);

	friend bool operator==(auto_fill_form_item item1, auto_fill_form_item item2) {
		return ((!item1.get_name() && !item2.get_name()) && !strcmp(item1.get_name(), item2.get_name()));
	}

	unsigned get_profile_id(void) { return m_item_data.profile_id; }
	const char *get_name(void) { return m_item_data.name; }
	const char *get_company(void) {return m_item_data.company; }
	const char *get_primary_address(void) {return m_item_data.primary_address; }
	const char *get_secondary_address2(void) { return m_item_data.secondary_address; }
	const char *get_city_town(void) { return m_item_data.city_town; }
	const char *get_state_province(void) { return m_item_data.state_province_region; }
	const char *get_post_code(void) { return m_item_data.post_code; }
	const char *get_country(void) { return m_item_data.country; }
	const char *get_phone_number(void) { return m_item_data.phone_number; }
	const char *get_email_address(void) { return m_item_data.email_address; }
	Eina_Bool get_activation(void) { return (m_item_data.activation == true) ? EINA_TRUE : EINA_FALSE; }
	profile_compose_mode get_item_compose_mode(void) { return m_item_data.compose_mode; }

	void set_name(const char *name) { m_item_data.name = name; }
	void set_company(const char *company) {m_item_data.company = company; }
	void set_primary_address(const char *primary_address) {m_item_data.primary_address = primary_address; }
	void set_secondary_address2(const char *secondary_address) { m_item_data.secondary_address = secondary_address; }
	void set_city_town(const char *city_town) { m_item_data.city_town = city_town; }
	void set_state_province(const char *state_province_region) { m_item_data.state_province_region = state_province_region; }
	void set_post_code(const char *post_code) { m_item_data.post_code = post_code; }
	void set_country(const char *country) { m_item_data.country = country; }
	void set_phone_number(const char *phone_number) { m_item_data.phone_number = phone_number; }
	void set_email_address(const char *email_address) { m_item_data.email_address = email_address; }
	void set_activation(Eina_Bool activation) { m_item_data.activation = (activation == EINA_TRUE) ? true : false;}

	profile_save_errorcode save_item(void);
	profile_edit_errorcode update_item(void);

private:
	auto_fill_form_item_data m_item_data;
};



#endif /* AUTOFILLFORMITEM_H_ */
