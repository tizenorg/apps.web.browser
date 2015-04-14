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

#include "HistorySQLite.h"

#include <BrowserAssert.h>

#include "BrowserLogger.h"
#include "DriverManager.h"
#include "Blob.h"
#include "StorageException.h"
#include "StorageExceptionInitialization.h"
#include "HistoryException.h"
#include "EflTools.h"
#include "StorageService.h"

namespace
{
// ------ Database FAVICON ------
const std::string TABLE_FAVICON = "FAVICON";
//const std::string COL_FAVICON_ID = "id";
const std::string COL_FAVICON_IMAGE_TYPE = "image_type";
const std::string COL_FAVICON_WIDTH = "width";
const std::string COL_FAVICON_HEIGHT = "height";
const std::string COL_FAVICON_URI_PK = "uri";

const std::string COL_FAVICON_FAVICON = "fav_icon";

const std::string FAVICON_MODIFIERS = "DEFERRABLE INITIALLY DEFERRED";

const std::string DDL_CREATE_TABLE_FAVICON = "CREATE TABLE " + TABLE_FAVICON
                                             + "(" + COL_FAVICON_URI_PK + " TEXT PRIMARY KEY, "
                                             + COL_FAVICON_WIDTH + " INTEGER DEFAULT 0, "
                                             + COL_FAVICON_HEIGHT + " INTEGER DEFAULT 0, "
                                             + COL_FAVICON_IMAGE_TYPE + " INTEGER DEFAULT 0, "
                                             + COL_FAVICON_FAVICON + " BLOB "
                                             + ")";

const std::string INSERT_TABLE_FAVICON = "insert into " + TABLE_FAVICON + " ("
                                         + COL_FAVICON_URI_PK + ", "
                                         + COL_FAVICON_WIDTH + ", "
                                         + COL_FAVICON_HEIGHT + ", "
                                         + COL_FAVICON_IMAGE_TYPE + ", "
                                         + COL_FAVICON_FAVICON
                                         + ") values (?, ?, ?, ?, ?)";

//const std::string VIRTUAL_COL_FAVICON_ROWID = "rowid";
//const std::string SELECT_FAVICON_LAST_ID = "SELECT last_insert_rowid() AS " + VIRTUAL_COL_FAVICON_ROWID + " FROM " + TABLE_FAVICON + " LIMIT 1";
const std::string SELECT_FAVICON_EXISTS =
    "select " + COL_FAVICON_URI_PK + " from " + TABLE_FAVICON
    + " where " + COL_FAVICON_URI_PK + "=?";

const std::string DELETE_ALL_FAVICON = "delete from " + TABLE_FAVICON;
// ------ (end) Database FAVICON ------

// ------ Database HISTORY ------
const std::string TABLE_HISTORY = "HISTORY";
const std::string COL_HISTORY_URL = "url";
const std::string COL_HISTORY_TITLE = "title";
const std::string COL_HISTORY_VISIT_DATE = "visit_date";
const std::string COL_HISTORY_VISIT_COUNTER  = "visit_counter";
const std::string COL_HISTORY_URI_FK = "uri_fk";

// foreign key
//const std::string COL_HISTORY_FAVICON_ID_FK = "favicon_id_fk";


const std::string DDL_CREATE_TABLE_HISTORY = "CREATE TABLE " + TABLE_HISTORY
                                             + "(" + COL_HISTORY_URL + " TEXT PRIMARY KEY, "
                                             + COL_HISTORY_TITLE + " TEXT, "
                                             + COL_HISTORY_VISIT_DATE + " DATETIME DEFAULT (datetime('now', 'localtime')), "
                                             + COL_HISTORY_VISIT_COUNTER + " INTEGER DEFAULT 1, "
                                             + COL_HISTORY_URI_FK + " TEXT REFERENCES " + TABLE_FAVICON + "(" + COL_FAVICON_URI_PK + ") "
                                             + FAVICON_MODIFIERS
                                             + ")";

const std::string INSERT_TABLE_HISTORY = "insert or replace into " + TABLE_HISTORY + " ("
                                         + COL_HISTORY_URL + ", "
                                         + COL_HISTORY_TITLE + ", "
                                         + COL_HISTORY_VISIT_DATE + ", "
                                         + COL_HISTORY_VISIT_COUNTER + ", "
                                         + COL_HISTORY_URI_FK
                                         + ") values (?,?,?,?,?)";

const std::string INSERT_TABLE_HISTORY_DEFAULT_VALUES = "insert or replace into " + TABLE_HISTORY + " ("
                                                        + COL_HISTORY_URL + ", "
                                                        + COL_HISTORY_TITLE + ", "
                                                        + COL_HISTORY_URI_FK
                                                        + ") values (?,?,?)";

const std::string VIRTUAL_COL_HISTORY_AMOUNT = "amount";
const std::string SELECT_HISTORY_COUNT_ID = "select count(*) as " + VIRTUAL_COL_HISTORY_AMOUNT + " from " + TABLE_HISTORY;
const std::string SELECT_HISTORY_VISIT_COUNTER = "select " + COL_HISTORY_VISIT_COUNTER + " from " + TABLE_HISTORY + " where " + COL_HISTORY_URL + "=?";

const std::string DELETE_HISTORY_ITEM = "delete from " + TABLE_HISTORY + " where " + COL_HISTORY_URL + "=?";
const std::string DELETE_ALL_HISTORY = "delete from " + TABLE_HISTORY;

const std::string UPDATE_HISTORY_ITEM_COUNTER_DATE =
    "update HISTORY set visit_counter=visit_counter+1, visit_date=(datetime('now', 'localtime')) where url=?";

// ------ (end) Database HISTORY ------

const std::string SELECT_FAVICON =
    "select f." + COL_FAVICON_FAVICON + " as " + COL_FAVICON_FAVICON + ","
    + "f." + COL_FAVICON_WIDTH + " as " + COL_FAVICON_WIDTH + ","
    + "f." + COL_FAVICON_HEIGHT + " as " + COL_FAVICON_HEIGHT + ","
    + "f." + COL_FAVICON_IMAGE_TYPE + " as " + COL_FAVICON_IMAGE_TYPE
    + " from "
    + TABLE_FAVICON + " f, " + TABLE_HISTORY + " h"
    + " where f." + COL_FAVICON_URI_PK + "=h." + COL_HISTORY_URI_FK
    + " and " + COL_HISTORY_URL + "=?";

const int SELECT_FAVICON_FAVICON = 0;
const int SELECT_FAVICON_WIDTH = 1;
const int SELECT_FAVICON_HEIGHT = 2;
const int SELECT_FAVICON_IMAGE_TYPE = 3;

const std::string SELECT_FULL_HISTORY_ALL_ROW =
    "select f." + COL_FAVICON_FAVICON + " as " + COL_FAVICON_FAVICON
    + ", f." + COL_FAVICON_WIDTH + " as " + COL_FAVICON_WIDTH
    + ", f." + COL_FAVICON_HEIGHT + " as " + COL_FAVICON_HEIGHT
    + ", f." + COL_FAVICON_IMAGE_TYPE + " as " + COL_FAVICON_IMAGE_TYPE
    + ", h." + COL_HISTORY_URL + " as " + COL_HISTORY_URL
    + ", h." + COL_HISTORY_TITLE + " as " + COL_HISTORY_TITLE
    + ", h." + COL_HISTORY_VISIT_DATE + " as " + COL_HISTORY_VISIT_DATE
    + ", h." + COL_HISTORY_VISIT_COUNTER + " as " + COL_HISTORY_VISIT_COUNTER
    + ", h." + COL_HISTORY_URI_FK + " as " + COL_HISTORY_URI_FK
    + " from "
    + TABLE_HISTORY + " h left outer join " + TABLE_FAVICON + " f"
    + " on f." + COL_FAVICON_URI_PK + "=h." + COL_HISTORY_URI_FK;

const int SELECT_FULL_HISTORY_ALL_ROW_URL   = 4;
const int SELECT_FULL_HISTORY_FAVICON       = 0;
const int SELECT_FULL_HISTORY_FAVICON_IMAGE_TYPE = 3;
const int SELECT_FULL_HISTORY_FAVICON_WIDTH = 1;
const int SELECT_FULL_HISTORY_FAVICON_HEIGHT = 2;
const int SELECT_FULL_HISTORY_TITLE = 5;
const int SELECT_FULL_HISTORY_VISIT_DATE = 6;
const int SELECT_FULL_HISTORY_VISIT_COUNTER = 7;
const int SELECT_FULL_HISTORY_URI_FK = 8;

const std::string SELECT_HISTORY_BY_URL =
    SELECT_FULL_HISTORY_ALL_ROW
    + " where " + COL_HISTORY_URL + "=?";

}


namespace tizen_browser
{
namespace services
{

HistorySQLite::HistorySQLite()
    : m_dbHistoryInitialised(false)
    , m_histItems()
    , m_isInitialized(false)
{
}


HistorySQLite::~HistorySQLite()
{
}

void HistorySQLite::init(bool testmode)
{
    if (m_isInitialized) {
        return;
    }

    config.load("whatever");

    std::string resourceDbDir;
    std::string dbHistory;

    if (!testmode) {
        resourceDbDir = boost::any_cast < std::string > (config.get("resourcedb/dir"));
        dbHistory = boost::any_cast < std::string > (config.get("DB_HISTORY"));
    } else {
        resourceDbDir = boost::any_cast < std::string > (config.get("resourcedb/dir"));
        dbHistory = "history_test.db";
    }

    DB_HISTORY = resourceDbDir + dbHistory;

    try {
        initDatabaseHistory(DB_HISTORY);
        m_isInitialized = true;
    } catch (storage::StorageExceptionInitialization & e) {
        BROWSER_LOGE("Cannot initialize database: %s!", DB_HISTORY.c_str());
    }

}

std::string message(const std::string & text, int errcode, const std::string & errMsg)
{
    return text + std::to_string(errcode) + "; " + errMsg;
}


//void History::addHistoryItem(
//    const std::string & url,
//    const std::string & title,
//    std::shared_ptr<tizen_browser::tools::BrowserImage> image,
//    tizen_browser::tools::BrowserImage::ImageType outputImageType)
//{
//}

void HistorySQLite::addHistoryItem(std::shared_ptr<HistoryItem> hi)
{
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_HISTORY));
    addHistoryItem(hi, &scope);
}

/**
 * @throws HistoryException on error
 */
void HistorySQLite::addHistoryItem(std::shared_ptr<HistoryItem> hi, storage::SQLTransactionScope * transactionScope)
{
    BROWSER_LOGI("History::addHistoryItem begin");

    std::unique_ptr<tizen_browser::tools::Blob> blob;
    if (hi->getFavIcon()) {
        if (tizen_browser::tools::BrowserImage::ImageTypePNG == hi->getFavIcon()->imageType) {
            blob = tizen_browser::tools::EflTools::getBlobPNG(hi->getFavIcon());
        } else {
            std::vector < uint8_t > rawImageData = tizen_browser::tools::EflTools::rawEvasImageData(hi->getFavIcon());
            blob = std::unique_ptr<tizen_browser::tools::Blob>(new tizen_browser::tools::Blob(rawImageData.data(), rawImageData.size()));
        }
    }
    std::shared_ptr<storage::SQLDatabase> conIns = transactionScope->database();
    try {
        storage::SQLQuery select(conIns->prepare(SELECT_FAVICON_EXISTS));
        select.bindText(1, hi->getUrl());
        select.exec();

        if (!select.hasNext() && blob) {
            BROWSER_LOGI("Favicon URI=%s - not found. Adding to database...", hi->getUriFavicon().c_str());
            storage::SQLQuery insertFavIcon(conIns->prepare(INSERT_TABLE_FAVICON));
            insertFavIcon.bindText(1, hi->getUrl());
            insertFavIcon.bindInt(2, hi->getFavIcon()->width);
            insertFavIcon.bindInt(3, hi->getFavIcon()->height);
            insertFavIcon.bindInt(4, hi->getFavIcon()->imageType);
            insertFavIcon.bindBlob(5, std::move(blob));
            insertFavIcon.exec();
            BROWSER_LOGI("Favicon added");
            BROWSER_LOGI("History item: url: %s", hi->getUriFavicon().c_str());
        } else {
            BROWSER_LOGI("Favicon URI=%s was found.", hi->getUriFavicon().c_str());
        }

        BROWSER_LOGI("A new record will be added to a table %s", TABLE_HISTORY.c_str());

        storage::SQLQuery insertHist(conIns->prepare(INSERT_TABLE_HISTORY_DEFAULT_VALUES));
        insertHist.bindText(1, hi->getUrl());
        insertHist.bindText(2, hi->getTitle());
        insertHist.bindText(3, hi->getUrl());
        insertHist.exec();

        BROWSER_LOGI("History::addHistoryItem - commited");
    } catch (storage::StorageException & e) {
        BROWSER_LOGE("Cannot add a history item (url='%s', title='%s'): %s",
                     hi->getUrl().c_str(), hi->getTitle().c_str(), e.getMessage());

        std::string msg = message("SQLite error (code = ", e.getErrorCode(), e.getMessage());

        throw HistoryException(msg);
    }

    BROWSER_LOGI("History::addHistoryItem end");
}


/**
 * If hi->getUrl() exists on a table HISTORY update visit_counter and visit_date, unless insert hi to database.
 *
 * @throws HistoryException on error
 */

void HistorySQLite::insertOrRefresh(std::shared_ptr<HistoryItem> hi)
{
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_HISTORY));
    insertOrRefresh(hi, &scope);
}

void HistorySQLite::insertOrRefresh(std::shared_ptr<HistoryItem> hi, storage::SQLTransactionScope * transactionScope)
{
    BROWSER_LOGI("History::insertOrRefresh begin");


    M_ASSERT(hi);

    std::shared_ptr<storage::SQLDatabase> con = transactionScope->database();

    bool isUpdate = false;

    try {
        storage::SQLQuery select(con->prepare(SELECT_HISTORY_VISIT_COUNTER));
        select.bindText(1, hi->getUrl());
        select.exec();

        if (select.hasNext()) {
            storage::SQLQuery update(con->prepare(UPDATE_HISTORY_ITEM_COUNTER_DATE));
            update.bindText(1, hi->getUrl());
            update.exec();
            isUpdate = true;
            BROWSER_LOGI("visit_counter and visit_date has been updated");

        }

        BROWSER_LOGI("[%s] - commited", __func__);
    } catch (storage::StorageException & e) {
        std::string msg = message("SQLite error (code = ", e.getErrorCode(), e.getMessage());

        BROWSER_LOGE("%s - exception: %s", __PRETTY_FUNCTION__, msg.c_str());

        throw HistoryException(msg);
    }

    if (!isUpdate) {
        addHistoryItem(hi, transactionScope);
        BROWSER_LOGI("New HistoryItem object has been added.");

    }

    BROWSER_LOGI("History::insertOrRefresh end");
}

/**
 * ... or just (better) use getHistoryItem method
 * @throws HistoryException on error
 */
std::shared_ptr<tizen_browser::tools::BrowserImage> HistorySQLite::getFavicon(const std::string & url)
{
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_HISTORY));
    return getFavicon(url, &scope);
}

std::shared_ptr<tizen_browser::tools::BrowserImage> HistorySQLite::getFavicon(const std::string & url, storage::SQLTransactionScope * transactionScope)
{
    BROWSER_LOGI("History::getFavicon begin");

    std::shared_ptr<tizen_browser::tools::Blob> rsBlob = std::make_shared<tizen_browser::tools::Blob>();
    std::shared_ptr<tizen_browser::tools::BrowserImage> bi
                = std::make_shared<tizen_browser::tools::BrowserImage>();

    auto imageType = tizen_browser::tools::BrowserImage::ImageTypeNoImage;

    auto con = transactionScope->database();

    try {
        storage::SQLQuery select(con->prepare(SELECT_FAVICON));
        select.bindText(1, url);
        select.exec();

        if (select.hasNext()) {
            rsBlob = select.getBlob(SELECT_FAVICON_FAVICON);
            if (rsBlob.get()) {
                bi->width = select.getInt(SELECT_FAVICON_WIDTH);
                bi->height = select.getInt(SELECT_FAVICON_HEIGHT);
                const auto autoimageType = select.getInt(SELECT_FAVICON_IMAGE_TYPE);
                bi->imageType = static_cast<tizen_browser::tools::BrowserImage::ImageType>(autoimageType);
                if(rsBlob->getLength() > 0 ){
                    bi->dataSize = rsBlob->transferData(&bi->imageData);
                } else {
                    bi->dataSize = 0;
                    bi->imageData = 0;
                }
                BROWSER_LOGI("History::getFavicon - commited");
            } else {
                BROWSER_LOGW("History::getFavicon - empty blob");
            }
        } else {
            BROWSER_LOGW("Can't find a favicon (url=%s)", url.c_str());
        }

        BROWSER_LOGI("History::getFavicon - commited");
    } catch (storage::StorageException & e) {
        std::string msg = message("SQLite error (code = ", e.getErrorCode(), e.getMessage());

        BROWSER_LOGE("%s - exception: %s", __PRETTY_FUNCTION__, msg.c_str());

        throw HistoryException(msg.c_str());
    }

    BROWSER_LOGI("History::getFavicon end");

    return bi;
}

/**
 * @throws HistoryException on error
 */
void HistorySQLite::deleteHistory()
{
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_HISTORY));
    deleteHistory(&scope);
}

void HistorySQLite::deleteHistory(storage::SQLTransactionScope * transactionScope)
{
    BROWSER_LOGI("History::deleteHistory begin");

    std::shared_ptr<storage::SQLDatabase> con = transactionScope->database();

    try {
        con->exec(DELETE_ALL_HISTORY);
        con->exec(DELETE_ALL_FAVICON);

        BROWSER_LOGI("History::deleteHistory - commited");
    } catch (storage::StorageException & e) {
        std::string msg = message("SQLite error (code = ", e.getErrorCode(), e.getMessage());

        throw HistoryException(msg.c_str());
    }
    m_histItems.clear();

    BROWSER_LOGI("History::deleteHistory end");
}

/**
 * @throws HistoryException on error
 */
void HistorySQLite::deleteHistory(const std::string & url)
{
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_HISTORY));
    deleteHistory(url, &scope);
}

void HistorySQLite::deleteHistory(const std::string & url, storage::SQLTransactionScope * transactionScope)
{
    BROWSER_LOGI("History::deleteHistory (url: %s) - begin", url.c_str());

    std::shared_ptr<storage::SQLDatabase> con = transactionScope->database();

    try {
        storage::SQLQuery deleteQuery(con->prepare(DELETE_HISTORY_ITEM));

        deleteQuery.bindText(1, url);
        deleteQuery.exec();

        // consider delete a corresponding record from a table FAVICON if dangling

        BROWSER_LOGI("History::deleteHistory (url: %s) - commited", url.c_str());
    } catch (storage::StorageException & e) {
        std::string msg = message("SQLite error (code = ", e.getErrorCode(), e.getMessage());

        BROWSER_LOGE("%s - exception: %s", __PRETTY_FUNCTION__, msg.c_str());

        throw HistoryException(msg.c_str());
    }

    for (HistoryItemVector::iterator it(m_histItems.begin()), end(m_histItems.end()); it != end; ++it) {
        if ((*it)->getUrl() == url) {
            m_histItems.erase(it);
            break;
        }
    }

    BROWSER_LOGI("History::deleteHistory (url: %s) - end", url.c_str());
}

/**
 * @throws StorageException on error
 */
/*private*/ std::shared_ptr<HistoryItem> HistorySQLite::createHistoryItem(const storage::SQLQuery & query)
{
    BROWSER_LOGI("History::createHistoryItem begin");

    std::shared_ptr<HistoryItem> hi;

    try {
        int len = 0;
        std::shared_ptr<tizen_browser::tools::Blob> rsBlob = std::make_shared<tizen_browser::tools::Blob>();
        auto imageType = tizen_browser::tools::BrowserImage::ImageTypeNoImage;

        std::shared_ptr<tizen_browser::tools::BrowserImage> bi = std::make_shared<tizen_browser::tools::BrowserImage>();

        std::string url = query.getString(SELECT_FULL_HISTORY_ALL_ROW_URL);

        hi = std::make_shared<HistoryItem>(url);
        hi->setUrl(url);
        hi->setTitle(query.getString(SELECT_FULL_HISTORY_TITLE));

        std::string strVisitDate = query.getString(SELECT_FULL_HISTORY_VISIT_DATE);
        BROWSER_LOGD("[%s:%d] VD!!!! %s", __PRETTY_FUNCTION__, __LINE__, strVisitDate.c_str());
        boost::posix_time::ptime visitDate = boost::posix_time::time_from_string(strVisitDate);

        BROWSER_LOGD("%s:%d\n", __FILE__, __LINE__);

        hi->setLastVisit(visitDate);
        hi->setVisitCounter(query.getInt(SELECT_FULL_HISTORY_VISIT_COUNTER));
        hi->setUriFavicon(query.getString(SELECT_FULL_HISTORY_URI_FK));

        rsBlob = query.getBlob(SELECT_FULL_HISTORY_FAVICON);
        if (rsBlob.get()) {
            const auto autoimageType = query.getInt(SELECT_FULL_HISTORY_FAVICON_IMAGE_TYPE);
            imageType = static_cast<tizen_browser::tools::BrowserImage::ImageType>(autoimageType);

            bi->imageType = imageType;
            bi->width = query.getInt(SELECT_FULL_HISTORY_FAVICON_WIDTH);
            bi->height = query.getInt(SELECT_FULL_HISTORY_FAVICON_HEIGHT);
            bi->url = url;

            len = rsBlob->getLength();
            BROWSER_LOGI("History::createHistoryItem. I've got an icon. Length = %d, url = %s", len, url.c_str());
        }

        if (len > 0) {
            bi->dataSize = rsBlob->transferData(&bi->imageData);
        } else {
            bi = std::make_shared<tizen_browser::tools::BrowserImage>();
            bi->imageData = NULL;
            bi->dataSize = 0;
        }

        if (bi.get()) {
            hi->setFavIcon(bi);
        }

        BROWSER_LOGI("Favicon uri: %s, visit_date: %s, title: %s", hi->getUriFavicon().c_str(), strVisitDate.c_str(), query.getString(SELECT_FULL_HISTORY_TITLE).c_str());
    } catch (storage::StorageException & e) {
        std::string msg = message("SQLite error (code = ", e.getErrorCode(), e.getMessage());

        BROWSER_LOGE("%s - exception: %s", __PRETTY_FUNCTION__, msg.c_str());

        throw;
    }

    BROWSER_LOGI("History::createHistoryItem end");

    return hi;
}

/**
 * @throws HistoryException on error
 */
std::shared_ptr<HistoryItem> HistorySQLite::getHistoryItem(const std::string & url)
{
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_HISTORY));
    return getHistoryItem(url, &scope);
}

std::shared_ptr<HistoryItem> HistorySQLite::getHistoryItem(const std::string & url, storage::SQLTransactionScope * transactionScope)
{
    BROWSER_LOGI("History::getHistoryItem begin");

    std::shared_ptr<HistoryItem> hi;

    std::shared_ptr<storage::SQLDatabase> con = transactionScope->database();

    try {
        storage::SQLQuery select(con->prepare(SELECT_HISTORY_BY_URL));
        select.bindText(1, url);
        select.exec();

        if (select.hasNext()) {
            hi = createHistoryItem(select);
        } else {
            BROWSER_LOGI("No records found for url %s.\n%s\n", url.c_str(), SELECT_HISTORY_BY_URL.c_str());
            hi = std::shared_ptr<HistoryItem>(new HistoryItem(url));
        }

        BROWSER_LOGI("History::getHistoryItem - commited");
    } catch (storage::StorageException & e) {
        std::string msg = message("SQLite error (code = ", e.getErrorCode(), e.getMessage());

        BROWSER_LOGE("%s - exception: %s", __PRETTY_FUNCTION__, msg.c_str());

        throw HistoryException(msg.c_str());
    }

    BROWSER_LOGI("History::getHistoryItem end");
    return hi;
}

/**
 * @throws HistoryException on error
 */
HistoryItemVector & HistorySQLite::getHistoryItems(int historyDepthInDays, int maxItems)
{
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_HISTORY));
    return getHistoryItems(&scope, historyDepthInDays, maxItems);
}

HistoryItemVector & HistorySQLite::getHistoryItems(storage::SQLTransactionScope * transactionScope, int historyDepthInDays, int maxItems)
{
    BROWSER_LOGI("History::getHistoryItems begin");
    m_histItems.clear();
    std::shared_ptr<storage::SQLDatabase> con = transactionScope->database();

    const std::string historyNotOlderThan = COL_HISTORY_VISIT_DATE + ">=date('now','-" + std::to_string(historyDepthInDays) + " day')";
    const std::string historyItemsLimit = std::to_string(maxItems);

    const std::string SQL = SELECT_FULL_HISTORY_ALL_ROW +
                            " and " + historyNotOlderThan +
                            " order by " + COL_HISTORY_VISIT_DATE +
                            " desc limit " + historyItemsLimit;

    try {
        storage::SQLQuery select(con->prepare(SQL));
        select.exec();

        while (select.hasNext()) {
            std::shared_ptr<HistoryItem> hi = createHistoryItem(select);
            m_histItems.push_back(hi);
            select.next();

        }
        BROWSER_LOGI("History::getHistoryItems - commited");
    } catch (storage::StorageException & e) {
        std::string msg = message("SQLite error (code = ", e.getErrorCode(), e.getMessage());

        BROWSER_LOGE("%s - exception: %s", __PRETTY_FUNCTION__, msg.c_str());

        throw HistoryException(msg.c_str());
    }

    BROWSER_LOGI("History::getHistoryItems end");
    return m_histItems;
}

/**
 * @throws HistoryException on error
 */
int HistorySQLite::getHistoryItemsCount()
{
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_HISTORY));
    return getHistoryItemsCount(&scope);
}

int HistorySQLite::getHistoryItemsCount(storage::SQLTransactionScope * transactionScope)
{
    BROWSER_LOGI("History::getHistoryItemsAmount begin");

    int res = 0;

    std::shared_ptr<storage::SQLDatabase> con = transactionScope->database();

    try {
        storage::SQLQuery select(con->prepare(SELECT_HISTORY_COUNT_ID));
        select.exec();

        bool isRecord = select.hasNext();

        (void)isRecord;
        M_ASSERT(isRecord);

        res = select.getInt(0);

        BROWSER_LOGI("History::getHistoryItemsAmount - commited");
    } catch (storage::StorageException & e) {
        std::string msg = message("SQLite error (code = ", e.getErrorCode(), e.getMessage());

        BROWSER_LOGE("%s - exception: %s", __PRETTY_FUNCTION__, msg.c_str());

        throw HistoryException(msg.c_str());
    }

    BROWSER_LOGI("History::getHistoryItemsAmount end");

    return res;
}

/**
 * @throws HistoryException on error
 */
int HistorySQLite::getHistoryVisitCounter(const std::string & url)
{
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(DB_HISTORY));
    return getHistoryVisitCounter(url, &scope);
}

int HistorySQLite::getHistoryVisitCounter(const std::string & url, storage::SQLTransactionScope * transactionScope)
{
    BROWSER_LOGI("History::getHistoryVisitCounter begin");

    int res = 0;

    std::shared_ptr<storage::SQLDatabase> con = transactionScope->database();

    try {
        storage::SQLQuery select(con->prepare(SELECT_HISTORY_VISIT_COUNTER));
        select.bindText(1, url);
        select.exec();

        if (select.hasNext()) {
            res = select.getInt(0);
        } else {
            res = -1; // no record was found
        }

        BROWSER_LOGI("History::getHistoryVisitCounter - commited");
    } catch (storage::StorageException & e) {
        std::string msg = message("SQLite error (code = ", e.getErrorCode(), e.getMessage());

        BROWSER_LOGE("%s - exception: %s", __PRETTY_FUNCTION__, msg.c_str());

        throw HistoryException(msg.c_str());
    }

    BROWSER_LOGI("History::getHistoryVisitCounter end");

    return res;
}

/**
 * @throws StorageExceptionInitialization on error
 */
void HistorySQLite::initDatabaseHistory(const std::string & db_str)
{
    BROWSER_LOGI("History::initDatabaseHistory begin");

    if (!m_dbHistoryInitialised) {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(db_str));

        try {
            StorageService::checkAndCreateTable(&scope, TABLE_FAVICON, DDL_CREATE_TABLE_FAVICON);
            StorageService::checkAndCreateTable(&scope, TABLE_HISTORY, DDL_CREATE_TABLE_HISTORY);

            BROWSER_LOGI("History::initDatabaseHistory - done");
        } catch (storage::StorageException & e) {
            BROWSER_LOGE("[ERROR] History::initDatabaseHistory (error code - %d): %s", e.getErrorCode(), e.getMessage());
            throw storage::StorageExceptionInitialization(e.getMessage(),
                                                          e.getErrorCode());
        }

        m_dbHistoryInitialised = true;
    }

    M_ASSERT(m_dbHistoryInitialised);

    BROWSER_LOGI("History::initDatabaseHistory end");
}

}
}

