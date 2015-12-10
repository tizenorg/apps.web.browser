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

#include "WebsiteHistoryItemMob.h"
#include "WebsiteHistoryVisitItemMob.h"
#include "../HistoryDayItemData.h"
#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

WebsiteHistoryItemMob::WebsiteHistoryItemMob(WebsiteHistoryItemDataPtr websiteHistoryItemData)
    : m_websiteHistoryItemData(websiteHistoryItemData)
    , m_websiteTitle(websiteHistoryItemData->websiteTitle)
{
    m_websiteTitle = "<font_size=40>" + m_websiteTitle + "</font>";
    for(auto& websiteVisitItemData : websiteHistoryItemData->websiteVisitItems) {
        auto websiteHistoryVisitItem = std::make_shared<WebsiteHistoryVisitItemMob>(
                websiteVisitItemData);
        m_websiteHistoryVisitItems.push_back(websiteHistoryVisitItem);
    }
}

WebsiteHistoryItemMob::~WebsiteHistoryItemMob()
{
    elm_box_clear(m_boxMain);
}

Evas_Object* WebsiteHistoryItemMob::init(Evas_Object* parent)
{
    std::string m_daysListEdjFilePath = EDJE_DIR;
    m_daysListEdjFilePath.append("HistoryUI/HistoryDaysList.edj");

    m_layout = elm_layout_add(parent);
    elm_layout_file_set(m_layout, m_daysListEdjFilePath.c_str(), "historyDaysList_WebsiteHistoryItem");
    tools::EflTools::setExpandHints(m_layout);

    m_boxMain = elm_box_add(m_layout);
    tools::EflTools::setExpandHints(m_boxMain);

    m_boxHeader = elm_box_add(m_layout);
    elm_box_align_set(m_boxHeader, 0.1, 0.0);
    tools::EflTools::setExpandHints(m_boxHeader);
    evas_object_color_set(m_boxHeader, 0, 0, 0, 255);
    elm_box_padding_set(m_boxHeader, 20, 20);
    elm_box_horizontal_set(m_boxHeader, EINA_TRUE);
    m_labelWebsiteTitle = elm_label_add(m_boxHeader);
    elm_object_text_set(m_labelWebsiteTitle, m_websiteTitle.c_str());

    m_boxVisitItems = elm_box_add(m_layout);
    tools::EflTools::setExpandHints(m_boxVisitItems);
    elm_box_padding_set(m_boxVisitItems, 20, 20);
    for (auto& websiteHistoryVisitItem : m_websiteHistoryVisitItems) {
        Evas_Object* boxVisitItem = websiteHistoryVisitItem->init(m_boxVisitItems);
        elm_box_pack_end(m_boxVisitItems, boxVisitItem);
    }

    elm_box_pack_end(m_boxHeader, m_labelWebsiteTitle);
    elm_box_pack_end(m_boxMain, m_boxHeader);
    elm_box_pack_end(m_boxMain, m_boxVisitItems);

    evas_object_show(m_labelWebsiteTitle);
    evas_object_show(m_boxHeader);
    evas_object_show(m_boxVisitItems);
    evas_object_show(m_boxMain);
    evas_object_show(m_layout);

    elm_object_part_content_set(m_layout, "main_box", m_boxMain);

    return m_layout;
}

} /* namespace base_ui */
} /* namespace tizen_browser */
