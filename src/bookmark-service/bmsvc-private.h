/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
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

#ifndef __BMSVC_PRIVATE_H_
#define __BMSVC_PRIVATE_H_

#include <db-util.h>
#include <glib.h>

#include "bookmark-service.h"

void __bookmark_db_close();
void _internet_bookmark_db_close(sqlite3_stmt *stmt);
int _internet_bookmark_db_open();
int _internet_id_get_editable(int id);
int _internet_id_get_type(int id);
int _internet_id_is_exist(int id, int type);
/* return uniq id of item. */
int _internet_bookmark_is_exist(int folder_id, char *title, int type);

/* search lase of sequence(order's index) */
int _internet_bookmark_get_lastindex(int locationId);
int _internet_bookmark_move_to_root(int uid);
int _internet_sub_bookmarks_move_to_root(int uid);
int _internet_bookmark_new(char *address, char *title, int folder_id, int property_flag);
int _internet_bookmark_update(int uid, char *address, char *title, int folder_id);
int _internet_bookmark_remove(int id);
int _internet_bookmark_reset();

/* 1 : ok   0 : exist but can not use it    -1 : not found */
int _file_is_exist(char *filename);

int _bmsvc_bookmark_is_exist(const char *address);
int _bmsvc_folder_is_exist(const char *title, int parent_id);
int _bmsvc_get_bookmark_id(const char *address);
int _bmsvc_get_folder_id(const char *title, const int parent);
int _bmsvc_tag_exists_at_bookmarks(const char *tag_name);
int _bmsvc_tag_exists_at_taglist(const char *tag_name);
int _bmsvc_add_tag_to_taglist(const char *tag_name);
int _bmsvc_delete_tag_from_taglist(const char *tag_name);
int _bmsvc_get_folder_subitems_recursive(int parent_id, GList **list);
int _bmsvc_delete_folder_subitems(int folder_id, int check_editable);
int _bmsvc_delete_bookmark_entry(int id);
int _bmsvc_delete_bookmark_tags(bookmark_entry *entry);

void _bmsvc_destroy_bookmark_list(gpointer data);
void _bmsvc_destroy_bookmark_entry(gpointer data);

int _bmsvc_copy_bookmark_entry_deep(bookmark_entry *dest, bookmark_entry *src);

int _bmsvc_update_bookmark_record(int id, const char *title, const char *address, int parent_id, int order,
						const char *tag1,
						const char *tag2,
						const char *tag3,
						const char *tag4);
int _bmsvc_update_folder_record(int id, const char *title, int parent_id, int order);

#endif