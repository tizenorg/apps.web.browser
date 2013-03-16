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

#include "bookmark.h"
#include "bookmark-item.h"
#include "bookmark-add-view.h"
#include "bookmark-select-folder-view.h"
#include "bookmark-create-folder-view.h"
#include "bookmark-create-folder-save-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"

bookmark_create_folder_view::bookmark_create_folder_view(Evas_Smart_Cb cb_func, void *cb_data)
:
	m_folder_genlist(NULL)
	,m_titlebar_btn_create_folder(NULL)
	,m_naviframe_item(NULL)
	,m_itc_folder(NULL)
	,m_cb_func(cb_func)
	,m_cb_data(cb_data)
{
	BROWSER_LOGD("");
	m_bookmark = m_browser->get_bookmark();
}

bookmark_create_folder_view::~bookmark_create_folder_view(void)
{
	BROWSER_LOGD("");

	evas_object_smart_callback_del(m_naviframe,
						"transition,finished",
						__naviframe_pop_finished_cb);
	if (m_itc_folder)
		elm_genlist_item_class_free(m_itc_folder);
}

char *bookmark_create_folder_view::__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	gl_cb_data *callback_data = (gl_cb_data *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)callback_data->user_data;

	if (!strcmp(part, "elm.text")) {
		if (!strcmp(bookmark_item_data->get_title(), "Bookmarks")) {
			BROWSER_LOGD("[%s][%s] is added", part, BR_STRING_MOBILE);
			return elm_entry_utf8_to_markup(BR_STRING_MOBILE);
		} else {
			BROWSER_LOGD("[%s][%s] is added", part, bookmark_item_data->get_title());
			return elm_entry_utf8_to_markup(bookmark_item_data->get_title());
		}
	}
	return NULL;
}

Evas_Object *bookmark_create_folder_view::__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
	gl_cb_data *callback_data = (gl_cb_data *)data;
	Eina_Bool expanded = EINA_FALSE;

	if (callback_data->it) {
		expanded = elm_genlist_item_expanded_get(callback_data->it);
	}

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.edit")) {
		Evas_Object *folder_icon = elm_icon_add(obj);
		if (folder_icon == NULL)
			return NULL;
		if (expanded) {
			elm_icon_standard_set(folder_icon, browser_img_dir"/bookmark_icon_folder_open.png");
		} else {
			elm_icon_standard_set(folder_icon, browser_img_dir"/bookmark_icon_folder.png");
		}
		evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return folder_icon;
	}
	return NULL;
}

void bookmark_create_folder_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	gl_cb_data *callback_data = (gl_cb_data *)elm_object_item_data_get((Elm_Object_Item *)event_info);
	bookmark_item *bookmark_item_data = (bookmark_item *)callback_data->user_data;
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	elm_genlist_item_selected_set(it, EINA_FALSE);

	cp->m_browser->get_bookmark_create_folder_save_view(cp->m_cb_func, cp->m_cb_data, bookmark_item_data->get_id())->show();
}

void bookmark_create_folder_view::__create_folder_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
}

void bookmark_create_folder_view::__back_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	cp->_back_to_previous_view();
}

void bookmark_create_folder_view::__naviframe_pop_finished_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_create_folder_view *cp = (bookmark_create_folder_view *)data;

	if (cp->m_naviframe_item != elm_naviframe_top_item_get(cp->m_naviframe))
		return;

	cp->m_browser->delete_bookmark_create_folder_save_view();
}

void bookmark_create_folder_view::_back_to_previous_view(void)
{
	BROWSER_LOGD("");
	if (elm_naviframe_bottom_item_get(m_naviframe) !=
						elm_naviframe_top_item_get(m_naviframe))
					elm_naviframe_item_pop(m_naviframe);
}

void bookmark_create_folder_view::_clear_genlist_item_data(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(genlist);

	Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
	while (it) {
		elm_genlist_item_expanded_set(it, EINA_TRUE);
		gl_cb_data *item_data = (gl_cb_data *)elm_object_item_data_get(it);
		bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;
		delete bookmark_item_data;
		free(item_data);
		it = elm_genlist_item_next_get(it);
	}
}

Eina_Bool bookmark_create_folder_view::_set_genlist_item_by_folder(Evas_Object *genlist, int folder_id, Elm_Object_Item *parent_it)
{
	BROWSER_LOGD("folder_id: %d", folder_id);
	std::vector<bookmark_item *> bookmark_list;
	Eina_Bool ret;

	ret = m_bookmark->get_list_by_folder(folder_id, bookmark_list);
	if (ret == EINA_FALSE) {
		BROWSER_LOGE("get_list_by_folder is failed(folder id:%d)",folder_id);
		return EINA_FALSE;
	}

	for(unsigned int j = 0 ; j < bookmark_list.size() ; j++ ) {
		if (bookmark_list[j]->is_folder()) {
			gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
			memset(item_data, 0x00, sizeof(gl_cb_data));

			bookmark_item *bookmark_item_data = new bookmark_item;
			/* deep copying by overloaded operator = */
			*bookmark_item_data = *bookmark_list[j];
			/* Folder item is found. get sub list */
			BROWSER_LOGD("Folder[%d] is %s(id: %d)\n", j,
					bookmark_list[j]->get_title(),
					bookmark_item_data->get_id());
			item_data->cp = this;
			item_data->user_data = (void *)bookmark_item_data;
			item_data->it = elm_genlist_item_append(genlist, m_itc_folder, item_data, parent_it,
					ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, this);
		}
	}
	m_bookmark->destroy_list(bookmark_list);
	return EINA_TRUE;
}

Eina_Bool bookmark_create_folder_view::_set_genlist_folder_tree(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(genlist, EINA_FALSE);

	_clear_genlist_item_data(genlist);
	elm_genlist_clear(genlist);
	elm_object_style_set(genlist, "handler");

	m_itc_folder = elm_genlist_item_class_new();
	m_itc_folder->item_style = "1text.1icon.2";
	m_itc_folder->func.text_get = __genlist_get_text_cb;
	m_itc_folder->func.content_get = __genlist_get_content_cb;
	m_itc_folder->func.state_get = NULL;
	m_itc_folder->func.del = NULL;

	int depth_count = 0;
	m_bookmark->get_folder_depth_count(&depth_count);
	BROWSER_LOGD("Final depth_count: %d", depth_count);

	for (int i=0 ; i <= depth_count ; i++) {
		BROWSER_LOGD("current depth: %d", i);
		if (i == 0 ) {
			BROWSER_LOGD("ROOT folder items are set", i);
			/* root folder */
			_set_genlist_item_by_folder(genlist, 0, NULL);
		} else {
			BROWSER_LOGD("SUB folder items are set", i);
			/* sub folder*/
			Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
			while (it) {
				Eina_Bool expanded = EINA_FALSE;
				expanded = elm_genlist_item_expanded_get(it);
				if (expanded == EINA_FALSE) {
					elm_genlist_item_expanded_set(it, EINA_TRUE);
					gl_cb_data *item_data = (gl_cb_data *)elm_object_item_data_get(it);
					bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;

					_set_genlist_item_by_folder(genlist, bookmark_item_data->get_id(), item_data->it);
				}
				it = elm_genlist_item_next_get(it);
			}
		}
	}

	return EINA_TRUE;
}

Evas_Object *bookmark_create_folder_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	_set_genlist_folder_tree(genlist);

	return genlist;
}

void bookmark_create_folder_view::refresh()
{
	BROWSER_LOGD("");
	if (m_folder_genlist)
		_set_genlist_folder_tree(m_folder_genlist);
}

void bookmark_create_folder_view::show()
{
	BROWSER_LOGD("");
	m_folder_genlist = _create_genlist(m_naviframe);

	m_naviframe_item = elm_naviframe_item_push(m_naviframe,
						BR_STRING_CREATE_FOLDER_IN, NULL, NULL, m_folder_genlist, NULL);

	evas_object_smart_callback_add(m_naviframe,
					"transition,finished",
					__naviframe_pop_finished_cb, this);
}
