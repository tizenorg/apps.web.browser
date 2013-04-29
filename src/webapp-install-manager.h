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

#ifndef WEBAPP_INSTALL_MANAGER_H
#define WEBAPP_INSTALL_MANAGER_H

#include <libsoup/soup.h>

#include "browser.h"
#include "browser-object.h"
#include "common-view.h"

class webapp_install_manager : public browser_object, public common_view {
public:
	webapp_install_manager();
	~webapp_install_manager();

	Eina_Bool request_make_webapp(Evas_Object *ewk_view);
private:
	Eina_Bool _prepare_make_webapp(Evas_Object *ewk_view);
	Eina_Bool _make_webapp(const char *icon_data, const char *uri, const char *title);
	Eina_Bool _make_webapp_with_snapshot(const char *uri, const char *title);
	Eina_Bool _webapp_icon_uri_get(void);
	Eina_Bool _remove_webapp_icon(const char *icon_path_attempt);
	Eina_Bool _remove_config_xml(const char *xml_path_attempt);
	Eina_Bool _create_config_xml(const char *uri, const char *title);
	Eina_Bool _save_webapp_icon(const char *icon_data);
	Eina_Bool _download_webapp_icon(const char *icon_data);
	Eina_Bool _install_webapp(void);
	Evas_Object *_capture_snapshot(Evas_Object *ewk_view, float scale);
	Eina_Bool _save_icon_with_snapshot(Evas_Object *snapshot);

	static void __webapp_capable_get_cb(Eina_Bool capable, void *user_data);
	static void __webapp_icon_uri_get_cb(const char* icon_data, void* user_data);
	static void __download_webapp_icon_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data);
	static int __install_webapp_result_cb(int req_id, const char *pkg_type, const char *pkg_name, const char *key,
											const char *val, const void *pmsg, void *data);

	Evas_Object *m_ewk_view;
	Eina_Bool m_is_capable;
	Eina_Bool m_has_webapp_icon_normally;
	const char *m_uri;
	const char *m_title;
};

#endif /* WEBAPP_INSTALL_MANAGER_H */


