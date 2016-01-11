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

boost::signals2::signal<void(const WebsiteVisitItemDataPtr)>
WebsiteHistoryItemVisitItemsMob::signalButtonClicked;
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

    initCallbacks();
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

    Evas_Object* buttonSelect = elm_button_add(parent);
    elm_object_part_content_set(lay, "buttonSelect", buttonSelect);
    elm_object_style_set(buttonSelect, "invisible_button");

    evas_object_show(lay);

    WebsiteHistoryItemVisitItemsMob::LayoutVisitItemObjects ret;
    ret.layout = lay;
    ret.buttonSelect = buttonSelect;
    return ret;
}

void WebsiteHistoryItemVisitItemsMob::initCallbacks()
{
    for (auto& websiteVisitItem : m_websiteVisitItems)
        evas_object_smart_callback_add(
                websiteVisitItem.layoutVisitItemObjects.buttonSelect, "clicked",
                _buttonSelectClicked, &websiteVisitItem);
}

void WebsiteHistoryItemVisitItemsMob::_buttonSelectClicked(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data)
        return;
    VisitItemObjects* visitItemObject = static_cast<VisitItemObjects*>(data);
    signalButtonClicked((*visitItemObject).websiteVisitItemData);
}

}
}
