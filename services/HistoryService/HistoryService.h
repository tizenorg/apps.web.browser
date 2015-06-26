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

#ifndef __HISTORY_SERVICE_H
#define __HISTORY_SERVICE_H

#include <vector>
#include <memory>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/signals2/signal.hpp>

#include "ServiceFactory.h"
#include "service_macros.h"
#include "BrowserImage.h"
#include "HistoryItem.h"
#include "HistoryStorage.h"
#include "StorageService.h"
#include <web/web_history.h>
#define DOMAIN_HISTORY_SERVICE "org.tizen.browser.historyservice"

namespace tizen_browser
{
namespace services
{

class BROWSER_EXPORT HistoryService: public tizen_browser::core::AbstractService
{
public:
    HistoryService();
    virtual ~HistoryService();
    virtual std::string getName();

    int getHistoryId(const std::string & url);
    /**
     * @throws HistoryException on error
     */
    void addHistoryItem(std::shared_ptr<HistoryItem> hi);

    /**
     * If hi->getUrl() exists on a table HISTORY update visit_counter and visit_date, unless insert hi to database.
     *
     * @throws HistoryException on error
     */
    void insertOrRefresh(std::shared_ptr<HistoryItem> hi);

    /**
     * @throws HistoryException on error
     */
    void clearAllHistory();

    /**
     * @throws HistoryException on error
     */
    void clearURLHistory(const std::string & url);

    /**
     * @throws HistoryException on error
     */
    std::shared_ptr<HistoryItem> getHistoryItem(const std::string & url);

    /**
     * @throws HistoryException on error
     */
    HistoryItemVector & getHistoryItems(int historyDepthInDays = 7, int maxItems = 50);

    /**
     * @throws HistoryException on error
     */
    int getHistoryItemsCount();

    /**
     * @throws HistoryException on error
     */
    int getHistoryVisitCounter(const std::string & url);

    void setStorageServiceTestMode(bool testmode = true);

    boost::signals2::signal<void (bool)>historyEmpty;
private:
    bool m_testDbMod;;
    std::vector<std::shared_ptr<HistoryItem>> history_list;
    std::shared_ptr<tizen_browser::services::StorageService> m_storageManager;

    /**
     * @throws StorageExceptionInitialization on error
     */
    void initDatabaseBookmark(const std::string & db_str);

    std::shared_ptr<tizen_browser::services::StorageService> getStorageManager();

};

}
}

#endif
