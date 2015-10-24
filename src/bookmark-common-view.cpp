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
 * Contact: Sangpyo Kim <sangpyo7.kim@samsung.com>
 *
 */

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"

#include "bookmark-common-view.h"
#include "bookmark.h"
#include "bookmark-item.h"
#include "bookmark-create-folder-view.h"
#include "platform-service.h"
#include "bookmark-add-view.h" //for temporary fix

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

bookmark *bookmark_common_view::m_bookmark;

#define browser_popup_edj_path browser_edj_dir"/browser-popup.edj"

#define bookmark_view_edj_path browser_edj_dir"/bookmark-view.edj"

bookmark_common_view::bookmark_common_view(void)
:
	m_popup_select_folder(NULL)
	,m_last_folder_item(NULL)
{
	m_bookmark = m_browser->get_bookmark();
}

bookmark_common_view::~bookmark_common_view(void)
{
	BROWSER_LOGD("");
	if (m_popup_select_folder)
		evas_object_del(m_popup_select_folder);
}

void bookmark_common_view::on_pause(void)
{
	BROWSER_LOGD("");
	common_view::on_pause();
}

char *bookmark_common_view::__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, NULL, "data is NULL");

	gl_common_cb_data *callback_data = (gl_common_cb_data *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)callback_data->user_data;

	if (!strcmp(part, "elm.text")) {
		if (bookmark_item_data->get_id() == m_browser->get_bookmark()->get_root_folder_id())
			return strdup(BR_STRING_BOOKMARKS);
		else
			return strdup(bookmark_item_data->get_title());
	}
	return NULL;
}

Evas_Object *bookmark_common_view::__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("");
	BROWSER_LOGD("[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
	RETV_MSG_IF(!obj, NULL, "obj is NULL");
	gl_common_cb_data *callback_data = (gl_common_cb_data *)data;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.edit")) {
		Evas_Object *folder_icon = elm_icon_add(obj);
		if (folder_icon == NULL)
			return NULL;
		elm_image_file_set(folder_icon, browser_img_dir"/U01_icon_folder_default.png", NULL);
		evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return folder_icon;
	}
	return NULL;
}

void bookmark_common_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	gl_common_cb_data *callback_data = (gl_common_cb_data *)elm_object_item_data_get((Elm_Object_Item *)event_info);
	bookmark_common_view *cp = (bookmark_common_view *)data;

	elm_genlist_item_selected_set(it, EINA_FALSE);

	if (cp->m_popup_select_folder) {
		evas_object_del(cp->m_popup_select_folder);
		cp->m_popup_select_folder = NULL;
	}
	elm_object_tree_focus_allow_set(m_naviframe, EINA_TRUE);
}

void bookmark_common_view::__genlist_del_cb(void *data, Evas *e, Evas_Object *genlist, void *ei)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!genlist, "genlist is NULL");

	Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
	while (it) {
		elm_genlist_item_expanded_set(it, EINA_TRUE);
		gl_common_cb_data *item_data = (gl_common_cb_data *)elm_object_item_data_get(it);
		bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;
		delete bookmark_item_data;
		free(item_data);
		it = elm_genlist_item_next_get(it);
	}
	elm_genlist_clear(genlist);

	Elm_Genlist_Item_Class *itc_folder = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_folder");
	if (itc_folder) elm_genlist_item_class_free(itc_folder);
}

