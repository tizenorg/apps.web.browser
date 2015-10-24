
/*
 *  browser
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "user-agent-manager.h"

#include <string>
#include <vconf-internal-browser-keys.h>
#include "browser.h"
#include "browser-dlog.h"
#include "webview.h"
#include "webview-list.h"
#include "preference.h"
#include "browser-object.h"

extern "C" {
#include "db-util.h"
}

#define user_agent_db_path	browser_data_dir"/db/.browser.db"

user_agent_manager::user_agent_manager(void)
:
	m_user_agent_string(NULL)
	,m_desktop_user_agent_string(NULL)
{
	BROWSER_LOGD("");

	if (vconf_notify_key_changed(VCONFKEY_BROWSER_BROWSER_USER_AGENT, __vconf_changed_cb, this) < 0)
		BROWSER_LOGE("user agent vconf_notify_key_changed failed");
}

user_agent_manager::~user_agent_manager(void)
{
	BROWSER_LOGD("");
	eina_stringshare_del(m_user_agent_string);
	eina_stringshare_del(m_desktop_user_agent_string);
}

const char *user_agent_manager::get_user_agent(void)
{
	char *user_agent_name = vconf_get_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT);
	BROWSER_SECURE_LOGD("user_agent_name = [%s]", user_agent_name);

	if (user_agent_name && (!strcmp(system_user_agent_title, user_agent_name) || !strcmp(tizen_user_agent_title, user_agent_name))) {
		// If the user agent is default(System user agent), set as null.
		// The webkit sets the own user agent in webkit side.
		free(user_agent_name);
		return NULL;
	}

	sqlite3* descriptor = NULL;
	int error = db_util_open(user_agent_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		free(user_agent_name);
		return NULL;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select value from user_agents where name=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("error = [%d]", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.");

		db_util_close(descriptor);
		free(user_agent_name);
		return NULL;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, user_agent_name, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.");

	error = sqlite3_step(sqlite3_stmt);
	BROWSER_LOGE("error=%d", error);
	if (error == SQLITE_DONE || error == SQLITE_ROW) {
		const char *user_agent = reinterpret_cast<const char*>(sqlite3_column_text(sqlite3_stmt, 0));
		BROWSER_SECURE_LOGD("user_agent = [%s]", user_agent);
		eina_stringshare_replace(&m_user_agent_string, user_agent);

		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.");

		free(user_agent_name);
		db_util_close(descriptor);
		return m_user_agent_string;
	}
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.");

	db_util_close(descriptor);

	free(user_agent_name);

	return NULL;
}

void user_agent_manager::__vconf_changed_cb(keynode_t *keynode, void *data)
{
	char *key = vconf_keynode_get_name(keynode);
	BROWSER_SECURE_LOGD("key = [%s]", key);

	user_agent_manager *uam = (user_agent_manager *)data;
	RET_MSG_IF(!key, "key is NULL");

	int webview_count = m_browser->get_webview_list()->get_count();
	const char *user_agent = uam->get_user_agent();
	BROWSER_SECURE_LOGD("user_agent=[%s]", user_agent);
	for (int i = 0 ; i < webview_count ; i++) {
		webview *wv = m_browser->get_webview_list()->get_webview(i);
		if (wv) {
			wv->user_agent_set(user_agent);
		}
	}

	m_preference->set_desktop_view_enabled(EINA_FALSE);
}

