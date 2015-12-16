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

#include "HistoryDayItemTv.h"
#include "WebsiteHistoryItem/WebsiteHistoryItemTv.h"
#include "../HistoryDayItemData.h"

#include "EflTools.h"

namespace tizen_browser{
namespace base_ui{

boost::signals2::signal<void(const HistoryDayItemTv*)> HistoryDayItemTv::signalHeaderFocus;

HistoryDayItemTv::HistoryDayItemTv(HistoryDayItemDataPtr dayItemData)
    : m_dayItemData(dayItemData)
{
    for(auto& websiteHistoryItemData : dayItemData->websiteHistoryItems) {
        auto websiteHistoryItem = std::make_shared<WebsiteHistoryItemTv>(
                websiteHistoryItemData);
        m_websiteHistoryItems.push_back(websiteHistoryItem);
    }
}

HistoryDayItemTv::~HistoryDayItemTv()
{
    deleteCallbacks();
    evas_object_del(m_layoutDayColumn);
}

Evas_Object* HistoryDayItemTv::init(Evas_Object* parent,
        HistoryDaysListManagerEdjeTvPtr edjeFiles)
{
    m_layoutDayColumn = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutDayColumn);
    elm_layout_file_set(m_layoutDayColumn, edjeFiles->historyDaysList.c_str(), "layoutDayColumn");

    m_boxMainVertical = elm_box_add(m_layoutDayColumn);
    elm_box_horizontal_set(m_boxMainVertical, EINA_FALSE);
    elm_object_part_content_set(m_layoutDayColumn, "boxMainVertical", m_boxMainVertical);

    m_layoutHeader = elm_layout_add(m_layoutDayColumn);
    evas_object_size_hint_align_set(m_layoutHeader, 0.0, 0.0);
    elm_layout_file_set(m_layoutHeader, edjeFiles->historyDaysList.c_str(), "layoutHeader");
    elm_object_text_set(m_layoutHeader, m_dayItemData->day.c_str());

    m_layoutBoxScrollerWebsites = elm_layout_add(m_layoutDayColumn);
    tools::EflTools::setExpandHints(m_layoutBoxScrollerWebsites);
    elm_layout_file_set(m_layoutBoxScrollerWebsites, edjeFiles->historyDaysList.c_str(), "layoutBoxScrollerWebsites");

    m_boxScrollerWebsites = elm_box_add(m_layoutDayColumn);
    tools::EflTools::setExpandHints(m_boxScrollerWebsites);
    m_scrollerWebsites = createScrollerWebsites(m_layoutDayColumn, edjeFiles);
    elm_box_pack_end(m_boxScrollerWebsites, m_scrollerWebsites);
    elm_object_part_content_set(m_layoutBoxScrollerWebsites, "boxScrollerWebsites", m_boxScrollerWebsites);

    elm_box_pack_end(m_boxMainVertical, m_layoutHeader);
    elm_box_pack_end(m_boxMainVertical, m_layoutBoxScrollerWebsites);

    evas_object_show(m_layoutHeader);
    evas_object_show(m_layoutBoxScrollerWebsites);

    evas_object_show(m_boxScrollerWebsites);
    evas_object_show(m_scrollerWebsites);
    evas_object_show(m_layoutScrollerWebsites);
    evas_object_show(m_boxWebsites);

    evas_object_show(m_boxMainVertical);
    evas_object_show(m_layoutDayColumn);

    initCallbacks();

    return m_layoutDayColumn;
}

void HistoryDayItemTv::setFocusChain(Evas_Object* obj)
{
    elm_object_focus_allow_set(m_layoutHeader, EINA_TRUE);
    elm_object_focus_custom_chain_append(obj, m_layoutHeader, NULL);

    for(auto& websiteHistoryItem : m_websiteHistoryItems) {
        websiteHistoryItem->setFocusChain(obj);
    }
}

Evas_Object* HistoryDayItemTv::createScrollerWebsites(Evas_Object* parent,
        HistoryDaysListManagerEdjeTvPtr edjeFiles)
{
    Evas_Object* scroller = elm_scroller_add(parent);
    tools::EflTools::setExpandHints(scroller);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF,
            ELM_SCROLLER_POLICY_OFF);

    m_layoutScrollerWebsites = elm_layout_add(parent);
    evas_object_size_hint_weight_set(m_layoutScrollerWebsites, EVAS_HINT_EXPAND, 0.0);
    elm_layout_file_set(m_layoutScrollerWebsites,
            edjeFiles->historyDaysList.c_str(), "layoutScrollerWebsites");
    elm_object_content_set(scroller, m_layoutScrollerWebsites);

    m_boxWebsites = elm_box_add(parent);
    elm_box_horizontal_set(m_boxWebsites, EINA_FALSE);
    elm_box_padding_set(m_boxWebsites, 0.0, 1.0);

    elm_object_part_content_set(m_layoutScrollerWebsites, "boxWebsites", m_boxWebsites);

    initBoxWebsites(edjeFiles);

    return scroller;
}

void HistoryDayItemTv::initCallbacks()
{
    evas_object_smart_callback_add(m_layoutHeader, "focused",
        HistoryDayItemTv::_layoutHeaderFocused, this);
}

void HistoryDayItemTv::deleteCallbacks()
{
    evas_object_smart_callback_del(m_layoutHeader, "focused", NULL);
}

void HistoryDayItemTv::initBoxWebsites(HistoryDaysListManagerEdjeTvPtr edjeFiles)
{
    for (auto& websiteHistoryItem : m_websiteHistoryItems) {
        Evas_Object* boxSingleWebsite = websiteHistoryItem->init(m_boxWebsites,
                edjeFiles);
        elm_box_pack_end(m_boxWebsites, boxSingleWebsite);
    }
}

void HistoryDayItemTv::_layoutHeaderFocused(void* data, Evas_Object* /*obj*/,
        void* /*event_info*/)
{
    HistoryDayItemTv *self = static_cast<HistoryDayItemTv*>(data);
    signalHeaderFocus(self);
}

}
}
