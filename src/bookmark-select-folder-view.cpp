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

#include "bookmark.h"
#include "bookmark-item.h"
#include "bookmark-add-view.h"
#include "bookmark-select-folder-view.h"
#include "bookmark-create-folder-view.h"
#include "platform-service.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

#define bookmark_view_edj_path browser_edj_dir"/bookmark-view.edj"

bookmark_select_folder_view::bookmark_select_folder_view(Evas_Smart_Cb cb_func, void *cb_data)
:
	m_folder_genlist(NULL)
	,m_ctx_popup_more_menu(NULL)
	,m_naviframe_item(NULL)
	,m_itc_folder(NULL)
	,m_cb_func(cb_func)
	,m_cb_data(cb_data)

{
	BROWSER_LOGD("");
	m_bookmark = m_browser->get_bookmark();
	m_browser->register_bookmark_listener(this);
}

bookmark_select_folder_view::~bookmark_select_folder_view(void)
{
	BROWSER_LOGD("");
	m_browser->unregister_bookmark_listener(this);

	evas_object_smart_callback_del(m_naviframe,
						"transition,finished",
						__naviframe_pop_finished_cb);
	if (m_itc_folder)
		elm_genlist_item_class_free(m_itc_folder);

	if (m_ctx_popup_more_menu)
	{
		evas_object_smart_callback_del(elm_object_top_widget_get(m_ctx_popup_more_menu), "rotation,changed", rotate_ctxpopup_cb);
		evas_object_del(m_ctx_popup_more_menu);
	}
}

char *bookmark_select_folder_view::__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	gl_cb_data *callback_data = (gl_cb_data *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)callback_data->user_data;

	if (!strcmp(part, "elm.text.main.left")) {
		if (bookmark_item_data->get_id() == m_browser->get_bookmark()->get_root_folder_id()) {
			return strdup(BR_STRING_BOOKMARKS);
		} else {
			return strdup(bookmark_item_data->get_title());
		}
	}
	return NULL;
}

Evas_Object *bookmark_select_folder_view::__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
	RETV_MSG_IF(!obj, NULL, "obj is NULL");
	gl_cb_data *callback_data = (gl_cb_data *)data;
	if (!strcmp(part, "elm.icon.left")) {
		Evas_Object *content = elm_layout_add(obj);
		if (content == NULL)
			return NULL;
		elm_layout_theme_set(content, "layout", "list/B/type.2", "default");
		Evas_Object *folder_icon = elm_icon_add(content);
		if (folder_icon == NULL)
			return NULL;
		elm_image_file_set(folder_icon, bookmark_view_edj_path, "contacts_ic_folder.png");
		evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_layout_content_set(content, "elm.swallow.content", folder_icon);
		return folder_icon;
	}

	return NULL;
}

void bookmark_select_folder_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	gl_cb_data *callback_data = (gl_cb_data *)elm_object_item_data_get((Elm_Object_Item *)event_info);
	bookmark_item *bookmark_item_data = (bookmark_item *)callback_data->user_data;
	bookmark_select_folder_view *cp = (bookmark_select_folder_view *)data;

	elm_genlist_item_selected_set(it, EINA_FALSE);

	if (cp->m_cb_func) {
		cp->m_cb_func(cp->m_cb_data, NULL, bookmark_item_data);
	}

	cp->_back_to_previous_view();
}

void bookmark_select_folder_view::__create_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_select_folder_view *cp = (bookmark_select_folder_view *)data;

	if (cp->m_folder_genlist)
		cp->_set_genlist_folder_tree(cp->m_folder_genlist);
}

void bookmark_select_folder_view::__back_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_select_folder_view *cp = (bookmark_select_folder_view *)data;

	if (cp->m_cb_func) {
		cp->m_cb_func(cp->m_cb_data, NULL, NULL);
	}

	cp->_back_to_previous_view();
}

void bookmark_select_folder_view::__naviframe_pop_finished_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_select_folder_view *cp = (bookmark_select_folder_view *)data;

	if (cp->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		return;

	m_browser->delete_bookmark_create_folder_view();
}

void bookmark_select_folder_view::_back_to_previous_view(void)
{
	BROWSER_LOGD("");
	if (elm_naviframe_bottom_item_get(m_naviframe) !=
						elm_naviframe_top_item_get(m_naviframe))
					elm_naviframe_item_pop(m_naviframe);
}

void bookmark_select_folder_view::_clear_genlist_item_data(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!genlist, "genlist is NULL");

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

Eina_Bool bookmark_select_folder_view::_set_genlist_item_by_folder(Evas_Object *genlist, int folder_id, Elm_Object_Item *parent_it)
{
	BROWSER_LOGD("folder_id: %d", folder_id);
	std::vector<bookmark_item *> bookmark_list;
	Eina_Bool ret;

	if (folder_id < 0) {
		//for root folder only
		gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
		if(item_data) {
			memset(item_data, 0x00, sizeof(gl_cb_data));

			bookmark_item *bookmark_item_data = new bookmark_item;
			/* deep copying by overloaded operator = */
			bookmark_item_data->set_id(0);
			bookmark_item_data->set_folder_flag(EINA_TRUE);
			bookmark_item_data->set_parent_id(-1);
			bookmark_item_data->set_title("Bookmarks");
			/* Folder item is found. get sub list */
			BROWSER_SECURE_LOGD("ROOT folder is %s(id: %d)",
			bookmark_item_data->get_title(),
			bookmark_item_data->get_id());
			item_data->cp = this;
			item_data->user_data = (void *)bookmark_item_data;
			item_data->it = elm_genlist_item_append(genlist, m_itc_folder, item_data, parent_it,
			ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, this);
		}
		return EINA_TRUE;
	}

	ret = m_bookmark->get_list_by_folder(folder_id, bookmark_list);
	if (ret == EINA_FALSE) {
		BROWSER_LOGE("get_list_by_folder is failed(folder id:%d)",folder_id);
		return EINA_FALSE;
	}

	for(unsigned int j = 0 ; j < bookmark_list.size() ; j++ ) {
		if (bookmark_list[j]->is_folder()) {
			gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
			if(item_data) {
				memset(item_data, 0x00, sizeof(gl_cb_data));

				bookmark_item *bookmark_item_data = new bookmark_item;
				/* deep copying by overloaded operator = */
				*bookmark_item_data = *bookmark_list[j];
				/* Folder item is found. get sub list */
				BROWSER_SECURE_LOGD("Folder[%d] is %s(id: %d)\n", j,
				bookmark_list[j]->get_title(),
				bookmark_item_data->get_id());
				item_data->cp = this;
				item_data->user_data = (void *)bookmark_item_data;
				item_data->it = elm_genlist_item_append(genlist, m_itc_folder, item_data, parent_it,
				ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, this);
			}
		}
	}
	m_bookmark->destroy_list(bookmark_list);
	return EINA_TRUE;
}

Eina_Bool bookmark_select_folder_view::_set_genlist_folder_tree(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!genlist, EINA_FALSE, "genlist is NULL");

	_clear_genlist_item_data(genlist);
	elm_genlist_clear(genlist);

	m_itc_folder = elm_genlist_item_class_new();
	if(!m_itc_folder)
		return EINA_FALSE;
	m_itc_folder->item_style = "1line";
	m_itc_folder->func.text_get = __genlist_get_text_cb;
	m_itc_folder->func.content_get = __genlist_get_content_cb;
	m_itc_folder->func.state_get = NULL;
	m_itc_folder->func.del = NULL;

	int depth_count = 0;
	m_bookmark->get_folder_depth_count(&depth_count);
	BROWSER_LOGD("Final depth_count: %d", depth_count);
	depth_count = depth_count + 1; //increase count for virtual root folder

	for (int i=0 ; i <= depth_count ; i++) {
		BROWSER_LOGD("current depth: %d", i);
		if (i == 0 ) {
			BROWSER_LOGD("ROOT folder item is set", i);
			/* root folder */
			_set_genlist_item_by_folder(genlist, -1, NULL);
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

Evas_Object *bookmark_select_folder_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	_set_genlist_folder_tree(genlist);

	return genlist;
}

void bookmark_select_folder_view::refresh()
{
	BROWSER_LOGD("");
	if (m_folder_genlist)
		_set_genlist_folder_tree(m_folder_genlist);
}

void bookmark_select_folder_view::__bookmark_removed(const char *uri, int bookmark_id, int parent_id){}
void bookmark_select_folder_view::__bookmark_updated(const char *uri, const char *title, int bookmark_id, int parent_id){}
void bookmark_select_folder_view::__bookmark_added(const char *uri,  int bookmark_id, int parent_id)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!uri, "uri is NULL");

	Elm_Object_Item *parent_it = NULL;
	Elm_Object_Item *it = elm_genlist_first_item_get(m_folder_genlist);
	while (it) {
		gl_cb_data *item_data = (gl_cb_data *)elm_object_item_data_get(it);
		bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;
		if (bookmark_item_data->get_id() == parent_id) {
			parent_it = it;
			break;
		}
		it = elm_genlist_item_next_get(it);
	}

	bookmark_item *item = new bookmark_item;
	gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
	if(!item_data){
		delete item;
		return;
	}
	memset(item_data, 0x00, sizeof(gl_cb_data));

	m_bookmark->get_item_by_id(bookmark_id, item);

	item_data->cp = this;
	item_data->user_data = (void *)item;
	BROWSER_SECURE_LOGD("bookmark[%s] is %d\n",  item->get_uri(), item->get_id());
	//Add the bookmark item at the top of the list
	unsigned int count = 0;
	if (m_folder_genlist)
		count = elm_genlist_items_count(m_folder_genlist);

	if (m_bookmark->get_root_folder_id() == parent_id && count > 1)
		item_data->it = elm_genlist_item_prepend(m_folder_genlist, m_itc_folder, item_data, parent_it ,ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, this);
	else
		item_data->it = elm_genlist_item_append(m_folder_genlist, m_itc_folder, item_data, parent_it ,ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, this);
	elm_genlist_item_bring_in(item_data->it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
	elm_genlist_realized_items_update(m_folder_genlist);
}

void bookmark_select_folder_view::rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	bookmark_select_folder_view *bv = (bookmark_select_folder_view *)data;
	__resize_more_ctxpopup_cb(bv);
}

Eina_Bool bookmark_select_folder_view::__resize_more_ctxpopup_cb(void *data)
{
	RETV_MSG_IF(!(data), EINA_FALSE, "data is NULL");
	bookmark_select_folder_view *bv = (bookmark_select_folder_view *)data;
	if (!bv->m_ctx_popup_more_menu)
		return ECORE_CALLBACK_CANCEL;

	BROWSER_LOGD("");

	// Move ctxpopup to proper position.
	int x = 0;
	int y = 0;
	platform_service ps;
	ps.get_more_ctxpopup_position(&x, &y);
	evas_object_move(bv->m_ctx_popup_more_menu, x, y);

	return ECORE_CALLBACK_CANCEL;
}

void bookmark_select_folder_view::__ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	bookmark_select_folder_view *cp = (bookmark_select_folder_view *)data;
	evas_object_smart_callback_del(elm_object_top_widget_get(cp->m_ctx_popup_more_menu), "rotation,changed", rotate_ctxpopup_cb);
	evas_object_del(cp->m_ctx_popup_more_menu);
	cp->m_ctx_popup_more_menu = NULL;
}

void bookmark_select_folder_view::__launch_create_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	bookmark_select_folder_view *cp = (bookmark_select_folder_view *)data;
	cp->m_browser->get_bookmark_create_folder_view(NULL, cp, cp->m_bookmark->get_root_folder_id())->show();
}

void bookmark_select_folder_view::__ctxpopup_create_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_select_folder_view *cp = (bookmark_select_folder_view *)data;
	cp->__launch_create_folder_cb(cp, NULL, NULL);
	evas_object_smart_callback_del(elm_object_top_widget_get(cp->m_ctx_popup_more_menu), "rotation,changed", rotate_ctxpopup_cb);
	evas_object_del(obj);
	cp->m_ctx_popup_more_menu = NULL;
}

void bookmark_select_folder_view::_show_more_context_popup(void)
{
	BROWSER_LOGD("");
	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}

	elm_object_style_set(more_popup, "more/default");
	elm_ctxpopup_direction_priority_set(more_popup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", __ctxpopup_dismissed_cb, this);
	elm_ctxpopup_auto_hide_disabled_set(more_popup, EINA_TRUE);
	evas_object_smart_callback_add(elm_object_top_widget_get(more_popup), "rotation,changed", rotate_ctxpopup_cb, this);
	m_ctx_popup_more_menu = more_popup;

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
#endif

	brui_ctxpopup_item_append(more_popup, BR_STRING_CREATE_FOLDER,
			__ctxpopup_create_folder_cb, NULL,
			NULL, this);

	__resize_more_ctxpopup_cb(this);

	evas_object_show(more_popup);
}

void bookmark_select_folder_view::__more_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	bookmark_select_folder_view *bv = (bookmark_select_folder_view *)data;
	bv->_show_more_context_popup();
}

void bookmark_select_folder_view::show()
{
	BROWSER_LOGD("");
	m_folder_genlist = _create_genlist(m_naviframe);

	m_naviframe_item = elm_naviframe_item_push(m_naviframe,
						"IDS_BR_BODY_SELECT_FOLDER", NULL, NULL, m_folder_genlist, NULL);//BR_STRING_SELECT_FOLDER

	elm_object_item_domain_text_translatable_set(m_naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);

	evas_object_smart_callback_add(m_naviframe,
					"transition,finished",
					__naviframe_pop_finished_cb, this);
#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(m_folder_genlist, EEXT_CALLBACK_MORE, __more_cb, this);
	eext_object_event_callback_add(m_folder_genlist, EEXT_CALLBACK_BACK, __back_btn_clicked_cb, this);
#endif
}
