/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
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

