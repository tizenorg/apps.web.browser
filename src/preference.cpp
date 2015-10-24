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

#include "preference.h"

#include <app_preference.h>
#include <Eina.h>
#include <string.h>
#include <vconf.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-view.h"
#include "main-view.h"
#include "network-manager.h"
#include "url-bar.h"
#include "url-input-bar.h"
#include "webview.h"
#include "webview-list.h"
#include "search-engine-manager.h"

static const char *PREF_KEY_DESKTOP_VIEW = "DesktopView";
static const char *PREF_KEY_BOOKMARK_VIEW_TYPE = "BookmarkViewMode";

static const char *PREF_VALUE_DEFAULT_USER_HOMEPAGE = "http://www.tizen.org";
static const char *PREF_VALUE_SEARCH_ENGINE_TYPE = "SearchEngineType";

/* settings basic */
static const char *PREF_KEY_HOMEPAGE_TYPE = "HomepageType";
static const char *PREF_KEY_CURRENT_PAGE_URI = "CurrentPageUri";
static const char *PREF_KEY_DEFAULT_HOMEPAGE = "DefaultHomepage";
static const char *PREF_KEY_USER_HOMEPAGE = "UserHomepage";
static const char *PREF_VALUE_DEFAULT_PAGE = "DefaultPage";
static const char *PREF_VALUE_CURRENT_PAGE = "CurrentPage";
static const char *PREF_VALUE_USER_HOMEPAGE = "UserHomepage";
static const char *PREF_KEY_AUTO_FILL_FORMS = "AutoFillForms";

/* settings advanced */
static const char *PREF_KEY_SEARCH_AND_URL_SUGGESTIONS = "SearchAndURLSuggestions";
static const char *PREF_KEY_INDICATOR_FULLSCREEN_VIEW = "IndicatorFullscreenView";
static const char *PREF_KEY_REMEMBER_FORM_DATA = "RememberFormData";
static const char *PREF_KEY_REMEMBER_PASSWORDS = "RememberPasswords";
static const char *PREF_KEY_ACCEPT_COOKIES = "AcceptCookies";
static const char *PREF_KEY_RUN_JAVASCRIPT = "RunJavaScript";
static const char *PREF_KEY_DEFAULT_STORAGE = "DefaultStorage";
static const char *PREF_KEY_ENABLE_WEB_NOTIFICATION = "EnableWebNotificaion";
static const char *PREF_KEY_DISPLAY_IMAGES = "DisplayImages";
static const char *PREF_KEY_HIDE_URL_TOOLBAR = "HideURLToolbar";
static const char *PREF_KEY_OPEN_PAGE_IN_OVERVIEW = "OpenPageInOverView";
static const char *PREF_KEY_SHOW_SECURITY_WARNINGS = "ShowSecurityWarnings";
static const char *PREF_KEY_SHOW_CERTIFICATE_WARNINGS = "ShowCertificateWarnings";
static const char *PREF_KEY_INCOGNITO_MODE = "IncognitoMode";
static const char *PREF_KEY_INCOGNITO_MODE_DESC_ASK = "IncognitoModeAsk";

/* settings for account */
static const char *PREF_KEY_ENABLE_ACCOUNT_VERIFY = "EnableAccountVerify";

/* for fast launch */
static const char *PREF_KEY_TAB_COUNT = "TabCount";

preference_type preference_types[] = {
	{PREF_VALUE_SEARCH_ENGINE_TYPE, INT_VALUE, (int *)0},
	{PREF_KEY_DESKTOP_VIEW, BOOL_VALUE, (bool *)false},
	{PREF_KEY_BOOKMARK_VIEW_TYPE, INT_VALUE, (int *)FOLDER_VIEW_NORMAL},
	{PREF_KEY_CURRENT_PAGE_URI, STRING_VALUE, (char *)PREF_VALUE_DEFAULT_USER_HOMEPAGE},
	{PREF_KEY_DEFAULT_HOMEPAGE, STRING_VALUE, (char *)PREF_VALUE_DEFAULT_USER_HOMEPAGE},
	{PREF_KEY_USER_HOMEPAGE, STRING_VALUE, (char *)PREF_VALUE_DEFAULT_USER_HOMEPAGE},
	{PREF_KEY_SEARCH_AND_URL_SUGGESTIONS, BOOL_VALUE, (bool *)true},
	{PREF_KEY_INDICATOR_FULLSCREEN_VIEW, BOOL_VALUE, (bool *)true},
	{PREF_KEY_ACCEPT_COOKIES, BOOL_VALUE, (bool *)true},
	{PREF_KEY_RUN_JAVASCRIPT, BOOL_VALUE, (bool *)true},
	{PREF_KEY_DEFAULT_STORAGE, INT_VALUE, (int *)DEFAULT_STORAGE_TYPE_PHONE},
	{PREF_KEY_ENABLE_WEB_NOTIFICATION, INT_VALUE, (int *)WEB_NOTI_ENABLE_TYPE_ON_DEMAND},
	{PREF_KEY_DISPLAY_IMAGES, BOOL_VALUE, (bool *)true},
	{PREF_KEY_HIDE_URL_TOOLBAR, BOOL_VALUE, (bool *)true},
	{PREF_KEY_OPEN_PAGE_IN_OVERVIEW, BOOL_VALUE, (bool *)true},
	{PREF_KEY_INCOGNITO_MODE,BOOL_VALUE, (bool *)false},
	{PREF_KEY_INCOGNITO_MODE_DESC_ASK,BOOL_VALUE, (bool *)true},
	{PREF_KEY_ENABLE_ACCOUNT_VERIFY, BOOL_VALUE, (bool *)true},
	{PREF_KEY_TAB_COUNT, INT_VALUE, (int *)1},
	{PREF_KEY_AUTO_FILL_FORMS,BOOL_VALUE,(bool *)true},
	{PREF_KEY_REMEMBER_FORM_DATA,BOOL_VALUE,(bool *)true},
	{PREF_KEY_REMEMBER_PASSWORDS,BOOL_VALUE,(bool *)true},
	{PREF_KEY_SHOW_SECURITY_WARNINGS,BOOL_VALUE,(bool *)false},
	{PREF_KEY_SHOW_CERTIFICATE_WARNINGS,BOOL_VALUE,(bool *)false},
	//{PREF_KEY_HOMEPAGE_TYPE,INT_VALUE, (int *)0},
	{PREF_KEY_HOMEPAGE_TYPE,STRING_VALUE, (char *)PREF_VALUE_DEFAULT_USER_HOMEPAGE},
	{PREF_KEY_DESKTOP_VIEW,BOOL_VALUE,(bool *)false},
	{NULL, STRING_VALUE, (char *)NULL}
};

static Eina_Bool __get_default_value(const char *item)
{
	bool value = false;
	int index = 0;
	while (preference_types[index].key) {
		if(!strcmp(preference_types[index].key, item)) {
			value = (bool)&preference_types[index].value;
			return value;
		}
		index++;
	}
	return value;
}

static Eina_Bool __create_key(pref_value_type type, const char *key, void *value)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!key, EINA_FALSE, "key is NULL");

	bool exist = false;
	int ret = preference_is_existing(key, &exist);

	switch (ret) {
	case PREFERENCE_ERROR_NONE:
		if (type == INT_VALUE)
			preference_set_int(key, (int)((int *)value));
		else if (type == BOOL_VALUE)
			preference_set_boolean(key, (int)((bool *)value));
		else
			preference_set_string(key, (const char *)value);
		return EINA_TRUE;
	case PREFERENCE_ERROR_INVALID_PARAMETER:
		BROWSER_LOGE("Can not initialize [%s] value. Invalid parameter\n", key);
		return EINA_FALSE;
	case PREFERENCE_ERROR_IO_ERROR:
		BROWSER_LOGE("Can not initialize [%s] value. Internal IO error\n", key);
		return EINA_FALSE;
	default:
		return EINA_FALSE;
	}
	return EINA_TRUE;
}

static Eina_Bool __set_value(pref_value_type type, const char *key, void *value)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!key, EINA_FALSE, "key is NULL");

	int ret;
	if (type == INT_VALUE)
		ret = preference_set_int(key, (int)((int *)value));
	else if (type == BOOL_VALUE)
		ret = preference_set_boolean(key, (int)(*(bool *)value));
	else
		ret = preference_set_string(key, (const char *)value);

	switch (ret) {
	case PREFERENCE_ERROR_NONE:
		return EINA_TRUE;
	case PREFERENCE_ERROR_INVALID_PARAMETER:
		BROWSER_LOGE("Can not get [%s] value. Invalid parameter\n", key);
		return EINA_FALSE;
	case PREFERENCE_ERROR_IO_ERROR:
		BROWSER_LOGE("Can not get [%s] value. Internal IO error\n", key);
		return EINA_FALSE;
	default:
		return EINA_FALSE;
	}
	return EINA_FALSE;
}

static Eina_Bool __get_value(pref_value_type type, const char *key, void *value)
{
	RETV_MSG_IF(!key, EINA_FALSE, "key is NULL");

	int ret;
	if (type == INT_VALUE)
		ret = preference_get_int(key, (int *)value);
	else if (type == BOOL_VALUE)
		ret = preference_get_boolean(key, (bool *)value);
	else
		ret = preference_get_string(key, (char **)value);

	switch (ret) {
	case PREFERENCE_ERROR_NONE:
		return EINA_TRUE;
	case PREFERENCE_ERROR_INVALID_PARAMETER:
		BROWSER_LOGE("Can not get [%s] value. Invalid parameter\n", key);
		return EINA_FALSE;
	case PREFERENCE_ERROR_OUT_OF_MEMORY:
		BROWSER_LOGE("Can not get [%s] value. Out of Memory\n", key);
		return EINA_FALSE;
	case PREFERENCE_ERROR_NO_KEY:
		BROWSER_LOGE("Can not get [%s] value. Required key not available\n", key);
		return EINA_FALSE;
	case PREFERENCE_ERROR_IO_ERROR:
		BROWSER_LOGE("Can not get [%s] value. Internal IO error\n", key);
		return EINA_FALSE;
	default:
		return EINA_FALSE;
	}
	return EINA_FALSE;

}

preference::preference(void)
{
	BROWSER_LOGD("");
}

preference::~preference(void)
{
	BROWSER_LOGD("");

	int index = 0;
	while (preference_types[index].key) {
		_unset_listener(preference_types[index].key);
		index++;
	}
}

Eina_Bool preference::init(void)
{
	BROWSER_LOGD("");

	bool exist = false;
	int index = 0;
	while (preference_types[index].key) {
		preference_is_existing(preference_types[index].key, &exist);
		if (!exist) {
			if (!__create_key(preference_types[index].type, preference_types[index].key, preference_types[index].value))
				return EINA_FALSE;
		}
		index++;
	}

	index = 0;
	while (preference_types[index].key) {
		_set_listener(preference_types[index].key);
		index++;
	}

	return EINA_TRUE;
}

void preference::reset(void)
{
	int index = 0;
	BROWSER_LOGD("");
	while (preference_types[index].key) {
		if (preference_types[index].type == BOOL_VALUE)
			set_value(preference_types[index].type, preference_types[index].key, &preference_types[index].value);
		else
			set_value(preference_types[index].type, preference_types[index].key, preference_types[index].value);
		index++;
	}
}

Eina_Bool preference::set_value(pref_value_type type, const char *key, void *value)
{
	RETV_MSG_IF(!key, EINA_FALSE, "key is NULL");
	std::string key_str = std::string(key);
	return __set_value(type, key_str.c_str(), value);
}

Eina_Bool preference::get_value(pref_value_type type, const char *key, void *value)
{
	RETV_MSG_IF(!key, EINA_FALSE, "key is NULL");
	std::string key_str = std::string(key);
	return __get_value(type, key_str.c_str(), value);
}

Eina_Bool preference::_get_bool_value_or_default(const char* key)
{
	bool value = false;
	bool ret = get_value(BOOL_VALUE, key, (void *)(&value));
	if(ret == EINA_FALSE)
		value = __get_default_value(key);
	return value;
}

void preference::set_desktop_view_enabled(Eina_Bool enable)
{
	set_value(BOOL_VALUE, PREF_KEY_DESKTOP_VIEW, (void *)&enable);
}

Eina_Bool preference::get_desktop_view_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_DESKTOP_VIEW);
}

void preference::set_search_engine_type(int type)
{
	BROWSER_LOGD("");
	set_value(INT_VALUE, PREF_VALUE_SEARCH_ENGINE_TYPE, (void *)type);
}

int preference::get_search_engine_type(void)
{
	BROWSER_LOGD("");
	int type = 0;

	get_value(INT_VALUE, PREF_VALUE_SEARCH_ENGINE_TYPE, (void *)(&type));

	return type;
}

void preference::set_bookmark_view_type(bookmark_view_type type)
{
	BROWSER_LOGD("bookmark_view_type = %d", type);

	set_value(INT_VALUE, PREF_KEY_BOOKMARK_VIEW_TYPE, (void *)type);
}

bookmark_view_type preference::get_bookmark_view_type(void)
{
	BROWSER_LOGD("");

	bookmark_view_type view_type = FOLDER_VIEW_NORMAL;
	get_value(INT_VALUE, PREF_KEY_BOOKMARK_VIEW_TYPE, (void *)(&view_type));

	return view_type;
}

void preference::set_homepage_type(homepage_type type)
{
	BROWSER_LOGD("");
	if (type == HOMEPAGE_TYPE_CURRENT_PAGE) {
		const char *value = PREF_VALUE_CURRENT_PAGE;
		set_value(STRING_VALUE, PREF_KEY_HOMEPAGE_TYPE, (void *)value);
	} else if (type == HOMEPAGE_TYPE_DEFAULT_PAGE) {
		const char *value = PREF_VALUE_DEFAULT_PAGE;
		__set_value(STRING_VALUE, PREF_KEY_HOMEPAGE_TYPE, (void *)value);
	} else if (type == HOMEPAGE_TYPE_USER_HOMEPAGE) {
		const char *value = PREF_VALUE_USER_HOMEPAGE;
		set_value(STRING_VALUE, PREF_KEY_HOMEPAGE_TYPE, (void *)value);
	}
}

homepage_type preference::get_homepage_type(void)
{
	BROWSER_LOGD("");
	char *value = NULL;
	homepage_type type = HOMEPAGE_TYPE_DEFAULT_PAGE;

	get_value(STRING_VALUE, PREF_KEY_HOMEPAGE_TYPE, (void *)(&value));

	if ((value == NULL)) {
		BROWSER_LOGD("Failed to get PREF_KEY_HOMEPAGE_TYPE");
		return type;
	}

	if (!strcmp((char *)value, PREF_VALUE_DEFAULT_PAGE))
		type = HOMEPAGE_TYPE_DEFAULT_PAGE;
	else if (!strcmp((char *)value, PREF_VALUE_CURRENT_PAGE))
		type = HOMEPAGE_TYPE_CURRENT_PAGE;
	else //if (!strcmp((char *)value, PREF_VALUE_USER_HOMEPAGE))
		type = HOMEPAGE_TYPE_USER_HOMEPAGE;

	free((char *)value);

	return type;
}

void preference::set_current_page_uri(const char *uri)
{
	BROWSER_SECURE_LOGD("uri=[%s]", uri);
	RET_MSG_IF(!uri, "uri is NULL");

	set_value(STRING_VALUE, PREF_KEY_CURRENT_PAGE_URI, (void *)uri);
}

char *preference::get_current_page_uri(void)
{
	BROWSER_LOGD("");
	
	char *value = NULL;
	bool ret = false;
	int index = 0;
		
	ret = get_value(STRING_VALUE, PREF_KEY_CURRENT_PAGE_URI, (void *)(&value));
	BROWSER_LOGD("uri : %s", value);
	
	if(ret == EINA_FALSE)  {
		BROWSER_LOGD("");
		while (preference_types[index].key) {
			if(!strcmp(preference_types[index].key, PREF_KEY_CURRENT_PAGE_URI)){
				value = (char *)(&preference_types[index].value);
				break;
			}
			index++;
		}
	}
	
	return value;
}


void preference::set_user_homagepage(const char *uri)
{
	BROWSER_SECURE_LOGD("uri=[%s]", uri);
	RET_MSG_IF(!uri, "uri is NULL");

	set_value(STRING_VALUE, PREF_KEY_USER_HOMEPAGE, (void *)uri);
}

char *preference::get_user_homagepage(void)
{
	BROWSER_LOGD("");
	
	char *value = NULL;
	get_value(STRING_VALUE, PREF_KEY_USER_HOMEPAGE, (void *)(&value));
	return value;

}

char *preference::get_default_homepage(void)
{
	BROWSER_LOGD("");
	char *value = NULL;
	bool ret = false;
	int index = 0;
		
	ret = get_value(STRING_VALUE, PREF_KEY_DEFAULT_HOMEPAGE, (void *)(&value));
		
	if(ret == EINA_FALSE)  {
		while (preference_types[index].key) {
		if(!strcmp(preference_types[index].key, PREF_KEY_DEFAULT_HOMEPAGE)){
			 value = (char *)(&preference_types[index].value);
			break;
		 }
		 index++;
		}
	}
		
	BROWSER_SECURE_LOGD("Tizen Org : %s", value);
	return value;
	}


char *preference::get_homepage_uri(void)
{
	BROWSER_LOGD("");
	homepage_type type = get_homepage_type();
	char *current_set_homepage_uri = NULL;
	if (type == HOMEPAGE_TYPE_DEFAULT_PAGE)
		current_set_homepage_uri = get_default_homepage();
	else if (type == HOMEPAGE_TYPE_CURRENT_PAGE)
		current_set_homepage_uri = get_current_page_uri();
	else if (type == HOMEPAGE_TYPE_USER_HOMEPAGE)
		current_set_homepage_uri = get_user_homagepage();
	else
		current_set_homepage_uri = get_user_homagepage();

	return current_set_homepage_uri;
}

void preference::set_auto_fill_forms_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("enable[%d]", enable);

	set_value(BOOL_VALUE, PREF_KEY_AUTO_FILL_FORMS, (void *)&enable);
}

Eina_Bool preference::get_auto_fill_forms_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_AUTO_FILL_FORMS);
}

void preference::set_remember_form_data_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("enable[%d]", enable);

	set_value(BOOL_VALUE, PREF_KEY_REMEMBER_FORM_DATA, (void *)&enable);
}

Eina_Bool preference::get_remember_form_data_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_REMEMBER_FORM_DATA);
}

void preference::set_remember_passwords_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("enable[%d]", enable);

	set_value(BOOL_VALUE, PREF_KEY_REMEMBER_PASSWORDS, (void *)&enable);
}

Eina_Bool preference::get_remember_passwords_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_REMEMBER_PASSWORDS);
}

void preference::set_search_and_url_suggestions_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("enable[%d]", enable);

	set_value(BOOL_VALUE, PREF_KEY_SEARCH_AND_URL_SUGGESTIONS, (void *)&enable);
}

Eina_Bool preference::get_search_and_url_suggestions_enabled(void)
{
	BROWSER_LOGD("");
	bool value = false;

	get_value(BOOL_VALUE, PREF_KEY_SEARCH_AND_URL_SUGGESTIONS, (void *)(&value));

	return value;
}

void preference::set_hide_URL_toolbar_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("enable[%d]", enable);
	set_value(BOOL_VALUE, PREF_KEY_HIDE_URL_TOOLBAR, (void *)&enable);
}

Eina_Bool preference::get_hide_URL_toolbar_enabled(void)
{
	bool value = false;

	get_value(BOOL_VALUE, PREF_KEY_HIDE_URL_TOOLBAR, (void *)(&value));

	return value;
}

void preference::set_accept_cookies_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("enable[%d]", enable);

	set_value(BOOL_VALUE, PREF_KEY_ACCEPT_COOKIES, (void *)&enable);
}

Eina_Bool preference::get_accept_cookies_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_ACCEPT_COOKIES);
}

void preference::set_javascript_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("enable[%d]", enable);

	set_value(BOOL_VALUE, PREF_KEY_RUN_JAVASCRIPT, (void *)&enable);
}

Eina_Bool preference::get_javascript_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_RUN_JAVASCRIPT);
}


void preference::set_default_storage_type(default_storage_type type)
{
	BROWSER_LOGD("type[%d]", type);

	set_value(INT_VALUE, PREF_KEY_DEFAULT_STORAGE, (void *)type);
}

default_storage_type preference::get_default_storage_type(void)
{
	BROWSER_LOGD("");
	default_storage_type type = DEFAULT_STORAGE_TYPE_PHONE;

	get_value(INT_VALUE, PREF_KEY_DEFAULT_STORAGE, (void *)(&type));

	return type;
}

void preference::set_web_notification_accept_type(web_notification_enable_type type)
{
	BROWSER_LOGD("type[%d]", type);

	set_value(INT_VALUE, PREF_KEY_ENABLE_WEB_NOTIFICATION, (void *)type);
}

web_notification_enable_type preference::get_web_notification_accept_type(void)
{
	BROWSER_LOGD("");
	web_notification_enable_type type = WEB_NOTI_ENABLE_TYPE_ON_DEMAND;

	get_value(INT_VALUE, PREF_KEY_ENABLE_WEB_NOTIFICATION, (void *)(&type));

	return type;
}

void preference::set_indicator_fullscreen_view_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	set_value(BOOL_VALUE, PREF_KEY_INDICATOR_FULLSCREEN_VIEW, (void *)&enable);
}

Eina_Bool preference::get_indicator_fullscreen_view_enabled(void)
{
	BROWSER_LOGD("");
	bool value = false;

	get_value(BOOL_VALUE, PREF_KEY_INDICATOR_FULLSCREEN_VIEW, (void *)(&value));

	return value;
}

void preference::set_display_images_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");
	
	set_value(BOOL_VALUE, PREF_KEY_DISPLAY_IMAGES, (void *)&enable);
}


Eina_Bool preference::get_display_images_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_DISPLAY_IMAGES);
}
void preference::set_open_pages_in_overview_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	set_value(BOOL_VALUE, PREF_KEY_OPEN_PAGE_IN_OVERVIEW, (void *)&enable);
}

Eina_Bool preference::get_open_pages_in_overview_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_OPEN_PAGE_IN_OVERVIEW);
}

void preference::set_security_warnings_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	set_value(BOOL_VALUE, PREF_KEY_SHOW_SECURITY_WARNINGS, (void *)&enable);
}

Eina_Bool preference::get_security_warnings_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_SHOW_SECURITY_WARNINGS);
}

void preference::set_certificate_warnings_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	set_value(BOOL_VALUE, PREF_KEY_SHOW_CERTIFICATE_WARNINGS, (void *)&enable);
}

Eina_Bool preference::get_certificate_warnings_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_SHOW_CERTIFICATE_WARNINGS);
}

void preference::set_incognito_mode_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");
	set_value(BOOL_VALUE, PREF_KEY_INCOGNITO_MODE, (void *)&enable);
}

Eina_Bool preference::get_incognito_mode_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_INCOGNITO_MODE);
}

void preference::set_incognito_mode_ask_again_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");
	set_value(BOOL_VALUE, PREF_KEY_INCOGNITO_MODE_DESC_ASK, (void *)&enable);
}

Eina_Bool preference::get_incognito_mode_ask_again_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_INCOGNITO_MODE_DESC_ASK);
}

void preference::set_verify_account_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");
	set_value(BOOL_VALUE, PREF_KEY_ENABLE_ACCOUNT_VERIFY, (void *)&enable);
}

Eina_Bool preference::get_verify_account_enabled(void)
{
	BROWSER_LOGD("");
	return _get_bool_value_or_default(PREF_KEY_ENABLE_ACCOUNT_VERIFY);
}

void preference::set_tab_counts(int count)
{
	BROWSER_LOGD("count[%d]", count);
	set_value(INT_VALUE, PREF_KEY_TAB_COUNT, (void *)count);
}

int preference::get_tab_counts(void)
{
	int value = 0;

	get_value(INT_VALUE, PREF_KEY_TAB_COUNT, (void *)(&value));
	BROWSER_LOGD("value[%d]", value);

	return value;
}

Eina_Bool preference::_set_listener(const char *key)
{
	RETV_MSG_IF(!key, EINA_FALSE, "key is NULL");

	int ret = preference_set_changed_cb(key, __preference_changed_cb, this);
	switch (ret) {
	case PREFERENCE_ERROR_NONE:
		return EINA_TRUE;
	case PREFERENCE_ERROR_INVALID_PARAMETER:
		BROWSER_LOGE("Can not set [%s] callback. Invalid parameter\n", key);
		return EINA_FALSE;
	case PREFERENCE_ERROR_OUT_OF_MEMORY:
		BROWSER_LOGE("Can not set [%s] callback. Out of Memory\n", key);
		return EINA_FALSE;
	case PREFERENCE_ERROR_NO_KEY:
		BROWSER_LOGE("Can not set [%s] callback. Required key not available\n", key);
		return EINA_FALSE;
	case PREFERENCE_ERROR_IO_ERROR:
		BROWSER_LOGE("Can not set [%s] callback. Internal IO error\n", key);
		return EINA_FALSE;
	default:
		return EINA_FALSE;
	}
	return EINA_TRUE;
}

Eina_Bool preference::_unset_listener(const char *key)
{
	RETV_MSG_IF(!key, EINA_FALSE, "key is NULL");

	int ret = preference_unset_changed_cb(key);

	if (ret != PREFERENCE_ERROR_NONE)
		return EINA_FALSE;
	else
		return EINA_TRUE;

}

void preference::__preference_changed_cb(const char *key, void *data)
{
	BROWSER_LOGD("key = [%s]", key);
	RET_MSG_IF(!key, "key is NULL");

	preference *pref = (preference *)data;

	int count = m_browser->get_webview_list()->get_count();
	std::string key_str = std::string(key);
	if (!strcmp(key_str.c_str(), PREF_KEY_HIDE_URL_TOOLBAR)) {
		main_view *mv = m_browser->get_main_view();
		if (m_preference->get_hide_URL_toolbar_enabled()) // Hide url bar
			mv->set_scroll_mode(CONTENT_VIEW_SCROLL_ENABLE);
		else
			mv->set_scroll_mode(CONTENT_VIEW_SCROLL_FIXED_TOOLBAR);
		mv->resize_scroll_content();
	} else if (!strcmp(key_str.c_str(), PREF_KEY_RUN_JAVASCRIPT)) {
		Eina_Bool enabled = pref->get_javascript_enabled();
		for (int i = 0 ; i < count ; i++){
			webview_list *temp_webview_list = m_browser->get_webview_list();
			if(temp_webview_list){
				webview *temp_webview = temp_webview_list->get_webview(i);
				if(temp_webview){
					temp_webview->javascript_enabled_set(enabled);
				}
			}
		}
	} else if (!strcmp(key_str.c_str(), PREF_KEY_DISPLAY_IMAGES)) {
		Eina_Bool enabled = pref->get_display_images_enabled();
		for (int i = 0 ; i < count ; i++){
			webview_list *temp_webview_list = m_browser->get_webview_list();
			if(temp_webview_list){
				webview *temp_webview = temp_webview_list->get_webview(i);
				if(temp_webview){
					temp_webview->auto_load_images_enabled_set(enabled);
				}
			}
		}
	} else if (!strcmp(key_str.c_str(), PREF_KEY_AUTO_FILL_FORMS)) {
		for (int i = 0 ; i < count ; i++){
			webview_list *temp_webview_list = m_browser->get_webview_list();
			if(temp_webview_list){
				webview *temp_webview = temp_webview_list->get_webview(i);
				if(temp_webview){
					temp_webview->auto_fill_forms_set(m_preference->get_auto_fill_forms_enabled());
				}
			}
		}
	} else if (!strcmp(key_str.c_str(), PREF_KEY_REMEMBER_PASSWORDS)) {
		Eina_Bool enabled = pref->get_remember_passwords_enabled();
		for (int i = 0 ; i < count ; i++){
			webview_list *temp_webview_list = m_browser->get_webview_list();
			if(temp_webview_list){
				webview *temp_webview = temp_webview_list->get_webview(i);
				if(temp_webview){
					temp_webview->save_ID_and_PW_enabled_set(enabled);
				}
			}
		}
	} else if (!strcmp(key_str.c_str(), PREF_KEY_REMEMBER_FORM_DATA)) {
		Eina_Bool enabled = pref->get_remember_form_data_enabled();
		for (int i = 0 ; i < count ; i++){
			webview_list *temp_webview_list = m_browser->get_webview_list();
			if(temp_webview_list){
				webview *temp_webview = temp_webview_list->get_webview(i);
				if(temp_webview){
					temp_webview->remember_form_data_enabled_set(enabled);
				}
			}
		}
	} else if (!strcmp(key_str.c_str(), PREF_KEY_OPEN_PAGE_IN_OVERVIEW)) {
		Eina_Bool enabled = pref->get_open_pages_in_overview_enabled();
		for (int i = 0 ; i < count ; i++){
			webview_list *temp_webview_list = m_browser->get_webview_list();
			if(temp_webview_list){
				webview *temp_webview = temp_webview_list->get_webview(i);
				if(temp_webview){
					temp_webview->auto_fitting_enabled_set(enabled);
				}
			}
		}
	}
#ifdef ENABLE_INCOGNITO_WINDOW
	else if (!strcmp(key_str.c_str(), PREF_KEY_INCOGNITO_MODE)) {
		browser_view *bv = m_browser->get_browser_view();
		RET_MSG_IF(!bv, "browser_view is NULL");

		url_bar *ub = bv->get_url_bar();
		RET_MSG_IF(!ub, "url_bar is NULL");
		ub->enable_private_mode(pref->get_incognito_mode_enabled());
	}
#endif /*ENABLE_INCOGNITO_WINDOW*/
	else if (!strcmp(key_str.c_str(), PREF_KEY_ACCEPT_COOKIES)) {
		Eina_Bool enabled = pref->get_accept_cookies_enabled();
		for (int i = 0 ; i < count ; i++){
			webview_list *temp_webview_list = m_browser->get_webview_list();
			if(temp_webview_list){
				webview *temp_webview = temp_webview_list->get_webview(i);
				if(temp_webview){
					Evas_Object *temp_ewk_view = temp_webview->get_ewk_view();
					if (temp_ewk_view){
						m_browser->get_webview_list()->get_webview(i)->accept_cookie_enabled_set(temp_ewk_view, enabled);
					}
				}
			}
		}
	}
}

