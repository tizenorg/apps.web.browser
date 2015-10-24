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

#ifndef PREFERENCE_H
#define PREFERENCE_H

#include "browser-object.h"

#include <vconf.h>

typedef enum {
	INT_VALUE = 0,
	BOOL_VALUE,
	STRING_VALUE
} pref_value_type ;

typedef struct _preference_type {
	const char *key;
	pref_value_type type;
	void *value;
} preference_type;

typedef enum {
	HOMEPAGE_TYPE_DEFAULT_PAGE = 0,
	HOMEPAGE_TYPE_CURRENT_PAGE,
	HOMEPAGE_TYPE_USER_HOMEPAGE,
	HOMEPAGE_TYPE_NUM
} homepage_type;

typedef enum {
	DEFAULT_STORAGE_TYPE_PHONE = 0,
	DEFAULT_STORAGE_TYPE_MEMORY_CARD,
	DEFAULT_STORAGE_TYPE_NUM
} default_storage_type;

typedef enum {
	WEB_NOTI_ENABLE_TYPE_ALWAYS = 0,
	WEB_NOTI_ENABLE_TYPE_ON_DEMAND,
	WEB_NOTI_ENABLE_TYPE_OFF,
	WEB_NOTI_ENABLE_TYPE_NUM
} web_notification_enable_type;

typedef enum {
	FOLDER_VIEW_NORMAL
	,UNKNOWN_VIEW
} bookmark_view_type;

class preference : public browser_object {
public:
	preference(void);
	~preference(void);

	Eina_Bool init(void);
	void reset(void);

	Eina_Bool set_value(pref_value_type type, const char *key, void *value);
	Eina_Bool get_value(pref_value_type type, const char *key, void *value);

	void set_desktop_view_enabled(Eina_Bool enable);
	Eina_Bool get_desktop_view_enabled(void);

	void set_search_engine_type(int type);
	int get_search_engine_type(void);

	void set_bookmark_view_type(bookmark_view_type type);
	bookmark_view_type get_bookmark_view_type(void);

	void set_search_and_url_suggestions_enabled(Eina_Bool enable);
	Eina_Bool get_search_and_url_suggestions_enabled(void);

	/* settings */
	void set_homepage_type(homepage_type type);
	homepage_type get_homepage_type(void);

	void set_current_page_uri(const char *uri);
	/* @ return - malloced char *, it should be freed by caller */
	char *get_current_page_uri(void);
	void set_user_homagepage(const char *uri);
	/* @ return - malloced char *, it should be freed by caller */
	char *get_user_homagepage(void);
	char *get_default_homepage(void);
	/* @ return - malloced char *, it should be freed by caller */
	char *get_homepage_uri(void);

	void set_auto_fill_forms_enabled(Eina_Bool enable);
	Eina_Bool get_auto_fill_forms_enabled(void);

	void set_remember_form_data_enabled(Eina_Bool enable);
	Eina_Bool get_remember_form_data_enabled(void);

	void set_remember_passwords_enabled(Eina_Bool enable);
	Eina_Bool get_remember_passwords_enabled(void);

	void set_hide_URL_toolbar_enabled(Eina_Bool enable);
	Eina_Bool get_hide_URL_toolbar_enabled(void);

	void set_accept_cookies_enabled(Eina_Bool enable);
	Eina_Bool get_accept_cookies_enabled(void);

	void set_javascript_enabled(Eina_Bool enable);
	Eina_Bool get_javascript_enabled(void);

	void set_default_storage_type(default_storage_type type);
	default_storage_type get_default_storage_type(void);

	void set_web_notification_accept_type(web_notification_enable_type type);
	web_notification_enable_type get_web_notification_accept_type(void);

	void set_indicator_fullscreen_view_enabled(Eina_Bool enable);
	Eina_Bool get_indicator_fullscreen_view_enabled(void);

	void set_display_images_enabled(Eina_Bool enable);
	Eina_Bool get_display_images_enabled(void);

	void set_open_pages_in_overview_enabled(Eina_Bool enable);
	Eina_Bool get_open_pages_in_overview_enabled(void);

	void set_security_warnings_enabled(Eina_Bool enable);
	Eina_Bool get_security_warnings_enabled(void);

	void set_certificate_warnings_enabled(Eina_Bool enable);
	Eina_Bool get_certificate_warnings_enabled(void);

	void set_incognito_mode_enabled(Eina_Bool enable);
	Eina_Bool get_incognito_mode_enabled(void);

	void set_incognito_mode_ask_again_enabled(Eina_Bool enable);
	Eina_Bool get_incognito_mode_ask_again_enabled(void);

	void set_verify_account_enabled(Eina_Bool enable);
	Eina_Bool get_verify_account_enabled(void);

	void set_tab_counts(int enable);
	int get_tab_counts(void);

	Eina_Bool get_yandex_search_enabled(void);

private:
	Eina_Bool _get_bool_value_or_default(const char *key);
	Eina_Bool _set_listener(const char *key);
	Eina_Bool _unset_listener(const char *key);
	static void __preference_changed_cb(const char *key, void *data);
};

#endif /* PREFERENCE_H */

