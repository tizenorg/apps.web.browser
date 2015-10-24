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

#include <app.h>
#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#include <regex.h>
#endif

#include "bookmark.h"
#include "bookmark-item.h"
#include "bookmark-view.h"
#include "bookmark-add-view.h"
#include "bookmark-create-folder-view.h"
#include "history-view.h"

#include "browser.h"
#include "history.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "platform-service.h"
#include "webview.h"
#include "webview-list.h"

#include <efl_extension.h>
#include <string.h>

typedef enum _item_type {
	ITEM_FOLDER
	,ITEM_BOOKMARK
	,ITEM_UNKNOWN
} item_type;

typedef struct _gl_cb_data {
	item_type type;
	void *user_data;
	void *cp;
	Elm_Object_Item *it;
} gl_cb_data;

#define bookmark_view_edj_path browser_edj_dir"/bookmark-view.edj"
#define browser_genlist_edj_path browser_edj_dir"/browser-genlist.edj"
#define bookmark_edit_view_edj_path browser_edj_dir"/bookmark-edit-view.edj"
#define browser_popup_edj_path browser_edj_dir"/browser-popup.edj"
#define history_view_edj_path browser_edj_dir"/history-view.edj"

#define PATH_WEBSITE_ICON_PNG		browser_data_dir"/website_icon.png"
#define PATH_DEFAULT_ICON_PNG		browser_res_dir"/template/default_application_icon.png"
#define PATH_ICON_NO_FAVICON_PNG	browser_res_dir"/template/internet_no_favicon.png"
#define PATH_TAGBG_ICON_PNG		browser_res_dir"/template/internet_tag_bg.png"

#define WEBSITE_ICON_WIDTH		(64 * efl_scale)
#define WEBSITE_ICON_HEIGHT		(64 * efl_scale)
#define FOLDER_NAME_ENTRY_MAX_COUNT 2048

#define HOMEPAGE_URLEXPR  "((https?|ftp|gopher|telnet|file|notes|ms-help):((//)|(\\\\))[\\w\\d:#@%/;$()~_?+-=\\.&]+)"

bookmark_view::bookmark_view(void)
	:
	m_main_layout(NULL)
	, m_genlist(NULL)
	, m_no_contents_scroller(NULL)
	, m_ctx_popup_more_menu(NULL)
	, m_tabbar(NULL)
	, m_history(NULL)
	, m_bookmarks(NULL)
	, m_naviframe_item(NULL)
	, m_prev_momentum_y(0)
	, m_curr_view_folder_count(0)
	, m_curr_view_editable_bookmark_count(0)
	, m_navigationbar(NULL)
{
	BROWSER_LOGD("");
	m_browser->register_bookmark_listener(this);
	m_bookmark = m_browser->get_bookmark();
	m_bookmark->clear_path_history();
	m_curr_folder_id = m_bookmark->get_root_folder_id();
}

bookmark_view::~bookmark_view(void)
{
	BROWSER_LOGD("");

	if (m_tabbar)
		evas_object_del(m_tabbar);

	if (m_bookmarks)
		evas_object_del(m_bookmarks);

	if (m_history)
		evas_object_del(m_history);

	if (m_no_contents_scroller) {
		Evas_Object *no_contents_layout = elm_object_content_get(m_no_contents_scroller);
		if (no_contents_layout)
			evas_object_del(no_contents_layout);
		evas_object_del(m_no_contents_scroller);
	}

	m_browser->unregister_bookmark_listener(this);
	m_bookmark->destroy_list(m_bookmark_list);
	if (m_ctx_popup_more_menu)
		evas_object_del(m_ctx_popup_more_menu);
	evas_object_smart_callback_del(m_naviframe,
							"transition,finished",
							__naviframe_pop_finished_cb);

	if (m_navigationbar)
		evas_object_del(m_navigationbar);
}
void bookmark_view::_set_focus(void)
{
	BROWSER_LOGD("");
	elm_object_focus_allow_set(m_main_layout, EINA_TRUE);
}

char *bookmark_view::__genlist_get_label_cb(void *data, Evas_Object *obj, const char *part)
{
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_item *item = (bookmark_item *)cb_data->user_data;

	if (!strcmp(part, "elm.text.main.left.top") || !strcmp(part, "elm.text.main.left")) {
		if(item->get_title()) {
			char *markup_title = elm_entry_utf8_to_markup(item->get_title());
			if (markup_title) {
				char *title = strdup(markup_title);
				free(markup_title);
				return title;
			} else
				return strdup(item->get_title());
		} else
			return NULL;
	} else if (!strcmp(part, "elm.text.sub.left.bottom")) {
		if(item->get_uri())
			return strdup(item->get_uri());
		else
			return NULL;
	}

	return NULL;
}

Evas_Object *bookmark_view::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	//BROWSER_LOGD("[%s]", part);
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_item *item = (bookmark_item *)cb_data->user_data;
	bookmark_view *cp = (bookmark_view *)cb_data->cp;
	Evas_Object *content = NULL;

	if (!strcmp(part, "elm.icon.1")) {
		content = elm_layout_add(obj);
		if (item->is_folder()) {
			elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
			Evas_Object *folder_icon = elm_icon_add(obj);
			elm_image_file_set(folder_icon, bookmark_view_edj_path, "contacts_ic_folder.png");
			elm_layout_content_set(content, "elm.swallow.content", folder_icon);
			return content;
		} else {
			elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
			Evas_Object *favicon = cp->m_bookmark->get_favicon(item->get_id(), obj);
			if (!favicon) {
				favicon = elm_icon_add(obj);
				elm_image_file_set(favicon, bookmark_view_edj_path, "internet_ic_default.png");
				elm_layout_content_set(content, "elm.swallow.content", favicon);
				return content;
			}
			elm_layout_content_set(content, "elm.swallow.content", favicon);
			return content;
		}
	}
	return NULL;
}

void bookmark_view::__genlist_cont_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_subitems_clear(it);
}

void bookmark_view::__genlist_exp_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *bv = (bookmark_view *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
	bookmark_item *item_data = (bookmark_item *)cb_data->user_data;
;
	if (item_data->is_folder())
		bv->_set_genlist_by_folder(item_data->get_id(), bv->m_genlist, it);
}

void bookmark_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data) return;
	if (!obj) return;
	if (!event_info) return;
	bookmark_view *cp = (bookmark_view *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
	bookmark_item *item = (bookmark_item *)cb_data->user_data;

	if (item->is_folder()) {
		BROWSER_LOGD("Folder Item clicked[Title:%s]", item->get_title());
		elm_genlist_item_selected_set(it, EINA_FALSE);
		cp->_go_into_sub_folder(item->get_id(), item->get_title());
	} else {
		BROWSER_SECURE_LOGD("Bookmark Item clicked[URL:%s]", item->get_uri());
		webview *wv = m_browser->get_browser_view()->get_current_webview();
		if (!wv) {
			wv = m_browser->get_webview_list()->create_webview(EINA_TRUE);
			wv->set_request_uri(item->get_uri());
			m_browser->get_browser_view()->set_current_webview(wv);
		}
		wv->load_uri(item->get_uri());
		elm_naviframe_item_pop_to(elm_naviframe_bottom_item_get(m_naviframe));
	}
}

void bookmark_view::_delete_ctx_popup(void)
{
	if (m_ctx_popup_more_menu)
		evas_object_del(m_ctx_popup_more_menu);
	m_ctx_popup_more_menu = NULL;
}

Eina_Bool bookmark_view::_is_valid_homepage (const char *uri)
{
	BROWSER_SECURE_LOGD("uri=[%s]", uri);

	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	if (!strcmp(uri, blank_page))
		return EINA_TRUE;

	regex_t regex;
	if (regcomp(&regex, HOMEPAGE_URLEXPR, REG_EXTENDED | REG_ICASE) != 0) {
		BROWSER_LOGD("regcomp failed");
		return EINA_FALSE;
	}

	if (regexec(&regex, uri, 0, NULL, REG_NOTEOL) == 0) {
		BROWSER_LOGD("url expression");
		regfree(&regex);
		if (m_browser->get_browser_view()->is_valid_uri(uri))
			return EINA_TRUE;
		else
			return EINA_FALSE;
	}

	regfree(&regex);
	if (m_browser->get_browser_view()->is_valid_uri(uri))
		return EINA_TRUE;

	return EINA_FALSE;
}

Evas_Object *bookmark_view::_create_genlist(Evas_Object *parent)
{
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, this);
	evas_object_smart_callback_add(genlist, "language,changed", common_view::__genlist_lang_changed, NULL);
	return genlist;
}

void bookmark_view::__genlist_del_cb(void *data, Evas *e, Evas_Object *genlist, void *ei)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!genlist, "genlist is NULL");
	RET_MSG_IF(!data, "data is NULL");

	Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
	while (it) {
		gl_cb_data *item_data = (gl_cb_data *)elm_object_item_data_get(it);
		if (item_data && item_data->user_data) {
			bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;
			BROWSER_LOGD("deleted item: %s", bookmark_item_data->get_title());
			delete bookmark_item_data;
			free(item_data);
		}
		it = elm_genlist_item_next_get(it);
	}
	elm_genlist_clear(genlist);

	Elm_Genlist_Item_Class *itc_folder, *itc_bookmark_folder;
	itc_folder = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_folder");
	itc_bookmark_folder = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_bookmark_folder");

	if (itc_folder) elm_genlist_item_class_free(itc_folder);
	if (itc_bookmark_folder) elm_genlist_item_class_free(itc_bookmark_folder);
}

Eina_Bool bookmark_view::_set_genlist_folder_view(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!genlist, EINA_FALSE, "genlist is NULL");

	m_list_item_count = 0;
	m_operator_genlist_set = EINA_FALSE;

	Elm_Genlist_Item_Class *itc_folder,*itc_bookmark_folder;
	itc_folder = elm_genlist_item_class_new();
	itc_bookmark_folder = elm_genlist_item_class_new();
	if(itc_folder) {
		itc_folder->item_style = "1line.2";
		itc_folder->func.text_get = __genlist_get_label_cb;
		itc_folder->func.content_get = __genlist_icon_get_cb;
		itc_folder->func.state_get = NULL;
		itc_folder->func.del = NULL;
	}

	if (itc_bookmark_folder) {
		itc_bookmark_folder->item_style = "2line.top";
		itc_bookmark_folder->func.text_get = __genlist_get_label_cb;
		itc_bookmark_folder->func.content_get = __genlist_icon_get_cb;
		itc_bookmark_folder->func.state_get = NULL;
		itc_bookmark_folder->func.del = NULL;
	}

	evas_object_data_set(genlist, "itc_folder", itc_folder);
	evas_object_data_set(genlist, "itc_bookmark_folder", itc_bookmark_folder);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_DEL, __genlist_del_cb, this);

	_set_genlist_by_dividing(m_curr_folder_id, genlist, NULL);

	return EINA_TRUE;
}

Eina_Bool bookmark_view::_set_genlist_by_folder(int folder_id,
							Evas_Object *genlist, Elm_Object_Item *parent_item)
{
	BROWSER_LOGD("");
	std::vector<bookmark_item *> bookmark_list;
	Eina_Bool ret;
	Elm_Genlist_Item_Class *itc = NULL;

	ret = m_bookmark->get_list_by_folder(folder_id, bookmark_list);

	if (ret == EINA_FALSE) {
		BROWSER_LOGE("get_list_by_folder is failed(folder id:%d)",folder_id);
		return EINA_FALSE;
	}

	for(unsigned int j=0 ; j < bookmark_list.size() ; j++ ) {
		gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
		if(item_data) {
			memset(item_data, 0x00, sizeof(gl_cb_data));

			bookmark_item *bookmark_item_data = new bookmark_item;
			/* deep copying by overloaded operator = */
			*bookmark_item_data = *bookmark_list[j];
			item_data->cp = this;
			item_data->user_data = (void *)bookmark_item_data;
			if (bookmark_item_data->is_folder()) {
				/* Folder item is found. get sub list */
				itc = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_folder");
				item_data->type = ITEM_FOLDER;
				BROWSER_SECURE_LOGD("Folder[%d] is %s(id: %d)\n", j, bookmark_list[j]->get_title(), bookmark_item_data->get_id());
				item_data->it = elm_genlist_item_append(genlist, itc, item_data, parent_item,
						ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
			} else {
				itc = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_bookmark_folder");
				item_data->type = ITEM_BOOKMARK;
				BROWSER_SECURE_LOGD("bookmark[%d] is %s\n", j, bookmark_list[j]->get_title());
				item_data->it = elm_genlist_item_append(genlist, itc, item_data, parent_item,
						ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
			}
		}
	}
	m_bookmark->destroy_list(bookmark_list);

	return EINA_TRUE;
}

Eina_Bool bookmark_view::_set_genlist_by_dividing(int folder_id,
							Evas_Object *genlist, Elm_Object_Item *parent_item)
{
	BROWSER_LOGD("");
	std::vector<bookmark_item *> bookmark_list;
	Elm_Genlist_Item_Class *itc = NULL;

	int genlist_block_count = elm_genlist_block_count_get(m_genlist);
	m_bookmark->get_list_by_dividing(m_curr_folder_id, bookmark_list, &m_list_item_count, genlist_block_count);

	m_curr_view_folder_count = m_bookmark->_get_folder_count(m_curr_folder_id);
	m_curr_view_editable_bookmark_count = 0;
	for(unsigned int j = 0; j < bookmark_list.size(); j++ ) {
		gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
		if (item_data) {
			memset(item_data, 0x00, sizeof(gl_cb_data));

			bookmark_item *bookmark_item_data = new bookmark_item;
			/* deep copying by overloaded operator = */
			*bookmark_item_data = *bookmark_list[j];
			item_data->cp = this;
			item_data->user_data = (void *)bookmark_item_data;
			if (bookmark_item_data->is_folder()) {
				/* Folder item is found. get sub list */
				itc = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_folder");
				item_data->type = ITEM_FOLDER;
				BROWSER_SECURE_LOGD("Folder[%d] is %s(id: %d)\n", j, bookmark_list[j]->get_title(), bookmark_item_data->get_id());
				item_data->it = elm_genlist_item_append(m_genlist, itc, item_data, NULL,
						ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
			} else {
				if (!bookmark_item_data->is_editable())
					m_curr_view_editable_bookmark_count++;
				itc = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_bookmark_folder");
				item_data->type = ITEM_BOOKMARK;
				BROWSER_SECURE_LOGD("bookmark[%d] is %s\n", j, bookmark_list[j]->get_title());
				item_data->it = elm_genlist_item_append(m_genlist, itc, item_data, NULL,
						ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
			}
		}
	}
	m_list_item_count = m_list_item_count + bookmark_list.size();
	m_bookmark->destroy_list(bookmark_list);

	return EINA_TRUE;
}
void bookmark_view::__scroller_edge_bottom_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	cp->_set_genlist_by_dividing(cp->m_curr_folder_id, cp->m_genlist, NULL);
}

void bookmark_view::_set_view_mode(bookmark_view_type mode)
{
	BROWSER_LOGD("mode=%d", mode);
	Evas_Object *contents = NULL;
	int count_contents = 0;

	if (!m_main_layout)//Safety check, rare case as if the content layout is null , return
		return;

	elm_object_part_content_unset(m_main_layout, "elm.swallow.contents");
	if (m_genlist != NULL) {
		evas_object_del(m_genlist);
		m_genlist = NULL;
	}
	if (m_no_contents_scroller != NULL) {
		evas_object_del(m_no_contents_scroller);
		m_no_contents_scroller = NULL;
	}

	switch (mode) {
	case FOLDER_VIEW_NORMAL:
	{
		contents = m_genlist = _create_genlist(m_main_layout);
		_set_genlist_folder_view(m_genlist);
		count_contents = elm_genlist_items_count(contents);
		evas_object_smart_callback_add(m_genlist, "edge,bottom", __scroller_edge_bottom_cb, this);
		break;
	}
	default:
		break;
	}

	if (count_contents == 0 && m_curr_folder_id == m_bookmark->get_root_folder_id()) {
		evas_object_del(contents);
		contents = NULL;
		m_genlist = NULL;
		if (m_no_contents_scroller) {
			evas_object_del(m_no_contents_scroller);
			m_no_contents_scroller = NULL;
		}
		contents = m_no_contents_scroller = _create_no_content(m_main_layout, BR_STRING_NO_BOOKMARKS, NULL);
		evas_object_smart_callback_add(m_no_contents_scroller, "language,changed", __no_content_lang_changed, this);
	}

	evas_object_show(contents);
	elm_object_part_content_set(m_main_layout, "elm.swallow.contents", contents);
	m_preference->set_bookmark_view_type(mode);
	m_view_mode = mode;
}

void bookmark_view::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (elm_naviframe_bottom_item_get(m_naviframe) !=
						elm_naviframe_top_item_get(m_naviframe))
					elm_naviframe_item_pop(m_naviframe);
}

void bookmark_view::__more_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *bv = (bookmark_view *)data;
	bv->_show_more_context_popup();
}

void bookmark_view::delete_bookmark_more_context_popup(void)
{
	BROWSER_LOGD("");

	if (m_ctx_popup_more_menu) {
		evas_object_del(m_ctx_popup_more_menu);
		m_ctx_popup_more_menu = NULL;
	}
}

void bookmark_view::__bookmark_updated(const char *uri, const char *title, int bookmark_id, int parent_id)
{
	BROWSER_LOGD("");

	switch (m_view_mode) {
		case FOLDER_VIEW_NORMAL:
		{
			Elm_Object_Item *it = elm_genlist_first_item_get(m_genlist);
			while (it) {
				gl_cb_data *item = (gl_cb_data *)elm_object_item_data_get(it);
				bookmark_item *update_item = (bookmark_item*) item->user_data;
				//Check whether the bookmark item already exists
				if (update_item && update_item->get_id() == bookmark_id) {
					//Update title/URI in case the updated bookmark belongs to the current folder id
					if (parent_id == m_curr_folder_id) {
						if (title)
							update_item->set_title(title);
						if (uri)
							update_item->set_uri(uri);
						update_item->set_parent_id(parent_id);
						elm_genlist_item_update(item->it);
					} else {
						//The bookmark item is moved to some folder, so delete the item
						delete update_item;
						free(item);
						elm_object_item_del(it);
					}
					break;
				}
				it = elm_genlist_item_next_get(it);
				//Some other bookmark from other folder moved to the current folder
				if (it == NULL && parent_id == m_curr_folder_id) {
					__bookmark_added(uri, bookmark_id, parent_id);
				}
			}
			break;
		}
		default:
			break;
	}
}

void bookmark_view::__bookmark_added(const char *uri,  int bookmark_id, int parent_id)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!uri, "uri is NULL");

	//if the bookmark added doesnt belong to the current folder, return
	if (m_curr_folder_id != parent_id)
		return;

	bookmark_item *item = new bookmark_item;
	RET_MSG_IF(!item, "item is NULL");

	Elm_Genlist_Item_Class *itc = NULL;
	gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
	if(item_data) {
		memset(item_data, 0x00, sizeof(gl_cb_data));

		m_bookmark->get_item_by_id(bookmark_id, item);

		item_data->cp = this;
		item_data->user_data = (void *)item;

		switch (m_view_mode) {
			case FOLDER_VIEW_NORMAL:
			{
				//This is the first bookmark added, create all views again
				if (!m_genlist) {
					_set_view_mode(m_view_mode);
					free(item_data);
					return;
				}
				itc = (Elm_Genlist_Item_Class *)evas_object_data_get(m_genlist, "itc_bookmark_folder");
				if (item->is_folder()) {
					m_curr_view_folder_count++;
					itc = (Elm_Genlist_Item_Class *)evas_object_data_get(m_genlist, "itc_folder");
					item_data->type = ITEM_FOLDER;
				}
				else
					item_data->type = ITEM_BOOKMARK;
				BROWSER_SECURE_LOGD("bookmark[%s] is %d\n",  item->get_uri(), item->get_id());
				//Add the bookmark item at the top of the list
				item_data->it = elm_genlist_item_prepend(m_genlist, itc, item_data, NULL ,ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
				elm_genlist_item_bring_in(item_data->it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
				elm_genlist_realized_items_update(m_genlist);
				m_list_item_count++;
				break;
			}
			default:
			{
				free(item_data);
				break;
			}
		}
	}
	Evas_Object *snapshot = m_browser->get_history()->get_snapshot(item->get_uri());
	if (snapshot) {
		m_browser->get_bookmark()->set_thumbnail(bookmark_id, snapshot);
		evas_object_del(snapshot);
	}
	// set bookmark favicon from history if there is same URI
	Evas_Object *favicon = m_browser->get_history()->get_history_favicon(item->get_uri());
	if (favicon) {
		m_browser->get_bookmark()->set_favicon(bookmark_id, favicon);
		evas_object_del(favicon);
	}
}

void bookmark_view::__bookmark_removed(const char *uri, int id, int parent_id)
{
	BROWSER_LOGD("");

	//if the bookmark removed doesnt belong to the current folder, return
	if (m_curr_folder_id != parent_id)
		return;

	BROWSER_SECURE_LOGD ("Current ID = %d , Parent id = %d",m_curr_folder_id, parent_id);

	switch (m_view_mode) {
		case FOLDER_VIEW_NORMAL:
		{
			if (elm_genlist_items_count(m_genlist) == 1)
				_set_view_mode(m_view_mode);
			else {
				Elm_Object_Item *it = elm_genlist_first_item_get(m_genlist);
				while (it) {
					gl_cb_data *item = (gl_cb_data *)elm_object_item_data_get(it);
					bookmark_item *del_item = (bookmark_item*) item->user_data;
					if (del_item->get_id() == id) {
						if (del_item->is_folder())
							m_curr_view_folder_count--;
						delete del_item;
						free(item);
						elm_object_item_del(it);
						break;
					}
					it = elm_genlist_item_next_get(it);
				}
			}
			break;
		}
		default:
			break;
	}
}

void bookmark_view::__launch_add_bookmark_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	evas_object_propagate_events_set(cp->m_window, EINA_TRUE);
	evas_object_repeat_events_set(cp->m_window, EINA_TRUE);

	webview *curr_wv = m_browser->get_browser_view()->get_current_webview();
	const char *title = NULL;
	const char *uri = NULL;
	if(curr_wv) {
		title = curr_wv->get_title();
		uri = curr_wv->get_uri();
	}

	if (m_browser->get_bookmark()->is_in_bookmark(uri))
		m_browser->create_bookmark_add_view(title, uri,
				cp->m_curr_folder_id, EINA_TRUE)->show();
	else
		m_browser->create_bookmark_add_view(title, uri,
				cp->m_curr_folder_id, EINA_FALSE)->show();
}

void bookmark_view::__add_bookmark_cb(void *data)
{
	BROWSER_LOGD("");
	if (!data)
		return;
	bookmark_view *cp = (bookmark_view *)data;

	cp->delete_bookmark_more_context_popup();

	switch (cp->m_view_mode) {
	case FOLDER_VIEW_NORMAL:
		evas_object_propagate_events_set(cp->m_window, EINA_FALSE);
		evas_object_repeat_events_set(cp->m_window, EINA_FALSE);
		cp->__launch_add_bookmark_cb(cp, NULL, NULL);
		break;
	default:
		break;
	}
}

void bookmark_view::__refresh_bookmark_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	cp->_set_view_mode(cp->m_view_mode);
}

void bookmark_view::__ctxpopup_add_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;

	switch (cp->m_view_mode) {
	case FOLDER_VIEW_NORMAL:
		evas_object_propagate_events_set(cp->m_window, EINA_FALSE);
		evas_object_repeat_events_set(cp->m_window, EINA_FALSE);
		cp->__launch_add_bookmark_cb(cp, NULL, NULL);
		break;
	default:
		break;
	}
	evas_object_del(obj);
	cp->m_ctx_popup_more_menu = NULL;
}

void bookmark_view::__launch_create_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	evas_object_propagate_events_set(cp->m_window, EINA_TRUE);
	evas_object_repeat_events_set(cp->m_window, EINA_TRUE);
	cp->m_browser->get_bookmark_create_folder_view(NULL, cp, cp->m_curr_folder_id)->show();
}

void bookmark_view::__ctxpopup_create_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;

	switch (cp->m_view_mode) {
	case FOLDER_VIEW_NORMAL:
		evas_object_propagate_events_set(cp->m_window, EINA_FALSE);
		evas_object_repeat_events_set(cp->m_window, EINA_FALSE);
		cp->__launch_create_folder_cb(cp, NULL, NULL);
		break;
	default:
		break;
	}
	evas_object_del(obj);
	cp->m_ctx_popup_more_menu = NULL;
}

void bookmark_view::__launch_reorder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	evas_object_propagate_events_set(cp->m_window, EINA_TRUE);
	evas_object_repeat_events_set(cp->m_window, EINA_TRUE);

	switch (cp->m_view_mode) {
	case FOLDER_VIEW_NORMAL:
		cp->m_browser->create_bookmark_edit_folder_reorder_view(__refresh_bookmark_view_cb, cp, cp->m_curr_folder_id, cp->m_bookmark->get_path_info())->show();
		break;
	default:
		break;
	}
}

void bookmark_view::__ctxpopup_reorder_edit_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	switch (cp->m_view_mode) {
	case FOLDER_VIEW_NORMAL:
		evas_object_propagate_events_set(cp->m_window, EINA_FALSE);
		evas_object_repeat_events_set(cp->m_window, EINA_FALSE);
		cp->__launch_reorder_cb(cp, NULL, NULL);
		break;
	default:
		break;
	}
	evas_object_del(obj);
	cp->m_ctx_popup_more_menu = NULL;
}

void bookmark_view::__launch_move_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	evas_object_propagate_events_set(cp->m_window, EINA_TRUE);
	evas_object_repeat_events_set(cp->m_window, EINA_TRUE);

	switch (cp->m_view_mode) {
	case FOLDER_VIEW_NORMAL:
		cp->m_browser->create_bookmark_edit_folder_move_view(__refresh_bookmark_view_cb, cp, cp->m_curr_folder_id, cp->m_bookmark->get_path_info())->show();
		break;
	default:
		break;
	}
}

void bookmark_view::__ctxpopup_move_edit_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	switch (cp->m_view_mode) {
	case FOLDER_VIEW_NORMAL:
		evas_object_propagate_events_set(cp->m_window, EINA_FALSE);
		evas_object_repeat_events_set(cp->m_window, EINA_FALSE);
		cp->__launch_move_cb(cp, NULL, NULL);
		break;
	default:
		break;
	}
	evas_object_del(obj);
	cp->m_ctx_popup_more_menu = NULL;
}

void bookmark_view::__launch_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	evas_object_propagate_events_set(cp->m_window, EINA_TRUE);
	evas_object_repeat_events_set(cp->m_window, EINA_TRUE);

	switch (cp->m_view_mode) {
	case FOLDER_VIEW_NORMAL:
		cp->m_browser->create_bookmark_edit_folder_delete_view(__refresh_bookmark_view_cb, cp, cp->m_curr_folder_id, cp->m_bookmark->get_path_info())->show();
		break;
	default:
		break;
	}
}

void bookmark_view::__ctxpopup_delete_edit_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	switch (cp->m_view_mode) {
	case FOLDER_VIEW_NORMAL:
		evas_object_propagate_events_set(cp->m_window, EINA_FALSE);
		evas_object_repeat_events_set(cp->m_window, EINA_FALSE);
		cp->__launch_delete_cb(cp, NULL, NULL);
		break;
	default:
		break;
	}
	evas_object_del(obj);
	cp->m_ctx_popup_more_menu = NULL;
}

void bookmark_view::__ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	evas_object_del(obj);
	cp->m_ctx_popup_more_menu = NULL;
}

void bookmark_view::__view_by_popup_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *bv = (bookmark_view *)data;
	bv->destroy_popup(obj);
	bv->_set_view_mode(FOLDER_VIEW_NORMAL);
}

void bookmark_view::__history_clicked_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	bookmark_view *bv = (bookmark_view *)data;
	edje_object_signal_emit(elm_layout_edje_get(bv->m_main_layout), "play,touch_sound,signal", "");

	if (!bv->m_browser->is_history_view_exist())
		bv->m_browser->get_history_view()->show();
}

void bookmark_view::_go_into_sub_folder(int folder_id, const char *folder_name)
{
	BROWSER_LOGD("folder_id: %d", folder_id);
	if (folder_id < 0)
		return;

	m_curr_folder_id = folder_id;
	//m_bookmark->path_into_sub_folder(folder_id, folder_name);
	_go_into_sub_path(m_curr_folder_id);

	_set_view_mode(m_view_mode);
}

void bookmark_view::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	elm_toolbar_item_selected_set(cp->m_bookmarks, EINA_TRUE);
	if (cp->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		return;

	m_browser->delete_history_view_view();
	m_browser->delete_bookmark_add_view();
	m_browser->delete_bookmark_edit_view();
	m_browser->delete_bookmark_create_folder_view();
}

void bookmark_view::_show_more_context_popup(void)
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
	m_ctx_popup_more_menu = more_popup;

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
#endif

	Elm_Object_Item *it = NULL;
	unsigned int count = 0;

	if (m_genlist)
		count = elm_genlist_items_count(m_genlist);

	browser_view *bv = m_browser->get_browser_view();
	RET_MSG_IF(!bv, "browser_view is NULL");

	webview *cur_wv = bv->get_current_webview();
	RET_MSG_IF(!cur_wv, "cur_wv is NULL");

	it = brui_ctxpopup_item_append(more_popup, BR_STRING_ADD_BOOKMARK,
			__ctxpopup_add_cb, NULL,
			NULL, this);

	webview *curr_wv = m_browser->get_browser_view()->get_current_webview();
	RET_MSG_IF(!curr_wv, "curr_wv is NULL");

	const char *uri = NULL;
	if (curr_wv)
		uri = curr_wv->get_uri();
	if (m_browser->get_bookmark()->is_in_bookmark(uri))
		elm_object_item_disabled_set(it, EINA_TRUE);

	BROWSER_SECURE_LOGD("uri : %s", cur_wv->get_uri());
	if (cur_wv->is_file_scheme() || curr_wv->is_data_scheme()) {
		BROWSER_LOGD("file scheme or data scheme");
		elm_object_item_disabled_set(it, EINA_TRUE);
	}

	it = brui_ctxpopup_item_append(more_popup, BR_STRING_CREATE_FOLDER,
			__ctxpopup_create_folder_cb, NULL,
			NULL, this);

	it = brui_ctxpopup_item_append(more_popup, BR_STRING_DELETE,
			__ctxpopup_delete_edit_view_cb, bookmark_view_edj_path,
			"I01_more_popup_icon_delete.png", this);
	//In case of sub folder if item count is one disable the delete.
	if ((count <= 0) || ((count - m_curr_view_editable_bookmark_count) < 1 ))
		elm_object_item_disabled_set(it, EINA_TRUE);

	it = brui_ctxpopup_item_append(more_popup, BR_STRING_REORDER,
			__ctxpopup_reorder_edit_view_cb, bookmark_view_edj_path,
			NULL, this);
	if ((count < 2))
		elm_object_item_disabled_set(it, EINA_TRUE);

	__resize_more_ctxpopup_cb(this);

	evas_object_show(more_popup);
}

void bookmark_view::__no_content_lang_changed(void *data, Evas_Object * obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	bookmark_view *cp = (bookmark_view *)data;

	elm_object_part_text_set(cp->m_no_contents_scroller, "elm.text", BR_STRING_NO_BOOKMARKS);
}

void bookmark_view::hide_bookmark_popups()
{
	_delete_ctx_popup();

	hide_common_view_popups();
}

void bookmark_view::refresh(void)
{
	_set_view_mode(m_view_mode);
}

void bookmark_view::show()
{
	BROWSER_LOGD("");

	m_main_layout = _create_main_layout(m_naviframe, bookmark_view_edj_path);
	_add_tabbar();
	m_view_mode = FOLDER_VIEW_NORMAL;
	if (m_view_mode == FOLDER_VIEW_NORMAL)
		_go_into_sub_folder(m_curr_folder_id, NULL);

	m_naviframe_item = elm_naviframe_item_push(m_naviframe,
			"IDS_BR_BODY_BOOKMARKS", NULL, NULL, m_main_layout, NULL);
	elm_object_item_domain_text_translatable_set(m_naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);
	//elm_naviframe_item_pop_cb_set(m_naviframe_item, __naviframe_pop_cb, this);

	evas_object_smart_callback_add(m_naviframe, "transition,finished",
			__naviframe_pop_finished_cb, this);

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_MORE, __more_cb, this);
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_BACK, __back_cb, this);
#endif

	//_create_navigator();
}

#if 0
void bookmark_view::_create_navigator(void)
{
	BROWSER_LOGD("");

	m_navigationbar = elm_toolbar_add(m_naviframe);
	elm_object_style_set(m_navigationbar, "navigationbar");
	elm_toolbar_shrink_mode_set(m_navigationbar, ELM_TOOLBAR_SHRINK_SCROLL);
	elm_toolbar_transverse_expanded_set(m_navigationbar, EINA_TRUE);
	elm_toolbar_align_set(m_navigationbar, 0.0);
	elm_toolbar_homogeneous_set(m_navigationbar, EINA_FALSE);
	elm_toolbar_select_mode_set(m_navigationbar, ELM_OBJECT_SELECT_MODE_DEFAULT);

	m_path_list.push_back(m_browser->get_bookmark()->get_root_folder_id());
	elm_object_item_part_content_set(m_naviframe_item, "tabbar", m_navigationbar);
}
#endif

void bookmark_view::_add_tabbar()
{
	BROWSER_LOGD("");

	m_tabbar = _create_tabbar(m_main_layout);
	m_bookmarks = elm_toolbar_item_append(m_tabbar, NULL, BR_STRING_BOOKMARKS, NULL, NULL);
	m_history = elm_toolbar_item_append(m_tabbar, NULL, BR_STRING_HISTORY, __history_clicked_cb, this);
	elm_toolbar_item_selected_set(m_bookmarks, EINA_TRUE);
	elm_object_part_content_set(m_main_layout, "elm.swallow.tabbar", m_tabbar);
}

void bookmark_view::_go_into_sub_path(int folder_id)
{
	BROWSER_LOGD("");

	bookmark_item * item = new bookmark_item;
	m_bookmark->get_item_by_id(folder_id, item);

	Elm_Object_Item * toolbar_item = NULL;
	if (folder_id != m_bookmark->get_root_folder_id()) {
		BROWSER_LOGD("%s", item->get_title());
		toolbar_item= elm_toolbar_item_append(m_navigationbar, NULL, item->get_title(), __navigator_clicked_cb, this);
		m_path_list.push_back(folder_id);
		if(toolbar_item!=NULL){
			elm_toolbar_item_bring_in(toolbar_item, ELM_TOOLBAR_ITEM_SCROLLTO_LAST);
		}
	}

	if (item)
		delete item;
}

void bookmark_view::_go_to_clicked_path(Elm_Object_Item *it)
{
	BROWSER_LOGD("");

	Elm_Object_Item *it_last = elm_toolbar_last_item_get(m_navigationbar);

	while(1) {
		Elm_Object_Item *it_prev = elm_toolbar_item_prev_get(it_last);

		if(it == it_last)
			break;

		elm_object_item_del(it_last);
		m_path_list.pop_back();
		it_last = it_prev;
	}
	m_curr_folder_id= m_path_list.back();
}

void bookmark_view::__navigator_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	cp->_go_to_clicked_path(it);
	cp->_set_view_mode(cp->m_view_mode);
}

void bookmark_view::rotate(void)
{
	__resize_more_ctxpopup_cb(this);
}

Eina_Bool bookmark_view::__resize_more_ctxpopup_cb(void *data)
{
	bookmark_view *bv = (bookmark_view *)data;
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

void bookmark_view::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *item = (Elm_Object_Item*)event_info;
	gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(item);
	bookmark_item *bookmark_it = (bookmark_item *)cb_data->user_data;
	std::string access_msg;
	if (cb_data->type == ITEM_BOOKMARK || cb_data->type == ITEM_FOLDER) {
		if (bookmark_it->get_title())
			access_msg = bookmark_it->get_title();
		else
			return;

		if (bookmark_it->is_folder())
			access_msg = access_msg + " " + BR_STRING_DOUBLE_TAP_TO_OPEN_THE_FOLDER_T;
		else
			access_msg = access_msg + " " + BR_STRING_ACCESS_DOUBLE_TAP_TO_OPEN_WEBPAGE_T;
	} else
		return;

	Evas_Object *item_access_obj = elm_object_item_access_object_get(item);
	if (item_access_obj) {
		elm_access_info_set(item_access_obj, ELM_ACCESS_INFO, access_msg.c_str());
	}
}

