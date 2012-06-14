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

