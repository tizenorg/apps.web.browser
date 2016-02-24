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

#ifndef BROWSER_OBJECT_H
#define BROWSER_OBJECT_H

#include <Eina.h>
#include <Elementary.h>
#include <Evas.h>

#define ftp_scheme	"ftp://"
#define http_scheme	"http://"
#define https_scheme	"https://"
#define file_scheme	"file://"
#define rtsp_scheme	"rtsp://"
#define tizenstore_scheme	"tizenstore://"
#define mailto_scheme	"mailto:"
#define tel_scheme	"tel:"
#define telto_scheme	"telto:"
#define callto_scheme	"callto:"
#define vtel_scheme	"vtel:"
#define wtai_wp_mc_scheme	"wtai://wp/mc;"
#define wtai_wp_sd_scheme	"wtai://wp/sd;"
#define wtai_wp_ap_scheme	"wtai://wp/ap;"
#define wtai_scheme	"wtai://"
#define sms_scheme	"sms:"
#define smsto_scheme	"smsto:"
#define data_scheme	"data:"
#define data_scheme_png_base64 "data:image/png;base64"
#define data_scheme_jpeg_base64 "data:image/jpeg;base64"
#define data_scheme_jpg_base64 "data:image/jpg;base64"
#define data_scheme_gif_base64 "data:image/gif;base64"

#define about_debug "about:debug"

#define intent_scheme   "intent:"
#define tizen_service_scheme   "tizen-service:"

#define sec_browser_app BROWSER_APP_NAME
#define sec_vt_app	"org.tizen.vtmain"
#define sec_streaming_player	"org.tizen.video-player"
#define sec_music_player	"org.tizen.sound-player"
#define tizen_store	"beu6y5fgnl.TizenStore"
#define sec_email_app	"email-composer-efl"
#define sec_message_app	"org.tizen.message"
#define sec_bluetooth_app	"ug-bluetooth-efl"
#define sec_snote_ug	"smemo-efl"
#define sec_nfc_app	"org.tizen.nfc-share-service"
#define sec_contact_app	"contacts-details-efl"

#define uri_entry_style "DEFAULT='font_size=34 color=#000000 ellipsis=1'"
#define uri_entry_style_color "color=#000000 ellipsis=1"

#define browser_bin_dir "/usr/apps/"sec_browser_app
#define browser_data_dir "/opt/usr/apps/"sec_browser_app"/data"
#define browser_edj_dir browser_bin_dir"/res/edje"
#define browser_img_dir browser_bin_dir"/res/images"
#define browser_locale_dir browser_bin_dir"/res/locale"
#define browser_res_dir browser_bin_dir"/res"

#define browser_view_edj_path browser_edj_dir"/browser-view.edj"

#define default_device_storage_path "/opt/usr/media/Downloads/"
#define default_sd_card_storage_path "/opt/storage/sdcard/Downloads/"

#define default_snapshot_local_path browser_data_dir"/snapshots"

#define efl_scale	(elm_config_scale_get() / elm_app_base_scale_get())

#define blank_page	"about:blank"

#define webapp_icon_url_list_reader_js_path browser_res_dir"/js/reader_webapp_icon_urls.js"
#define webapp_icon_size_list_reader_js_path browser_res_dir"/js/reader_webapp_icon_sizes.js"
#define webapp_icon_capability_reader_js_path browser_res_dir"/js/reader_webapp_icon_capability.js"

#define PDF_FILE_PATH	browser_data_dir"/print.pdf"

#define BROWSER_MAX_TAG_COUNT	4
#define TITLE_INPUT_ENTRY_MAX_COUNT 4096
#define URI_INPUT_ENTRY_MAX_COUNT 4096
#define USER_HOMEPAGE_ENTRY_MAX_COUNT 4096
#define AUTO_FILL_FORM_ENTRY_MAX_COUNT 1024
#define FIND_ON_PAGE_MAX_TEXT 1000

#define cms_mime_type "application/vnd.ms-playready.initiator+xml"

#define APP_IN_APP_X	(50 * efl_scale)
#define APP_IN_APP_Y	(100 * efl_scale)
#define APP_IN_APP_W	(625 * efl_scale)
#define APP_IN_APP_H	(612 * efl_scale)
#define APP_IN_APP_MIN_W	(560 * efl_scale)
#define APP_IN_APP_MIN_H	(320 * efl_scale)

#define URI_VISIBLE_LENGTH 128

#define VIEWER_ATTACH_LIST_ITEM_HEIGHT 116

#define BROWSER_PACKAGE_NAME	"browser"
#define BROWSER_DOMAIN_NAME BROWSER_PACKAGE_NAME
#define SYSTEM_DOMAIN_NAME "sys_string"

#define MAX_HISTORY_COUNT_TODAY 10

/* Tab manager */
#define TAB_MANAGER_SNAPSHOT_HEIGHT 156
#define TAB_MANAGER_SNAPSHOT_WIDTH 272
#define TAB_MANAGER_SNAPSHOT_LEFT_PADDING 8
#define TAB_MANAGER_SNAPSHOT_RIGHT_PADDING 8
#define TAB_MANAGER_SNAPSHOT_UPPER_PADDING 6
#define TAB_MANAGER_SNAPSHOT_BOTTOM_PADDING 14

#define TAB_MANAGER_TITLE_HEIGHT 52

#define TAB_MANAGER_SNAPSHOT_ITEM_WIDTH (TAB_MANAGER_SNAPSHOT_LEFT_PADDING + TAB_MANAGER_SNAPSHOT_WIDTH + TAB_MANAGER_SNAPSHOT_RIGHT_PADDING)
#define TAB_MANAGER_SNAPSHOT_ITEM_HEIGHT (TAB_MANAGER_SNAPSHOT_UPPER_PADDING + TAB_MANAGER_TITLE_HEIGHT + TAB_MANAGER_SNAPSHOT_HEIGHT + TAB_MANAGER_SNAPSHOT_BOTTOM_PADDING)

#define TAB_MANAGER_ITEM_UPPER_PADDING 24
#define TAB_MANAGER_ITEM_RIGHT_PADDING 42
#define TAB_MANAGER_ITEM_HEIGHT 122 * efl_scale
#define TAB_MANAGER_ITEM_WIDTH 480 * efl_scale
#define TAB_MANAGER_LANDSCAPE_ITEM_WIDTH 420 * efl_scale
#define TAB_MANAGER_LANDSCAPE_ITEM_HEIGHT 132 * efl_scale
#define URI_BAR_HEIGHT	108
#define NAVIFRAME_TITLE_H_INC 96
#define NAVIFRAME_LANDSCAPE_TITLE_H_INC 91
#define URL_BAR_HEIGHT	98
/*General*/
#define LONG_PRESS_LIST_POPUP_MIN_HEIGHT (288 * efl_scale)

typedef enum {
	webapp_icon_reader_excute_mode_urls_get = 0,
	webapp_icon_reader_excute_mode_sizes_get,
	webapp_icon_reader_excute_mode_capability_get,

	webapp_icon_reader_excute_mode_end,
} webapp_icon_reader_excute_mode;

typedef enum {
	popup_with_only_top = 0,
	popup_with_only_bottom,
	popup_with_top_bottom,
} popup_shape;

typedef enum {
	browser_view_mode_normal = 0,

	browser_view_mode_end,
} browser_view_mode;


class browser;
class preference;
class webview_context;

class browser_object {
public:
	browser_object(void);
	~browser_object(void);

	Eina_Bool is_supported_scheme(const char *uri);
	void set_screen_bg(Evas_Object *screen_bg);
protected:
	static browser *m_browser;
	static Evas_Object *m_window;
	static preference *m_preference;
	static Evas_Object *m_screen_bg;
};

#endif /* BROWSER_OBJECT_H */

