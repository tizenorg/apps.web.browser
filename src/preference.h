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

#ifndef PREFERENCE_H
#define PREFERENCE_H

#include "browser-object.h"
#include <vconf.h>

typedef enum {
	HOMEPAGE_TYPE_CURRENT_PAGE = 0,
	HOMEPAGE_TYPE_EMPTY_PAGE,
	HOMEPAGE_TYPE_RECENTLY_VISITED_SITE,
	HOMEPAGE_TYPE_USER_HOMEPAGE
} homepage_type;

typedef enum {
	VIEW_LEVEL_TYPE_FIT_TO_WIDTH,
	VIEW_LEVEL_TYPE_READABLE
} view_level_type;

static const char *PREF_VALUE_TEXT_ENCODING_ISD_8859_1 = "ISO-8859-1";
static const char *PREF_VALUE_TEXT_ENCODING_UTF_8 = "UTF-8";
static const char *PREF_VALUE_TEXT_ENCODING_GBK = "GBK";
static const char *PREF_VALUE_TEXT_ENCODING_BIG5 = "Big5";
static const char *PREF_VALUE_TEXT_ENCODING_ISO_2022_JP = "ISO-2022-JP";
static const char *PREF_VALUE_TEXT_ENCODING_SHIFT_JIS = "SHIFT JIS";
static const char *PREF_VALUE_TEXT_ENCODING_EUC_JP = "EUC-JP";
static const char *PREF_VALUE_TEXT_ENCODING_EUC_KR = "EUC-KR";
static const char *PREF_VALUE_TEXT_ENCODING_AUTOMATIC = "Automatic";

typedef enum {
	TEXT_ENCODING_TYPE_ISO_8859_1 = 0,
	TEXT_ENCODING_TYPE_UTF_8,
	TEXT_ENCODING_TYPE_GBK,
	TEXT_ENCODING_TYPE_BIG5,
	TEXT_ENCODING_TYPE_ISO_2022_JP,
	TEXT_ENCODING_TYPE_SHIFT_JIS,
	TEXT_ENCODING_TYPE_EUC_JP,
	TEXT_ENCODING_TYPE_EUC_KR,
	TEXT_ENCODING_TYPE_AUTOMATIC,
	TEXT_ENCODING_TYPE_NUM
} text_encoding_type;

class preference : public browser_object {
public:
	preference(void);
	~preference(void);

	Eina_Bool init(void);
	void reset(void);
	void set_last_visited_uri(const char *uri);
	/* @ return - malloced char *, it should be freed by caller */
	char *get_last_visited_uri(void);

	void set_user_homagepage(const char *uri);
	/* @ return - malloced char *, it should be freed by caller */
	char *get_user_homagepage(void);

	/* @ return - malloced char *, it should be freed by caller */
	char *get_homepage_uri(void);

	void set_homepage_type(homepage_type type);
	homepage_type get_homepage_type(void);

	void set_desktop_view_enabled(Eina_Bool enable);
	Eina_Bool get_desktop_view_enabled(void);

	void set_view_level_type(view_level_type type);
	view_level_type get_view_level_type(void);

	void set_javascript_enabled(Eina_Bool enable);
	Eina_Bool get_javascript_enabled(void);

	void set_display_images_enabled(Eina_Bool enable);
	Eina_Bool get_display_images_enabled(void);

	void set_block_popup_enabled(Eina_Bool enable);
	Eina_Bool get_block_popup_enabled(void);

	void set_text_encoding_type(text_encoding_type encoding_type);
	text_encoding_type get_text_encoding_type_index(void);
	const char *get_text_encoding_type_str(text_encoding_type index);

	void set_security_warnings_enabled(Eina_Bool enable);
	Eina_Bool get_security_warnings_enabled(void);

	void set_accept_cookies_enabled(Eina_Bool enable);
	Eina_Bool get_accept_cookies_enabled(void);

	void set_auto_save_id_password_enabled(Eina_Bool enable);
	Eina_Bool get_auto_save_id_password_enabled(void);

	void set_auto_save_form_data_enabled(Eina_Bool enable);
	Eina_Bool get_auto_save_form_data_enabled(void);

	void set_location_enabled(Eina_Bool enable);
	Eina_Bool get_location_enabled(void);

	void set_frequent_homepage(Eina_Bool set);
	Eina_Bool get_frequent_homepage(void);

	void set_renew_homepage(Eina_Bool set);
	Eina_Bool get_renew_homepage(void);

	void set_multiwindow_handle_rate(int rate);
	int get_multiwindow_handle_rate(void);

	void set_search_engine_type(int type);
	int get_search_engine_type(void);
#if defined(MDM)
	/* MDM related API, the status should be mdm_status_t value. */
	void set_mdm_auto_fill_status(int status);
	int get_mdm_auto_fill_status(void);
	void set_mdm_cookies_status(int status);
	int get_mdm_cookies_status(void);
	void set_mdm_fraud_warning_status(int status);
	int get_mdm_fraud_warning_status(void);
	void set_mdm_javascript_status(int status);
	int get_mdm_javascript_status(void);
	void set_mdm_popup_status(int status);
	int get_mdm_popup_status(void);
#endif

#if !defined(TIZEN_PUBLIC)
	void set_reader_enabled(Eina_Bool enable);
	Eina_Bool get_reader_enabled(void);

	void set_reader_font_size(int size);
	int get_reader_font_size(void);

	void set_plugins_enabled(Eina_Bool enable);
	Eina_Bool get_plugins_enabled(void);
	Eina_Bool get_tilt_zoom_enabled(void);
#if defined(TEST_CODE)
	void set_demo_mode_enabled(Eina_Bool enable);
	Eina_Bool get_demo_mode_enabled(void);
	void set_recording_surface_enabled(Eina_Bool enable);
	Eina_Bool get_recording_surface_enabled(void);
	void set_remote_web_inspector_enabled(Eina_Bool enable);
	Eina_Bool get_remote_web_inspector_enabled(void);
	void set_accelerated_composition_enabled(Eina_Bool enable);
	Eina_Bool get_accelerated_composition_enabled(void);
	void set_external_video_player_enabled(Eina_Bool enable);
	Eina_Bool get_external_video_player_enabled(void);
	void set_composited_render_layer_borders_enabled(Eina_Bool enable);
	Eina_Bool get_composited_render_layer_borders_enabled(void);
	void set_memory_usage_profiling_enabled(Eina_Bool enable);
	Eina_Bool get_memory_usage_profiling_enabled(void);
	void set_zoom_button_enabled(Eina_Bool enable);
	Eina_Bool get_zoom_button_enabled(void);
	void set_low_memory_mode_enabled(Eina_Bool enable);
	Eina_Bool get_low_memory_mode_enabled(void);
#endif /* TEST_CODE */
#endif /* TIZEN_PUBLIC */
private:
	Eina_Bool _set_listener(const char *key);
	Eina_Bool _unset_listener(const char *key);
	static void __preference_changed_cb(const char *key, void *data);
	static void __vconf_changed_cb(keynode_t *keynode, void *data);

};

#endif /* PREFERENCE_H */

