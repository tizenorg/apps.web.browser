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
    , m_item_class(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("HistoryUI/History.edj");
}

HistoryUI::~HistoryUI()
{

}

void HistoryUI::show(Evas_Object* parent)
{
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_history_layout = elm_layout_add(parent);
    elm_layout_file_set(m_history_layout, m_edjFilePath.c_str(), "history-layout");
    evas_object_size_hint_weight_set(m_history_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_history_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_history_layout);

    showActionBar();

    m_gengrid = elm_gengrid_add(m_history_layout);
    elm_object_part_content_set(m_history_layout, "history_gengird", m_gengrid);

    if (!m_item_class) {
        m_item_class = elm_gengrid_item_class_new();
        m_item_class->item_style = "history_item";
        m_item_class->func.text_get = _grid_text_get;
        m_item_class->func.content_get =  _history_grid_content_get;
        m_item_class->func.state_get = nullptr;
        m_item_class->func.del = nullptr;
    }

    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(m_gengrid, EINA_TRUE);
    elm_gengrid_highlight_mode_set(m_gengrid, EINA_TRUE);
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
    elm_scroller_page_size_set(m_gengrid, 0, 580);

    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
    elm_gengrid_item_size_set(m_gengrid, 580 * efl_scale, 580 * efl_scale);

    addItems();
}

void HistoryUI::showActionBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_actionBar = elm_layout_add(m_history_layout);
    elm_object_part_content_set(m_history_layout, "action_bar_history", m_actionBar);
    evas_object_size_hint_weight_set(m_actionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_actionBar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_layout_file_set(m_actionBar, m_edjFilePath.c_str(), "action_bar");

    Evas_Object *button = elm_button_add(m_actionBar);
    elm_object_style_set(button, "history_button");
    evas_object_smart_callback_add(button, "clicked", HistoryUI::_clearHistory_clicked, this);
    elm_object_part_content_set(m_actionBar, "clearhistory_click", button);

    button = elm_button_add(m_actionBar);
    elm_object_style_set(button, "history_button");
    evas_object_smart_callback_add(button, "clicked", HistoryUI::_close_clicked_cb, this);
    elm_object_part_content_set(m_actionBar, "close_click", button);

    evas_object_show(m_actionBar);
}

void HistoryUI::_close_clicked_cb(void * data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        HistoryUI *historyUI = static_cast<HistoryUI*>(data);
        historyUI->closeHistoryUIClicked(std::string());
        historyUI->clearItems();
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
    historyUI->clearHistoryClicked(std::string());
    historyUI->clearItems();
}

void HistoryUI::addItems()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    for (size_t i = 0; i < 1; i++) {
        HistoryItemData *itemData = new HistoryItemData();
        itemData->historyUI = std::shared_ptr<tizen_browser::base_ui::HistoryUI>(this);
        Elm_Object_Item* historyView = elm_gengrid_item_append(m_gengrid, m_item_class, itemData, nullptr, this);
        elm_gengrid_item_selected_set(historyView, EINA_FALSE);
    }
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
}

void HistoryUI::addHistoryItem(std::shared_ptr<tizen_browser::services::HistoryItem> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    HistoryItemData *itemData = new HistoryItemData();
    itemData->item = hi;
    itemData->historyUI = std::shared_ptr<tizen_browser::base_ui::HistoryUI>(this);
    _history_item_data.push_back(itemData);
    setEmptyGengrid(false);
}

void HistoryUI::addHistoryItems(std::vector<std::shared_ptr<tizen_browser::services::HistoryItem> > items)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    _history_item_data.clear();
    for (auto it = items.begin(); it != items.end(); ++it)
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
            id->historyUI->m_genListToday = elm_genlist_add(id->historyUI->m_history_layout);

            elm_genlist_homogeneous_set(id->historyUI->m_genListToday, EINA_FALSE);
            elm_genlist_multi_select_set(id->historyUI->m_genListToday, EINA_FALSE);
            elm_genlist_select_mode_set(id->historyUI->m_genListToday, ELM_OBJECT_SELECT_MODE_ALWAYS);
            elm_genlist_mode_set(id->historyUI->m_genListToday, ELM_LIST_LIMIT);
            elm_genlist_decorate_mode_set(id->historyUI->m_genListToday, EINA_TRUE);
            evas_object_size_hint_weight_set(id->historyUI->m_genListToday, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

            id->historyUI->m_itemClassToday = elm_genlist_item_class_new();
            id->historyUI->m_itemClassToday->item_style = "history_url_items";
            id->historyUI->m_itemClassToday->func.text_get =  &_listTodayTextGet;
            id->historyUI->m_itemClassToday->func.content_get = nullptr;
            id->historyUI->m_itemClassToday->func.state_get = nullptr;
            id->historyUI->m_itemClassToday->func.del = nullptr;

            for(auto it = _history_item_data.begin(); it != _history_item_data.end(); it++) {
                Elm_Object_Item* historyView = elm_genlist_item_append(id->historyUI->m_genListToday, id->historyUI->m_itemClassToday, *it, nullptr, ELM_GENLIST_ITEM_NONE, _history_item_clicked_cb, (*it));
                id->historyUI->m_map_history_views.insert(std::pair<std::string,Elm_Object_Item*>((*it)->item->getUrl(), historyView));
            }

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

    setEmptyGengrid(0 == m_map_history_views.size());
}

Evas_Object* HistoryUI::createNoHistoryLabel()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object *label = elm_label_add(m_parent);
    elm_object_text_set(label, "No favorite websites.");
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    return label;
}

void HistoryUI::setEmptyGengrid(bool setEmpty)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object *obj = (setEmpty ? createNoHistoryLabel() : nullptr);
    elm_object_part_content_set(m_gengrid, "elm.swallow.empty", obj);
}

void HistoryUI::hide()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(elm_layout_content_get(m_history_layout, "m_genListToday"));
    evas_object_hide(elm_layout_content_get(m_history_layout, "m_gengrid"));
    evas_object_hide(elm_layout_content_get(m_history_layout, "m_ActionBar"));
    evas_object_hide(m_history_layout);
}

void HistoryUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    hide();
    elm_genlist_clear(m_genListToday);
    elm_gengrid_clear(m_gengrid);
    m_map_history_views.clear();
    _history_item_data.clear();
    setEmptyGengrid(true);
}

void HistoryUI::_history_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
    itemData->historyUI->historyItemClicked(itemData->item);
}

}
}
