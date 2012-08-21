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


#include "browser-class.h"
#include "browser-meta-tag.h"

#include <fcntl.h>
#include <libsoup/soup.h>
#include <package-manager.h>

#define TIME_STAMP_MAX_LENGTH		255
#define XML_RAW_DATA_MAX_SIZE		4096
#define PATH_CONFIGURE_SAMPLE_XML	"/opt/apps/org.tizen.browser/data/config_sample.xml"
#define PATH_CONFIGURE_XML			"/opt/apps/org.tizen.browser/data/config.xml"
#define PATH_ICON_PNG				"/opt/apps/org.tizen.browser/data/icon.png"

Browser_Meta_Tag::Browser_Meta_Tag(void)
:
	m_ewk_view(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Meta_Tag::~Browser_Meta_Tag(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_Meta_Tag::init(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	deinit();

	m_ewk_view = ewk_view;

	m_xml_path = std::string(PATH_CONFIGURE_XML);
	m_icon_path = std::string(PATH_ICON_PNG);

	remove_config_xml(m_xml_path.c_str());

	evas_object_smart_callback_add(m_ewk_view, "webapp,metatag,standalone", __webapp_metatag_standalone_cb, this);

	return EINA_TRUE;
}

void Browser_Meta_Tag::deinit()
{
	evas_object_smart_callback_del(m_ewk_view, "webapp,metatag,standalone", __webapp_metatag_standalone_cb);
}

Eina_Bool Browser_Meta_Tag::create_config_xml(const char *url, const char *title, const char *xml_path_attempt)
{
	BROWSER_LOGD("[%s]", __func__);

	char *xml_data = NULL;
	FILE *file_read = NULL;
	FILE *file_write = NULL;
	long xml_data_size = 0;
	long xml_extra_data_size = 0;

	/* Read raw data from default config_sample.xml */
	file_read = fopen(PATH_CONFIGURE_SAMPLE_XML, "r");

	if (!file_read) {
		BROWSER_LOGE("failed to open config_sample.xml");
		return EINA_FALSE;
	}

	fseek(file_read, 0, SEEK_END);
	xml_data_size = ftell(file_read);
	rewind(file_read);

	xml_extra_data_size = strlen(url) + strlen(title);
	xml_data = (char *)malloc(sizeof(char) * (xml_data_size + xml_extra_data_size));

	if (!xml_data) {
		BROWSER_LOGE("Failed to allocate memory to read files");
		fclose(file_read);
		return EINA_FALSE;
	}
	memset(xml_data, 0x00, (xml_data_size + xml_extra_data_size));

	size_t result = fread(xml_data, sizeof(char), (xml_data_size + xml_extra_data_size), file_read);
	fclose(file_read);

	if (result != xml_data_size) {
		BROWSER_LOGD("Reading error\n, result[%d]", result);
		free (xml_data);
		xml_data = NULL;
		return EINA_FALSE;
	}

	/* write xml raw data as config.xml */
	if (xml_path_attempt && strlen(xml_path_attempt) > 0) {
		m_xml_path = std::string(xml_path_attempt);
	}

	BROWSER_LOGD("m_xml_path[%s]", m_xml_path.c_str());

	char *markup_converted_url = elm_entry_utf8_to_markup(url);

	if(!markup_converted_url) {
		BROWSER_LOGE("failed to convert url to markup");
		return EINA_FALSE;
	}
	std::string url_string = std::string(markup_converted_url);
	BROWSER_LOGD("url_string[%s]", url_string.c_str());
	free(markup_converted_url);

	double time_stamp = ecore_loop_time_get();
	char time_stamp_string[TIME_STAMP_MAX_LENGTH] = {0, };
	snprintf(time_stamp_string, TIME_STAMP_MAX_LENGTH, "%lf", time_stamp);

	std::string title_string = std::string(title);
	std::string id_string = url_string + std::string("/") + std::string(time_stamp_string);
	std::string::size_type pos = std::string::npos;
	m_xml_law_data = std::string(xml_data);
	BROWSER_LOGD("id_string[%s]", id_string.c_str());
	free(xml_data);
	xml_data = NULL;

	while ((pos = m_xml_law_data.find("id_need")) != std::string::npos)
		m_xml_law_data.replace(pos, strlen("id_need"), id_string);

	while ((pos = m_xml_law_data.find("url_need")) != std::string::npos)
		m_xml_law_data.replace(pos, strlen("url_need"), url_string);

	while ((pos = m_xml_law_data.find("title_need")) != std::string::npos)
		m_xml_law_data.replace(pos, strlen("title_need"), title_string);

	BROWSER_LOGD("xml_raw_data - changed[%s]", m_xml_law_data.c_str());

	file_write = fopen(m_xml_path.c_str(), "w");

	if (!file_write) {
		BROWSER_LOGD("fopen failed", __func__);
		return EINA_FALSE;
	} else {
		BROWSER_LOGD("fopen success");
	}

	fwrite(m_xml_law_data.c_str(), sizeof(char), m_xml_law_data.length(), file_write);
	fclose(file_write);

	return EINA_TRUE;
}

Eina_Bool Browser_Meta_Tag::remove_config_xml(const char *xml_path_attempt)
{
	if (xml_path_attempt && strlen(xml_path_attempt) > 0)
		m_xml_path = std::string(xml_path_attempt);

	BROWSER_LOGD("xml_path[%s]", m_xml_path.c_str());

	if (unlink(m_xml_path.c_str()) == -1) {
		BROWSER_LOGE("Failed to remove config.xml in the path");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}
/* junghwankang_Native_APIs */
Eina_Bool Browser_Meta_Tag::request_download_icon(const char *url)
{
	BROWSER_LOGD("[%s], request url[%s]", __func__, url);
	SoupSession *soup_session = NULL;
	SoupMessage *soup_msg = NULL;
	SoupMessageHeaders *headers = NULL;

	soup_session = soup_session_async_new();
	g_object_set(soup_session, SOUP_SESSION_TIMEOUT, 15, NULL);
	soup_msg = soup_message_new("GET", url);

	BROWSER_LOGD("request url[%s]", url);
	headers = soup_msg->request_headers;

	soup_session_queue_message(soup_session, soup_msg, __download_icon_finished_cb, (void *)this);
}

void Browser_Meta_Tag::__download_icon_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Meta_Tag *browser_meta_tag = NULL;
	browser_meta_tag = (Browser_Meta_Tag *)data;

	SoupBuffer *body = soup_message_body_flatten(msg->response_body);

	int fd;
	int write_len = 0;
	browser_meta_tag->icon_remove(browser_meta_tag->m_icon_path.c_str());

	if (!body->data || body->length <= 0) {
		soup_buffer_free(body);
		return;
	}
	if ((fd = open((browser_meta_tag->m_icon_path.c_str()), O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		soup_buffer_free(body);
		return;
	}

	write_len = write(fd, body->data, body->length);
	close(fd);

	soup_buffer_free(body);

	if (write_len != body->length) {
		browser_meta_tag->icon_remove((browser_meta_tag->m_icon_path.c_str()));
		return;
	}

	/* send pkgmgr the wgt file */
	browser_meta_tag->wgt_install(NULL);
}

Eina_Bool Browser_Meta_Tag::icon_remove(const char *icon_path_attempt)
{
	if (icon_path_attempt && strlen(icon_path_attempt) > 0)
		m_icon_path = std::string(icon_path_attempt);

	BROWSER_LOGD("m_icon_path[%s]", m_icon_path.c_str());

	if (unlink(m_icon_path.c_str()) == -1) {
		BROWSER_LOGE("Failed to remove icon.png in the path");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_Meta_Tag::zip(const char *file_path_for_xml, const char* file_path_for_icon, const char* file_path_for_zipped_data)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_Meta_Tag::unzip(const char *file_path_for_zipped_data)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_Meta_Tag::wgt_install(const char *file_path)
{
	BROWSER_LOGD("[%s]", __func__);

	std::string wgt_file_path;

	if (file_path && strlen(file_path) > 0)
		wgt_file_path = std::string(file_path);
	else
		wgt_file_path = std::string("/opt/apps/org.tizen.browser/data/config.xml");

	BROWSER_LOGD("wgt_file_path[%s]", wgt_file_path.c_str());

	pkgmgr_client *pc = pkgmgr_client_new(PC_REQUEST);

	if (!pc)
		BROWSER_LOGD("pkgmgr_client_new is failed");

	pkgmgr_client_install(pc, "wgt", NULL, wgt_file_path.c_str(), NULL, PM_QUIET, __wgt_install_ret_cb, this);
	pkgmgr_client_free(pc);
}

int Browser_Meta_Tag::__wgt_install_ret_cb(int req_id, const char *pkg_type, const char *pkg_name, const char *key,
											const char *val, const void *pmsg, void *data)
{
	BROWSER_LOGD("[%s]", __func__);

	Browser_Meta_Tag *browser_meta_tag = NULL;
	browser_meta_tag = (Browser_Meta_Tag *)data;

	browser_meta_tag->remove_config_xml(NULL);
	unlink((browser_meta_tag->m_icon_path).c_str());
}

void Browser_Meta_Tag::__webapp_metatag_standalone_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data) {
		BROWSER_LOGE("__webapp_metatag_standalone_cb - data is null");
		return;
	}
}

