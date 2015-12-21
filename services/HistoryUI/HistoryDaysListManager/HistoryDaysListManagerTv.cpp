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
#include "HistoryDaysListManagerTv.h"
#include "HistoryDayItemData.h"
#include "tv/HistoryDayItemTv.h"
#include "tv/WebsiteHistoryItem/WebsiteHistoryItemTitleTv.h"
#include "tv/WebsiteHistoryItem/WebsiteHistoryItemVisitItemsTv.h"
#include <services/HistoryUI/HistoryDeleteManager.h>

#include <GeneralTools.h>
#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

HistoryDaysListManagerTv::HistoryDaysListManagerTv(HistoryDeleteManagerPtrConst deleteManager)
    : m_edjeFiles(std::make_shared<HistoryDaysListManagerEdjeTv>())
    , m_historyDeleteManager(deleteManager)
{
    connectSignals();
}

HistoryDaysListManagerTv::~HistoryDaysListManagerTv()
{
}

Evas_Object* HistoryDaysListManagerTv::createDaysList(Evas_Object* parent)
{
    m_scrollerDaysColumns = elm_scroller_add(parent);
    elm_scroller_bounce_set(m_scrollerDaysColumns, EINA_FALSE, EINA_FALSE);
    tools::EflTools::setExpandHints(m_scrollerDaysColumns);
    elm_scroller_policy_set(m_scrollerDaysColumns, ELM_SCROLLER_POLICY_OFF,
            ELM_SCROLLER_POLICY_OFF);
    elm_scroller_movement_block_set(m_scrollerDaysColumns,
            ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);

    m_layoutScrollerDaysColumns = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutScrollerDaysColumns);
    elm_layout_file_set(m_layoutScrollerDaysColumns,
            m_edjeFiles->historyDaysList.c_str(), "historyDaysList");

    elm_object_content_set(m_scrollerDaysColumns, m_layoutScrollerDaysColumns);

    m_boxDaysColumns = elm_box_add(parent);
    tools::EflTools::setExpandHints(m_boxDaysColumns);
    elm_box_horizontal_set(m_boxDaysColumns, EINA_TRUE);
    elm_object_part_content_set(m_layoutScrollerDaysColumns, "daysColumns",
            m_boxDaysColumns);
    elm_box_padding_set(m_boxDaysColumns, 65, 0.0);

    return m_scrollerDaysColumns;
}

void HistoryDaysListManagerTv::addHistoryItems(
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
        historyItems.push_back(std::make_shared<WebsiteHistoryItemData>(
                "Title", itemPair.first, pageViewItems));
    }
    HistoryDayItemDataPtr dayItem = std::make_shared <HistoryDayItemData> (
            toString(period), historyItems);
    appendDayItem(dayItem);
}

void HistoryDaysListManagerTv::clear()
{
    elm_box_clear(m_boxDaysColumns);
    m_dayItems.clear();
}

void HistoryDaysListManagerTv::connectSignals()
{
    HistoryDayItemTv::signalHeaderFocus.connect(
            boost::bind(&HistoryDaysListManagerTv::onHistoryDayItemHeaderFocus,
                    this, _1));
    WebsiteHistoryItemTitleTv::signalWebsiteHistoryItemClicked.connect(
            boost::bind(&HistoryDaysListManagerTv::onWebsiteHistoryItemClicked,
                    this, _1));
    WebsiteHistoryItemVisitItemsTv::signalWebsiteVisitItemClicked.connect(
            boost::bind(&HistoryDaysListManagerTv::onWebsiteHistoryItemVisitItemClicked,
                    this, _1));
}

void HistoryDaysListManagerTv::appendDayItem(HistoryDayItemDataPtr dayItemData)
{
    auto item = std::make_shared<HistoryDayItemTv>(dayItemData, m_historyDeleteManager);
    m_dayItems.push_back(item);
    elm_box_pack_end(m_boxDaysColumns, item->init(m_boxDaysColumns, m_edjeFiles));
}

void HistoryDaysListManagerTv::setFocusChain(Evas_Object* obj)
{
    for(auto& dayItem : m_dayItems) {
        dayItem->setFocusChain(obj);
    }
}

void HistoryDaysListManagerTv::onHistoryDayItemHeaderFocus(
        const HistoryDayItemTv* focusedItem)
{
    scrollToDayItem(focusedItem);
}

void HistoryDaysListManagerTv::onWebsiteHistoryItemClicked(
        const WebsiteHistoryItemDataPtr websiteHistoryItemData)
{
    historyItemClicked(tools::PROTOCOL_DEFAULT + websiteHistoryItemData->websiteDomain,
            websiteHistoryItemData->websiteTitle);
}

void HistoryDaysListManagerTv::onWebsiteHistoryItemVisitItemClicked(
        const WebsiteVisitItemDataPtr websiteVisitItemData)
{
    historyItemClicked(websiteVisitItemData->link, websiteVisitItemData->title);
}

void HistoryDaysListManagerTv::scrollToDayItem(const HistoryDayItemTv* item)
{
    int itemX, itemY, itemW, itemH;
    itemX = itemY = itemW = itemH = 0;
    evas_object_geometry_get(item->getLayoutDayColumn(), &itemX, &itemY, &itemW, &itemH);
    int index = getHistoryItemIndex(item);
    elm_scroller_region_show(m_scrollerDaysColumns, index*itemW, 1, 2*itemW, 1);
}

int HistoryDaysListManagerTv::getHistoryItemIndex(const HistoryDayItemTv* item)
{
    int index = 0;
    for(auto& dayItem : m_dayItems) {
        if(dayItem.get() == item) {
            return index;
        }
        index++;
    }
    return -1;
}

}
}
