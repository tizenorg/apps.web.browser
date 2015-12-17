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
#include <boost/concept_check.hpp>
#include <vector>
#include <string>
#include <string.h>
#include <AbstractMainWindow.h>

#include "HistoryUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "Tools/GeneralTools.h"
#include "HistoryDaysListManager/HistoryDaysListManagerMob.h"
#include "HistoryDaysListManager/HistoryDaysListManagerTv.h"
#include "services/HistoryService/HistoryItem.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(HistoryUI, "org.tizen.browser.historyui")

typedef struct _HistoryItemData
{
    std::shared_ptr<tizen_browser::services::HistoryItem> item;
    std::shared_ptr<tizen_browser::base_ui::HistoryUI> historyUI;
} HistoryItemData;

struct ItemData{
    tizen_browser::base_ui::HistoryUI* historyUI;
    Elm_Object_Item * e_item;
};

static std::vector<HistoryItemData*> _history_item_data;

HistoryUI::HistoryUI()
    : m_parent(nullptr)
    , m_history_layout(nullptr)
    , m_actionBar(nullptr)
    , m_daysList(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("HistoryUI/History.edj");

#if PROFILE_MOBILE
    m_historyDaysListManager = std::unique_ptr<HistoryDaysListManager>(new HistoryDaysListManagerMob());
#else
    m_historyDaysListManager = std::unique_ptr<HistoryDaysListManager>(new HistoryDaysListManagerTv());
#endif
}

HistoryUI::~HistoryUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void HistoryUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_history_layout);
    evas_object_show(m_actionBar);
    evas_object_show(m_history_layout);
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);
}

void HistoryUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_history_layout);
    evas_object_hide(m_actionBar);
    evas_object_hide(m_history_layout);
    elm_object_focus_custom_chain_unset(m_history_layout);
    clearItems();
}


void HistoryUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* HistoryUI::getContent()
{
    M_ASSERT(m_parent);
    if (!m_history_layout)
        createHistoryUILayout(m_parent);
    return m_history_layout;
}

void HistoryUI::createHistoryUILayout(Evas_Object* parent)
{
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_history_layout = elm_layout_add(parent);

    elm_layout_file_set(m_history_layout, m_edjFilePath.c_str(), "history-layout");
    evas_object_size_hint_weight_set(m_history_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_history_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_actionBar = createActionBar(m_history_layout);
    m_daysList = createDaysList(m_history_layout);
    clearItems();
}

std::map<std::string, services::HistoryItemVector>
HistoryUI::groupItemsByDomain(const services::HistoryItemVector& historyItems) {
    std::map<std::string, services::HistoryItemVector> groupedMap;
    for(auto& item : historyItems) {
        std::string domain = tools::extractDomain(item->getUrl());
        if(groupedMap.find(domain) == groupedMap.end()) {
            groupedMap.insert(std::pair<std::string, services::HistoryItemVector>(domain, {}));
        }
        groupedMap.find(domain)->second.push_back(item);
    }
    return groupedMap;
}

Evas_Object *HistoryUI::createDaysList(Evas_Object *history_layout)
{
    M_ASSERT(history_layout);

    Evas_Object* list = m_historyDaysListManager->createDaysList(
            history_layout);

    elm_object_part_content_set(history_layout, "history_list", list);

    evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    return list;
}

Evas_Object* HistoryUI::createActionBar(Evas_Object* history_layout)
{
    Evas_Object* actionBar = elm_layout_add(history_layout);
    elm_object_part_content_set(history_layout, "action_bar_history", actionBar);
    evas_object_size_hint_weight_set(actionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(actionBar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_layout_file_set(actionBar, m_edjFilePath.c_str(), "action_bar");

    Evas_Object *button = elm_button_add(actionBar);
    elm_object_style_set(button, "history_button");
    evas_object_smart_callback_add(button, "clicked", HistoryUI::_clearHistory_clicked, this);
    elm_object_part_content_set(actionBar, "clearhistory_click", button);
    elm_object_focus_custom_chain_append(history_layout, button, nullptr);

    button = elm_button_add(actionBar);
#if PROFILE_MOBILE
    elm_object_style_set(button, "history_button");
#else
    elm_object_style_set(button, "close_history_button");
#endif
    evas_object_smart_callback_add(button, "clicked", HistoryUI::_close_clicked_cb, this);
    elm_object_part_content_set(actionBar, "close_click", button);
    elm_object_focus_custom_chain_append(history_layout, button, nullptr);
    return actionBar;
}

void HistoryUI::_close_clicked_cb(void * data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        HistoryUI *historyUI = static_cast<HistoryUI*>(data);
        historyUI->closeHistoryUIClicked();
    }
}

void HistoryUI::_clearHistory_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    HistoryUI *historyUI = static_cast<HistoryUI*>(data);
    historyUI->clearHistoryClicked();
    historyUI->clearItems();
}

void HistoryUI::addHistoryItems(std::shared_ptr<services::HistoryItemVector> items,
        HistoryPeriod period)
{
    if(items->size() == 0) return;
    auto grouped = groupItemsByDomain(*items);
    m_historyDaysListManager->addHistoryItems(grouped, period);
}

void HistoryUI::removeHistoryItem(const std::string& uri)
{
    BROWSER_LOGD("[%s] uri=%s", __func__, uri.c_str());
}

void HistoryUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_historyDaysListManager->clear();
}

}
}
