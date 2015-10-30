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

#ifndef __HISTORY_H
#define __HISTORY_H

#include <vector>
#include <memory>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "BrowserImage.h"
#include "Config.h"
#include "HistoryStorage.h"
#include "SQLDatabase.h"

///\todo temporary dirty include
#include "../HistoryService/HistoryItem.h"

namespace tizen_browser
{
namespace services
{

class HistorySQLite: virtual public HistoryStorage
{
public:
    HistorySQLite();
    ~HistorySQLite();

    /**
     * @throws HistoryException on error
     */
    void addHistoryItem(std::shared_ptr<HistoryItem> hi, storage::SQLTransactionScope * transactionScope);

    void addHistoryItem(std::shared_ptr<HistoryItem> hi);

    /**
     * If hi->getUrl() exists on a table HISTORY update visit_counter and visit_date, unless insert hi to database.
     *
     * @throws HistoryException on error
     */
    void insertOrRefresh(std::shared_ptr<HistoryItem> hi, storage::SQLTransactionScope * transactionScope);

    void insertOrRefresh(std::shared_ptr<HistoryItem> hi);

    /**
     * @throws HistoryException on error
     */
    std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon(const std::string & url, storage::SQLTransactionScope * transactionScope);

    std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon(const std::string & url);

    /**
     * @throws HistoryException on error
     */
    void deleteHistory(storage::SQLTransactionScope * transactionScope);

    void deleteHistory();

    /**
     * @throws HistoryException on error
     */
    void deleteHistory(const std::string & url, storage::SQLTransactionScope * transactionScope);

    void deleteHistory(const std::string & url);

    /**
     * @throws HistoryException on error
     */
    std::shared_ptr<HistoryItem> getHistoryItem(const std::string & url, storage::SQLTransactionScope * transactionScope);

    std::shared_ptr<HistoryItem> getHistoryItem(const std::string & url);

    /**
     * @throws HistoryException on error
     */
    HistoryItemVector & getHistoryItems(storage::SQLTransactionScope * transactionScope, int historyDepthInDays, int maxItems);

    HistoryItemVector & getHistoryItems(int historyDepthInDays, int maxItems);

    /**
     * @throws HistoryException on error
     */
    int getHistoryItemsCount(storage::SQLTransactionScope * transactionScope);

    int getHistoryItemsCount();

    /**
     * @throws HistoryException on error
     */
    int getHistoryVisitCounter(const std::string & url, storage::SQLTransactionScope * transactionScope);

    int getHistoryVisitCounter(const std::string & url);

    void init(bool testmode = false);

private:
    config::DefaultConfig config;
    std::string DB_HISTORY;
    bool m_dbHistoryInitialised;

    HistoryItemVector m_histItems;
    /**
     * @throws StorageExceptionInitialization on error
     */
    void initDatabaseHistory(const std::string & db_str);

    /**
     * @throws StorageException on error
     */
    std::shared_ptr<HistoryItem> createHistoryItem(const storage::SQLQuery & query);

    bool m_isInitialized;

};

}
}

#endif
