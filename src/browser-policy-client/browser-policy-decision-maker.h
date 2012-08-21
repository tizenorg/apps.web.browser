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
 *
 */


#ifndef BROWSER_POLICY_DECISION_MAKER_H
#define BROWSER_POLICY_DECISION_MAKER_H

#include "browser-config.h"
#include "browser-view.h"

#include <string>

using namespace std;

class Browser_Common_View;

class Browser_Policy_Decision_Maker : public Browser_Common_View {
public:
	Browser_Policy_Decision_Maker(Evas_Object *navi_bar, Browser_View *browser_view);
	~Browser_Policy_Decision_Maker(void);

	void init(WKPageRef page_ref);
	void deinit(void);
	void pause(void);
private:
	enum {
		policy_use,
		policy_download,
		policy_ignore
	};

	/* ewk view event callback functions */
	static void __decide_policy_for_navigation_action(
			WKPageRef page, WKFrameRef frame, WKFrameNavigationType navigationType,
			WKEventModifiers modifiers, WKEventMouseButton mouseButton,
			WKURLRequestRef request, WKFramePolicyListenerRef listener,
			WKTypeRef userData, const void* clientInfo);
	static void __decide_policy_for_response_cb(WKPageRef page, WKFrameRef frame,
			WKURLResponseRef response, WKURLRequestRef request,
			WKFramePolicyListenerRef listener, WKTypeRef user_data,
			const void *client_info);

	/* download client callback functions */
	static void __download_did_start_cb(const char *download_url, void *user_data);

	/* Elementary event callback functions */
	static void __popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __player_cb(void *data, Evas_Object *obj, void *event_info);
	static void __internet_cb(void *data, Evas_Object *obj, void *event_info);

	/* Warning : MUST free() returned char* */
	char *_convert_WKStringRef_to_cstring(WKStringRef string_ref);
	string _convert_WKStringRef_to_string(WKStringRef string_ref);
	int _decide_policy_type(WKFrameRef frame, WKStringRef content_type_ref, string &content_type);
	void _request_download(WKURLRequestRef request, WKURLResponseRef response, string& content_type);
	Eina_Bool _launch_download_app(const char *url, const char *cookies = NULL);
	string _get_extension_name_from_url(string &url);
	Eina_Bool _show_app_list_popup(void);
	const char *_get_app_name_from_pkg_name(string& pkg_name);
	Eina_Bool _handle_exscheme(void);

	WKPageRef m_wk_page_ref;
	Evas_Object *m_navi_bar;
	Evas_Object *m_list_popup;
	Evas_Object *m_app_list;
	Browser_View *m_browser_view;

	string m_url;
	string m_cookies;
	string m_default_player_pkg_name;
};
#endif /* BROWSER_POLICY_DECISION_MAKER_H */

