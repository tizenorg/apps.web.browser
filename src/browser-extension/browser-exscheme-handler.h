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

#ifndef BROWSER_EXSCHEME_HANDLER_H
#define BROWSER_EXSCHEME_HANDLER_H

#include "browser-config.h"
#include "browser-view.h"

#include <string>

using namespace std;

class Browser_Common_View;
class Browser_View;
class Browser_Exscheme_Handler : public Browser_Common_View {
public:
	Browser_Exscheme_Handler(void);
	~Browser_Exscheme_Handler(void);

	Eina_Bool init(void) {}
	void init(Browser_View *browser_view, Evas_Object *webview);

	typedef enum _call_type {
		VOICE_CALL,
		VIDEO_CALL,
		CALL_UNKNOWN
	} call_type;
private:
	static Eina_Bool _launch_daum_tv(std::string uri);

	static Eina_Bool __daum_tv_cb(Evas_Object *webview, const char *uri);
	static Eina_Bool __sms_cb(Evas_Object *webview, const char *uri);
	static Eina_Bool __mms_cb(Evas_Object *webview, const char *uri);
	static Eina_Bool __mail_to_cb(Evas_Object *webview, const char *uri);
	static Eina_Bool __rtsp_cb(Evas_Object *webview, const char *uri);

	static Browser_View *m_browser_view;
	static Evas_Object *m_confirm_popup;
	static Evas_Object *m_webview;
	static std::string m_excheme_url;
	static Browser_Exscheme_Handler m_excheme_handler;
};
#endif /* BROWSER_EXSCHEME_HANDLER_H */

