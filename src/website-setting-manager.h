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

#ifndef WEBSITE_SETTING_MANAGER_H
#define WEBSITE_SETTING_MANAGER_H

#include <vector>

#include "browser-object.h"
#include "geolocation-manager.h"
#include "website-setting-item.h"

class website_setting_item;
class website_setting_manager : public browser_object {
public:
	website_setting_manager(void);
	~website_setting_manager(void);
	std::vector<website_setting_item *> get_website_setting_list(void);
	Eina_Bool delete_all_website_settings(void);
	Eina_Bool delete_all_geolocation_data(void);
	Eina_Bool delete_all_web_storate_data(void);
private:
	static void __application_cache_origin_get_cb(Eina_List* origins, void* user_data);
	static void __web_storage_origin_get_cb(Eina_List* origins, void* user_data);
	static void __web_database_origin_get_cb(Eina_List* origins, void* user_data);

	std::vector<website_setting_item *> m_website_setting_list;
	std::vector<geolocation_item *> m_geolocation_list;
protected:
};
#endif /* WEBSITE_SETTING_MANAGER_H */

