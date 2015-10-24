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

#include "cloud-sync-manager.h"

#include <ITapiModem.h>
#include <algorithm>
#include <openssl/md5.h>
#include <string>
#include <system_info.h>
#include <web_tab.h>

#include "browser.h"
#include "browser-view.h"
#include "browser-dlog.h"
#include "history.h"
#include "main-view.h"
#include "platform-service.h"
#include "preference.h"
#include "url-bar.h"
#include "webview.h"
#include "webview-list.h"

#define MD5_DIGEST_LEN 16

typedef struct _tab_snapshot_data {
	int tab_id;
	void *icon_data;
	int w;
	int h;
	int len;
} tab_snapshot_data;


tab_sync_info::tab_sync_info(int index, const char *title, const char *uri, Eina_Bool activate,
	Eina_Bool incognito, int tab_id, const char *device_name, const char *device_id)
	: m_index(index)
	, m_title(NULL)
	, m_uri(NULL)
	, m_is_activated(activate)
	, m_is_incognito(incognito)
	, m_tab_id(tab_id)
	, m_device_name(NULL)
	, m_device_id(NULL)
{
	if (title)
		m_title = strdup(title);

	if (uri)
		m_uri = strdup(uri);

	if (device_name)
		m_device_name = strdup(device_name);

	if (device_id)
		m_device_id = strdup(device_id);
}

tab_sync_info::~tab_sync_info(void)
{
	if (m_title)
		free(m_title);

	if (m_uri)
		free(m_uri);

	if (m_device_name)
		free(m_device_name);

	if (m_device_id)
		free(m_device_id);
}

cloud_sync_manager::cloud_sync_manager(void)
	: m_device_name(NULL)
	, m_device_id(NULL)
	, m_snapshot_thread(NULL)
	, m_working_webview(NULL)
{

	TapiHandle *handle = tel_init(NULL);
	int status = 0;

	// Check modem is available.
	int ret = tel_check_modem_power_status(handle, &status);
	BROWSER_LOGD("mode status: %d, ret: %d", status, ret);
	if (status != 0) {
		// Modem error.
		m_device_id = eina_stringshare_add("000001111122222");
		BROWSER_LOGD("IMEI read fail");
		m_browser->get_browser_view()->show_noti_popup("Failed to read IMEI");
	} else {
		char *imei = tel_get_misc_me_imei_sync(handle);
		BROWSER_SECURE_LOGD("******* imei=[%s]", imei);
		if (imei)
			m_device_id = eina_stringshare_add(imei);
		else
			m_device_id = eina_stringshare_add("333334444455555");
	}
	tel_deinit(handle);

	char *model_name = NULL;
	if (system_info_get_platform_string("http://tizen.org/system/model_name", &model_name) == SYSTEM_INFO_ERROR_NONE) {
		BROWSER_SECURE_LOGD("model name: %s", model_name);
		m_device_name = eina_stringshare_add(model_name);
		free(model_name);
	} else {
		BROWSER_LOGE("can not get model name");
		m_device_name = eina_stringshare_add("Tizen Device");
	}

	if (bp_tab_adaptor_initialize() == -1)
		BROWSER_LOGD("bp_tab_adaptor_initialize failed");

	// Make hash key with IMEI.
	MD5_CTX context;
	unsigned char digest[MD5_DIGEST_LEN] = { 0, };

	MD5_Init(&context);
	MD5_Update(&context, m_device_id, eina_stringshare_strlen(m_device_id));
	MD5_Final(digest, &context);

	for (int i = 0; i < MD5_DIGEST_LEN; i++)
		digest[i] = '0' + (digest[i] % 10);

	eina_stringshare_replace(&m_device_id, (const char *)digest);
	BROWSER_SECURE_LOGD("*** device id: %s", m_device_id);
}

cloud_sync_manager::~cloud_sync_manager(void)
{
	BROWSER_LOGD("");

	std::vector<tab_sync_info *> tab_sync_list = get_tab_sync_info_list();

	m_preference->set_tab_counts((int)tab_sync_list.size());
	for (unsigned int i = 0; i < tab_sync_list.size(); i++)
		delete tab_sync_list[i];

	eina_stringshare_del(m_device_id);
	eina_stringshare_del(m_device_name);

	bp_tab_adaptor_deinitialize();

	if (m_snapshot_thread) {
		BROWSER_LOGD("cancel snapshot thread");
		ecore_thread_cancel(m_snapshot_thread);
		m_snapshot_thread = NULL;
	}
}

std::vector<tab_sync_info *> cloud_sync_manager::get_tab_sync_info_list(Eina_Bool my_device)
{
	BROWSER_LOGD("my device: %d", my_device);

	int *ids = NULL;
	int count = 0;

	if (my_device) {
		// Get my tab info.
		if (bp_tab_adaptor_get_duplicated_ids_p(&ids, &count, -1, 0, BP_TAB_O_INDEX, 0 /* ASC */,
			BP_TAB_O_DEVICE_ID, m_device_id, 0) < 0) {
			BROWSER_LOGE("bp_tab_adaptor_get_duplicated_ids_p fails");
			return std::vector<tab_sync_info *>();
		}
	} else {
		// Get other tab info.
		if (bp_tab_adaptor_get_duplicated_ids_p(&ids, &count, -1, 0, BP_TAB_O_DEVICE_ID, 0 /* ASC */,
			BP_TAB_O_DEVICE_ID, m_device_id, -1) < 0) {
			BROWSER_LOGE("bp_tab_adaptor_get_duplicated_ids_p fails");
			return std::vector<tab_sync_info *>();
		}
	}
	BROWSER_LOGD("count: %d", count);

	std::vector<tab_sync_info *> tab_list;
	bp_tab_offset offset = BP_TAB_O_INDEX | BP_TAB_O_IS_ACTIVATED |
			BP_TAB_O_IS_INCOGNITO | BP_TAB_O_URL | BP_TAB_O_TITLE |
			BP_TAB_O_DEVICE_NAME | BP_TAB_O_DEVICE_ID;

	for (int i = 0 ; i < count ; i++) {
		int tab_id = *(ids + i);
		BROWSER_LOGD("tab_id=[%d]", tab_id);

		bp_tab_info_fmt tab_fmt;
		memset(&tab_fmt, 0x00, sizeof(bp_tab_info_fmt));

		if (bp_tab_adaptor_get_info(tab_id, offset, &tab_fmt) == 0) {
			if (tab_fmt.device_name) {
				tab_sync_info *tab_info = new tab_sync_info(tab_fmt.index,
					tab_fmt.title, tab_fmt.url, tab_fmt.is_activated,
					tab_fmt.is_incognito, tab_id, tab_fmt.device_name,
					tab_fmt.device_id);
				tab_list.push_back(tab_info);
			}
			bp_tab_adaptor_easy_free(&tab_fmt);
		}
	}

	if (ids)
		free(ids);

	return tab_list;
}

void cloud_sync_manager::unsync_webview(webview *wv)
{
	RET_MSG_IF(!wv, "wv is NULL");

	if (wv->sync_id_get() != -1)
		bp_tab_adaptor_delete(wv->sync_id_get());
}

void cloud_sync_manager::unsync_tab(int id)
{
	bp_tab_adaptor_delete(id);
}

void cloud_sync_manager::sync_webview(webview_info *wvi)
{
	RET_MSG_IF(!wvi, "wvi is NULL");

	// Do not preserve incognito tabs
	if (wvi->is_private_mode_enabled()) {
		delete wvi;
		return;
	}

	// Do not sync if the uri is NULL.
	const char *uri = wvi->get_uri();
	std::string uri_str;
	if (!uri) {
			delete wvi;
			return;
	}

	// Do not sync if the scheme is data:.
	if (strlen(uri) > strlen("data:") && !strncmp(uri, "data:", 5)) {
		delete wvi;
		return;
	}

	// Can not save tab if the uri length is over than 4K.
	if (strlen(uri) >= 4096) {
		delete wvi;
		return;
	}

	int wv_tab_id = wvi->get_sync_id();

	bp_tab_info_fmt tab_info;
	memset(&tab_info, 0x00, sizeof(bp_tab_info_fmt));

	if (wv_tab_id < 0) {
		// if newly created, index has to be set to zero
		tab_info.index = 0;
		webview_list *wv_list = m_browser->get_webview_list();
		for (unsigned int i = 1; i < wv_list->get_count(); i++) { // start at 2nd item because newly created was moved to beginning of list
		         if(!wv_list->get_webview(i))
				return;
			int tab_id = wv_list->get_webview(i)->sync_id_get();
			int old_index = 0;
			bp_tab_adaptor_get_index(tab_id, &old_index);
			BROWSER_LOGD("prev_index[%d] new_index[%d]",old_index, i);
			bp_tab_adaptor_set_index(tab_id, i);
			bp_tab_adaptor_set_dirty(tab_id);
		}
	} else
		tab_info.index = m_browser->get_webview_list()->get_index(wvi->get_webview());

	BROWSER_LOGD("tab_info.index[%d]", tab_info.index);
	if (tab_info.index < 0) {
		delete wvi;
		return;
	}
	BROWSER_LOGD("tab_info.index[%d]", tab_info.index);
	tab_info.is_activated = -1;

	tab_info.url = (char *)uri;
	tab_info.title = (char *)wvi->get_title();

	if (wvi->is_private_mode_enabled())
		tab_info.is_incognito = 1;
	else
		tab_info.is_incognito = 0;

	tab_info.device_name = (char *)m_device_name;
	tab_info.device_id = (char *)m_device_id;

	BROWSER_SECURE_LOGD("tab_info.url=[%s]", tab_info.url);
	bp_tab_adaptor_easy_create(&wv_tab_id, &tab_info);
	if (m_browser->get_browser_view()->get_current_webview() == wvi->get_webview())
		bp_tab_adaptor_activate(wv_tab_id);

	// Save favicon&snapshot for sync.
	_set_tab_favicon(wv_tab_id, tab_info.url);

	if (wvi->get_snapshot()) {
		m_working_webview = wvi->get_webview();
		set_tab_snapshot(wv_tab_id, wvi->get_snapshot());
	} else {
		// Hide url bar.
		browser_view *bv = m_browser->get_browser_view();
		webview *wv = bv->get_current_webview();
		url_bar *ub = bv->get_url_bar();
		if (bv->is_top_view() == EINA_TRUE && wvi->get_webview() == wv &&
			wv->is_error_page() == EINA_FALSE && ub->is_loading_status() == EINA_FALSE)
			m_browser->get_main_view()->show_url_bar_animator(EINA_FALSE);
	}

	if (m_browser->get_webview_list()->is_webview_exist(wvi->get_webview()))
		wvi->get_webview()->sync_id_set(wv_tab_id);

	delete wvi;
}

void cloud_sync_manager::activate_webview(int tab_id)
{
	BROWSER_SECURE_LOGD("id: %d", tab_id);

	if (tab_id < 0)
		return;

	bp_tab_adaptor_activate(tab_id);
	bp_tab_adaptor_set_date_modified(tab_id, -1);
}

void cloud_sync_manager::_set_tab_favicon(int tab_id, char *url)
{
	Evas_Object *favicon = m_browser->get_history()->get_history_favicon(url);
	if (!favicon)
		return;

	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	platform_service ps;
	ps.evas_image_size_get(favicon, &w, &h, &stride);

	BROWSER_SECURE_LOGD("favicon w=[%d], h=[%d], stride=[%d]", w, h, stride);

	len = stride * h;
	if (len == 0)
		return;

	void *icon_data = evas_object_image_data_get(favicon, EINA_TRUE);
	if (bp_tab_adaptor_set_icon(tab_id, w, h, (const unsigned char *)icon_data, len) < 0) {
		BROWSER_SECURE_LOGE("Failed to set Favicon info for URL= %s", url);
		return;
	}

	SAFE_FREE_OBJ(favicon);
}

void cloud_sync_manager::set_tab_snapshot(int tab_id, Evas_Object *snapshot)
{
	RET_MSG_IF(!snapshot, "snapshot is NULL");

	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	platform_service ps;
	ps.evas_image_size_get(snapshot, &w, &h, &stride);

	BROWSER_SECURE_LOGD("thumbnail w=[%d], h=[%d], stride=[%d]", w, h, stride);

	len = stride * h;
	if (len == 0)
		return;

	tab_snapshot_data *snapshot_data = (tab_snapshot_data *)malloc(sizeof(tab_snapshot_data));
	if (!snapshot_data) {
		BROWSER_LOGE("malloc fail");
		return;
	}
	memset(snapshot_data, 0x0, sizeof(tab_snapshot_data));
	snapshot_data->icon_data = malloc(len);
	if (!snapshot_data->icon_data) {
		BROWSER_LOGE("Fail to malloc");
		free(snapshot_data);
		return;
	}
	snapshot_data->tab_id = tab_id;
	snapshot_data->w = w;
	snapshot_data->h = h;
	snapshot_data->len = len;
	memcpy(snapshot_data->icon_data, evas_object_image_data_get(snapshot, EINA_TRUE), len);

	m_snapshot_thread = ecore_thread_run(__set_tab_snapshot_thread, __thread_end_cb, NULL, snapshot_data);
}

void cloud_sync_manager::__set_tab_snapshot_thread(void *data, Ecore_Thread *th)
{
	BROWSER_LOGD("-- START");

	tab_snapshot_data *snapshot_data = (tab_snapshot_data *)data;

	if (bp_tab_adaptor_set_snapshot(snapshot_data->tab_id, snapshot_data->w, snapshot_data->h,
		(const unsigned char *)(snapshot_data->icon_data), snapshot_data->len) < 0)
		BROWSER_SECURE_LOGE("Failed to set Favicon info: %d", snapshot_data->tab_id);

	free(snapshot_data->icon_data);
	free(snapshot_data);

	BROWSER_LOGD("-- END");
}

void cloud_sync_manager::__thread_end_cb(void *data, Ecore_Thread *th)
{
	BROWSER_LOGD("");

	cloud_sync_manager *csm = m_browser->get_cloud_sync_manager();
	if (!csm)
		return;

	csm->m_snapshot_thread = NULL;

	// Hide url bar after save snapshot.
	browser_view *bv = m_browser->get_browser_view();
	webview *wv = bv->get_current_webview();
	url_bar *ub = bv->get_url_bar();
	if (bv->is_top_view() == EINA_TRUE && csm->m_working_webview == wv &&
		wv->is_error_page() == EINA_FALSE && ub->is_loading_status() == EINA_FALSE)
		m_browser->get_main_view()->show_url_bar_animator(EINA_FALSE);

	csm->m_working_webview = NULL;
}

Evas_Object *cloud_sync_manager::get_tab_snapshot(int tab_id)
{
	BROWSER_LOGD("id : %d", tab_id);
	void *snapshot_data = NULL;
	int w = 0;
	int h = 0;
	int len = 0;

	Evas_Object *icon = NULL;

	int ret = bp_tab_adaptor_get_snapshot(tab_id, &w, &h, (unsigned char **)&snapshot_data, &len);
	if (ret < 0) {
		BROWSER_LOGE("bp_bookmark_adaptor_set_thumbnail is failed");
		return NULL;
	}

	BROWSER_LOGE("len: %d, w:%d, h:%d", len, w, h);
	if (len > 0){
		icon = evas_object_image_filled_add(evas_object_evas_get(m_window));
		evas_object_image_colorspace_set(icon, EVAS_COLORSPACE_ARGB8888);
		evas_object_image_size_set(icon, w, h);
		evas_object_image_fill_set(icon, 0, 0, w, h);
		evas_object_image_filled_set(icon, EINA_TRUE);
		evas_object_image_alpha_set(icon,EINA_TRUE);

		void *target_image_data = evas_object_image_data_get(icon, EINA_TRUE);
		memcpy(target_image_data, snapshot_data, len);
		evas_object_image_data_set(icon, target_image_data);
	}

	return icon;
}
