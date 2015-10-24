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

#ifndef BOOKMARK_ITEM_H
#define BOOKMARK_ITEM_H

#include <Evas.h>
#include "browser-object.h"

class bookmark_item : public browser_object {
public:
	bookmark_item(const char *title = NULL, const char *uri = NULL, int id = 0, int parent_id = 0, int order = 0, Eina_Bool is_folder = EINA_FALSE);
	~bookmark_item(void);

	/* If the title & uri is same, they are equal. */
	friend bool operator==(bookmark_item &item1, bookmark_item &item2) {
		return ((!item1.get_uri() && !item2.get_uri()) || !strcmp(item1.get_uri(), item2.get_uri()));
	}
	bookmark_item& operator=(const bookmark_item &item_src);

	void set_title(const char *title);
	const char *get_title(void) const { return m_title; }
	void set_uri(const char *uri);
	const char *get_uri(void) const { return m_uri; }
	Eina_Bool is_folder(void) const { return m_is_folder; }
	Eina_Bool is_editable(void) const { return m_is_editable; }
	int is_modified(void) const { return m_is_modified; }
	void set_folder_flag(Eina_Bool flag) { m_is_folder = flag; }
	void set_editable_flag(Eina_Bool flag) { m_is_editable = flag; }
	int get_id(void) const { return m_id; }
	void set_id(int id) { m_id = id; }
	int get_parent_id(void) const { return m_parent_id; }
	void set_parent_id(int parent_id) { m_parent_id = parent_id; }
	int get_order(void) const { return m_order; }
	void set_order(int order) { m_order = order; }
	int get_initial_order(void) const { return m_initial_order; }
	void set_initial_order(int order) { m_initial_order = order; }
	void set_modified(Eina_Bool is_modified) { m_is_modified = is_modified; }
private:
	const char *m_title;
	const char *m_uri;
	Eina_Bool m_is_folder;
	Eina_Bool m_is_editable;
	int m_id;
	int m_parent_id;
	int m_order;
	int m_initial_order;//to keep track of changes in reorder
	Eina_Bool m_is_modified;
};

#endif /* BOOKMARK_ITEM_H */

