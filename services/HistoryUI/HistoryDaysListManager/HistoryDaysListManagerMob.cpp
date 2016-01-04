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
    : m_edjeFiles(std::make_shared<HistoryDaysListManagerEdje>())
    , m_parent(nullptr)
    , m_scrollerDays(nullptr)
    , m_layoutScrollerDays(nullptr)
    , m_boxDays(nullptr)
{
}

HistoryDaysListManagerMob::~HistoryDaysListManagerMob()
{
    for(auto& dayItem : m_dayItems) {
        dayItem->setEflObjectsAsDeleted();
    }
}

Evas_Object* HistoryDaysListManagerMob::createDaysList(
        Evas_Object* parent)
{
    m_parent = parent;
    m_scrollerDays = elm_scroller_add(parent);
    tools::EflTools::setExpandHints(m_scrollerDays);

    m_layoutScrollerDays = elm_layout_add(parent);
    evas_object_size_hint_weight_set(m_layoutScrollerDays, EVAS_HINT_EXPAND,
            0.0);
    evas_object_size_hint_align_set(m_layoutScrollerDays, EVAS_HINT_FILL, 0.0);
    elm_layout_file_set(m_layoutScrollerDays,
            m_edjeFiles->historyDaysList.c_str(), "layoutScrollerDays");

    elm_object_content_set(m_scrollerDays, m_layoutScrollerDays);

    m_boxDays = elm_box_add(m_layoutScrollerDays);
    tools::EflTools::setExpandHints(m_boxDays);
    elm_box_horizontal_set(m_boxDays, EINA_FALSE);
    elm_object_part_content_set(m_layoutScrollerDays, "boxDays",
            m_boxDays);

    return m_scrollerDays;
}

void HistoryDaysListManagerMob::addHistoryItems(
        const std::map<std::string, services::HistoryItemVector>& items,
        HistoryPeriod period)
{
    std::vector<WebsiteHistoryItemDataPtr> historyItems;
    for (auto& itemPair : items) {
        std::vector<WebsiteVisitItemDataPtr> pageViewItems;
        for (auto& hi : itemPair.second)
            pageViewItems.push_back(
                    std::make_shared < WebsiteVisitItemData > (hi));
        historyItems.push_back(
                std::make_shared < WebsiteHistoryItemData
                        > ("Title", itemPair.first, pageViewItems));
    }
    HistoryDayItemDataPtr dayItem = std::make_shared < HistoryDayItemData
            > (toString(period), historyItems);
    appendDayItem(dayItem);
}

void HistoryDaysListManagerMob::clear()
{
    elm_box_clear(m_boxDays);
    m_dayItems.clear();
}

void HistoryDaysListManagerMob::appendDayItem(HistoryDayItemDataPtr dayItemData)
{
    auto item = std::make_shared<HistoryDayItemMob>(dayItemData);
    m_dayItems.push_back(item);

    Evas_Object* dayItemLayout = item->init(m_parent, m_edjeFiles);
    elm_box_pack_end(m_boxDays, dayItemLayout);
}

} /* namespace base_ui */
} /* namespace tizen_browser */
