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

#include <string>
#include <BrowserAssert.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "ServiceManager.h"
#include "HistoryService.h"
#include "AbstractWebEngine.h"

namespace tizen_browser
{
namespace services
{

EXPORT_SERVICE(HistoryService, DOMAIN_HISTORY_SERVICE)

HistoryService::HistoryService() : m_testDbMod(false)
{
    BROWSER_LOGD("HistoryService");
}

HistoryService::~HistoryService()
{
}

std::shared_ptr<tizen_browser::services::StorageService> HistoryService::getStorageManager()
{
    if (!m_storageManager) {
        m_storageManager = std::dynamic_pointer_cast <
                           tizen_browser::services::StorageService,
                           tizen_browser::core::AbstractService > (
                               tizen_browser::core::ServiceManager::getInstance().getService(
                                   DOMAIN_STORAGE_SERVICE));
    }

    M_ASSERT(m_storageManager);
    m_storageManager->init(m_testDbMod);

    return m_storageManager;
}

void HistoryService::setStorageServiceTestMode(bool testmode) {
	m_testDbMod = testmode;
}

/**
 * @throws HistoryException on error
 */
void HistoryService::addHistoryItem(std::shared_ptr<HistoryItem> hi)
{
    getStorageManager()->addHistoryItem(hi);
}

/**
 * If hi->getUrl() exists on a table HISTORY update visit_counter and visit_date, unless insert hi to database.
 *
 * @throws HistoryException on error
 */
void HistoryService::insertOrRefresh(std::shared_ptr<HistoryItem> hi) {
	getStorageManager()->insertOrRefresh(hi);
}

/**
 * @throws HistoryException on error
 */
void HistoryService::clearAllHistory()
{
    getStorageManager()->deleteHistory();
}

/**
 * @throws HistoryException on error
 */
void HistoryService::clearURLHistory(const std::string & url)
{
    getStorageManager()->deleteHistory(url);
    if(0 == getHistoryItemsCount()){
	historyEmpty(true);
    }
}

/**
 * @throws HistoryException on error
 */
std::shared_ptr<HistoryItem> HistoryService::getHistoryItem(const std::string & url)
{
    return getStorageManager()->getHistoryItem(url);
}

/**
 * @throws HistoryException on error
 */
HistoryItemVector& HistoryService::getHistoryItems(int historyDepthInDays, int maxItems)
{
    return getStorageManager()->getHistoryItems(historyDepthInDays, maxItems);
}

/**
 * @throws HistoryException on error
 */
int HistoryService::getHistoryItemsCount()
{
    return getStorageManager()->getHistoryItemsCount();
}

/**
 * @throws HistoryException on error
 */
int HistoryService::getHistoryVisitCounter(const std::string & url)
{
    return getStorageManager()->getHistoryVisitCounter(url);
}

}
}
