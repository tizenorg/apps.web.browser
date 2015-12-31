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

#include "WebsiteHistoryItemVisitItemsMob.h"
#include "../../HistoryDayItemData.h"
#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

WebsiteHistoryItemVisitItemsMob::WebsiteHistoryItemVisitItemsMob(
        const std::vector<WebsiteVisitItemDataPtr> websiteVisitItems)
    : m_eflObjectsDeleted(nullptr)
    , m_layoutMain(nullptr)
    , m_boxMainVertical(nullptr)
{
    for (auto& visitItem : websiteVisitItems) {
        VisitItemObjects obj;
        obj.websiteVisitItemData = visitItem;
        m_websiteVisitItems.push_back(obj);
    }
}

WebsiteHistoryItemVisitItemsMob::~WebsiteHistoryItemVisitItemsMob()
{
    // clear all widgets aded by this class
    if (!m_eflObjectsDeleted)
        evas_object_del(m_layoutMain);
}

Evas_Object* WebsiteHistoryItemVisitItemsMob::init(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    m_layoutMain = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutMain);
    elm_layout_file_set(m_layoutMain, edjeFilePath.c_str(),
            "layoutWebsiteHistoryItemVisitItems");

    m_boxMainVertical = elm_box_add(m_layoutMain);
    tools::EflTools::setExpandHints(m_boxMainVertical);
    elm_object_part_content_set(m_layoutMain,
            "boxMainVertical", m_boxMainVertical);

    for (auto& item : m_websiteVisitItems) {
        LayoutVisitItemObjects layoutObjects = createLayoutVisitItem(
                m_layoutMain, edjeFilePath, item.websiteVisitItemData);
        item.layoutVisitItemObjects = layoutObjects;
        elm_box_pack_end(m_boxMainVertical, layoutObjects.layout);
    }

    evas_object_show(m_boxMainVertical);
    evas_object_show(m_layoutMain);

    return m_layoutMain;
}

void WebsiteHistoryItemVisitItemsMob::setEflObjectsAsDeleted()
{
    m_eflObjectsDeleted = true;
}

WebsiteHistoryItemVisitItemsMob::LayoutVisitItemObjects
WebsiteHistoryItemVisitItemsMob::createLayoutVisitItem(
        Evas_Object* parent, const std::string& edjeFilePath,
        WebsiteVisitItemDataPtr websiteVisitItemData)
{
    Evas_Object* lay = elm_layout_add(parent);
    tools::EflTools::setExpandHints(lay);
    elm_layout_file_set(lay, edjeFilePath.c_str(),
            "layoutWebsiteHistoryVisitItem");

    elm_object_part_text_set(lay, "textTitle",
            websiteVisitItemData->historyItem->getTitle().c_str());

    elm_object_part_text_set(lay, "textUrl",
            websiteVisitItemData->historyItem->getUrl().c_str());

    // TODO: add timestamp conversion to "HH:MM AM/PM"
    elm_object_part_text_set(lay, "textTime",
            "00:00 AM");

    evas_object_show(lay);

    WebsiteHistoryItemVisitItemsMob::LayoutVisitItemObjects ret;
    ret.layout = lay;
    return ret;
}

}
}
