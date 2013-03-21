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

#include <app.h>

#include "bookmark.h"
#include "bookmark-item.h"
#include "bookmark-view.h"
#include "bookmark-add-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "multiwindow-view.h"
#include "webview.h"
#include "platform-service.h"

typedef struct _gl_cb_data {
	void *user_data;
	void *cp;
	Elm_Object_Item *it;
} gl_cb_data;

#define THUMBNAIL_ITEM_W	((206 + 26) * efl_scale)
#define THUMBNAIL_ITEM_H	((190 + 10 + 30) * efl_scale)

#define bookmark_view_edj_path browser_edj_dir"/bookmark-view.edj"
#define browser_genlist_edj_path browser_edj_dir"/browser-genlist.edj"

bookmark_view::bookmark_view(void)
:
	m_gesture_layer(NULL)
	,m_toolbar_layout(NULL)
	,m_box(NULL)
	,m_contents_layout(NULL)
	,m_genlist(NULL)
#if defined(BROWSER_THUMBNAIL_VIEW)
	,m_gengrid(NULL)
#endif
{
	BROWSER_LOGD("");
	elm_theme_extension_add(NULL, bookmark_view_edj_path);
	elm_theme_extension_add(NULL, browser_genlist_edj_path);

	m_bookmark = m_browser->get_bookmark();
	m_curr_folder_id = m_bookmark->get_root_folder_id();
	m_main_layout = _create_main_layout(m_window);

	m_toolbar_layout = _create_toolbar_layout(m_window);
}

bookmark_view::~bookmark_view(void)
{
	BROWSER_LOGD("");
	m_bookmark->destroy_list(m_bookmark_list);

	if (m_genlist) {
		_clear_genlist_item_data(m_genlist, FOLDER_VIEW_NORMAL);
		evas_object_del(m_genlist);
	}
#if defined(BROWSER_THUMBNAIL_VIEW)
	if (m_gengrid) {
		_clear_genlist_item_data(m_gengrid, THUMBNAIL_VIEW_NORMAL);
		evas_object_del(m_gengrid);
	}
#endif

	if (m_toolbar_layout)
		evas_object_del(m_toolbar_layout);

	if (m_gesture_layer)
		evas_object_del(m_gesture_layer);

	elm_theme_extension_del(NULL, bookmark_view_edj_path);
	elm_theme_extension_del(NULL, browser_genlist_edj_path);
}

char *bookmark_view::__genlist_get_label_cb(void *data, Evas_Object *obj, const char *part)
{
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_item *item = (bookmark_item *)cb_data->user_data;

	if (!strcmp(part, "elm.text"))
		return strdup(item->get_title());

	return NULL;
}

Evas_Object *bookmark_view::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_item *item = (bookmark_item *)cb_data->user_data;
	bookmark_view *cp = (bookmark_view *)cb_data->cp;

	if (!strcmp(part, "elm.icon")) {
		if (item->is_folder()) {
			Evas_Object *folder_icon = elm_icon_add(obj);
			Eina_Bool expanded = elm_genlist_item_expanded_get(cb_data->it);
			BROWSER_LOGD("[%s]%s :%d", part, item->get_title(), expanded);
			if (expanded)
				elm_icon_standard_set(folder_icon, browser_img_dir"/bookmark_icon_folder_open.png");
			else
				elm_icon_standard_set(folder_icon, browser_img_dir"/bookmark_icon_folder.png");
			evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return folder_icon;
		} else {
			Evas_Object *favicon = m_webview_context->get_favicon(item->get_uri());
			if (!favicon) {
				favicon = elm_icon_add(obj);
				elm_icon_standard_set(favicon, browser_img_dir"/I01_icon_default_internet.png");
				evas_object_size_hint_aspect_set(favicon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			}
			return favicon;
		}
	} else if (!strcmp(part, "elm.icon.edit")) {
		if (item->is_folder()) {
			Evas_Object *folder_icon = elm_icon_add(obj);
			elm_icon_standard_set(folder_icon, browser_img_dir"/bookmark_icon_folder.png");
			evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return folder_icon;
		} else {
			Evas_Object *favicon = m_webview_context->get_favicon(item->get_uri());
			if (!favicon) {
				favicon = elm_icon_add(obj);
				elm_icon_standard_set(favicon, browser_img_dir"/I01_icon_default_internet.png");
				evas_object_size_hint_aspect_set(favicon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			}
			return favicon;
		}
	}
#if defined(BROWSER_THUMBNAIL_VIEW)
	else if (!strcmp(part, "elm.swallow.icon")) {
		if (item->is_folder()) {
			Evas_Object *folder_icon = elm_icon_add(obj);
			BROWSER_LOGD("[%s]%s", part, item->get_title());
			elm_icon_standard_set(folder_icon, browser_img_dir"/I01_bookmark_thumbnailview_folder.png");
			evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return folder_icon;
		} else {
			Evas_Object *thumbnail = cp->m_bookmark->get_thumbnail(item->get_id(), obj);
			if (!thumbnail) {
				thumbnail = elm_icon_add(obj);
				elm_icon_standard_set(thumbnail, browser_img_dir"/I01_bookmark_02.png");
				evas_object_size_hint_aspect_set(thumbnail, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			}
			return thumbnail;
		}
	}
#endif
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
		bv->_set_genlist_folder_tree_recursive(item_data->get_id(), bv->m_genlist, it);
}

void bookmark_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!event_info) return;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
	bookmark_item *item = (bookmark_item *)cb_data->user_data;

	if (item->is_folder()) {
		BROWSER_LOGD("Folder Item clicked[Title:%s]", item->get_title());
		Eina_Bool expanded = EINA_FALSE;
		Elm_Object_Item *it = (Elm_Object_Item *)event_info;
		elm_genlist_item_selected_set(it, EINA_FALSE);
		expanded = elm_genlist_item_expanded_get(it);
		elm_genlist_item_expanded_set(it, !expanded);
		elm_genlist_item_update(it);
	} else {
		BROWSER_LOGD("Bookmark Item clicked[URL:%s]", item->get_uri());
		char *uri = elm_entry_utf8_to_markup(item->get_uri());
		m_browser->get_browser_view()->get_current_webview()->load_uri(uri);

		if (uri)
			free(uri);

		m_browser->get_multiwindow_view()->close_multiwindow_view();
	}
}

#if defined(BROWSER_THUMBNAIL_VIEW)
void bookmark_view::__gengrid_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!event_info) return;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
	bookmark_item *item = (bookmark_item *)cb_data->user_data;

	if (item->is_folder()) {
		BROWSER_LOGD("Folder Item clicked[Title:%s]", item->get_title());
	} else {
		BROWSER_LOGD("Bookmark Item clicked[URL:%s]", item->get_uri());
		m_browser->get_browser_view()->get_current_webview()->load_uri(item->get_uri());

		m_browser->get_multiwindow_view()->close_multiwindow_view();
	}
}
#endif

Evas_Object *bookmark_view::_create_genlist(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	return genlist;
}

#if defined(BROWSER_THUMBNAIL_VIEW)
Evas_Object *bookmark_view::_create_gengrid(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *gengrid = elm_gengrid_add(parent);
	if (!gengrid) {
		BROWSER_LOGE("elm_gengrid_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_gengrid_item_size_set(gengrid, THUMBNAIL_ITEM_W, THUMBNAIL_ITEM_H);
	elm_gengrid_align_set(gengrid, 0.5, 0.0);
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
	elm_scroller_bounce_set(gengrid, EINA_FALSE, EINA_FALSE);
	elm_gengrid_multi_select_set(gengrid, EINA_TRUE);

	return gengrid;
}
#endif

Eina_Bool bookmark_view::_set_genlist_folder_view(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(genlist, EINA_FALSE);

	_clear_genlist_item_data(genlist, m_view_mode);
	elm_genlist_clear(genlist);

	m_itc_folder.item_style = "1text.1icon.2";
	m_itc_folder.func.text_get = __genlist_get_label_cb;
	m_itc_folder.func.content_get = __genlist_icon_get_cb;
	m_itc_folder.func.state_get = NULL;
	m_itc_folder.func.del = NULL;

	m_itc_bookmark_folder.item_style = "1text.1icon.2";
	m_itc_bookmark_folder.func.text_get = __genlist_get_label_cb;
	m_itc_bookmark_folder.func.content_get = __genlist_icon_get_cb;
	m_itc_bookmark_folder.func.state_get = NULL;
	m_itc_bookmark_folder.func.del = NULL;

	_set_genlist_folder_tree_recursive(m_bookmark->get_root_folder_id(), genlist, NULL);
	evas_object_smart_callback_add(genlist, "expanded", __genlist_exp_cb, this);
	evas_object_smart_callback_add(genlist, "contracted", __genlist_cont_cb, this);

	return EINA_TRUE;
}

Eina_Bool bookmark_view::_set_genlist_folder_tree_recursive(int folder_id,
							Evas_Object *genlist, Elm_Object_Item *parent_item)
{
	BROWSER_LOGD("");
	std::vector<bookmark_item *> bookmark_list;
	Eina_Bool ret;

	ret = m_bookmark->get_list_by_folder(folder_id, bookmark_list);
	if (ret == EINA_FALSE) {
		BROWSER_LOGE("get_list_by_folder is failed(folder id:%d)",folder_id);
		return EINA_FALSE;
	}

	for(unsigned int j = 0 ; j < bookmark_list.size() ; j++ ) {
		gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
		memset(item_data, 0x00, sizeof(gl_cb_data));

		bookmark_item *bookmark_item_data = new bookmark_item;
		/* deep copying by overloaded operator = */
		*bookmark_item_data = *bookmark_list[j];
		item_data->cp = this;
		item_data->user_data = (void *)bookmark_item_data;
		if (bookmark_item_data->is_folder()) {
			/* Folder item is found. get sub list */
			BROWSER_LOGD("Folder[%d] is %s(id: %d)\n", j, bookmark_list[j]->get_title(), bookmark_item_data->get_id());
			item_data->it = elm_genlist_item_append(genlist, &m_itc_folder, item_data, parent_item,
					ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, this);
		} else {
			BROWSER_LOGD("bookmark[%d] is %s\n", j, bookmark_list[j]->get_title());
			item_data->it = elm_genlist_item_append(genlist, &m_itc_bookmark_folder, item_data, parent_item,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
		}
	}
	m_bookmark->destroy_list(bookmark_list);
	return EINA_TRUE;
}

#if defined(BROWSER_THUMBNAIL_VIEW)
Eina_Bool bookmark_view::_set_gengrid_thumbnail_view(Evas_Object *gengrid)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(gengrid, EINA_FALSE);
	_clear_genlist_item_data(gengrid, m_view_mode);
	elm_gengrid_clear(gengrid);

	m_itc_gengrid_folder.item_style = "default_gridtext";
	m_itc_gengrid_folder.func.text_get = __genlist_get_label_cb;
	m_itc_gengrid_folder.func.content_get = __genlist_icon_get_cb;
	m_itc_gengrid_folder.func.state_get = NULL;
	m_itc_gengrid_folder.func.del = NULL;

	m_itc_gengrid_bookmark.item_style = "default_gridtext";
	m_itc_gengrid_bookmark.func.text_get = __genlist_get_label_cb;
	m_itc_gengrid_bookmark.func.content_get = __genlist_icon_get_cb;
	m_itc_gengrid_bookmark.func.state_get = NULL;
	m_itc_gengrid_bookmark.func.del = NULL;

	_set_gengrid_by_folder(m_curr_folder_id, gengrid);
	return EINA_TRUE;
}

Eina_Bool bookmark_view::_set_gengrid_by_folder(int folder_id, Evas_Object *gengrid)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(gengrid, EINA_FALSE);
	std::vector<bookmark_item *> bookmark_list;
	Eina_Bool ret;

	ret = m_bookmark->get_list_by_folder(folder_id, bookmark_list);
	if (ret == EINA_FALSE) {
		BROWSER_LOGE("get_list_by_folder is failed(folder id:%d)",folder_id);
		return EINA_FALSE;
	}

	for(unsigned int j = 0 ; j < bookmark_list.size() ; j++ ) {
		BROWSER_LOGD("bookmark[%d] is %s\n", j, bookmark_list[j]->get_title());
		gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
		memset(item_data, 0x00, sizeof(gl_cb_data));

		bookmark_item *bookmark_item_data = new bookmark_item;
		/* deep copying by overloaded operator = */
		*bookmark_item_data = *bookmark_list[j];
		/* Folder item is found. get sub list */
		BROWSER_LOGD("Title[%d] is %s(id: %d)\n", j,
				bookmark_list[j]->get_title(),
				bookmark_item_data->get_id());
		item_data->cp = this;
		item_data->user_data = (void *)bookmark_item_data;

		if (bookmark_item_data->is_folder()) {
			/* Folder */
			if (bookmark_item_data->is_editable()) {
				BROWSER_LOGD("Folder[%d] is %s(id: %d)\n",
					j, bookmark_list[j]->get_title(), bookmark_item_data->get_id());
				item_data->it = elm_gengrid_item_append(gengrid,
					&m_itc_gengrid_folder, item_data, __gengrid_item_clicked_cb, this);
			}
		} else {
			if (bookmark_item_data->is_editable()) {
				BROWSER_LOGD("bookmark[%d] is %s\n", j, bookmark_list[j]->get_title());
				item_data->it = elm_gengrid_item_append(gengrid,
					&m_itc_gengrid_bookmark, item_data, __gengrid_item_clicked_cb, this);
			}
		}
	}
	m_bookmark->destroy_list(bookmark_list);
	return EINA_TRUE;
}
#endif

void bookmark_view::_set_view_mode(view_mode mode)
{
	BROWSER_LOGD("mode=%d", mode);
	switch (mode) {
	case FOLDER_VIEW_NORMAL:
	{
		Evas_Object *unused = elm_object_part_content_unset(m_contents_layout, "elm.swallow.bookmarks");
		evas_object_hide(unused);
		if (!m_genlist)
			m_genlist = _create_genlist(m_box);
		_set_genlist_folder_view(m_genlist);
		evas_object_show(m_genlist);
		elm_object_part_content_set(m_contents_layout, "elm.swallow.bookmarks", m_genlist);
		break;
	}
#if defined(BROWSER_THUMBNAIL_VIEW)
	case THUMBNAIL_VIEW_NORMAL:
	{
		Evas_Object *unused = elm_object_part_content_unset(m_contents_layout, "elm.swallow.bookmarks");
		evas_object_hide(unused);
		if (!m_gengrid)
			m_gengrid = _create_gengrid(m_box);
		_set_gengrid_thumbnail_view(m_gengrid);
		evas_object_show(m_gengrid);
		elm_object_part_content_set(m_contents_layout, "elm.swallow.bookmarks", m_gengrid);
		break;
	}
#endif
#if defined(BROWSER_TAG)
	case TAG_VIEW_NORMAL:
		_set_genlist_tag_view(m_genlist);
		break;
	case TAG_VIEW_INDEX:
		_set_genlist_tag_index_view(m_genlist);
		break;
#endif
	default:
		break;
	}
	m_view_mode = mode;
}

Eina_Bool bookmark_view::_clear_genlist_item_data(Evas_Object *genlist, view_mode mode)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(genlist, EINA_FALSE);
	switch (mode) {
	case FOLDER_VIEW_NORMAL:
	{
		Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
		while (it) {
			gl_cb_data *item_data = (gl_cb_data *)elm_object_item_data_get(it);
			if (item_data && item_data->user_data) {
				bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;
				delete bookmark_item_data;
				free(item_data);
			}
			it = elm_genlist_item_next_get(it);
		}
		break;
	}
#if defined(BROWSER_THUMBNAIL_VIEW)
	case THUMBNAIL_VIEW_NORMAL:
	{
		Elm_Object_Item *it = elm_gengrid_first_item_get(genlist);
		while (it) {
			bookmark_item *item_data = (bookmark_item *)elm_object_item_data_get(it);
			delete item_data;
			it = elm_gengrid_item_next_get(it);
		}
		break;
	}
#endif
#if defined(BROWSER_TAG)
	case TAG_VIEW_NORMAL:
	{
		Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
		while (it) {
			if (elm_genlist_item_type_get(it) == ELM_GENLIST_ITEM_GROUP) {
				/* if item is grouptitle */
				char *item_data = (char *)elm_object_item_data_get(it);
				free(item_data);
			} else {
				bookmark_item *item_data = (bookmark_item *)elm_object_item_data_get(it);
				delete item_data;
			}
			it = elm_genlist_item_next_get(it);
		}
		break;
	}
	case TAG_VIEW_INDEX:
		break;
#endif
	default:
		break;
	}

	return EINA_TRUE;
}
void bookmark_view::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_multiwindow_view()->close_multiwindow_view();
}

void bookmark_view::__more_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *bv = (bookmark_view *)data;
	bv->_show_more_context_popup(obj);
}

void bookmark_view::__plus_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_multiwindow_view()->close_multiwindow_view();

	const char *title = m_browser->get_browser_view()->get_current_webview()->get_title();
	const char *uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
	m_browser->create_bookmark_add_view(title, uri,
				m_browser->get_bookmark()->get_root_folder_id(), EINA_FALSE)->show();
}

Evas_Object *bookmark_view::_create_toolbar_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *toolbar_layout = elm_layout_add(parent);
	if (!toolbar_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(toolbar_layout, multiwindow_view_edj_path, "toolbar-layout");
	evas_object_size_hint_weight_set(toolbar_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *more_button = elm_button_add(toolbar_layout);
	if (!more_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(more_button, "browser/toolbar_menu");
	elm_object_part_content_set(toolbar_layout, "elm.swallow.more_button", more_button);
	evas_object_smart_callback_add(more_button, "clicked", __more_cb, this);

	Evas_Object *plus_button = elm_button_add(toolbar_layout);
	if (!plus_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(plus_button, "browser/bookmark_plus_menu");
	elm_object_part_content_set(toolbar_layout, "elm.swallow.plus_button", plus_button);
	evas_object_smart_callback_add(plus_button, "clicked", __plus_cb, this);

	Evas_Object *back_button = elm_button_add(toolbar_layout);
	if (!back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(back_button, "naviframe/back_btn/default");
	elm_object_part_content_set(toolbar_layout, "elm.swallow.back_button", back_button);
	evas_object_smart_callback_add(back_button, "clicked", __back_cb, this);

	return toolbar_layout;
}

Evas_Event_Flags bookmark_view::__gesture_momentum_move(void *data, void *event_info)
{
	Elm_Gesture_Momentum_Info *momentum_info = (Elm_Gesture_Momentum_Info *)event_info;
	multiwindow_view *mv = m_browser->get_multiwindow_view();

	if (!mv->is_multiwindow_showing() && momentum_info->mx > FLICK_MOMENTUM_THRESHOLD) {
		if (momentum_info->my < (-1) * FLICK_MOMENTUM_THRESHOLD)
			return EVAS_EVENT_FLAG_NONE;

		mv->change_view(EINA_TRUE);
		return EVAS_EVENT_FLAG_NONE;
	}

	return EVAS_EVENT_FLAG_NONE;
}

Evas_Object *bookmark_view::_create_gesture_layer(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *gesture_layer = elm_gesture_layer_add(parent);
	if (!gesture_layer) {
		BROWSER_LOGD("elm_gesture_layer_add failed");
		return NULL;
	}
	elm_gesture_layer_attach(gesture_layer, parent);

	elm_gesture_layer_cb_set(gesture_layer, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_MOVE, __gesture_momentum_move, this);

	return gesture_layer;
}

#if defined(BROWSER_THUMBNAIL_VIEW)
void bookmark_view::__ctxpopup_thumbnail_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	cp->m_view_mode = THUMBNAIL_VIEW_NORMAL;
	cp->_set_view_mode(cp->m_view_mode);

	evas_object_del(obj);
}
void bookmark_view::__ctxpopup_folder_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	cp->m_view_mode = FOLDER_VIEW_NORMAL;
	cp->_set_view_mode(cp->m_view_mode);

	evas_object_del(obj);
}
#endif

void bookmark_view::__ctxpopup_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *cp = (bookmark_view *)data;
	m_browser->get_multiwindow_view()->close_multiwindow_view();

	switch (cp->m_view_mode) {
#if defined(BROWSER_TAG)
	case TAG_VIEW_INDEX:
	case TAG_VIEW_NORMAL:
	{
		BROWSER_LOGD("Edit tag view");
		cp->m_browser->create_bookmark_edit_view(true)->show();
		break;
	}
#endif:
#if defined(BROWSER_THUMBNAIL_VIEW)
	case THUMBNAIL_VIEW_NORMAL:
#endif
	case FOLDER_VIEW_NORMAL:
	{
		BROWSER_LOGD("Edit folder view");
		cp->m_browser->create_bookmark_edit_view(false)->show();
		break;
	}
	default:
		break;
	}
	evas_object_del(obj);
}

void bookmark_view::__ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	evas_object_del(obj);
}

void bookmark_view::__view_by_popup_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *bv = (bookmark_view *)data;
	bv->destroy_popup(obj);
	bv->_set_view_mode(FOLDER_VIEW_NORMAL);
}

void bookmark_view::_show_more_context_popup(Evas_Object *parent)
{
	BROWSER_LOGD("");
	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", __ctxpopup_dismissed_cb, NULL);

#if defined(BROWSER_THUMBNAIL_VIEW)
	if (m_view_mode == THUMBNAIL_VIEW_NORMAL)
		elm_ctxpopup_item_append(more_popup, BR_STRING_LIST_VIEW, NULL, __ctxpopup_folder_view_cb, this);
	else if (m_view_mode == FOLDER_VIEW_NORMAL)
		elm_ctxpopup_item_append(more_popup, BR_STRING_THUMBNAIL_VIEW, NULL, __ctxpopup_thumbnail_view_cb, this);
#endif
	elm_ctxpopup_item_append(more_popup, BR_STRING_EDIT, NULL, __ctxpopup_edit_cb, this);
#if defined(BROWSER_TAG)
	elm_ctxpopup_item_append(more_popup, BR_STRING_POPUP_VIEW_BY, NULL, __ctxpopup_view_by_cb, this);
#endif

	int x, y, w, h;
	evas_object_geometry_get(parent, &x, &y, &w, &h);
	evas_object_move(more_popup, x + (w / 2), y + (h /2));
	evas_object_show(more_popup);
}

Evas_Object *bookmark_view::_create_box(Evas_Object * parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *box = NULL;
	box = elm_box_add(parent);
	EINA_SAFETY_ON_NULL_RETURN_VAL(box, NULL);
	elm_object_focus_set(box, EINA_FALSE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_clear(box);
	evas_object_show(box);
	return box;
}

Evas_Object *bookmark_view::_create_main_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *main_layout = elm_layout_add(parent);
	if (!main_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(main_layout, bookmark_view_edj_path, "main-layout");
	evas_object_size_hint_weight_set(main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_box = _create_box(main_layout);
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_box, NULL);

	m_contents_layout = elm_layout_add(m_box);
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_contents_layout, NULL);
	elm_object_focus_set(m_contents_layout, EINA_FALSE);
	elm_layout_file_set(m_contents_layout, bookmark_view_edj_path, "contents");
	evas_object_size_hint_weight_set(m_contents_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_contents_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_contents_layout);

	elm_box_pack_end(m_box, m_contents_layout);

#if defined(BROWSER_THUMBNAIL_VIEW)
	m_view_mode = THUMBNAIL_VIEW_NORMAL;
	//m_view_mode = FOLDER_VIEW_NORMAL;
#else
	m_view_mode = FOLDER_VIEW_NORMAL;
#endif

	_set_view_mode(m_view_mode);

	elm_object_part_content_set(main_layout, "elm.swallow.contents", m_box);

	Evas_Object *event_rect = evas_object_rectangle_add(evas_object_evas_get(main_layout));
	if (!event_rect) {
		BROWSER_LOGD("evas_object_rectangle_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(event_rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_color_set(event_rect, 0, 0, 0, 0);
	elm_object_part_content_set(main_layout, "elm.swallow.gesture", event_rect);

	m_gesture_layer = _create_gesture_layer(event_rect);

	return main_layout;
}

#if defined(BROWSER_TAG)
char *bookmark_view::__genlist_get_tag_cb(void *data, Evas_Object *obj, const char *part)
{
	char *tag = (char *)data;
	if (!strcmp(part, "elm.text"))
		return elm_entry_utf8_to_markup(tag);

	return NULL;
}

void bookmark_view::__genlist_tag_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);
	EINA_SAFETY_ON_NULL_RETURN(data);

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	bookmark_view *cp = (bookmark_view *)data;
	char *tag_name = (char *)elm_object_item_data_get(it);

	elm_genlist_item_selected_set(it, EINA_FALSE);

	BROWSER_LOGD("VIEW MODE[%d]", cp->_get_view_mode());
	if (cp->_get_view_mode() == TAG_VIEW_NORMAL)
		cp->_set_view_mode(TAG_VIEW_INDEX);
	else if (cp->_get_view_mode() == TAG_VIEW_INDEX) {
		cp->_set_view_mode(TAG_VIEW_NORMAL);
		BROWSER_LOGD("[%p]", it);
		Elm_Object_Item *cur_it = elm_genlist_first_item_get(cp->m_genlist);
		while (cur_it) {
			if (elm_genlist_item_type_get(cur_it) == ELM_GENLIST_ITEM_GROUP) {
				/* if item is grouptitle */
				char *item_data = (char *)elm_object_item_data_get(cur_it);
				if (!strcmp(tag_name, item_data)) {
					BROWSER_LOGD("CURIT[%p]", cur_it);
					elm_genlist_item_show(cur_it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
					break;
				}
			}
			cur_it = elm_genlist_item_next_get(cur_it);
		}
	}
}

Eina_Bool bookmark_view::_set_genlist_tag_view(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(genlist, EINA_FALSE);

	_clear_genlist_item_data(genlist, m_view_mode);
	elm_genlist_clear(genlist);
	elm_object_style_set(genlist, "handler");

	m_itc_tag.item_style = "groupindex";
	m_itc_tag.func.text_get = __genlist_get_tag_cb;
	m_itc_tag.func.content_get = NULL;
	m_itc_tag.func.state_get = NULL;
	m_itc_tag.func.del = NULL;

	m_itc_bookmark_tag.item_style = "1text.1icon.7";
	m_itc_bookmark_tag.func.text_get = __genlist_get_label_cb;
	m_itc_bookmark_tag.func.content_get = __genlist_icon_get_cb;
	m_itc_bookmark_tag.func.state_get = NULL;
	m_itc_bookmark_tag.func.del = NULL;

	std::vector<char *> tag_list;
	if (m_bookmark->get_tag_list(tag_list) == EINA_FALSE)
		return EINA_FALSE;

	/* Get tagged item list */
	for(unsigned int i = 0 ; i < tag_list.size() ; i++ ) {
		if (tag_list[i]) {
			BROWSER_LOGD("tag[%d] is %s\n", i, tag_list[i]);
			/* Grouptitle - Tag name*/
			Elm_Object_Item *it = elm_genlist_item_append(genlist, &m_itc_tag,
					strdup(tag_list[i]), NULL,
					ELM_GENLIST_ITEM_GROUP, __genlist_tag_item_clicked_cb, this);

			/* Items - bookmark items*/
			std::vector<bookmark_item *> bookmark_list;
			m_bookmark->get_list_by_tag(tag_list[i], &bookmark_list);
			for(unsigned int j = 0 ; j < bookmark_list.size() ; j++ ) {
				BROWSER_LOGD("bookmark[%d] is %s\n", j, bookmark_list[j]->get_title());
				bookmark_item *item = new bookmark_item;
				/* deep copying by overloaded operator = */
				*item = *bookmark_list[j];
				elm_genlist_item_append(genlist, &m_itc_bookmark_tag, item, it,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
			}
			m_bookmark->destroy_list(bookmark_list);
		}
	}
	m_bookmark->destroy_tag_list(tag_list);

	/* Get Untagged list and append those to genlist */
	std::vector<bookmark_item *> bookmark_list_untagged;
	m_bookmark->get_list_by_tag("", &bookmark_list_untagged);
	Elm_Object_Item *untagged_group = NULL;
	if (bookmark_list_untagged.size() > 0) {
			untagged_group = elm_genlist_item_append(genlist, &m_itc_tag,
					strdup(BR_STRING_UNTAGGED), NULL,
					ELM_GENLIST_ITEM_GROUP, __genlist_tag_item_clicked_cb, this);
	}

	for(unsigned int i = 0 ; i < bookmark_list_untagged.size() ; i++ ) {
		BROWSER_LOGD("untagged bookmark[%d] is %s\n", i, bookmark_list_untagged[i]->get_title());
		bookmark_item *item = new bookmark_item;
		/* deep copying by overloaded operator = */
		*item = *bookmark_list_untagged[i];
		elm_genlist_item_append(genlist, &m_itc_bookmark_tag, item, untagged_group,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
	}
	m_bookmark->destroy_list(bookmark_list_untagged);

	return EINA_TRUE;
}

Eina_Bool bookmark_view::_set_genlist_tag_index_view(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(genlist, EINA_FALSE);

	_clear_genlist_item_data(genlist, m_view_mode);
	elm_genlist_clear(genlist);
	elm_object_style_set(genlist, "handler");

	m_itc_tag.item_style = "groupindex";
	m_itc_tag.func.text_get = __genlist_get_tag_cb;
	m_itc_tag.func.content_get = NULL;
	m_itc_tag.func.state_get = NULL;
	m_itc_tag.func.del = NULL;

	std::vector<char *> tag_list;
	if (m_bookmark->get_tag_list(tag_list) == EINA_FALSE)
		return EINA_FALSE;

	/* Get tagged item list */
	for(unsigned int i = 0 ; i < tag_list.size() ; i++ ) {
		if (tag_list[i]) {
			BROWSER_LOGD("tag[%d] is %s\n", i, tag_list[i]);
			/* Grouptitle - Tag name*/
			elm_genlist_item_append(genlist, &m_itc_tag,
					strdup(tag_list[i]), NULL,
					ELM_GENLIST_ITEM_GROUP, __genlist_tag_item_clicked_cb, this);
		}
	}
	m_bookmark->destroy_tag_list(tag_list);

	/* Get Untagged list and append those to genlist */
	std::vector<bookmark_item *> bookmark_list_untagged;
	m_bookmark->get_list_by_tag("", &bookmark_list_untagged);
	if (bookmark_list_untagged.size() > 0) {
			elm_genlist_item_append(genlist, &m_itc_tag,
					strdup(BR_STRING_UNTAGGED), NULL,
					ELM_GENLIST_ITEM_GROUP, __genlist_tag_item_clicked_cb, this);
	}
	m_bookmark->destroy_list(bookmark_list_untagged);

	return EINA_TRUE;
}

void bookmark_view::__view_by_popup_tag_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *bv = (bookmark_view *)data;
	bv->destroy_popup(obj);
	bv->_set_view_mode(TAG_VIEW_NORMAL);
}


void bookmark_view::__ctxpopup_view_by_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_view *bv = (bookmark_view *)data;
	bv->_show_view_by_popup();
	evas_object_del(obj);
}

void bookmark_view::_show_view_by_popup()
{
	Evas_Object *content_list = elm_list_add(m_window);
	if (!content_list) {
		BROWSER_LOGE("elm_list_add failed");
	}
	elm_list_mode_set(content_list, ELM_LIST_EXPAND);
	elm_list_item_append(content_list, BR_STRING_TAG, NULL, NULL, __view_by_popup_tag_cb, this);
	elm_list_item_append(content_list, BR_STRING_FOLDER, NULL, NULL, __view_by_popup_folder_cb, this);

	show_content_popup(BR_STRING_POPUP_VIEW_BY, content_list, BR_STRING_CANCEL, NULL, NULL, NULL, NULL);
}
#endif
