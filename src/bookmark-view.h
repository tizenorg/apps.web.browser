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

#ifndef BOOKMARK_VIEW_H
#define BOOKMARK_VIEW_H

#include <Elementary.h>
#include <vector>
#include "browser-object.h"
#include "common-view.h"
#include "bookmark-edit-view.h"

class bookmark_item;
class bookmark;

class bookmark_view : public browser_object, public common_view {
public:
	bookmark_view(void);
	~bookmark_view(void);

	Evas_Object *get_layout(void) { return m_main_layout; }
	Evas_Object *get_toolbar_layout(void) { return m_toolbar_layout; }
private:
	typedef enum _view_mode {
		FOLDER_VIEW_NORMAL
#if defined(BROWSER_THUMBNAIL_VIEW)
		,THUMBNAIL_VIEW_NORMAL
#endif
#if defined(BROWSER_TAG)
		,TAG_VIEW_NORMAL
		,TAG_VIEW_INDEX
#endif
		,UNKNOWN_VIEW
	} view_mode;
#if defined(BROWSER_THUMBNAIL_VIEW)
	typedef struct _folder_info {
		int folder_id;
		char *folder_name;
	} folder_info;
#endif

	Evas_Object *_create_genlist(Evas_Object *parent);
	Evas_Object *_create_gesture_layer(Evas_Object *parent);
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_toolbar_layout(Evas_Object *parent);
	Evas_Object *_create_box(Evas_Object * parent);
	Evas_Object *_create_no_content(Evas_Object *parent, const char *text);
#if defined(BROWSER_THUMBNAIL_VIEW)
	Evas_Object *_create_gengrid(Evas_Object *parent);
	void _set_path_info_layout(void);
#endif

	Eina_Bool _set_genlist_folder_view(Evas_Object *genlist);
	Eina_Bool _set_genlist_folder_tree_recursive(int folder_id,
							Evas_Object *genlist,Elm_Object_Item *parent_item);
#if defined(BROWSER_THUMBNAIL_VIEW)
	Eina_Bool _set_gengrid_thumbnail_view(Evas_Object *gengrid);
	Eina_Bool _set_gengrid_by_folder(int folder_id, Evas_Object *gengrid);
#endif
	void _set_view_mode(view_mode mode);
	view_mode _get_view_mode(void) { return m_view_mode; }
	Eina_Bool _clear_genlist_item_data(Evas_Object *genlist, view_mode mode);
	void _show_more_context_popup(Evas_Object *parent);
#if defined(BROWSER_THUMBNAIL_VIEW)
	void _go_into_sub_folder(int folder_id, const char *folder_name);
	Eina_Bool _go_to_upper_folder();
#endif

	static void __back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_cb(void *data, Evas_Object *obj, void *event_info);
	static void __plus_cb(void *data, Evas_Object *obj, void *event_info);
	static Evas_Event_Flags __gesture_momentum_move(void *data, void *event_info);
	static char *__genlist_get_label_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_cont_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_exp_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
#if defined(BROWSER_THUMBNAIL_VIEW)
	static void __gengrid_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static void __ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_edit_cb(void *data, Evas_Object *obj, void *event_info);
#if defined(BROWSER_THUMBNAIL_VIEW)
	static void __ctxpopup_thumbnail_view_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_folder_view_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static void __view_by_popup_folder_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_main_layout;
	Evas_Object *m_gesture_layer;
	Evas_Object *m_toolbar_layout;
	Evas_Object *m_box;
	Evas_Object *m_contents_layout;
	Evas_Object *m_genlist;
#if defined(BROWSER_THUMBNAIL_VIEW)
	Evas_Object *m_gengrid;
	Evas_Object *m_path_info_layout;
#endif

	Elm_Genlist_Item_Class m_itc_folder;
	Elm_Genlist_Item_Class m_itc_bookmark_folder;
#if defined(BROWSER_THUMBNAIL_VIEW)
	Elm_Gengrid_Item_Class m_itc_gengrid_folder;
	Elm_Gengrid_Item_Class m_itc_gengrid_bookmark;
	Elm_Gengrid_Item_Class m_itc_gengrid_upper_folder;
#endif
	std::vector<bookmark_item *> m_bookmark_list;
	bookmark *m_bookmark;
	view_mode m_view_mode;
#if defined(BROWSER_THUMBNAIL_VIEW)
	std::vector<folder_info *> m_path_history;
	std::string m_path_string;
	int m_curr_folder_id;
#endif

#if defined(BROWSER_TAG)
	Eina_Bool _set_genlist_tag_view(Evas_Object *genlist);
	Eina_Bool _set_genlist_tag_index_view(Evas_Object *genlist);
	static char *__genlist_get_tag_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_tag_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __view_by_popup_tag_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_view_by_cb(void *data, Evas_Object *obj, void *event_info);
	void _show_view_by_popup();

	Elm_Genlist_Item_Class m_itc_tag;
	Elm_Genlist_Item_Class m_itc_bookmark_tag;
#endif
};

#endif /* BOOKMARK_VIEW_H */

