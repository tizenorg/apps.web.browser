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
 *
 *
 */

#ifndef BOOKMARKSSQLITE_H
#define BOOKMARKSSQLITE_H

#include <map>
#include "BookmarkItem.h"
#include "BookmarksStorage.h"
#include "Config.h"
#include "SQLDatabase.h"

namespace tizen_browser
{
namespace services
{


class BookmarksSQLite:public BookmarksStorage
{
public:
    BookmarksSQLite();
    virtual ~BookmarksSQLite();
    void init();
    virtual void addBookmarkItem(SharedBookmarkItem bookmark);
    virtual void deleteBookmark(SharedBookmarkItem bookmark);
    virtual void clearBookmarks();

    virtual unsigned int addDir(const std::string& name, unsigned int parentDirId = 0);
    virtual void removeDir(unsigned int dirId);
    virtual BookmarkPath getDirPathFromId(unsigned int dirId);


    virtual unsigned int addTag(const std::string& tagName);
    virtual std::map<unsigned int, std::string> getAllTags();
    virtual std::string getTagName(unsigned int id);
    virtual std::vector< unsigned int > getTagsForBookmark(unsigned int bookmarkId);



    virtual SharedBookmarkItem getBookmarksById(unsigned int id);
    virtual SharedBookmarkItemList getBookmarksAll();
    virtual SharedBookmarkItemList getBookmarksByDir(unsigned int dirId);
    virtual SharedBookmarkItemList getBookmarksByTag(const std::vector< unsigned int >& tagList);

private:
    BookmarkPathItem getDir(unsigned int dirId);
    void initBookmarksDatabase();
    SharedBookmarkItem readBookmark(storage::SQLQuery &bookQuery);

    bool m_isInitialised;
    bool m_isDBInitialised;
    config::DefaultConfig config;
    std::string dbConnectionString;
};

}//namespace services
}//namespace tizen_browser

#endif // BOOKMARKSSQLITE_H
