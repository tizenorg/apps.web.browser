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
#include "HistoryDaysListManagerMob.h"
#include "HistoryDayItemData.h"
#include "mob/HistoryDayItemMob.h"
#include "mob/WebsiteHistoryItem/WebsiteHistoryItemMob.h"
#include "mob/WebsiteHistoryItem/WebsiteHistoryItemTitleMob.h"
#include "mob/WebsiteHistoryItem/WebsiteHistoryItemVisitItemsMob.h"
#include <services/HistoryUI/HistoryDeleteManager.h>

#include <GeneralTools.h>
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
    connectSignals();
}

HistoryDaysListManagerMob::~HistoryDaysListManagerMob()
{
    for (auto& dayItem : m_dayItems)
        dayItem->setEflObjectsAsDeleted();
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
                        > (itemPair.first, itemPair.first, pageViewItems));
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

HistoryDayItemMobPtr HistoryDaysListManagerMob::getItem(
        HistoryDayItemDataPtrConst historyDayItemData)
{
    for(auto& historyDayItem : m_dayItems) {
        if(historyDayItem->getData() == historyDayItemData)
            return historyDayItem;
    }
    return nullptr;
}

void HistoryDaysListManagerMob::connectSignals()
{
    HistoryDayItemMob::signaButtonClicked.connect(
            boost::bind(&HistoryDaysListManagerMob::onHistoryDayItemButtonClicked,
                    this, _1, _2));
    WebsiteHistoryItemTitleMob::signalButtonClicked.connect(
            boost::bind(&HistoryDaysListManagerMob::onWebsiteHistoryItemClicked,
                    this, _1, _2));
    WebsiteHistoryItemVisitItemsMob::signalButtonClicked.connect(
            boost::bind(
                    &HistoryDaysListManagerMob::onWebsiteHistoryItemVisitItemClicked,
                    this, _1, _2));
}


void HistoryDaysListManagerMob::appendDayItem(HistoryDayItemDataPtr dayItemData)
{
    auto item = std::make_shared<HistoryDayItemMob>(dayItemData);
    m_dayItems.push_back(item);

    Evas_Object* dayItemLayout = item->init(m_parent, m_edjeFiles);
    elm_box_pack_end(m_boxDays, dayItemLayout);
}
void HistoryDaysListManagerMob::onHistoryDayItemButtonClicked(
        const HistoryDayItemDataPtrConst clickedItem, bool deleteItem)
{
    if (deleteItem)
        removeItem(clickedItem);
}

void HistoryDaysListManagerMob::onWebsiteHistoryItemClicked(
        const WebsiteHistoryItemDataPtrConst clickedItem, bool deleteItem)
{
    if (deleteItem)
        removeItem(clickedItem);
    else
        signalHistoryItemClicked(
                tools::PROTOCOL_DEFAULT + clickedItem->websiteDomain,
                clickedItem->websiteTitle);
}

void HistoryDaysListManagerMob::onWebsiteHistoryItemVisitItemClicked(
        const WebsiteVisitItemDataPtrConst clickedItem, bool deleteItem)
{
    if (deleteItem) {
        removeItem(clickedItem);
        signalDeleteHistoryItems(
                std::make_shared<std::vector<int>>(std::initializer_list<int> {
                        clickedItem->historyItem->getId() }));
    } else
        signalHistoryItemClicked(clickedItem->historyItem->getUrl(),
                clickedItem->historyItem->getTitle());
}

void HistoryDaysListManagerMob::removeItem(
        HistoryDayItemDataPtrConst historyDayItemData)
{
    auto item = getItem(historyDayItemData);
    if (!item)
        return;
    // remove day item from vector, destructor will clear efl objects
    remove(item);
    elm_box_unpack(m_boxDays, item->getLayoutMain());
}

void HistoryDaysListManagerMob::removeItem(
        WebsiteHistoryItemDataPtrConst websiteHistoryItemData)
{
    if (!websiteHistoryItemData) {
        BROWSER_LOGE("%s remove error", __PRETTY_FUNCTION__);
        return;
    }
    for (auto& dayItem : m_dayItems) {
        auto websiteHistoryItem = dayItem->getItem(websiteHistoryItemData);
        if (websiteHistoryItem) {
            signalDeleteHistoryItems(websiteHistoryItem->getVisitItemsIds());
            dayItem->removeItem(websiteHistoryItemData);
            return;
        }
    }
}

void HistoryDaysListManagerMob::removeItem(
        WebsiteVisitItemDataPtrConst websiteVisitItemData)
{
    for (auto& dayItem : m_dayItems) {
        if (dayItem->getItem(websiteVisitItemData)) {
            dayItem->removeItem(websiteVisitItemData);
            return;
        }
    }
}

void HistoryDaysListManagerMob::remove(HistoryDayItemMobPtr historyDayItem)
{
    for (auto it = m_dayItems.begin(); it != m_dayItems.end();) {
        if ((*it) == historyDayItem) {
            m_dayItems.erase(it);
            return;
        } else {
            ++it;
        }
    }
}

} /* namespace base_ui */
} /* namespace tizen_browser */
