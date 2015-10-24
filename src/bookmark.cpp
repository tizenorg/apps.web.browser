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

#include <storage.h>

#include "bookmark.h"
#include "bookmark-item.h"
#include "browser.h"
#include "browser-dlog.h"
#include "platform-service.h"
#include "browser-string.h"
#include "web_bookmark.h"

bookmark::bookmark(void)
	: m_memory_full(EINA_FALSE)
	, m_bookmark_adaptor_initialize(EINA_TRUE)
{
	BROWSER_LOGD("");
	if (bp_bookmark_adaptor_initialize() == -1) {
		BROWSER_LOGD("bp_bookmark_adaptor_initialize failed");
		m_bookmark_adaptor_initialize = EINA_FALSE;
	}
	m_curr_folder = get_root_folder_id();
}

bookmark::~bookmark(void)
{
	BROWSER_LOGD("");
	bp_bookmark_adaptor_deinitialize();
	free_path_history();
}

int bookmark::get_root_folder_id(void)
{
	int root_id = 0;
	bp_bookmark_adaptor_get_root(&root_id);
	return root_id;
}

int bookmark::save_bookmark(const char *title, const char *uri,
					int *saved_bookmark_id,
					int parent_id
					)
{
	BROWSER_SECURE_LOGD("title=[%s], uri=[%s]", title, uri);
	RETV_MSG_IF(!(title), -1, "title is NULL");
	RETV_MSG_IF(!(uri), -1, "uri is NULL");

	if (!m_bookmark_adaptor_initialize) {
		if (bp_bookmark_adaptor_initialize() < 0) {
			if (bp_bookmark_adaptor_get_errorcode() == BP_BOOKMARK_ERROR_DISK_FULL)
				m_memory_full = EINA_TRUE;
			else
				m_memory_full = EINA_FALSE;
			return -1;
		} else
			m_bookmark_adaptor_initialize = EINA_TRUE;
	}

	struct statvfs s;
	unsigned long content_size;
	int result;
	result = storage_get_internal_memory_size(&s);
	double available_size = (double)(s.f_bsize) * s.f_bavail;
	m_memory_full = EINA_FALSE;

	content_size = (unsigned long)sizeof(bp_bookmark_info_fmt);

	if (result < 0) {
		BROWSER_LOGE(" Could not get available memory ");
		return -1;
	} else if (available_size < content_size) {
		BROWSER_LOGD(" Not enough Memory ");
		m_memory_full = EINA_TRUE;
		return -1;
	}

	bp_bookmark_property_cond_fmt properties;
	bp_bookmark_rows_cond_fmt conds; //conditions for querying
	memset(&properties, 0x00, sizeof(bp_bookmark_property_cond_fmt));
	memset(&conds, 0x00, sizeof(bp_bookmark_rows_cond_fmt));

	properties.parent = -1;
	properties.type = 0;
	properties.is_operator = -1;
	properties.is_editable = -1;
	conds.limit = 1;
	conds.offset = 0;
	conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
	conds.ordering = 0;
	conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
	conds.period_type = BP_BOOKMARK_DATE_ALL;

	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count,
		&properties, &conds, BP_BOOKMARK_O_URL, uri, 0);

	free(ids);
	if (ret < 0)
		return -1;

	if (ids_count > 0) {
		BROWSER_LOGD("same URI is already exist");
		return 0;
	}

	bp_bookmark_info_fmt info;
	memset(&info, 0x00, sizeof(bp_bookmark_info_fmt));
	info.type = 0;
	info.parent = parent_id;
	info.sequence = -1;
	info.access_count = -1;
	info.editable = 1;
	if (uri != NULL && strlen(uri) > 0) {
		info.url = (char *)uri;
	}
	if (title != NULL && strlen(title) > 0)
		info.title = (char *)title;

	ret = bp_bookmark_adaptor_easy_create(&id, &info);
	if (ret == 0) {
		ret = bp_bookmark_adaptor_set_sequence(id, -1); // max sequence
		if (ret == 0) {
			*saved_bookmark_id = id;
			BROWSER_LOGD("bp_bookmark_adaptor_easy_create is success(id:%d)", *saved_bookmark_id);
			bp_bookmark_adaptor_publish_notification();
			m_browser->notify_bookmark_added(uri, id, parent_id);
			return 1;
		}
	}
	int errcode = bp_bookmark_adaptor_get_errorcode();
	BROWSER_LOGD("bp_bookmark_adaptor_easy_create is failed[%d]", errcode);
	return -1;
}

int bookmark::save_folder(const char *title, int *saved_folder_id, int parent_id)
{
	BROWSER_SECURE_LOGD("title=[%s], parent_id[%d]", title, parent_id);
	RETV_MSG_IF(!(title), -1, "title is NULL");

	bp_bookmark_property_cond_fmt properties;
	bp_bookmark_rows_cond_fmt conds; //conditions for querying
	memset(&properties, 0x00, sizeof(bp_bookmark_property_cond_fmt));
	memset(&conds, 0x00, sizeof(bp_bookmark_rows_cond_fmt));

	properties.parent = parent_id;
	properties.type = 1;
	properties.is_operator = -1;
	properties.is_editable = -1;
	conds.limit = 1;
	conds.offset = 0;
	conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
	conds.ordering = 0;
	conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
	conds.period_type = BP_BOOKMARK_DATE_ALL;

	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count,
		&properties, &conds, BP_BOOKMARK_O_TITLE, title, 0);

	free(ids);
	if (ret < 0)
		return -1;

	if (ids_count > 0) {
		BROWSER_LOGD("same title with same parent is already exist");
		return 0;
	}

	bp_bookmark_info_fmt info;
	memset(&info, 0x00, sizeof(bp_bookmark_info_fmt));
	info.type = 1;
	info.parent = parent_id;
	info.sequence = -1;
	info.access_count = -1;
	info.editable = 1;
	if (title != NULL && strlen(title) > 0)
		info.title = (char *)title;
	ret = bp_bookmark_adaptor_easy_create(&id, &info);
	if (ret == 0) {
		ret = bp_bookmark_adaptor_set_sequence(id, -1); // max sequence
		if (ret == 0) {
			*saved_folder_id = id;
			BROWSER_LOGD("bmsvc_add_bookmark is success(id:%d)", *saved_folder_id);
			bp_bookmark_adaptor_publish_notification();
			m_browser->notify_bookmark_added(title, id, parent_id);
			return 1;
		}
	}
	BROWSER_LOGD("bmsvc_add_bookmark is failed");
	return -1;
}

Eina_Bool bookmark::delete_by_id(int id)
{
	BROWSER_LOGD("id[%d]", id);
	if (bp_bookmark_adaptor_delete(id) < 0)
		return EINA_FALSE;
	else {
		bp_bookmark_adaptor_publish_notification();
		return EINA_TRUE;
	}
}

Eina_Bool bookmark::delete_by_id_notify(int id)
{
	BROWSER_LOGD("id[%d]", id);

	bookmark_item *item = new bookmark_item;
	RETV_MSG_IF(!item, EINA_FALSE, "item is NULL");
	get_item_by_id(id, item);
	int parent_id = item->get_parent_id();

	Eina_Bool result = delete_by_id(id);

	if (result)
		m_browser->notify_bookmark_removed(item->get_uri(), id, parent_id);

	if (item)
		delete item;
	return result;
}

Eina_Bool bookmark::delete_by_uri(const char *uri)
{
	BROWSER_SECURE_LOGD("uri[%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");
	bp_bookmark_property_cond_fmt properties;
	bp_bookmark_rows_cond_fmt conds; //conditions for querying
	memset(&properties, 0x00, sizeof(bp_bookmark_property_cond_fmt));
	memset(&conds, 0x00, sizeof(bp_bookmark_rows_cond_fmt));

	properties.parent = -1;
	properties.type = 0;
	properties.is_operator = -1;
	properties.is_editable = -1;
	conds.limit = 1;
	conds.offset = 0;
	conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
	conds.ordering = 0;
	conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
	conds.period_type = BP_BOOKMARK_DATE_ALL;

	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count,
		&properties, &conds, BP_BOOKMARK_O_URL, uri, 0);
	if (ret < 0)
		return EINA_FALSE;
	if (ids_count > 0)
		id = ids[0];
	free(ids);

	if (ids_count > 0) {
		if (delete_by_id_notify(id))
			return EINA_TRUE;
	}
	return EINA_FALSE;
}

int bookmark::update_bookmark(int id, const char *title, const char *uri,
					int parent_id, int order, Eina_Bool is_duplicate_check_needed, Eina_Bool is_URI_check_needed
					)
{
	BROWSER_SECURE_LOGD("title=[%s], uri=[%s], parent_id[%d], order[%d]", title, uri, parent_id, order);
	RETV_MSG_IF(!(title), -1, "title is NULL");

	Eina_Bool is_URI_exist = uri != NULL && strlen(uri) > 0;
	Eina_Bool is_title_exist = title != NULL && strlen(title) > 0;
	int ret = -1;
	if (is_duplicate_check_needed) {
		bp_bookmark_property_cond_fmt properties;
		bp_bookmark_rows_cond_fmt conds; //conditions for querying
		memset(&properties, 0x00, sizeof(bp_bookmark_property_cond_fmt));
		memset(&conds, 0x00, sizeof(bp_bookmark_rows_cond_fmt));
		int *ids = NULL;
		int ids_count = -1;
		if (is_URI_exist) {
			//This is a bookmark item so check for any such URL already exists
			if (is_URI_check_needed) {
				properties.parent = -1;
				properties.type = 0;
				properties.is_operator = -1;
				properties.is_editable = -1;
				conds.limit = 1;
				conds.offset = 0;
				conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
				conds.ordering = 0;
				conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
				conds.period_type = BP_BOOKMARK_DATE_ALL;
				ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count,
					&properties, &conds, BP_BOOKMARK_O_URL, uri, 0);
				if (ret < 0)
					return -1;
			}
		}
		else if (is_title_exist) {
			//This is a bookmark folder so check for any such folder with same title already exists
			properties.parent = parent_id;
			properties.type = 1;
			properties.is_operator = -1;
			properties.is_editable = -1;
			conds.limit = 1;
			conds.offset = 0;
			conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
			conds.ordering = 0;
			conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
			conds.period_type = BP_BOOKMARK_DATE_ALL;
			ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count,
					&properties, &conds, BP_BOOKMARK_O_TITLE, title, 0);
			if (ret < 0)
				return -1;
		}
		if (ids)
			free(ids);

		if (ids_count > 0) {
			BROWSER_LOGD("same bookmark already exist");
			return 0;
		}
	}
	bp_bookmark_info_fmt info;
	memset(&info, 0x00, sizeof(bp_bookmark_info_fmt));
	info.type = -1;
	info.parent = parent_id;
	info.sequence = order;
	info.editable = -1;
	if (is_URI_exist)
		info.url = (char *)uri;
	if (is_title_exist)
		info.title = (char *)title;

	ret = bp_bookmark_adaptor_easy_create(&id, &info);
	if (ret == 0) {
		BROWSER_LOGD("bp_bookmark_adaptor_easy_create is success");
		bp_bookmark_adaptor_publish_notification();
		return 1;
	}
	int errcode = bp_bookmark_adaptor_get_errorcode();
	BROWSER_LOGD("bp_bookmark_adaptor_easy_create is failed[%d]", errcode);
	return -1;
}

int bookmark::update_bookmark_notify(int id, const char *title, const char *uri,
					int parent_id, int order, Eina_Bool is_duplicate_check_needed, Eina_Bool is_URI_check_needed
					)
{
	BROWSER_LOGD("");
	int ret = update_bookmark(id, title, uri, parent_id, order, is_duplicate_check_needed, is_URI_check_needed);
	if (ret)
		m_browser->notify_bookmark_updated(uri, title,id, parent_id);
	return ret;
}

Eina_Bool bookmark::is_in_bookmark(const char *uri)
{
	BROWSER_SECURE_LOGD("uri[%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	int id = 0;
	Eina_Bool result = get_id(uri, &id);

	return result;
}

Eina_Bool bookmark::is_in_folder(int parent_folder, const char *title)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!title, EINA_FALSE, "title is NULL");

	bp_bookmark_property_cond_fmt properties;
	bp_bookmark_rows_cond_fmt conds; //conditions for querying
	memset(&properties, 0x00, sizeof(bp_bookmark_property_cond_fmt));
	memset(&conds, 0x00, sizeof(bp_bookmark_rows_cond_fmt));

	properties.parent = parent_folder;
	properties.type = 1;
	properties.is_operator = -1;
	properties.is_editable = -1;
	conds.limit = 1;
	conds.offset = 0;
	conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
	conds.ordering = 0;
	conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
	conds.period_type = BP_BOOKMARK_DATE_ALL;

	int *ids = NULL;
	int ids_count = -1;
	int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count,
		&properties, &conds, BP_BOOKMARK_O_TITLE, title, 0);

	if (ret < 0 || ids_count <= 0) {
		free(ids);
		return EINA_FALSE;
	}
	free(ids);

	if (ids_count > 0) {
		return EINA_TRUE;
	}
}

Eina_Bool bookmark::get_id(const char *uri, int *bookmark_id)
{
	//BROWSER_LOGD("");
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");
	RETV_MSG_IF(!(bookmark_id), EINA_FALSE, "bookmark_id is NULL");
	bp_bookmark_property_cond_fmt properties;
	bp_bookmark_rows_cond_fmt conds; //conditions for querying
	memset(&properties, 0x00, sizeof(bp_bookmark_property_cond_fmt));
	memset(&conds, 0x00, sizeof(bp_bookmark_rows_cond_fmt));

	properties.parent = -1;
	properties.type = 0;
	properties.is_operator = -1;
	properties.is_editable = -1;
	conds.limit = 1;
	conds.offset = 0;
	conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
	conds.ordering = 0;
	conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
	conds.period_type = BP_BOOKMARK_DATE_ALL;

	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count,
		&properties, &conds, BP_BOOKMARK_O_URL, uri, 0);
	if (ret < 0 || ids_count <= 0) {
		free(ids);
		return EINA_FALSE;
	}
	if (ids_count > 0)
		id = ids[0];
	free(ids);

	if (ids_count > 0) {
		*bookmark_id = id;
		return EINA_TRUE;
	}
}

Eina_Bool bookmark::get_folder_id(const char *title, int parent_id, int *folder_id)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!(title), EINA_FALSE, "title is NULL");
	RETV_MSG_IF(!(folder_id), EINA_FALSE, "folder_id is NULL");

	bp_bookmark_property_cond_fmt properties;
	bp_bookmark_rows_cond_fmt conds; //conditions for querying
	memset(&properties, 0x00, sizeof(bp_bookmark_property_cond_fmt));
	memset(&conds, 0x00, sizeof(bp_bookmark_rows_cond_fmt));

	properties.parent = parent_id;
	properties.type = 1;
	properties.is_operator = -1;
	properties.is_editable = -1;
	conds.limit = 1;
	conds.offset = 0;
	conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
	conds.ordering = 0;
	conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
	conds.period_type = BP_BOOKMARK_DATE_ALL;

	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count,
		&properties, &conds, BP_BOOKMARK_O_TITLE, title, 0);
	if (ret < 0)
		return EINA_FALSE;
	if (ids_count > 0)
		id = ids[0];
	free(ids);

	if (ids_count > 0) {
		*folder_id = id;
		return EINA_TRUE;
	}
	return EINA_FALSE;
}

Eina_Bool bookmark::get_item_by_id(int id, bookmark_item *item)
{
	BROWSER_LOGD("ID: %d", id);
	if (!item) {
		BROWSER_LOGE("item is NULL");
		return EINA_FALSE;
	}

	if (id == get_root_folder_id()) {
		(*item).set_title("Bookmarks");
		(*item).set_uri("");
		(*item).set_id(id);
		(*item).set_folder_flag(1);
		(*item).set_editable_flag(1);
		(*item).set_parent_id(-1);
		(*item).set_order(0);
		(*item).set_initial_order(0);
		return EINA_TRUE;
	}
	bp_bookmark_info_fmt info;
	if (bp_bookmark_adaptor_get_info(id, (BP_BOOKMARK_O_TYPE |
				BP_BOOKMARK_O_PARENT | BP_BOOKMARK_O_SEQUENCE |
				BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
				BP_BOOKMARK_O_TITLE), &info) == 0) {
		(*item).set_id(id);
		if (info.type > 0)
			(*item).set_folder_flag(EINA_TRUE);
		else
			(*item).set_folder_flag(EINA_FALSE);
		(*item).set_parent_id(info.parent);
		(*item).set_order(info.sequence);
		(*item).set_initial_order(info.sequence);
		if (info.editable > 0)
			(*item).set_editable_flag(EINA_TRUE);
		else
			(*item).set_editable_flag(EINA_FALSE);
		if (info.url != NULL && strlen(info.url) > 0)
			(*item).set_uri(info.url);
		if (info.title != NULL && strlen(info.title) > 0)
			(*item).set_title(info.title);
		bp_bookmark_adaptor_easy_free(&info);
		return EINA_TRUE;
	}
	BROWSER_LOGD("bp_bookmark_adaptor_get_easy_all is failed");
	return EINA_FALSE;
}

Eina_Bool bookmark::get_list_by_folder(const int folder_id, std::vector<bookmark_item *> &list)
{
	BROWSER_LOGD("folder ID: %d", folder_id);
	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_info_fmt info;
	if (bp_bookmark_adaptor_get_ids_p
		(&ids, &ids_count, -1, 0, folder_id, -1, -1, -1,
		BP_BOOKMARK_O_SEQUENCE, 1) < 0) {
		BROWSER_LOGD("bp_bookmark_adaptor_get_sequence_child_ids_p is failed");
		return EINA_FALSE;
	}

	if (ids_count <= 0) {
		BROWSER_LOGD("bookmark list is empty");
		if (ids)
			free(ids);
		return EINA_FALSE;
	}

	for(int i = 0; i < ids_count; i++) {
		if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
				BP_BOOKMARK_O_SEQUENCE |
				BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
				BP_BOOKMARK_O_TITLE | BP_BOOKMARK_O_IS_OPERATOR), &info) == 0) {
			bookmark_item *item = new bookmark_item;
			item->set_id(ids[i]);
			if (info.type > 0)
				item->set_folder_flag(EINA_TRUE);
			else
				item->set_folder_flag(EINA_FALSE);
			item->set_parent_id(folder_id);
			item->set_order(info.sequence);
			item->set_initial_order(info.sequence);
			if (info.editable > 0)
				item->set_editable_flag(EINA_TRUE);
			else
				item->set_editable_flag(EINA_FALSE);
			if (info.url != NULL && strlen(info.url) > 0)
				item->set_uri(info.url);
			if (info.title != NULL && strlen(info.title) > 0)
				item->set_title(info.title);
			list.push_back(item);
			bp_bookmark_adaptor_easy_free(&info);
		}
	}
	free(ids);
	return EINA_TRUE;
}

Eina_Bool bookmark::get_list_users_by_folder(const int folder_id, std::vector<bookmark_item *> &list)
{
	BROWSER_LOGD("folder ID: %d", folder_id);
	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_info_fmt info;
	if (bp_bookmark_adaptor_get_ids_p
		(&ids, &ids_count, -1, 0, folder_id, -1, 0, -1,
		BP_BOOKMARK_O_SEQUENCE, 1) < 0) {
		BROWSER_LOGD("bp_bookmark_adaptor_get_ids_p is failed");
		return EINA_FALSE;
	}

	if (ids_count <= 0) {
		BROWSER_LOGD("bookmark list is empty");
		if (ids)
			free(ids);
		return EINA_FALSE;
	}

	for(int i = 0; i < ids_count; i++) {
		if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
				BP_BOOKMARK_O_SEQUENCE |
				BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
				BP_BOOKMARK_O_TITLE |
				BP_BOOKMARK_O_IS_OPERATOR), &info) == 0) {
			bookmark_item *item = new bookmark_item;
			item->set_id(ids[i]);
			if (info.type > 0)
				item->set_folder_flag(EINA_TRUE);
			else
				item->set_folder_flag(EINA_FALSE);
			item->set_parent_id(folder_id);
			item->set_order(info.sequence);
			item->set_initial_order(info.sequence);
			if (info.editable > 0)
				item->set_editable_flag(EINA_TRUE);
			else
				item->set_editable_flag(EINA_FALSE);
			if (info.url != NULL && strlen(info.url) > 0)
				item->set_uri(info.url);
			if (info.title != NULL && strlen(info.title) > 0)
				item->set_title(info.title);
			if (info.is_operator > 0) {
				delete item;
			} else {
				list.push_back(item);
			}
			bp_bookmark_adaptor_easy_free(&info);
		}
	}
	free(ids);
	if (list.size() == 0)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

Eina_Bool bookmark::get_list_by_dividing(const int folder_id, std::vector<bookmark_item *> &list, int *list_item_count, int genlist_block_size)
{
	BROWSER_LOGD("folder ID: %d", folder_id);

	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_info_fmt info;
	if (bp_bookmark_adaptor_get_ids_p
		(&ids, &ids_count, genlist_block_size, *list_item_count, folder_id, -1, -1, -1,
		BP_BOOKMARK_O_SEQUENCE, 1) < 0) {
		BROWSER_LOGD("bp_bookmark_adaptor_get_ids_p is failed");
		return EINA_FALSE;
	}

	if (ids_count <= 0) {
		BROWSER_LOGD("bookmark list is empty");
		if (ids)
			free(ids);

		return EINA_FALSE;
	}

	for(int i = 0; i < ids_count; i++) {
		if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
				BP_BOOKMARK_O_SEQUENCE |
				BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
				BP_BOOKMARK_O_TITLE | BP_BOOKMARK_O_IS_OPERATOR), &info) == 0) {
			bookmark_item *item = new bookmark_item;
			item->set_id(ids[i]);
			if (info.type > 0)
				item->set_folder_flag(EINA_TRUE);
			else
				item->set_folder_flag(EINA_FALSE);
			item->set_parent_id(folder_id);
			item->set_order(info.sequence);
			item->set_initial_order(info.sequence);
			if (info.editable > 0)
				item->set_editable_flag(EINA_TRUE);
			else
				item->set_editable_flag(EINA_FALSE);
			if (info.url != NULL && strlen(info.url) > 0)
				item->set_uri(info.url);
			if (info.title != NULL && strlen(info.title) > 0)
				item->set_title(info.title);
			list.push_back(item);
			bp_bookmark_adaptor_easy_free(&info);
		}
	}
	free(ids);
	return EINA_TRUE;
}

Eina_Bool bookmark::get_list_users_by_dividing(const int folder_id, std::vector<bookmark_item *> &list, int *list_item_count, int genlist_block_size)
{
	BROWSER_LOGD("folder ID: %d", folder_id);
	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_info_fmt info;
	if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count,
			genlist_block_size, *list_item_count, folder_id, -1, 0, 1,
			BP_BOOKMARK_O_SEQUENCE, 1) < 0) {
		BROWSER_LOGD("bp_bookmark_adaptor_get_ids_p is failed");
		return EINA_FALSE;
	}

	if (ids_count <= 0) {
		BROWSER_LOGD("bookmark list is empty");
		if (ids)
			free(ids);
		return EINA_FALSE;
	}

	BROWSER_LOGD("list.size() before : %d", list.size());
	BROWSER_LOGD("ids_count: %d", ids_count);
	for(int i = 0; i < ids_count; i++) {
		BROWSER_LOGD("list.size(): %d", list.size());
		BROWSER_LOGD("index : %d", i);
		if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
				BP_BOOKMARK_O_SEQUENCE |
				BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
				BP_BOOKMARK_O_TITLE |
				BP_BOOKMARK_O_IS_OPERATOR), &info) == 0) {
			bookmark_item *item = new bookmark_item;
			item->set_id(ids[i]);
			if (info.type > 0)
				item->set_folder_flag(EINA_TRUE);
			else
				item->set_folder_flag(EINA_FALSE);
			item->set_parent_id(folder_id);
			item->set_order(info.sequence);
			item->set_initial_order(info.sequence);
			if (info.editable > 0)
				item->set_editable_flag(EINA_TRUE);
			else
				item->set_editable_flag(EINA_FALSE);
			if (info.url != NULL && strlen(info.url) > 0)
				item->set_uri(info.url);
			if (info.title != NULL && strlen(info.title) > 0)
				item->set_title(info.title);
			if (info.is_operator > 0) {
				BROWSER_LOGD("this is operator bookmark");
				delete item;
			} else {
				BROWSER_LOGD("item is pushed");
				list.push_back(item);
			}
			bp_bookmark_adaptor_easy_free(&info);
		}
	}
	BROWSER_LOGD("list.size() after: %d", list.size());
	free(ids);
	if (list.size() == 0)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

int bookmark::_get_folder_count(const int folder_id)
{
	BROWSER_LOGD("folder ID: %d", folder_id);

	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id,
		1, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0);
	free(ids);
	RETV_MSG_IF(ids_count < 0, 0, "bp_bookmark_adaptor_get_ids_p is failed");
	return ids_count;
}

Eina_Bool bookmark::get_list_operators(const int folder_id, std::vector<bookmark_item *> &list)
{
	BROWSER_LOGD("folder ID: %d", folder_id);
	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_info_fmt info;

	if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count,
			-1, 0, folder_id, -1, 1, -1, BP_BOOKMARK_O_SEQUENCE, 1) < 0) {
		BROWSER_LOGD("bp_bookmark_adaptor_get_ids_p is failed");
		return EINA_FALSE;
	}

	if (ids_count <= 0) {
		BROWSER_LOGD("bookmark list is empty");
		if (ids)
			free(ids);
		return EINA_FALSE;
	}

	for(int i = 0; i < ids_count; i++) {
		if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
				BP_BOOKMARK_O_SEQUENCE |
				BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
				BP_BOOKMARK_O_TITLE), &info) == 0) {
			bookmark_item *item = new bookmark_item;
			item->set_id(ids[i]);
			if (info.type > 0)
				item->set_folder_flag(EINA_TRUE);
			else
				item->set_folder_flag(EINA_FALSE);
			item->set_parent_id(folder_id);
			item->set_order(info.sequence);
			item->set_initial_order(info.sequence);
			if (info.editable > 0)
				item->set_editable_flag(EINA_TRUE);
			else
				item->set_editable_flag(EINA_FALSE);
			if (info.url != NULL && strlen(info.url) > 0)
				item->set_uri(info.url);
			if (info.title != NULL && strlen(info.title) > 0)
				item->set_title(info.title);
			list.push_back(item);
			bp_bookmark_adaptor_easy_free(&info);
		}
	}
	free(ids);
	if (list.size() == 0)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

Eina_Bool bookmark::get_list_by_keyword(const char *keyword, std::vector<bookmark_item *> &list)
{
	BROWSER_LOGD("");

	if (!keyword || (strlen(keyword) == 0)) {
		BROWSER_LOGD("keyword is NULL");
		return EINA_FALSE;
	}

	std::string buf_str(keyword);
	buf_str = "%" + buf_str + "%";

	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_info_fmt info;
	bp_bookmark_property_cond_fmt properties;
	bp_bookmark_rows_cond_fmt conds; //conditions for querying
	memset(&properties, 0x00, sizeof(bp_bookmark_property_cond_fmt));
	memset(&conds, 0x00, sizeof(bp_bookmark_rows_cond_fmt));

	properties.parent = -1;
	properties.type = 0;
	properties.is_operator = -1;
	properties.is_editable = -1;
	conds.limit = -1;
	conds.offset = 0;
	conds.order_offset = BP_BOOKMARK_O_TITLE;
	conds.ordering = -1;
	conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
	conds.period_type = BP_BOOKMARK_DATE_ALL;

	if (bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count,
		&properties, &conds,
		(BP_BOOKMARK_O_TITLE | BP_BOOKMARK_O_URL), buf_str.c_str(), 1) < 0) {
		BROWSER_LOGE("bp_bookmark_adaptor_get_cond_ids_p is failed");
		return EINA_FALSE;
	}

	if (ids_count <= 0) {
		BROWSER_LOGD("bookmark list is empty");
		if (ids)
			free(ids);
		return EINA_FALSE;
	}

	for(int i = 0; i < ids_count; i++) {
		if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
				BP_BOOKMARK_O_PARENT | BP_BOOKMARK_O_SEQUENCE |
				BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
				BP_BOOKMARK_O_TITLE), &info) == 0) {
			bookmark_item *item = new bookmark_item;
			item->set_id(ids[i]);
			if (info.type > 0)
				item->set_folder_flag(EINA_TRUE);
			else
				item->set_folder_flag(EINA_FALSE);
			item->set_parent_id(info.parent);
			item->set_order(info.sequence);
			item->set_initial_order(info.sequence);
			if (info.editable > 0)
				item->set_editable_flag(EINA_TRUE);
			else
				item->set_editable_flag(EINA_FALSE);
			if (info.url != NULL && strlen(info.url) > 0)
				item->set_uri(info.url);
			if (info.title != NULL && strlen(info.title) > 0)
				item->set_title(info.title);
			list.push_back(item);
			bp_bookmark_adaptor_easy_free(&info);
		}
	}
	free(ids);
	return EINA_TRUE;
}

void bookmark::destroy_list(std::vector<bookmark_item *> &list)
{
	BROWSER_LOGD("");

	for(unsigned int i = 0 ; i < list.size() ; i++) {
		if (list[i])
			delete list[i];
	}
	list.clear();
}

Eina_Bool bookmark::get_folder_depth_count(int *depth_count)
{
	BROWSER_LOGD("depth_count: %d", *depth_count);
	return _get_depth_count_recursive(get_root_folder_id(), 0, depth_count);
}

Eina_Bool bookmark::_get_depth_count_recursive(int folder_id, int cur_depth, int *depth_count)
{
	BROWSER_LOGD("current_depth: %d, depth_count:%d", cur_depth, *depth_count);
	std::vector<bookmark_item *> bookmark_list;
	Eina_Bool ret;

	ret = get_list_by_folder(folder_id, bookmark_list);
	if (ret == EINA_FALSE) {
		BROWSER_LOGE("get_list_by_folder is failed(folder id:%d)",folder_id);
		return EINA_FALSE;
	}

	for(unsigned int j = 0 ; j < bookmark_list.size() ; j++ ) {
		if (bookmark_list[j]->is_folder()) {
			/* Folder item is found. get sub list */
			//BROWSER_LOGD("Folder[%d] is %s(id: %d)\n", j, bookmark_list[j]->get_title(), bookmark_list[j]->get_id());
			if ((cur_depth+1) > *depth_count)
				*depth_count = cur_depth+1;

			//BROWSER_LOGD("full_depth_count %d)", depth_count);
			_get_depth_count_recursive(bookmark_list[j]->get_id(), cur_depth+1, depth_count);
		}
	}
	destroy_list(bookmark_list);
	return EINA_TRUE;
}

Eina_Bool bookmark::set_thumbnail(int id, Evas_Object *thumbnail)
{
	BROWSER_LOGD("id : %d", id);
	RETV_MSG_IF(!(thumbnail), EINA_FALSE, "thumbnail is NULL");

	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	platform_service ps;
	ps.evas_image_size_get(thumbnail, &w, &h, &stride);
	len = stride * h;
	BROWSER_LOGD("thumbnail w=[%d], h=[%d], stride=[%d], len=[%d]", w, h, stride, len);

	if (len == 0)
		return EINA_FALSE;

	void *thumbnail_data = evas_object_image_data_get(thumbnail, EINA_TRUE);
	int ret = bp_bookmark_adaptor_set_snapshot(id, w, h, (const unsigned char *)thumbnail_data, len);
	if (ret < 0) {
		BROWSER_LOGE("bp_bookmark_adaptor_set_thumbnail is failed");
		return EINA_FALSE;
	}
	return EINA_TRUE;
}

Evas_Object *bookmark::get_thumbnail(int id, Evas_Object *parent)
{
	BROWSER_LOGD("id : %d", id);
	RETV_MSG_IF(!parent, NULL, "parent is NULL");
	void *thumbnail_data = NULL;
	int w = 0;
	int h = 0;
	int len = 0;

	Evas_Object *icon = NULL;
	Evas *e = NULL;
	e = evas_object_evas_get(parent);
	RETV_MSG_IF(!e, NULL, "canvas is NULL");

	int ret = bp_bookmark_adaptor_get_snapshot(id, &w, &h, (unsigned char **)&thumbnail_data, &len);
	RETV_MSG_IF(ret < 0, NULL, "bp_bookmark_adaptor_set_thumbnail is failed");
	BROWSER_LOGE("len: %d, w:%d, h:%d", len, w, h);
	if (len > 0){
		/* gengrid event area has scaled with original image if it is evas image.
			therefore use elm_image*/
		icon = elm_image_add(parent);
		RETV_MSG_IF(!icon, NULL, "icon is NULL");
		Evas_Object *icon_object = elm_image_object_get(icon);
		evas_object_image_colorspace_set(icon_object, EVAS_COLORSPACE_ARGB8888);
		evas_object_image_size_set(icon_object, w, h);
		evas_object_image_fill_set(icon_object, 0, 0, w, h);
		evas_object_image_filled_set(icon_object, EINA_TRUE);
		evas_object_image_alpha_set(icon_object,EINA_TRUE);

		void *target_image_data = evas_object_image_data_get(icon_object, EINA_TRUE);
		memcpy(target_image_data, thumbnail_data, len);
		evas_object_image_data_set(icon_object, target_image_data);
	}

	return icon;
}

Eina_Bool bookmark::set_favicon(int id, Evas_Object *favicon)
{
	BROWSER_LOGD("id : %d", id);
	RETV_MSG_IF(!(favicon), EINA_FALSE, "favicon is NULL");

	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	platform_service ps;
	ps.evas_image_size_get(favicon, &w, &h, &stride);
	len = stride * h;
	BROWSER_LOGD("favicon w=[%d], h=[%d], stride=[%d], len=[%d]", w, h, stride, len);

	if (len == 0)
		return EINA_FALSE;

	void *favicon_data = evas_object_image_data_get(favicon, EINA_TRUE);
	int ret = bp_bookmark_adaptor_set_icon(id, w, h, (const unsigned char *)favicon_data, len);
	if (ret < 0) {
		BROWSER_LOGE("bp_bookmark_adaptor_set_favicon is failed");
		return EINA_FALSE;
	}
	return EINA_TRUE;
}

Evas_Object *bookmark::get_favicon(int id, Evas_Object *parent)
{
	BROWSER_LOGD("id : %d", id);
	RETV_MSG_IF(!(parent), EINA_FALSE, "parent is NULL");
	void *favicon_data = NULL;
	int w = 0;
	int h = 0;
	int len = 0;

	Evas_Object *icon = NULL;
	Evas *e = NULL;
	e = evas_object_evas_get(parent);
	if (!e) {
		BROWSER_LOGE("canvas is NULL");
		return NULL;
	}
	int ret = bp_bookmark_adaptor_get_icon(id, &w, &h, (unsigned char **)&favicon_data, &len);
	if (ret < 0) {
		BROWSER_LOGE("bp_bookmark_adaptor_set_favicon is failed");
		return NULL;
	}
	BROWSER_LOGE("len: %d, w:%d, h:%d", len, w, h);
	if (len > 0){
		icon = evas_object_image_filled_add(e);
		if (w == 0 || h == 0) {
			// Android bookmark.
			evas_object_image_memfile_set(icon, favicon_data, len, NULL, NULL);
		} else {
			// Tizen bookmark.
			evas_object_image_colorspace_set(icon, EVAS_COLORSPACE_ARGB8888);
			evas_object_image_size_set(icon, w, h);
			evas_object_image_fill_set(icon, 0, 0, w, h);
			evas_object_image_filled_set(icon, EINA_TRUE);
			evas_object_image_alpha_set(icon,EINA_TRUE);

			void *target_image_data = evas_object_image_data_get(icon, EINA_TRUE);
			memcpy(target_image_data, favicon_data, len);
			evas_object_image_data_set(icon, target_image_data);
		}
	}
	return icon;
}

Eina_Bool bookmark::set_webicon(int id, Evas_Object *webicon)
{
	BROWSER_LOGD("id : %d", id);
	RETV_MSG_IF(!webicon, EINA_FALSE, "webicon is NULL");

	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	platform_service ps;
	ps.evas_image_size_get(webicon, &w, &h, &stride);
	len = stride * h;
	BROWSER_LOGD("webicon w=[%d], h=[%d], stride=[%d], len=[%d]", w, h, stride, len);

	if (len == 0)
		return EINA_FALSE;

	void *webicon_data = evas_object_image_data_get(webicon, EINA_TRUE);
	int ret = bp_bookmark_adaptor_set_webicon(id, w, h, (const unsigned char *)webicon_data, len);
	if (ret != 0)
		BROWSER_LOGE("set webicon is failed");
	return EINA_FALSE;
}

Evas_Object *bookmark::get_webicon(int id, Evas_Object *parent)
{
	BROWSER_LOGD("id : %d", id);
	RETV_MSG_IF(!(parent), EINA_FALSE, "parent is NULL");
	void *webicon_data = NULL;
	int w = 0;
	int h = 0;
	int len = 0;

	Evas_Object *icon = NULL;
	Evas *e = NULL;
	e = evas_object_evas_get(parent);
	RETV_MSG_IF(!e, NULL, "canvas is NULL");

	int ret = bp_bookmark_adaptor_get_webicon(id, &w, &h, (unsigned char **)&webicon_data, &len);
	if (ret != 0) {
		BROWSER_LOGE("bp_bookmark_adaptor_set_webicon is failed");
		return NULL;
	}

	BROWSER_LOGE("len: %d, w:%d, h:%d", len, w, h);
	if (len > 0){
		icon = evas_object_image_filled_add(e);

		evas_object_image_colorspace_set(icon, EVAS_COLORSPACE_ARGB8888);
		evas_object_image_size_set(icon, w, h);
		evas_object_image_fill_set(icon, 0, 0, w, h);
		evas_object_image_filled_set(icon, EINA_TRUE);
		evas_object_image_alpha_set(icon,EINA_TRUE);

		void *target_image_data = evas_object_image_data_get(icon, EINA_TRUE);
		memcpy(target_image_data, webicon_data, len);
		evas_object_image_data_set(icon, target_image_data);
	}

	return icon;
}

Eina_Bool bookmark::set_access_count(int id, int count)
{
	BROWSER_LOGD("");
	if(!bp_bookmark_adaptor_set_access_count(id, count))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool bookmark::get_access_count(int id, int *count)
{
	BROWSER_LOGD("");
	if(!bp_bookmark_adaptor_get_access_count(id, count))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool bookmark::increase_access_count(int id)
{
	BROWSER_LOGD("");
	int count;
	if(get_access_count(id, &count) == EINA_FALSE) {
		BROWSER_LOGD("get_access_count is failed");
		return EINA_FALSE;
	}

	if(set_access_count(id, count + 1) == EINA_FALSE) {
		BROWSER_LOGD("set_access_count is failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool bookmark::set_last_sequence(int id)
{
	BROWSER_LOGD("");

	if(bp_bookmark_adaptor_set_sequence(id, -1) == -1)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

const char* bookmark::get_path_info(void)
{
	BROWSER_LOGD("%s", m_path_string.c_str());
	return m_path_string.c_str();
}

bookmark::folder_info *bookmark::get_path_by_index(unsigned int index)
{
	BROWSER_LOGD("%s", m_path_history[index]->folder_name);

	return m_path_history[index];
}

int bookmark::get_path_size(void)
{
	BROWSER_LOGD("%d", m_path_history.size());

	return m_path_history.size();
}

void bookmark::push_back_path(folder_info *item)
{
	BROWSER_LOGD("path size before push: %d", m_path_history.size());
	m_path_history.push_back(item);
}

void bookmark::pop_back_path(void)
{
	BROWSER_LOGD("path size before pop: %d", m_path_history.size());
	m_path_history.pop_back();
}

void bookmark::clear_path_history(void)
{
	BROWSER_LOGD("");

	free_path_history();
	m_path_history.clear();
}

void bookmark::free_path_history(void)
{
	BROWSER_LOGD("");
	for(unsigned int i = 0 ; i < m_path_history.size() ; i++) {
		if (m_path_history[i]) {
			if (m_path_history[i]->folder_name)
				free(m_path_history[i]->folder_name);

			free(m_path_history[i]);
		}
	}
}

void bookmark::change_path_lang(void)
{
	BROWSER_LOGD("");
	m_path_string.clear();

	char *old_str = m_path_history[0]->folder_name;
	m_path_history[0]->folder_name = strdup(BR_STRING_BOOKMARKS);
	free(old_str);

	for(int i = 0 ; i < get_path_size(); i++) {
		if (get_path_by_index(i)) {
			if (m_path_string.empty()) {
				m_path_string = m_path_history[0]->folder_name;
			} else {
				m_path_string += "/";
				m_path_string += get_path_by_index(i)->folder_name;
			}
		}
	}
	BROWSER_LOGD("str: %s", m_path_string.c_str());
}

void bookmark::path_into_sub_folder(int folder_id, const char *folder_name)
{
	BROWSER_LOGD("");

	folder_info *item = (folder_info *)malloc(sizeof(folder_info));
	if (!item)
		return;
	memset(item, 0x00, sizeof(folder_info));

	item->folder_id = folder_id;
	if (folder_id == get_root_folder_id()) {
		item->folder_name = strdup(BR_STRING_BOOKMARKS);
	} else
		item->folder_name = strdup(folder_name);
	push_back_path(item);
	m_path_string.clear();
	for(int i = 0 ; i < get_path_size() ; i++) {
		if (get_path_by_index(i)) {
			if (m_path_string.empty()) {
				m_path_string = get_path_by_index(i)->folder_name;
			} else {
				m_path_string += "/";
				m_path_string += get_path_by_index(i)->folder_name;
			}
		}
	}
	char *path_string = (elm_entry_utf8_to_markup(m_path_string.c_str()));
	if(!path_string)
		return;
	m_path_string = path_string;
	free(path_string);
	BROWSER_LOGD("m_path_string: %s", m_path_string.c_str());

	m_curr_folder = folder_id;
}

Eina_Bool bookmark::path_to_upper_folder(int *curr_folder)
{
	BROWSER_LOGD("");

	int current_depth = get_path_size();
	BROWSER_LOGD("current_depth: %d", current_depth);

	if (current_depth <= 0) {
		BROWSER_LOGE("[ERROR] top folder is not valid");
		return EINA_TRUE;
	}

	int curr_depth = current_depth - 1;
	if (curr_depth > 0) {
		*curr_folder = get_path_by_index(curr_depth - 1)->folder_id;
		if (get_path_by_index(curr_depth) && get_path_by_index(curr_depth)->folder_name) {
			free(get_path_by_index(curr_depth)->folder_name);
			free(get_path_by_index(curr_depth));
		}
		pop_back_path();
	} else {
		/* Current folder is root folder */
		if (*curr_folder != get_root_folder_id()) {
			BROWSER_LOGE("[ERROR] top folder is not root folder");
			return EINA_TRUE;
		}
		if (get_path_by_index(curr_depth) && get_path_by_index(curr_depth)->folder_name) {
			free(get_path_by_index(curr_depth)->folder_name);
			free(get_path_by_index(curr_depth));
		}
		pop_back_path();
		m_path_string.clear();
		return EINA_FALSE;
	}

	m_path_string.clear();
	for(int i = 0 ; i < get_path_size() ; i++) {
		if (get_path_by_index(i)) {
			if (m_path_string.empty()) {
				m_path_string = get_path_by_index(i)->folder_name;
			} else {
				m_path_string += "/";
				m_path_string += get_path_by_index(i)->folder_name;
			}
		}
	}
	BROWSER_LOGD("m_path_string: %s", m_path_string.c_str());

	m_curr_folder = *curr_folder;
	return EINA_TRUE;
}

