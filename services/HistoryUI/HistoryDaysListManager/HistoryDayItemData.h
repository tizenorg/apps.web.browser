/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#ifndef HISTORYDAYITEMDATA_H_
#define HISTORYDAYITEMDATA_H_

#include <string>
#include <vector>
#include "HistoryDayItemDataTypedef.h"
#include <services/HistoryService/HistoryItem.h>

namespace tizen_browser {
namespace base_ui {

typedef struct WebsiteVisitItemData_
{
    WebsiteVisitItemData_(std::shared_ptr<services::HistoryItem> historyItem)
    : historyItem(historyItem)
    {
    }
    std::shared_ptr<const services::HistoryItem> historyItem;
} WebsiteVisitItemData;

typedef struct WebsiteHistoryItemData_
{
    WebsiteHistoryItemData_(const std::string& websiteTitle,
            const std::string& websiteDomain,
            const std::vector<WebsiteVisitItemDataPtr>& list) :
            websiteTitle(websiteTitle), websiteDomain(websiteDomain),
            websiteVisitItems(list)
    {
    }
    const std::string websiteTitle;
    const std::string websiteDomain;
    const std::vector<WebsiteVisitItemDataPtr> websiteVisitItems;
} WebsiteHistoryItemData;

typedef struct HistoryDayItemData_
{
    HistoryDayItemData_(const std::string& day,
            const std::vector<WebsiteHistoryItemDataPtr>& list) :
            day(day), websiteHistoryItems(list)
    {
    }
    const std::string day;
    const std::vector<WebsiteHistoryItemDataPtr> websiteHistoryItems;
} HistoryDayItemData;

}
}

#endif /* HISTORYDAYITEMDATA_H_ */
