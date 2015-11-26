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
 * Contact: Minseok Choi <min7.choi@samsung.com>
 *
 */

#include "DownloadControl.h"

#include <appsvc.h>
#include <app.h>
#include <app_manager.h>

#include <fcntl.h>
#include <notification.h>
#include <notification_internal.h>
#include <time.h>

#include "media_content.h"
#include "media_info.h"

#include "../../services/WebKitEngineService/WebView.h"

static const char *sdp_mime = "application/sdp";
//static const char *sdp_path = "/opt/usr/apps/" + BROWSER_APP_NAME + "/data/browser.sdp";

#define efl_scale	(elm_config_scale_get() / elm_app_base_scale_get())
#define default_download_item_name "download"

#define BROWSER_DATA_SCHEME_DOWNLOAD_ALLOW_MAX_COUNT 100000
#define APP_LIST_HEIGHT	(230 * efl_scale)


download_control::download_control(void)
	:m_download_uri()
{
	BROWSER_LOGD("");
}

download_control::~download_control(void)
{
	BROWSER_LOGD("");
}

void download_control::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
}

Evas_Object *download_control::brui_popup_add(Evas_Object *parent)
{
	Evas_Object *popup = NULL;
	popup = elm_popup_add(parent);
	elm_popup_align_set(popup, -1.0, 1.0);
	return popup;
}

Eina_Bool download_control::launch_streaming_player(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);

	bool vt_call_check = false;
	if (app_manager_is_running(sec_vt_app, &vt_call_check) < 0) {
		BROWSER_LOGE("Fail to get app running information");
		return EINA_FALSE;
	}

	if (vt_call_check) {
		//show_msg_popup(NULL, BR_STRING_WARNING_VIDEO_PLAYER);
		return EINA_FALSE;
	}

	app_control_h app_control = NULL;

	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return EINA_FALSE;
	}

	if (!app_control) {
		BROWSER_LOGE("app_control handle is NULL");
		return EINA_FALSE;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) < 0) {
		BROWSER_LOGE("app_control_set_operation failed");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_set_mime(app_control, "video/") < 0) {
		BROWSER_LOGE("Fail to app_control_set_mime");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	if (app_control_add_extra_data(app_control, "path", uri) < 0) {
		BROWSER_LOGE("Fail to set extra data");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
/*
	if (cookie) {
		if (app_control_add_extra_data(app_control, "cookie", cookie) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
	}
*/
	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}

	app_control_destroy(app_control);

	return EINA_TRUE;
}

void download_control::__sdp_download_finished_cb(const char *file_path, void *data)
{
	BROWSER_LOGD("file_path = [%s]", file_path);
	BROWSER_LOGD("data = [%p]", data);
	//launch_streaming_player(file_path);
}
/*
void download_control::__sdp_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	download_control *dc = (download_control *)data;
	//dc->request_file_download(dc->m_download_uri.c_str(), sdp_path, __sdp_download_finished_cb, NULL);

	//m_browser->get_browser_view()->destroy_popup(obj);
}

void download_control::__internet_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	download_control *dc = (download_control *)data;

	dc->launch_download_app(dc->m_download_uri.c_str());

	//m_browser->get_browser_view()->destroy_popup(obj);
}
*/
char *download_control::get_file_size_str(const char *full_path)
{
	FILE *fp = NULL;
	unsigned long long size = 0;
	double size_double = 0.0f;
	char size_str[10 + 1] = {0, };
	std::string size_text;

	fp = fopen(full_path, "r");
	if (!fp)
		return NULL;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fclose(fp);

	if (size >= unit_size) {
		size_double = (double)size / (double)unit_size;
		if (size_double >= unit_size) {
			size_double = (double)size_double / (double)unit_size;
			if (size_double >= unit_size) {
				size_double = (double)size_double / (double)unit_size;
				snprintf(size_str, 10, "%.2f", size_double);
				size_text = std::string(size_str) + std::string("GB");
			} else {
				snprintf(size_str, 10, "%.2f", size_double);
				size_text = std::string(size_str) + std::string("MB");
			}
		} else {
			snprintf(size_str, 10, "%.2f", size_double);
			size_text = std::string(size_str) + std::string("KB");
		}
	} else {
		snprintf(size_str, 10, "%u", (int)size);
		size_text = std::string(size_str) + std::string("B");
	}

	return strdup(size_text.c_str());
}

Eina_Bool download_control::_check_file_exist(const char *path)
{
	BROWSER_LOGD("path = [%s]", path);

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

Eina_Bool download_control::_save_file(const char *raw_data, const char *path)
{
	BROWSER_LOGD("");

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

Eina_Bool download_control::_update_contents_on_media_db(const char *path)
{
	BROWSER_LOGD("");

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

Eina_Bool download_control::_get_download_path(const char *extension, char **full_path, char **file_name)
{
	BROWSER_LOGD("");

	char temp_count_str[10] = {0};
	int count = 0;

	std::string storing_path;

/*	if (m_preference->get_default_storage_type() == DEFAULT_STORAGE_TYPE_MEMORY_CARD)
		storing_path = std::string(default_sd_card_storage_path);
	else //if (m_preference->get_default_storage_type() == DEFAULT_STORAGE_TYPE_PHONE)*/
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


Eina_Bool download_control::_set_downloaded_file_on_notification(const char *downloaded_path, const char *file_name, const char *uri)
{
	BROWSER_LOGD("");

	notification_h noti_handle = notification_create(NOTIFICATION_TYPE_NOTI);

	Eina_Bool success = EINA_FALSE;
	bundle *bundle = NULL;
	char *file_size_str = get_file_size_str(downloaded_path);

	int priv_Id = 0;
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
/*
	err = notification_set_image(noti_handle, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, browser_img_dir"/Q01_notification_download_complete.png");
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Fail to notification_set_image(NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR) with error : [%d]", err);
		goto ERROR;
	}
*/
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

void download_control::handle_download_request(const char *uri, const char *content_type)
{
	BROWSER_LOGD("uri = [%s], content_type = [%s]", uri, content_type);

	m_download_uri.clear();

	m_download_uri = std::string(uri);

	if (!content_type) {
		launch_download_app(uri);
		return;
	}

	if (!strcmp(content_type, sdp_mime)) {
		Evas_Object *popup = brui_popup_add(m_parent);

		Evas_Object *app_list = elm_list_add(popup);

		elm_list_mode_set(app_list, ELM_LIST_EXPAND);
		//elm_list_item_append(app_list, BR_STRING_STREAMING_PLAYER, NULL, NULL, __sdp_cb, this);
		//elm_list_item_append(app_list, BR_STRING_INTERNET, NULL, NULL, __internet_cb, this);
		evas_object_size_hint_max_set(app_list, -1, APP_LIST_HEIGHT);

		//m_browser->get_browser_view()->show_content_popup(popup, BR_STRING_SELECT_AVAILABLE_APP, app_list, NULL, BR_STRING_CANCEL);
	} else {
		launch_download_app(uri);
	}
}

Eina_Bool download_control::handle_data_scheme(const char *uri)
{
	BROWSER_LOGD("");

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
		BROWSER_LOGE("failed to _save_file with path [%s]", full_path);
		goto HANDLE_DATA_SCHEME_ERROR;
	}

	if (_update_contents_on_media_db(full_path) == EINA_FALSE) {
		BROWSER_LOGE("failed to _update_contents_on_media_db with path [%s]", full_path);
		goto HANDLE_DATA_SCHEME_ERROR;
	}

	if (_set_downloaded_file_on_notification(full_path, file_name, uri) == EINA_FALSE) {
		BROWSER_LOGE("failed to _set_downloaded_file_on_notification with path [%s] [uri]", file_name, uri);
		goto HANDLE_DATA_SCHEME_ERROR;
	}

	return_val = EINA_TRUE;

HANDLE_DATA_SCHEME_ERROR:
	free(full_path);
	free(file_name);

	return return_val;
}

Eina_Bool download_control::launch_download_app(const char *uri)
{

	BROWSER_LOGD("uri = [%s]", uri);

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
	/*if (m_preference->get_default_storage_type() == DEFAULT_STORAGE_TYPE_MEMORY_CARD)
		storage_type = "1";
*/

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

	if ( app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	BROWSER_LOGD("===================show noti=============1==========");

	show_noti_popup(BR_STRING_DOWNLOADING_ING);
	//m_browser->get_browser_view()->show_noti_popup(BR_STRING_DOWNLOADING_ING);
	BROWSER_LOGD("===================show noti==========2============");

	return EINA_TRUE;
}

void download_control::show_noti_popup(const char *msg)
{
	BROWSER_LOGD("");
//	RET_MSG_IF(!msg, "msg is NULL");
	BROWSER_LOGD("===================show noti==========3============");
	int tmp;
	BROWSER_LOGD("==NOTIFICATION_ERROR_NONE==%d==", NOTIFICATION_ERROR_NONE);
	BROWSER_LOGD("==NOTIFICATION_ERROR_INVALID_PARAMETER==%d=", NOTIFICATION_ERROR_INVALID_PARAMETER);
	BROWSER_LOGD("==NOTIFICATION_ERROR_FROM_DBUS=--%d==", NOTIFICATION_ERROR_FROM_DBUS);
	BROWSER_LOGD("==NOTIFICATION_ERROR_PERMISSION_DENIED==%d===", NOTIFICATION_ERROR_PERMISSION_DENIED);

	tmp = notification_status_message_post(msg);
	BROWSER_LOGD("==noti_result==%d===", tmp);
	elm_access_say(msg);
}

