/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef BROWSER_META_TAG_H
#define BROWSER_META_TAG_H

#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <zlib.h>

#include "browser-config.h"
#include "browser-view.h"

class Browser_Meta_Tag {
public:
	Browser_Meta_Tag(void);
	~Browser_Meta_Tag(void);
	Eina_Bool init(Evas_Object *ewk_view);
	void deinit(void);

	Eina_Bool create_config_xml(const char *url, const char *title, const char *xml_path_attempt);
	Eina_Bool remove_config_xml(const char *xml_path_attempt);
	Eina_Bool request_download_icon(const char *url);
	Eina_Bool icon_remove(const char *icon_path_attempt);
	Eina_Bool wgt_install(const char *file_path);
private:
	Eina_Bool zip(const char *file_path_for_xml, const char* file_path_for_icon, const char* file_path_for_zipped_data);
	Eina_Bool unzip(const char *file_path_for_zipped_data);

	static void __download_icon_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data);
	static void __webapp_metatag_standalone_cb(void *data, Evas_Object *obj, void *event_info);
	static int __wgt_install_ret_cb(int req_id, const char *pkg_type,const char *pkg_name, const char *key,
                                    const char *val, const void *pmsg, void *data);
	std::string m_xml_path;
	std::string m_xml_law_data;
	std::string m_icon_path;

	Evas_Object *m_ewk_view;
};

#endif
