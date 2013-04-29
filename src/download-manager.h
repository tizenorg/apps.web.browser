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

#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include "browser-object.h"

#include <app_service.h>
#include <glib.h>
#include <glib-object.h>
#include <libsoup/soup.h>
#include <string>

typedef void (*download_finish_callback)(const char *file_path, void *data);

class download_manager : public browser_object {
public:
	download_manager(void);
	~download_manager(void);

	Eina_Bool launch_download_app(const char *uri, const char *cookie = NULL);
	void request_file_download(const char *uri, const char *file_path, download_finish_callback cb, void *data);
	/* If know the content type, use this method to handle it. */
	void handle_download_request(const char *uri, const char *cookie, const char *content_type);
private:
	Eina_Bool _show_avaiable_app_popup(const char *pkg_name, const char *uri, const char *cookie = NULL);

	static bool __app_matched_cb(service_h service_handle, const char *package, void *data);
	static void __sdp_cb(void *data, Evas_Object *obj, void *event_info);
	static void __internet_cb(void *data, Evas_Object *obj, void *event_info);
	static void __player_cb(void *data, Evas_Object *obj, void *event_info);

	static void __sdp_download_finished_cb(const char *file_path, void *data);
	static void __file_download_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data);

	std::string m_download_uri;
	std::string m_cookie;

	Eina_Bool m_is_matched_app;
};

#endif /* DOWNLOAD_MANAGER_H */

