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
 * Contact: Jiwon Lee <jiwonear.lee@samsung.com>
 *
 */

#include <Ecore_X.h>
#include <efl_extension.h>

#include "bookmark.h"
#include "bookmark-item.h"
#include "bookmark-view.h"
#include "bookmark-edit-view.h"
#include "bookmark-create-folder-view.h"
#include "bookmark-add-view.h"
#include "bookmark-select-folder-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "common-view.h"
#include "platform-service.h"

typedef struct _gl_cb_data {
	void *user_data;
	Eina_Bool is_checked;
	void *cp;
	Elm_Object_Item *it;
} gl_cb_data;

#define bookmark_edit_view_edj_path browser_edj_dir"/bookmark-edit-view.edj"
#define bookmark_view_edj_path browser_edj_dir"/bookmark-view.edj"
#define browser_popup_edj_path browser_edj_dir"/browser-popup.edj"
#define browser_genlist_edj_path browser_edj_dir"/browser-genlist.edj"
#define history_view_edj_path browser_edj_dir"/history-view.edj"
#define br_navigationbar_edj_path browser_edj_dir"/br-navigationbar.edj"

#define GENLIST_GROUPINDEX_LIST_SIZE (76 * efl_scale)
#define GENLIST_GROUPINDEX_LIST_SIZE_HUGE_FONT (98* efl_scale)
#define GENLIST_GROUPINDEX_LIST_SIZE_GIANT_FONT (114 * efl_scale)

Ecore_Thread *bookmark_edit_view::m_current_thread;

bookmark_edit_view::bookmark_edit_view(Evas_Smart_Cb cb_func, void *cb_data, int folder_id, const char *path_string, edit_view_mode view_mode)
:
	m_outer_layout(NULL)
	,m_box(NULL)
	,m_main_layout(NULL)
	,m_genlist(NULL)
	,m_no_contents_layout(NULL)
	,m_select_all_layout(NULL)
	,m_ctx_popup_more_menu(NULL)
	,m_popup_processing(NULL)
	,m_progressbar(NULL)
	,m_btn_done(NULL)
	,m_btn_cancel(NULL)
	,m_delete_confirm_popup(NULL)
	,m_naviframe_item(NULL)
	,m_count_checked_item(0)
	,m_check_registered(EINA_FALSE)
	,m_cb_func(cb_func)
	,m_cb_data(cb_data)
{
	BROWSER_LOGD("view_mode: %d", view_mode);

	if (m_genlist) {
		elm_genlist_clear(m_genlist);
		evas_object_del(m_genlist);
	}

	m_bookmark = m_browser->get_bookmark();
	if (folder_id > 0)
		m_curr_folder = folder_id;
	else
		m_curr_folder = m_bookmark->get_root_folder_id();
	m_bookmark_list.clear();
	m_view_mode = view_mode;
}

bookmark_edit_view::~bookmark_edit_view(void)
{
	BROWSER_LOGD("");
	elm_object_part_content_unset(m_main_layout, "elm.swallow.contents");
	if (m_current_thread) {
		_close_processing_popup();
		ecore_thread_cancel(m_current_thread);
	}
	if (m_bookmark_list.size())
		m_bookmark->destroy_list(m_bookmark_list);
	if (m_genlist) {
		evas_object_del(m_genlist);
		m_genlist = NULL;
	}
	if (m_ctx_popup_more_menu)
		evas_object_del(m_ctx_popup_more_menu);
	if (m_no_contents_layout)
		evas_object_del(m_no_contents_layout);
	if (m_select_all_layout) {
		if(m_box)
			elm_box_unpack(m_box, m_select_all_layout);
		evas_object_del(m_select_all_layout);
		m_select_all_layout = NULL;
	}
	if (m_popup_processing) {
		evas_object_del(m_popup_processing);
		m_popup_processing = NULL;
	}
	elm_object_style_set(get_elm_bg(), "default");
	evas_object_smart_callback_del(m_naviframe,
						"transition,finished",
						__naviframe_pop_finished_cb);
}

char *bookmark_edit_view::__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)cb_data->user_data;
	bookmark_edit_view *cp = (bookmark_edit_view *)cb_data->cp;

	if (!strcmp(part, "elm.text.main.left.top") || !strcmp(part, "elm.text.main.left")) {
		BROWSER_LOGD("[%s]", part);
		RETV_MSG_IF(!(bookmark_item_data->get_title()), NULL, "title is empty");
		BROWSER_SECURE_LOGD("[%s][%s] is added", part, bookmark_item_data->get_title());
		/* only textblock genlist item style(#text.#icon,tb) need to encode to mark up */

		if (cp->m_view_mode == EDIT_FOLDER_REORDER_VIEW) {
			Evas_Object *item_access_obj = elm_object_item_access_object_get(cb_data->it);
			elm_access_info_set(item_access_obj, ELM_ACCESS_STATE, BR_STRING_ACCESS_DOUBLE_TAP_AND_DRAG_TO_REORDER);
		}
		char *markup_title = elm_entry_utf8_to_markup(bookmark_item_data->get_title());
		if (markup_title) {
			char *title = strdup(markup_title);
			free(markup_title);
			return title;
		} else
			return strdup(bookmark_item_data->get_title());
	} else if (!strcmp(part, "elm.text.sub.left.bottom")) {
		RETV_MSG_IF(!(bookmark_item_data->get_uri()), NULL, "title is empty");
		BROWSER_SECURE_LOGD("[%s][%s] is added", part, bookmark_item_data->get_uri());
		return strdup(bookmark_item_data->get_uri());
	}
	return NULL;
}

Evas_Object *bookmark_edit_view::__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
	RETV_MSG_IF(!obj, NULL, "obj is NULL");

	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_edit_view *cp = (bookmark_edit_view *)cb_data->cp;
	bookmark_item *item = (bookmark_item *)cb_data->user_data;

	if (!strcmp(part, "elm.icon.2")) {
		Evas_Object *content = elm_layout_add(obj);
		if (content == NULL) {
			BROWSER_LOGD("elm_layout_add is failed");
			return NULL;
		}
		switch (cp->m_view_mode) {
		case EDIT_FOLDER_REORDER_VIEW:
		{
			elm_layout_theme_set(content, "layout", "list/C/type.1", "default");
			Evas_Object *btn_reorder = elm_button_add(content);
			if (btn_reorder == NULL) {
				BROWSER_LOGD("elm_button_add is failed");
				evas_object_del(content);
				return NULL;
			}
			elm_object_style_set(btn_reorder, "icon_reorder");
			evas_object_event_callback_add(btn_reorder, EVAS_CALLBACK_MOUSE_DOWN, cp->__reorder_button_mouse_down_cb, cb_data);
			evas_object_event_callback_add(btn_reorder, EVAS_CALLBACK_MOUSE_UP, cp->__reorder_button_mouse_up_cb, cb_data);
			elm_layout_content_set(content, "elm.swallow.content", btn_reorder);
			return content;
		}
		case EDIT_FOLDER_DELETE_VIEW:
		{
			elm_layout_theme_set(content, "layout", "list/C/type.2", "default");
			// swallow checkbox
			Evas_Object *check_box = elm_check_add(obj);
			if (check_box == NULL) {
				BROWSER_LOGD("elm_check_add is failed");
				evas_object_del(content);
				return NULL;
			}
			evas_object_size_hint_align_set(check_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(check_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			// keep & revoke the state from integer pointer
			elm_check_state_pointer_set(check_box, &(cb_data->is_checked));
			evas_object_smart_callback_add(check_box, "changed", __chk_changed_cb, cb_data);
			evas_object_propagate_events_set(check_box, EINA_FALSE);
			evas_object_repeat_events_set(check_box, EINA_FALSE);
			elm_layout_content_set(content, "elm.swallow.content", check_box);
			return content;
		}
		default:
			break;
		}
	} else if (!strcmp(part, "elm.icon.1")) {
		Evas_Object *content = elm_layout_add(obj);
		if (content == NULL) {
			BROWSER_LOGD("elm_layout_add is failed");
			return NULL;
		}
		elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
		if (item->is_folder()) {
			Evas_Object *folder_icon = elm_icon_add(obj);
			elm_image_file_set(folder_icon, bookmark_view_edj_path, "contacts_ic_folder.png");
			elm_layout_content_set(content, "elm.swallow.content", folder_icon);
			return content;;
		} else {
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

void bookmark_edit_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	RET_MSG_IF(!(event_info), "event_info is NULL");

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	elm_genlist_item_selected_set(it, EINA_FALSE);

	switch (cp->m_view_mode) {
	case EDIT_FOLDER_DELETE_VIEW:
	{
		gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
		Evas_Object *layout = elm_object_item_part_content_get(it, "elm.icon.2");
		Evas_Object *checkbox = elm_layout_content_get(layout, "elm.swallow.content");

		Eina_Bool state = elm_check_state_get(checkbox);
		elm_check_state_set(checkbox, !state);

		cp->__chk_changed_cb(cb_data, checkbox, NULL);
		break;
	}
	default:
		break;
	}
}

void bookmark_edit_view::__genlist_moved_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	RET_MSG_IF(!(event_info), "event_info is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
	RET_MSG_IF(!(cb_data), "cb_data is NULL");

	bookmark_item *item_data = (bookmark_item *)cb_data->user_data;
	RET_MSG_IF(!(item_data), "item_data is NULL");

	BROWSER_LOGD("cur item : %s", item_data->get_title());
	bookmark_edit_view *cp = (bookmark_edit_view *)cb_data->cp;

	int curr_order = elm_genlist_item_index_get(cb_data->it) - 1;	// Genlist&Gengrid change item index start value from 0 to 1
	BROWSER_LOGD("index %d", curr_order);
	RET_MSG_IF((curr_order < 0), "curr_order is wrong index");
	cp->_reorder_bookmark_items(item_data, curr_order);

	if (cp->m_btn_done)
		elm_object_disabled_set(cp->m_btn_done, cp->_is_disable_done_button(item_data, curr_order));
}

void bookmark_edit_view::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!event_info) {
		BROWSER_LOGE("event_info is NULL");
		return;
	}
	Elm_Object_Item *item = (Elm_Object_Item*)event_info;

	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	Eina_List *items = NULL;

	btn1 = elm_object_item_part_content_get(item, "elm.icon.1");

	Eina_Bool state = elm_check_state_get(btn1);
	char *tickbox_type = elm_access_info_get(btn1, ELM_ACCESS_TYPE);
	Evas_Object *item_access_obj = elm_object_item_access_object_get(item);
	elm_access_info_set(item_access_obj, ELM_ACCESS_TYPE, tickbox_type);

	if (state)
		elm_access_info_set(item_access_obj, ELM_ACCESS_STATE, BR_STRING_OPT_SELECTED);
	else
		elm_access_info_set(item_access_obj, ELM_ACCESS_STATE, _("IDS_BR_SK_UNTICK_ABB"));

	if (btn1)
		items = eina_list_append(items, btn1);

	btn2 = elm_object_item_part_content_get(item, "elm.edit.icon.2");
	if (btn2)
		items = eina_list_append(items, btn2);

	if (items)
		elm_object_item_access_order_set(item, items);

	if (tickbox_type)
		free(tickbox_type);
}

void bookmark_edit_view::__chk_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_edit_view *cp = (bookmark_edit_view *)cb_data->cp;

	Eina_Bool state = elm_check_state_get(obj);

	char *tickbox_type = elm_access_info_get(obj, ELM_ACCESS_TYPE);
	Evas_Object *item_access_obj = elm_object_item_access_object_get(cb_data->it);
	elm_access_info_set(item_access_obj, ELM_ACCESS_TYPE, tickbox_type);

	if (cb_data->is_checked)
		elm_access_info_set(item_access_obj, ELM_ACCESS_STATE, BR_STRING_OPT_SELECTED);
	else
		elm_access_info_set(item_access_obj, ELM_ACCESS_STATE, _("IDS_BR_SK_UNTICK_ABB"));

	cp->_stat_checked_item(state, cb_data);

	if(cp->m_count_checked_item)
		cp->_set_selected_title();
	else
		cp->_unset_selected_title();

	unsigned int count_bookmarks = 0;
	Elm_Object_Item *it = elm_genlist_first_item_get(cp->m_genlist);
	while (it) {
		cb_data = (gl_cb_data *)elm_object_item_data_get(it);
		if (cb_data) {
			count_bookmarks++;
		}
		it = elm_genlist_item_next_get(it);
	}
	BROWSER_LOGD("count_bookmarks:%d", count_bookmarks);
	BROWSER_LOGD("m_count_checked_item:%d", cp->m_count_checked_item);
	if (cp->m_count_checked_item == count_bookmarks) {
		// uncheck
		Evas_Object *ck_all = elm_object_part_content_get(cp->m_select_all_layout, "elm.icon");
		if (ck_all) elm_check_state_set(ck_all, EINA_TRUE);
	} else {
		//check
		Evas_Object *ck_all = elm_object_part_content_get(cp->m_select_all_layout, "elm.icon");
		if (ck_all) elm_check_state_set(ck_all, EINA_FALSE);
	}

	if (tickbox_type)
		free(tickbox_type);
}


void bookmark_edit_view::__select_all_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info, Eina_Bool checkbox_flow_switch)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	unsigned int count_bookmarks = 0;
	Elm_Object_Item *it = elm_genlist_first_item_get(cp->m_genlist);
	while (it) {
		gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
		if (cb_data) {
			count_bookmarks++;
		}
		it = elm_genlist_item_next_get(it);
	}
	BROWSER_LOGD("count_bookmarks:%d", count_bookmarks);
	BROWSER_LOGD("m_count_checked_item:%d", cp->m_count_checked_item);
	if (cp->m_count_checked_item == count_bookmarks) {
		// uncheck
		if(checkbox_flow_switch){
			Evas_Object *ck_all = elm_object_part_content_get(cp->m_select_all_layout, "elm.icon");
			if (ck_all) elm_check_state_set(ck_all, EINA_FALSE);
		}

		Elm_Object_Item *it = elm_genlist_first_item_get(cp->m_genlist);
		while (it) {
			gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
			if (cb_data) {
				if (cb_data->is_checked == EINA_TRUE) {
					cb_data->is_checked = EINA_FALSE;
					cp->_stat_checked_item(EINA_FALSE, cb_data);
					Evas_Object *layout = elm_object_item_part_content_get(it, "elm.icon.2");
					Evas_Object *ck = elm_layout_content_get(layout, "elm.swallow.content");
					if (ck) elm_check_state_set(ck, cb_data->is_checked);
				}
			}
			it = elm_genlist_item_next_get(it);
		}
	} else {
		//check
		if(checkbox_flow_switch){
			Evas_Object *ck_all = elm_object_part_content_get(cp->m_select_all_layout, "elm.icon");
			if (ck_all) elm_check_state_set(ck_all, EINA_TRUE);
		}

		Elm_Object_Item *it = elm_genlist_first_item_get(cp->m_genlist);
		while (it) {
			gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
			if (cb_data) {
				if (cb_data->is_checked == EINA_FALSE) {
					cb_data->is_checked = EINA_TRUE;
					cp->_stat_checked_item(EINA_TRUE, cb_data);
					Evas_Object *layout = elm_object_item_part_content_get(it, "elm.icon.2");
					Evas_Object *ck = elm_layout_content_get(layout, "elm.swallow.content");
					if (ck) elm_check_state_set(ck, cb_data->is_checked);
				}
			}
			it = elm_genlist_item_next_get(it);
		}
	}

	if(cp->m_count_checked_item)
		cp->_set_selected_title();
	else
		cp->_unset_selected_title();
}

void bookmark_edit_view::__reorder_button_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	gl_cb_data *cb_data = (gl_cb_data *)data;
	elm_genlist_item_reorder_start(cb_data->it);

}

void bookmark_edit_view::__reorder_button_mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	gl_cb_data *cb_data = (gl_cb_data *)data;
	elm_genlist_item_reorder_stop(cb_data->it);
}

void bookmark_edit_view::_clear_delete_confirm_popup(void)
{
	BROWSER_LOGD("");

	if(m_delete_confirm_popup) {
		evas_object_del(m_delete_confirm_popup);
		m_delete_confirm_popup = NULL;
	}
}

void bookmark_edit_view::__cancel_delete_confirm_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	cp->_clear_delete_confirm_popup();
}

void bookmark_edit_view::__ok_delete_confirm_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	cp->_clear_delete_confirm_popup();
	cp->_show_processing_popup();
}

void bookmark_edit_view::_show_delete_confirm_popup(void)
{
	BROWSER_LOGD("count : %d", m_count_checked_item);

	Evas_Object *btn1;
	Evas_Object *btn2;

	/* popup */
	m_delete_confirm_popup = elm_popup_add(m_window);
	elm_popup_align_set(m_delete_confirm_popup, -1.0, 1.0); 
	evas_object_size_hint_weight_set(m_delete_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(m_delete_confirm_popup, "title,text", BR_STRING_DELETE);

#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(m_delete_confirm_popup, EEXT_CALLBACK_BACK, __cancel_delete_confirm_popup_cb, this);
#endif

	if(m_count_checked_item == 1) {
		elm_object_text_set(m_delete_confirm_popup, BR_STRING_1_ITEM_DELETED);
	} else {

		char label_count[256] = {'\0', };
		char *text = NULL;
		int len ;
		snprintf(label_count, sizeof(label_count), "%d", m_count_checked_item);
		len = strlen(label_count) + strlen(BR_STRING_PD_ITEMS_DELETED);
		text = (char *)malloc(len * sizeof(char));
		RET_MSG_IF(!text, "text is NULL");
		if (text){
			memset(text, 0x00, len);
			snprintf(text, len, BR_STRING_PD_ITEMS_DELETED, m_count_checked_item);
			m_popup_text.clear();
			m_popup_text.append(text);
			elm_object_text_set(m_delete_confirm_popup, m_popup_text.c_str());
			free(text);
		}
	}

	/* cancel button */
	btn1 = elm_button_add(m_delete_confirm_popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, BR_STRING_CANCEL);
	elm_object_part_content_set(m_delete_confirm_popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", __cancel_delete_confirm_popup_cb, this);

	/* delete button */
	btn2 = elm_button_add(m_delete_confirm_popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, BR_STRING_DELETE);
	elm_object_part_content_set(m_delete_confirm_popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", __ok_delete_confirm_popup_cb, this);

	evas_object_show(m_delete_confirm_popup);
}

void bookmark_edit_view::__done_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	if(cp->m_view_mode == EDIT_FOLDER_DELETE_VIEW)
		cp->_show_delete_confirm_popup();
	else
		cp->_show_processing_popup();
}

void bookmark_edit_view::__cancel_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	cp->_back_to_previous_view();
}

void bookmark_edit_view::__naviframe_pop_finished_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	if (cp->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		return;

	cp->m_browser->delete_bookmark_create_folder_view();
	cp->m_browser->delete_bookmark_select_folder_view();
	cp->m_browser->delete_bookmark_add_view();

	/* if(cp->m_folder_selected) {
		BROWSER_LOGD("folder selected");
		cp->m_folder_selected = EINA_FALSE;
		cp->_folder_selected();
	} */
}

void bookmark_edit_view::__ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	evas_object_del(obj);
	cp->m_ctx_popup_more_menu = NULL;
}

Eina_Bool bookmark_edit_view::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	if (!data) {
		BROWSER_LOGE("data is NULL");
		return EINA_FALSE;
	}

	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	cp->_do_before_naviframe_pop();
	return EINA_TRUE;
}

void bookmark_edit_view::_close_processing_popup(void)
{
	BROWSER_LOGD("");
	if (m_popup_processing) {
		evas_object_del(m_popup_processing);
		m_popup_processing = NULL;
	}

	if (m_genlist) {
		evas_object_freeze_events_set(m_genlist, EINA_FALSE);
	}
}

void bookmark_edit_view::__processing_popup_thread_cb(void *data, Ecore_Thread *th)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	int ret = 0;
	switch (cp->m_view_mode) {
	case EDIT_FOLDER_REORDER_VIEW:
	{
		unsigned int bookmark_list_size = cp->m_bookmark_list.size();
		for (unsigned int j = 0 ; j < bookmark_list_size ; j++ ) {
			if (ecore_thread_check(cp->m_current_thread)) {
				BROWSER_LOGE("Cancelling the thread");
				cp->m_current_thread = NULL;
				return;
			}
			if (cp->m_bookmark_list[j]->get_order() != cp->m_bookmark_list[j]->get_initial_order()) {
				BROWSER_LOGD("title = %s", cp->m_bookmark_list[j]->get_title());
				ret = cp->m_bookmark->update_bookmark(cp->m_bookmark_list[j]->get_id()
						, cp->m_bookmark_list[j]->get_title()
						, cp->m_bookmark_list[j]->get_uri()
						, cp->m_curr_folder
						, bookmark_list_size - j);
				if (ret < 0) {
					BROWSER_LOGD("modify_bookmark_order_index failed");
					cp->_close_processing_popup();
					break;
				}
			}
		}
		break;
	}
	case EDIT_FOLDER_DELETE_VIEW:
	{
		unsigned int bookmark_list_size = cp->m_bookmark_list.size();
		for (unsigned int j = 0 ; j < bookmark_list_size ; j++ ) {
			if (ecore_thread_check(cp->m_current_thread)) {
				BROWSER_LOGE("Cancelling the thread");
				cp->m_current_thread = NULL;
				return;
			}
			if (cp->m_bookmark_list[j]->is_modified()) {
				cp->m_bookmark->delete_by_id(cp->m_bookmark_list[j]->get_id());
				cp->m_count_checked_item--;
				if (cp->m_count_checked_item <= 0)
					break;
			}
		}
		break;
	}
	default:
		break;
	}
}

void bookmark_edit_view::__thread_end_cb(void *data, Ecore_Thread *th)
{
	BROWSER_LOGD("");

	m_current_thread = NULL;
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	cp->_close_processing_popup();
	cp->_back_to_previous_view();
}

void bookmark_edit_view::__thread_cancel_cb(void *data, Ecore_Thread *th)
{
	BROWSER_LOGD("");

	m_current_thread = NULL;
}

void bookmark_edit_view::_show_processing_popup(void)
{
	BROWSER_LOGD("");
	if (m_popup_processing) {
		evas_object_del(m_popup_processing);
		m_popup_processing = NULL;
	}

	/*if (m_toolbar_item)
		elm_object_item_disabled_set(m_toolbar_item, EINA_TRUE);*/
	m_popup_processing = brui_popup_add(m_naviframe);
	RET_MSG_IF(!m_popup_processing, "m_popup_processing is NULL");
	if (m_genlist) {
		evas_object_freeze_events_set(m_genlist, EINA_TRUE);
	}

	Evas_Object *pgbar_outer_layout = elm_layout_add(m_popup_processing);
	elm_layout_file_set(pgbar_outer_layout, browser_edj_dir"/browser-popup-lite.edj", "processing_popup");
	evas_object_size_hint_weight_set(pgbar_outer_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_progressbar = elm_progressbar_add(m_popup_processing);
	elm_progressbar_pulse(m_progressbar, EINA_TRUE);
	elm_object_style_set(m_progressbar, "pending");

	elm_progressbar_horizontal_set(m_progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(m_progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(m_progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_current_thread = ecore_thread_run(__processing_popup_thread_cb, __thread_end_cb, __thread_cancel_cb, this);
	evas_object_show(m_progressbar);

	elm_object_part_content_set(pgbar_outer_layout, "elm.swallow.content", m_progressbar);
	elm_object_content_set(m_popup_processing, pgbar_outer_layout);

	/*
	 * TODO: The line below should be commented out as long as EFL Elementary
	 * popup does not work correctly on Tizen platform.
	 *
	 * Issue: http://107.108.218.239/bugzilla/show_bug.cgi?id=11724
	 *
	 */
	// evas_object_show(m_popup_processing);
}

void bookmark_edit_view::apply_changed_language()
{
	BROWSER_LOGD("");

	/* change titles */
	if (m_count_checked_item)
		_set_selected_title();
	else
		_unset_selected_title();
}

void bookmark_edit_view::_set_selected_title()
{
	BROWSER_LOGD("count : %d", m_count_checked_item);
	m_sub_title.clear();

	char label_count[1024] = {'\0', };
	snprintf(label_count, sizeof(label_count), "%d", (m_count_checked_item));

	char *text = NULL;
	int len = strlen(label_count) + strlen(_("IDS_BR_HEADER_PD_SELECTED_ABB")) + 1;
	text = (char *)malloc(len * sizeof(char));
	RET_MSG_IF(!text, "text is NULL");
	memset(text, 0x00, len);
	snprintf(text, len, _("IDS_BR_HEADER_PD_SELECTED_ABB"), m_count_checked_item);
	m_sub_title.append(text);

	if(text)
		free(text);

	elm_object_item_part_text_set(m_naviframe_item, "default", m_sub_title.c_str());
}

void bookmark_edit_view::_unset_selected_title()
{
	if (!m_naviframe_item)
		return;

	switch (m_view_mode) {
	case EDIT_FOLDER_REORDER_VIEW:
		elm_object_item_part_text_set(m_naviframe_item, "default", BR_STRING_REORDER);
		break;
	case EDIT_FOLDER_DELETE_VIEW:
		_set_selected_title();
		break;
	default:
		elm_object_item_part_text_set(m_naviframe_item, "default", BR_STRING_EDIT);
		break;
	}
}

void bookmark_edit_view::_reorder_bookmark_items_all(void)
{
	BROWSER_LOGD("");
	Elm_Object_Item *it = NULL;
	gl_cb_data *cb_data = NULL;
	int ret = -1;

	if (!m_reorder_dirty_flag)
		return;

	switch (m_view_mode) {
	case EDIT_FOLDER_REORDER_VIEW:
	{
		if (!m_genlist)
			return;
		unsigned int sequence = 1;
		it = elm_genlist_first_item_get(m_genlist);
		while(it) {
			cb_data = (gl_cb_data *)elm_object_item_data_get(it);
			bookmark_item *item_data = (bookmark_item *)cb_data->user_data;
			item_data->set_order(sequence);
			ret = m_bookmark->update_bookmark(item_data->get_id()
					, item_data->get_title()
					, item_data->get_uri()
					, m_curr_folder
					, item_data->get_order());
			if (ret < 0)
				BROWSER_LOGD("modify_bookmark_order_index failed");
			BROWSER_LOGD("[%s] %d", item_data->get_title(), item_data->get_order());
			it = elm_genlist_item_next_get(it);
			sequence++;
		}
		break;
	}
	default:
		break;
	}

	m_reorder_dirty_flag = 0;
}

void bookmark_edit_view::_reorder_bookmark_items(bookmark_item *moved_item, int current_order)
{
	BROWSER_LOGD("current_order %d",current_order);

	int prev_order = moved_item->get_order();
	if (current_order == prev_order)
		return;
	moved_item->set_order(current_order);

	m_bookmark_list.erase(m_bookmark_list.begin() + prev_order);
	std::vector<bookmark_item *>::iterator it = m_bookmark_list.begin() + current_order;
	m_bookmark_list.insert(it, moved_item);

	int index = -1;
	if (current_order < prev_order) {
		//Item is moved up
		index = current_order + 1;
		for (int i = 0; i < prev_order - current_order; i++) {
			m_bookmark_list[index]->set_order(index);
			index ++;
		}
	} else {
		//Item is moved down
		index = current_order - 1;
		for (int i =0; i < current_order - prev_order; i++) {
			m_bookmark_list[index]->set_order(index);
			index --;
		}
	}
}

Eina_Bool bookmark_edit_view::_is_disable_done_button(bookmark_item* moved_item, int current_order)
{
	BROWSER_LOGD("");

	//Check whether the current moved item is brought back to its initial position
	if (current_order == moved_item->get_initial_order()) {
		unsigned int bookmark_list_size = m_bookmark_list.size();
		for (unsigned int i = 0; i < bookmark_list_size; i++) {
			if (m_bookmark_list[i]->get_order() != m_bookmark_list[i]->get_initial_order())
				return EINA_FALSE;
		}
	} else
		return EINA_FALSE;

	return EINA_TRUE;
}

void bookmark_edit_view::_stat_checked_item(Eina_Bool check_state, void *data)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)cb_data->user_data;

	if (check_state) {
		m_count_checked_item++;
	} else {
		m_count_checked_item--;
	}
	bookmark_item_data->set_modified(check_state);

	if (m_count_checked_item == 0) {
		elm_object_disabled_set(m_btn_done, EINA_TRUE);
	} else {
		elm_object_disabled_set(m_btn_done, EINA_FALSE);
	}
	BROWSER_LOGD("m_count_checked_item: %d",m_count_checked_item);
}

void bookmark_edit_view::_do_before_naviframe_pop(void)
{
	BROWSER_LOGD("");

	if (m_cb_func) {
		m_cb_func(m_cb_data, NULL, NULL);
	}
}

void bookmark_edit_view::_back_to_previous_view(void)
{
	BROWSER_LOGD("");

	if (elm_naviframe_bottom_item_get(m_naviframe) !=
						elm_naviframe_top_item_get(m_naviframe))
					elm_naviframe_item_pop(m_naviframe);
}

void bookmark_edit_view::_back_to_browser_view(void)
{
	BROWSER_LOGD("");
	Elm_Object_Item *bottom_item = elm_naviframe_bottom_item_get(m_naviframe);

	elm_naviframe_item_pop_to(bottom_item);
}

void bookmark_edit_view::__genlist_del_cb(void *data, Evas *e, Evas_Object *genlist, void *ei)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!genlist, "genlist is NULL");
	RET_MSG_IF(!data, "data is NULL");

	Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
	while (it) {
		gl_cb_data *item_data = (gl_cb_data *)elm_object_item_data_get(it);
		if (item_data) {
			free(item_data);
		}
		it = elm_genlist_item_next_get(it);
	}
	elm_genlist_clear(genlist);

	Elm_Genlist_Item_Class *itc_folder = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_folder");
	if (itc_folder) elm_genlist_item_class_free(itc_folder);

	Elm_Genlist_Item_Class *itc_bookmark_folder = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_bookmark_folder");
	if (itc_bookmark_folder) elm_genlist_item_class_free(itc_bookmark_folder);
}

Eina_Bool bookmark_edit_view::_set_genlist_folder_view(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!genlist, EINA_FALSE, "genlist is NULL");

	Elm_Genlist_Item_Class *itc_bookmark_folder = elm_genlist_item_class_new();
	if(itc_bookmark_folder) {
		itc_bookmark_folder->item_style = "2line.top";
		itc_bookmark_folder->func.text_get = __genlist_get_text_cb;
		itc_bookmark_folder->func.content_get = __genlist_get_content_cb;
		itc_bookmark_folder->func.state_get = NULL;
		itc_bookmark_folder->func.del = NULL;
		itc_bookmark_folder->decorate_all_item_style = NULL;
	}
	evas_object_data_set(genlist, "itc_bookmark_folder", itc_bookmark_folder);

	Elm_Genlist_Item_Class *itc_bookmark_reorder_folder = elm_genlist_item_class_new();
	if(itc_bookmark_reorder_folder) {
		itc_bookmark_reorder_folder->item_style = "2line.top";
		itc_bookmark_reorder_folder->func.text_get = __genlist_get_text_cb;
		itc_bookmark_reorder_folder->func.content_get = __genlist_get_content_cb;
		itc_bookmark_reorder_folder->func.state_get = NULL;
		itc_bookmark_reorder_folder->func.del = NULL;
		itc_bookmark_reorder_folder->decorate_all_item_style =  "edit_default";
	}
	evas_object_data_set(genlist, "itc_bookmark_reorder_folder", itc_bookmark_reorder_folder);

	Elm_Genlist_Item_Class *itc_folder = elm_genlist_item_class_new();
	if(itc_folder) {
		itc_folder->item_style = "1line";
		itc_folder->func.text_get = __genlist_get_text_cb;
		itc_folder->func.content_get = __genlist_get_content_cb;
		itc_folder->func.state_get = NULL;
		itc_folder->func.del = NULL;
		itc_folder->decorate_all_item_style =  NULL;
	}
	evas_object_data_set(genlist, "itc_folder", itc_folder);

	Elm_Genlist_Item_Class *itc_reorder_folder = elm_genlist_item_class_new();
	if(itc_reorder_folder){
		itc_reorder_folder->item_style = "1line";
		itc_reorder_folder->func.text_get = __genlist_get_text_cb;
		itc_reorder_folder->func.content_get = __genlist_get_content_cb;
		itc_reorder_folder->func.state_get = NULL;
		itc_reorder_folder->func.del = NULL;
		itc_reorder_folder->decorate_all_item_style = "edit_default";
	}
	evas_object_data_set(genlist, "itc_reorder_folder", itc_reorder_folder);

	evas_object_event_callback_add(genlist, EVAS_CALLBACK_DEL, __genlist_del_cb, this);
	_set_genlist_by_folder(m_curr_folder, genlist);
	return EINA_TRUE;
}

Eina_Bool bookmark_edit_view::_set_genlist_by_folder(int folder_id,
							Evas_Object *genlist)
{
	BROWSER_LOGD("");

	Eina_Bool ret;
	Elm_Genlist_Item_Class *itc = NULL;

	ret = m_bookmark->get_list_by_folder(folder_id, m_bookmark_list);

	if (ret == EINA_FALSE) {
		BROWSER_LOGE("get_list_by_folder is failed(folder id:%d)",folder_id);
		return EINA_FALSE;
	}

	unsigned int bookmark_list_size = m_bookmark_list.size();
	for(unsigned int j = 0 ; j < bookmark_list_size ; j++ ) {
		BROWSER_SECURE_LOGD("bookmark[%d] is %s\n", j, m_bookmark_list[j]->get_title());
		gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
		if (item_data) {
			memset(item_data, 0x00, sizeof(gl_cb_data));

			m_bookmark_list[j]->set_initial_order(j);
			m_bookmark_list[j]->set_order(j);
			/* Folder item is found. get sub list */
			BROWSER_SECURE_LOGD("Title[%d] is %s(id: %d)\n", j,
					m_bookmark_list[j]->get_title(),
					m_bookmark_list[j]->get_id());
			item_data->cp = this;
			item_data->user_data = (void *)m_bookmark_list[j];

			if (m_bookmark_list[j]->is_folder()) {
				/* Folder */
				BROWSER_SECURE_LOGD("Folder[%d] is %s(id: %d)\n",
					j, m_bookmark_list[j]->get_title(), m_bookmark_list[j]->get_id());
				if (m_view_mode == EDIT_FOLDER_REORDER_VIEW)
					itc = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_reorder_folder");
				else
					itc = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_folder");
				item_data->it = elm_genlist_item_append(genlist,
					itc, item_data, NULL,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
			} else {
				BROWSER_SECURE_LOGD("bookmark[%d] is %s\n", j, m_bookmark_list[j]->get_title());
				if (m_view_mode == EDIT_FOLDER_REORDER_VIEW)
					itc = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_bookmark_reorder_folder");
				else
					itc = (Elm_Genlist_Item_Class *)evas_object_data_get(genlist, "itc_bookmark_folder");
				if(m_bookmark_list[j]->is_editable())
					item_data->it = elm_genlist_item_append(genlist, itc, item_data, NULL,
						ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
				else {
					BROWSER_SECURE_LOGD("bookmark is not editable");
					free(item_data);
				}
			}
		}
	}

	return EINA_TRUE;
}

void bookmark_edit_view::_stay_current_folder(void)
{
	BROWSER_LOGD("");
	_set_contents();

	if (!m_genlist)
		return;
}

Evas_Object *bookmark_edit_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	RETV_MSG_IF(!genlist, NULL, "elm_genlist_add is failed");

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, NULL);
	evas_object_smart_callback_add(genlist, "language,changed", common_view::__genlist_lang_changed, NULL);

	return genlist;
}

Evas_Object *bookmark_edit_view::_create_no_content(Evas_Object *parent, const char *text)
{
	RETV_MSG_IF(!parent, NULL, "parent is NULL");
	Evas_Object *nocontent = elm_layout_add(parent);
	if (!nocontent) {
		BROWSER_LOGD("elm_layout_add is failed");
		return NULL;
	}
	elm_layout_file_set(nocontent, bookmark_view_edj_path, "elm/layout/nocontents/bookmark");
	elm_object_focus_set(nocontent, EINA_FALSE);
	elm_object_part_text_set(nocontent, "elm.text", text);
	return nocontent;
}

Evas_Object *bookmark_edit_view::_create_box(Evas_Object * parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *box = NULL;
	box = elm_box_add(parent);
	RETV_MSG_IF(!box, NULL, "box is NULL");
	elm_object_focus_set(box, EINA_FALSE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_clear(box);
	evas_object_show(box);
	return box;
}

void bookmark_edit_view::__select_all_key_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	cp->__select_all_btn_clicked_cb(cp, obj, event_info, EINA_TRUE);
}

void bookmark_edit_view::__select_all_key_down_checkbox_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	cp->__select_all_btn_clicked_cb(cp, obj, event_info, EINA_FALSE);
}

void bookmark_edit_view::_set_select_all_layout(void)
{
	BROWSER_LOGD("");
	if (m_select_all_layout) {
		elm_box_unpack(m_box, m_select_all_layout);
		evas_object_del(m_select_all_layout);
		m_select_all_layout = NULL;
	}

	m_select_all_layout = elm_layout_add(m_outer_layout);
	if (!m_select_all_layout) {
		BROWSER_LOGD("elm_layout_add is failed");
		return;
	}

	elm_object_focus_allow_set(m_select_all_layout, EINA_TRUE);

	elm_layout_theme_set(m_select_all_layout, "genlist", "item", "select_all/default");
	evas_object_size_hint_weight_set(m_select_all_layout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(m_select_all_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *check = elm_check_add(m_select_all_layout);
	evas_object_propagate_events_set(check, EINA_FALSE);
	elm_object_part_content_set(m_select_all_layout, "elm.icon", check);
	elm_object_part_text_set(m_select_all_layout, "elm.text.main", BR_STRING_SELECT_ALL);
	evas_object_show(m_select_all_layout);
	elm_box_pack_start(m_box,m_select_all_layout);

	evas_object_event_callback_add(m_select_all_layout, EVAS_CALLBACK_MOUSE_DOWN, __select_all_key_down_cb, this);
	evas_object_event_callback_add(check, EVAS_CALLBACK_MOUSE_DOWN, __select_all_key_down_checkbox_cb, this);

BROWSER_LOGD("");
}

void bookmark_edit_view::_set_contents()
{
	BROWSER_LOGD("");
	Evas_Object *content = NULL;

	std::vector<bookmark_item *> bookmark_list;
	Eina_Bool ret = m_bookmark->get_list_by_folder(m_bookmark->get_root_folder_id(), bookmark_list);

	if (ret == EINA_FALSE) {
		BROWSER_LOGE("get_list_by_folder is failed");
		return;
	}

	elm_object_part_content_unset(m_main_layout, "elm.swallow.contents");
	if (m_genlist != NULL) {
		evas_object_del(m_genlist);
		m_genlist = NULL;
	}

	if (m_no_contents_layout != NULL) {
		evas_object_del(m_no_contents_layout);
		m_no_contents_layout = NULL;
	}
	m_count_checked_item = 0;

	switch (m_view_mode) {
	case EDIT_FOLDER_REORDER_VIEW:
		content = m_genlist = _create_genlist(m_main_layout);
		if (!m_genlist)
			return;
		_set_genlist_folder_view(m_genlist);
		elm_genlist_decorate_mode_set(m_genlist, EINA_FALSE);
		elm_genlist_homogeneous_set(m_genlist, EINA_TRUE);
		evas_object_smart_callback_add(m_genlist, "moved", __genlist_moved_cb, this);
		break;
	case EDIT_FOLDER_DELETE_VIEW:
		content = m_genlist = _create_genlist(m_main_layout);
		if (!m_genlist)
			return;
		_set_genlist_folder_view(m_genlist);
		elm_genlist_homogeneous_set(m_genlist, EINA_TRUE);
		_set_select_all_layout();
		BROWSER_LOGD("");
		break;
	default:
		break;
	}

	evas_object_show(content);
	elm_object_part_content_set(m_main_layout, "elm.swallow.contents", content);
	BROWSER_LOGD("");
}

void bookmark_edit_view::refresh()
{
	BROWSER_LOGD("");
	_set_contents();
}

void bookmark_edit_view::show()
{
	BROWSER_LOGD("");

	m_outer_layout = elm_layout_add(m_naviframe);
	RET_MSG_IF(!m_outer_layout, "m_main_layout is NULL");
	elm_object_focus_set(m_outer_layout, EINA_FALSE);
	elm_layout_file_set(m_outer_layout, bookmark_view_edj_path, "edit-layout");
	evas_object_size_hint_weight_set(m_outer_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_outer_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_outer_layout);
#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(m_outer_layout, EEXT_CALLBACK_BACK, __cancel_btn_clicked_cb, this);
#endif

	m_box = _create_box(m_outer_layout);
	elm_object_part_content_set(m_outer_layout, "elm.swallow.contents", m_box);

	m_main_layout = elm_layout_add(m_box);
	RET_MSG_IF(!m_main_layout, "m_main_layout is NULL");
	elm_object_focus_set(m_main_layout, EINA_FALSE);
	elm_layout_file_set(m_main_layout, bookmark_edit_view_edj_path, "contents");
	evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_main_layout);

	elm_box_pack_end(m_box, m_main_layout);

	switch (m_view_mode) {
	case EDIT_FOLDER_REORDER_VIEW:
		m_naviframe_item = elm_naviframe_item_push(m_naviframe,
						"IDS_BR_OPT_REORDER_ABB", NULL, NULL, m_outer_layout, NULL);
		elm_object_item_domain_text_translatable_set(m_naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);
		break;
	case EDIT_FOLDER_DELETE_VIEW:
		m_naviframe_item = elm_naviframe_item_push(m_naviframe,
						"IDS_BR_HEADER_SELECT_BOOKMARK", NULL, NULL, m_outer_layout, NULL);
		_set_selected_title();
		elm_object_item_domain_text_translatable_set(m_naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);
		break;
	default:
		m_naviframe_item = elm_naviframe_item_push(m_naviframe,
						BR_STRING_EDIT, NULL, NULL, m_outer_layout, NULL);
		break;
	}
	elm_naviframe_item_pop_cb_set(m_naviframe_item, __naviframe_pop_cb, this);

	evas_object_smart_callback_add(m_naviframe,
					"transition,finished",
					__naviframe_pop_finished_cb, this);

	m_btn_cancel = elm_button_add(m_naviframe);
	if (!m_btn_cancel) return;
	elm_object_style_set(m_btn_cancel, "naviframe/title_cancel");
	evas_object_smart_callback_add(m_btn_cancel, "clicked", __cancel_btn_clicked_cb, this);
	elm_object_item_part_content_set(m_naviframe_item, "title_left_btn", m_btn_cancel);

	m_btn_done = elm_button_add(m_naviframe);
	if (!m_btn_done) return;
	elm_object_style_set(m_btn_done, "naviframe/title_done");
	evas_object_smart_callback_add(m_btn_done, "clicked", __done_btn_clicked_cb, this);
	elm_object_item_part_content_set(m_naviframe_item, "title_right_btn", m_btn_done);
	elm_object_disabled_set(m_btn_done, EINA_TRUE);

	_stay_current_folder();
BROWSER_LOGD("");
}


