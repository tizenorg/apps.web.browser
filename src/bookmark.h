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

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <Evas.h>
#include <vector>
#include <string>
#include "browser-object.h"

class bookmark_item;
class bookmark : public browser_object {
public:
	typedef struct _folder_info {
		int folder_id;
		char *folder_name;
	} folder_info;

	bookmark(void);
	~bookmark(void);

	int get_root_folder_id(void);
	int save_bookmark(const char *title, const char *uri, int *saved_bookmark_id, int parent_id);
	int save_folder(const char *title, int *saved_bookmark_id, int parent_id);
	Eina_Bool delete_by_id(int id);
	Eina_Bool delete_by_id_notify(int id);
	Eina_Bool delete_by_uri(const char *uri);
	int update_bookmark(int id, const char *title, const char *uri,
					int parent_id, int order, Eina_Bool is_duplicate_check_needed = EINA_FALSE, Eina_Bool is_URI_check_needed = EINA_FALSE
					);
	int update_bookmark_notify(int id, const char *title, const char *uri,
					int parent_id, int order, Eina_Bool is_duplicate_check_needed = EINA_FALSE, Eina_Bool is_URI_check_needed = EINA_FALSE
					);
	Eina_Bool delete_all(void);
	Eina_Bool get_item_by_id(int id, bookmark_item *item);
	Eina_Bool get_list_by_folder(const int folder_id, std::vector<bookmark_item *> &list);
	Eina_Bool get_list_users_by_folder(const int folder_id, std::vector<bookmark_item *> &list);
	Eina_Bool get_list_by_dividing(const int folder_id, std::vector<bookmark_item *> &list, int *last_item, int genlist_block_size);
	Eina_Bool get_list_users_by_dividing(const int folder_id, std::vector<bookmark_item *> &list, int *last_item, int genlist_block_size);
	int _get_folder_count(const int folder_id);
	Eina_Bool get_list_operators(const int folder_id, std::vector<bookmark_item *> &list);
	Eina_Bool get_list_by_keyword(const char *keyword, std::vector<bookmark_item *> &list);
	void destroy_list(std::vector<bookmark_item *> &list);
	int get_count(void);
	Eina_Bool get_id(const char *uri, int *bookmark_id);
	Eina_Bool get_folder_id(const char *title, int parent_id, int *folder_id);
	Eina_Bool is_in_bookmark(const char *uri);
	Eina_Bool is_in_folder(int parent_folder, const char *title);
	Eina_Bool get_folder_depth_count(int *depth_count);
	Eina_Bool set_thumbnail(int id, Evas_Object *thumbnail);
	Evas_Object *get_thumbnail(int id, Evas_Object *parent);
	Eina_Bool set_favicon(int id, Evas_Object *favicon);
	Evas_Object *get_favicon(int id, Evas_Object *parent);
	Eina_Bool set_webicon(int id, Evas_Object *webicon);
	Evas_Object *get_webicon(int id, Evas_Object *parent);
	Eina_Bool set_access_count(int id, int count);
	Eina_Bool get_access_count(int id, int *count);
	Eina_Bool increase_access_count(int id);
	Eina_Bool set_last_sequence(int id);
	const char* get_path_info(void);
	folder_info *get_path_by_index(unsigned int index);
	int get_path_size(void);
	void push_back_path(folder_info *item);
	void pop_back_path(void);
	void clear_path_history(void);
	void free_path_history(void);
	void change_path_lang(void);
	void path_into_sub_folder(int folder_id, const char *folder_name);
	Eina_Bool path_to_upper_folder(int *curr_folder);

	Eina_Bool get_memory_full(void) { return m_memory_full; }
	int get_current_folder_id(void) { return m_curr_folder; }

private:
	Eina_Bool _get_depth_count_recursive(int folder_id, int cur_depth, int *depth_count);

	std::vector<bookmark_item *> m_bookmark_list;
	std::vector<folder_info *> m_path_history;
	std::string m_path_string;
	Eina_Bool m_memory_full;
	Eina_Bool m_bookmark_adaptor_initialize;
	int m_curr_folder;

};

#endif /* BOOKMARK_H */

