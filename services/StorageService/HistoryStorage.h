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

#ifndef __HISTORYSTORAGE_H
#define __HISTORYSTORAGE_H

#include <vector>
#include <memory>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "BrowserImage.h"

///\todo temporary dirty include
#include "../HistoryService/HistoryItem.h"

namespace tizen_browser
{
namespace services
{

class HistoryStorage
{
public:
    HistoryStorage();
    virtual ~HistoryStorage();

    /**
     * @throws HistoryException on error
     */
    virtual void addHistoryItem(std::shared_ptr<HistoryItem> hi) = 0;

    /**
     * If hi->getUrl() exists on a table HISTORY update visit_counter and visit_date, unless insert hi to database.
     *
     * @throws HistoryException on error
     */
    virtual void insertOrRefresh(std::shared_ptr<HistoryItem> hi) = 0;

    /**
     * @throws HistoryException on error
     */
    virtual std::shared_ptr<tizen_browser::tools::BrowserImage> getFavicon(const std::string & url) = 0;

    /**
     * @throws HistoryException on error
     */
    virtual void deleteHistory() = 0;

    /**
     * @throws HistoryException on error
     */
    virtual void deleteHistory(const std::string & url) = 0;

    /**
     * @throws HistoryException on error
     */
    virtual std::shared_ptr<HistoryItem> getHistoryItem(const std::string & url) = 0;

    /**
     * @throws HistoryException on error
     */
    virtual HistoryItemVector & getHistoryItems(int historyDepthInDays, int maxItems) = 0;

    /**
     * @throws HistoryException on error
     */
    virtual int getHistoryItemsCount() = 0;

    /**
     * @throws HistoryException on error
     */
    virtual int getHistoryVisitCounter(const std::string & url) = 0;

};

}
}

#endif
