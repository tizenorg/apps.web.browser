/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *
 * Created on: Dec, 2014
 *     Author: m.kielak
 */

#ifndef BOOKMARKSERVICE_H
#define BOOKMARKSERVICE_H

#include "browser_config.h"
#include <vector>
#include <boost/signals2/signal.hpp>
#include <Evas.h>

#include "AbstractService.h"
#include "service_macros.h"
#include "BookmarkItem.h"
#include "BrowserImage.h"
#include "Config/Config.h"
#include "AbstractFavoriteService.h"

namespace tizen_browser{
namespace services{

class BROWSER_EXPORT BookmarkService
        : public tizen_browser::interfaces::AbstractFavoriteService
{
public:
    BookmarkService();
    virtual ~BookmarkService();
    virtual std::string getName();

    /**
     * @brief Add page to bookmarks
     *
     * @param address Webpage url.
     * @param tittle Title of bookmark.
     * @param note Bookmark note, default is empty .
     * @param dirId Directory numeric ID, default is 0.
     * @param thumbnail Page thumbnail, default is empty image.
     * @param favicon Page favicon image, default is empty image.
     *
     * @return BookmarkItem class
     */
    std::shared_ptr<BookmarkItem> addToBookmarks(const std::string & address,
                                                 const std::string & tittle,
                                                 const std::string & note = std::string(),
                                                 std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail=std::shared_ptr<tizen_browser::tools::BrowserImage>(),
                                                 std::shared_ptr<tizen_browser::tools::BrowserImage> favicon = std::shared_ptr<tizen_browser::tools::BrowserImage>(),
                                                 unsigned int dirId = 0);
    /**
     * @brief Count bookmarks and subfolders
     * @return Number of bookmarks and subfolders
     */
    int countBookmarksAndSubFolders();

    /** \todo Need to change this callback function for finding stored bookmark, check getBookmarkId function
     * @brief Check if bookmark exists
     *
     * @param url url to find
     * @return true if exists, false if not
     */
     bool bookmarkExists(const std::string & url);

    /**
     * @brief Get bookmarks from platform service and store it in private m_bookmarksList
     *
     * @return list of bookmark items, bookmark items in a folder & bookmark folders
     */
    std::vector<std::shared_ptr<BookmarkItem> > getBookmarks(int folder_id = 0);
    std::vector<std::shared_ptr<BookmarkItem> > getBookmarkFolders();

   /**
     * @brief Delete all bookmarks
     *
     * @return true if success, false on error
     */
    bool deleteAllBookmarks();

    /**
     * @brief Delete bookmark by given url
     *
     * @param url of bookmark to delete
     * @return true if success, false on error of not found bookmark
     */
    bool deleteBookmark(const std::string & url);


    void synchronizeBookmarks();

    typedef struct _folder_info {
        int folder_id;
        char *folder_name;
    } folder_info;

    int get_root_folder_id(void);
    int save_folder(const char *title, int *saved_bookmark_id, int parent_id=0,int by_operator = 0);
    bool delete_by_id(int id);
    bool delete_by_id_notify(int id);
    bool delete_by_uri(const char *uri);
    int update_bookmark(int id, const char *title, const char *uri, int parent_id, int order,
                        bool is_duplicate_check_needed = false, bool is_URI_check_needed = false);
    int update_bookmark_notify(int id, const char *title, const char *uri, int parent_id, int order,
                               bool is_duplicate_check_needed = false, bool is_URI_check_needed = false);
    bool delete_all(void);
    bool get_item_by_id(int id, BookmarkItem *item);
    std::vector<BookmarkItem *> get_list_by_folder(const int folder_id);
    bool get_list_users_by_folder(const int folder_id, std::vector<BookmarkItem *> &list);
    bool get_list_by_dividing(const int folder_id, std::vector<BookmarkItem *> &list, int *last_item, int genlist_block_size);
    bool get_list_users_by_dividing(const int folder_id, std::vector<BookmarkItem *> &list, int *last_item, int genlist_block_size);
    int _get_folder_count(const int folder_id);
    int _get_bookmarks_count(const int folder_id);
    int _get_total_contents_count(const int folder_id);
    bool get_list_operators(const int folder_id, std::vector<BookmarkItem *> &list);
    bool get_list_by_keyword(const char *keyword, std::vector<BookmarkItem *> &list);
    void destroy_list(std::vector<BookmarkItem *> &list);
    int get_count(void);
    bool get_id(const char *uri, int *bookmark_id);
    bool get_folder_id(const char *title, int parent_id, int *folder_id);
    bool is_in_bookmark(const char *uri);
    bool is_in_folder(int parent_folder, const char *title);
    bool get_folder_depth_count(int *depth_count);
    bool set_thumbnail(int id, Evas_Object *thumbnail);
    Evas_Object *get_thumbnail(int id, Evas_Object *parent);
    bool set_favicon(int id, Evas_Object *favicon);
    Evas_Object *get_favicon(int id, Evas_Object *parent);
    bool set_webicon(int id, Evas_Object *webicon);
    Evas_Object *get_webicon(int id, Evas_Object *parent);
    bool set_access_count(int id, int count);
    bool get_access_count(int id, int *count);
    bool increase_access_count(int id);
    bool set_last_sequence(int id);
    const char* get_path_info(void);
    folder_info *get_path_by_index(unsigned int index);
    int get_path_size(void);
    void push_back_path(folder_info *item);
    void pop_back_path(void);
    void clear_path_history(void);
    void free_path_history(void);
    void change_path_lang(void);
    void path_into_sub_folder(int folder_id, const char *folder_name);
    bool path_to_upper_folder(int *curr_folder);

    bool get_memory_full(void) { return m_memory_full; }
    int get_current_folder_id(void) { return m_curr_folder; }

private:
    bool _get_depth_count_recursive(int folder_id, int cur_depth, int *depth_count);

    std::vector<BookmarkItem *> m_bookmark_list;
    std::vector<folder_info *> m_path_history;
    std::string m_path_string;
    bool m_memory_full;
    bool m_bookmark_adaptor_initialize;
    int m_curr_folder;
    std::shared_ptr<tizen_browser::services::StorageService> m_storageManager;
    std::vector<std::shared_ptr<BookmarkItem> > m_bookmarks;

///    \todo Need to change getBookmarkId function for finding stored bookmark - check getBookmarkExists function
    int getBookmarkId(const std::string & url);
    std::shared_ptr<tizen_browser::services::StorageService> getStorageManager();
    config::DefaultConfig config;
};

}
}

#endif // FAVORITESERVICE_H
