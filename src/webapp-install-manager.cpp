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

#include "webapp-install-manager.h"

#include <cairo.h>
#include <Ecore.h>
#include <Elementary.h>
#include <fcntl.h>
#include <gio/gio.h>
#include <image_util.h>
#include <package-manager.h>
#include <string>

#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "uri-bar.h"
#include "webview.h"

#define TIME_STAMP_MAX_LENGTH		255
#define XML_RAW_DATA_MAX_SIZE		4096
#define PATH_CONFIGURE_SAMPLE_XML	browser_res_dir"/template/config_sample.xml"
#define PATH_DEFAULT_ICON_PNG		browser_res_dir"/template/default_application_icon.png"
#define PATH_CONFIGURE_XML			browser_data_dir"/config.xml"
#define PATH_ICON_PNG				browser_data_dir"/icon.png"
#define WEBAPP_ICON_PNG_WITH_SNAPSHOT	"snapshot"
#define WEBAPP_ICON_PNG_RAW_DATA_TYPE	"data:image/png;base64"
#define WEBAPP_ICON_WIDTH		106
#define WEBAPP_ICON_HEIGHT		106

webapp_install_manager::webapp_install_manager(void)
:	m_ewk_view(NULL)
	,m_is_capable(EINA_FALSE)
	,m_has_webapp_icon_normally(EINA_FALSE)
	,m_uri(NULL)
	,m_title(NULL)
{
	BROWSER_LOGD("");
}

webapp_install_manager::~webapp_install_manager(void)
{
	BROWSER_LOGD("");
	_remove_webapp_icon(PATH_ICON_PNG);
	_remove_config_xml(PATH_CONFIGURE_XML);
}

Eina_Bool webapp_install_manager::request_make_webapp(Evas_Object *ewk_view)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(ewk_view, EINA_FALSE);

	return _prepare_make_webapp(ewk_view);
}

Eina_Bool webapp_install_manager::_prepare_make_webapp(Evas_Object *ewk_view)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(ewk_view, EINA_FALSE);

	m_ewk_view = ewk_view;
	ewk_view_web_application_capable_get(ewk_view, __webapp_capable_get_cb, this);

	m_uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
	m_title = m_browser->get_browser_view()->get_current_webview()->get_title();

	EINA_SAFETY_ON_NULL_RETURN_VAL(m_uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_title, EINA_FALSE);

#if 0
	if (m_ewk_view != ewk_view) {
		BROWSER_LOGE("ewk_view is different which is used in capable checking. Check Again");
		return EINA_FALSE;
	}

	if (m_is_capable != EINA_TRUE) {
		BROWSER_LOGE("The result of capable of the webapp with the ewk_view is checked as FALSE");
		return EINA_FALSE;
	}

	if (_webapp_icon_uri_get() != EINA_TRUE) {
		BROWSER_LOGE("Failed to set _webapp_icon_uri_get");
		return EINA_FALSE;
	}
#endif

	return EINA_TRUE;
}

Eina_Bool webapp_install_manager::_make_webapp(const char *icon_data, const char *uri, const char *title)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(icon_data, EINA_FALSE);

	if (!uri || !strlen(uri)) {
		BROWSER_LOGE("uri is invalid to make webapp");
		return EINA_FALSE;
	}

	/* make configure file */
	if (_create_config_xml(uri, title) == EINA_FALSE) {
		BROWSER_LOGE("Failed to create");
		return EINA_FALSE;
	}

	if (!strncmp(icon_data, WEBAPP_ICON_PNG_WITH_SNAPSHOT, strlen(WEBAPP_ICON_PNG_WITH_SNAPSHOT))) {
		return _install_webapp();
	} else if (m_is_capable == EINA_TRUE) {
		if (!strncmp(icon_data, WEBAPP_ICON_PNG_RAW_DATA_TYPE, strlen(WEBAPP_ICON_PNG_RAW_DATA_TYPE))) {
			_save_webapp_icon(icon_data);
			return _install_webapp();
		} else
			return _download_webapp_icon(icon_data);
	} else {
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool webapp_install_manager::_make_webapp_with_snapshot(const char *uri, const char *title)
{
	BROWSER_LOGD("");

	if (!uri || !strlen(uri)) {
		BROWSER_LOGE("uri is invalid to make webapp");
		return EINA_FALSE;
	}

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (wv->is_loading())
		m_browser->get_browser_view()->show_msg_popup(BR_STRING_LOADING_PLZ_WAIT, 3);
	else {
		/* make configure file */
		if (_create_config_xml(uri, title) == EINA_FALSE) {
			BROWSER_LOGE("Failed to create");
			return EINA_FALSE;
		}

		Evas_Object *snapshot = _capture_snapshot(m_ewk_view, 1.0);
		if (snapshot && (_save_icon_with_snapshot(snapshot) != EINA_TRUE)) {
			m_browser->get_browser_view()->show_msg_popup(BR_STRING_FAILED_TO_CREATE_AS_WEBAPP, 3);
			return EINA_FALSE;
		}

		_make_webapp(WEBAPP_ICON_PNG_WITH_SNAPSHOT, uri, title);
	}
}

Eina_Bool webapp_install_manager::_webapp_icon_uri_get(void)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_ewk_view, EINA_FALSE);

	if (m_is_capable)
		return ewk_view_web_application_icon_url_get(m_ewk_view, __webapp_icon_uri_get_cb, this);

	return EINA_FALSE;
}

Eina_Bool webapp_install_manager::_remove_webapp_icon(const char *icon_path_attempt)
{
	BROWSER_LOGD("icon_path_attempt[%s]", icon_path_attempt);

	if (icon_path_attempt && strlen(icon_path_attempt)) {
		if (unlink(icon_path_attempt) == -1) {
			BROWSER_LOGE("Failed to remove icon.png in the path");
			return EINA_FALSE;
		}
	} else {
		BROWSER_LOGE("Failed to remove icon.png. Path is invalid");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool webapp_install_manager::_remove_config_xml(const char *xml_path_attempt)
{
	BROWSER_LOGD("xml_path_attempt[%s]", xml_path_attempt);

	if (xml_path_attempt && strlen(xml_path_attempt)) {
		if (unlink(xml_path_attempt) == -1) {
			BROWSER_LOGE("Failed to remove config.xml in the path");
			return EINA_FALSE;
		}
	} else {
		BROWSER_LOGE("Failed to remove config.xml. Path is invalid");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool webapp_install_manager::_create_config_xml(const char *uri, const char *title)
{
	BROWSER_LOGD("uri[%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, EINA_FALSE);

	_remove_config_xml(PATH_CONFIGURE_XML);

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

	xml_extra_data_size = strlen(uri) + strlen(title);
	xml_data = (char *)malloc(sizeof(char) * (xml_data_size + xml_extra_data_size + 1));
	if (!xml_data) {
		BROWSER_LOGE("Failed to allocate memory to read files");
		fclose(file_read);
		return EINA_FALSE;
	}
	memset(xml_data, 0x00, (xml_data_size + xml_extra_data_size + 1));

	size_t result = fread(xml_data, sizeof(char), (xml_data_size + xml_extra_data_size), file_read);
	fclose(file_read);

	if (result != (size_t)xml_data_size) {
		BROWSER_LOGE("Reading error, result[%d]", result);
		free (xml_data);
		xml_data = NULL;
		return EINA_FALSE;
	}

	char *markup_converted_uri = elm_entry_utf8_to_markup(uri);
	std::string uri_string;
	if(!markup_converted_uri || strlen(markup_converted_uri) <= 0) {
		BROWSER_LOGE("failed to convert uri to markup");
		uri_string = std::string(uri);
	} else {
		BROWSER_LOGD("markup_converted_uri[%s]", markup_converted_uri);
		uri_string = std::string(markup_converted_uri);
	}
	if (markup_converted_uri) {
		free(markup_converted_uri);
		markup_converted_uri = NULL;
	}

	double time_stamp = ecore_loop_time_get();
	char time_stamp_string[TIME_STAMP_MAX_LENGTH] = {0, };
	snprintf(time_stamp_string, TIME_STAMP_MAX_LENGTH, "%lf", time_stamp);

	std::string title_string;
	std::string id_string = uri_string + std::string("/") + std::string(time_stamp_string);
	std::string::size_type pos = std::string::npos;
	std::string xml_law_data = std::string(xml_data);
	free(xml_data);
	xml_data = NULL;

	if (!title || !strlen(title))
		title_string = std::string(m_browser->get_browser_view()->get_uri_bar()->get_uri());
	else
		title_string = std::string(title);

	while ((pos = xml_law_data.find("id_need")) != std::string::npos)
		xml_law_data.replace(pos, strlen("id_need"), id_string);

	while ((pos = xml_law_data.find("uri_need")) != std::string::npos)
		xml_law_data.replace(pos, strlen("uri_need"), uri_string);

	while ((pos = xml_law_data.find("title_need")) != std::string::npos)
		xml_law_data.replace(pos, strlen("title_need"), title_string);

	file_write = fopen(PATH_CONFIGURE_XML, "w");

	if (!file_write) {
		BROWSER_LOGE("fopen failed");
		return EINA_FALSE;
	}

	fwrite(xml_law_data.c_str(), sizeof(char), xml_law_data.length(), file_write);
	fclose(file_write);

	return EINA_TRUE;
}

Eina_Bool webapp_install_manager::_save_webapp_icon(const char *icon_data)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(icon_data, EINA_FALSE);

	_remove_webapp_icon(PATH_ICON_PNG);

	if (!strncmp(icon_data, WEBAPP_ICON_PNG_RAW_DATA_TYPE, strlen(WEBAPP_ICON_PNG_RAW_DATA_TYPE))) {
		BROWSER_LOGD("icon_data is a type of encoded image raw data, make an image with it");
		int fd = 0;
		int write_len = 0;
		gsize length = 0;
		const char *decoded_data = NULL;

		decoded_data = (const char *)g_base64_decode((gchar *)icon_data + strlen(WEBAPP_ICON_PNG_RAW_DATA_TYPE), &length);
		if (!decoded_data || !strlen(decoded_data)) {
			BROWSER_LOGE("failed to decode raw data");
			if (decoded_data) {
				g_free((guchar *)decoded_data);
				decoded_data = NULL;
			}
			return EINA_FALSE;
		}

		if ((fd = open(PATH_ICON_PNG, O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
			BROWSER_LOGE("failed to file open to save raw data");
			if (decoded_data){
				g_free((guchar *)decoded_data);
				decoded_data = NULL;
			}
			return EINA_FALSE;
		}

		write_len = write(fd, decoded_data, length);
		close(fd);

		if (write_len != (int)length) {
			BROWSER_LOGE("failed to save raw data normally");
			_remove_webapp_icon(PATH_ICON_PNG);
			if (decoded_data){
				g_free((guchar *)decoded_data);
				decoded_data = NULL;
			}
			return EINA_FALSE;
		}

		if (decoded_data){
			g_free((guchar *)decoded_data);
			decoded_data = NULL;
		}
		return EINA_TRUE;

	} else {
		BROWSER_LOGE("icon_data is unknown type");
		return EINA_FALSE;
	}
}

Eina_Bool webapp_install_manager::_download_webapp_icon(const char *icon_data)
{
	BROWSER_LOGD("icon_data[%s] is uri type, take a step to download icon with the uri", icon_data);
	SoupSession *soup_session = NULL;
	SoupMessage *soup_msg = NULL;
	SoupMessageHeaders *headers = NULL;

	soup_session = soup_session_async_new();
	g_object_set(soup_session, SOUP_SESSION_TIMEOUT, 15, NULL);

	const char *default_proxy_uri = ewk_context_proxy_uri_get(ewk_context_default_get());
	BROWSER_LOGD("default_proxy_uri = [%s]", default_proxy_uri);
	if (default_proxy_uri) {
		std::string proxy_uri = std::string("http://") + std::string(default_proxy_uri);
		SoupURI *soup_uri = soup_uri_new(proxy_uri.c_str());
		g_object_set(soup_session, SOUP_SESSION_PROXY_URI, soup_uri, NULL);
		if (soup_uri)
			soup_uri_free(soup_uri);
	}
	soup_msg = soup_message_new("GET", icon_data);

	BROWSER_LOGD("request uri[%s]", icon_data);
	headers = soup_msg->request_headers;

	soup_session_queue_message(soup_session, soup_msg, __download_webapp_icon_finished_cb, (void *)this);

	return EINA_TRUE;
}

Eina_Bool webapp_install_manager::_install_webapp(void)
{
	BROWSER_LOGD("");

	int pkgmgr_request_result = 0;
	pkgmgr_client *pc = pkgmgr_client_new(PC_REQUEST);

	if (!pc) {
		BROWSER_LOGE("pkgmgr_client_new is failed");
		return EINA_FALSE;
	}

	pkgmgr_request_result = pkgmgr_client_install(pc, "wgt", NULL, PATH_CONFIGURE_XML, NULL, PM_QUIET, __install_webapp_result_cb, this);
		BROWSER_LOGE("pkgmgr_request_result is [%d]", pkgmgr_request_result);

	pkgmgr_client_free(pc);

	if (pkgmgr_request_result < 0)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

Evas_Object *webapp_install_manager::_capture_snapshot(Evas_Object *ewk_view, float scale)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(ewk_view, NULL);

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (wv->is_loading()) {
		m_browser->get_browser_view()->show_msg_popup(BR_STRING_LOADING_PLZ_WAIT, 3);
		return NULL;
	}

	int focused_ewk_view_w = 0;
	int focused_ewk_view_h = 0;
	evas_object_geometry_get(ewk_view, NULL, NULL, &focused_ewk_view_w, &focused_ewk_view_h);

	Eina_Rectangle snapshot_rect;
	snapshot_rect.x = snapshot_rect.y = 0;
	snapshot_rect.w = focused_ewk_view_w;
	snapshot_rect.h = focused_ewk_view_h;

	Evas_Object *snapshot = ewk_view_screenshot_contents_get(ewk_view, snapshot_rect, scale, evas_object_evas_get(m_naviframe));
	if (!snapshot) {
		BROWSER_LOGE("ewk_view_screenshot_contents_get failed");

		Evas_Object *rectangle = evas_object_rectangle_add(evas_object_evas_get(m_naviframe));
		evas_object_size_hint_min_set(rectangle, focused_ewk_view_w * scale, focused_ewk_view_h * scale);
		evas_object_resize(rectangle, focused_ewk_view_w * scale, focused_ewk_view_h * scale);
		evas_object_color_set(rectangle, 255, 255, 255, 255);
		return rectangle;
	}

	return snapshot;
}

Eina_Bool webapp_install_manager::_save_icon_with_snapshot(Evas_Object *snapshot)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(snapshot, EINA_FALSE);

	_remove_webapp_icon(PATH_ICON_PNG);

	int surface_width = 0;
	int surface_height = 0;
	unsigned int resized_image_buffer_size = 0;
	const unsigned char *pixels = (const unsigned char *)(evas_object_image_data_get(snapshot, EINA_TRUE));

	evas_object_image_size_get(snapshot, &surface_width, &surface_height);
	image_util_calculate_buffer_size(WEBAPP_ICON_WIDTH, WEBAPP_ICON_HEIGHT, IMAGE_UTIL_COLORSPACE_ARGB8888, &resized_image_buffer_size);

	unsigned char *resized_image = new unsigned char[resized_image_buffer_size + 1];
	int resized_width = WEBAPP_ICON_WIDTH;
	int resized_height = WEBAPP_ICON_HEIGHT;

	image_util_resize(resized_image, &resized_width, &resized_height, pixels, surface_width, surface_height, IMAGE_UTIL_COLORSPACE_ARGB8888);
	cairo_surface_t *snapshot_surface = cairo_image_surface_create_for_data((uint8_t *)resized_image,
																			CAIRO_FORMAT_ARGB32,
																			WEBAPP_ICON_WIDTH, WEBAPP_ICON_HEIGHT,
																			cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, WEBAPP_ICON_WIDTH));
	if (cairo_surface_write_to_png(snapshot_surface, PATH_ICON_PNG) != CAIRO_STATUS_SUCCESS) {
		BROWSER_LOGD("cairo_surface_write_to_png FAILED");
		delete [] resized_image;
		return EINA_FALSE;
	}
	cairo_surface_destroy(snapshot_surface);
	delete [] resized_image;

	return EINA_TRUE;
}

void webapp_install_manager::__webapp_capable_get_cb(Eina_Bool capable, void *user_data)
{
	BROWSER_LOGD("capable[%d]", capable);
	EINA_SAFETY_ON_NULL_RETURN(user_data);

	webapp_install_manager *wim = (webapp_install_manager *)user_data;
	wim->m_is_capable = capable;

	if ((wim->m_is_capable == EINA_TRUE) && (wim->_webapp_icon_uri_get() == EINA_TRUE))
		return;

	wim->_make_webapp_with_snapshot(wim->m_uri, wim->m_title);
}

void webapp_install_manager::__webapp_icon_uri_get_cb(const char* icon_data, void* user_data)
{
	/* for degug, remove it when merged */
	BROWSER_LOGD("icon_data[%s]", icon_data);
	EINA_SAFETY_ON_NULL_RETURN(user_data);

	webapp_install_manager *wim = (webapp_install_manager *)user_data;
	wim->_make_webapp(icon_data, wim->m_uri, wim->m_title);
}

void webapp_install_manager::__download_webapp_icon_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	webapp_install_manager *wim = (webapp_install_manager *)data;
	SoupBuffer *body = soup_message_body_flatten(msg->response_body);

	int fd;
	int write_len = 0;

	if (!body->data || body->length <= 0) {
		soup_buffer_free(body);
		return;
	}

	wim->_remove_webapp_icon(PATH_ICON_PNG);
	if ((fd = open(PATH_ICON_PNG, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		soup_buffer_free(body);
		return;
	}
	write_len = write(fd, body->data, body->length);
	close(fd);

	soup_buffer_free(body);

	if (write_len != (int)body->length) {
		wim->_remove_webapp_icon(PATH_ICON_PNG);
		return;
	}

	/* send pkgmgr the wgt file */
	wim->_install_webapp();
}

int webapp_install_manager::__install_webapp_result_cb(int req_id, const char *pkg_type, const char *pkg_name, const char *key,
											const char *val, const void *pmsg, void *data)
{
	BROWSER_LOGD("");

	webapp_install_manager *wim = (webapp_install_manager *)data;

	wim->_remove_config_xml(PATH_CONFIGURE_XML);
	wim->_remove_webapp_icon(PATH_ICON_PNG);

	m_browser->get_browser_view()->show_msg_popup(BR_STRING_CREATED, 3);

	return 0;
}
