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
#include "bookmark-view.h"
#include "bookmark-edit-view.h"
#include "bookmark-create-folder-view.h"
#include "bookmark-add-view.h"
#include "bookmark-select-folder-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"

typedef struct _gl_cb_data {
	void *user_data;
	Eina_Bool is_checked;
	void *cp;
	Elm_Object_Item *it;
} gl_cb_data;

#define bookmark_edit_view_edj_path browser_edj_dir"/bookmark-edit-view.edj"

bookmark_edit_view::bookmark_edit_view(bool tag_mode)
:
	m_box(NULL)
	,m_main_layout(NULL)
	,m_genlist(NULL)
	,m_titlebar_btn_select_all(NULL)
	,m_toolbar_btn_more(NULL)
	,m_toolbar_btn_move(NULL)
	,m_toolbar_btn_delete(NULL)
	,m_toolbar_btn_back(NULL)
	,m_naviframe_item(NULL)
	,m_count_checked_item(0)
	,m_count_editable_item(0)
	,m_count_folder_item(0)
#if defined(BROWSER_TAG)
	,m_toolbar_btn_remove_tag(NULL)
#endif
{
	BROWSER_LOGD("tag_mode: %d", tag_mode);
	m_bookmark = m_browser->get_bookmark();
	m_curr_folder = m_bookmark->get_root_folder_id();
	m_folder_id_to_move = m_bookmark->get_root_folder_id();
	m_bookmark_list.clear();
#if defined(BROWSER_TAG)
	if (tag_mode)
		m_view_mode = EDIT_TAG_VIEW;
	else
#endif
		m_view_mode = EDIT_FOLDER_VIEW;
}

bookmark_edit_view::~bookmark_edit_view(void)
{
	BROWSER_LOGD("");
	_clear_genlist_item_data(m_genlist, m_view_mode);
	elm_object_style_set(m_bg, "default");
	evas_object_smart_callback_del(m_naviframe,
						"transition,finished",
						__naviframe_pop_finished_cb);
}

char *bookmark_edit_view::__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)cb_data->user_data;

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

Evas_Object *bookmark_edit_view::__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_edit_view *cp = (bookmark_edit_view *)cb_data->cp;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.edit")) {
		Evas_Object *folder_icon = elm_icon_add(obj);
		if (folder_icon == NULL)
			return NULL;
		elm_icon_standard_set(folder_icon, browser_img_dir"/bookmark_icon_folder.png");
		evas_object_size_hint_aspect_set(folder_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return folder_icon;
	}

	Evas_Object *content = NULL;
	if (elm_genlist_decorate_mode_get(obj)) {
		if (!strcmp(part, "elm.edit.icon.1")) {
			// swallow checkbox
			content = elm_check_add(obj);
			// keep & revoke the state from integer pointer
			elm_check_state_pointer_set(content, &(cb_data->is_checked));
			evas_object_smart_callback_add(content, "changed", __chk_changed_cb, cb_data);
		} else if (!strcmp(part, "elm.edit.icon.2")) {
			switch (cp->m_view_mode) {
#if defined(BROWSER_TAG)
			case EDIT_TAG_VIEW:
				content = elm_button_add(obj);
				elm_object_style_set(content, "reveal");
				evas_object_smart_callback_add(content, "clicked",
												__edit_bookmark_btn_cb, cb_data);
				break;
#endif
			case EDIT_FOLDER_VIEW:
			{
				content = elm_button_add(obj);
				bookmark_item *item = (bookmark_item *)cb_data->user_data;
				if (item->is_folder()) {
					elm_object_style_set(content, "send");
				evas_object_smart_callback_add(content, "clicked",
												__go_into_sub_folder_btn_cb, cb_data);
				} else {
					elm_object_style_set(content, "reveal");
					evas_object_smart_callback_add(content, "clicked",
												__edit_bookmark_btn_cb, cb_data);
				}
				break;
			}
			default:
				break;
			}
		}
		evas_object_size_hint_aspect_set(content, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		evas_object_propagate_events_set(content, EINA_FALSE);
		evas_object_repeat_events_set(content, EINA_FALSE);
		return content;
	}
	return NULL;
}

void bookmark_edit_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	switch (cp->m_view_mode) {
	case EDIT_FOLDER_VIEW:
	{
		gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);

		cb_data->is_checked = !(cb_data->is_checked);
		if (cb_data->is_checked) {
			/* CHECKED */
			BROWSER_LOGD("CHECKED");
		} else {
			/* UNCHECKED */
			BROWSER_LOGD("UNCHECKED");
		}
		cp->_stat_checked_item(cb_data->is_checked, cb_data);
		elm_genlist_item_update(it);
		break;
	}
	default:
		break;
	}

	elm_genlist_item_selected_set(it, EINA_FALSE);
}

void bookmark_edit_view::__genlist_moved_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	EINA_SAFETY_ON_NULL_RETURN(event_info);
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
	EINA_SAFETY_ON_NULL_RETURN(cb_data);

	bookmark_item *item_data = (bookmark_item *)cb_data->user_data;
	EINA_SAFETY_ON_NULL_RETURN(item_data);

	BROWSER_LOGD("cur item : %s", item_data->get_title());
	bookmark_edit_view *cp = (bookmark_edit_view *)cb_data->cp;
	Elm_Object_Item *next_it = elm_genlist_item_next_get(it);
	gl_cb_data *next_it_cb_data = NULL;

	bookmark_item *next_item_data = NULL;
	if (next_it) {
		next_it_cb_data =  (gl_cb_data *)elm_object_item_data_get(next_it);
		if (next_it_cb_data) {
			next_item_data = (bookmark_item *)next_it_cb_data->user_data;
			BROWSER_LOGD("next item : %s", next_item_data->get_title());
		}
	}

	gl_cb_data *test_it_cb_data = NULL;
	Elm_Object_Item *index_it = elm_genlist_first_item_get(obj);
	while (index_it) {
		test_it_cb_data = (gl_cb_data *)elm_object_item_data_get(index_it);
		if (test_it_cb_data) {
			bookmark_item *test_item_data = (bookmark_item *)test_it_cb_data->user_data;
			BROWSER_LOGD("Test item :%s, order index=%d", test_item_data->get_title(), test_item_data->get_order());
		}
		index_it = elm_genlist_item_next_get(index_it);
	}

	if (next_item_data && (item_data->get_order() < next_item_data->get_order()))
		/* move down */
		cp->_reorder_bookmark_items(item_data->get_order(), EINA_TRUE);
	else if (!next_item_data)
		/* move to bottom */
		cp->_reorder_bookmark_items(item_data->get_order(), EINA_TRUE);
	else
		/* move up */
		cp->_reorder_bookmark_items(item_data->get_order(), EINA_FALSE);
}

void bookmark_edit_view::__chk_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_edit_view *cp = (bookmark_edit_view *)cb_data->cp;

	Eina_Bool state = elm_check_state_get(obj);

	if (state) {
		/* CHECKED */
		BROWSER_LOGD("CHECKED");
	} else {
		/* UNCHECKED */
		BROWSER_LOGD("UNCHECKED");
	}
	cp->_stat_checked_item(state, cb_data);
}

void bookmark_edit_view::__back_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;

	switch (cp->m_view_mode) {
	case EDIT_FOLDER_VIEW:
		if (!(cp->_go_to_upper_folder()))
			cp->_back_to_previous_view();
		break;
	default:
		cp->_back_to_previous_view();
		break;
	}
}

void bookmark_edit_view::__select_all_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	switch (cp->m_view_mode) {
	case EDIT_FOLDER_VIEW:
	{
		if (cp->m_count_checked_item == elm_genlist_items_count(cp->m_genlist)) {
			Elm_Object_Item *it = elm_genlist_first_item_get(cp->m_genlist);
			while (it) {
				gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
				if (cb_data) {
					if (cb_data->is_checked == EINA_TRUE) {
						cb_data->is_checked = EINA_FALSE;
						cp->_stat_checked_item(EINA_FALSE, cb_data);
						elm_genlist_item_update(it);
					}
				}
				it = elm_genlist_item_next_get(it);
			}
		} else {
			Elm_Object_Item *it = elm_genlist_first_item_get(cp->m_genlist);
			while (it) {
				gl_cb_data *cb_data = (gl_cb_data *)elm_object_item_data_get(it);
				if (cb_data) {
					if (cb_data->is_checked == EINA_FALSE) {
						cb_data->is_checked = EINA_TRUE;
						cp->_stat_checked_item(EINA_TRUE, cb_data);
						elm_genlist_item_update(it);
					}
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

void bookmark_edit_view::__move_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	m_browser->create_bookmark_select_folder_view(cp->__select_folder_cb, cp, EINA_TRUE)->show();
}

void bookmark_edit_view::__delete_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	cp->_show_delete_confirm_popup();
}

void bookmark_edit_view::__more_btn_clicked_cb(
									void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	cp->_show_more_context_popup(obj);
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
}

void bookmark_edit_view::__ctxpopup_add_new_folder_by_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	cp->m_browser->get_bookmark_create_folder_view(__create_folder_cb, cp)->show();
	evas_object_del(obj);
}

void bookmark_edit_view::__create_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	cp->_set_contents();
}

void bookmark_edit_view::__ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	evas_object_del(obj);
}

void bookmark_edit_view::__ok_response_delete_confirm_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	cp->_delete_selected_items();
}

void bookmark_edit_view::__ok_response_move_confirm_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	cp->_move_selected_items();
}

void bookmark_edit_view::__edit_bookmark_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)cb_data->user_data;

	m_browser->create_bookmark_add_view(NULL, bookmark_item_data->get_uri(), 0, EINA_TRUE)->show();
}

void bookmark_edit_view::__go_into_sub_folder_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_edit_view *cp = (bookmark_edit_view *)cb_data->cp;
	bookmark_item *bookmark_item_data = (bookmark_item *)cb_data->user_data;

	cp->_go_into_sub_folder(bookmark_item_data->get_id(), bookmark_item_data->get_title());
}

void bookmark_edit_view::__select_folder_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	bookmark_edit_view *cp = (bookmark_edit_view *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)event_info;

	cp->m_folder_id_to_move = bookmark_item_data->get_id();
	cp->_show_move_confirm_popup();
}

void bookmark_edit_view::_reorder_bookmark_items(int order_index, Eina_Bool is_move_down)
{
	BROWSER_LOGD("");

	Elm_Object_Item *it = NULL;
	gl_cb_data *cb_data = NULL;
	Eina_Bool ret = EINA_TRUE;
	int index = order_index;

	if (is_move_down) {
		BROWSER_LOGD("Item is moved down");
		it = elm_genlist_first_item_get(m_genlist);
		while(it) {
			cb_data = (gl_cb_data *)elm_object_item_data_get(it);
			bookmark_item *item_data = (bookmark_item *)cb_data->user_data;
			if (item_data->get_order() > index) {
				m_bookmark->update_bookmark(item_data->get_id()
							, item_data->get_title()
							, item_data->get_uri()
							, m_curr_folder
							, index);
				if (!ret)
					BROWSER_LOGD("modify_bookmark_order_index failed");
				int temp = item_data->get_order();
				item_data->set_order(index);
				index = temp;
				elm_object_item_data_set(it, cb_data);
			} else if (item_data->get_order() == order_index) {
				m_bookmark->update_bookmark(item_data->get_id()
							, item_data->get_title()
							, item_data->get_uri()
							, m_curr_folder
							, index);
				item_data->set_order(index);
				elm_object_item_data_set(it, cb_data);
				if (!ret)
					BROWSER_LOGD("modify_bookmark_order_index failed");
				break;
			}
			it = elm_genlist_item_next_get(it);
		}
	} else {
		BROWSER_LOGD("Item is moved up");
		index = order_index;
		it = elm_genlist_last_item_get(m_genlist);
		while(it) {
			cb_data = (gl_cb_data *)elm_object_item_data_get(it);
			bookmark_item *item_data = (bookmark_item *)cb_data->user_data;
			if (item_data->get_order() < index) {
				m_bookmark->update_bookmark(item_data->get_id()
							, item_data->get_title()
							, item_data->get_uri()
							, m_curr_folder
							, index);
				if (!ret)
					BROWSER_LOGD("modify_bookmark_order_index failed");
				int temp = item_data->get_order();
				item_data->set_order(index);
				index = temp;
				elm_object_item_data_set(it, cb_data);
			} else if (item_data->get_order() == order_index) {
				m_bookmark->update_bookmark(item_data->get_id()
							, item_data->get_title()
							, item_data->get_uri()
							, m_curr_folder
							, index);
				item_data->set_order(index);
				elm_object_item_data_set(it, cb_data);
				if (!ret)
					BROWSER_LOGD("modify_bookmark_order_index failed");
				break;
			}
			it = elm_genlist_item_prev_get(it);
		}
	}
}

void bookmark_edit_view::_go_into_sub_folder(int folder_id, const char *folder_name)
{
	BROWSER_LOGD("folder_id: %d", folder_id);
	if (folder_id < 0)
		return;

	folder_info *item = (folder_info *)malloc(sizeof(folder_info));
	memset(item, 0x00, sizeof(folder_info));
	BROWSER_LOGD("item");

	item->folder_id = folder_id;
	if (folder_id == m_bookmark->get_root_folder_id()) {
		item->folder_name = strdup(BR_STRING_MOBILE);
	} else
		item->folder_name = strdup(folder_name);
	m_path_history.push_back(item);
	m_curr_folder = folder_id;
	m_path_string.clear();
	for(unsigned int i = 0 ; i < m_path_history.size() ; i++) {
		if (m_path_history[i]) {
			if (m_path_string.empty()) {
				m_path_string = m_path_history[i]->folder_name;
			} else {
				m_path_string += "/";
				m_path_string += m_path_history[i]->folder_name;
			}
		}
	}
	BROWSER_LOGD("m_path_string: %s", m_path_string.c_str());

	_set_contents();
}

Eina_Bool bookmark_edit_view::_go_to_upper_folder()
{
	BROWSER_LOGD("current_depth: %d", m_path_history.size());

	int curr_depth = m_path_history.size() - 1;
	if (curr_depth > 0) {
		m_curr_folder = m_path_history[curr_depth - 1]->folder_id;
		if (m_path_history[curr_depth] && m_path_history[curr_depth]->folder_name) {
			free(m_path_history[curr_depth]->folder_name);
			free(m_path_history[curr_depth]);
		}
		m_path_history.pop_back();
	} else {
		/* Current folder is root folder */
		if (m_curr_folder != m_bookmark->get_root_folder_id()) {
			BROWSER_LOGE("[ERROR] top folder is not root folder");
			return EINA_TRUE;
		}
		if (m_path_history[curr_depth] && m_path_history[curr_depth]->folder_name) {
			free(m_path_history[curr_depth]->folder_name);
			free(m_path_history[curr_depth]);
		}
		m_path_history.pop_back();
		m_path_string.clear();
		return EINA_FALSE;
	}

	m_path_string.clear();
	for(unsigned int i = 0 ; i < m_path_history.size() ; i++) {
		if (m_path_history[i]) {
			if (m_path_string.empty()) {
				m_path_string = m_path_history[i]->folder_name;
			} else {
				m_path_string += "/";
				m_path_string += m_path_history[i]->folder_name;
			}
		}
	}
	BROWSER_LOGD("m_path_string: %s", m_path_string.c_str());

	_set_contents();

	return EINA_TRUE;
}

void bookmark_edit_view::_delete_selected_items(void)
{
	BROWSER_LOGD("");
	switch (m_view_mode) {
	case EDIT_FOLDER_VIEW:
	{
		Elm_Object_Item *it = elm_genlist_first_item_get(m_genlist);
		while (it) {
			gl_cb_data *item_data = (gl_cb_data *)elm_object_item_data_get(it);
			if (item_data && item_data->user_data) {
				bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;
				if (item_data->is_checked) {
					if (m_bookmark->delete_by_id(bookmark_item_data->get_id())) {
						_stat_checked_item(EINA_FALSE, item_data);
						Elm_Object_Item *it_del = it;
						it = elm_genlist_item_next_get(it);
						delete bookmark_item_data;
						free(item_data);
						elm_object_item_del(it_del);
					} else {
						/* delete failed : db error or uneditable item - just skip */
						it = elm_genlist_item_next_get(it);
					}
				} else {
					it = elm_genlist_item_next_get(it);
				}
			}
		}
		break;
	}
	default:
		break;
	}

	if (elm_genlist_items_count(m_genlist) == 0)
		_set_contents();
}

void bookmark_edit_view::_move_selected_items(void)
{
	BROWSER_LOGD("m_genlist (%p)", m_genlist);
	Elm_Object_Item *it = elm_genlist_first_item_get(m_genlist);
	while (it) {
		gl_cb_data *item_data = (gl_cb_data *)elm_object_item_data_get(it);
		if (item_data && item_data->user_data) {
			bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;
			if (item_data->is_checked && (m_folder_id_to_move != m_curr_folder)) {
				if (m_browser->get_bookmark()->update_bookmark(
									bookmark_item_data->get_id(),
									bookmark_item_data->get_title(),
									bookmark_item_data->get_uri(),
									m_folder_id_to_move,
									-1
#if defined(BROWSER_TAG)
									, bookmark_item_data->get_tag1(),
									bookmark_item_data->get_tag2(),
									bookmark_item_data->get_tag3(),
									bookmark_item_data->get_tag4()
#endif
									)) {
					_stat_checked_item(EINA_FALSE, item_data);
					Elm_Object_Item *it_moved = it;
					it = elm_genlist_item_next_get(it);
					delete bookmark_item_data;
					free(item_data);
					elm_object_item_del(it_moved);
				} else {
					/* delete failed : db error or uneditable item - just skip */
					it = elm_genlist_item_next_get(it);
				}
			} else {
				it = elm_genlist_item_next_get(it);
			}
		}
	}

	_set_contents();
}

void bookmark_edit_view::_stat_checked_item(Eina_Bool check_state, void *data)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	gl_cb_data *cb_data = (gl_cb_data *)data;
	bookmark_item *bookmark_item_data = (bookmark_item *)cb_data->user_data;

	if (check_state) {
		m_count_checked_item++;
		if (bookmark_item_data->is_folder())
			m_count_folder_item++;
	} else {
		m_count_checked_item--;
		if (bookmark_item_data->is_folder())
			m_count_folder_item--;
	}

	if (m_count_checked_item == 0) {
		elm_object_disabled_set(m_toolbar_btn_move, EINA_TRUE);
		elm_object_disabled_set(m_toolbar_btn_delete, EINA_TRUE);
	} else {
		if (m_count_folder_item > 0)
			elm_object_disabled_set(m_toolbar_btn_move, EINA_TRUE);
		else
			elm_object_disabled_set(m_toolbar_btn_move, EINA_FALSE);
		elm_object_disabled_set(m_toolbar_btn_delete, EINA_FALSE);
	}
	BROWSER_LOGD("m_count_checked_item: %d",m_count_checked_item);
	BROWSER_LOGD("m_count_folder_item: %d",m_count_folder_item);
}

void bookmark_edit_view::_back_to_previous_view(void)
{
	BROWSER_LOGD("");
	if (elm_naviframe_bottom_item_get(m_naviframe) !=
						elm_naviframe_top_item_get(m_naviframe))
					elm_naviframe_item_pop(m_naviframe);
}

void bookmark_edit_view::_show_more_context_popup(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(parent);
	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", __ctxpopup_dismissed_cb, NULL);

	elm_ctxpopup_item_append(more_popup, BR_STRING_ADD_NEW_FOLDER, NULL, __ctxpopup_add_new_folder_by_cb, this);

	int x, y, w, h;
	evas_object_geometry_get(parent, &x, &y, &w, &h);
	evas_object_move(more_popup, x + (w / 2), y + (h /2));
	evas_object_show(more_popup);
}

void bookmark_edit_view::_show_delete_confirm_popup(void)
{
	BROWSER_LOGD("");
	show_msg_popup(NULL,
					BR_STRING_DELETE_Q,
					BR_STRING_OK,
					__ok_response_delete_confirm_popup_cb,
					BR_STRING_CANCEL,
					NULL,
					this);
}

void bookmark_edit_view::_show_move_confirm_popup(void)
{
	BROWSER_LOGD("");
	show_msg_popup(NULL,
					BR_STRING_MOVE_Q,
					BR_STRING_OK,
					__ok_response_move_confirm_popup_cb,
					BR_STRING_CANCEL,
					NULL,
					this);
}

void bookmark_edit_view::_clear_genlist_item_data(Evas_Object *genlist, edit_view_mode mode)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(genlist);
	switch (mode) {
#if defined(BROWSER_TAG)
	case EDIT_TAG_VIEW:
	{
		Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
		while (it) {
			if (elm_genlist_item_type_get(it) == ELM_GENLIST_ITEM_TREE) {
				/* if this item is a tag */
				gl_cb_data *item_data = (gl_cb_data *)elm_object_item_data_get(it);
				if (item_data && item_data->user_data) {
					char *tag_name = (char *)item_data->user_data;
					free(tag_name);
				}
				free(item_data);
			} else {
				gl_cb_data *item_data = (gl_cb_data *)elm_object_item_data_get(it);
				if (item_data && item_data->user_data) {
					bookmark_item *bookmark_item_data = (bookmark_item *)item_data->user_data;
					delete bookmark_item_data;
				}
				free(item_data);
			}
			it = elm_genlist_item_next_get(it);
		}
		break;
	}
#endif
	case EDIT_FOLDER_VIEW:
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
	default:
		break;
	}
}

Eina_Bool bookmark_edit_view::_set_genlist_folder_view(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(genlist, EINA_FALSE);

	_clear_genlist_item_data(genlist, m_view_mode);
	elm_genlist_clear(genlist);

	//m_itc_folder.item_style = "dialogue/2text.1icon/expandable";
	m_itc_folder.item_style = "1text.1icon.2";
	m_itc_folder.func.text_get = __genlist_get_text_cb;
	m_itc_folder.func.content_get = __genlist_get_content_cb;
	m_itc_folder.func.state_get = NULL;
	m_itc_folder.func.del = NULL;
	m_itc_folder.decorate_all_item_style = "edit_default";

	//m_itc_bookmark_folder.item_style = "dialogue/1text.1icon";
	m_itc_bookmark_folder.item_style = "1text";
	m_itc_bookmark_folder.func.text_get = __genlist_get_text_cb;
	m_itc_bookmark_folder.func.content_get = __genlist_get_content_cb;
	m_itc_bookmark_folder.func.state_get = NULL;
	m_itc_bookmark_folder.func.del = NULL;
	m_itc_bookmark_folder.decorate_all_item_style = "edit_default";

	_set_genlist_by_folder(m_curr_folder, genlist);

	return EINA_TRUE;
}

Eina_Bool bookmark_edit_view::_set_genlist_by_folder(int folder_id,
							Evas_Object *genlist)
{
	BROWSER_LOGD("");
	std::vector<bookmark_item *> bookmark_list;
	Eina_Bool ret;

	ret = m_bookmark->get_list_by_folder(folder_id, bookmark_list);
	if (ret == EINA_FALSE) {
		BROWSER_LOGE("get_list_by_folder is failed(folder id:%d)",folder_id);
		return EINA_FALSE;
	}

	m_count_editable_item = 0;
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
				item_data->it = elm_genlist_item_append(genlist,
					&m_itc_folder, item_data, NULL,
					ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, this);
			}
		} else {
			if (bookmark_item_data->is_editable()) {
				BROWSER_LOGD("bookmark[%d] is %s\n", j, bookmark_list[j]->get_title());
				item_data->it = elm_genlist_item_append(genlist, &m_itc_bookmark_folder, item_data, NULL,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
			}
		}
		if (bookmark_item_data->is_editable())
			m_count_editable_item++;
#if 0
		else
			elm_object_item_disabled_set(item_data->it, EINA_TRUE);
#endif
	}
	m_bookmark->destroy_list(bookmark_list);
	return EINA_TRUE;
}

Evas_Object *bookmark_edit_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
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

Evas_Object *bookmark_edit_view::_create_toolbar_btn(Evas_Object *parent, const char *text, Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) {
		BROWSER_LOGD("elm_button_add is failed");
		return NULL;
	}
	elm_object_text_set(btn, text);
	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

Evas_Object *bookmark_edit_view::_create_no_content(Evas_Object *parent, const char *text)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *nocontent = elm_layout_add(parent);
	if (!nocontent) {
		BROWSER_LOGD("elm_layout_add is failed");
		return NULL;
	}
	elm_layout_theme_set(nocontent, "layout", "nocontents", "text");
	elm_object_focus_set(nocontent, EINA_FALSE);
	elm_object_part_text_set(nocontent, "elm.text", text);
	return nocontent;
}

Evas_Object *bookmark_edit_view::_create_box(Evas_Object * parent)
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

void bookmark_edit_view::_set_contents()
{
	BROWSER_LOGD("");
	Evas_Object *content = NULL;

	Evas_Object *unused = elm_object_part_content_unset(m_main_layout, "elm.swallow.contents");
	if (unused != NULL) {
		/* even though unused is not NULL, it may be not a genlist if item count is 0 */
		if (m_genlist != NULL) {
			_clear_genlist_item_data(m_genlist, m_view_mode);
		}
		evas_object_del(unused);
		unused = NULL;
	}
	m_count_checked_item = 0;
	m_count_folder_item = 0;
	m_genlist = _create_genlist(m_naviframe);
	if (!m_genlist)
		return;
	switch (m_view_mode) {
#if defined(BROWSER_TAG)
	case EDIT_TAG_VIEW:
		BROWSER_LOGE("EDIT_TAG_VIEW");
		_set_genlist_tag_view(m_genlist);
		elm_object_style_set(m_bg, "edit_mode");
		elm_genlist_reorder_mode_set(m_genlist, EINA_FALSE);
		elm_genlist_decorate_mode_set(m_genlist, EINA_TRUE);
		break;
#endif
	case EDIT_FOLDER_VIEW:
		BROWSER_LOGE("EDIT_FOLDER_VIEW");
		_set_genlist_folder_view(m_genlist);
		elm_object_style_set(m_bg, "edit_mode");
		elm_genlist_reorder_mode_set(m_genlist, EINA_TRUE);
		elm_genlist_decorate_mode_set(m_genlist, EINA_TRUE);
		evas_object_smart_callback_add(m_genlist, "moved", __genlist_moved_cb, this);
		break;
	default:
		break;
	}

	if (elm_genlist_items_count(m_genlist) > 0) {
		content = m_genlist;
		elm_object_disabled_set(m_titlebar_btn_select_all, EINA_FALSE);
		elm_object_disabled_set(m_toolbar_btn_move, EINA_TRUE);
		elm_object_disabled_set(m_toolbar_btn_delete, EINA_TRUE);
	} else {
		_clear_genlist_item_data(m_genlist, m_view_mode);
		evas_object_del(m_genlist);
		m_genlist = NULL;
		content = _create_no_content(m_naviframe, BR_STRING_NO_BOOKMARKS);
		elm_object_disabled_set(m_titlebar_btn_select_all, EINA_TRUE);
		elm_object_disabled_set(m_toolbar_btn_move, EINA_TRUE);
		elm_object_disabled_set(m_toolbar_btn_delete, EINA_TRUE);
	}

	evas_object_show(content);
	elm_object_part_content_set(m_main_layout, "elm.swallow.contents", content);
}

void bookmark_edit_view::refresh()
{
	BROWSER_LOGD("");
	_set_contents();
}

void bookmark_edit_view::show()
{
	BROWSER_LOGD("");
	m_box = _create_box(m_naviframe);

	m_main_layout = elm_layout_add(m_box);
	EINA_SAFETY_ON_NULL_RETURN(m_main_layout);
	elm_object_focus_set(m_main_layout, EINA_FALSE);
	elm_layout_file_set(m_main_layout, bookmark_edit_view_edj_path, "main-layout");
	evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_main_layout);

	elm_box_pack_end(m_box, m_main_layout);

	m_toolbar_btn_back = elm_button_add(m_naviframe);
	elm_object_style_set(m_toolbar_btn_back, "naviframe/end_btn/default");
	evas_object_smart_callback_add(m_toolbar_btn_back, "clicked", __back_btn_clicked_cb, this);
	m_naviframe_item = elm_naviframe_item_push(m_naviframe,
						BR_STRING_EDIT, m_toolbar_btn_back, NULL, m_box, NULL);

	evas_object_smart_callback_add(m_naviframe,
					"transition,finished",
					__naviframe_pop_finished_cb, this);

	m_titlebar_btn_select_all = elm_button_add(m_naviframe);
	if (!m_titlebar_btn_select_all) {
		BROWSER_LOGD("elm_button_add is failed");
		return;
	}
	elm_object_style_set(m_titlebar_btn_select_all, "naviframe/title_icon");
	Evas_Object *ic = elm_icon_add(m_naviframe);
	if (!ic) {
		BROWSER_LOGD("elm_icon_add is failed");
		return;
	}
	elm_icon_standard_set(ic, browser_img_dir"/00_icon_edit.png");
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(m_titlebar_btn_select_all, "icon", ic);
	evas_object_smart_callback_add(m_titlebar_btn_select_all,
					"clicked",
					__select_all_btn_clicked_cb,
					this);
	elm_object_item_part_content_set(m_naviframe_item, "title_right_btn", m_titlebar_btn_select_all);

	switch (m_view_mode) {
#if defined(BROWSER_TAG)
	case EDIT_TAG_VIEW:
		m_toolbar_btn_remove_tag = _create_toolbar_btn(m_naviframe, BR_STRING_REMOVE_TAG, __remove_tag_btn_clicked_cb, this);
		elm_object_style_set(m_toolbar_btn_remove_tag, "naviframe/toolbar/default");
		elm_object_item_part_content_set(m_naviframe_item, "toolbar_button1", m_toolbar_btn_remove_tag);
		break;
#endif
	case EDIT_FOLDER_VIEW:
		m_toolbar_btn_more = elm_button_add(m_naviframe);
		if (!m_toolbar_btn_more) {
			BROWSER_LOGD("elm_button_add is failed");
			return;
		}
		elm_object_style_set(m_toolbar_btn_more, "naviframe/more/default");
		evas_object_smart_callback_add(m_toolbar_btn_more, "clicked", __more_btn_clicked_cb, this);
		elm_object_item_part_content_set(m_naviframe_item, "toolbar_more_btn", m_toolbar_btn_more);

		m_toolbar_btn_move = _create_toolbar_btn(m_naviframe, BR_STRING_MOVE, __move_btn_clicked_cb, this);
		elm_object_style_set(m_toolbar_btn_move, "naviframe/toolbar/left");
		elm_object_item_part_content_set(m_naviframe_item, "toolbar_button1", m_toolbar_btn_move);

		m_toolbar_btn_delete = _create_toolbar_btn(m_naviframe, BR_STRING_DELETE, __delete_btn_clicked_cb, this);
		elm_object_style_set(m_toolbar_btn_delete, "naviframe/toolbar/right");
		elm_object_item_part_content_set(m_naviframe_item, "toolbar_button2", m_toolbar_btn_delete);
		break;
	default:
		break;
	}

	_go_into_sub_folder(m_bookmark->get_root_folder_id(), NULL);
}

#if defined(BROWSER_TAG)
char *bookmark_edit_view::__genlist_get_tag_text_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

	gl_cb_data *cb_data = (gl_cb_data *)data;
	if (!strcmp(part, "elm.text")) {
		BROWSER_LOGD("tag[%s]", (char *)cb_data->user_data);
		return elm_entry_utf8_to_markup((char *)cb_data->user_data);
	}

	return NULL;
}

Evas_Object *bookmark_edit_view::__genlist_get_tag_content_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
	gl_cb_data *cb_data = (gl_cb_data *)data;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.edit")) {
		Evas_Object *tag_icon = elm_icon_add(obj);
		if (tag_icon == NULL)
			return NULL;
		elm_icon_standard_set(tag_icon, browser_img_dir"/I01_icon_tag.png");
		evas_object_size_hint_aspect_set(tag_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return tag_icon;
	}

	Evas_Object *content = NULL;
	if (elm_genlist_decorate_mode_get(obj)) {
		if (!strcmp(part, "elm.edit.icon.1")) {
			// swallow checkbox or radio button
			content = elm_check_add(obj);
			// keep & revoke the state from integer pointer
			elm_check_state_pointer_set(content, &(cb_data->is_checked));
			evas_object_propagate_events_set(content, EINA_FALSE);
			//evas_object_smart_callback_add(content, "changed", _chk_changed_cb, id);
		} else if (!strcmp(part, "elm.edit.icon.2")) {
			// swallow rename button if need
			content = elm_button_add(obj);
			elm_object_style_set(content, "rename");
			//if (cb_data->it) evas_object_smart_callback_add(content, "clicked",
			//										_rename_button_cb, cb_data->it);
		}
		evas_object_size_hint_aspect_set(content, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		evas_object_propagate_events_set(content, EINA_FALSE);
		evas_object_repeat_events_set(content, EINA_FALSE);
		return content;
	}
	return NULL;
}

void bookmark_edit_view::__remove_tag_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
}

Eina_Bool bookmark_edit_view::_set_genlist_tag_view(Evas_Object *genlist)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(genlist, EINA_FALSE);

	_clear_genlist_item_data(genlist, m_view_mode);
	elm_genlist_clear(genlist);

	m_itc_tag.item_style = "1text.1icon.2";
	m_itc_tag.func.text_get = __genlist_get_tag_text_cb;
	m_itc_tag.func.content_get = __genlist_get_tag_content_cb;
	m_itc_tag.func.state_get = NULL;
	m_itc_tag.func.del = NULL;
	m_itc_tag.decorate_all_item_style = "edit_default";

	m_itc_bookmark_tag.item_style = "1text";
	m_itc_bookmark_tag.func.text_get = __genlist_get_text_cb;
	m_itc_bookmark_tag.func.content_get = __genlist_get_content_cb;
	m_itc_bookmark_tag.func.state_get = NULL;
	m_itc_bookmark_tag.func.del = NULL;
	m_itc_bookmark_tag.decorate_all_item_style = "edit_default";

	std::vector<char *> tag_list;
	if (m_bookmark->get_tag_list(tag_list) == EINA_FALSE)
		return EINA_FALSE;

	/* Get tagged item list */
	for(unsigned int i = 0 ; i < tag_list.size() ; i++ ) {
		if (tag_list[i]) {
			BROWSER_LOGD("tag[%d] is %s\n", i, tag_list[i]);
			/* Grouptitle - Tag name*/
			gl_cb_data *tag_item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
			memset(tag_item_data, 0x00, sizeof(gl_cb_data));
			tag_item_data->is_checked = EINA_FALSE;
			tag_item_data->cp = this;
			tag_item_data->user_data = (void *)strdup(tag_list[i]);
			Elm_Object_Item *it = elm_genlist_item_append(genlist, &m_itc_tag,
					tag_item_data, NULL,
					ELM_GENLIST_ITEM_TREE, NULL, this);

			/* Items - bookmark items*/
			std::vector<bookmark_item *> bookmark_list;
			m_bookmark->get_list_by_tag(tag_list[i], &bookmark_list);
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
				item_data->is_checked = EINA_FALSE;
				item_data->cp = this;
				item_data->user_data = (void *)bookmark_item_data;
				elm_genlist_item_append(genlist, &m_itc_bookmark_tag, item_data, it,
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
			gl_cb_data *tag_item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
			memset(tag_item_data, 0x00, sizeof(gl_cb_data));
			tag_item_data->is_checked = EINA_FALSE;
			tag_item_data->cp = this;
			tag_item_data->user_data = (void *)strdup(BR_STRING_UNTAGGED);
			untagged_group = elm_genlist_item_append(genlist, &m_itc_tag,
					tag_item_data, NULL,
					ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, this);
	}

	for(unsigned int i = 0 ; i < bookmark_list_untagged.size() ; i++ ) {
		BROWSER_LOGD("untagged bookmark[%d] is %s\n", i, bookmark_list_untagged[i]->get_title());
		gl_cb_data *item_data = (gl_cb_data *)malloc(sizeof(gl_cb_data));
		memset(item_data, 0x00, sizeof(gl_cb_data));

		bookmark_item *bookmark_item_data = new bookmark_item;
		/* deep copying by overloaded operator = */
		*bookmark_item_data = *bookmark_list_untagged[i];
		/* Folder item is found. get sub list */
		BROWSER_LOGD("Title[%d] is %s(id: %d)\n", i,
				bookmark_list_untagged[i]->get_title(),
				bookmark_item_data->get_id());
		item_data->is_checked = EINA_FALSE;
		item_data->cp = this;
		item_data->user_data = (void *)bookmark_item_data;

		elm_genlist_item_append(genlist, &m_itc_bookmark_tag, item_data, untagged_group,
					ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, this);
	}
	m_bookmark->destroy_list(bookmark_list_untagged);

	return EINA_TRUE;
}

#endif
