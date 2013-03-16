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

#ifndef USER_AGENT_MANAGER_H
#define USER_AGENT_MANAGER_H

#include <vconf.h>
#include "browser-object.h"

#define system_user_agent_title "System user agent"
#define tizen_user_agent_title "Tizen"
#define chrome_user_agent_title "Chrome 20"
#define tizen_user_agent "Mozilla/5.0 (Linux; U; Tizen 1.0; en-us) AppleWebKit/534.46 (KHTML,like Gecko) Mobile Tizen Browser/1.0"
#define chrome_user_agent "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/536.11 (KHTML, like Gecko) Chrome/20.0.1132.57 Safari/536.11"

/* user_agent_manager instance should be one.
    Because the vconf_notify_key_changed is registered to monitor if the user agent vconf is changed.
    If you use own instance, vconf_notify_key_changed is called duplicately.
    If you want to access the user_agent_manager instance, user browser::get_user_agent_manager() */
class user_agent_manager : public browser_object {
public:
	user_agent_manager(void);
	~user_agent_manager(void);

	const char *get_user_agent(void);
	const char *get_desktop_user_agent(void);
private:
	static void __vconf_changed_cb(keynode_t *keynode, void *data);

	const char *m_user_agent_string;
	const char *m_desktop_user_agent_string;
};

#endif /* USER_AGENT_MANAGER_H */

