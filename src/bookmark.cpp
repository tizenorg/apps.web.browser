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

extern "C" {
#include "db-util.h"
}

#include "browser-object.h"

#include "bookmark.h"
#include "bookmark-item.h"
#include "browser-dlog.h"
#include "platform-service.h"
#if defined(BROWSER_BOOKMARK_SYNC)
#include "bookmark-adaptor.h"
#else
#include "bookmark-service.h"
#endif

bookmark::bookmark(void)
{
	BROWSER_LOGD("");
}

bookmark::~bookmark(void)
{
	BROWSER_LOGD("");
}

int bookmark::get_root_folder_id(void)
{
#if defined(BROWSER_BOOKMARK_SYNC)
	return bp_bookmark_adaptor_get_root_id();
#else
	return internet_bookmark_get_root_id();
#endif
}

int bookmark::save_bookmark(const char *title, const char *uri,
					int *saved_bookmark_id,
					int parent_id
#if defined(BROWSER_TAG)
					, const char *tag1,
					const char *tag2,
					const char *tag3,
					const char *tag4
#endif
					)
{
	BROWSER_LOGD("title=[%s], uri=[%s]", title, uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, -1);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, -1);

#if defined(BROWSER_BOOKMARK_SYNC)
	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_adaptor_get_duplicated_url_ids_p(&ids, &ids_count, 1,
		-1, -1, 0, 0, uri, 0);
	if (ids_count > 0)
		id = ids[0];
	free(ids);

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
	if (uri != NULL && strlen(uri) > 0)
		info.url = strdup(uri);
	if (title != NULL && strlen(title) > 0)
		info.title = strdup(title);

	int ret = bp_bookmark_adaptor_easy_create(&id, &info);
	if (ret == 0) {
		ret = bp_bookmark_adaptor_set_sequence(id, -1); // max sequence
		bp_bookmark_adaptor_easy_free(&info);
		if (ret == 0) {
			*saved_bookmark_id = id;
			BROWSER_LOGD("bp_bookmark_adaptor_easy_create is success(id:%d)", *saved_bookmark_id);
			bp_bookmark_adaptor_publish_notification();
			return 1;
		}
	}
	int errcode = bp_bookmark_adaptor_get_errorcode();
	bp_bookmark_adaptor_easy_free(&info);
	BROWSER_LOGD("bp_bookmark_adaptor_easy_create is failed[%d]", errcode);
	return -1;
#else
#if defined(BROWSER_TAG)
	int ret = bmsvc_add_bookmark(title, uri, parent_id,
				tag1, tag2, tag3, tag4, saved_bookmark_id);
#else
	int ret = bmsvc_add_bookmark(title, uri, parent_id,
				NULL, NULL, NULL, NULL, saved_bookmark_id);
#endif
	if (ret == BMSVC_ERROR_ITEM_ALREADY_EXIST) {
		BROWSER_LOGD("same URI is already exist");
		return 0;
	}
	if (ret != BMSVC_ERROR_NONE && ret != BMSVC_ERROR_ITEM_ALREADY_EXIST) {
		BROWSER_LOGD("bmsvc_add_bookmark is failed");
		return -1;
	}

	BROWSER_LOGD("bmsvc_add_bookmark is success(id:%d)", *saved_bookmark_id);
	return 1;
#endif
}

int bookmark::save_folder(const char *title, int *saved_folder_id, int parent_id)
{
	BROWSER_LOGD("title=[%s], parent_id[%d]", title, parent_id);
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, -1);

#if defined(BROWSER_BOOKMARK_SYNC)
	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_adaptor_get_duplicated_title_ids_p(&ids, &ids_count, 1,
		-1, parent_id, 1, 0, title, 0);
	if (ids_count > 0)
		id = ids[0];
	free(ids);

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
		info.title = strdup(title);
	int ret = bp_bookmark_adaptor_easy_create(&id, &info);
	if (ret == 0) {
		ret = bp_bookmark_adaptor_set_sequence(id, -1); // max sequence
		bp_bookmark_adaptor_easy_free(&info);
		if (ret == 0) {
			*saved_folder_id = id;
			BROWSER_LOGD("bmsvc_add_bookmark is success(id:%d)", *saved_folder_id);
			bp_bookmark_adaptor_publish_notification();
			return 1;
		}
	}
	bp_bookmark_adaptor_easy_free(&info);
	BROWSER_LOGD("bmsvc_add_bookmark is failed");
	return -1;
#else
	int ret = bmsvc_add_folder(title, parent_id, 1, saved_folder_id);
	if (ret == BMSVC_ERROR_ITEM_ALREADY_EXIST) {
		BROWSER_LOGD("same URI is already exist");
		return 0;
	}
	if (ret != BMSVC_ERROR_NONE && ret != BMSVC_ERROR_ITEM_ALREADY_EXIST) {
		BROWSER_LOGD("bmsvc_add_bookmark is failed");
		return -1;
	}

	BROWSER_LOGD("bmsvc_add_folder is success(id:%d)", *saved_folder_id);
	return 1;
#endif
}

Eina_Bool bookmark::delete_by_id(int id)
{
	BROWSER_LOGD("id[%d]", id);
#if defined(BROWSER_BOOKMARK_SYNC)
	if (bp_bookmark_adaptor_delete(id) < 0)
		return EINA_FALSE;
	else {
		bp_bookmark_adaptor_publish_notification();
		return EINA_TRUE;
	}
#else
	if (bmsvc_delete_bookmark(id, true) == BMSVC_ERROR_NONE)
		return EINA_TRUE;
	else
		return EINA_FALSE;
#endif
}

Eina_Bool bookmark::delete_by_uri(const char *uri)
{
	BROWSER_LOGD("uri[%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);
#if defined(BROWSER_BOOKMARK_SYNC)
	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_adaptor_get_duplicated_url_ids_p(&ids, &ids_count, 1,
		-1, -1, 0, 0, uri, 0);
	if (ids_count > 0)
		id = ids[0];
	free(ids);

	if (ids_count > 0) {
		if (delete_by_id(id))
			return EINA_TRUE;
	}
	return EINA_FALSE;
#else
	int id = 0;
	Eina_Bool ret = get_id(uri, &id);
	if (ret) {
		if (delete_by_id(id))
			return EINA_TRUE;
	}
	return EINA_FALSE;
#endif
}

int bookmark::update_bookmark(int id, const char *title, const char *uri,
					int parent_id, int order
#if defined(BROWSER_TAG)
					, const char *tag1,
					const char *tag2,
					const char *tag3,
					const char *tag4
#endif
					)
{
	BROWSER_LOGD("title=[%s], uri=[%s], parent_id[%d], order[%d]", title, uri, parent_id, order);
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, -1);
#if defined(BROWSER_BOOKMARK_SYNC)
	bp_bookmark_info_fmt info;
	memset(&info, 0x00, sizeof(bp_bookmark_info_fmt));
	info.type = -1;
	info.parent = parent_id;
	info.sequence = order;
	info.editable = -1;
	if (uri != NULL && strlen(uri) > 0)
		info.url = strdup(uri);
	if (title != NULL && strlen(title) > 0)
		info.title = strdup(title);

	int ret = bp_bookmark_adaptor_easy_create(&id, &info);
	if (ret == 0) {
		bp_bookmark_adaptor_easy_free(&info);
		BROWSER_LOGD("bp_bookmark_adaptor_easy_create is success");
		bp_bookmark_adaptor_publish_notification();
		return 1;
	}
	int errcode = bp_bookmark_adaptor_get_errorcode();
	bp_bookmark_adaptor_easy_free(&info);
	BROWSER_LOGD("bp_bookmark_adaptor_easy_create is failed[%d]", errcode);
	return -1;
#else
#if defined(BROWSER_TAG)
	int ret = bmsvc_update_bookmark(id, title, uri, parent_id, order,
				tag1, tag2, tag3, tag4, EINA_TRUE);
#else
	int ret = bmsvc_update_bookmark(id, title, uri, parent_id, order,
				NULL, NULL, NULL, NULL, EINA_TRUE);
#endif

	if (ret != BMSVC_ERROR_NONE ) {
		BROWSER_LOGD("bmsvc_add_bookmark is failed");
		return -1;
	}

	BROWSER_LOGD("bmsvc_update_bookmark is success)");
	return 1;
#endif
}

Eina_Bool bookmark::is_in_bookmark(const char *uri)
{
	BROWSER_LOGD("uri[%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	int id = 0;
	return get_id(uri, &id);
}

Eina_Bool bookmark::get_id(const char *uri, int *bookmark_id)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(bookmark_id, EINA_FALSE);
#if defined(BROWSER_BOOKMARK_SYNC)
	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_adaptor_get_duplicated_url_ids_p(&ids, &ids_count, 1,
		-1, -1, 0, 0, uri, 0);
	if (ids_count > 0)
		id = ids[0];
	free(ids);

	if (ids_count > 0) {
		*bookmark_id = id;
		return EINA_TRUE;
	}
	return EINA_FALSE;
#else
	int ret = bmsvc_get_bookmark_id(uri, bookmark_id);
	if (ret == BMSVC_ERROR_NONE)
		return EINA_TRUE;
	else
		return EINA_FALSE;
#endif
}

Eina_Bool bookmark::get_folder_id(const char *title, int parent_id, int *folder_id)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(folder_id, EINA_FALSE);

#if defined(BROWSER_BOOKMARK_SYNC)
	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	bp_bookmark_adaptor_get_duplicated_title_ids_p(&ids, &ids_count, 1,
		-1, parent_id, 1, 0, title, 0);
	if (ids_count > 0)
		id = ids[0];
	free(ids);

	if (ids_count > 0) {
		*folder_id = id;
		return EINA_TRUE;
	}
	return EINA_FALSE;
#else
	int ret = bmsvc_get_folder_id(title, parent_id, folder_id);
	if (ret == BMSVC_ERROR_NONE)
		return EINA_TRUE;
	else
		return EINA_FALSE;
#endif
}

Eina_Bool bookmark::get_item_by_id(int id, bookmark_item *item)
{
	BROWSER_LOGD("ID: %d", id);
	if (!item) {
		BROWSER_LOGE("item is NULL");
		return EINA_FALSE;
	}

#if defined(BROWSER_BOOKMARK_SYNC)
	if (id == get_root_folder_id()) {
		(*item).set_title("Bookmarks");
		(*item).set_uri("");
		(*item).set_id(id);
		(*item).set_folder_flag(1);
		(*item).set_editable_flag(1);
		(*item).set_parent_id(-1);
		(*item).set_order(0);
		return EINA_TRUE;
	}
	bp_bookmark_info_fmt info;
	if (bp_bookmark_adaptor_get_easy_all(id, &info) == 0) {
		(*item).set_title(info.title);
		(*item).set_uri(info.url);
		(*item).set_id(id);
		(*item).set_folder_flag(info.type);
		(*item).set_editable_flag(info.editable);
		(*item).set_parent_id(info.parent);
		(*item).set_order(info.sequence);
		bp_bookmark_adaptor_easy_free(&info);
		return EINA_TRUE;
	}
	bp_bookmark_adaptor_easy_free(&info);
	BROWSER_LOGD("bp_bookmark_adaptor_get_easy_all is failed");
	return EINA_FALSE;
#else
	bookmark_entry *entry = bmsvc_get_bookmark_by_id(id);

	if (entry == NULL)
		return EINA_FALSE;

	if (_convert_bookmark_entry(*item, (void *)entry)) {
		bmsvc_free_bookmark_entry(entry);
		return EINA_TRUE;
	}

	bmsvc_free_bookmark_entry(entry);
	return EINA_FALSE;
#endif
}

Eina_Bool bookmark::get_list_by_folder(const int folder_id, std::vector<bookmark_item *> &list)
{
	BROWSER_LOGD("folder ID: %d", folder_id);
#if defined(BROWSER_BOOKMARK_SYNC)
	bookmark_list *bookmarks = bp_bookmark_adaptor_list(folder_id, 2);
	if (bookmarks == NULL) {
		BROWSER_LOGD("bp_bookmark_adaptor_list is failed");
		return EINA_FALSE;
	}

	BROWSER_LOGD("count: %d", bookmarks->count);
	for (int i = 0 ; i < bookmarks->count ; i++) {
		BROWSER_LOGD("Bookmark[%d] is %s[ID: %d]", i, (*((bookmarks->item)+i)).title, ((bookmarks->item)+i)->id);
		bookmark_item *item = new bookmark_item;
		_convert_bookmark_entry(*item, (void *)((bookmarks->item)+i));
		list.push_back(item);
	}

	bp_bookmark_adaptor_free(bookmarks);
	return EINA_TRUE;
#else
	bookmark_list *bookmarks = internet_bookmark_list(folder_id, 2);
	if (bookmarks == NULL) {
		BROWSER_LOGD("internet_bookmark_list is failed");
		return EINA_FALSE;
	}

	BROWSER_LOGD("count: %d", bookmarks->count);
	for (int i = 0 ; i < bookmarks->count ; i++) {
		BROWSER_LOGD("Bookmark[%d] is %s\n", i, (*((bookmarks->item)+i)).title);
		bookmark_item *item = new bookmark_item;
		_convert_bookmark_entry(*item, (void *)((bookmarks->item)+i));
		list.push_back(item);
	}

	internet_bookmark_free(bookmarks);
	return EINA_TRUE;
#endif
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


Eina_Bool bookmark::_convert_bookmark_entry(bookmark_item &dest, void *src)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
	bookmark_entry *entry = (bookmark_entry *)src;

	dest.set_title(entry->title);
	dest.set_uri(entry->address);
	dest.set_id(entry->id);
	dest.set_folder_flag(entry->is_folder);
	dest.set_editable_flag(entry->editable);
	dest.set_parent_id(entry->folder_id);
	dest.set_order(entry->orderIndex);
#if defined(BROWSER_TAG)
	dest.set_tag1(entry->tag1);
	dest.set_tag2(entry->tag2);
	dest.set_tag3(entry->tag3);
	dest.set_tag4(entry->tag4);
#endif
	return EINA_TRUE;
}

Eina_Bool bookmark::set_thumbnail(int id, Evas_Object *thumbnail)
{
	BROWSER_LOGD("id : %d", id);
	EINA_SAFETY_ON_NULL_RETURN_VAL(thumbnail, EINA_FALSE);

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
#if defined(BROWSER_BOOKMARK_SYNC)
#if 0
	int ret = bp_bookmark_adaptor_set_thumbnail(id,(const unsigned char *)thumbnail_data, len);
	if (ret == 0) {
		if (bp_bookmark_adaptor_set_thumbnail_width(id, w) == 0) {
			if (bp_bookmark_adaptor_set_thumbnail_height(id, h) == 0) {
				BROWSER_LOGD("thumbnail is successfully saved.");
				bp_bookmark_adaptor_publish_notification();
				return EINA_TRUE;
			} else
				BROWSER_LOGE("bp_bookmark_adaptor_set_thumbnail_height is failed");
		} else
			BROWSER_LOGE("bp_bookmark_adaptor_set_thumbnail_width is failed");
	} else
		BROWSER_LOGE("bp_bookmark_adaptor_set_thumbnail is failed");
#endif
	BROWSER_LOGE("set thumbnail is failed");
	return EINA_FALSE;
#else
	if (bmsvc_set_thumbnail(id, thumbnail_data, w, h, len) != BMSVC_ERROR_NONE)
		return EINA_FALSE;

	return EINA_TRUE;
#endif
}

Evas_Object *bookmark::get_thumbnail(int id, Evas_Object *parent)
{
	BROWSER_LOGD("id : %d", id);
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, EINA_FALSE);
	void *thumbnail_data = NULL;
	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	Evas_Object *icon = NULL;
	Evas *e = NULL;
	e = evas_object_evas_get(parent);
	if (!e) {
		BROWSER_LOGE("canvas is NULL");
		return NULL;
	}
#if defined(BROWSER_BOOKMARK_SYNC)
	int ret = bp_bookmark_adaptor_get_thumbnail(id, (unsigned char **)&thumbnail_data, &len);
	if (ret == 0) {
		if (bp_bookmark_adaptor_get_thumbnail_width(id, &w) == 0) {
			if (bp_bookmark_adaptor_get_thumbnail_height(id, &h) == 0) {
				BROWSER_LOGD("thumbnail is successfully loaded.");
			} else {
				BROWSER_LOGE("bp_bookmark_adaptor_set_thumbnail_height is failed");
				return NULL;
			}
		} else {
			BROWSER_LOGE("bp_bookmark_adaptor_set_thumbnail_width is failed");
			return NULL;
		}
	} else {
		BROWSER_LOGE("bp_bookmark_adaptor_set_thumbnail is failed");
		return NULL;
	}
#else
	if (bmsvc_get_thumbnail(id, &thumbnail_data, &w, &h, &len) != BMSVC_ERROR_NONE) {
		BROWSER_LOGE("bmsvc_get_thumbnail failed");
		return NULL;
	}
#endif
	BROWSER_LOGE("len: %d, w:%d, h:%d", len, w, h);
	if (len > 0){
		icon = evas_object_image_filled_add(e);
		evas_object_image_colorspace_set(icon, EVAS_COLORSPACE_ARGB8888);
		evas_object_image_size_set(icon, w, h);
		evas_object_image_fill_set(icon, 0, 0, w, h);
		evas_object_image_filled_set(icon, EINA_TRUE);
		evas_object_image_alpha_set(icon,EINA_TRUE);
		evas_object_image_data_set(icon, thumbnail_data);
	}

	return icon;
}

#if defined(BROWSER_TAG)
Eina_Bool bookmark::get_tag_list(std::vector<char *> &list)
{
	BROWSER_LOGD("");

	tag_list *tags = bmsvc_get_tag_list();

	if (tags == NULL)
		return EINA_FALSE;

	for(int i = 0; i < tags->count; i++) {
		BROWSER_LOGD("key[%d] is %s\n", i, *((tags->tag)+i));
		list.push_back(strdup(*((tags->tag)+i)));
	}

	bmsvc_free_tag_list(tags);

	return EINA_TRUE;
}

Eina_Bool bookmark::get_tag_count(int *count)
{
	return EINA_TRUE;
}

Eina_Bool  bookmark::get_max_tag_count_for_a_bookmark_item(int *count)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(count, EINA_FALSE);
	bmsvc_get_max_tag_count_for_a_bookmark_entry(count);
	return EINA_TRUE;
}

Eina_Bool bookmark::get_list_by_tag(const char *tag, std::vector<bookmark_item *> *list)
{
	BROWSER_LOGD("");

	bookmark_list *bookmarks = bmsvc_get_bookmark_list_by_tag(tag);
	EINA_SAFETY_ON_NULL_RETURN_VAL(bookmarks, EINA_FALSE);

	for (int i = 0 ; i < bookmarks->count ; i++) {
		BROWSER_LOGD("Bookmark[%d] is %s\n", i, (*((bookmarks->item)+i)).title);
		bookmark_item *item = new bookmark_item;
		_convert_bookmark_entry(*item, (void *)((bookmarks->item)+i));
		list->push_back(item);
	}

	internet_bookmark_free(bookmarks);
	return EINA_TRUE;
}

Eina_Bool bookmark::get_count_by_tag(const char *tag, int *count)
{
	return EINA_TRUE;
}

void bookmark::destroy_tag_list(std::vector<char *> &list)
{
	BROWSER_LOGD("");

	for(unsigned int i = 0 ; i < list.size() ; i++) {
		if (list[i])
			free(list[i]);
	}
	list.clear();
}
#endif
