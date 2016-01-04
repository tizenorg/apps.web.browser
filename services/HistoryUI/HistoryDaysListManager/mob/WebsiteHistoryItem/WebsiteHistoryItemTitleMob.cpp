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

#include "WebsiteHistoryItemTitleMob.h"
#include "../../HistoryDayItemData.h"
#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

WebsiteHistoryItemTitleMob::WebsiteHistoryItemTitleMob(
        WebsiteHistoryItemDataPtr websiteHistoryItemData)
    : m_websiteHistoryItemData(websiteHistoryItemData)
    , m_layoutMain(nullptr)
    , m_boxMainHorizontal(nullptr)
{
}

WebsiteHistoryItemTitleMob::~WebsiteHistoryItemTitleMob()
{
}

Evas_Object* WebsiteHistoryItemTitleMob::init(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    m_layoutMain = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutMain);
    elm_layout_file_set(m_layoutMain, edjeFilePath.c_str(),
            "layoutWebsiteHistoryItemTitle");

    m_boxMainHorizontal = elm_box_add(parent);
    elm_box_align_set(m_boxMainHorizontal, 0.0, 0.0);
    elm_box_horizontal_set(m_boxMainHorizontal, EINA_TRUE);
    elm_object_part_content_set(m_layoutMain, "boxMainHorizontal",
            m_boxMainHorizontal);

    Evas_Object* layoutIcon = createLayoutIcon(parent, edjeFilePath);
    Evas_Object* layoutSummary = createLayoutSummary(parent, edjeFilePath);
    elm_box_pack_end(m_boxMainHorizontal, layoutIcon);
    elm_box_pack_end(m_boxMainHorizontal, layoutSummary);

    evas_object_show(layoutIcon);
    evas_object_show(layoutSummary);
    evas_object_show(m_layoutMain);

    return m_layoutMain;
}

Evas_Object* WebsiteHistoryItemTitleMob::createLayoutIcon(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* layout = elm_layout_add(parent);
    elm_layout_file_set(layout, edjeFilePath.c_str(), "layoutItemIcon");
    return layout;
}

Evas_Object* WebsiteHistoryItemTitleMob::createLayoutSummary(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* layout = elm_layout_add(parent);
    tools::EflTools::setExpandHints(layout);
    elm_layout_file_set(layout, edjeFilePath.c_str(), "layoutTextSummaryTitle");
    elm_object_text_set(layout, m_websiteHistoryItemData->websiteTitle.c_str());

    return layout;
}

}
}
