/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#include "browser-context-menu.h"
#include "browser-view.h"
#include "browser-window.h"

#define BROWSER_CONTEXT_MENU_MARGIN (60 * elm_scale_get())

Browser_Context_Menu::Browser_Context_Menu(Evas_Object *naviframe, Browser_View *browser_view)
:	m_naviframe(naviframe)
	,m_browser_view(browser_view)
	,m_webview(NULL)
	,m_context_popup(NULL)
	,m_context_menu_data(NULL)
	,m_for_reader_view(EINA_FALSE)
	,m_current_context_menu_item(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Context_Menu::~Browser_Context_Menu(void)
{
	BROWSER_LOGD("[%s]", __func__);
	destroy_context_popup();
}

Eina_Bool Browser_Context_Menu::init(Evas_Object *webview, Eina_Bool reader_view)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!webview) {
		BROWSER_LOGE("webview is null");
		return EINA_FALSE;
	}

	m_webview = webview;
	m_for_reader_view = reader_view;

	deinit();

	Evas_Object *webkit = elm_webview_webkit_get(m_webview);

	evas_object_smart_callback_add(webkit, "webview,contextmenu,new", __context_menu_new_cb, this);
	evas_object_smart_callback_add(webkit, "webview,contextmenu,show", __context_menu_show_cb, this);
	evas_object_smart_callback_add(webkit, "webview,contextmenu,move", __context_menu_move_cb, this);
	evas_object_smart_callback_add(webkit, "webview,contextmenu,hide", __context_menu_hide_cb, this);
	evas_object_smart_callback_add(webkit, "webview,contextmenu,del", __context_menu_del_cb, this);
	evas_object_smart_callback_add(webkit, "contextmenu,customize", __context_menu_customize_cb, this);
	evas_object_smart_callback_add(webkit, "contextmenu,save,as", __context_menu_save_as_cb, this);
	if (!reader_view) {
		evas_object_smart_callback_add(webkit, "magnifier,shown", __magnifier_shown_cb, this);
		evas_object_smart_callback_add(webkit, "magnifier,hidden", __magnifier_hidden_cb, this);
	}

	return EINA_TRUE;
}

void Browser_Context_Menu::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	evas_object_smart_callback_del(webkit, "webview,contextmenu,new", __context_menu_new_cb);
	evas_object_smart_callback_del(webkit, "webview,contextmenu,show", __context_menu_show_cb);
	evas_object_smart_callback_del(webkit, "webview,contextmenu,move", __context_menu_move_cb);
	evas_object_smart_callback_del(webkit, "webview,contextmenu,hide", __context_menu_hide_cb);
	evas_object_smart_callback_del(webkit, "webview,contextmenu,del", __context_menu_del_cb);
	evas_object_smart_callback_del(webkit, "contextmenu,customize", __context_menu_customize_cb);
	evas_object_smart_callback_del(webkit, "contextmenu,save,as", __context_menu_save_as_cb);
	evas_object_smart_callback_del(webkit, "magnifier,shown", __magnifier_shown_cb);
	evas_object_smart_callback_del(webkit, "magnifier,hidden", __magnifier_hidden_cb);

}

void Browser_Context_Menu::__magnifier_shown_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;
	elm_object_scroll_freeze_pop(context_menu->m_browser_view->m_scroller);
	elm_object_scroll_freeze_push(context_menu->m_browser_view->m_scroller);
}

void Browser_Context_Menu::__magnifier_hidden_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;
	elm_object_scroll_freeze_pop(context_menu->m_browser_view->m_scroller);
}

void Browser_Context_Menu::__context_menu_new_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data || !event_info) {
		BROWSER_LOGE("data or event_info is null");
		return;
	}

	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;
	context_menu->m_context_menu_data = (Elm_WebView_Context_Menu_Data *)event_info;

	context_menu->destroy_context_popup();

	context_menu->m_context_popup = elm_ctxpopup_add(context_menu->m_naviframe);
	if (!context_menu->m_context_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	evas_object_size_hint_weight_set(context_menu->m_context_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
}

void Browser_Context_Menu::__context_menu_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;
	if (!context_menu->m_context_popup) {
		BROWSER_LOGE("context_menu->m_context_popup is null");
		return;
	}

	evas_object_move(context_menu->m_context_popup,
			context_menu->m_context_menu_data->mouse_down_event.canvas.x,
			context_menu->m_context_menu_data->mouse_down_event.canvas.y - BROWSER_CONTEXT_MENU_MARGIN);
	evas_object_show(context_menu->m_context_popup);

	const char *context_popup_style = elm_object_style_get(context_menu->m_context_popup);
	if (context_popup_style && strlen(context_popup_style)) {
		/* If context popup is normal style(not copy & paste style),
		  * give focus to context popup to hide the imf. */
		if (strncmp(context_popup_style, "extended/entry/pass_event", strlen("extended/entry/pass_event")))
			evas_object_focus_set(context_menu->m_context_popup, EINA_TRUE);
	}
}

void Browser_Context_Menu::__context_menu_move_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data || !event_info)
		return;

	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;
	if (!context_menu->m_context_popup)
		return;

	Evas_Point *position = (Evas_Point *)event_info;
	evas_object_move(context_menu->m_context_popup, position->x, position->y - BROWSER_CONTEXT_MENU_MARGIN);
	evas_object_show(context_menu->m_context_popup);
}

void Browser_Context_Menu::__context_menu_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;
	if (!context_menu->m_context_popup)
		return;

	evas_object_hide(context_menu->m_context_popup);
}

void Browser_Context_Menu::__context_menu_del_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;
	if (!context_menu->m_context_popup)
		return;

	evas_object_del(context_menu->m_context_popup);
	context_menu->m_context_popup = NULL;
}

std::string Browser_Context_Menu::_get_context_menu_item_text(Ewk_Context_Menu_Action action)
{
	switch(action) {
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW:
			return std::string(BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW);
		case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK:
			return std::string(BR_STRING_CTXMENU_DOWNLOAD_LINKED_FILE);
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD:
			return std::string(BR_STRING_CTXMENU_COPY_LINK_LOCATION);
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW:
			return std::string(BR_STRING_CTXMENU_OPEN_IMAGE_IN_NEW_WINDOW);
		case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK:
			return std::string(BR_STRING_CTXMENU_SAVE_IMAGE);
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD:
			return std::string(BR_STRING_CTXMENU_COPY_IMAGE);
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_FRAME_IN_NEW_WINDOW:
			return std::string(BR_STRING_CTXMENU_OPEN_FRAME_IN_NEW_WINDOW);
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY:
			return std::string(BR_STRING_CTXMENU_COPY);
		case EWK_CONTEXT_MENU_ITEM_TAG_GO_BACK:
			return std::string(BR_STRING_CTXMENU_GO_BACK);
		case EWK_CONTEXT_MENU_ITEM_TAG_GO_FORWARD:
			return std::string(BR_STRING_CTXMENU_GO_FORWARD);
		case EWK_CONTEXT_MENU_ITEM_TAG_STOP:
			return std::string(BR_STRING_CTXMENU_STOP);
		case EWK_CONTEXT_MENU_ITEM_TAG_RELOAD:
			return std::string(BR_STRING_CTXMENU_RELOAD);
		case EWK_CONTEXT_MENU_ITEM_TAG_CUT:
			return std::string(BR_STRING_CTXMENU_CUT);
		case EWK_CONTEXT_MENU_ITEM_TAG_PASTE:
			return std::string(BR_STRING_CTXMENU_PASTE);
		case EWK_CONTEXT_MENU_ITEM_TAG_NO_GUESSES_FOUND:
			return std::string(BR_STRING_CTXMENU_NO_GUESS_FOUND);
		case EWK_CONTEXT_MENU_ITEM_TAG_IGNORE_SPELLING:
			return std::string(BR_STRING_CTXMENU_IGNORE_SPELLING);
		case EWK_CONTEXT_MENU_ITEM_TAG_LEARN_SPELLING:
			return std::string(BR_STRING_CTXMENU_LEARN_SPELLING);
		case EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_IN_SPOTLIGHT:
			return std::string(BR_STRING_CTXMENU_SEARCH_IN_SPOTLIGHT);
		case EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB:
			return std::string(BR_STRING_CTXMENU_SEARCH_THE_WEB);
		case EWK_CONTEXT_MENU_ITEM_TAG_LOOK_UP_IN_DICTIONARY:
			return std::string(BR_STRING_CTXMENU_LOCK_UP_IN_DIRECTORY);
		case EWK_CONTEXT_MENU_ITEM_TAG_OTHER:
		case EWK_CONTEXT_MENU_ITEM_TAG_SPELLING_GUESS:
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_WITH_DEFAULT_APPLICATION:
		case EWK_CONTEXT_MENU_ITEM_PDFACTUAL_SIZE:
		case EWK_CONTEXT_MENU_ITEM_PDFZOOM_IN:
		case EWK_CONTEXT_MENU_ITEM_PDFZOOM_OUT:
		case EWK_CONTEXT_MENU_ITEM_PDFAUTO_SIZE:
		case EWK_CONTEXT_MENU_ITEM_PDFSINGLE_PAGE:
		case EWK_CONTEXT_MENU_ITEM_PDFCONTINUOUS:
		case EWK_CONTEXT_MENU_ITEM_PDFNEXT_PAGE:
		case EWK_CONTEXT_MENU_ITEM_PDFPREVIOUS_PAGE:
		case EWK_CONTEXT_MENU_ITEM_TAG_SHOW_SPELLING_PANEL:
		case EWK_CONTEXT_MENU_ITEM_TAG_WRITING_DIRECTION_MENU:
		case EWK_CONTEXT_MENU_ITEM_TAG_DEFAULT_DIRECTION:
		case EWK_CONTEXT_MENU_ITEM_TAG_LEFT_TO_RIGHT:
		case EWK_CONTEXT_MENU_ITEM_TAG_RIGHT_TO_LEFT:
		case EWK_CONTEXT_MENU_ITEM_TAG_PDFSINGLE_PAGE_SCROLLING:
		case EWK_CONTEXT_MENU_ITEM_TAG_PDFFACING_PAGES_SCROLLING:
		case EWK_CONTEXT_MENU_ITEM_TAG_TEXT_DIRECTION_MENU:
		case EWK_CONTEXT_MENU_ITEM_TAG_TEXT_DIRECTION_DEFAULT:
		case EWK_CONTEXT_MENU_ITEM_TAG_TEXT_DIRECTION_LEFT_TO_RIGHT:
		case EWK_CONTEXT_MENU_ITEM_TAG_TEXT_DIRECTION_RIGHT_TO_LEFT:
		case EWK_CONTEXT_MENU_ITEM_BASE_CUSTOM_TAG:
		case EWK_CONTEXT_MENU_ITEM_CUSTOM_TAG_NO_ACTION:
		case EWK_CONTEXT_MENU_ITEM_LAST_CUSTOM_TAG:
		case EWK_CONTEXT_MENU_ITEM_BASE_APPLICATION_TAG:
			/* Not spport yet */
			return std::string();
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK:
			return std::string(BR_STRING_CTXMENU_OPEN_LINK);
		case EWK_CONTEXT_MENU_ITEM_TAG_IGNORE_GRAMMAR:
			return std::string(BR_STRING_CTXMENU_IGNORE_GRAMMAR);
		case EWK_CONTEXT_MENU_ITEM_TAG_SPELLING_MENU:
			return std::string(BR_STRING_CTXMENU_SPELLING_AND_GRAMMAR);
		case EWK_CONTEXT_MENU_ITEM_TAG_CHECK_SPELLING:
			return std::string(BR_STRING_CTXMENU_CHECK_DOCUMENT_NOW);
		case EWK_CONTEXT_MENU_ITEM_TAG_CHECK_SPELLING_WHILE_TYPING:
			return std::string(BR_STRING_CTXMENU_CHECK_SPELLING_WHILE_TYPEING);
		case EWK_CONTEXT_MENU_ITEM_TAG_CHECK_GRAMMAR_WITH_SPELLING:
			return std::string(BR_STRING_CTXMENU_CHECK_GRAMMAR_WITH_SPELLING);
		case EWK_CONTEXT_MENU_ITEM_TAG_FONT_MENU:
			return std::string(BR_STRING_CTXMENU_FONT);
		case EWK_CONTEXT_MENU_ITEM_TAG_SHOW_FONTS:
			return std::string(BR_STRING_CTXMENU_SHOW_FONTS);
		case EWK_CONTEXT_MENU_ITEM_TAG_BOLD:
			return std::string(BR_STRING_CTXMENU_BOLD);
		case EWK_CONTEXT_MENU_ITEM_TAG_ITALIC:
			return std::string(BR_STRING_CTXMENU_ITALIC);
		case EWK_CONTEXT_MENU_ITEM_TAG_UNDERLINE:
			return std::string(BR_STRING_CTXMENU_UNDERLINE);
		case EWK_CONTEXT_MENU_ITEM_TAG_OUTLINE:
			return std::string(BR_STRING_CTXMENU_OUTLINE);
		case EWK_CONTEXT_MENU_ITEM_TAG_STYLES:
			return std::string(BR_STRING_CTXMENU_STYLE);
		case EWK_CONTEXT_MENU_ITEM_TAG_SHOW_COLORS:
			return std::string(BR_STRING_CTXMENU_SHOW_COLORS);
		case EWK_CONTEXT_MENU_ITEM_TAG_SPEECH_MENU:
			return std::string(BR_STRING_CTXMENU_SPEECH);
		case EWK_CONTEXT_MENU_ITEM_TAG_START_SPEAKING:
			return std::string(BR_STRING_CTXMENU_START_SPEAKING);
		case EWK_CONTEXT_MENU_ITEM_TAG_STOP_SPEAKING:
			return std::string(BR_STRING_CTXMENU_STOP_SPEAKING);
		case EWK_CONTEXT_MENU_ITEM_TAG_SEND_VIA_EMAIL:
			return std::string(BR_STRING_CTXMENU_SEND_IMAGE_VIA_EMAIL);
		case EWK_CONTEXT_MENU_ITEM_TAG_SEND_VIA_MESSAGE:
			return std::string(BR_STRING_CTXMENU_SEND_IMAGE_VIA_MESSAGE);
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_LOCATION_TO_CLIPBOARD:
			return std::string(BR_STRING_CTXMENU_COPY_IMAGE_LOCATION);
		case EWK_CONTEXT_MENU_ITEM_OPEN_MEDIA_IN_NEW_WINDOW:
			return std::string(BR_STRING_CTXMENU_OPEN_MEDIA_IN_NEW_WINDOW);
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_MEDIA_LINK_TO_CLIPBOARD:
			return std::string(BR_STRING_CTXMENU_COPY_MEDIA);
		case EWK_CONTEXT_MENU_ITEM_TAG_TOGGLE_MEDIA_CONTROLS:
			return std::string(BR_STRING_CTXMENU_TOGGLE_MEDIA_CONTOLS);
		case EWK_CONTEXT_MENU_ITEM_TAG_TOGGLE_MEDIA_LOOP:
			return std::string(BR_STRING_CTXMENU_TOGGLE_MEDIA_LOOP_PLAYBACK);
		case EWK_CONTEXT_MENU_ITEM_TAG_ENTER_VIDEO_FULLSCREEN:
			return std::string(BR_STRING_CTXMENU_SWITCH_VIDEO_TO_FUUL_SCREEN);
		case EWK_CONTEXT_MENU_ITEM_TAG_MEDIA_PLAY_PAUSE:
			return std::string(BR_STRING_CTXMENU_PAUSE);
		case EWK_CONTEXT_MENU_ITEM_TAG_MEDIA_MUTE:
			return std::string(BR_STRING_CTXMENU_MUTE);
		case EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL:
			return std::string(BR_STRING_CTXMENU_SELECT_ALL);
		case EWK_CONTEXT_MENU_ITEM_TAG_SELECT_WORD:
			return std::string(BR_STRING_CTXMENU_SELECT);
		default:
			return std::string();
	}
}

void Browser_Context_Menu::destroy_context_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	for(int i = 0 ; i < m_param_list.size() ; i++) {
		if (m_param_list[i])
			free(m_param_list[i]);
	}
	m_param_list.clear();

	if (m_context_popup) {
		evas_object_del(m_context_popup);
		m_context_popup = NULL;
	}
}

void Browser_Context_Menu::__context_menu_customize_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data || !event_info)
		return;

	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;
	if (!context_menu->m_context_popup)
		return;

	Eina_List *menu_list = (Eina_List *)event_info;

	Eina_List *list = NULL;
	void *item_data = NULL;
	Ewk_Context_Menu_Item *menu_item = NULL;

	if (!context_menu->m_context_menu_data->is_text_selection) {
		BROWSER_LOGD("text is not selected");
		EINA_LIST_FOREACH(menu_list, list, item_data) {
			menu_item = (Ewk_Context_Menu_Item *)item_data;
			if (ewk_context_menu_item_enabled_get(menu_item)
			    && ewk_context_menu_item_type_get(menu_item) == EWK_ACTION_TYPE) {
				Ewk_Context_Menu_Action action = ewk_context_menu_item_action_get(menu_item);
				if (action == EWK_CONTEXT_MENU_ITEM_TAG_COPY
				    || action == EWK_CONTEXT_MENU_ITEM_TAG_RELOAD
				    || action == EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB
				    || action == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK
				    || action == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD)
				continue;
				if (action == EWK_CONTEXT_MENU_ITEM_TAG_SEND_VIA_EMAIL
				    || action == EWK_CONTEXT_MENU_ITEM_TAG_SEND_VIA_MESSAGE)
					continue;

				if (context_menu->m_for_reader_view) {
					if (action == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW
					    || action == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW)
						continue;
				}

				std::string menu_text = context_menu->_get_context_menu_item_text(action);
				if (!menu_text.empty()) {
					context_popup_item_callback_param *param = NULL;
					param = (context_popup_item_callback_param *)malloc(sizeof(context_popup_item_callback_param));
					if (!param) {
						BROWSER_LOGE("malloc failed");
						return;
					}
					context_menu->m_current_context_menu_item = param->menu_item = (Ewk_Context_Menu_Item *)item_data;
					param->user_data = context_menu;
					context_menu->m_param_list.push_back(param);
					BROWSER_LOGD("apend item=[%s]", menu_text.c_str());
					elm_ctxpopup_item_append(context_menu->m_context_popup, menu_text.c_str(),
								NULL, __context_menu_item_selected_cb, param);
				}
				if (action == EWK_CONTEXT_MENU_ITEM_TAG_PASTE) {
					BROWSER_LOGD("apend item=[%s]", BR_STRING_CTXMENU_MORE);
					elm_ctxpopup_item_append(context_menu->m_context_popup, BR_STRING_CTXMENU_MORE,
							NULL, __context_menu_paste_more_item_selected_cb, context_menu);
				}
			}
		}
		evas_object_smart_callback_del(context_menu->m_context_popup, "dismissed",
									__context_popup_dismissed_cb);
		evas_object_smart_callback_add(context_menu->m_context_popup, "dismissed",
									__context_popup_dismissed_cb, context_menu);
	} else {
		BROWSER_LOGD("text is selected");
		if (!(context_menu->m_context_menu_data->hit_test_result_context & EWK_HIT_TEST_RESULT_CONTEXT_IMAGE)) {
			/* If not on image, context popup is copy & paste style. */
			elm_object_style_set(context_menu->m_context_popup, "extended/entry/pass_event");
			elm_ctxpopup_horizontal_set(context_menu->m_context_popup, EINA_TRUE);
			elm_ctxpopup_direction_priority_set(context_menu->m_context_popup,
								ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UP,
								ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UP);
		}

		Eina_Bool search_menu = EINA_FALSE;
		EINA_LIST_FOREACH(menu_list, list, item_data) {
			menu_item = (Ewk_Context_Menu_Item *)item_data;
			if (ewk_context_menu_item_enabled_get(menu_item)
			    && ewk_context_menu_item_type_get(menu_item) == EWK_ACTION_TYPE) {
				Ewk_Context_Menu_Action action = ewk_context_menu_item_action_get(menu_item);
				/* EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB is shown when WebKit decides that selected content
				  * is a word - whitespace characters only selections are ignored */
				if (action == EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB)
					search_menu = EINA_TRUE;
				/* if we call context menu on image in editable (is_text_selection = TRUE) we should
				  * only show context menu related to image, otherwise... */
				if (context_menu->m_context_menu_data->hit_test_result_context
				    & EWK_HIT_TEST_RESULT_CONTEXT_IMAGE) {
					if (action != EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD
					    && action != EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK
					    && action != EWK_CONTEXT_MENU_ITEM_TAG_SEND_VIA_EMAIL
					    && action != EWK_CONTEXT_MENU_ITEM_TAG_SEND_VIA_MESSAGE
					    && action != EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_LOCATION_TO_CLIPBOARD)
						continue;
				} else {
					if (action != EWK_CONTEXT_MENU_ITEM_TAG_COPY
					    && action != EWK_CONTEXT_MENU_ITEM_TAG_CUT
					    && action != EWK_CONTEXT_MENU_ITEM_TAG_PASTE
					    && action != EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL
					    && action != EWK_CONTEXT_MENU_ITEM_TAG_SELECT_WORD)
						continue;
				}

				if (action == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD)
					continue;

				if (action == EWK_CONTEXT_MENU_ITEM_TAG_SEND_VIA_EMAIL
				    || action == EWK_CONTEXT_MENU_ITEM_TAG_SEND_VIA_MESSAGE)
					continue;

				if (context_menu->m_for_reader_view) {
					if (action == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW
					    || action == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW)
						continue;
				}

				std::string menu_text = context_menu->_get_context_menu_item_text(action);
				if (!menu_text.empty()) {
					context_popup_item_callback_param *param = NULL;
					param = (context_popup_item_callback_param *)malloc(sizeof(context_popup_item_callback_param));
					if (!param) {
						BROWSER_LOGE("malloc failed");
						return;
					}
					context_menu->m_current_context_menu_item = param->menu_item = (Ewk_Context_Menu_Item *)item_data;
					param->user_data = context_menu;
					context_menu->m_param_list.push_back(param);
					BROWSER_LOGD("apend item=[%s]", menu_text.c_str());
					elm_ctxpopup_item_append(context_menu->m_context_popup, menu_text.c_str(),
								NULL, __context_menu_item_selected_cb, param);
				}
				if (action == EWK_CONTEXT_MENU_ITEM_TAG_PASTE) {
					BROWSER_LOGD("apend item=[%s]", BR_STRING_CTXMENU_MORE);
					elm_ctxpopup_item_append(context_menu->m_context_popup, BR_STRING_CTXMENU_MORE,
							NULL, __context_menu_paste_more_item_selected_cb, context_menu);
				}
			}
		}

		elm_object_tree_focus_allow_set(context_menu->m_context_popup, EINA_TRUE);
		if (context_menu->m_context_menu_data->hit_test_result_context & EWK_HIT_TEST_RESULT_CONTEXT_IMAGE) {
			evas_object_smart_callback_del(context_menu->m_context_popup, "dismissed",
										__context_popup_dismissed_cb);
			evas_object_smart_callback_add(context_menu->m_context_popup, "dismissed",
										__context_popup_dismissed_cb, context_menu);
		}
	}
}

void Browser_Context_Menu::__context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;
	context_menu->destroy_context_popup();
}

void Browser_Context_Menu::__context_menu_paste_more_item_selected_cb(void *data,
								Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;
	Evas_Object *webview = context_menu->m_webview;
	elm_webview_cbhm_run(webview);
	elm_webview_context_menu_item_selected(webview, NULL);

	context_menu->destroy_context_popup();
}

void Browser_Context_Menu::__context_menu_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	context_popup_item_callback_param *param = (context_popup_item_callback_param *)data;
	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)(param->user_data);
	Ewk_Context_Menu_Item *menu_item = param->menu_item;
	if (!menu_item)
		return;

	context_menu->destroy_context_popup();

	Evas_Object *webview = context_menu->m_webview;
	elm_webview_context_menu_item_selected(webview, menu_item);
}

void Browser_Context_Menu::__context_menu_save_as_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Context_Menu *context_menu = (Browser_Context_Menu *)data;

	const char *image_file_name = (const char *)event_info;
	if (!image_file_name || !strlen(image_file_name)) {
		BROWSER_LOGE("image file name is empty");
		return;
	}

	BROWSER_LOGD("image file name=[%s]", image_file_name);

	if (!context_menu->m_current_context_menu_item) {
		BROWSER_LOGE("context_menu->m_current_context_menu_item is null");
		return;
	}
	Ewk_Context_Menu *ewk_context_menu = ewk_context_menu_item_parent_get(context_menu->m_current_context_menu_item);

	int result = ewk_context_menu_cached_image_save(ewk_context_menu, image_file_name);
	if (!result)
		context_menu->m_browser_view->show_notify_popup(BR_STRING_SAVED, 2, EINA_TRUE);

	context_menu->destroy_context_popup();
}

