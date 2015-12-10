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

#include "WebsiteHistoryItemVisitItemsTv.h"
#include "../../HistoryDayItemData.h"
#include <EflTools.h>


namespace tizen_browser{
namespace base_ui{

WebsiteHistoryItemVisitItemsTv::WebsiteHistoryItemVisitItemsTv(
        const std::vector<WebsiteVisitItemDataPtr> websiteVisitItems) :
    m_websiteVisitItems(websiteVisitItems)
{
}

WebsiteHistoryItemVisitItemsTv::~WebsiteHistoryItemVisitItemsTv()
{
}

Evas_Object* WebsiteHistoryItemVisitItemsTv::init(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    m_layoutHistoryItemVisitItems = elm_layout_add(parent);
    elm_layout_file_set(m_layoutHistoryItemVisitItems, edjeFilePath.c_str(),
            "layoutWebsiteHistoryItemVisitItems");
    evas_object_size_hint_align_set(m_layoutHistoryItemVisitItems, 0.0, 0.0);

    m_boxMainVertical = elm_box_add(parent);
    tools::EflTools::setExpandHints(m_boxMainVertical);
    elm_object_part_content_set(m_layoutHistoryItemVisitItems,
            "boxMainVertical", m_boxMainVertical);
    elm_box_padding_set(m_boxMainVertical, 0.0, 1.0);

    for(auto& item : m_websiteVisitItems) {
        Evas_Object* visitLayout = createLayoutVisitItem(parent, edjeFilePath, item);
        elm_box_pack_end(m_boxMainVertical, visitLayout);
        evas_object_show(visitLayout);
    }

    evas_object_show(m_boxMainVertical);
    evas_object_show(m_layoutHistoryItemVisitItems);

    return m_layoutHistoryItemVisitItems;
}

Evas_Object* WebsiteHistoryItemVisitItemsTv::createLayoutVisitItem(
        Evas_Object* parent, const std::string& edjeFilePath,
        WebsiteVisitItemDataPtr websiteVisitItemData)
{
    Evas_Object* lay = elm_layout_add(parent);
    tools::EflTools::setExpandHints(lay);
    elm_layout_file_set(lay, edjeFilePath.c_str(),
            "layoutWebsiteHistoryVisitItem");

    Evas_Object* boxMainHorizontal = elm_box_add(parent);
    tools::EflTools::setExpandHints(boxMainHorizontal);
    elm_box_horizontal_set(boxMainHorizontal, EINA_TRUE);
    elm_box_align_set(boxMainHorizontal, 0.0, 0.0);
    elm_object_part_content_set(lay, "boxMainHorizontal", boxMainHorizontal);

    Evas_Object* layoutDate = createLayoutVisitItemDate(parent, edjeFilePath,
            websiteVisitItemData);
    elm_box_pack_end(boxMainHorizontal, layoutDate);

    Evas_Object* layoutUrl = createLayoutVisitItemUrl(parent, edjeFilePath,
            websiteVisitItemData);
    elm_box_pack_end(boxMainHorizontal, layoutUrl);

    evas_object_show(layoutDate);
    evas_object_show(layoutUrl);

    return lay;
}

Evas_Object* WebsiteHistoryItemVisitItemsTv::createLayoutVisitItemDate(
        Evas_Object* parent, const std::string& edjeFilePath,
        WebsiteVisitItemDataPtr websiteVisitItemData)
{
    Evas_Object* layoutDate = elm_layout_add(parent);
    elm_layout_file_set(layoutDate, edjeFilePath.c_str(),
            "layoutWebsiteHistoryVisitItemDate");
    elm_object_text_set(layoutDate, websiteVisitItemData->date.c_str());
    return layoutDate;
}

Evas_Object* WebsiteHistoryItemVisitItemsTv::createLayoutVisitItemUrl(
        Evas_Object* parent, const std::string& edjeFilePath,
        WebsiteVisitItemDataPtr websiteVisitItemData)
{
    Evas_Object* layoutUrl = elm_layout_add(parent);
    evas_object_size_hint_weight_set(layoutUrl, EVAS_HINT_EXPAND,
            EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layoutUrl, EVAS_HINT_FILL, 0.5);
    elm_layout_file_set(layoutUrl, edjeFilePath.c_str(),
            "layoutWebsiteHistoryVisitItemUrl");

    std::string text = "<font_weight=bold>" + websiteVisitItemData->title
            + "</font_weight>" + " - " + websiteVisitItemData->link;
    elm_object_text_set(layoutUrl, text.c_str());
    return layoutUrl;
}

}
}
