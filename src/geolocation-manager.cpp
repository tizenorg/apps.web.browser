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

#include "geolocation-manager.h"

extern "C" {
#include "db-util.h"
}

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "geolocation-item.h"
#include "webview.h"

#define geolocation_db_path	"/opt/usr/apps/org.tizen.browser/data/db/.browser-geolocation.db"

static Eina_Bool _is_geolocation_exist(const char *uri)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(geolocation_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select accept from geolocation where address=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize failed");
		db_util_close(descriptor);
		return EINA_TRUE;
	} else {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize failed");
		db_util_close(descriptor);
	}

	return EINA_FALSE;
}

static Eina_Bool _update_geolocation(const char *uri, bool accept)
{
	BROWSER_LOGD("uri[%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(geolocation_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "update geolocation set accept=?, updatedate=DATETIME('now') where address=?", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_int(sqlite3_stmt, 1, accept) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize for accept is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 2, uri, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	db_util_close(descriptor);

	return EINA_TRUE;
}

static Eina_Bool _save_geolocation(const char *uri, const char *title, bool accept)
{
	BROWSER_LOGD("uri[%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(geolocation_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "insert into geolocation (address, title, accept, updatedate) values(?, ?, ?, DATETIME('now'))", -1, &sqlite3_stmt, NULL);

	if (sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("uri save : SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (title) {
		if (sqlite3_bind_text(sqlite3_stmt, 2, title, -1, NULL) != SQLITE_OK) {
			BROWSER_LOGD("title save : SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			db_util_close(descriptor);
			return EINA_FALSE;
		}
	} else {
		if (sqlite3_bind_text(sqlite3_stmt, 2, "", -1, NULL) != SQLITE_OK) {
			BROWSER_LOGD("title save : SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			db_util_close(descriptor);
			return EINA_FALSE;
		}
	}
	if (sqlite3_bind_int(sqlite3_stmt, 3, accept) != SQLITE_OK) {
		BROWSER_LOGD("accept save :SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_step - sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	db_util_close(descriptor);

	return EINA_TRUE;
}

static Eina_Bool _delete_geolocation(const char *uri)
{
	BROWSER_LOGD("uri[%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(geolocation_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "delete from geolocation where address=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.");

	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_OK && error != SQLITE_DONE) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	db_util_close(descriptor);

	return EINA_TRUE;
}

static int _get_count(void)
{
	BROWSER_LOGD("");

	sqlite3 *descriptor = NULL;
	int count = 0;
	int error = db_util_open(geolocation_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return -1;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select count(*) from geolocation", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return -1;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_ROW) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return -1;
	}

	count = sqlite3_column_int(sqlite3_stmt, 0);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return -1;
	}

	BROWSER_LOGD("total num of geolocation count[%d]", count);
	return count;
}


geolocation_manager::geolocation_manager(void)
:
	m_geolocation_never_ask_checkbox(NULL)
	,m_user_data(NULL)
{
	BROWSER_LOGD("");
}

geolocation_manager::~geolocation_manager(void)
{
	BROWSER_LOGD("");
}

Eina_Bool geolocation_manager::save_geolocation(const char *uri, bool accept, const char *title)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	int error = 0;

	if (_is_geolocation_exist(uri) == EINA_TRUE) {
		error = _update_geolocation(uri, accept);
	} else {
		error = _save_geolocation(uri, title, accept);
	}

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

Eina_Bool geolocation_manager::get_geolocation_allow(const char *uri, bool &accept)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	if (check_geolocation_setting_exist(uri) == EINA_FALSE) {
		BROWSER_LOGE("No data or database error\n");
		return EINA_FALSE;
	}

	sqlite3 *descriptor = NULL;
	int error = db_util_open(geolocation_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select accept from geolocation where address=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return -1;
	}

	error = sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
		accept = sqlite3_column_int(sqlite3_stmt, 0);
		BROWSER_LOGD("uri: %s accept %d", uri, accept);
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	db_util_close(descriptor);

	if (error == SQLITE_DONE || error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool geolocation_manager::delete_geolocation_setting(const char *uri)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	return _delete_geolocation(uri);
}

Eina_Bool geolocation_manager::delete_all(void)
{
	BROWSER_LOGD("");

	sqlite3 *descriptor = NULL;
	int error = db_util_open(geolocation_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "delete from geolocation", -1, &sqlite3_stmt, NULL);
	sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize failed");
		return EINA_FALSE;
	}

	db_util_close(descriptor);

	return EINA_TRUE;
}

void geolocation_manager::show_geolocation_allow_popup(const char *host_address, void *user_data)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(host_address);

	/* Make popup for permission request to user */
	char *request_msg = NULL;
	int host_string_len = strlen(host_address);
	int popup_string_len = strlen(BR_STRING_REQUEST_LOCATION) + 2;// margin of %s

	request_msg = (char *)malloc((host_string_len + popup_string_len + 1) * sizeof(char));// 1 is reserves for '\0'
	if (!request_msg) {
		BROWSER_LOGE("Failed to allocate memory for request_msg");
		return;
	}
	memset(request_msg, 0, (host_string_len + popup_string_len + 1));// 1 is reserves for '\0'
	snprintf(request_msg, (host_string_len + popup_string_len), BR_STRING_REQUEST_LOCATION, host_address);

	Evas_Object *popup_layout = elm_layout_add(m_window);
	if (!popup_layout) {
		BROWSER_LOGE("Failed to elm_layout_add");
		free(request_msg);
		return;
	}
	elm_layout_file_set(popup_layout, browser_edj_dir"/browser-popup.edj", "geolocation_popup");
	evas_object_size_hint_weight_set(popup_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *scroller = elm_scroller_add(popup_layout);
	if (!scroller) {
		BROWSER_LOGE("Failed to elm_scroller_add");
		free(request_msg);
		return;
	}
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	Evas_Object *label = elm_label_add(scroller);
	if (!label) {
		BROWSER_LOGE("Failed to elm_label_add");
		free(request_msg);
		return;
	}
	elm_label_line_wrap_set(label, ELM_WRAP_WORD);
	elm_object_text_set(label, request_msg);

	m_geolocation_never_ask_checkbox = elm_check_add(scroller);
	if (!m_geolocation_never_ask_checkbox) {
		BROWSER_LOGE("Failed to elm_check_add");
		free(request_msg);
		return;
	}
	elm_check_state_set(m_geolocation_never_ask_checkbox, EINA_TRUE);
	elm_object_part_content_set(popup_layout, "elm.swallow.checkbox", m_geolocation_never_ask_checkbox);

	elm_object_content_set(scroller, label);
	elm_object_part_content_set(popup_layout, "elm.swallow.scroller", scroller);
	edje_object_part_text_set(elm_layout_edje_get(popup_layout), "elm.text.never_ask", BR_STRING_REMEMBER_SETTING);

	m_user_data = (void *)user_data;
	m_browser->get_browser_view()->show_content_popup(NULL,
												popup_layout,
												BR_STRING_YES,
												__request_geolocation_permission_ok_cb,
												BR_STRING_NO,
												__request_geolocation_permission_no_cb,
												this);
	free(request_msg);
	request_msg = NULL;
}

std::vector<geolocation_item *> geolocation_manager::get_geolocation_list(void)
{
	BROWSER_LOGD("");

	std::vector<geolocation_item *> geolocation_list;
	sqlite3 *descriptor = NULL;
	int error = db_util_open(geolocation_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return geolocation_list;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select id, address, title, accept from geolocation order by id desc", -1, &sqlite3_stmt, NULL);
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		const char *uri = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));
		const char *title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 2));
		bool accept = (bool)sqlite3_column_int(sqlite3_stmt, 3);
		BROWSER_LOGD("uri=[%s], accept=[%d], title[%s]", uri, accept, title);
		if (!uri || !strlen(uri)) {
			BROWSER_LOGE("invalid geolocation");
			continue;
		}

		geolocation_item *item = new geolocation_item(uri, accept, title);
		geolocation_list.push_back(item);
	}

	db_util_close(descriptor);

	return geolocation_list;
}

Eina_Bool geolocation_manager::check_geolocation_setting_exist(const char *uri)
{
	BROWSER_LOGD("uri[%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	return _is_geolocation_exist(uri);
}

int geolocation_manager::get_count(void)
{
	BROWSER_LOGD("");
	return _get_count();
}

void geolocation_manager::__request_geolocation_permission_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	geolocation_manager *gm = (geolocation_manager *)data;
	Ewk_Geolocation_Permission_Request *permission_request = (Ewk_Geolocation_Permission_Request *)gm->m_user_data;
	const Ewk_Security_Origin *origin = ewk_geolocation_permission_request_origin_get(permission_request);
	const char *host_address = ewk_security_origin_host_get(origin);

	Eina_Bool checkbox_state = elm_check_state_get(gm->m_geolocation_never_ask_checkbox);

	if (checkbox_state == EINA_TRUE) {
		if (host_address && strlen(host_address))
			m_browser->get_geolocation_manager()->save_geolocation(host_address, true);
	}

	ewk_geolocation_permission_request_set(permission_request, EINA_TRUE);
}

void geolocation_manager::__request_geolocation_permission_no_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	geolocation_manager *gm = (geolocation_manager *)data;
	Ewk_Geolocation_Permission_Request *permission_request = (Ewk_Geolocation_Permission_Request *)gm->m_user_data;
	const Ewk_Security_Origin *origin = ewk_geolocation_permission_request_origin_get(permission_request);
	const char *host_address = ewk_security_origin_host_get(origin);

	Eina_Bool checkbox_state = elm_check_state_get(gm->m_geolocation_never_ask_checkbox);

	if (checkbox_state == EINA_TRUE) {
		if (host_address && strlen(host_address))
			m_browser->get_geolocation_manager()->save_geolocation(host_address, false);
	}

	ewk_geolocation_permission_request_set(permission_request, EINA_FALSE);
}

