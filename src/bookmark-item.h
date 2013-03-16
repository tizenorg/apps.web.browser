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

#ifndef BOOKMARK_ITEM_H
#define BOOKMARK_ITEM_H

#include <Evas.h>
#include "browser-object.h"

class bookmark_item : public browser_object {
public:
	bookmark_item(const char *title = NULL, const char *uri = NULL, int id = 0, int parent_id = 0, int order = 0, Eina_Bool is_folder = EINA_FALSE);
	~bookmark_item(void);

	/* If the title & uri is same, they are equal. */
	friend bool operator==(bookmark_item item1, bookmark_item item2) {
		return ((!item1.get_uri() && !item2.get_uri()) || !strcmp(item1.get_uri(), item2.get_uri()));
	}
	bookmark_item& operator=(const bookmark_item &item_src);

	void set_title(const char *title);
	const char *get_title(void) const { return m_title; }
	void set_uri(const char *uri);
	const char *get_uri(void) const { return m_uri; }
	Eina_Bool is_folder(void) const { return m_is_folder; }
	Eina_Bool is_editable(void) const { return m_is_editable; }
	void set_folder_flag(Eina_Bool flag) { m_is_folder = flag; }
	void set_editable_flag(Eina_Bool flag) { m_is_editable = flag; }
	int get_id(void) const { return m_id; }
	void set_id(int id) { m_id = id; }
	int get_parent_id(void) const { return m_parent_id; }
	void set_parent_id(int parent_id) { m_parent_id = parent_id; }
	int get_order(void) const { return m_order; }
	void set_order(int order) { m_order = order; }
#if defined(BROWSER_TAG)
	const char *get_tag1(void) const { return m_tag1; }
	Eina_Bool set_tag1(const char *tag);
	const char *get_tag2(void) const { return m_tag2; }
	Eina_Bool set_tag2(const char *tag);
	const char *get_tag3(void) const { return m_tag3; }
	Eina_Bool set_tag3(const char *tag);
	const char *get_tag4(void) const { return m_tag4; }
	Eina_Bool set_tag4(const char *tag);
#endif
private:
	const char *m_title;
	const char *m_uri;
	Eina_Bool m_is_folder;
	Eina_Bool m_is_editable;
	int m_id;
	int m_parent_id;
	int m_order;
#if defined(BROWSER_TAG)
	const char *m_tag1;
	const char *m_tag2;
	const char *m_tag3;
	const char *m_tag4;
#endif
};

#endif /* BOOKMARK_ITEM_H */

