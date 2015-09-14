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


#include <Elementary.h>
#include <vector>
#include <algorithm>
#include "BrowserAssert.h"
#include "DetailPopup.h"
#include "BrowserLogger.h"
#include "Tools/GeneralTools.h"
#include "Tools/EflTools.h"
#include "MainUI.h"

namespace tizen_browser{
namespace base_ui{

const char * DetailPopup::URL_SEPARATOR = " - ";
const int DetailPopup::HISTORY_ITEMS_NO = 5;

DetailPopup::DetailPopup(MainUI *mainUI)
    : m_layout(nullptr)
    , m_historyList(nullptr)
    , m_mainUI(mainUI)
{
    edjFilePath = EDJE_DIR;
    edjFilePath.append("MainUI/DetailPopup.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());

    m_history_item_class = elm_genlist_item_class_new();
    m_history_item_class->item_style = "history_grid_item";
    m_history_item_class->func.text_get = _get_history_link_text;
    m_history_item_class->func.content_get =  nullptr;
    m_history_item_class->func.state_get = nullptr;
    m_history_item_class->func.del = nullptr;
}

DetailPopup::~DetailPopup()
{
    elm_genlist_item_class_free(m_history_item_class);
}

void DetailPopup::createLayout(Evas_Object *parent)
{
    m_layout = elm_layout_add(parent);
    elm_layout_file_set(m_layout, edjFilePath.c_str(), "popup");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    // TODO: add following (or simillar) callback for hardware back button when it will be available
    //evas_object_event_callback_add(m_layout, EVAS_CALLBACK_KEY_DOWN, _back, this);
    edje_object_signal_callback_add(elm_layout_edje_get(m_layout), "mouse,clicked,1", "bg", _bg_click, this);
    edje_object_signal_callback_add(elm_layout_edje_get(m_layout), "mouse,clicked,1", "url_over", _url_click, this);
    edje_object_signal_callback_add(elm_layout_edje_get(m_layout), "mouse,clicked,1", "thumbnail", _url_click, this);
    elm_layout_text_set(m_layout, "history_title", "History");
    elm_layout_text_set(m_layout, "url", tools::clearURL(m_item->getUrl()));

    m_historyList = elm_genlist_add(m_layout);
    evas_object_size_hint_weight_set(m_historyList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_historyList, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(m_historyList, "pressed", _history_url_click, this);
    for (auto it = m_prevItems->begin(); it != m_prevItems->end(); ++it) {
        elm_genlist_item_append(m_historyList, m_history_item_class, it->get(), nullptr, ELM_GENLIST_ITEM_NONE, nullptr, this);
    }
    evas_object_show(m_historyList);
    elm_object_part_content_set(m_layout, "history_list", m_historyList);

    if (m_item->getThumbnail()) {
        Evas_Object * thumb = tools::EflTools::getEvasImage(m_item->getThumbnail(), m_layout);
        elm_object_part_content_set(m_layout, "thumbnail", thumb);
    }

    evas_object_show(m_layout);
}

void DetailPopup::show(Evas_Object *parent, std::shared_ptr<services::HistoryItem> currItem, std::shared_ptr<services::HistoryItemVector> prevItems)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_item = currItem;
    m_prevItems = prevItems;
    createLayout(parent);
}

void DetailPopup::hide()
{
    edje_object_signal_callback_del(elm_layout_edje_get(m_layout), "mouse,clicked,1", "bg", _bg_click);
    edje_object_signal_callback_del(elm_layout_edje_get(m_layout), "mouse,clicked,1", "url_over", _url_click);
    edje_object_signal_callback_del(elm_layout_edje_get(m_layout), "mouse,clicked,1", "thumbnail", _url_click);
    evas_object_smart_callback_del(m_historyList, "pressed", _history_url_click);
    elm_genlist_clear(m_historyList);
    evas_object_hide(m_layout);
    evas_object_del(m_layout);
}

void DetailPopup::_bg_click(void* data, Evas_Object*, const char*, const char*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    DetailPopup *dp = reinterpret_cast<DetailPopup*>(data);
    dp->hide();
}

void DetailPopup::_url_click(void* data, Evas_Object*, const char*, const char*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    DetailPopup *dp = reinterpret_cast<DetailPopup*>(data);
    dp->hide();
    dp->openURLInNewTab(dp->m_item, dp->m_mainUI->isDesktopMode());
}

void DetailPopup::_history_url_click(void* data, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *glit = reinterpret_cast<Elm_Object_Item*>(event_info);
    void *itemData = elm_object_item_data_get(glit);
    services::HistoryItem *item = reinterpret_cast<services::HistoryItem*>(itemData);

    DetailPopup *dp = reinterpret_cast<DetailPopup*>(data);
    // find shared pointer pointed to HistoryItem object
    auto it = std::find_if(dp->m_prevItems->begin(),
                           dp->m_prevItems->end(),
                           [item] (const std::shared_ptr<services::HistoryItem> i)
                           { return i.get() == item; }
                          );
    std::shared_ptr<services::HistoryItem> itemPtr= *it;
    dp->hide();
    dp->openURLInNewTab(itemPtr, dp->m_mainUI->isDesktopMode());
}

char* DetailPopup::_get_history_link_text(void* data, Evas_Object*, const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    services::HistoryItem *item = reinterpret_cast<services::HistoryItem*>(data);
    if (!strcmp(part, "page_dsc")) {
        if (item != nullptr) {
            std::string text = item->getTitle() + URL_SEPARATOR + tools::clearURL(item->getUrl());
            return strdup(text.c_str());
        }
    }
    return strdup("");
}

}
}