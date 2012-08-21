/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include "browser-context-menu.h"
#include "browser-view.h"

Browser_Context_Menu::Browser_Context_Menu(Browser_View *browser_view)
:	m_browser_view(browser_view)
	,m_ewk_view(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Context_Menu::~Browser_Context_Menu(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

void Browser_Context_Menu::init(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	deinit();

	m_ewk_view = ewk_view;

	evas_object_smart_callback_add(ewk_view, "contextmenu,customize", __get_context_menu_from_proposed_context_menu_cb, NULL);
	evas_object_smart_callback_add(ewk_view, "contextmenu,selected", __custom_context_menu_item_selected_cb, this);
}

void Browser_Context_Menu::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_ewk_view) {
		evas_object_smart_callback_del(m_ewk_view, "contextmenu,customize",
						__get_context_menu_from_proposed_context_menu_cb);
		evas_object_smart_callback_del(m_ewk_view, "contextmenu,selected",
						__custom_context_menu_item_selected_cb);
	}
}

void Browser_Context_Menu::__get_context_menu_from_proposed_context_menu_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!event_info)
		return;

	Ewk_Context_Menu *menu = static_cast<Ewk_Context_Menu*>(event_info);
	Ewk_Context_Menu_Item *item;
	Ewk_Context_Menu_Item_Tag tag;
	int count = ewk_context_menu_item_count(menu);
	for (int i = 0; i < count; i++) {
		item = ewk_context_menu_nth_item_get(menu, 0);
		tag = ewk_context_menu_item_tag_get(item);
		ewk_context_menu_item_remove(menu, item);

		switch (tag) {
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_DOWNLOAD_LINKED_FILE, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_OPEN_IMAGE_IN_NEW_WINDOW, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_SAVE_IMAGE_AS, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY_IMAGE, true);
			ewk_context_menu_item_append_as_action(menu, CustomContextMenuItemImageSendViaMessageTag, BR_STRING_SHARE_VIA_MESSAGE, true);
			ewk_context_menu_item_append_as_action(menu, CustomContextMenuItemImageSendViaEmailTag, BR_STRING_SHARE_VIA_EMAIL, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY_IMAGE_LOCATION, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_CUT:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_CUT, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_PASTE:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_PASTE, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_COPY:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_COPY, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_SELECT_ALL, true);
			break;
		case EWK_CONTEXT_MENU_ITEM_TAG_SELECT_WORD:
			ewk_context_menu_item_append_as_action(menu, tag, BR_STRING_CTXMENU_SELECT_WORD, true);
			break;
		default:
			break;
		}
	}
}

void Browser_Context_Menu::__custom_context_menu_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!event_info)
		return;

	Ewk_Context_Menu_Item* item = static_cast<Ewk_Context_Menu_Item*>(event_info);
	Browser_Context_Menu *context_menu = static_cast<Browser_Context_Menu *>(data);

	std::string link_url_string = ewk_context_menu_item_link_url_get(item);
	BROWSER_LOGD("link url=[%s]", link_url_string.c_str());
	std::string image_url_string = ewk_context_menu_item_image_url_get(item);
	BROWSER_LOGD("image url=[%s]", image_url_string.c_str());

	Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
	switch (tag) {

	default:
		break;
	}
}

