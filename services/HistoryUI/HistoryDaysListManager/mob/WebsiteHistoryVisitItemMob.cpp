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

#include "WebsiteHistoryVisitItemMob.h"
#include "../HistoryDayItemData.h"

#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

WebsiteHistoryVisitItemMob::WebsiteHistoryVisitItemMob(
        WebsiteVisitItemDataPtr visitItemData)
    : m_websiteVisitItemData(visitItemData)
    , m_websiteTitle(visitItemData->title)
    , m_websiteUrl(visitItemData->link)
    , m_websiteTimestamp(visitItemData->date)
{
    m_websiteTitle = "<font_size=34>" + m_websiteTitle + "</font>";
    m_websiteUrl = "<font_size=30>" + m_websiteUrl + "</font>";
    m_websiteTimestamp = "<font_size=30>" + m_websiteTimestamp + "</font>";
}

WebsiteHistoryVisitItemMob::~WebsiteHistoryVisitItemMob()
{
    elm_box_clear(m_boxMain);
}

Evas_Object* WebsiteHistoryVisitItemMob::init(Evas_Object* parent)
{
    std::string m_daysListEdjFilePath = EDJE_DIR;
    m_daysListEdjFilePath.append("HistoryUI/HistoryDaysList.edj");

    m_layout = elm_layout_add(parent);
    elm_layout_file_set(m_layout, m_daysListEdjFilePath.c_str(), "historyDaysList_WebsiteHistoryVisitItem");
    tools::EflTools::setExpandHints(m_layout);

    m_boxMain = elm_box_add(m_layout);
    evas_object_color_set(m_boxMain, 0, 0, 0, 255);
    tools::EflTools::setExpandHints(m_boxMain);

    m_tableVisit = elm_table_add(m_layout);
    tools::EflTools::setExpandHints(m_tableVisit);

    m_labelTitle = elm_label_add(parent);
    tools::EflTools::setExpandHints(m_labelTitle);
    evas_object_size_hint_align_set(m_labelTitle, 0.0, 0.0);
    elm_object_text_set(m_labelTitle, m_websiteTitle.c_str());

    m_labelUrl = elm_label_add(parent);
    tools::EflTools::setExpandHints(m_labelUrl);
    evas_object_size_hint_align_set(m_labelUrl, 0.0, 0.0);
    elm_object_text_set(m_labelUrl, m_websiteUrl.c_str());

    m_labelTimestamp = elm_label_add(parent);
    tools::EflTools::setExpandHints(m_labelTimestamp);
    evas_object_size_hint_align_set(m_labelTimestamp, 1.0, 0.0);
    elm_object_text_set(m_labelTimestamp, m_websiteTimestamp.c_str());

    elm_table_pack(m_tableVisit, m_labelTitle, 0, 0, 1, 1);
    elm_table_pack(m_tableVisit, m_labelUrl, 0, 1, 1, 1);
    elm_table_pack(m_tableVisit, m_labelTimestamp, 1, 1, 1, 1);

    elm_box_pack_end(m_boxMain, m_tableVisit);

    evas_object_show(m_labelTitle);
    evas_object_show(m_labelUrl);
    evas_object_show(m_labelTimestamp);
    evas_object_show(m_tableVisit);
    evas_object_show(m_boxMain);
    evas_object_show(m_layout);

    elm_object_part_content_set(m_layout, "main_box", m_boxMain);

    return m_layout;
}

} /* namespace base_ui */
} /* namespace tizen_browser */
