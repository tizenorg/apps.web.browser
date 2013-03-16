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

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <Evas.h>
#include <vector>
#include "browser-object.h"

#define bookmark_max_count	1000

class bookmark_item;
class bookmark : public browser_object {
public:
	bookmark(void);
	~bookmark(void);

	int save_bookmark(const char *title, const char *uri, int *saved_bookmark_id,
						int parent_id = root_folder_id
#if defined(BROWSER_TAG)
						, const char *tag1 = NULL,
						const char *tag2 = NULL,
						const char *tag3 = NULL,
						const char *tag4 = NULL
#endif
						);
	int save_bookmark(bookmark_item item, int parent_id = root_folder_id);
	int save_folder(const char *title, int *saved_bookmark_id, int parent_id = root_folder_id);
	Eina_Bool delete_by_id(int id);
	Eina_Bool delete_by_uri(const char *uri);
	//Eina_Bool delete_bookmark(const char *uri);
	//Eina_Bool delete_folder(const char *title, int parent_id = root_folder_id);
	int update_bookmark(int id, const char *title, const char *uri,
					int parent_id, int order
#if defined(BROWSER_TAG)
					, const char *tag1 = NULL,
					const char *tag2 = NULL,
					const char *tag3 = NULL,
					const char *tag4 = NULL
#endif
					);
	Eina_Bool delete_all(void);
	Eina_Bool get_item_by_id(int id, bookmark_item *item);
	Eina_Bool get_list_by_folder(const int folder_id, std::vector<bookmark_item *> &list);
	Eina_Bool get_count_by_folder(const int folder_id, int *count);
	void destroy_list(std::vector<bookmark_item *> &list);
	int get_count(void);
	Eina_Bool get_id(const char *uri, int *bookmark_id);
	Eina_Bool get_folder_id(const char *title, int parent_id, int *folder_id);
	Eina_Bool is_in_bookmark(const char *uri);
	// The return value is malloced list, it should be freed by caller.
	std::vector<bookmark_item *> get_bookmark_list(int folder_id = 1);
	Eina_Bool get_folder_depth_count(int *depth_count);
#if defined(BROWSER_TAG)
	Eina_Bool get_tag_list(std::vector<char *> &list);
	Eina_Bool get_tag_count(int *count);
	Eina_Bool get_max_tag_count_for_a_bookmark_item(int *count);
	Eina_Bool get_list_by_tag(const char *tag, std::vector<bookmark_item *> *list);
	Eina_Bool get_count_by_tag(const char *tag, int *count);
	void destroy_tag_list(std::vector<char *> &list);
#endif
private:
	Eina_Bool _convert_bookmark_entry(bookmark_item &dest, void *src);
	Eina_Bool _get_depth_count_recursive(int folder_id, int cur_depth, int *depth_count);

	std::vector<bookmark_item *> m_bookmark_list;
#if defined(BROWSER_TAG)
	std::vector<char *> m_tag_list;
#endif
};

#endif /* BOOKMARK_H */

