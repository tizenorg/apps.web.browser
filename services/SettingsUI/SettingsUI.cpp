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
#include <string.h>
#include <vector>
#include <AbstractMainWindow.h>

#include "SettingsUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SettingsUI, "org.tizen.browser.settingsui")

struct ItemData {
    tizen_browser::base_ui::SettingsUI* settingsUI;
    Elm_Object_Item * e_item;
};

SettingsUI::SettingsUI()
    : m_settings_layout(nullptr)
    , m_genListActionBar(nullptr)
    , m_itemClassActionBar(nullptr)
    , m_parent(nullptr)
    , m_item_class(nullptr)
    , m_scroller(nullptr)
    , m_items_layout(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/SettingsUI.edj");
}

SettingsUI::~SettingsUI()
{

}

void SettingsUI::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_settings_layout = elm_layout_add(parent);
    elm_layout_file_set(m_settings_layout, m_edjFilePath.c_str(), "settings-layout");
    evas_object_size_hint_weight_set(m_settings_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_settings_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_settings_layout);

    showActionBar();
    showSettingsPage();
}

void SettingsUI::showActionBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_genListActionBar = elm_genlist_add(m_settings_layout);
    elm_object_part_content_set(m_settings_layout, "actionbar_swallow", m_genListActionBar);
    elm_genlist_homogeneous_set(m_genListActionBar, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListActionBar, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListActionBar, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListActionBar, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genListActionBar, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genListActionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    m_itemClassActionBar = elm_genlist_item_class_new();
    m_itemClassActionBar->item_style = "settings_action_bar_items";
    m_itemClassActionBar->func.text_get = nullptr;
    m_itemClassActionBar->func.content_get = &listActionBarContentGet;
    m_itemClassActionBar->func.state_get = nullptr;
    m_itemClassActionBar->func.del = nullptr;

    ItemData *id = new ItemData;
    id->settingsUI = this;
    Elm_Object_Item *elmItem = elm_genlist_item_append(m_genListActionBar,    //genlist
                                                       m_itemClassActionBar,  //item Class
                                                       id,
                                                       nullptr,               //parent item
                                                       ELM_GENLIST_ITEM_NONE, //item type
                                                       nullptr,
                                                       nullptr                //data passed to above function
                                                      );
    id->e_item = elmItem;

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsUI::showSettingsPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    ItemData *id = new ItemData;
    id->settingsUI = this;

    m_scroller = elm_scroller_add(m_settings_layout);
    m_items_layout = elm_layout_add(m_scroller);
    elm_object_content_set(m_scroller, m_items_layout);
    elm_layout_file_set(m_items_layout, m_edjFilePath.c_str(), "settings_items");
    elm_object_part_content_set(m_settings_layout, "settings_scroller_swallow", m_scroller);
    evas_object_size_hint_weight_set(m_scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_scroller_policy_set(m_items_layout, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_AUTO);
    elm_scroller_bounce_set(m_scroller, EINA_TRUE, EINA_FALSE);
    elm_scroller_propagate_events_set(m_scroller, EINA_TRUE);
    evas_object_show(m_scroller);
    evas_object_show(m_items_layout);

    Evas_Object *del_selected_data_button = elm_button_add(m_items_layout);
    elm_object_style_set(del_selected_data_button, "basic_button");
    evas_object_smart_callback_add(del_selected_data_button, "clicked", _del_selected_data_clicked_cb, (void*)id);
    elm_layout_content_set(m_items_layout, "del_selected_data_click", del_selected_data_button);

    Evas_Object *reset_mv_button = elm_button_add(m_items_layout);
    elm_object_style_set(reset_mv_button, "basic_button");
    evas_object_smart_callback_add(reset_mv_button, "clicked", _reset_mv_clicked_cb, (void*)id);
    elm_layout_content_set(m_items_layout, "reset_mv_click", reset_mv_button);

    Evas_Object *reset_browser_button = elm_button_add(m_items_layout);
    elm_object_style_set(reset_browser_button, "basic_button");
    evas_object_smart_callback_add(reset_browser_button, "clicked", _reset_browser_clicked_cb, (void*)id);
    elm_layout_content_set(m_items_layout, "reset_browser_click", reset_browser_button);


    Evas_Object *cache_checkbox = elm_check_add(m_items_layout);
    elm_layout_content_set(m_items_layout, "cache_cb", cache_checkbox);
    elm_check_state_set(cache_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(elm_layout_edje_get(m_items_layout), "mouse,clicked,1", "cache_cb_text", __checkbox_label_click_cb, (void*)id);

    Evas_Object *cookies_checkbox = elm_check_add(m_items_layout);
    elm_layout_content_set(m_items_layout, "cookies_cb", cookies_checkbox);
    elm_check_state_set(cookies_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(elm_layout_edje_get(m_items_layout), "mouse,clicked,1", "cookies_cb_text", __checkbox_label_click_cb, (void*)id);

    Evas_Object *history_checkbox = elm_check_add(m_items_layout);
    elm_layout_content_set(m_items_layout, "history_cb", history_checkbox);
    elm_check_state_set(history_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(elm_layout_edje_get(m_items_layout), "mouse,clicked,1", "history_cb_text", __checkbox_label_click_cb, (void*)id);


    Evas_Object *accept_all_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(accept_all_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "accept_all_rb", accept_all_rb);

    Evas_Object *ask_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(ask_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "ask_rb", ask_rb);

    Evas_Object *sr_disable_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(sr_disable_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "sr_disable_rb", sr_disable_rb);

    Evas_Object *bs_enable_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(bs_enable_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "bs_enable_rb", bs_enable_rb);

    Evas_Object *bs_disable_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(bs_disable_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "bs_disable_rb", bs_disable_rb);

    Evas_Object *ts_enable_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(ts_enable_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "ts_enable_rb", ts_enable_rb);

    Evas_Object *ts_disable_rb = elm_radio_add(m_items_layout);
    elm_object_style_set(ts_disable_rb, "settings_radio");
    elm_layout_content_set(m_items_layout, "ts_disable_rb", ts_disable_rb);

}

Evas_Object* SettingsUI::listActionBarContentGet(void* data, Evas_Object* obj , const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (obj && part) {
        const char *part_name = "close_click";
        static const int part_name_len = strlen(part_name);
        if (!strncmp(part_name, part, part_name_len)) {
            Evas_Object *close_click = elm_button_add(obj);
            elm_object_style_set(close_click, "basic_button");
            evas_object_smart_callback_add(close_click, "clicked", SettingsUI::close_clicked_cb, data);
            return close_click;
        }
    }
    return nullptr;
}

void SettingsUI::__checkbox_label_click_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData *id = static_cast<ItemData*>(data);

        if(strcmp(source, "cache_cb_text") == 0 ){
            Evas_Object *cache_check = elm_layout_content_get(id->settingsUI->m_items_layout, "cache_cb");
            elm_check_state_set(cache_check, !elm_check_state_get(cache_check));
        }
        else if (strcmp(source, "cookies_cb_text") == 0 ){
            Evas_Object *cookies_check = elm_layout_content_get(id->settingsUI->m_items_layout, "cookies_cb");
            elm_check_state_set(cookies_check, !elm_check_state_get(cookies_check));
        }
        else if (strcmp(source, "history_cb_text") == 0 ){
            Evas_Object *history_check = elm_layout_content_get(id->settingsUI->m_items_layout, "history_cb");
            elm_check_state_set(history_check, !elm_check_state_get(history_check));
        }
        else{
            BROWSER_LOGD("[%s:%d] - no matched source", __PRETTY_FUNCTION__, __LINE__);
        }
    }
}

void SettingsUI::close_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData * id = static_cast<ItemData*>(data);
        id->settingsUI->closeSettingsUIClicked(std::string());
        id->settingsUI->clearItems();
    }
}

void SettingsUI::hide()
{
    evas_object_hide(elm_layout_content_get(m_settings_layout, "action_bar_swallow"));
    evas_object_hide(elm_layout_content_get(m_settings_layout, "settings_scroller_swallow"));
    evas_object_hide(m_settings_layout);
}

void SettingsUI::_del_selected_data_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData *id = static_cast<ItemData*>(data);
        Evas_Object *cache_check = elm_layout_content_get(id->settingsUI->m_items_layout, "cache_cb");
        Evas_Object *cookies_check = elm_layout_content_get(id->settingsUI->m_items_layout, "cookies_cb");
        Evas_Object *history_check = elm_layout_content_get(id->settingsUI->m_items_layout, "history_cb");
        std::string type;
        elm_check_state_get(cache_check) ? type += "_CACHE" : "";
        elm_check_state_get(cookies_check) ? type += "_COOKIES" : "";
        elm_check_state_get(history_check) ? type += "_HISTORY" : "";
        id->settingsUI->deleteSelectedDataClicked(type);
    }
}

void SettingsUI::_reset_mv_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* itemData = static_cast<ItemData*>(data);
        itemData->settingsUI->resetMostVisitedClicked(std::string());
    }
}

void SettingsUI::_reset_browser_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* itemData = static_cast<ItemData*>(data);
        itemData->settingsUI->resetBrowserClicked(std::string());
    }
}

void SettingsUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    hide();
    elm_genlist_clear(m_genListActionBar);
}

}
}
