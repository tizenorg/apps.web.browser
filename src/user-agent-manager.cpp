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

#include "user-agent-manager.h"

#include <string>
#include <vconf-internal-browser-keys.h>
#include "browser.h"
#include "browser-dlog.h"
#include "webview.h"
#include "webview-list.h"
#include "preference.h"

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
	BROWSER_LOGD("user_agent_name = [%s]", user_agent_name);

	if (user_agent_name && !strcmp(system_user_agent_title, user_agent_name)) {
		char *user_agent = vconf_get_str(VCONFKEY_BROWSER_USER_AGENT);
		eina_stringshare_replace(&m_user_agent_string, user_agent);
		free(user_agent);
		free(user_agent_name);
		return m_user_agent_string;
	} else if (user_agent_name && !strcmp(tizen_user_agent_title, user_agent_name)) {
		eina_stringshare_replace(&m_user_agent_string, tizen_user_agent);
		free(user_agent_name);
		return m_user_agent_string;
	} else if (user_agent_name && !strcmp(chrome_user_agent_title, user_agent_name)) {
		eina_stringshare_replace(&m_user_agent_string, chrome_user_agent);
		free(user_agent_name);
		return m_user_agent_string;
	}

	free(user_agent_name);
	return NULL;
}


const char *user_agent_manager::get_desktop_user_agent(void)
{
	const char *user_agent = get_user_agent();
	if (!user_agent || !strlen(user_agent))
		return NULL;

	std::string desktop_user_agent = std::string(user_agent);
	int pos = desktop_user_agent.find(" Mobile");
	if (pos != std::string::npos)
		desktop_user_agent.replace(pos, strlen(" Desktop"), " Desktop");

	eina_stringshare_replace(&m_desktop_user_agent_string, desktop_user_agent.c_str());
	BROWSER_LOGD("m_desktop_user_agent_string=[%s]", m_desktop_user_agent_string);

	return m_desktop_user_agent_string;
}

void user_agent_manager::__vconf_changed_cb(keynode_t *keynode, void *data)
{
	char *key = vconf_keynode_get_name(keynode);
	BROWSER_LOGD("key = [%s]", key);

	user_agent_manager *uam = (user_agent_manager *)data;
	EINA_SAFETY_ON_NULL_RETURN(key);

	int webview_count = m_browser->get_webview_list()->get_count();
	const char *user_agent = uam->get_user_agent();
	for (int i = 0 ; i < webview_count ; i++) {
		webview *wv = m_browser->get_webview_list()->get_webview(i);
		wv->user_agent_set(user_agent);
	}

	m_preference->set_desktop_view_enabled(EINA_FALSE);
}

