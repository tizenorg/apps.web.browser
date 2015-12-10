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

#include "HistoryDayItemMob.h"
#include "WebsiteHistoryItemMob.h"
#include "../HistoryDayItemData.h"

#include <EflTools.h>
#include "BrowserLogger.h"

namespace tizen_browser {
namespace base_ui {

HistoryDayItemMob::HistoryDayItemMob(HistoryDayItemDataPtr dayItemData)
    : m_dayItemData(dayItemData)
    , m_day(dayItemData->day)
{
    m_day = "<font_size=30>" + m_day + "</font>";

    for(auto& websiteHistoryItemData : dayItemData->websiteHistoryItems) {
        auto websiteHistoryItem = std::make_shared<WebsiteHistoryItemMob>(
                websiteHistoryItemData);
        m_websiteHistoryItems.push_back(websiteHistoryItem);
    }
}

HistoryDayItemMob::~HistoryDayItemMob()
{
    elm_box_clear(m_boxMain);
}

Evas_Object* HistoryDayItemMob::init(Evas_Object* parent)
{
    std::string m_daysListEdjFilePath = EDJE_DIR;
    m_daysListEdjFilePath.append("HistoryUI/HistoryDaysList.edj");

    m_layout = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layout);
    elm_layout_file_set(m_layout, m_daysListEdjFilePath.c_str(), "historyDaysList_DayItem");

    m_boxMain = elm_box_add(m_layout);
    tools::EflTools::setExpandHints(m_boxMain);
    elm_box_padding_set(m_boxWebsites, 20, 20);

    m_boxHeader = elm_box_add(m_layout);
    tools::EflTools::setExpandHints(m_boxHeader);
    elm_box_horizontal_set(m_boxHeader, EINA_TRUE);

    m_labelDay = elm_label_add(m_layout);
    tools::EflTools::setExpandHints(m_labelDay);
    evas_object_color_set(m_labelDay, 0, 0, 0, 255);
    evas_object_size_hint_align_set(m_labelDay, 0.0, 0.5);
    elm_object_text_set(m_labelDay, m_day.c_str());

    m_boxWebsites = elm_box_add(m_layout);
    tools::EflTools::setExpandHints(m_boxWebsites);
    elm_box_padding_set(m_boxWebsites, 20, 20);
    for (auto& websiteHistoryItem : m_websiteHistoryItems) {
        Evas_Object* boxSingleWebsite = websiteHistoryItem->init(m_boxWebsites);
        elm_box_pack_end(m_boxWebsites, boxSingleWebsite);
    }

    elm_box_pack_end(m_boxHeader, m_labelDay);

    elm_box_pack_end(m_boxMain, m_boxHeader);
    elm_box_pack_end(m_boxMain, m_boxWebsites);

    evas_object_show(m_boxHeader);
    evas_object_show(m_labelDay);
    evas_object_show(m_boxWebsites);
    evas_object_show(m_boxMain);

    evas_object_show(m_layout);

    elm_object_part_content_set(m_layout, "main_box", m_boxMain);

    return m_layout;
}

} /* namespace base_ui */
} /* namespace tizen_browser */
