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
#include <services/HistoryUI/HistoryDeleteManager.h>
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

boost::signals2::signal<void(const WebsiteVisitItemDataPtr)>
WebsiteHistoryItemVisitItemsTv::signalWebsiteVisitItemClicked;

WebsiteHistoryItemVisitItemsTv::WebsiteHistoryItemVisitItemsTv(
        const std::vector<WebsiteVisitItemDataPtr> websiteVisitItems,
        HistoryDeleteManagerPtrConst historyDeleteManager)
    : m_eflObjectsDeleted(false)
    , m_layoutMain(nullptr)
    , m_boxMainVertical(nullptr)
    , m_historyDeleteManager(historyDeleteManager)
{
    for (auto& visitItem : websiteVisitItems) {
        VisitItemObjects obj;
        obj.websiteVisitItemData = visitItem;
        obj.deleteManager = historyDeleteManager;
        m_websiteVisitItems.push_back(obj);
    }
}

WebsiteHistoryItemVisitItemsTv::~WebsiteHistoryItemVisitItemsTv()
{
    deleteCallbacks();
    if (!m_eflObjectsDeleted)
        evas_object_del(m_layoutMain);
}

Evas_Object* WebsiteHistoryItemVisitItemsTv::init(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    m_layoutMain = elm_layout_add(parent);
    elm_layout_file_set(m_layoutMain, edjeFilePath.c_str(),
            "layoutWebsiteHistoryItemVisitItems");
    evas_object_size_hint_align_set(m_layoutMain, 0.0, 0.0);

    m_boxMainVertical = elm_box_add(m_layoutMain);
    tools::EflTools::setExpandHints(m_boxMainVertical);
    elm_object_part_content_set(m_layoutMain,
            "boxMainVertical", m_boxMainVertical);
    elm_box_padding_set(m_boxMainVertical, 0.0, 1.0);

    for (auto& item : m_websiteVisitItems) {
        LayoutVisitItemObjects layoutObjects = createLayoutVisitItem(
                m_layoutMain, edjeFilePath, item.websiteVisitItemData);
        item.layoutVisitItemObjects = layoutObjects;
        elm_box_pack_end(m_boxMainVertical, layoutObjects.layout);
        evas_object_show(layoutObjects.layout);
    }

    evas_object_show(m_boxMainVertical);
    evas_object_show(m_layoutMain);

    initCallbacks();

    return m_layoutMain;
}

void WebsiteHistoryItemVisitItemsTv::setFocusChain(Evas_Object* obj)
{
    for (auto& websiteVisitItem : m_websiteVisitItems) {
        elm_object_focus_allow_set(
                websiteVisitItem.layoutVisitItemObjects.buttonSelect,
                EINA_TRUE);
        elm_object_focus_custom_chain_append(obj,
                websiteVisitItem.layoutVisitItemObjects.buttonSelect, NULL);
    }
}

WebsiteHistoryItemVisitItemsTv::LayoutVisitItemObjects
WebsiteHistoryItemVisitItemsTv::createLayoutVisitItem(
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

    Evas_Object* buttonSelect = elm_button_add(parent);
    elm_object_part_content_set(lay, "buttonSelect", buttonSelect);
    evas_object_color_set(buttonSelect, 0, 0, 0, 0);
    elm_object_style_set(buttonSelect, "anchor");

    Evas_Object* imageClear = createImageClear(parent, edjeFilePath);
    elm_object_part_content_set(lay, "imageClear", imageClear);

    Evas_Object* layoutDate = createLayoutVisitItemDate(parent, edjeFilePath,
            websiteVisitItemData);
    elm_box_pack_end(boxMainHorizontal, layoutDate);

    Evas_Object* layoutUrl = createLayoutVisitItemUrl(parent, edjeFilePath,
            websiteVisitItemData);
    elm_box_pack_end(boxMainHorizontal, layoutUrl);

    evas_object_show(layoutDate);
    evas_object_show(layoutUrl);

    WebsiteHistoryItemVisitItemsTv::LayoutVisitItemObjects ret;
    ret.layout = lay;
    ret.buttonSelect = buttonSelect;
    ret.imageClear = imageClear;
    return ret;
}

Evas_Object* WebsiteHistoryItemVisitItemsTv::createImageClear(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* imageClear = elm_image_add(parent);
    elm_image_file_set(imageClear, edjeFilePath.c_str(), "groupImageClear");
    return imageClear;
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

void WebsiteHistoryItemVisitItemsTv::deleteCallbacks()
{
    for (auto& visitItem : m_websiteVisitItems)
        evas_object_smart_callback_del(
                visitItem.layoutVisitItemObjects.buttonSelect, "clicked", NULL);
}

void WebsiteHistoryItemVisitItemsTv::initCallbacks()
{
    for (auto& websiteVisitItem : m_websiteVisitItems) {
        evas_object_smart_callback_add(
                websiteVisitItem.layoutVisitItemObjects.buttonSelect, "clicked",
                _buttonSelectClicked, &websiteVisitItem);

        evas_object_smart_callback_add(
                websiteVisitItem.layoutVisitItemObjects.buttonSelect, "focused",
                _buttonSelectFocused, &websiteVisitItem);

        evas_object_smart_callback_add(
                websiteVisitItem.layoutVisitItemObjects.buttonSelect,
                "unfocused", _buttonSelectUnfocused, &websiteVisitItem);
    }
}

void WebsiteHistoryItemVisitItemsTv::_buttonSelectClicked(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data) return;
    WebsiteVisitItemDataPtr* websiteVisitItemData = static_cast<WebsiteVisitItemDataPtr*>(data);
    signalWebsiteVisitItemClicked(*websiteVisitItemData);
}

void WebsiteHistoryItemVisitItemsTv::_buttonSelectFocused(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data) return;
    VisitItemObjects* visitItemObject = static_cast<VisitItemObjects*>(data);
    Evas_Object* layout = visitItemObject->layoutVisitItemObjects.layout;
    elm_object_signal_emit(layout, "buttonSelectFocused", "ui");
    if (visitItemObject->deleteManager->getDeleteMode())
        elm_object_signal_emit(layout, "showImageClear", "ui");
}

void WebsiteHistoryItemVisitItemsTv::_buttonSelectUnfocused(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    if (!data) return;
    VisitItemObjects* visitItemObject = static_cast<VisitItemObjects*>(data);
    Evas_Object* layout = visitItemObject->layoutVisitItemObjects.layout;
    elm_object_signal_emit(layout, "buttonSelectUnfocused", "ui");
}

void WebsiteHistoryItemVisitItemsTv::setEflObjectsAsDeleted()
{
    m_eflObjectsDeleted = true;
}

}
}
