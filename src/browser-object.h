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

#ifndef BROWSER_OBJECT_H
#define BROWSER_OBJECT_H

#include <Eina.h>
#include <Elementary.h>
#include <Evas.h>

#define http_scheme	"http://"
#define https_scheme	"https://"
#define file_scheme	"file://"
#define rtsp_scheme	"rtsp://"
#define tizenstore_scheme	"tizenstore://"
#define mailto_scheme	"mailto:"
#define tel_scheme	"tel:"
#define telto_scheme	"telto:"
#define vtel_scheme	"vtel:"
#define wtai_wp_mc_scheme	"wtai://wp/mc;"
#define wtai_wp_sd_scheme	"wtai://wp/sd;"
#define wtai_wp_ap_scheme	"wtai://wp/ap;"
#define wtai_scheme	"wtai://"
#define sms_scheme	"sms:"
#define smsto_scheme	"smsto:"
#define mms_scheme	"mms:"
#define mmsto_scheme	"mmsto:"
#define intent_scheme   "intent:"
#define tizen_service_scheme   "tizen-service:"

#define sec_browser_app	"org.tizen.browser"
#define sec_vt_app	"org.tizen.vtmain"
#define sec_streaming_player	"org.tizen.video-player"
#define sec_music_player	"org.tizen.sound-player"
#define tizen_store	"beu6y5fgnl.TizenStore"
#define sec_email_app	"email-composer-efl"
#define sec_message_app	"org.tizen.message"
#define sec_bluetooth_app	"ug-bluetooth-efl"
#define sec_snote_ug	"smemo-efl"
#define sec_contact_app	"contacts-details-efl"

#define uri_entry_style "DEFAULT='font_size=34 color=#808080 ellipsis=1'"

#define browser_bin_dir "/usr/apps/org.tizen.browser"
#define browser_data_dir "/opt/usr/apps/org.tizen.browser/data"
#define browser_edj_dir browser_bin_dir"/res/edje"
#define browser_img_dir browser_bin_dir"/res/images"
#define browser_locale_dir browser_bin_dir"/res/locale"
#define browser_res_dir browser_bin_dir"/res"
#define browser_template_dir browser_bin_dir"/res/tempalte"

#define efl_scale	elm_config_scale_get()

#define google_query_uri	"http://www.google.com/m/search?q="
#define yahoo_query_uri	"http://search.yahoo.com/search?ei=UTF-8&fr=crmas&p="
#define bing_query_uri_prefix	"http://m.bing.com/search/?q="
#define bing_query_uri_postfix	"&PC=SMSM&FORM=MBDPSB&mid=10006"

#define blank_page	"about:blank"

#define BROWSER_MAX_TAG_COUNT	4

class browser;
class preference;
class webview_context;

class browser_object {
public:
	browser_object(void);
	~browser_object(void);

	Eina_Bool is_supported_scheme(const char *uri);
protected:
	static browser *m_browser;
	static Evas_Object *m_window;
	static preference *m_preference;
	static webview_context *m_webview_context;
	static int m_browsing_count;
};

#endif /* BROWSER_OBJECT_H */

