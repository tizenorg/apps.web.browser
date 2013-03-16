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

#include "download-manager.h"

#include <Elementary.h>
#include <ail.h>
#include <fcntl.h>
#include <download.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "webview.h"

static const char *unknown_mime1 = "text/plain";
static const char *unknown_mime2 = "application/octet-stream";
static const char *sdp_mime = "application/sdp";

static const char *sdp_path = "/opt/usr/apps/org.tizen.browser/data/browser.sdp";

#ifdef SUPPORT_HLS
static const char *hls_support_mime[] = {"application/vnd.apple.mpegurl", "application/x-mpegurl", "application/m3u", "audio/x-mpegurl", "audio/m3u", "audio/x-m3u", NULL};
#endif

#define APP_LIST_HEIGHT	(230 * efl_scale)

using namespace std;

typedef struct _download_request {
	char *file_path;
	download_finish_callback cb;
	void *data;
} download_request;

download_manager::download_manager(void)
:
	m_is_matched_app(EINA_FALSE)
{
	BROWSER_LOGD("");
}

download_manager::~download_manager(void)
{
	BROWSER_LOGD("");
}

void download_manager::__sdp_download_finished_cb(const char *file_path, void *data)
{
	BROWSER_LOGD("file_path = [%s]", file_path);
	EINA_SAFETY_ON_NULL_RETURN(file_path);
	m_browser->get_browser_view()->launch_streaming_player(file_path);
}

void download_manager::__sdp_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	download_manager *dm = (download_manager *)data;
	dm->request_file_download(dm->m_download_uri.c_str(), sdp_path, __sdp_download_finished_cb, NULL);

	m_browser->get_browser_view()->destroy_popup(obj);
}

void download_manager::__internet_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	download_manager *dm = (download_manager *)data;
	dm->launch_download_app(dm->m_download_uri.c_str());

	m_browser->get_browser_view()->destroy_popup(obj);
}

/* @return - The return value should be freed by caller. */
static char *_extract_extension(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, NULL);

	std::string uri_str = std::string(uri);

	size_t pos = 0;
	pos = uri_str.find_last_of("/");

	if (pos != std::string::npos) {
		uri_str = uri_str.substr(pos + 1);
		pos = uri_str.find_last_of(".");
		if (pos != std::string::npos) {
			uri_str = uri_str.substr(pos + 1);
			pos = uri_str.find_last_of("?");
			/* The pos value is started with zero */
			if (pos != std::string::npos)
				uri_str = uri_str.substr(0, pos);
		}
	}

	BROWSER_LOGD("extension name[%s]\n", uri_str.c_str());
	return strdup(uri_str.c_str());
}

/* @return - The return value should be freed by caller. */
static char *_get_app_name(const char *pkg_name)
{
	BROWSER_LOGD("pkg_name = [%s]", pkg_name);
	EINA_SAFETY_ON_NULL_RETURN_VAL(pkg_name, NULL);

	ail_appinfo_h handle;
	ail_error_e ret = ail_package_get_appinfo(pkg_name, &handle);
	if (ret != AIL_ERROR_OK) {
		return NULL;
	}

	char *app_name = NULL;
	ret = ail_appinfo_get_str(handle, AIL_PROP_NAME_STR, &app_name);
	if (ret != AIL_ERROR_OK) {
		ail_package_destroy_appinfo(handle);
		return NULL;
	}

	ret = ail_package_destroy_appinfo(handle);
	if (ret != AIL_ERROR_OK)
		return NULL;

	BROWSER_LOGD("app_name = [%s]", app_name);
	return strdup(app_name);
}

void download_manager::__player_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	download_manager *dm = (download_manager *)data;

	if (dm->m_cookie.empty())
		dm->launch_download_app(dm->m_download_uri.c_str());
	else
		dm->launch_download_app(dm->m_download_uri.c_str(), dm->m_cookie.c_str());

	m_browser->get_browser_view()->destroy_popup(obj);
}

Eina_Bool download_manager::_show_avaiable_app_popup(const char *pkg_name, const char *uri, const char *cookie)
{
	BROWSER_LOGD("pkg_name = [%s]", pkg_name);
	EINA_SAFETY_ON_NULL_RETURN_VAL(pkg_name, EINA_FALSE);
	Evas_Object *app_list = elm_list_add(m_window);
	if (!app_list) {
		BROWSER_LOGE("elm_list_add failed");
		return EINA_FALSE;
	}
	elm_list_mode_set(app_list, ELM_LIST_EXPAND);

	m_cookie.clear();

	m_download_uri = std::string(uri);

	if (cookie)
		m_cookie = std::string(cookie);

	char *app_name = _get_app_name(pkg_name);
	if (app_name) {
		elm_list_item_append(app_list, app_name, NULL, NULL, __player_cb, this);
		free(app_name);
	}

	elm_list_item_append(app_list, BR_STRING_INTERNET, NULL, NULL, __internet_cb, this);
	evas_object_size_hint_max_set(app_list, -1, APP_LIST_HEIGHT);

	m_browser->get_browser_view()->show_content_popup(BR_STRING_TITLE_SELECT_AN_ACTION, app_list, BR_STRING_CANCEL);

	return EINA_TRUE;
}

void download_manager::handle_download_request(const char *uri, const char *cookie, const char *content_type)
{
	BROWSER_LOGD("uri = [%s], cookie = [%s], content_type = [%s]", uri, cookie, content_type);
	EINA_SAFETY_ON_NULL_RETURN(uri);

	m_download_uri.clear();
	m_cookie.clear();
	m_is_matched_app = EINA_FALSE;

	m_download_uri = std::string(uri);
	if (cookie)
		m_cookie = std::string(cookie);

	if (!content_type) {
		launch_download_app(uri, cookie);
		return;
	}

	if (!strcmp(content_type, sdp_mime)) {
		Evas_Object *app_list = elm_list_add(m_window);
		if (!app_list) {
			BROWSER_LOGE("elm_list_add failed");
			return;
		}

		elm_list_mode_set(app_list, ELM_LIST_EXPAND);
		elm_list_item_append(app_list, BR_STRING_STREAMING_PLAYER, NULL, NULL, __sdp_cb, this);
		elm_list_item_append(app_list, BR_STRING_INTERNET, NULL, NULL, __internet_cb, this);
		evas_object_size_hint_max_set(app_list, -1, APP_LIST_HEIGHT);

		m_browser->get_browser_view()->show_content_popup(BR_STRING_TITLE_SELECT_AN_ACTION, app_list, BR_STRING_CANCEL);

		return;
	}

	/* If mime is ambiguous and extension name is existed,
	*   decide streaming player according to extension name from url
	* 1. mp4, 3gp : streaming video player case
	* 2. mp3 : streaming music player case
	* 3. otherewise : download app case
	*/

	if (!strcmp(content_type, unknown_mime1) || !strcmp(content_type, unknown_mime2)) {
		char *extension = _extract_extension(uri);
		if (extension) {
#ifdef SUPPORT_HLS
			if (!strcmp(extension, "m3u") || !strcmp(extension, "m3u8")) {
				m_browser->get_browser_view()->launch_streaming_player(uri, cookie);
				free(extension);
				return;
			}
#endif /* SUPPORT_HLS */
			if (!strcmp(extension, "mp4") || !strcmp(extension, "3gp")) {
				_show_avaiable_app_popup(sec_streaming_player, uri, cookie);
				free(extension);
				return;
			} else if (!strcmp(extension, "mp3")) {
				_show_avaiable_app_popup(sec_music_player, uri, cookie);
				free(extension);
				return;
			} else {
				launch_download_app(uri, cookie);
				free(extension);
				return;
			}
		}
	}

#ifdef SUPPORT_HLS
	int index = 0;
	while (hls_support_mime[index]) {
		if (!strcmp(hls_support_mime[index], content_type)) {
			m_browser->get_browser_view()->launch_streaming_player(uri, cookie);
			return;
		}
		index++;
	}
#endif /* SUPPORT_HLS */

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return;
	}

	if (service_set_operation(service_handle, SERVICE_OPERATION_VIEW) < 0) {
		BROWSER_LOGE("Fail to set service operation");
		service_destroy(service_handle);
		return;
	}

	if (service_set_mime(service_handle, content_type) < 0) {
		BROWSER_LOGE("Fail to set mime type");
		service_destroy(service_handle);
		return;
	}

	service_foreach_app_matched(service_handle, __app_matched_cb, this);

	if (!m_is_matched_app) {
		launch_download_app(uri, cookie);
		service_destroy(service_handle);
		m_is_matched_app = EINA_FALSE;
	}
}

bool download_manager::__app_matched_cb(service_h service_handle, const char *package, void *data)
{
	BROWSER_LOGD("");
	download_manager *dm = (download_manager *)data;

	if (package && !strcmp(package, sec_streaming_player)) {
		if (dm->m_cookie.empty())
			dm->_show_avaiable_app_popup(sec_streaming_player, dm->m_download_uri.c_str());
		else
			dm->_show_avaiable_app_popup(sec_streaming_player, dm->m_download_uri.c_str(), dm->m_cookie.c_str());
	} else {
		if (dm->m_cookie.empty())
			dm->launch_download_app(dm->m_download_uri.c_str());
		else
			dm->launch_download_app(dm->m_download_uri.c_str(), dm->m_cookie.c_str());
	}

	service_destroy(service_handle);

	dm->m_is_matched_app = EINA_TRUE;

	return true;
}

Eina_Bool download_manager::launch_download_app(const char *uri, const char *cookie)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);
	BROWSER_LOGD("uri = [%s], cookie = [%s]", uri, cookie);

	service_h service_handle = NULL;

	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("service handle is NULL");
		return EINA_FALSE;
	}

	if (service_set_operation(service_handle, SERVICE_OPERATION_DOWNLOAD) < 0) {
		BROWSER_LOGE("Fail to set service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_set_uri(service_handle, uri) < 0) {
		BROWSER_LOGE("Fail to set uri");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (cookie && strlen(cookie)) {
		if (service_add_extra_data(service_handle, "cookie", cookie) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

void download_manager::request_file_download(const char *uri, const char *file_path, download_finish_callback cb, void *data)
{
	EINA_SAFETY_ON_NULL_RETURN(uri);
	EINA_SAFETY_ON_NULL_RETURN(file_path);
	EINA_SAFETY_ON_NULL_RETURN(cb);

	BROWSER_LOGD("uri = [%s], file_path = [%s]", uri, file_path);

	SoupSession *soup_session = NULL;
	SoupMessage *soup_msg = NULL;
	SoupMessageHeaders *headers = NULL;

	soup_session = soup_session_async_new();
	g_object_set(soup_session, SOUP_SESSION_TIMEOUT, 15, NULL);

	const char *proxy = m_webview_context->get_proxy_address();
	BROWSER_LOGD("proxy = [%s]", proxy);
	if (proxy) {
		std::string proxy_address = std::string("http://") + std::string(proxy);
		SoupURI *soup_uri = soup_uri_new(proxy_address.c_str());
		g_object_set(soup_session, SOUP_SESSION_PROXY_URI, soup_uri, NULL);
		if (soup_uri)
			soup_uri_free(soup_uri);
	}

	soup_msg = soup_message_new("GET", uri);

	download_request *request = (download_request *)malloc(sizeof(download_request));
	request->cb = cb;
	request->data = data;
	request->file_path = strdup(file_path);

	soup_session_queue_message(soup_session, soup_msg, __file_download_finished_cb, (void *)request);
}

void download_manager::__file_download_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data)
{
	BROWSER_LOGD("");
	download_request *request = (download_request *)data;

	SoupBuffer *body = soup_message_body_flatten(msg->response_body);

	int fd;
	unsigned int write_len = 0;

	if (!body->data || body->length <= 0) {
		soup_buffer_free(body);
		free(request->file_path);
		free(request);
		return;
	}
	if ((fd = open(request->file_path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		soup_buffer_free(body);
		free(request->file_path);
		free(request);
		return;
	}

	write_len = write(fd, body->data, body->length);
	close(fd);

	soup_buffer_free(body);

	if (write_len != body->length) {
		unlink(request->file_path);
		free(request->file_path);
		free(request);
		return;
	}

	request->cb(request->file_path, request->data);

	free(request->file_path);
	free(request);
}

