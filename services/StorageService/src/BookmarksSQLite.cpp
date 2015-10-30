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
#include <memory.h>
#include <boost/format.hpp>

#include "BookmarksSQLite.h"

#include <BrowserAssert.h>
#include "DriverManager.h"
#include "Blob.h"
#include "StorageException.h"
#include "StorageExceptionInitialization.h"
#include "StorageService.h"
#include "BookmarkItem.h"
#include "EflTools.h"

/// @todo Mutex all inserts to make service thread safe.

namespace{
    // ---------- TAG ---------------
    const std::string TABLE_TAG = "TAG";
    const std::string COL_TAG_ID = "id";
    const std::string COL_TAG_NAME = "name";

    const std::string DDL_CREATE_TABLE_TAG = "CREATE TABLE IF NOT EXISTS " + TABLE_TAG
                                           + " ("
                                           + COL_TAG_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, "
                                           + COL_TAG_NAME + " TEXT, "
                                           + " CONSTRAINT UK_tag_name "
                                           + " UNIQUE (" + COL_TAG_NAME + ")"
                                           + " ); ";
    const std::string DDL_DROP_TABLE_TAG = "DROP TABLE IF EXISTS " + TABLE_TAG;
    // ---------- (end) TAG ---------

    // ---------- DIR ---------------
    const std::string TABLE_DIR = "DIR";
    const std::string COL_DIR_ID = "id";
    const std::string COL_DIR_PARENT_ID = "parent_id";
    const std::string COL_DIR_NAME = "name";

    std::map<std::string, int> COL_DIR_COL_MAP =
                        {
                            {COL_DIR_ID,        1},
                            {COL_DIR_PARENT_ID, 2},
                            {COL_DIR_NAME,      3}
                        };
    const std::string DDL_CREATE_TABLE_DIR = "CREATE TABLE IF NOT EXISTS " + TABLE_DIR
                                           + "("
                                           + COL_DIR_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, "
                                           + COL_DIR_PARENT_ID + " INTEGER DEFAULT 0, "
                                           + COL_DIR_NAME + " TEXT, "
                                           + " CONSTRAINT FK_parent_id "
                                           + " FOREIGN KEY(" + COL_DIR_PARENT_ID
                                           + " )"
                                           + " REFERENCES " + TABLE_DIR + "(" + COL_DIR_ID + ")"
                                           + " ON DELETE CASCADE "
                                           + " ON UPDATE CASCADE "
                                           + " );";

    const std::string DDL_INSER_ROOT_TABLE_DIR = "INSERT OR IGNORE INTO "
                                               + TABLE_DIR
                                               + " ("
                                               + COL_DIR_ID + ","
                                               + COL_DIR_PARENT_ID + ","
                                               + COL_DIR_NAME
                                               + ") VALUES(0,0,\"root\");";
    const std::string DDL_DROP_TABLE_DIR = "DROP TABLE IF EXISTS " + TABLE_DIR;
    // ---------- (end) DIR ---------

    // ---------- BOOKMARK ---------------
    const std::string TABLE_BOOKMARK = "BOOKMARK";
    const std::string COL_BOOKMARK_ID = "id";
    const std::string COL_BOOKMARK_URL = "url";
    const std::string COL_BOOKMARK_TITLE = "title";
    const std::string COL_BOOKMARK_THUMBNAIL = "thumbnail";
    const std::string COL_BOOKMARK_WIDTH = "width";
    const std::string COL_BOOKMARK_HEIGHT = "height";
    const std::string COL_BOOKMARK_IMAGE_TYPE = "image_type";
    const std::string COL_BOOKMARK_NOTE = "note";
    const std::string COL_BOOKMARK_DIR = "dir_id";
    std::map<std::string, int>COL_BOOKMARK_COL_MAP = {
        {COL_BOOKMARK_ID,        0},
        {COL_BOOKMARK_URL,       1},
        {COL_BOOKMARK_TITLE,     2},
        {COL_BOOKMARK_THUMBNAIL, 3},
        {COL_BOOKMARK_WIDTH,     4},
        {COL_BOOKMARK_HEIGHT,    5},
        {COL_BOOKMARK_IMAGE_TYPE,6},
        {COL_BOOKMARK_NOTE,      7},
        {COL_BOOKMARK_DIR,       8}
    };
    const std::string  DDL_CREATE_TABLE_BOOKMARK = "CREATE TABLE IF NOT EXISTS " + TABLE_BOOKMARK
                                                 + " ("
                                                 + COL_BOOKMARK_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                 + COL_BOOKMARK_URL + " TEXT, "
                                                 + COL_BOOKMARK_TITLE + " TEXT, "
                                                 + COL_BOOKMARK_THUMBNAIL + " BLOB, "
                                                 + COL_BOOKMARK_WIDTH + " INTEGER DEFAULT 0, "
                                                 + COL_BOOKMARK_HEIGHT + " INTEGER DEFAULT 0, "
                                                 + COL_BOOKMARK_IMAGE_TYPE + " INTEGER DEFAULT 0, "
                                                 + COL_BOOKMARK_NOTE + " TEXT, "
                                                 + COL_BOOKMARK_DIR + " INTEGER, "
                                                 + " CONSTRAINT FK_bookmark_dir_id"
                                                 + " FOREIGN KEY(" + COL_BOOKMARK_DIR
                                                 + " ) "
                                                 + " REFERENCES " + TABLE_DIR + "(" + COL_DIR_ID + ") "
                                                 + " ON DELETE CASCADE "
                                                 + " ON UPDATE CASCADE "
                                                 + " );";

    const std::string DDL_DROP_TABLE_BOOKMARK = "DROP TABLE IF EXISTS" + TABLE_BOOKMARK;
    // ---------- (end) BOOKMARK ---------



    // ---------- TAG_BOOKMARK ---------------
    const std::string TABLE_TAG_BOOKMARK = "TAG_BOOKMARK";
    const std::string COL_TAG_BOOKMARK_TAG_ID  = "tag_id";
    const std::string COL_TAG_BOOKMARK_BOOK_ID = "bookmark_id";

    const std::string DDL_CREATE_TABLE_TAG_BOOKMARK = "CREATE TABLE IF NOT EXISTS " + TABLE_TAG_BOOKMARK
                                                    + "("
                                                    + COL_TAG_BOOKMARK_TAG_ID + " INTEGER, "
                                                    + COL_TAG_BOOKMARK_BOOK_ID + " INTEGER, "
                                                    + " CONSTRAINT FK_tag_id_tb "
                                                    + " FOREIGN KEY(" + COL_TAG_BOOKMARK_TAG_ID + " ) "
                                                    + " REFERENCES " + TABLE_TAG + "(" + COL_TAG_ID + ") "
                                                    + " ON UPDATE CASCADE ON DELETE CASCADE, "

                                                    + " CONSTRAINT FK_bookmak_id_tb "
                                                    + " FOREIGN KEY(" + COL_TAG_BOOKMARK_BOOK_ID + " ) "
                                                    + " REFERENCES " + TABLE_BOOKMARK + "(" + COL_BOOKMARK_ID + ") "
                                                    + " ON UPDATE CASCADE ON DELETE CASCADE, "

                                                    + " CONSTRAINT PK_tag_bookmark "
                                                    + " PRIMARY KEY (" + COL_TAG_BOOKMARK_TAG_ID + "," + COL_TAG_BOOKMARK_BOOK_ID + ")"
                                                    + ");";
    const std::string DDL_DROP_TABLE_TAG_BOOKMARK = "DROP TABLE IF EXISTS " + TABLE_TAG_BOOKMARK;
    // ---------- (end) TAG_BOOKMARK ---------

    // ---------- DIR_BOOKMARK ---------------
    const std::string TABLE_DIR_BOOKMARK = "DIR_BOOKMARK";
    const std::string COL_DIR_BOOKMARK_DIR_ID = "dir_id";
    const std::string COL_DIR_BOOKMARK_BOOK_ID = "bookmark_id";
    const std::string DDL_CREATE_TABLE_DIR_BOOKMARK = "CREATE TABLE IF NOT EXISTS " + TABLE_DIR_BOOKMARK
                                                    + " ( "
                                                    + COL_DIR_BOOKMARK_DIR_ID + " INTEGER, "
                                                    + COL_DIR_BOOKMARK_BOOK_ID + " INTEGER, "

                                                    + " CONSTRAINT FK_dir_id_db "
                                                    + " FOREIGN KEY(" + COL_DIR_BOOKMARK_DIR_ID + " ) "
                                                    + " REFERENCES " + TABLE_DIR + "(" + COL_DIR_ID + ") "
                                                    + " ON UPDATE CASCADE ON DELETE CASCADE, "

                                                    + " CONSTRAINT FK_bookmak_id_db "
                                                    + " FOREIGN KEY(" + COL_DIR_BOOKMARK_BOOK_ID + " ) "
                                                    + " REFERENCES " + TABLE_BOOKMARK + "(" + COL_BOOKMARK_ID + ") "
                                                    + " ON UPDATE CASCADE ON DELETE CASCADE, "

                                                    + " CONSTRAINT PK_dir_bookmark "
                                                    + " PRIMARY KEY (" + COL_DIR_BOOKMARK_DIR_ID + "," + COL_DIR_BOOKMARK_BOOK_ID + ")"
                                                    + " );";
    const std::string DDL_DROP_TABLE_DIR_BOOKMARK = "DROP TABLE IF EXISTS " + TABLE_DIR_BOOKMARK;
    // ---------- (end) DIR_BOOKMARK ---------


}

namespace tizen_browser
{
namespace services
{

BookmarksSQLite::BookmarksSQLite()
: m_isInitialised(false)
, m_isDBInitialised(false)
{

}

BookmarksSQLite::~BookmarksSQLite()
{

}

void BookmarksSQLite::init()
{
    if(!m_isInitialised){
        config.load("whatever");
        std::string resourceDbDir;
        std::string dbName;
        resourceDbDir = boost::any_cast < std::string > (config.get("resourcedb/dir"));
        dbName = boost::any_cast < std::string > (config.get("DB_BOOKMARK"));
        dbConnectionString = resourceDbDir + dbName;
        try{
            initBookmarksDatabase();
            m_isInitialised = true;
        } catch ( storage::StorageExceptionInitialization &e ){
            BROWSER_LOGE("[%s:%d] Cannot initalize database: %s! ", __PRETTY_FUNCTION__, __LINE__, dbConnectionString.c_str());
        }
    }
}

void BookmarksSQLite::initBookmarksDatabase()
{
    BROWSER_LOGD("[%s:%d] BEGIN ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_isDBInitialised){
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
        try{
            StorageService::checkAndCreateTable(&scope, TABLE_DIR, DDL_CREATE_TABLE_DIR);
            scope.database()->prepare(DDL_INSER_ROOT_TABLE_DIR).exec();
            StorageService::checkAndCreateTable(&scope, TABLE_BOOKMARK, DDL_CREATE_TABLE_BOOKMARK );

            StorageService::checkAndCreateTable(&scope, TABLE_TAG, DDL_CREATE_TABLE_TAG);
            //StorageService::checkAndCreateTable(&scope, TABLE_DIR_BOOKMARK, DDL_CREATE_TABLE_DIR_BOOKMARK);
            StorageService::checkAndCreateTable(&scope, TABLE_TAG_BOOKMARK, DDL_CREATE_TABLE_TAG_BOOKMARK);
        }catch (storage::StorageException & e) {
            BROWSER_LOGE("[%s:%d] errod code: %d: %s", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
            throw storage::StorageExceptionInitialization(e.getMessage(),
                                                          e.getErrorCode());
        }
        m_isDBInitialised = true;
    }
    M_ASSERT(m_isDBInitialised);
    BROWSER_LOGD("[%s:%d] end ", __PRETTY_FUNCTION__, __LINE__);
}

void BookmarksSQLite::addBookmarkItem(SharedBookmarkItem bookmark)
{
    boost::format checkIfExistQueryString("SELECT %1% FROM %2% WHERE %1% = \"?\";");
    checkIfExistQueryString % COL_BOOKMARK_ID % TABLE_BOOKMARK;

    boost::format updateQueryString("UPDATE %10% SET "
                                   " %1% = \"?\", "
                                   " %2% = \"?\", "
                                   " %3% = \"?\", "
                                   " %4% = \"?\", "
                                   " %5% = \"?\", "
                                   " %6% = \"?\", "
                                   " %7% = \"?\" "
                                   " %8% = \"?\", "
                                   " WHERE %9% = \"?\"; "
    );
    updateQueryString
                      % COL_BOOKMARK_URL
                      % COL_BOOKMARK_TITLE
                      % COL_BOOKMARK_THUMBNAIL
                      % COL_BOOKMARK_WIDTH
                      % COL_BOOKMARK_HEIGHT
                      % COL_BOOKMARK_IMAGE_TYPE
                      % COL_BOOKMARK_NOTE
                      % COL_BOOKMARK_DIR
                      % COL_BOOKMARK_ID
                      % TABLE_BOOKMARK;

    boost::format insertQueryString("INSERT INTO %9% ("
                                   " %1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%) VALUES "
                                   "( ? ,  ? ,  ? ,  ? ,  ? ,  ? ,  ? ,  ? );"
    );
    insertQueryString % COL_BOOKMARK_URL
                      % COL_BOOKMARK_TITLE
                      % COL_BOOKMARK_THUMBNAIL
                      % COL_BOOKMARK_WIDTH
                      % COL_BOOKMARK_HEIGHT
                      % COL_BOOKMARK_IMAGE_TYPE
                      % COL_BOOKMARK_NOTE
                      % COL_BOOKMARK_DIR
                      % TABLE_BOOKMARK;
    try{
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        //check if bookmark exists
        if(bookmark->getId()!=0){
            storage::SQLQuery checkIfExistQuery(db->prepare(checkIfExistQueryString.str()));
            checkIfExistQuery.bindInt(1, bookmark->getId());
            checkIfExistQuery.exec();
            if(checkIfExistQuery.hasNext()){//item exists updating
                storage::SQLQuery updateQuery(db->prepare(updateQueryString.str()));
                updateQuery.bindText(1, bookmark->getAddress());
                updateQuery.bindText(2, bookmark->getTittle());

                std::shared_ptr<tizen_browser::tools::BrowserImage> thumb = bookmark->getThumbnail();
                if(!bookmark->getThumbnail()){
                    thumb = std::make_shared<tizen_browser::tools::BrowserImage>();
                }
                updateQuery.bindBlob(3, std::move(tizen_browser::tools::EflTools::getBlobPNG(thumb)));
                updateQuery.bindInt(4, thumb->width);
                updateQuery.bindInt(5, thumb->height);
                updateQuery.bindInt(6, tizen_browser::tools::BrowserImage::ImageTypePNG);

                updateQuery.bindText(7, bookmark->getNote());
                updateQuery.bindInt(8, bookmark->getDir());
                updateQuery.bindInt(9, bookmark->getId());
                updateQuery.exec();
                return;
            }else{
                BROWSER_LOGD("[%s:%d] Bookmark has ID but there is no such in db: ignoring bookmakr id: %d ", __PRETTY_FUNCTION__, __LINE__, bookmark->getId());
            }
        }else{
            //inserting new bookmark
            storage::SQLQuery insertQuery(db->prepare(insertQueryString.str()));
            insertQuery.bindText(1, bookmark->getAddress());
            insertQuery.bindText(2, bookmark->getTittle());

            std::shared_ptr<tizen_browser::tools::BrowserImage> thumb = bookmark->getThumbnail();
            if(!bookmark->getThumbnail()){
                thumb = std::make_shared<tizen_browser::tools::BrowserImage>();
            }
            insertQuery.bindBlob(3, std::move(tizen_browser::tools::EflTools::getBlobPNG(thumb)));
            insertQuery.bindInt(4, thumb->width);
            insertQuery.bindInt(5, thumb->height);
            insertQuery.bindInt(6, tizen_browser::tools::BrowserImage::ImageTypePNG);

            insertQuery.bindText(7, bookmark->getNote());
            insertQuery.bindInt(8, bookmark->getDir());
            insertQuery.exec();
            bookmark->setId(db->lastInsertId());
        }
    }catch (storage::StorageException & e) {

        BROWSER_LOGE("[%s:%d] SQLException (%d): %s", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());

        throw;
    }

}

void BookmarksSQLite::deleteBookmark(SharedBookmarkItem bookmark)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(bookmark->getId() > 0 ){
        boost::format deleteString("DELETE FROM %1% WHERE %2% = ?;");
        deleteString % TABLE_BOOKMARK % COL_BOOKMARK_ID;
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
        try{
            std::shared_ptr<storage::SQLDatabase> db = scope.database();
            storage::SQLQuery deleteBookTab(db->prepare(deleteString.str()));
            deleteBookTab.bindInt(1, bookmark->getId());
            deleteBookTab.exec();
            BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        }catch(storage::StorageException &e ){
            BROWSER_LOGD("[%s:%d] Clear Bookmark Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
            throw;
        }
    }
}

void BookmarksSQLite::clearBookmarks()
{
    boost::format deleteAllString("DELETE FROM %1%;");
    deleteAllString % TABLE_BOOKMARK;
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
    try{
        std::shared_ptr<storage::SQLDatabase> db = scope.database();


        storage::SQLQuery deleteBookTab(db->prepare(deleteAllString.str()));
        deleteBookTab.exec();

    }catch(storage::StorageException &e ){
        BROWSER_LOGD("[%s:%d] Clear Bookmark Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        throw;
    }

}

unsigned int BookmarksSQLite::addDir(const std::string& name, unsigned int parentDirId)
{
    boost::format insertQueryString("INSERT INTO %1% ( %2%, %3%) VALUES ( ? , ?); ");
    insertQueryString % TABLE_DIR % COL_DIR_PARENT_ID % COL_DIR_NAME;

    try{
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
        std::shared_ptr<storage::SQLDatabase> db = scope.database();

        storage::SQLQuery insertQuery(db->prepare(insertQueryString.str()));
        insertQuery.bindInt(1, parentDirId);
        insertQuery.bindText(2, name);
        insertQuery.exec();
        return db->lastInsertId();
    }catch (storage::StorageException & e) {

        BROWSER_LOGE("[%s:%d] SQLException (%d): %s", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());

        throw;
    }
}
void BookmarksSQLite::removeDir(unsigned int dirId)
{
    boost::format deleteDirString("DELETE FROM %1% WHERE %2% = ?;");
    deleteDirString % TABLE_DIR % COL_DIR_ID;
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
    try{
        std::shared_ptr<storage::SQLDatabase> db = scope.database();


        storage::SQLQuery deleteDirQuery(db->prepare(deleteDirString.str()));
        deleteDirQuery.bindInt(1, dirId);
        deleteDirQuery.exec();
    }catch(storage::StorageException &e ){
        BROWSER_LOGD("[%s:%d] Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        throw;
    }
}

tizen_browser::services::BookmarkPath BookmarksSQLite::getDirPathFromId(unsigned int dirId)
{
    BookmarkPath bpath;
    try{
        BookmarkPathItem bpi;
        bpi = getDir(dirId);
        bpath.push_front(bpi);
        while(bpi.id != 0 ){
            bpi = getDir(bpi.parentId);
            bpath.push_front(bpi);
        }
    }catch(storage::StorageException &e ){
        BROWSER_LOGD("[%s:%d] Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        throw;
    }
    return bpath;
}
BookmarkPathItem BookmarksSQLite::getDir(unsigned int dirId)
{
    boost::format getDirString("SELECT %1%, %2%, %3% FROM %4% WHERE %1% = ?;");
    getDirString % COL_DIR_ID % COL_DIR_PARENT_ID % COL_DIR_NAME;

    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
    BookmarkPathItem item;
    try{
        std::shared_ptr<storage::SQLDatabase> db = scope.database();


        storage::SQLQuery getDirQuery(db->prepare(getDirString.str()));
        getDirQuery.bindInt(1, dirId);
        getDirQuery.exec();
        if(getDirQuery.hasNext()){
            item.id = getDirQuery.getInt(1);
            item.parentId = getDirQuery.getInt(2);
            item.name = getDirQuery.getString(3);
            return item;
        }

    }catch(storage::StorageException &e ){
        BROWSER_LOGD("[%s:%d] Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        throw;
    }
    return item;
}


unsigned int BookmarksSQLite::addTag(const std::string& tagName)
{
    if(tagName.length()==0){
        return 0;
    }

    boost::format checkTagString("SELECT %1%, %2% FROM %3% WHERE %2% = ? ;");
    checkTagString % COL_TAG_ID % COL_TAG_NAME % TABLE_TAG;

    boost::format insertTagString("INSERT INTO %1% ( %2% ) VALUES ( ? )");
    insertTagString % TABLE_TAG % COL_TAG_NAME;


    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
    std::shared_ptr<storage::SQLDatabase> db = scope.database();

    try{
        storage::SQLQuery checkTag(db->prepare(checkTagString.str()));
        checkTag.bindText(1, tagName);
        checkTag.exec();
        if(checkTag.hasNext()){
            return checkTag.getInt(1);
        } else {
            storage::SQLQuery insertTag(db->prepare(insertTagString.str()));
            insertTag.bindText(1, tagName);
            insertTag.exec();
            return db->lastInsertId();
        }

    } catch (storage::StorageException &e){
        BROWSER_LOGE("[%s:%d] sql exception (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        throw;
    }

}

std::map< unsigned int, std::string > BookmarksSQLite::getAllTags()
{
    boost::format getAllTasString("SELECT %1%, %2% FROM %3%;");
    getAllTasString % COL_TAG_ID % COL_TAG_NAME % TABLE_TAG;


    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
    std::map< unsigned int, std::string> bookmarks;
    try{
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        storage::SQLQuery getAllTagsQuery(db->prepare(getAllTasString.str()));
        getAllTagsQuery.exec();
        while(getAllTagsQuery.hasNext()){
            bookmarks[getAllTagsQuery.getInt(1)] = getAllTagsQuery.getString(2);
            getAllTagsQuery.next();
        }
    }catch(storage::StorageException &e ){
        BROWSER_LOGD("[%s:%d] Clear Bookmark Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        throw;
    }
    return bookmarks;
}

std::string BookmarksSQLite::getTagName(unsigned int id)
{

    boost::format getTagNameString("SELECT %1% FROM %2% WHERE = ?;");
    getTagNameString % COL_TAG_NAME % TABLE_BOOKMARK;

    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
    try{
        std::shared_ptr<storage::SQLDatabase> db = scope.database();


        storage::SQLQuery getTagNameQuery(db->prepare(getTagNameString.str()));
        getTagNameQuery.bindInt(1, id);
        getTagNameQuery.exec();

        if(getTagNameQuery.hasNext()){
            return getTagNameQuery.getString(1);
        }

    }catch(storage::StorageException &e ){
        BROWSER_LOGD("[%s:%d] Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        throw;
    }

    return std::string();
}

std::vector< unsigned int > BookmarksSQLite::getTagsForBookmark(unsigned int bookmarkId)
{
    boost::format getTags("SELECT %1% FROM %2% WERE %3% = \"?\";");
    getTags % COL_TAG_BOOKMARK_TAG_ID % TABLE_TAG_BOOKMARK % COL_TAG_BOOKMARK_BOOK_ID;
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));

    std::vector<unsigned int> tags;

    try{
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        storage::SQLQuery tagsSelect(db->prepare(getTags.str()));
            tagsSelect.bindInt(1, bookmarkId);
            tagsSelect.exec();
            while(tagsSelect.hasNext()){
                tags.push_back(tagsSelect.getInt(1));
                tagsSelect.next();
            }
    }catch(storage::StorageException &e ){
        BROWSER_LOGD("[%s:%d] Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        throw;
    }
    return tags;
}

SharedBookmarkItem BookmarksSQLite::readBookmark(storage::SQLQuery& bookQuery)
{
    try{
        if (bookQuery.hasNext()){
            //get base data
            SharedBookmarkItem bookmarkItem = std::make_shared<BookmarkItem>(
                        bookQuery.getString(COL_BOOKMARK_COL_MAP[COL_BOOKMARK_URL]),
                        bookQuery.getString(COL_BOOKMARK_COL_MAP[COL_BOOKMARK_TITLE]),
                        bookQuery.getString(COL_BOOKMARK_COL_MAP[COL_BOOKMARK_NOTE]),
                        bookQuery.getInt(COL_BOOKMARK_COL_MAP[COL_BOOKMARK_DIR]),
                        bookQuery.getInt(COL_BOOKMARK_COL_MAP[COL_BOOKMARK_ID]) );

            //bookmarkItem->setTags(getTagsForBookmark(bookmarkItem->getId()));

            //get thumbnail
            std::shared_ptr<tizen_browser::tools::Blob> blob = std::make_shared<tizen_browser::tools::Blob>();
            std::shared_ptr<tizen_browser::tools::BrowserImage> bi = std::make_shared<tizen_browser::tools::BrowserImage>();
            blob = bookQuery.getBlob(COL_BOOKMARK_COL_MAP[COL_BOOKMARK_THUMBNAIL]);
            if(blob.get()){
                if(blob->getLength() > 0 ){
                    const auto autoImageType = bookQuery.getInt(COL_BOOKMARK_COL_MAP[COL_BOOKMARK_IMAGE_TYPE]);
                    bi->imageType = static_cast<tizen_browser::tools::BrowserImage::ImageType>(autoImageType);
                    bi->width = bookQuery.getInt(COL_BOOKMARK_COL_MAP[COL_BOOKMARK_WIDTH]);
                    bi->height = bookQuery.getInt(COL_BOOKMARK_COL_MAP[COL_BOOKMARK_HEIGHT]);
                    bi->dataSize = blob->transferData(&bi->imageData);
                }
            }

            bookmarkItem->setThumbnail(bi);

            /// @todo: get favicon
            //bookmarkItem->setFavicon(std::make_shared<tizen_browser::tools::BrowserImage>());
            return bookmarkItem;
        }
    } catch (storage::StorageException &e){
        BROWSER_LOGE("[%s:%d] Error (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }

    //no items or error.
    return std::make_shared<BookmarkItem>();
}

SharedBookmarkItem BookmarksSQLite::getBookmarksById(unsigned int id)
{
    if(id == 0)
        return std::make_shared<BookmarkItem>();


    boost::format selectOne("SELECT %1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9% FROM %10% WHERE %1% = \"?\";");


    selectOne % COL_BOOKMARK_ID
              % COL_BOOKMARK_URL
              % COL_BOOKMARK_TITLE
              % COL_BOOKMARK_THUMBNAIL
              % COL_BOOKMARK_WIDTH
              % COL_BOOKMARK_HEIGHT
              % COL_BOOKMARK_IMAGE_TYPE
              % COL_BOOKMARK_NOTE
              % COL_BOOKMARK_DIR
              % TABLE_BOOKMARK
              ;

    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
    std::shared_ptr<storage::SQLDatabase> db = scope.database();

    try{
        storage::SQLQuery select(db->prepare(selectOne.str()));
        select.bindInt(1, id);
        select.exec();
        return readBookmark(select);
    } catch (storage::StorageException &e){
        BROWSER_LOGE("[%s:%d] sql exception (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }

    //no items or error.
    return std::make_shared<BookmarkItem>();
}

SharedBookmarkItemList BookmarksSQLite::getBookmarksAll()
{

    boost::format getAllBookmarksString("SELECT %1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9% FROM %10%;");

    getAllBookmarksString
            % COL_BOOKMARK_ID
            % COL_BOOKMARK_URL
            % COL_BOOKMARK_TITLE
            % COL_BOOKMARK_THUMBNAIL
            % COL_BOOKMARK_WIDTH
            % COL_BOOKMARK_HEIGHT
            % COL_BOOKMARK_IMAGE_TYPE
            % COL_BOOKMARK_NOTE
            % COL_BOOKMARK_DIR
            % TABLE_BOOKMARK
            ;

    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
    SharedBookmarkItemList bookmarksList;
    try{
        std::shared_ptr<storage::SQLDatabase> db = scope.database();


        storage::SQLQuery getAllBookmarksQuery(db->prepare(getAllBookmarksString.str()));
        getAllBookmarksQuery.exec();

        while(getAllBookmarksQuery.hasNext()){
            bookmarksList.push_back(readBookmark(getAllBookmarksQuery));
            getAllBookmarksQuery.next();
        }
    }catch(storage::StorageException &e ){
        BROWSER_LOGD("[%s:%d] Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return bookmarksList;
}

SharedBookmarkItemList BookmarksSQLite::getBookmarksByDir(unsigned int dirId)
{
    boost::format getBooksString("SELECT %1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%"
                                 "FROM %10%, %11%"
                                 "WHERE %1% = %12% AND %13% = ?;");

    getBooksString
            % COL_BOOKMARK_ID           //1
            % COL_BOOKMARK_URL          //
            % COL_BOOKMARK_TITLE        //
            % COL_BOOKMARK_THUMBNAIL    //
            % COL_BOOKMARK_WIDTH        //5
            % COL_BOOKMARK_HEIGHT       //
            % COL_BOOKMARK_IMAGE_TYPE   //
            % COL_BOOKMARK_NOTE         //
            % COL_BOOKMARK_DIR          //
            % TABLE_BOOKMARK            //10
            % TABLE_DIR_BOOKMARK        //
            % COL_DIR_BOOKMARK_BOOK_ID  //
            % COL_DIR_BOOKMARK_DIR_ID   //13

            ;

    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
    SharedBookmarkItemList bookmarksList;
    try{
        std::shared_ptr<storage::SQLDatabase> db = scope.database();


        storage::SQLQuery getBookmarksQuery(db->prepare(getBooksString.str()));
        getBookmarksQuery.bindInt(1, dirId);
        getBookmarksQuery.exec();

        while(getBookmarksQuery.hasNext()){
            bookmarksList.push_back(readBookmark(getBookmarksQuery));
            getBookmarksQuery.next();
        }
    }catch(storage::StorageException &e ){
        BROWSER_LOGD("[%s:%d] Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return bookmarksList;
}

SharedBookmarkItemList BookmarksSQLite::getBookmarksByTag(const std::vector<unsigned int>& tagList)
{
    if(tagList.size()==0)
        return SharedBookmarkItemList();

    std::string questionMarks;
    for(int i = tagList.size()-1; i>=0; i--){
        questionMarks+="?,";
    }
    questionMarks = questionMarks.substr(0, questionMarks.size() - 1);

    boost::format getBooksString("SELECT DISTINCT %1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%"
                                 "FROM %10%, %11%"
                                 "WHERE %1% = %12% AND %13% in ( %14% ) ;");

    getBooksString
            % COL_BOOKMARK_ID           //1
            % COL_BOOKMARK_URL          //
            % COL_BOOKMARK_TITLE        //
            % COL_BOOKMARK_THUMBNAIL    //
            % COL_BOOKMARK_WIDTH        //5
            % COL_BOOKMARK_HEIGHT       //
            % COL_BOOKMARK_IMAGE_TYPE   //
            % COL_BOOKMARK_NOTE         //
            % COL_BOOKMARK_DIR          //
            % TABLE_BOOKMARK            //10
            % TABLE_TAG_BOOKMARK        //
            % COL_TAG_BOOKMARK_BOOK_ID  //
            % COL_TAG_BOOKMARK_TAG_ID   //13
            % questionMarks;

    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(dbConnectionString));
    SharedBookmarkItemList bookmarksList;
    try{
        std::shared_ptr<storage::SQLDatabase> db = scope.database();


        storage::SQLQuery getBookmarksQuery(db->prepare(getBooksString.str()));
        for(unsigned int i=0; i< tagList.size(); i++){
            getBookmarksQuery.bindInt(i+1, tagList.at(i));
        }
        getBookmarksQuery.exec();

        while(getBookmarksQuery.hasNext()){
            bookmarksList.push_back(readBookmark(getBookmarksQuery));
            getBookmarksQuery.next();
        }
    }catch(storage::StorageException &e ){
        BROWSER_LOGD("[%s:%d] Error: %d: %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return bookmarksList;
}





}
}
