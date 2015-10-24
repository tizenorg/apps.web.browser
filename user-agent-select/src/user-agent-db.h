/*
 *  ug-browser-user-agent-efl
 *
 * Copyright (c) 2012 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *              Sangpyo Kim <sangpyo7.kim@samsung.com>
 *              Junghwan Kang <junghwan.kang@samsung.com>
 *              Inbum Chang <ibchang@samsung.com>
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

#ifndef USER_AGENT_DB_H
#define USER_AGENT_DB_H

/************************************************************************
 * Includes
 ***********************************************************************/
extern "C" {
#include "db-util.h"
}

#include <Elementary.h>
#include <Evas.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <vconf.h>
#include <map>


class User_Agent_DB {
public:

	User_Agent_DB(void);
	~User_Agent_DB(void);

	Eina_Bool get_user_agent_list(std::map<std::string, std::string>& list);
	Eina_Bool get_user_agent_title(std::string value, std::string &title);

private:
	sqlite3* m_db_descriptor;

	Eina_Bool _open_db(void);
	Eina_Bool _close_db(void);
};

#endif /* USER_AGENT_DB_H */
