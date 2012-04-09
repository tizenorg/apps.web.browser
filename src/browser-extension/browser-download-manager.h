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

#ifndef BROWSER_POLICY_DECISION_MAKER_H
#define BROWSER_POLICY_DECISION_MAKER_H

#include "browser-config.h"
#include "browser-view.h"

#include <string>

using namespace std;

class Browser_Common_View;

class Browser_Download_Manager : public Browser_Common_View {
public:
	Browser_Download_Manager(Evas_Object *navi_bar, Browser_View *browser_view);
	~Browser_Download_Manager(void);

	Eina_Bool init(void) {}

	void init(Evas_Object *webview);
	void deinit(void);
	void pause(void);
private:
	/* Elementary event callback functions */
	static void __popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __player_cb(void *data, Evas_Object *obj, void *event_info);
	static void __internet_cb(void *data, Evas_Object *obj, void *event_info);

	static void __download_request_cb(void *data, Evas_Object *obj, void *event_info);

	void _request_download(Ewk_Download *download_info);
	Eina_Bool _launch_download_app(void);
	Eina_Bool _call_streaming_player(void);
	string _get_extension_name_from_url(string &url);
	Eina_Bool _show_app_list_popup(void);
	const char *_get_app_name_from_pkg_name(string& pkg_name);

	Evas_Object *m_webview;
	Evas_Object *m_navi_bar;
	Evas_Object *m_list_popup;
	Evas_Object *m_app_list;
	Browser_View *m_browser_view;

	string m_url;
	string m_cookies;
	string m_default_player_pkg_name;
};
#endif /* BROWSER_POLICY_DECISION_MAKER_H */

