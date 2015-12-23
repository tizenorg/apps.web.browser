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
#include "HistoryDaysListManagerMob.h"
#include "HistoryDayItemData.h"
#include "mob/HistoryDayItemMob.h"

#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

HistoryDaysListManagerMob::HistoryDaysListManagerMob()
{
    m_daysListEdjFilePath = EDJE_DIR;
    m_daysListEdjFilePath.append("HistoryUI/HistoryDaysList.edj");
    elm_theme_extension_add(nullptr, m_daysListEdjFilePath.c_str());
}

HistoryDaysListManagerMob::~HistoryDaysListManagerMob()
{
    clear();
    elm_box_clear(m_boxMain);
    evas_object_del(m_boxMain);
    evas_object_del(m_scrollerMain);
    evas_object_del(m_layoutScroller);
}

Evas_Object* HistoryDaysListManagerMob::createDaysList(
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

void HistoryDaysListManagerMob::addHistoryItems(
        const std::map<std::string, services::HistoryItemVector>& items,
        HistoryPeriod period)
{
    std::vector<WebsiteHistoryItemDataPtr> historyItems;
    for (auto& itemPair : items) {
        std::vector<WebsiteVisitItemDataPtr> pageViewItems;
        for(auto& hi : itemPair.second) {
            pageViewItems.push_back(std::make_shared<WebsiteVisitItemData>(hi));
        }
        historyItems.push_back(std::make_shared<WebsiteHistoryItemData>(
                "title", itemPair.first, pageViewItems));
    }
    HistoryDayItemDataPtr dayItem = std::make_shared<HistoryDayItemData>(
            toString(period), historyItems);
    appendDayItem(dayItem);
}

void HistoryDaysListManagerMob::clear()
{
    elm_box_clear(m_boxMain);
    m_dayItems.clear();
}

void HistoryDaysListManagerMob::appendDayItem(HistoryDayItemDataPtr dayItemData)
{
    auto item = std::make_shared<HistoryDayItemMob>(dayItemData);
    m_dayItems.push_back(item);

    Evas_Object* boxDay = item->init(m_boxMain);
    elm_box_pack_end(m_boxMain, boxDay);
}

} /* namespace base_ui */
} /* namespace tizen_browser */
