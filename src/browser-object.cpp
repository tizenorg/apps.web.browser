/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 */

#include "browser-object.h"

#include <string>
#include <string.h>

#include "browser.h"
#include "browser-dlog.h"
#include "preference.h"
#include "webview.h"

static const char *support_schemes[] = {ftp_scheme, http_scheme, https_scheme, file_scheme, rtsp_scheme, tizenstore_scheme, mailto_scheme, tel_scheme,
				telto_scheme, vtel_scheme, wtai_wp_mc_scheme, wtai_wp_sd_scheme, sms_scheme, smsto_scheme,
				wtai_scheme, wtai_wp_ap_scheme, blank_page, tizen_service_scheme,
				NULL};

browser *browser_object::m_browser;
Evas_Object *browser_object::m_window;
preference *browser_object::m_preference;
Evas_Object *browser_object::m_screen_bg;

browser_object::browser_object(void)
{
}

browser_object::~browser_object(void)
{
}

void browser_object::set_screen_bg(Evas_Object *screen_bg)
{
	m_screen_bg = screen_bg;
}

Eina_Bool browser_object::is_supported_scheme(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_SECURE_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	int index = 0;
	while (support_schemes[index]) {
		if (!strncasecmp(uri, support_schemes[index], strlen(support_schemes[index])))
			return EINA_TRUE;
		index++;
	}

	return EINA_FALSE;
}
