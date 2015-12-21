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

#include "HistoryDayItemTv.h"
#include "WebsiteHistoryItem/WebsiteHistoryItemTv.h"
#include "../HistoryDayItemData.h"
#include <services/HistoryUI/HistoryDeleteManager.h>

#include "EflTools.h"

namespace tizen_browser{
namespace base_ui{

boost::signals2::signal<void(const HistoryDayItemTv*)> HistoryDayItemTv::signalHeaderFocus;
boost::signals2::signal<void(const HistoryDayItemDataPtr)>
        HistoryDayItemTv::signaButtonClicked;

HistoryDayItemTv::HistoryDayItemTv(HistoryDayItemDataPtr dayItemData,
        HistoryDeleteManagerPtrConst deleteManager)
    : m_eflObjectsDeleted(nullptr)
    , m_dayItemData(dayItemData)
    , m_layoutMain(nullptr)
    , m_buttonSelect(nullptr)
    , m_imageClear(nullptr)
    , m_boxMainVertical(nullptr)
    , m_layoutHeader(nullptr)
    , m_boxHeader(nullptr)
    , m_layoutBoxScrollerWebsites(nullptr)
    , m_boxScrollerWebsites(nullptr)
    , m_scrollerWebsites(nullptr)
    , m_layoutScrollerWebsites(nullptr)
    , m_boxWebsites(nullptr)
    , m_historyDeleteManager(deleteManager)
{
    for(auto& websiteHistoryItemData : dayItemData->websiteHistoryItems) {
        auto websiteHistoryItem = std::make_shared<WebsiteHistoryItemTv>(
                websiteHistoryItemData, m_historyDeleteManager);
        m_websiteHistoryItems.push_back(websiteHistoryItem);
    }
}

HistoryDayItemTv::~HistoryDayItemTv()
{
    // clear all widgets aded by this class
    if(!m_eflObjectsDeleted) {
        evas_object_del(m_layoutMain);
    }
    deleteCallbacks();
}

Evas_Object* HistoryDayItemTv::init(Evas_Object* parent,
        HistoryDaysListManagerEdjeTvPtr edjeFiles)
{
    m_layoutMain = elm_layout_add(parent);
    tools::EflTools::setExpandHints(m_layoutMain);
    elm_layout_file_set(m_layoutMain, edjeFiles->historyDaysList.c_str(), "layoutDayColumn");

    // add all children widgets with m_layoutMain as parent
    m_boxMainVertical = elm_box_add(m_layoutMain);
    elm_box_horizontal_set(m_boxMainVertical, EINA_FALSE);
    elm_object_part_content_set(m_layoutMain, "boxMainVertical", m_boxMainVertical);

    m_layoutHeader = elm_layout_add(m_layoutMain);
    evas_object_size_hint_align_set(m_layoutHeader, 0.0, 0.0);
    elm_layout_file_set(m_layoutHeader, edjeFiles->historyDaysList.c_str(), "layoutHeader");
    elm_object_text_set(m_layoutHeader, m_dayItemData->day.c_str());

    m_buttonSelect = elm_button_add(m_layoutMain);
    elm_object_part_content_set(m_layoutHeader, "buttonSelect",
            m_buttonSelect);
    evas_object_color_set(m_buttonSelect, 0, 0, 0, 0);

    m_imageClear = createImageClear(m_layoutMain, edjeFiles->historyDaysList.c_str());
    elm_object_part_content_set(m_layoutHeader, "imageClear",
            m_imageClear);

    m_layoutBoxScrollerWebsites = elm_layout_add(m_layoutMain);
    tools::EflTools::setExpandHints(m_layoutBoxScrollerWebsites);
    elm_layout_file_set(m_layoutBoxScrollerWebsites, edjeFiles->historyDaysList.c_str(), "layoutBoxScrollerWebsites");

    m_boxScrollerWebsites = elm_box_add(m_layoutMain);
    tools::EflTools::setExpandHints(m_boxScrollerWebsites);
    m_scrollerWebsites = createScrollerWebsites(m_layoutMain, edjeFiles);
    elm_box_pack_end(m_boxScrollerWebsites, m_scrollerWebsites);
    elm_object_part_content_set(m_layoutBoxScrollerWebsites, "boxScrollerWebsites", m_boxScrollerWebsites);

    elm_box_pack_end(m_boxMainVertical, m_layoutHeader);
    elm_box_pack_end(m_boxMainVertical, m_layoutBoxScrollerWebsites);

    evas_object_show(m_buttonSelect);
    evas_object_show(m_layoutHeader);
    evas_object_show(m_layoutBoxScrollerWebsites);

    evas_object_show(m_boxScrollerWebsites);
    evas_object_show(m_scrollerWebsites);
    evas_object_show(m_layoutScrollerWebsites);
    evas_object_show(m_boxWebsites);

    evas_object_show(m_boxMainVertical);
    evas_object_show(m_layoutMain);

    initCallbacks();
    return m_layoutMain;
}

void HistoryDayItemTv::setEflObjectsAsDeleted()
{
    m_eflObjectsDeleted = true;
    for(auto& websiteHistoryItem : m_websiteHistoryItems) {
        websiteHistoryItem->setEflObjectsAsDeleted();
    }
}

void HistoryDayItemTv::setFocusChain(Evas_Object* obj)
{
    elm_object_focus_allow_set(m_buttonSelect, EINA_TRUE);
    elm_object_focus_custom_chain_append(obj, m_buttonSelect, NULL);

    for(auto& websiteHistoryItem : m_websiteHistoryItems) {
        websiteHistoryItem->setFocusChain(obj);
    }
}

Evas_Object* HistoryDayItemTv::createScrollerWebsites(Evas_Object* parent,
        HistoryDaysListManagerEdjeTvPtr edjeFiles)
{
    Evas_Object* scroller = elm_scroller_add(parent);
    tools::EflTools::setExpandHints(scroller);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF,
            ELM_SCROLLER_POLICY_OFF);

    m_layoutScrollerWebsites = elm_layout_add(parent);
    evas_object_size_hint_weight_set(m_layoutScrollerWebsites, EVAS_HINT_EXPAND, 0.0);
    elm_layout_file_set(m_layoutScrollerWebsites,
            edjeFiles->historyDaysList.c_str(), "layoutScrollerWebsites");
    elm_object_content_set(scroller, m_layoutScrollerWebsites);

    m_boxWebsites = elm_box_add(parent);
    elm_box_horizontal_set(m_boxWebsites, EINA_FALSE);
    elm_box_padding_set(m_boxWebsites, 0.0, 1.0);

    elm_object_part_content_set(m_layoutScrollerWebsites, "boxWebsites", m_boxWebsites);

    initBoxWebsites(edjeFiles);
    return scroller;
}

Evas_Object* HistoryDayItemTv::createImageClear(Evas_Object* parent,
        const std::string& edjeFilePath)
{
    Evas_Object* imageClear = elm_image_add(parent);
    elm_image_file_set(imageClear, edjeFilePath.c_str(), "groupImageClear");
    return imageClear;
}

void HistoryDayItemTv::initCallbacks()
{
    evas_object_smart_callback_add(m_layoutHeader, "focused",
        HistoryDayItemTv::_layoutHeaderFocused, this);
    evas_object_smart_callback_add(m_buttonSelect, "clicked",
            HistoryDayItemTv::_buttonSelectClicked, &m_dayItemData);
    evas_object_smart_callback_add(m_buttonSelect, "focused",
            HistoryDayItemTv::_buttonSelectFocused, this);
    evas_object_smart_callback_add(m_buttonSelect, "unfocused",
            HistoryDayItemTv::_buttonSelectUnfocused, this);
}

void HistoryDayItemTv::_buttonSelectFocused(void* data, Evas_Object* /*obj*/,
        void* /*event_info*/)
{
    HistoryDayItemTv* self = static_cast<HistoryDayItemTv*>(data);
    Evas_Object* layoutHeader = self->getLayoutHeader();
    elm_object_signal_emit(layoutHeader, "buttonSelectFocused", "ui");
    if(self->getDeleteManager()->getDeleteMode()) {
        elm_object_signal_emit(layoutHeader, "showImageClear", "ui");
    }
}

void HistoryDayItemTv::_buttonSelectUnfocused(void* data, Evas_Object* /*obj*/,
        void* /*event_info*/)
{
    HistoryDayItemTv* self = static_cast<HistoryDayItemTv*>(data);
    Evas_Object* layoutHeader = self->getLayoutHeader();
    elm_object_signal_emit(layoutHeader, "buttonSelectUnfocused", "ui");
}

void HistoryDayItemTv::_buttonSelectClicked(void* data, Evas_Object* /*obj*/,
        void*)
{
    HistoryDayItemDataPtr* historyDayItemData =
            static_cast<HistoryDayItemDataPtr*>(data);
    signaButtonClicked(*historyDayItemData);
}

void HistoryDayItemTv::deleteCallbacks()
{
    evas_object_smart_callback_del(m_layoutHeader, "focused", NULL);
}

void HistoryDayItemTv::initBoxWebsites(HistoryDaysListManagerEdjeTvPtr edjeFiles)
{
    for (auto& websiteHistoryItem : m_websiteHistoryItems) {
        Evas_Object* boxSingleWebsite = websiteHistoryItem->init(m_boxWebsites,
                edjeFiles);
        elm_box_pack_end(m_boxWebsites, boxSingleWebsite);
    }
}

void HistoryDayItemTv::_layoutHeaderFocused(void* data, Evas_Object* /*obj*/,
        void* /*event_info*/)
{
    HistoryDayItemTv *self = static_cast<HistoryDayItemTv*>(data);
    signalHeaderFocus(self);
}

}
}
