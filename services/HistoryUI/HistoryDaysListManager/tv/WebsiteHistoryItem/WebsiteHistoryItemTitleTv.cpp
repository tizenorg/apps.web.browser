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

#include "WebsiteHistoryItemTitleTv.h"
#include "../../HistoryDayItemData.h"
#include <EflTools.h>

namespace tizen_browser{
namespace base_ui{

WebsiteHistoryItemTitleTv::WebsiteHistoryItemTitleTv(
        WebsiteHistoryItemDataPtr websiteHistoryItemData)
    : m_websiteHistoryItemData(websiteHistoryItemData)
{
}

WebsiteHistoryItemTitleTv::~WebsiteHistoryItemTitleTv()
{
}

Evas_Object* WebsiteHistoryItemTitleTv::init(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    m_layoutHistoryItemTitle = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutHistoryItemTitle);
    elm_layout_file_set(m_layoutHistoryItemTitle, edjeFilePath.c_str(),
            "layoutWebsiteHistoryItemTitle");
    evas_object_size_hint_align_set(m_layoutHistoryItemTitle, 0.0, 0.0);

    m_boxMainHorizontal = elm_box_add(parent);
    tools::EflTools::setExpandHints(m_boxMainHorizontal);
    elm_box_align_set(m_boxMainHorizontal, 0.0, 0.0);
    elm_box_horizontal_set(m_boxMainHorizontal, EINA_TRUE);
    elm_object_part_content_set(m_layoutHistoryItemTitle, "boxMainHorizontal",
            m_boxMainHorizontal);

    Evas_Object* layoutIcon = createLayoutIcon(parent, edjeFilePath);
    Evas_Object* layoutSummary = createLayoutSummary(parent, edjeFilePath);
    elm_box_pack_end(m_boxMainHorizontal, layoutIcon);
    elm_box_pack_end(m_boxMainHorizontal, layoutSummary);

    evas_object_show(layoutIcon);
    evas_object_show(layoutSummary);
    evas_object_show(m_layoutHistoryItemTitle);

    return m_layoutHistoryItemTitle;
}

Evas_Object* WebsiteHistoryItemTitleTv::createLayoutIcon(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* layout = elm_layout_add(parent);
    elm_layout_file_set(layout, edjeFilePath.c_str(), "layoutItemIcon");
    return layout;
}

Evas_Object* WebsiteHistoryItemTitleTv::createLayoutSummary(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* layout = elm_layout_add(parent);
    tools::EflTools::setExpandHints(layout);
    elm_layout_file_set(layout, edjeFilePath.c_str(), "layoutItemSummary");

    Evas_Object* boxVertical = elm_box_add(parent);
    tools::EflTools::setExpandHints(boxVertical);
    elm_box_horizontal_set(boxVertical, EINA_FALSE);
    elm_object_part_content_set(layout, "boxMainVertical", boxVertical);

    Evas_Object* layoutTextSummaryTitle = elm_layout_add(parent);
    tools::EflTools::setExpandHints(layoutTextSummaryTitle);
    elm_layout_file_set(layoutTextSummaryTitle, edjeFilePath.c_str(), "layoutTextSummaryTitle");
    elm_object_text_set(layoutTextSummaryTitle,
            m_websiteHistoryItemData->websiteTitle.c_str());

    Evas_Object* layoutTextSummaryDomain = elm_layout_add(parent);
    tools::EflTools::setExpandHints(layoutTextSummaryDomain);
    elm_layout_file_set(layoutTextSummaryDomain, edjeFilePath.c_str(), "layoutTextSummaryTitle");
    elm_object_text_set(layoutTextSummaryDomain,
            m_websiteHistoryItemData->websiteDomain.c_str());

    elm_box_pack_end(boxVertical, layoutTextSummaryTitle);
    elm_box_pack_end(boxVertical, layoutTextSummaryDomain);

    evas_object_show(layout);
    evas_object_show(layoutTextSummaryTitle);
    evas_object_show(layoutTextSummaryDomain);

    return layout;
}

}
}
