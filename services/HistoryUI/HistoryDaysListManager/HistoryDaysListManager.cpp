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

#include <services/HistoryUI/HistoryUI.h>
#include <services/HistoryService/HistoryItem.h>
#include "BrowserLogger.h"
#include "HistoryDaysListManager.h"
#include "HistoryDayItemData.h"
#include "HistoryDayItem.h"

#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

HistoryDaysListManager::HistoryDaysListManager()
{
    m_daysListEdjFilePath = EDJE_DIR;
    m_daysListEdjFilePath.append("HistoryUI/HistoryDaysList.edj");
    elm_theme_extension_add(nullptr, m_daysListEdjFilePath.c_str());
}

HistoryDaysListManager::~HistoryDaysListManager()
{
    clear();
    elm_box_clear(m_boxMain);
    evas_object_del(m_boxMain);
    evas_object_del(m_scrollerMain);
    evas_object_del(m_layoutScroller);
}

Evas_Object* HistoryDaysListManager::createDayGenlist(
        Evas_Object* parentLayout)
{
    m_scrollerMain = elm_scroller_add(parentLayout);
    elm_scroller_policy_set(m_scrollerMain, ELM_SCROLLER_POLICY_OFF,
            ELM_SCROLLER_POLICY_OFF);
    elm_scroller_bounce_set(m_scrollerMain, EINA_FALSE, EINA_FALSE);
    tools::EflTools::setExpandHints(m_scrollerMain);

    m_layoutScroller = elm_layout_add(parentLayout);
    tools::EflTools::setExpandHints(m_layoutScroller);
    elm_layout_file_set(m_layoutScroller, m_daysListEdjFilePath.c_str(), "historyDaysList");

    elm_object_content_set(m_scrollerMain, m_layoutScroller);

    m_boxMain = elm_box_add(m_layoutScroller);
    evas_object_box_align_set(m_boxMain, 0.0, 0.0);
    tools::EflTools::setExpandHints(m_boxMain);
    elm_object_part_content_set(m_layoutScroller, "main_box", m_boxMain);

    return m_scrollerMain;
}

void HistoryDaysListManager::addHistoryItems(
        const std::map<std::string, services::HistoryItemVector>& items,
        HistoryPeriod period)
{
    std::vector<WebsiteHistoryItemDataPtr> historyItems;
    for (auto& itemPair : items) {
        std::vector<WebsiteVisitItemDataPtr> pageViewItems;
        for(auto& hi : itemPair.second) {
            pageViewItems.push_back(std::make_shared<WebsiteVisitItemData>(
                    hi->getTitle(), hi->getUrl(), "00:00 AM"));
        }
        historyItems.push_back(std::make_shared<WebsiteHistoryItemData>(itemPair.first, pageViewItems));
    }
    HistoryDayItemDataPtr dayItem = std::make_shared <HistoryDayItemData> (
            toString(period), historyItems);
    appendDayItem(dayItem);
}

void HistoryDaysListManager::clear()
{
    elm_box_clear(m_boxMain);
    m_dayItems.clear();
}

void HistoryDaysListManager::appendDayItem(HistoryDayItemDataPtr dayItemData)
{
    auto item = std::make_shared<HistoryDayItem>(dayItemData);
    m_dayItems.push_back(item);

    Evas_Object* boxDay = item->init(m_boxMain);
    elm_box_pack_end(m_boxMain, boxDay);
}

std::string HistoryDaysListManager::toString(HistoryPeriod period)
{
    switch (period) {
    case HistoryPeriod::HISTORY_TODAY:
        return "Today";
    case HistoryPeriod::HISTORY_YESTERDAY:
        return "Yesterday";
    case HistoryPeriod::HISTORY_LASTWEEK:
        return "Last Week";
    case HistoryPeriod::HISTORY_LASTMONTH:
        return "Last Month";
    default:
        BROWSER_LOGE("[%s:%d]not handled period ",
                __PRETTY_FUNCTION__, __LINE__);
        return "";
    }
}

} /* namespace base_ui */
} /* namespace tizen_browser */
