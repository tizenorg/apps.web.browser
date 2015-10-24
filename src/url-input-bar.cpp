/*
 * Copyright 2014  Samsung Electronics Co., Ltd
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
 * Contact: Kwangyong Choi <ky0.choi@samsung.com>
 *
 */

#include "url-input-bar.h"

#include <cctype>
#include <regex.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "preference.h"
#include "search-engine-manager.h"
#include "url-bar.h"
#include "webview-list.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

#define URL_INPUT_BAR_EDJ browser_edj_dir"/url-input-bar.edj"
#define URI_ENTRY_FONT_SIZE "font_size=22"
#define URI_ENTRY_STYLE "DEFAULT='"URI_ENTRY_FONT_SIZE" color=#000000 ellipsis=1'"
#define URLEXPR "((https?|ftp|gopher|telnet|file|notes|ms-help):((//)|(\\\\))[\\w\\d:#@%/;$()~_?+-=\\.&]+)"
#define TRIM_SPACE " \t\n\v"

Eina_Bool url_input_bar::m_double_or_long_click = EINA_FALSE;

inline std::string _trim(std::string& s, const std::string& drop = TRIM_SPACE)
{
	std::string r = s.erase(s.find_last_not_of(drop) + 1);
	return r.erase(0, r.find_first_not_of(drop));
}

static Eina_Bool _is_regular_express(const char *uri)
{
	BROWSER_SECURE_LOGD("uri=[%s]", uri);

	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");
	regex_t regex;
	if (regcomp(&regex, URLEXPR, REG_EXTENDED | REG_ICASE) != 0) {
		BROWSER_LOGD("regcomp failed");
		return EINA_FALSE;
	}

	if (regexec(&regex, uri, 0, NULL, REG_NOTEOL) == 0) {
		BROWSER_LOGD("url expression");
		regfree(&regex);
		return EINA_TRUE;
	}

	regfree(&regex);
	if (!strcmp(uri, blank_page))
		return EINA_TRUE;

	int len = strlen(uri);
	if (*uri != '.' && *(uri + len - 1) != '.' && strstr(uri, ".")) {
		BROWSER_LOGD("url tmp expression");
		return EINA_TRUE;
	}

	return EINA_FALSE;
}

url_input_bar::url_input_bar(Evas_Object *parent)
	: m_main_layout(NULL)
	, m_search_engine_button(NULL)
	, m_context_popup(NULL)
	, m_entry(NULL)
{
	BROWSER_LOGD("");

	elm_theme_extension_add(NULL, URL_INPUT_BAR_EDJ);
	_create_main_layout(parent);
}

url_input_bar::~url_input_bar(void)
{
	BROWSER_LOGD("");

	if (m_main_layout)
		evas_object_del(m_main_layout);

	elm_theme_extension_del(NULL, URL_INPUT_BAR_EDJ);

	m_saved_string.clear();
}

void url_input_bar::set_uri(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RET_MSG_IF(!uri, "uri is NULL");
	char *markup = elm_entry_utf8_to_markup(uri);

	elm_entry_entry_set(m_entry, markup);
	std::string font_size_tag = "DEFAULT='" + m_browser->get_browser_view()->get_font_size_tag() + " " + uri_entry_style_color +"'";
	elm_entry_text_style_user_push(m_entry, font_size_tag.c_str());
	Evas_Object *access = elm_access_object_get(elm_entry_textblock_get(m_entry));
	elm_access_info_set(access, ELM_ACCESS_TYPE, BR_STRING_SEARCH_FIELD_T);

	free(markup);
}

void url_input_bar::show_ime(Eina_Bool enable_voice)
{
	BROWSER_LOGD("");
	const char *init_text = NULL;
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (!wv)
		init_text = NULL;
	else
		init_text = wv->get_uri();

	BROWSER_SECURE_LOGD("init_text=[%s]", init_text);

	//When the uri is null then we should set that to blank page that is about:blank
	if(init_text == NULL) {
		elm_object_signal_emit(m_main_layout, "hide,cross,button,signal", "");
		set_uri(blank_page);
	} else {
		Eina_Bool is_data_scheme = m_browser->get_browser_view()->get_current_webview()->is_data_scheme();
		elm_object_signal_emit(m_main_layout, "show,cross,button,signal", "");
		if (is_data_scheme) {
			char *visible_uri = strndup(init_text, URI_VISIBLE_LENGTH);
			if (!visible_uri) {
				set_uri(init_text);
				return;
			}
			set_uri(visible_uri);
			free(visible_uri);
		} else
			set_uri(init_text);
	}

	// Set guide text.
	elm_object_part_text_set(m_entry, "elm.guide", (std::string("<"URI_ENTRY_FONT_SIZE">") + BR_STRING_URL_GUIDE_TEXT + "</font>").c_str());

	if (m_browser->is_keyboard_active())
		elm_object_focus_allow_set(m_search_engine_button, EINA_TRUE);
	else
		elm_object_focus_allow_set(m_search_engine_button, EINA_FALSE);

	elm_entry_context_menu_clear(m_entry);
	elm_entry_context_menu_item_add(m_entry,"Clipboard",NULL,ELM_ICON_STANDARD,NULL,NULL);
	elm_entry_context_menu_item_add(m_entry,"Translate",NULL,ELM_ICON_STANDARD,NULL,NULL);
	elm_entry_context_menu_item_add(m_entry, gettext(BR_STRING_SHARE), browser_img_dir"/I01_ctx_popup_icon_share.png", ELM_ICON_FILE, __share_cb, this);
	elm_entry_context_menu_disabled_set(m_entry, EINA_TRUE);

	ecore_idler_add(__select_all_text_cb, this);
}

Eina_Bool url_input_bar::__select_all_text_cb(void *data)
{
	BROWSER_LOGD("");

	url_input_bar *uib = (url_input_bar *)data;
	uib->select_all_text();

	return ECORE_CALLBACK_CANCEL;
}

void url_input_bar::select_all_text(void)
{
	BROWSER_LOGD("");
	const char *input_uri = elm_entry_entry_get(m_entry);

	elm_object_focus_set(m_entry, EINA_TRUE);

	if (input_uri && strlen(input_uri) > 0) {
		elm_entry_select_all(m_entry);
		elm_object_signal_emit(m_entry, "app,selection,handler,disable", "app");
	} else {
		elm_entry_context_menu_clear(m_entry);
		elm_entry_context_menu_item_add(m_entry,"Clipboard",NULL,ELM_ICON_STANDARD,NULL,NULL);
	}

	if (m_browser->is_tts_enabled()) {
		// Set access highlight.
		Evas_Object *access = elm_access_object_get(elm_entry_textblock_get(m_entry));
		elm_access_highlight_set(access);
	}
}

void url_input_bar::set_entry_changed_callback(void)
{
	BROWSER_LOGD("");

	evas_object_smart_callback_del(m_entry, "preedit,changed", __entry_changed_cb);
	evas_object_smart_callback_del(m_entry, "changed", __entry_changed_cb);

	evas_object_smart_callback_add(m_entry, "preedit,changed", __entry_changed_cb, this);
	evas_object_smart_callback_add(m_entry, "changed", __entry_changed_cb, this);
}

void url_input_bar::unset_entry_changed_callback(void)
{
	BROWSER_LOGD("");

	evas_object_smart_callback_del(m_entry, "preedit,changed", __entry_changed_cb);
	evas_object_smart_callback_del(m_entry, "changed", __entry_changed_cb);

	// Context popup can be remains when the IME is hide by back key.
	if (m_context_popup) {
		evas_object_del(m_context_popup);
		m_context_popup = NULL;
	}
}

void url_input_bar::resize_search_ctxpopup(void)
{
	if (!m_context_popup)
		return;

	ecore_idler_add(__resize_search_ctxpopup_cb, this);
}

void url_input_bar::set_landscape_mode(Eina_Bool landscape)
{
	if (landscape) {
		elm_object_signal_emit(m_main_layout, "show,landscape-mode,signal", "");
	} else {
		elm_object_signal_emit(m_main_layout, "show,portrait-mode,signal", "");
	}
}

Eina_Bool url_input_bar::get_focus()
{
	return elm_object_focus_get(m_entry);
}

void url_input_bar::set_focus()
{
	elm_object_focus_set(m_entry, EINA_TRUE);
}

void url_input_bar::set_focus(Eina_Bool focus)
{
	elm_object_focus_set(m_entry, focus);
}

void url_input_bar::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	// Url input bar layout.
	m_main_layout = elm_layout_add(parent);
	RET_MSG_IF(!m_main_layout, "elm_layout_add failed.");

	elm_layout_file_set(m_main_layout, URL_INPUT_BAR_EDJ, "url-input-bar-layout");

	// Search engine button.
	m_search_engine_button = elm_button_add(m_main_layout);
	RET_MSG_IF(!m_search_engine_button, "elm_button_add failed");

	// Set default search engine based on csc feature.
	int search_engine_type = m_preference->get_search_engine_type();
	if (search_engine_type == SEARCH_ENGINE_NOT_SELECTED) {
		BROWSER_LOGD("Default search engine google");
		search_engine_type = SEARCH_ENGINE_GOOGLE;
		m_preference->set_search_engine_type(search_engine_type);
	}

	std::string msg;
	if (search_engine_type == SEARCH_ENGINE_YAHOO) {
		elm_object_style_set(m_search_engine_button, "browser/search/yahoo");
		msg = BR_STRING_CTXMENU_YAHOO;
	} else if (search_engine_type == SEARCH_ENGINE_BING) {
		elm_object_style_set(m_search_engine_button, "browser/search/bing");
		msg = BR_STRING_CTXMENU_BING;
	} else {
		elm_object_style_set(m_search_engine_button, "browser/search/google");
		msg = BR_STRING_CTXMENU_GOOGLE;
	}
	msg = msg + " " + BR_STRING_DOUBLE_TAP_TO_SELECT_A_SEARCH_ENGINE_T;
	elm_access_info_set(m_search_engine_button, ELM_ACCESS_INFO, msg.c_str());

	evas_object_smart_callback_add(m_search_engine_button, "clicked", __search_engine_clicked_cb, this);
	elm_object_focus_allow_set(m_search_engine_button, EINA_FALSE);

	elm_object_part_content_set(m_main_layout, "elm.swallow.search-engine-button", m_search_engine_button);

	// Entry.
	m_entry = elm_entry_add(m_main_layout);
	RET_MSG_IF(!m_entry, "elm_entry_add failed.");
	Evas_Object *dim_area = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(m_main_layout), "cross.swallow.button");
	if (dim_area)
		edje_object_signal_callback_add(elm_layout_edje_get(m_main_layout), "mouse,down,1", "cross.swallow.button", __dim_area_clicked_cb, this);

	elm_entry_single_line_set(m_entry, EINA_TRUE);
	elm_entry_scrollable_set(m_entry,  EINA_TRUE);

	elm_entry_cnp_mode_set(m_entry, ELM_CNP_MODE_PLAINTEXT);
	elm_entry_prediction_allow_set(m_entry, EINA_TRUE);
	elm_entry_input_panel_enabled_set(m_entry, EINA_TRUE);
	elm_entry_input_panel_layout_set(m_entry, ELM_INPUT_PANEL_LAYOUT_URL);
	elm_entry_input_panel_return_key_type_set(m_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_GO);
	evas_object_size_hint_weight_set(m_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
#if defined(HW_MORE_BACK_KEY)
	eext_entry_selection_back_event_allow_set(m_entry, EINA_TRUE);
#endif
	elm_object_part_text_set(m_entry, "elm.guide", (std::string("<"URI_ENTRY_FONT_SIZE">") + BR_STRING_URL_GUIDE_TEXT + "</font>").c_str());
	elm_entry_text_style_user_push(m_entry, URI_ENTRY_STYLE);

	Evas_Object *bt_clear = elm_object_part_content_get(m_entry, "elm.swallow.clear");
	elm_access_info_set(bt_clear, ELM_ACCESS_INFO, BR_STRING_CLEAR_ALL);

	evas_object_smart_callback_add(m_entry, "activated", __enter_key_cb, this);
	evas_object_smart_callback_add(m_entry, "clicked", __entry_clicked_cb, this);
	evas_object_smart_callback_add(m_entry, "clicked,double", __entry_double_clicked_cb, this);
	evas_object_smart_callback_add(m_entry, "longpressed", __entry_longpressed_cb, this);
	evas_object_smart_callback_add(m_entry, "language,changed", __entry_language_changed_cb, this);

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callbacks.
	eext_object_event_callback_add(m_entry, EEXT_CALLBACK_BACK, __back_cb, this);
	eext_object_event_callback_add(m_entry, EEXT_CALLBACK_MORE, __menu_clicked_cb, this);
#endif

	elm_object_part_content_set(m_main_layout, "elm.swallow.entry", m_entry);
}

void url_input_bar::__dim_area_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	url_input_bar *uib = (url_input_bar *)data;
	elm_object_signal_emit(uib->m_main_layout, "hide,cross,button,signal", "");
	elm_entry_entry_set(uib->m_entry, "");
}

#if defined(HW_MORE_BACK_KEY)
void url_input_bar::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	m_browser->get_browser_view()->show_url_bar();
}

void url_input_bar::__menu_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	browser_view *bv = m_browser->get_browser_view();
	bv->show_url_bar();
	bv->get_url_bar()->get_more_menu_manager()->show_more_menu();
}
#endif

void url_input_bar::__search_engine_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	url_input_bar *uib = (url_input_bar *)data;
	if (!uib->m_context_popup)
		uib->_show_search_context_popup(obj);
}

Eina_Bool url_input_bar::__resize_search_ctxpopup_cb(void *data)
{
	BROWSER_LOGD("");

	url_input_bar *uib = (url_input_bar *)data;

	uib->_calculate_ctx_popup_height(uib->m_context_popup);

	// Move search ctxpopup to proper position.
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(uib->m_search_engine_button, &x, &y, &w, &h);
	evas_object_move(uib->m_context_popup, x + w - 2, y + h - 2);

	return ECORE_CALLBACK_CANCEL;
}

void url_input_bar::_show_search_context_popup(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!parent, "parent is NULL");

	Evas_Object *context_popup = elm_ctxpopup_add(m_window);
	RET_MSG_IF(!context_popup, "elm_ctxpopup_add failed");

	Eina_Bool is_keyboard = m_browser->is_keyboard_active();

	elm_ctxpopup_direction_priority_set(context_popup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UP);
	evas_object_smart_callback_add(context_popup,"dismissed", __dismissed_cb, this);
	elm_ctxpopup_auto_hide_disabled_set(context_popup, EINA_TRUE);

	elm_object_style_set(context_popup, "dropdown/label");

	if (is_keyboard == EINA_TRUE)
		elm_object_focus_allow_set(context_popup, EINA_TRUE);
	else
		elm_object_focus_allow_set(context_popup, EINA_FALSE);

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(context_popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(context_popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
#endif

	Elm_Object_Item *it = NULL;
	Evas_Object *it_obj = NULL;
	Evas_Object *it_high_obj = NULL;

	it = elm_ctxpopup_item_append(context_popup, BR_STRING_CTXMENU_GOOGLE, NULL, __google_engine_selected_cb, this);
	it_obj = elm_object_item_access_object_get(it);
	elm_access_info_set(it_obj, ELM_ACCESS_INFO, BR_STRING_CTXMENU_GOOGLE);
	elm_access_info_set(it_obj, ELM_ACCESS_STATE, BR_STRING_DOUBLE_TAP_TO_SELECT_A_SEARCH_ENGINE_T);
	it_high_obj = it_obj;
	if (is_keyboard == EINA_TRUE)
		elm_object_focus_allow_set(it_obj, EINA_TRUE);
	else
		elm_object_focus_allow_set(it_obj, EINA_FALSE);

#if defined(ENABLE_BING_SEARCH)
	BROWSER_LOGD("bing");
	it = elm_ctxpopup_item_append(context_popup, BR_STRING_CTXMENU_BING, NULL, __bing_engine_selected_cb, this);
	it_obj = elm_object_item_access_object_get(it);
	elm_access_info_set(it_obj, ELM_ACCESS_INFO, BR_STRING_CTXMENU_BING);
	elm_access_info_set(it_obj, ELM_ACCESS_STATE, BR_STRING_DOUBLE_TAP_TO_SELECT_A_SEARCH_ENGINE_T);
	if (is_keyboard == EINA_TRUE)
		elm_object_focus_allow_set(it_obj, EINA_TRUE);
	else
		elm_object_focus_allow_set(it_obj, EINA_FALSE);
#endif

#if defined(ENABLE_YAHOO_SEARCH)
	BROWSER_LOGD("yahoo");
	it = elm_ctxpopup_item_append(context_popup, BR_STRING_CTXMENU_YAHOO, NULL, __yahoo_engine_selected_cb, this);
	it_obj = elm_object_item_access_object_get(it);
	elm_access_info_set(it_obj, ELM_ACCESS_INFO, BR_STRING_CTXMENU_YAHOO);
	elm_access_info_set(it_obj, ELM_ACCESS_STATE, BR_STRING_DOUBLE_TAP_TO_SELECT_A_SEARCH_ENGINE_T);
	if (is_keyboard == EINA_TRUE)
		elm_object_focus_allow_set(it_obj, EINA_TRUE);
	else
		elm_object_focus_allow_set(it_obj, EINA_FALSE);
#endif

	Evas_Coord x, y, w, h;
	evas_object_geometry_get(parent, &x, &y, &w, &h);
	evas_object_move(context_popup, x + w - 2, y + h - 2);
	_calculate_ctx_popup_height(context_popup);
	evas_object_show(context_popup);

	m_context_popup = context_popup;

	//Keep focus to prevent keypad hide.
	if (m_browser->get_browser_view()->is_ime_on())
		elm_object_focus_set(m_entry, EINA_TRUE);
	elm_access_highlight_set(it_high_obj);
}

//This method is to alter the height of ctx popup to fit to the screen and to make direction UP
void url_input_bar::_calculate_ctx_popup_height(Evas_Object *ctx_popup)
{
	int changed_ang = elm_win_rotation_get(m_window);
	if (changed_ang == 90 || changed_ang == 270) {
		/* landscape mode */
		int popup_height = VIEWER_ATTACH_LIST_ITEM_HEIGHT * 2; // Show max 2 items in landscape mode
		evas_object_size_hint_max_set(ctx_popup, 0, ELM_SCALE_SIZE(popup_height));
	} else
		/* vertical mode */
		evas_object_size_hint_max_set(ctx_popup, 0, 0);

	elm_layout_sizing_eval(ctx_popup);
}

void url_input_bar::update_search_engine(int type)
{
	Evas_Object *search_engine_button = elm_object_part_content_get(m_main_layout, "elm.swallow.search-engine-button");
	RET_MSG_IF(!search_engine_button, "can not find search engine button");

	switch (type) {
	case SEARCH_ENGINE_GOOGLE:
		elm_object_style_set(search_engine_button, "browser/search/google");
		elm_access_info_set(search_engine_button, ELM_ACCESS_INFO, BR_STRING_CTXMENU_GOOGLE);
		break;

	case SEARCH_ENGINE_YAHOO:
		elm_object_style_set(search_engine_button, "browser/search/yahoo");
		elm_access_info_set(search_engine_button, ELM_ACCESS_INFO, BR_STRING_CTXMENU_YAHOO);
		break;

	case SEARCH_ENGINE_BING:
		elm_object_style_set(search_engine_button, "browser/search/bing");
		elm_access_info_set(search_engine_button, ELM_ACCESS_INFO, BR_STRING_CTXMENU_BING);
		break;

	case SEARCH_ENGINE_NOT_SELECTED:
	default:
		elm_object_style_set(search_engine_button, "browser/search/google");
		elm_access_info_set(search_engine_button, ELM_ACCESS_INFO, BR_STRING_CTXMENU_GOOGLE);
		break;
	}
}

void url_input_bar::__dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	url_input_bar *uib = (url_input_bar *)data;

	evas_object_del(obj);
	uib->m_context_popup = NULL;
}

void url_input_bar::__google_engine_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	url_input_bar *uib = (url_input_bar *)data;

	m_preference->set_search_engine_type(SEARCH_ENGINE_GOOGLE);
	uib->update_search_engine(SEARCH_ENGINE_GOOGLE);
	evas_object_del(obj);
	uib->m_context_popup = NULL;
}

void url_input_bar::__yahoo_engine_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	url_input_bar *uib = (url_input_bar *)data;

	m_preference->set_search_engine_type(SEARCH_ENGINE_YAHOO);
	uib->update_search_engine(SEARCH_ENGINE_YAHOO);
	evas_object_del(obj);
	uib->m_context_popup = NULL;
}

void url_input_bar::__bing_engine_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	url_input_bar *uib = (url_input_bar *)data;

	m_preference->set_search_engine_type(SEARCH_ENGINE_BING);
	uib->update_search_engine(SEARCH_ENGINE_BING);
	evas_object_del(obj);
	uib->m_context_popup = NULL;
}

void url_input_bar::__share_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	url_input_bar *uib = (url_input_bar *)data;
	const char *uri = NULL;

	uri = elm_entry_selection_get(uib->m_entry);
	if (!uri)
		uri = elm_entry_entry_get(uib->m_entry);

	m_browser->get_browser_view()->share(uri, NULL);
}

void url_input_bar::__enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *input_uri = elm_entry_entry_get(obj);
	if (!input_uri || strlen(input_uri) == 0)
		return;

	browser_view *bv = m_browser->get_browser_view();
	url_input_bar *uib = (url_input_bar *)data;
	evas_object_smart_callback_del(uib->m_entry, "preedit,changed", __entry_changed_cb);
	evas_object_smart_callback_del(uib->m_entry, "changed", __entry_changed_cb);

	std::string input_uri_str = std::string(input_uri);
	std::string trim_uri = _trim(input_uri_str);
	input_uri = trim_uri.c_str();

	char *utf8 = elm_entry_markup_to_utf8(input_uri);
	if (!utf8 || strlen(utf8) == 0) {
		if (utf8)
			free(utf8);
		return;
	}

	BROWSER_SECURE_LOGD("input_uri=[%s]", utf8);

	webview *wv = bv->get_current_webview();
	if (!wv) {
		wv = m_browser->get_webview_list()->create_webview(EINA_TRUE);
		wv->set_request_uri(utf8);
		bv->set_current_webview(wv);
	}
	if (utf8 && strlen(utf8) == strlen(about_debug) && !strncmp(utf8, about_debug, strlen(about_debug))) {
		m_browser->set_developer_mode(EINA_TRUE);
		BROWSER_SECURE_LOGD("set_developer_mode : Enabled");
		free(utf8);
		return;
	}
	if (_is_regular_express(utf8))
		wv->load_uri(utf8);
	else {
		search_engine_manager *search_manager = new search_engine_manager();
		std::string query_uri = search_manager->query_string_get(m_preference->get_search_engine_type(), utf8);

		wv->load_uri(query_uri.c_str());

		delete search_manager;
	}

	free(utf8);

	// Hide url input bar.
	m_browser->get_browser_view()->show_url_bar();
}

void url_input_bar::__entry_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (m_double_or_long_click == EINA_FALSE) {
		BROWSER_LOGD("");
		elm_entry_select_none(obj);
		elm_entry_context_menu_disabled_set(obj, EINA_FALSE);
		//TODO: after selecting text and than single tapping (clicking) context menu
		// should hide, now it's kept (or it should change to 'paste' context menu,
		// without cut and copy options)
	} else {
		m_double_or_long_click = EINA_FALSE;
	}
}

void url_input_bar::__entry_double_clicked_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("");
	url_input_bar *uib = (url_input_bar *)data;
	elm_object_signal_emit(uib->m_entry, "app,selection,handler,enable", "app");
	m_double_or_long_click = EINA_TRUE;
}

void url_input_bar::__entry_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	url_input_bar *uib = (url_input_bar *)data;
	elm_entry_context_menu_disabled_set(obj, EINA_FALSE);
	elm_object_signal_emit(uib->m_entry, "app,selection,handler,enable", "app");
	m_double_or_long_click = EINA_TRUE;
}

char *url_input_bar::_search_keyword_get(Evas_Object *obj)
{
	char *text = NULL;
	char *result = NULL;

	RETV_MSG_IF(obj == NULL, NULL, "obj is NULL");

	text = elm_entry_markup_to_utf8(elm_object_text_get(obj));
	if(text){
		if (strlen(text) > 0) {
			char *trim = text;
			char *tmp = NULL;

			while (trim[0] && isspace(trim[0]))
				trim++;

			if (strlen(trim) == 0) {
				BROWSER_LOGD("white spaces only, skipped");
			} else {
				// tmp points at the end
				tmp = trim + strlen(trim) - 1;
				while (tmp[0] && isspace(tmp[0])) {
					tmp[0] = '\0';
					tmp--;
				}
				result = strdup(trim);
			}
		}
		free(text);
	}
	return result;
}

void url_input_bar::reset_string(void)
{
	m_saved_string = "";
}

void url_input_bar::_set_string(std::string &kw)
{
	m_saved_string = kw;
}

void url_input_bar::__entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	RET_MSG_IF(!data, "data is NULL");
	url_input_bar *uib = (url_input_bar *)data;

	elm_entry_context_menu_clear(uib->m_entry);
	elm_entry_context_menu_item_add(uib->m_entry,"Clipboard",NULL,ELM_ICON_STANDARD,NULL,NULL);
	if (!elm_entry_is_empty(uib->m_entry)) {
		elm_object_signal_emit(uib->m_main_layout, "show,cross,button,signal", "");
		elm_entry_context_menu_item_add(uib->m_entry,"Translate",NULL,ELM_ICON_STANDARD,NULL,NULL);
		elm_entry_context_menu_item_add(uib->m_entry, gettext(BR_STRING_SHARE), browser_img_dir"/I01_ctx_popup_icon_share.png", ELM_ICON_FILE, __share_cb, data);
	} else
		elm_object_signal_emit(uib->m_main_layout, "hide,cross,button,signal", "");


	char *kw = uib->_search_keyword_get(obj);
	std::string new_str;
	if (kw) {
		new_str = kw;
		free(kw);
	} else
		new_str = "";

	if(strlen(new_str.c_str()) >= URI_INPUT_ENTRY_MAX_COUNT){
		m_browser->get_browser_view()->show_notification(new_str.c_str(), obj, URI_INPUT_ENTRY_MAX_COUNT);
		std::string substr = new_str.substr(0, URI_INPUT_ENTRY_MAX_COUNT-1);
		elm_object_text_set(obj, substr.c_str());
		elm_entry_cursor_end_set(obj);
		new_str = substr;
	}

	std::string prev_str = uib->_get_string();
	BROWSER_SECURE_LOGD("new[%s] previous[%s]", new_str.c_str(), prev_str.c_str());

	if (new_str.compare(prev_str) == 0) {
		BROWSER_SECURE_LOGD("SAME");
		return;
	}

	uib->_set_string(new_str);
}

void url_input_bar::__entry_language_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	// Update guide text for the language.
	url_input_bar *uib = (url_input_bar *)data;
	elm_object_part_text_set(uib->m_entry, "elm.guide", (std::string("<"URI_ENTRY_FONT_SIZE">") + BR_STRING_URL_GUIDE_TEXT + "</font>").c_str());
}

void url_input_bar::_remove_preedit_tags(std::string &kw)
{
	if (kw.empty()) {
		BROWSER_LOGE("kw empty");
		return;
	}
	unsigned int pos = kw.find("<preedit>");
	if (pos != std::string::npos)
		kw.erase(pos, strlen("<preedit>"));

	pos = kw.find("</preedit>");
	if (pos != std::string::npos)
		kw.erase(pos, strlen("</preedit>"));
}

void url_input_bar::_remove_underline_color_tags(std::string &kw)
{
	if (kw.empty()) {
		BROWSER_LOGE("kw empty");
		return;
	}
	unsigned int pos = kw.find("<underline_color");
	unsigned int end_pos = kw.find(">", pos);

	if (end_pos != std::string::npos && pos != std::string::npos)
		kw.erase(pos, end_pos - pos + 1);
	else
		return;

	pos = kw.find("</underline_color>");

	if(pos != std::string::npos)
		kw.erase(pos, strlen("</underline_color>"));

}

void url_input_bar::_remove_preedit_sel_section(std::string &kw)
{
	if (kw.empty()) {
		BROWSER_LOGE("kw empty");
		return;
	}
	unsigned int start = kw.find("<preedit_sel>");
	if (start != std::string::npos) {
		unsigned int length = kw.find("</preedit_sel>") - start + strlen("</preedit_sel>");
		BROWSER_SECURE_LOGD("KW:%s",kw.c_str());
		kw.erase(start, length);
		BROWSER_SECURE_LOGD("KW(preedit_sel tags removed):%s",kw.c_str());
	}
}
