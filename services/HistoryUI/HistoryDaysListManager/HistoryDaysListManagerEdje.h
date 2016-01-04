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

#ifndef HISTORYDAYSLISTMANAGEREDJETV_H_
#define HISTORYDAYSLISTMANAGEREDJETV_H_

#include <string>
#include <memory>

namespace tizen_browser {
namespace base_ui {

typedef struct HistoryDaysListManagerEdje_ {
    HistoryDaysListManagerEdje_()
    : historyDaysList(EDJE_DIR + std::string("HistoryUI/HistoryDaysList.edj"))
    , websiteHistoryItem(EDJE_DIR + std::string("HistoryUI/WebsiteHistoryItem.edj"))
    , websiteHistoryItemTitle(EDJE_DIR + std::string("HistoryUI/WebsiteHistoryItemTitle.edj"))
    , websiteHistoryItemVisitItems(EDJE_DIR + std::string("HistoryUI/WebsiteHistoryItemVisitItems.edj"))
    {}
    const std::string historyDaysList;
    const std::string websiteHistoryItem;
    const std::string websiteHistoryItemTitle;
    const std::string websiteHistoryItemVisitItems;
} HistoryDaysListManagerEdje;

typedef std::shared_ptr<const HistoryDaysListManagerEdje> HistoryDaysListManagerEdjePtr;

}
}

#endif /* HISTORYDAYSLISTMANAGEREDJETV_H_ */
