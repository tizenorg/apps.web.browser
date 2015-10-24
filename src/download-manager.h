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
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 */

#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include "browser-object.h"

#include <glib.h>
#include <libsoup/soup.h>
#include <string>

typedef void (*download_finish_callback)(const char *file_path, void *data);

class download_manager : public browser_object {
public:
	download_manager(void);
	~download_manager(void);

	Eina_Bool launch_download_app(const char *uri);
	void request_file_download(const char *uri, const char *file_path, download_finish_callback cb, void *data);
	/* If know the content type, use this method to handle it. */
	void handle_download_request(const char *uri, const char *content_type);

	Eina_Bool handle_data_scheme(const char *uri);
private:

	Eina_Bool _check_file_exist(const char *path);
	Eina_Bool _save_file(const char *raw_data, const char *path);
	Eina_Bool _update_contents_on_media_db(const char *path);

	/* should be freed from caller side */
	Eina_Bool _get_download_path(const char *extension, char **full_path, char **file_name);
	Eina_Bool _set_downloaded_file_on_notification(const char *downloaded_path, const char *file_name);

	static void __sdp_cb(void *data, Evas_Object *obj, void *event_info);
	static void __internet_cb(void *data, Evas_Object *obj, void *event_info);

	static void __sdp_download_finished_cb(const char *file_path, void *data);
	static void __file_download_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data);

	std::string m_download_uri;
};

#endif /* DOWNLOAD_MANAGER_H */

