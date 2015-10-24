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

#include "download-manager.h"

#include <appsvc.h>
#include <fcntl.h>
#include <notification.h>
#include <notification_internal.h>
#include <time.h>

#include "media_content.h"
#include "media_info.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "platform-service.h"
#include "preference.h"
#include "webview.h"

static const char *sdp_mime = "application/sdp";
static const char *sdp_path = "/opt/usr/apps/"BROWSER_APP_NAME"/data/browser.sdp";
#define default_download_item_name "download"
#define BROWSER_DATA_SCHEME_DOWNLOAD_ALLOW_MAX_COUNT 100000
#define APP_LIST_HEIGHT	(230 * efl_scale)

struct download_request
{
public:
	download_request(char* file_path_ = NULL, download_finish_callback cb_ = NULL, void* data_ = NULL) :
		file_path(file_path_),
		cb(cb_),
		data(data_)
	{}

	~download_request() {}

	char *file_path;
	download_finish_callback cb;
	void *data;

private:
	download_request& operator=(const download_request&);
	download_request(const download_request&);
};

download_manager::download_manager(void)
	:m_download_uri()
{
	BROWSER_LOGD("");
}

download_manager::~download_manager(void)
{
	BROWSER_LOGD("");
}

void download_manager::__sdp_download_finished_cb(const char *file_path, void *data)
{
	BROWSER_SECURE_LOGD("file_path = [%s]", file_path);
	RET_MSG_IF(!file_path, "file_path is NULL");
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

Eina_Bool download_manager::_check_file_exist(const char *path)
{
	BROWSER_SECURE_LOGD("path = [%s]", path);
	RETV_MSG_IF(!path, EINA_FALSE, "path is NULL");

	if (!strlen(path)) {
		BROWSER_LOGE("path has no string");
		return EINA_FALSE;
	}

	struct stat file_state;
	int stat_ret = 0;

	stat_ret = stat(path, &file_state);
	if (stat_ret != 0){
		BROWSER_LOGE("failed to stat");
		return EINA_FALSE;
	}

	if (!S_ISREG(file_state.st_mode)) {
		BROWSER_LOGE("The file is not existed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool download_manager::_save_file(const char *raw_data, const char *path)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!raw_data, EINA_FALSE, "raw_data is NULL");
	RETV_MSG_IF(!path, EINA_FALSE, "path is NULL");

	int fd = 0;
	int write_len = 0;
	gsize length = 0;
	const char *decoded_data = NULL;

	decoded_data = (const char *)g_base64_decode((gchar *)raw_data, &length);
	if (!decoded_data) {
		BROWSER_LOGE("failed to decode raw data");
		return EINA_FALSE;
	}

	if (!strlen(decoded_data)) {
		BROWSER_LOGE("has no data");
		g_free((guchar *)decoded_data);
		decoded_data = NULL;
		return EINA_FALSE;
	}

	if ((fd = open(path, O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		BROWSER_LOGE("failed to file open to save raw data");
		g_free((guchar *)decoded_data);
		decoded_data = NULL;
		return EINA_FALSE;
	}

	write_len = write(fd, decoded_data, length);
	close(fd);

	if (write_len != (int)length) {
		BROWSER_LOGE("failed to save raw data normally");
		unlink(path);
		g_free((guchar *)decoded_data);
		decoded_data = NULL;
		return EINA_FALSE;
	}

	if (decoded_data){
		g_free((guchar *)decoded_data);
		decoded_data = NULL;
	}

	return EINA_TRUE;
}

Eina_Bool download_manager::_update_contents_on_media_db(const char *path)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!path, EINA_FALSE, "path is NULL");

	if (!strlen(path)) {
		BROWSER_LOGD("path has no string");
		return EINA_FALSE;
	}

	int ret = -1;
	media_info_h info = NULL;

	ret = media_content_connect();
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		BROWSER_LOGE("Fail to media_content_connect");
		return EINA_FALSE;
	}

	ret = media_info_insert_to_db(path, &info);
	if (ret != MEDIA_CONTENT_ERROR_NONE || info == NULL) {
		BROWSER_LOGE("Fail to media_info_insert_to_db [%d]", ret);
		media_content_disconnect();
		if (info)
			media_info_destroy(info);
		return EINA_FALSE;
	}

	media_info_destroy(info);
	ret = media_content_disconnect();
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		BROWSER_LOGE("Fail to media_content_disconnect");
	}

	return EINA_TRUE;
}

Eina_Bool download_manager::_get_download_path(const char *extension, char **full_path, char **file_name)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!extension, EINA_FALSE, "extension is NULL");

	char temp_count_str[10] = {0};
	int count = 0;

	std::string storing_path;
	if (m_preference->get_default_storage_type() == DEFAULT_STORAGE_TYPE_MEMORY_CARD)
		storing_path = std::string(default_sd_card_storage_path);
	else //if (m_preference->get_default_storage_type() == DEFAULT_STORAGE_TYPE_PHONE)
		storing_path = std::string(default_device_storage_path);

	std::string only_file_name = std::string(default_download_item_name) + std::string(".") + std::string(extension);
	std::string entire_path_name = storing_path + std::string(default_download_item_name) + std::string(".") + std::string(extension);

	while(_check_file_exist((const char *)entire_path_name.c_str()) == EINA_TRUE) {
		count++;
		memset(temp_count_str, 0, 10);
		snprintf(temp_count_str, 10, "_%d", count);

		only_file_name.clear();
		only_file_name = std::string(default_download_item_name)
				+ std::string(temp_count_str)
				+ std::string(".")
				+ std::string(extension);

		entire_path_name.clear();
		entire_path_name = storing_path
						+ std::string(default_download_item_name)
						+ std::string(temp_count_str)
						+ std::string(".")
						+ std::string(extension);

		if (count > BROWSER_DATA_SCHEME_DOWNLOAD_ALLOW_MAX_COUNT) {
			entire_path_name.clear();
			only_file_name.clear();
			break;
		}
	}

	Eina_Bool ok = EINA_FALSE;

	if (!entire_path_name.empty())
	{
		*full_path = strdup(entire_path_name.c_str());
		*file_name = strdup(only_file_name.c_str());
		ok = EINA_TRUE;
	}

	return ok;
}

Eina_Bool download_manager::_set_downloaded_file_on_notification(const char *downloaded_path, const char *file_name)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!downloaded_path, EINA_FALSE, "downloaded_path is NULL");

	notification_h noti_handle = notification_create(NOTIFICATION_TYPE_NOTI);
	RETV_MSG_IF(!noti_handle, EINA_FALSE, "Failed to notification_create");

	Eina_Bool success = EINA_FALSE;
	platform_service ps;
	bundle *bundle = NULL;
	char *file_size_str = ps.get_file_size_str(downloaded_path);
	int priv_Id = 0;
	const char *uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
	int err = NOTIFICATION_ERROR_NONE;

	if (file_name && strlen(file_name))
		err = notification_set_text(noti_handle, NOTIFICATION_TEXT_TYPE_CONTENT, file_name, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		err = notification_set_text(noti_handle, NOTIFICATION_TEXT_TYPE_CONTENT, BR_STRING_NO_NAME, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Fail to notification_set_text(file_name) with error : [%d]", err);
		goto ERROR;
	}

	err = notification_set_text(noti_handle, NOTIFICATION_TEXT_TYPE_TITLE, BR_STRING_DOWNLOAD_COMPLETE, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Fail to notification_set_text with error(NOTIFICATION_TEXT_TYPE_TITLE) : [%d]", err);
		goto ERROR;
	}

	if (file_size_str && strlen(file_size_str)) {
		err = notification_set_text(noti_handle, NOTIFICATION_TEXT_TYPE_INFO_1, file_size_str, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		if (err != NOTIFICATION_ERROR_NONE) {
			BROWSER_LOGE("Fail to notification_set_text(NOTIFICATION_TEXT_TYPE_INFO_1) with error : [%d]", err);
			goto ERROR;
		}
	}

	err = notification_set_image(noti_handle, NOTIFICATION_IMAGE_TYPE_BACKGROUND, downloaded_path);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Fail to notification_set_image(NOTIFICATION_IMAGE_TYPE_ICON) with error : [%d]", err);
		goto ERROR;
	}

	err = notification_set_image(noti_handle, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, browser_img_dir"/Q01_notification_download_complete.png");
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Fail to notification_set_image(NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR) with error : [%d]", err);
		goto ERROR;
	}

	if (uri && strlen(uri)) {
		err = notification_set_text(noti_handle, NOTIFICATION_TEXT_TYPE_INFO_2, uri, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		if (err != NOTIFICATION_ERROR_NONE) {
			BROWSER_LOGE("Fail to notification_set_image(NOTIFICATION_TEXT_TYPE_INFO_2) with error : [%d]", err);
			goto ERROR;
		}
	}

	time_t current_time;
	time(&current_time);
	err = notification_set_time_to_text(noti_handle, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, current_time);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Fail to notification_set_time_to_text with error : [%d]", err);
		goto ERROR;
	}

	bundle = bundle_create();
	if (!bundle) {
		BROWSER_LOGE("Fail to bundle_create");
		goto ERROR;
	}

	if (appsvc_set_operation(bundle, APPSVC_OPERATION_VIEW) != APPSVC_RET_OK) {
		BROWSER_LOGE("Fail to appsvc_set_operation");
		goto ERROR;
	}

	if (appsvc_set_uri(bundle, downloaded_path) != APPSVC_RET_OK) {
		BROWSER_LOGE("Fail to appsvc_set_uri");
		goto ERROR;
	}

	err = notification_set_execute_option(noti_handle, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, "View", NULL, bundle);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Fail to notification_set_execute_option with error [%d]", err);
		goto ERROR;
	}

	err = notification_set_property(noti_handle, NOTIFICATION_PROP_DISABLE_TICKERNOTI);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Fail to notification_set_propertywith error [%d]", err);
		goto ERROR;
	}

	err = notification_insert(noti_handle, &priv_Id);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Fail to notification_insert with error [%d]", err);
		goto ERROR;
	}

	// Being here means success !
	success = EINA_TRUE;

ERROR:
	err = notification_free(noti_handle);
	if(err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("notification_free is failed with err[%d]", err);
	}

	bundle_free(bundle);
	free(file_size_str);
	return success;
}

void download_manager::handle_download_request(const char *uri, const char *content_type)
{
	BROWSER_SECURE_LOGD("uri = [%s], content_type = [%s]", uri, content_type);
	RET_MSG_IF(!uri, "uri is NULL");

	m_download_uri.clear();

	m_download_uri = std::string(uri);

	if (!content_type) {
		launch_download_app(uri);
		return;
	}

	if (!strcmp(content_type, sdp_mime)) {
		Evas_Object *popup = brui_popup_add(m_window);
		RET_MSG_IF(!popup, "popup is null");

		Evas_Object *app_list = elm_list_add(popup);
		RET_MSG_IF(!app_list, "app_list is null");

		elm_list_mode_set(app_list, ELM_LIST_EXPAND);
		elm_list_item_append(app_list, BR_STRING_STREAMING_PLAYER, NULL, NULL, __sdp_cb, this);
		elm_list_item_append(app_list, BR_STRING_INTERNET, NULL, NULL, __internet_cb, this);
		evas_object_size_hint_max_set(app_list, -1, APP_LIST_HEIGHT);

		m_browser->get_browser_view()->show_content_popup(popup, BR_STRING_SELECT_AVAILABLE_APP, app_list, NULL, BR_STRING_CANCEL);
	} else {
		launch_download_app(uri);
	}
}

Eina_Bool download_manager::handle_data_scheme(const char *uri)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	const char *encoded_str = NULL;
	const char *extension = NULL;
	Eina_Bool return_val = EINA_FALSE;

	if (!strncmp(uri, data_scheme_jpeg_base64, strlen(data_scheme_jpeg_base64))) {
		encoded_str = uri + strlen(data_scheme_jpeg_base64) + 1; // 1 is reserved for separator ','
		extension = (const char *)"jpeg";
	} else if (!strncmp(uri, data_scheme_jpg_base64, strlen(data_scheme_jpg_base64))) {
		encoded_str = uri + strlen(data_scheme_jpg_base64) + 1; // 1 is reserved for separator ','
		extension = (const char *)"jpg";
	} else if (!strncmp(uri, data_scheme_png_base64, strlen(data_scheme_png_base64))) {
		encoded_str = uri + strlen(data_scheme_png_base64) + 1; // 1 is reserved for separator ','
		extension = (const char *)"png";
	} else if (!strncmp(uri, data_scheme_gif_base64, strlen(data_scheme_gif_base64))) {
		encoded_str = uri + strlen(data_scheme_gif_base64) + 1; // 1 is reserved for separator ','
		extension = (const char *)"gif";
	} else {
		BROWSER_LOGD("Un-recognizable data scheme type");
		return EINA_FALSE;
	}

	char *full_path = NULL;
	char *file_name = NULL;
	if (_get_download_path(extension, &full_path, &file_name) == EINA_FALSE) {
		BROWSER_LOGE("failed to _get_download_path");
		goto HANDLE_DATA_SCHEME_ERROR;
	}

	if (!(full_path && strlen(full_path)) || !(file_name && strlen(file_name))) {
		BROWSER_LOGE("has problem to _get_download_path");
		goto HANDLE_DATA_SCHEME_ERROR;
	}

	if (_save_file(encoded_str, full_path) == EINA_FALSE) {
		BROWSER_SECURE_LOGE("failed to _save_file with path [%s]", full_path);
		goto HANDLE_DATA_SCHEME_ERROR;
	}

	if (_update_contents_on_media_db(full_path) == EINA_FALSE) {
		BROWSER_SECURE_LOGE("failed to _update_contents_on_media_db with path [%s]", full_path);
		goto HANDLE_DATA_SCHEME_ERROR;
	}

	if (_set_downloaded_file_on_notification(full_path, file_name) == EINA_FALSE) {
		BROWSER_SECURE_LOGE("failed to _set_downloaded_file_on_notification with path [%s]", file_name);
		goto HANDLE_DATA_SCHEME_ERROR;
	}

	return_val = EINA_TRUE;

HANDLE_DATA_SCHEME_ERROR:
	free(full_path);
	free(file_name);

	return return_val;
}

Eina_Bool download_manager::launch_download_app(const char *uri)
{
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");
	BROWSER_SECURE_LOGD("uri = [%s]", uri);

	app_control_h app_control = NULL;

	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (!app_control) {
		BROWSER_LOGE("app_control handle is NULL");
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_DOWNLOAD) < 0) {
		BROWSER_LOGE("Fail to set app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_uri(app_control, uri) < 0) {
		BROWSER_LOGE("Fail to set uri");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_add_extra_data(app_control, "mode", "silent") < 0) {
		BROWSER_LOGE("Fail to set app_control extra data");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	const char *storage_type = "0"; //phone memory
	if (m_preference->get_default_storage_type() == DEFAULT_STORAGE_TYPE_MEMORY_CARD)
		storage_type = "1";

	if (app_control_add_extra_data(app_control, "default_storage", storage_type) < 0) {
		BROWSER_LOGE("Fail to set app_control extra data");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_add_extra_data(app_control, "network_bonding", "true") < 0) {
		BROWSER_LOGE("Fail to set app_control extra data");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	m_browser->get_browser_view()->show_noti_popup(BR_STRING_DOWNLOADING_ING);

	return EINA_TRUE;
}

void download_manager::request_file_download(const char *uri, const char *file_path, download_finish_callback cb, void *data)
{
	RET_MSG_IF(!uri, "uri is NULL");
	RET_MSG_IF(!file_path, "file_path is NULL");
	RET_MSG_IF(!cb, "cb is NULL");

	BROWSER_SECURE_LOGD("uri = [%s], file_path = [%s]", uri, file_path);

	SoupSession *soup_session = NULL;
	SoupMessage *soup_msg = NULL;

	soup_session = soup_session_async_new();
	g_object_set(soup_session, SOUP_SESSION_TIMEOUT, 15, (char *)NULL);

	const char *proxy = "0.0.0.0";
	BROWSER_LOGD("proxy = [%s]", proxy);
	if (proxy) {
		std::string proxy_address = std::string("http://") + std::string(proxy);
		SoupURI *soup_uri = soup_uri_new(proxy_address.c_str());
		g_object_set(soup_session, SOUP_SESSION_PROXY_URI, soup_uri, (char *)NULL);
		if (soup_uri)
			soup_uri_free(soup_uri);
	}

	soup_msg = soup_message_new("GET", uri);

	download_request *request = new(std::nothrow) download_request(strdup(file_path), cb, data);

	soup_session_queue_message(soup_session, soup_msg, __file_download_finished_cb, static_cast<void*>(request));
}

void download_manager::__file_download_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data)
{
	BROWSER_LOGD("");
	download_request *request = static_cast<download_request*>(data);

	SoupBuffer *body = soup_message_body_flatten(msg->response_body);

	int fd = 0;
	if (body->data && (body->length > 0) &&
		((fd = open(request->file_path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) >= 0))
	{
		unsigned int write_len = write(fd, body->data, body->length);
		close(fd);

		if (write_len == body->length)
		{
			request->cb(request->file_path, request->data);
		}
		else
		{
			unlink(request->file_path);
		}
	}

	soup_buffer_free(body);
	free(request->file_path);
	delete request;
}

