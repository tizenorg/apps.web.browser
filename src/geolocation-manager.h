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

#ifndef GEOLOCATION_H
#define GEOLOCATION_H

#include <Evas.h>
#include <vector>

#include "browser-object.h"

class geolocation_item;
class geolocation_manager : public browser_object {
public:
	geolocation_manager();
	~geolocation_manager();

	Eina_Bool save_geolocation(const char *uri, bool accept = false, const char *title = NULL);
	Eina_Bool get_geolocation_allow(const char *uri, bool &accept);
	Eina_Bool delete_geolocation_setting(const char *uri);
	Eina_Bool delete_all(void);
	void show_geolocation_allow_popup(const char *host_address, void *user_data);
	std::vector<geolocation_item *> get_geolocation_list(void);

	Eina_Bool check_geolocation_setting_exist(const char *uri);
	int get_count(void);

private:
	static void __request_geolocation_permission_ok_cb(void *data, Evas_Object *obj, void *event_info);
	static void __request_geolocation_permission_no_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_geolocation_never_ask_checkbox;
	void *m_user_data;
};

#endif	/* GEOLOCATION_H */

