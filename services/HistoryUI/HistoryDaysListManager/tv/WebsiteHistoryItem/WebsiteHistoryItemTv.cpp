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

#include "WebsiteHistoryItemTv.h"
#include "WebsiteHistoryItemTitleTv.h"
#include "WebsiteHistoryItemVisitItemsTv.h"
#include "../../HistoryDayItemData.h"
#include <EflTools.h>

namespace tizen_browser{
namespace base_ui{

WebsiteHistoryItemTv::WebsiteHistoryItemTv(
        WebsiteHistoryItemDataPtr websiteHistoryItemData)
    : m_websiteHistoryItemData(websiteHistoryItemData)
    , m_websiteHistoryItemTitle(std::make_shared<WebsiteHistoryItemTitleTv>(websiteHistoryItemData))
    , m_websiteHistoryItemVisitItems(
            std::make_shared<WebsiteHistoryItemVisitItemsTv>(
                    websiteHistoryItemData->websiteVisitItems))
{
}

WebsiteHistoryItemTv::~WebsiteHistoryItemTv()
{
}

Evas_Object* WebsiteHistoryItemTv::init(Evas_Object* parent,
        HistoryDaysListManagerEdjeTvPtr edjeFiles)
{
    m_layoutHistoryItem = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutHistoryItem);
    elm_layout_file_set(m_layoutHistoryItem,
            edjeFiles->websiteHistoryItem.c_str(), "layoutWebsiteHistoryItem");

    m_boxMainHorizontal = createBoxMainHorizontal(parent, edjeFiles);
    elm_object_part_content_set(m_layoutHistoryItem, "boxMainHorizontal",
            m_boxMainHorizontal);

    evas_object_show(m_boxMainHorizontal);
    evas_object_show(m_layoutHistoryItem);

    return m_layoutHistoryItem;
}

void WebsiteHistoryItemTv::setFocusChain(Evas_Object* obj)
{
    m_websiteHistoryItemTitle->setFocusChain(obj);
}

Evas_Object* WebsiteHistoryItemTv::createBoxMainHorizontal(Evas_Object* parent,
        HistoryDaysListManagerEdjeTvPtr edjeFiles)
{
    Evas_Object* box = elm_box_add(parent);
    elm_box_horizontal_set(box, EINA_TRUE);

    elm_box_pack_end(box, m_websiteHistoryItemTitle->init(parent,
            edjeFiles->websiteHistoryItemTitle.c_str()));
    elm_box_pack_end(box, m_websiteHistoryItemVisitItems->init(parent,
            edjeFiles->websiteHistoryItemVisitItems.c_str()));

    elm_box_align_set(box, 0.0, 0.0);

    return box;
}

}
}
