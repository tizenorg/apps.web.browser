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

#define efl_scale	(elm_config_scale_get() / elm_app_base_scale_get())
#define default_download_item_name "download"

#define BROWSER_DATA_SCHEME_DOWNLOAD_ALLOW_MAX_COUNT 100000
#define APP_LIST_HEIGHT	(230 * efl_scale)

download_control::download_control(void)
    :m_download_uri()
{
}

download_control::~download_control(void)
{
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
}

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
        evas_object_size_hint_max_set(app_list, -1, APP_LIST_HEIGHT);
    } else {
        launch_download_app(uri);
    }
}

Eina_Bool download_control::handle_data_scheme(const char *uri)
{
    BROWSER_LOGD("");

    const char *encoded_str = NULL;
    const char *extension = NULL;

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
        return EINA_FALSE;
    }

    if (!(full_path && strlen(full_path)) || !(file_name && strlen(file_name))) {
        BROWSER_LOGE("has problem to _get_download_path");
        return EINA_FALSE;
    }

    if (_save_file(encoded_str, full_path) == EINA_FALSE) {
        BROWSER_LOGE("failed to _save_file with path [%s]", full_path);
        return EINA_FALSE;
    }

    if (_update_contents_on_media_db(full_path) == EINA_FALSE) {
        BROWSER_LOGE("failed to _update_contents_on_media_db with path [%s]", full_path);
        return EINA_FALSE;
    }

    return  EINA_TRUE;
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

    return EINA_TRUE;
}

void download_control::show_noti_popup(const char *msg)
{
    BROWSER_LOGD("");
    notification_status_message_post(msg);
    elm_access_say(msg);
}

