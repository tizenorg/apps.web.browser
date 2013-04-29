/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
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

#include "browser-object.h"

#include <string>
#include "browser.h"
#include "browser-dlog.h"
#include "preference.h"
#include "webview.h"

static const char *support_schemes[] = {http_scheme, https_scheme, file_scheme, rtsp_scheme, tizenstore_scheme, mailto_scheme, tel_scheme,
				telto_scheme, vtel_scheme, wtai_wp_mc_scheme, wtai_wp_sd_scheme, sms_scheme, smsto_scheme, mms_scheme, mmsto_scheme,
				wtai_scheme, wtai_wp_ap_scheme, blank_page, tizen_service_scheme, NULL};

browser *browser_object::m_browser;
Evas_Object *browser_object::m_window;
preference *browser_object::m_preference;
webview_context *browser_object::m_webview_context;
int browser_object::m_browsing_count;

browser_object::browser_object(void)
{
	//BROWSER_LOGD("");
}

browser_object::~browser_object(void)
{
	//BROWSER_LOGD("");
}

Eina_Bool browser_object::is_supported_scheme(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	int index = 0;
	while (support_schemes[index]) {
		if (!strncmp(uri, support_schemes[index], strlen(support_schemes[index])))
			return EINA_TRUE;
		index++;
	}

	return EINA_FALSE;
}
