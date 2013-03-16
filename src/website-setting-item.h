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

#ifndef WEBSITE_SETTING_ITEM_H
#define WEBSITE_SETTING_ITEM_H


#include "browser-object.h"
#include "common-view.h"
#include "website-setting-item-view.h"
#include "webview.h"

class website_setting_item : browser_object, public common_view {
public:
	website_setting_item(const char *uri);
	~website_setting_item(void);
	friend bool operator==(website_setting_item item1, website_setting_item item2) {
		return ((!item1.get_uri() && !item2.get_uri()) || !strcmp(item1.get_uri(), item2.get_uri()));
	}
	const char *get_title(void) { return m_title; }
	const char *get_uri(void) { return m_uri; }
	const char *get_date(void) { return m_date; }
	Eina_Bool check_has_web_storage_data(void) { return m_has_web_storage_data; }
	Eina_Bool check_has_geolocation_data(void) { return m_has_geolocation_data; }
	bool get_geolocation_allow(void) { return m_geolocation_allow; }
	Ewk_Security_Origin *get_origin(void) { return m_origin; }
	void *get_user_data(void) { return m_user_data; }
	website_setting_item_view *get_current_website_setting_item_view(void) { return m_item_view; }
	void set_title(const char *title);
	void set_date(const char *date);
	void set_has_geolocation_data(Eina_Bool has_geolocation_data) { m_has_geolocation_data = has_geolocation_data; }
	void set_has_web_storage_data(Eina_Bool has_web_storage_data) { m_has_web_storage_data = has_web_storage_data; }
	void set_geolocation_allow(bool geolocation_allow) { m_geolocation_allow = geolocation_allow; }
	void set_origin(Ewk_Security_Origin *origin) { m_origin = origin; }
	void set_user_data(void *user_data) { m_user_data = user_data; }
	void show(void);
	Eina_Bool delete_geolocation_item(const char *uri);
	Eina_Bool delete_web_storage_item(Ewk_Security_Origin *origin);
private:
	const char *m_uri;
	const char *m_title;
	const char *m_date;
	Eina_Bool m_has_web_storage_data;
	Eina_Bool m_has_geolocation_data;
	bool m_geolocation_allow;
	Eina_Bool m_need_update;
	Ewk_Security_Origin *m_origin;
	void *m_user_data;
	website_setting_item_view *m_item_view;
protected:
};
#endif /* WEBSITE_SETTING_ITEM_H */

