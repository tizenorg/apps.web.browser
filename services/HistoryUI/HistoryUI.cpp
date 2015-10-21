/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "../Tools/BrowserImage.h"


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
    : m_history_layout(nullptr)
    , m_actionBar(nullptr)
    , m_genListToday(nullptr)
    , m_itemClassToday(nullptr)
    , m_gengrid(nullptr)
    , m_parent(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("HistoryUI/History.edj");
    m_item_class = crateItemClass();
}

HistoryUI::~HistoryUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_itemClassToday)
        elm_gengrid_item_class_free(m_itemClassToday);
}

void HistoryUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_history_layout);
    evas_object_show(m_actionBar);
    evas_object_show(m_history_layout);
    elm_object_focus_custom_chain_append(m_history_layout, m_genListToday, nullptr);
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
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_history_layout)
        createHistoryUILayout(m_parent);
    return m_history_layout;
}

void HistoryUI::createHistoryUILayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_history_layout = elm_layout_add(parent);
    elm_layout_file_set(m_history_layout, m_edjFilePath.c_str(), "history-layout");
    evas_object_size_hint_weight_set(m_history_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_history_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_actionBar = createActionBar(m_history_layout);
    m_gengrid = createGengrid(m_history_layout);
    clearItems();
}

Evas_Object* HistoryUI::createGengrid(Evas_Object* history_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(history_layout);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    Evas_Object* gengrid = elm_gengrid_add(history_layout);
    elm_object_part_content_set(history_layout, "history_gengird", gengrid);

    elm_gengrid_align_set(gengrid, 0, 0);
    elm_gengrid_select_mode_set(gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(gengrid, EINA_TRUE);
    elm_gengrid_highlight_mode_set(gengrid, EINA_TRUE);
    elm_scroller_policy_set(gengrid, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
    elm_scroller_page_size_set(gengrid, 0, 580);
    evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
    elm_gengrid_item_size_set(gengrid, 580 * efl_scale, 580 * efl_scale);

    HistoryItemData *itemData = new HistoryItemData();
    itemData->historyUI = std::shared_ptr<tizen_browser::base_ui::HistoryUI>(this);
    Elm_Object_Item* historyView = elm_gengrid_item_append(gengrid, m_item_class, itemData, nullptr, this);
    elm_gengrid_item_selected_set(historyView, EINA_FALSE);

    // create genlist for today entries
    m_genListToday = elm_genlist_add(m_history_layout);
    elm_genlist_homogeneous_set(m_genListToday, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListToday, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListToday, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListToday, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genListToday, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genListToday, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    m_itemClassToday = elm_genlist_item_class_new();
    m_itemClassToday->item_style = "history_url_items";
    m_itemClassToday->func.text_get =  &_listTodayTextGet;
    m_itemClassToday->func.content_get = nullptr;
    m_itemClassToday->func.state_get = nullptr;
    m_itemClassToday->func.del = nullptr;

    return gengrid;
}

Elm_Gengrid_Item_Class* HistoryUI::crateItemClass()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Gengrid_Item_Class* item_class = elm_gengrid_item_class_new();
    item_class->item_style = "history_item";
    item_class->func.text_get = _grid_text_get;
    item_class->func.content_get =  _history_grid_content_get;
    item_class->func.state_get = nullptr;
    item_class->func.del = nullptr;
    return item_class;
}

Evas_Object* HistoryUI::createActionBar(Evas_Object* history_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

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
    elm_object_style_set(button, "history_button");
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

char* HistoryUI::_listTodayTextGet(void* data, Evas_Object*, const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        HistoryItemData *id = static_cast<HistoryItemData*>(data);
        const char *part_name = "history_url_text";
        static const int part_name_len = strlen(part_name);

        if (!strncmp(part_name, part, part_name_len)) {
            if (!id->item->getUrl().empty()) {
                std::string str = id->item->getTitle() + " - " + id->item->getUrl();
                return strdup(str.c_str());
            }
        }
    }
    return nullptr;
}

void HistoryUI::_clearHistory_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    HistoryUI *historyUI = static_cast<HistoryUI*>(data);
    historyUI->clearHistoryClicked();
    historyUI->clearItems();
}

void HistoryUI::addHistoryItem(std::shared_ptr<services::HistoryItem> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    HistoryItemData *itemData = new HistoryItemData();
    itemData->item = hi;
    itemData->historyUI = std::shared_ptr<tizen_browser::base_ui::HistoryUI>(this);
    _history_item_data.push_back(itemData);
    Elm_Object_Item* historyView = elm_genlist_item_append(itemData->historyUI->m_genListToday, itemData->historyUI->m_itemClassToday, itemData, nullptr, ELM_GENLIST_ITEM_NONE, _history_item_clicked_cb, itemData);
    itemData->historyUI->m_map_history_views.insert(std::pair<std::string,Elm_Object_Item*>(hi->getUrl(), historyView));
}

void HistoryUI::addHistoryItems(std::shared_ptr<services::HistoryItemVector> items)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    _history_item_data.clear();
    for (auto it = items->begin(); it != items->end(); ++it)
        addHistoryItem(*it);
}

char* HistoryUI::_grid_text_get(void*, Evas_Object*, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    const char* part_name = "menu_label";
    static const int part_name_len = strlen(part_name);

    if (!strncmp(part_name, part, part_name_len)) {
        return strdup("Today");
    }
    return nullptr;
}

Evas_Object * HistoryUI::_history_grid_content_get(void *data, Evas_Object*, const char *part)
{
    BROWSER_LOGD("[%s:%d] part : %s", __PRETTY_FUNCTION__, __LINE__, part);
    if (data && part) {
        HistoryItemData *id = static_cast<HistoryItemData*>(data);
        const char *part_name = "history_genlist";
        static const int part_name_len = strlen(part_name);

        if(!strncmp(part_name, part, part_name_len)) {
            return id->historyUI->m_genListToday;
        }
    }
    return nullptr;
}

void HistoryUI::removeHistoryItem(const std::string& uri)
{
    BROWSER_LOGD("[%s] uri=%s", __func__, uri.c_str());
    if(m_map_history_views.find(uri) == m_map_history_views.end())
        return;

    Elm_Object_Item* historyView = m_map_history_views.at(uri);
    elm_object_item_del(historyView);
    m_map_history_views.erase(uri);
}

void HistoryUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_genlist_clear(m_genListToday);
    m_map_history_views.clear();
    _history_item_data.clear();
}

void HistoryUI::_history_item_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    HistoryItemData * itemData = static_cast<HistoryItemData *>(data);
    itemData->historyUI->historyItemClicked(itemData->item);
}

}
}
