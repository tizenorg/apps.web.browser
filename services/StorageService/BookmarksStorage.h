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

#ifndef __BOOKMARKSTORAGE_H
#define __BOOKMARKSTORAGE_H

#include <vector>
#include <memory>
#include <map>
#include <list>
#include <vector>

#include "BrowserImage.h"

#include "BookmarkItem.h"

namespace tizen_browser
{
namespace services
{


struct BookmarkPathItem{
    unsigned int id;
    unsigned int parentId;
    std::string name;
};

typedef std::list<BookmarkPathItem> BookmarkPath;

class BookmarksStorage
{
public:
    BookmarksStorage();
    virtual ~BookmarksStorage();

    /**
     * @brief Add Bookmark to storage if bookmark exists it is updated.
     *
     * If bookmark id is not equal 0 it is treated as existing bookmark and bookmark is updated.
     *
     * If bookmark id is 0 and update is successful new ID is set to bookmark.
     */
    virtual void addBookmarkItem(SharedBookmarkItem bookmark) = 0;

    /**
     * @brief Delete all bookmarks.
     *
     * @return void
     */
    virtual void clearBookmarks() = 0;


    /**
     * @brief Deletes passed bookmark.
     */
    virtual void deleteBookmark(SharedBookmarkItem bookmark) = 0;

    /**
     * @brief Returns bookmark with given ID.
     *
     * @param id internal bookmark ID.
     * @return tizen_browser::services::SharedBookmarkItem
     */
    virtual SharedBookmarkItem     getBookmarksById(unsigned int id) = 0;

    /**
     * @brief Returns list of bookmarks form a directory.
     *
     * @param dirId Direcotry ID;
     * @return tizen_browser::services::SharedBookmarkItemList
     */
    virtual SharedBookmarkItemList getBookmarksByDir(unsigned int dirId) = 0;

    /**
     * @brief Returns all bookmarks in storage
     *
     * @return tizen_browser::services::SharedBookmarkItemList
     */
    virtual SharedBookmarkItemList getBookmarksAll() = 0;
    /**
     * @brief Return list of bookmarks with any of the tags on list
     *
     * @param tagList ...
     * @return tizen_browser::services::SharedBookmarkItemList
     */

    virtual SharedBookmarkItemList getBookmarksByTag(const std::vector< unsigned int >& tagList) = 0;

    /**
     * @brief Returns name of the tag.
     *
     * @param  id of a tag.
     * @return std::string
     */
    virtual std::string getTagName(unsigned int id) = 0;

    /**
     * @brief Adds new tag and reurns its id.
     *
     * If Tag already exists returns it's id.
     *
     * @param tagName Tag to add
     * @return unsigned int
     */
    virtual unsigned int addTag(const std::string& tagName) = 0;


    /**
     * @brief Returns map of all Tags with id's
     *
     * @return std::map< unsigned::int, std::string >
     */
    virtual std::map<unsigned int, std::string> getAllTags() = 0;

    /**
     * @brief Reurns list of tags for given bookmarkId.
     *
     * @param bookmarkId bookmark to get tags for.
     * @return std::vector< unsigned int> list of tags id.
     */
    virtual std::vector<unsigned int> getTagsForBookmark(unsigned int bookmarkId) = 0;


//------------- Bookmark dir/path ------------------------
    /**
     * @brief Returns directory path.
     *
     * @param dirId Id of a directory.
     * @return tizen_browser::services::BookmarkPath
     */
    virtual BookmarkPath getDirPathFromId(unsigned int dirId) = 0;


    /**
     * @brief Adds new direcotr.
     *
     * Creates new directory and returns its ID.
     * If directory already exists return its ID with no error.
     *
     * @param name Directory name.
     * @param parentDirId Id of parent directory, default is 0 which is root dir.
     * @return unsigned int id of created directory.
     */
    virtual unsigned int addDir(const std::string& name, unsigned int parentDirId = 0) = 0;

    /**
     * @brief Removes directory and all its, content.
     *
     * @param dirId Directory id to be removed.
     * @return void
     */

    virtual void removeDir(unsigned int dirId) = 0;


};

}
}

#endif
