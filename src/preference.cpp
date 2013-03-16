/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
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

#include "preference.h"

#include <Eina.h>
#if defined(MDM)
#include <mdm.h>
#endif
#include <vconf.h>
#include "app.h" /* for preference CAPI */
#include "browser.h"
#include "browser-dlog.h"
#include "webview.h"
#include "webview-list.h"

static const char *PREF_KEY_LAST_VISITED_URL = "LastVisitedUrl";
static const char *PREF_KEY_HOMEPAGE_TYPE = "HomepageType";
static const char *PREF_KEY_USER_HOMEPAGE = "UserHomepage";
static const char *PREF_KEY_SEARCH_URL = "SearchUrl";
static const char *PREF_KEY_DEFAULT_VIEW_LEVEL = "DefaultViewLevel";
static const char *PREF_KEY_RUN_JAVASCRIPT = "RunJavaScript";
static const char *PREF_KEY_DISPLAY_IMAGES = "DisplayImages";
static const char *PREF_KEY_BLOCK_POPUP = "BlockPopup";
static const char *PREF_KEY_TEXT_ENCODING = "TextEncoding";
static const char *PREF_KEY_SHOW_SECURITY_WARNINGS = "ShowSecurityWarnings";
static const char *PREF_KEY_ACCEPT_COOKIES = "AcceptCookies";
static const char *PREF_KEY_ENABLE_LOCATION = "EnableLocation";
static const char *PREF_KEY_MULTIWINDOW_HANDLE_RATE = "MultiWindowHandleRate";
static const char *PREF_KEY_AUTO_SAVE_ID_PASSWORD = "AutoSaveIDPassword";
static const char *PREF_KEY_AUTO_SAVE_FORM_DATA = "AutoSaveFormData";
#if defined(MDM)
static const char *PREF_KEY_MDM_AUTO_FILL = "MDMAutoFill";
static const char *PREF_KEY_MDM_COOKIES = "MDMCookies";
static const char *PREF_KEY_MDM_FRAUD_WARNING = "MDMFraudWarning";
static const char *PREF_KEY_MDM_JAVASCRIPT = "MDMJavaScript";
static const char *PREF_KEY_MDM_POPUP = "MDMPopup";
#endif
static const char *PREF_KEY_SET_FREQUENT_HOMEPAGE = "SetFrequentHomepage";
static const char *PREF_KEY_SET_RENEW_HOMEPAGE = "SetRenewHomepage";
static const char *PREF_KEY_DESKTOP_VIEW = "DesktopView";
static const char *PREF_KEY_RUN_PLUGINS = "RunPlugins";
#if !defined(TIZEN_PUBLIC)
static const char *PREF_KEY_RUN_READER = "RunReader";
static const char *PREF_KEY_READER_FONT_SIZE = "ReaderFontSize";
#if defined(TEST_CODE)
static const char *PREF_KEY_DEMO_MODE = "DemoMode";
static const char *PREF_KEY_RECORDING_SURFACE = "RecordingSurface";
static const char *PREF_KEY_REMOTE_WEB_INSPECTOR = "RemoteWebInspector";
static const char *PREF_KEY_ACCELERATED_COMPOSITION = "AcceleratedComposition";
static const char *PREF_KEY_EXTERNAL_VIDEO_PLAYER = "ExternalVideoPlayer";
static const char *PREF_KEY_COMPOSITED_RENDER_LAYER_BORDERS = "CompositedRenderLayerBorders";
static const char *PREF_KEY_MEMORY_USAGE_PROFILING = "MemoryUsageProfiling";
static const char *PREF_KEY_ZOOM_BUTTON = "Zoombutton";
static const char *PREF_KEY_LOW_MEMORY_MODE = "LowMemoryMode";
#endif
static const char *PREF_KEY_BRIGHTNESS_LEVEL = "BrowserBrightnessLevel";
static const char *PREF_KEY_POWER_SAVE_LEVEL = "BrightnessPowerSaveLevel";
#endif

static const char *PREF_VALUE_DEFAULT_USER_HOMEPAGE = "www.tizen.org";
static const char *PREF_VALUE_MOST_VISITED_SITES = "MOST_VISITED_SITES";
static const char *PREF_VALUE_RECENTLY_VISITED_SITE = "RECENTLY_VISITED_SITE";
static const char *PREF_VALUE_USER_HOMEPAGE = "USER_HOMEPAGE";
static const char *PREF_VALUE_EMPTY_PAGE = "EMPTY_PAGE";
static const char *PREF_VALUE_FIT_TO_WIDTH = "FIT_TO_WIDTH";
static const char *PREF_VALUE_READABLE = "READABLE";
static const char *PREF_VALUE_SEARCH_URL = "http://search.yahoo.com/search?p=";
static const char *PREF_VALUE_SEARCH_ENGINE_TYPE = "SearchEngineType";

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

preference_type preference_types[] = {
	{PREF_KEY_LAST_VISITED_URL, STRING_VALUE, (char *)""},
	{PREF_KEY_SEARCH_URL, STRING_VALUE, (char *)PREF_VALUE_SEARCH_URL},
	{PREF_KEY_DEFAULT_VIEW_LEVEL, STRING_VALUE, (char *)PREF_VALUE_FIT_TO_WIDTH}, /* FIT_TO_WIDTH, READABLE */
	{PREF_KEY_RUN_JAVASCRIPT, BOOL_VALUE, (bool *)true},
	{PREF_KEY_DISPLAY_IMAGES, BOOL_VALUE, (bool *)true},
	{PREF_KEY_BLOCK_POPUP, BOOL_VALUE, (bool *)true},
	{PREF_KEY_TEXT_ENCODING, INT_VALUE, (int *)TEXT_ENCODING_TYPE_UTF_8},
	{PREF_KEY_SHOW_SECURITY_WARNINGS, BOOL_VALUE, (bool *)true},
	{PREF_KEY_ACCEPT_COOKIES, BOOL_VALUE, (bool *)true},
	{PREF_KEY_AUTO_SAVE_ID_PASSWORD, BOOL_VALUE, (bool *)false},
	{PREF_KEY_AUTO_SAVE_FORM_DATA, BOOL_VALUE, (bool *)false},
	{PREF_KEY_ENABLE_LOCATION, BOOL_VALUE, (bool *)true},
	{PREF_KEY_MULTIWINDOW_HANDLE_RATE, INT_VALUE, (int *)10},
#if defined(MDM)
	{PREF_KEY_MDM_AUTO_FILL, INT_VALUE, (int *)MDM_ALLOWED},
	{PREF_KEY_MDM_COOKIES, INT_VALUE, (int *)MDM_ALLOWED},
	{PREF_KEY_MDM_FRAUD_WARNING, INT_VALUE, (int *)MDM_ALLOWED},
	{PREF_KEY_MDM_JAVASCRIPT, INT_VALUE, (int *)MDM_ALLOWED},
	{PREF_KEY_MDM_POPUP, INT_VALUE, (int *)MDM_ALLOWED},
#endif
	{PREF_KEY_USER_HOMEPAGE, STRING_VALUE, (char *)PREF_VALUE_DEFAULT_USER_HOMEPAGE},
	{PREF_KEY_SET_FREQUENT_HOMEPAGE, BOOL_VALUE, (bool *)false},
	{PREF_KEY_SET_RENEW_HOMEPAGE, BOOL_VALUE, (bool *)false},
	{PREF_KEY_DESKTOP_VIEW, BOOL_VALUE, (bool *)false},
	{PREF_KEY_RUN_PLUGINS, BOOL_VALUE, (bool *)false},
#if !defined(TIZEN_PUBLIC)
	{PREF_KEY_RUN_READER, BOOL_VALUE, (bool *)true},
	{PREF_KEY_READER_FONT_SIZE, INT_VALUE, (int *)16},
	{PREF_KEY_HOMEPAGE_TYPE, STRING_VALUE, (char *)PREF_VALUE_USER_HOMEPAGE}, /* MOST_VISITED_SITES, RECENTLY_VISITED_SITE, USER_HOMEPAGE, EMPTY_PAGE */
	{PREF_KEY_LOW_MEMORY_MODE, BOOL_VALUE, (bool *)false},
#if defined(TEST_CODE)
	{PREF_KEY_DEMO_MODE, BOOL_VALUE, (bool *)false},
	{PREF_KEY_RECORDING_SURFACE, BOOL_VALUE, (bool *)true},
	{PREF_KEY_REMOTE_WEB_INSPECTOR, BOOL_VALUE, (bool *)false},
	{PREF_KEY_ACCELERATED_COMPOSITION, BOOL_VALUE, (bool *)true},
	{PREF_KEY_EXTERNAL_VIDEO_PLAYER, BOOL_VALUE, (bool *)false},
	{PREF_KEY_COMPOSITED_RENDER_LAYER_BORDERS, BOOL_VALUE, (bool *)false},
	{PREF_KEY_MEMORY_USAGE_PROFILING, BOOL_VALUE, (bool *)false},
	{PREF_KEY_ZOOM_BUTTON, BOOL_VALUE, (bool *)false},
#endif
	{PREF_KEY_BRIGHTNESS_LEVEL, INT_VALUE, (int *)(-1)},
	{PREF_KEY_POWER_SAVE_LEVEL, INT_VALUE, (int *)0},
#else
	{PREF_KEY_HOMEPAGE_TYPE, STRING_VALUE, (char *)PREF_VALUE_USER_HOMEPAGE}, /* MOST_VISITED_SITES, RECENTLY_VISITED_SITE, USER_HOMEPAGE, EMPTY_PAGE */
#endif
	{NULL, STRING_VALUE, (char *)NULL}
};

static Eina_Bool __create_key(pref_value_type type, const char *key, void *value)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(key, EINA_FALSE);

	bool exist = false;
	int ret = preference_is_existing(key, &exist);

	switch (ret) {
	case PREFERENCE_ERROR_NONE:
		if (!exist) {
			if (type == INT_VALUE)
				preference_set_int(key, (int)((int *)value));
			else if (type == BOOL_VALUE)
				preference_set_boolean(key, (int)((bool *)value));
			else
				preference_set_string(key, (const char *)value);
		}
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
	EINA_SAFETY_ON_NULL_RETURN_VAL(key, EINA_FALSE);

	int ret;
	if (type == INT_VALUE)
		ret = preference_set_int(key, (int)((int *)value));
	else if (type == BOOL_VALUE)
		ret = preference_set_boolean(key, (int)((bool *)value));
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
	EINA_SAFETY_ON_NULL_RETURN_VAL(key, EINA_FALSE);

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
#if defined(TEST_CODE)
	set_remote_web_inspector_enabled(EINA_FALSE);
#endif
}

Eina_Bool preference::init(void)
{
	BROWSER_LOGD("");

	bool exist = false;
	/* If the first key is existing, assume that the pref keys are already created all.
	    So, skip checking code for reducing the launching time. */
	int ret = preference_is_existing(preference_types[0].key, &exist);
	if (!exist) {
		int index = 0;
		while (preference_types[index].key) {
			if (!__create_key(preference_types[index].type, preference_types[index].key, preference_types[index].value))
				return EINA_FALSE;
			index++;
		}
	}

	int index = 0;
	while (preference_types[index].key) {
		_set_listener(preference_types[index].key);
		index++;
	}

	if (vconf_notify_key_changed(VCONFKEY_SETAPPL_MOTION_ACTIVATION, __vconf_changed_cb, this) < 0)
		BROWSER_LOGE("VCONFKEY_SETAPPL_MOTION_ACTIVATION vconf_notify_key_changed failed");
	if (vconf_notify_key_changed(VCONFKEY_SETAPPL_USE_TILT, __vconf_changed_cb, this) < 0)
		BROWSER_LOGE("VCONFKEY_SETAPPL_USE_TILT vconf_notify_key_changed failed");

	return EINA_TRUE;
}

Eina_Bool preference::get_tilt_zoom_enabled(void)
{
	int motion_enabled = 0;
	vconf_get_bool(VCONFKEY_SETAPPL_MOTION_ACTIVATION, &motion_enabled);
	if (motion_enabled) {
		int tilt_enabled = 0;
		vconf_get_bool(VCONFKEY_SETAPPL_USE_TILT, &tilt_enabled);
		BROWSER_LOGD("******* motion_enabled=[%d], tilt_enabled=[%d]", motion_enabled, tilt_enabled);
		if (tilt_enabled)
			return EINA_TRUE;
	}

	return EINA_FALSE;
}

void preference::__vconf_changed_cb(keynode_t *keynode, void *data)
{
	preference *pref = (preference *)data;
	Eina_Bool enabled = pref->get_tilt_zoom_enabled();
	int count = m_browser->get_webview_list()->get_count();
	for (int i = 0 ; i < count ; i++)
		m_browser->get_webview_list()->get_webview(i)->motion_enabled_set(enabled);
}

void preference::__preference_changed_cb(const char *key, void *data)
{
	BROWSER_LOGD("key = [%s]", key);
	EINA_SAFETY_ON_NULL_RETURN(key);
	preference *pref = (preference *)data;

	int count = m_browser->get_webview_list()->get_count();
	if (!strcmp(key, PREF_KEY_DEFAULT_VIEW_LEVEL)) {
		for (int i = 0 ; i < count ; i++) {
			if (pref->get_view_level_type() == VIEW_LEVEL_TYPE_FIT_TO_WIDTH)
				m_browser->get_webview_list()->get_webview(i)->auto_fitting_enabled_set(EINA_TRUE);
			else
				m_browser->get_webview_list()->get_webview(i)->auto_fitting_enabled_set(EINA_FALSE);
		}
	} else if (!strcmp(key, PREF_KEY_RUN_JAVASCRIPT)) {
		for (int i = 0 ; i < count ; i++)
			m_browser->get_webview_list()->get_webview(i)->javascript_enabled_set(pref->get_javascript_enabled());
	} else if (!strcmp(key, PREF_KEY_DISPLAY_IMAGES)) {
		for (int i = 0 ; i < count ; i++)
			m_browser->get_webview_list()->get_webview(i)->auto_load_images_enabled_set(pref->get_display_images_enabled());
	} else if (!strcmp(key, PREF_KEY_BLOCK_POPUP)) {
		for (int i = 0 ; i < count ; i++)
			m_browser->get_webview_list()->get_webview(i)->scripts_window_open_enabled_set(!(pref->get_block_popup_enabled()));
	} else if (!strcmp(key, PREF_KEY_TEXT_ENCODING)) {
		for (int i = 0 ; i < count ; i++)
			m_browser->get_webview_list()->get_webview(i)->text_encoding_type_set(pref->get_text_encoding_type_str(pref->get_text_encoding_type_index()));
	} else if (!strcmp(key, PREF_KEY_AUTO_SAVE_ID_PASSWORD)) {
		for (int i = 0 ; i < count ; i++)
			m_browser->get_webview_list()->get_webview(i)->save_ID_and_PW_enabled_set(pref->get_auto_save_id_password_enabled());
	} else if (!strcmp(key, PREF_KEY_RUN_PLUGINS)) {
		for (int i = 0 ; i < count ; i++)
			m_browser->get_webview_list()->get_webview(i)->plugin_enabled_set(pref->get_plugins_enabled());
	} else if (!strcmp(key, PREF_KEY_ACCEPT_COOKIES)) {
		m_webview_context->accept_cookie_enabled_set(pref->get_accept_cookies_enabled());
	}
#if defined(TEST_CODE)
	else if (!strcmp(key, PREF_KEY_RECORDING_SURFACE)) {
		for (int i = 0 ; i < count ; i++)
			m_browser->get_webview_list()->get_webview(i)->recording_surface_enabled_set(pref->get_recording_surface_enabled());
	} else if (!strcmp(key, PREF_KEY_COMPOSITED_RENDER_LAYER_BORDERS)) {
		for (int i = 0 ; i < count ; i++)
			m_browser->get_webview_list()->get_webview(i)->layer_borders_enabled_set(pref->get_composited_render_layer_borders_enabled());
	} else if (!strcmp(key, PREF_KEY_MEMORY_USAGE_PROFILING)) {
		m_webview_context->memory_sampler_enabled_set(pref->get_memory_usage_profiling_enabled());
	} else if (!strcmp(key, PREF_KEY_RUN_PLUGINS)) {
		for (int i = 0 ; i < count ; i++)
			m_browser->get_webview_list()->get_webview(i)->plugin_enabled_set(pref->get_plugins_enabled());
	} else if (!strcmp(key, PREF_KEY_REMOTE_WEB_INSPECTOR)) {
		for (int i = 0 ; i < count ; i++)
			m_browser->get_webview_list()->get_webview(i)->inspector_server_enabled_set(pref->get_remote_web_inspector_enabled());
	}
#endif /* TEST_CODE */
}

Eina_Bool preference::_unset_listener(const char *key)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(key, EINA_FALSE);

	preference_unset_changed_cb(key);

	return EINA_TRUE;
}

Eina_Bool preference::_set_listener(const char *key)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(key, EINA_FALSE);

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

void preference::reset(void)
{
	BROWSER_LOGD("");
	int index = 0;
	while (preference_types[index].key) {
		__set_value(preference_types[index].type, preference_types[index].key, preference_types[index].value);

		index++;
	}
}

void preference::set_frequent_homepage(Eina_Bool set)
{
	__set_value(BOOL_VALUE, PREF_KEY_SET_FREQUENT_HOMEPAGE, (void *)set);
}

Eina_Bool preference::get_frequent_homepage(void)
{
	BROWSER_LOGD("");
	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_SET_FREQUENT_HOMEPAGE, (void *)(&value));

	return value;
}

void preference::set_renew_homepage(Eina_Bool set)
{
	__set_value(BOOL_VALUE, PREF_KEY_SET_RENEW_HOMEPAGE, (void *)set);
}

Eina_Bool preference::get_renew_homepage(void)
{
	BROWSER_LOGD("");
	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_SET_RENEW_HOMEPAGE, (void *)(&value));

	return value;
}

void preference::set_desktop_view_enabled(Eina_Bool enable)
{
	__set_value(BOOL_VALUE, PREF_KEY_DESKTOP_VIEW, (void *)enable);
}

Eina_Bool preference::get_desktop_view_enabled(void)
{
	BROWSER_LOGD("");
	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_DESKTOP_VIEW, (void *)(&value));

	return value;
}

void preference::set_view_level_type(view_level_type type)
{
	if (type == VIEW_LEVEL_TYPE_FIT_TO_WIDTH) {
		const char *value = PREF_VALUE_FIT_TO_WIDTH;
		__set_value(STRING_VALUE, PREF_KEY_DEFAULT_VIEW_LEVEL, (void *)value);
	} else {
		const char *value = PREF_VALUE_READABLE;
		__set_value(STRING_VALUE, PREF_KEY_DEFAULT_VIEW_LEVEL, (void *)value);
	}
}

view_level_type preference::get_view_level_type(void)
{
	char *value = NULL;
	view_level_type type;

	__get_value(STRING_VALUE, PREF_KEY_DEFAULT_VIEW_LEVEL, (void *)(&value));

	if (!strcmp((char *)value, PREF_VALUE_FIT_TO_WIDTH))
		type = VIEW_LEVEL_TYPE_FIT_TO_WIDTH;
	else
		type = VIEW_LEVEL_TYPE_READABLE;

	free((char *)value);

	return type;
}

void preference::set_homepage_type(homepage_type type)
{
	if (type == HOMEPAGE_TYPE_RECENTLY_VISITED_SITE) {
		const char *value = PREF_VALUE_RECENTLY_VISITED_SITE;
		__set_value(STRING_VALUE, PREF_KEY_HOMEPAGE_TYPE, (void *)value);
	} else if (type == HOMEPAGE_TYPE_USER_HOMEPAGE) {
		const char *value = PREF_VALUE_USER_HOMEPAGE;
		__set_value(STRING_VALUE, PREF_KEY_HOMEPAGE_TYPE, (void *)value);
	} else {
		const char *value = PREF_VALUE_EMPTY_PAGE;
		__set_value(STRING_VALUE, PREF_KEY_HOMEPAGE_TYPE, (void *)value);
	}
}

homepage_type preference::get_homepage_type(void)
{
	int index = 0;
	char *value = NULL;
	homepage_type type = HOMEPAGE_TYPE_EMPTY_PAGE;

	__get_value(STRING_VALUE, PREF_KEY_HOMEPAGE_TYPE, (void *)(&value));

	if (!strcmp((char *)value, PREF_VALUE_RECENTLY_VISITED_SITE))
		type = HOMEPAGE_TYPE_RECENTLY_VISITED_SITE;
	else if (!strcmp((char *)value, PREF_VALUE_USER_HOMEPAGE))
		type = HOMEPAGE_TYPE_USER_HOMEPAGE;
	else if (!strcmp((char *)value, PREF_VALUE_EMPTY_PAGE))
		type = HOMEPAGE_TYPE_EMPTY_PAGE;
	else
		type = HOMEPAGE_TYPE_USER_HOMEPAGE;

	free((char *)value);

	return type;
}

void preference::set_last_visited_uri(const char *uri)
{
	BROWSER_LOGD("uri=[%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN(uri);

	__set_value(STRING_VALUE, PREF_KEY_LAST_VISITED_URL, (void *)uri);
}

char *preference::get_last_visited_uri(void)
{
	BROWSER_LOGD("");

	char *value = NULL;
	__get_value(STRING_VALUE, PREF_KEY_LAST_VISITED_URL, (void *)(&value));

	return value;
}

void preference::set_user_homagepage(const char *uri)
{
	BROWSER_LOGD("uri=[%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN(uri);

	__set_value(STRING_VALUE, PREF_KEY_USER_HOMEPAGE, (void *)uri);
}

char *preference::get_user_homagepage(void)
{
	BROWSER_LOGD("");

	char *value = NULL;
	__get_value(STRING_VALUE, PREF_KEY_USER_HOMEPAGE, (void *)(&value));

	return value;
}

char *preference::get_homepage_uri(void)
{
	homepage_type type = get_homepage_type();
	if (type == HOMEPAGE_TYPE_RECENTLY_VISITED_SITE)
		return get_last_visited_uri();
	else if (type == HOMEPAGE_TYPE_USER_HOMEPAGE)
		return get_user_homagepage();
	else if (type == HOMEPAGE_TYPE_EMPTY_PAGE)
		return NULL;

	return get_user_homagepage();
}

void preference::set_javascript_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_RUN_JAVASCRIPT, (void *)enable);
}

Eina_Bool preference::get_javascript_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_RUN_JAVASCRIPT, (void *)(&value));

	return value;
}

void preference::set_display_images_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_DISPLAY_IMAGES, (void *)enable);
}

Eina_Bool preference::get_display_images_enabled(void)
{
	BROWSER_LOGD("");
	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_DISPLAY_IMAGES, (void *)(&value));

	return value;
}

void preference::set_block_popup_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_BLOCK_POPUP, (void *)enable);
}

Eina_Bool preference::get_block_popup_enabled(void)
{
	BROWSER_LOGD("");
	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_BLOCK_POPUP, (void *)(&value));

	return value;
}

void preference::set_text_encoding_type(text_encoding_type encoding_type)
{
	BROWSER_LOGD("");

	__set_value(INT_VALUE, PREF_KEY_TEXT_ENCODING, (void *)encoding_type);
}

text_encoding_type preference::get_text_encoding_type_index(void)
{
	BROWSER_LOGD("");

	text_encoding_type encoding_type = TEXT_ENCODING_TYPE_UTF_8;
	__get_value(INT_VALUE, PREF_KEY_TEXT_ENCODING, (void *)(&encoding_type));

	return encoding_type;
}

const char *preference::get_text_encoding_type_str(text_encoding_type index)
{
	BROWSER_LOGD("index[%d]", index);

	switch (index) {
	case TEXT_ENCODING_TYPE_ISO_8859_1:
		return PREF_VALUE_TEXT_ENCODING_ISD_8859_1;
	case TEXT_ENCODING_TYPE_UTF_8:
		return PREF_VALUE_TEXT_ENCODING_UTF_8;
	case TEXT_ENCODING_TYPE_GBK:
		return PREF_VALUE_TEXT_ENCODING_GBK;
	case TEXT_ENCODING_TYPE_BIG5:
		return PREF_VALUE_TEXT_ENCODING_BIG5;
	case TEXT_ENCODING_TYPE_ISO_2022_JP:
		return PREF_VALUE_TEXT_ENCODING_ISO_2022_JP;
	case TEXT_ENCODING_TYPE_SHIFT_JIS:
		return PREF_VALUE_TEXT_ENCODING_SHIFT_JIS;
	case TEXT_ENCODING_TYPE_EUC_JP:
		return PREF_VALUE_TEXT_ENCODING_EUC_JP;
	case TEXT_ENCODING_TYPE_EUC_KR:
		return PREF_VALUE_TEXT_ENCODING_EUC_KR;
	case TEXT_ENCODING_TYPE_AUTOMATIC:
		return PREF_VALUE_TEXT_ENCODING_AUTOMATIC;
	case TEXT_ENCODING_TYPE_NUM:
	default :
		break;
	}

	return NULL;
}

void preference::set_security_warnings_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_SHOW_SECURITY_WARNINGS, (void *)enable);
}

Eina_Bool preference::get_security_warnings_enabled(void)
{
	BROWSER_LOGD("");
	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_SHOW_SECURITY_WARNINGS, (void *)(&value));

	return value;
}

void preference::set_accept_cookies_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");
	__set_value(BOOL_VALUE, PREF_KEY_ACCEPT_COOKIES, (void *)enable);
}

Eina_Bool preference::get_accept_cookies_enabled(void)
{
	BROWSER_LOGD("");
	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_ACCEPT_COOKIES, (void *)(&value));

	return value;
}

void preference::set_auto_save_id_password_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");
	__set_value(BOOL_VALUE, PREF_KEY_AUTO_SAVE_ID_PASSWORD, (void *)enable);
}

Eina_Bool preference::get_auto_save_id_password_enabled(void)
{
	BROWSER_LOGD("");
	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_AUTO_SAVE_ID_PASSWORD, (void *)(&value));

	return value;
}

void preference::set_auto_save_form_data_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");
	__set_value(BOOL_VALUE, PREF_KEY_AUTO_SAVE_FORM_DATA, (void *)enable);
}

Eina_Bool preference::get_auto_save_form_data_enabled(void)
{
	BROWSER_LOGD("");
	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_AUTO_SAVE_FORM_DATA, (void *)(&value));

	return value;
}

void preference::set_location_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");
	__set_value(BOOL_VALUE, PREF_KEY_ENABLE_LOCATION, (void *)enable);
}

Eina_Bool preference::get_location_enabled(void)
{
	BROWSER_LOGD("");
	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_ENABLE_LOCATION, (void *)(&value));

	return value;
}

void preference::set_multiwindow_handle_rate(int size)
{
	BROWSER_LOGD("");
	__set_value(INT_VALUE, PREF_KEY_MULTIWINDOW_HANDLE_RATE, (void *)size);
}

int preference::get_multiwindow_handle_rate(void)
{
	BROWSER_LOGD("");
	int size = 0;

	__get_value(INT_VALUE, PREF_KEY_MULTIWINDOW_HANDLE_RATE, (void *)(&size));

	BROWSER_LOGD("size = [%d]", size);

	return size;
}

void preference::set_search_engine_type(int type)
{
	BROWSER_LOGD("[%s]", __func__);
	__set_value(INT_VALUE, PREF_VALUE_SEARCH_ENGINE_TYPE, (void *)type);
}

int preference::get_search_engine_type(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int type = 0;

	__get_value(INT_VALUE, PREF_VALUE_SEARCH_ENGINE_TYPE, (void *)(&type));

	return type;
}

#if defined(MDM)
void preference::set_mdm_auto_fill_status(int status)
{
	BROWSER_LOGD("");
	__set_value(INT_VALUE, PREF_KEY_MDM_AUTO_FILL, (void *)status);
}

int preference::get_mdm_auto_fill_status(void)
{
	BROWSER_LOGD("");
	int status = 0;

	__get_value(INT_VALUE, PREF_KEY_MDM_AUTO_FILL, (void *)(&status));

	return status;
}

void preference::set_mdm_cookies_status(int status)
{
	BROWSER_LOGD("");
	__set_value(INT_VALUE, PREF_KEY_MDM_COOKIES, (void *)status);
}

int preference::get_mdm_cookies_status(void)
{
	BROWSER_LOGD("");
	int status = 0;

	__get_value(INT_VALUE, PREF_KEY_MDM_COOKIES, (void *)(&status));

	return status;
}

void preference::set_mdm_fraud_warning_status(int status)
{
	BROWSER_LOGD("");
	__set_value(INT_VALUE, PREF_KEY_MDM_FRAUD_WARNING, (void *)status);
}

int preference::get_mdm_fraud_warning_status(void)
{
	BROWSER_LOGD("");
	int status = 0;

	__get_value(INT_VALUE, PREF_KEY_MDM_FRAUD_WARNING, (void *)(&status));

	return status;
}

void preference::set_mdm_javascript_status(int status)
{
	BROWSER_LOGD("");
	__set_value(INT_VALUE, PREF_KEY_MDM_JAVASCRIPT, (void *)status);
}

int preference::get_mdm_javascript_status(void)
{
	BROWSER_LOGD("");
	int status = 0;

	__get_value(INT_VALUE, PREF_KEY_MDM_JAVASCRIPT, (void *)(&status));

	return status;
}

void preference::set_mdm_popup_status(int status)
{
	BROWSER_LOGD("");
	__set_value(INT_VALUE, PREF_KEY_MDM_POPUP, (void *)status);
}

int preference::get_mdm_popup_status(void)
{
	BROWSER_LOGD("");
	int status = 0;

	__get_value(INT_VALUE, PREF_KEY_MDM_POPUP, (void *)(&status));

	return status;
}
#endif

#if !defined(TIZEN_PUBLIC)
void preference::set_reader_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_RUN_READER, (void *)enable);
}

Eina_Bool preference::get_reader_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_RUN_READER, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

void preference::set_reader_font_size(int size)
{
	BROWSER_LOGD("");
	__set_value(INT_VALUE, PREF_KEY_READER_FONT_SIZE, (void *)size);
}

int preference::get_reader_font_size(void)
{
	BROWSER_LOGD("");
	int size = 0;

	__get_value(INT_VALUE, PREF_KEY_READER_FONT_SIZE, (void *)(&size));

	BROWSER_LOGD("size = [%d]", size);

	return size;
}

void preference::set_plugins_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_RUN_PLUGINS, (void *)enable);
}

Eina_Bool preference::get_plugins_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_RUN_PLUGINS, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

#if defined(TEST_CODE)
void preference::set_demo_mode_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_DEMO_MODE, (void *)enable);
}

Eina_Bool preference::get_demo_mode_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_DEMO_MODE, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

void preference::set_recording_surface_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_RECORDING_SURFACE, (void *)enable);
}

Eina_Bool preference::get_recording_surface_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_RECORDING_SURFACE, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

void preference::set_remote_web_inspector_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_REMOTE_WEB_INSPECTOR, (void *)enable);
}

Eina_Bool preference::get_remote_web_inspector_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_REMOTE_WEB_INSPECTOR, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

void preference::set_accelerated_composition_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_ACCELERATED_COMPOSITION, (void *)enable);
}

Eina_Bool preference::get_accelerated_composition_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_ACCELERATED_COMPOSITION, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

void preference::set_external_video_player_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_EXTERNAL_VIDEO_PLAYER, (void *)enable);
}

Eina_Bool preference::get_external_video_player_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_EXTERNAL_VIDEO_PLAYER, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

void preference::set_composited_render_layer_borders_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_COMPOSITED_RENDER_LAYER_BORDERS, (void *)enable);
}

Eina_Bool preference::get_composited_render_layer_borders_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_COMPOSITED_RENDER_LAYER_BORDERS, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

void preference::set_memory_usage_profiling_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_MEMORY_USAGE_PROFILING, (void *)enable);
}

Eina_Bool preference::get_memory_usage_profiling_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_MEMORY_USAGE_PROFILING, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

void preference::set_zoom_button_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_ZOOM_BUTTON, (void *)enable);
}

Eina_Bool preference::get_zoom_button_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_ZOOM_BUTTON, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

void preference::set_low_memory_mode_enabled(Eina_Bool enable)
{
	BROWSER_LOGD("");

	__set_value(BOOL_VALUE, PREF_KEY_LOW_MEMORY_MODE, (void *)enable);
}

Eina_Bool preference::get_low_memory_mode_enabled(void)
{
	BROWSER_LOGD("");

	bool value = false;

	__get_value(BOOL_VALUE, PREF_KEY_LOW_MEMORY_MODE, (void *)(&value));

	BROWSER_LOGD("value = [%d]", value);

	return value;
}

#endif /* TEST_CODE */

#endif /* TIZEN_PUBLIC */

