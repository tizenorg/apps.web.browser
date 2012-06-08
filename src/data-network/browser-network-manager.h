/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#ifndef BROWSER_NETWORK_MANAGER_H
#define BROWSER_NETWORK_MANAGER_H

#include "browser-config.h"

class Browser_View;
class Browser_Network_Manager {
public:
	Browser_Network_Manager(void);
	~Browser_Network_Manager(void);

	Eina_Bool init(Browser_View *browser_view, Evas_Object *webview);
private:
	static void __network_changed_ind_cb(keynode_t *keynode, void *data);
	static Eina_Bool __connection_cb(Evas_Object *webview, const char *uri);

	Browser_View *m_browser_view;
};
#endif /* BROWSER_NETWORK_MANAGER_H */

