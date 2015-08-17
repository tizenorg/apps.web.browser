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
    showSettingsGenlist();
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

void SettingsUI::showSettingsGenlist()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_genList = elm_genlist_add(m_settings_layout);
    elm_object_part_content_set(m_settings_layout, "settings_genlist_swallow", m_genList);
    elm_genlist_homogeneous_set(m_genList, EINA_FALSE);
    elm_genlist_multi_select_set(m_genList, EINA_FALSE);
    elm_genlist_select_mode_set(m_genList, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genList, ELM_LIST_SCROLL);
    elm_genlist_decorate_mode_set(m_genList, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_scroller_policy_set(m_genList, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_AUTO);

    m_item_class = elm_genlist_item_class_new();
    m_item_class->item_style = "settings_items";
    m_item_class->func.text_get = nullptr;
    m_item_class->func.content_get = &listSettingsGenlistContentGet;
    m_item_class->func.state_get = nullptr;
    m_item_class->func.del = nullptr;

    ItemData *id = new ItemData;
    id->settingsUI = this;
    Elm_Object_Item *elmItem = elm_genlist_item_append(m_genList,             //genlist
                                                       m_item_class,          //item Class
                                                       id,
                                                       nullptr,               //parent item
                                                       ELM_GENLIST_ITEM_NONE, //item type
                                                       nullptr,
                                                       nullptr                //data passed to above function
                                                      );
    id->e_item = elmItem;

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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

Evas_Object* SettingsUI::listSettingsGenlistContentGet(void* data, Evas_Object* obj , const char* part)
{
    BROWSER_LOGD("[%s:%d] Part %s", __PRETTY_FUNCTION__, __LINE__, part);
    if (obj && part) {
        const char * part_name1  = "del_selected_data_click";
        static const int part_name1_len = strlen(part_name1);
        if (!strncmp(part_name1, part, part_name1_len)) {
            Evas_Object *button = elm_button_add(obj);
            elm_object_style_set(button, "basic_button");
            evas_object_smart_callback_add(button, "clicked", SettingsUI::_del_selected_data_clicked_cb, data);
            return button;
        }

        const char * part_name2  = "reset_mv_click";
        static const int part_name2_len = strlen(part_name2);
        if (!strncmp(part_name2, part, part_name2_len)) {
            Evas_Object *button = elm_button_add(obj);
            elm_object_style_set(button, "basic_button");
            evas_object_smart_callback_add(button, "clicked", SettingsUI::_reset_mv_clicked_cb, data);
            return button;
        }

        const char * part_name3  = "reset_browser_click";
        static const int part_name3_len = strlen(part_name3);
        if (!strncmp(part_name3, part, part_name3_len)) {
            Evas_Object *button = elm_button_add(obj);
            elm_object_style_set(button, "basic_button");
            evas_object_smart_callback_add(button, "clicked", SettingsUI::_reset_browser_clicked_cb, data);
            return button;
        }

        const char * part_name4  = "cache_cb";
        static const int part_name4_len = strlen(part_name4);
        const char * part_name5  = "cookies_cb";
        static const int part_name5_len = strlen(part_name5);
        const char * part_name6  = "history_cb";
        static const int part_name6_len = strlen(part_name6);
        if (!strncmp(part_name4, part, part_name4_len) ||
            !strncmp(part_name5, part, part_name5_len) ||
            !strncmp(part_name6, part, part_name6_len))
        {
            Evas_Object *checkbox = elm_check_add(obj);
            elm_object_style_set(checkbox, "on&off");
            evas_object_smart_callback_add(checkbox, "changed", __check_changed_cb, data);
            elm_check_state_set(checkbox, EINA_TRUE);
            return checkbox;
        }

        const char * part_name7  = "accept_all_rb";
        static const int part_name7_len = strlen(part_name7);
        const char * part_name8  = "ask_rb";
        static const int part_name8_len = strlen(part_name8);
        const char * part_name9  = "sr_disable_rb";
        static const int part_name9_len = strlen(part_name9);
        const char * part_name10 = "bs_enable_rb";
        static const int part_name10_len = strlen(part_name10);
        const char * part_name11 = "bs_disable_rb";
        static const int part_name11_len = strlen(part_name11);
        const char * part_name12 = "ts_enable_rb";
        static const int part_name12_len = strlen(part_name12);
        const char * part_name13 = "ts_disable_rb";
        static const int part_name13_len = strlen(part_name13);
        if (!strncmp(part_name7, part, part_name7_len) ||
            !strncmp(part_name8, part, part_name8_len) ||
            !strncmp(part_name9, part, part_name9_len) ||
            !strncmp(part_name10, part, part_name10_len) ||
            !strncmp(part_name11, part, part_name11_len) ||
            !strncmp(part_name12, part, part_name12_len) ||
            !strncmp(part_name13, part, part_name13_len))
        {
            Evas_Object *rb = elm_radio_add(obj);
            elm_object_style_set(rb, "on&off");
            //evas_object_smart_callback_add(rb, "changed", __radio_changed_cb, data);
            return rb;
        }
    }
    return nullptr;
}

void SettingsUI::__check_changed_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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
    evas_object_hide(elm_layout_content_get(m_settings_layout, "settings_genlist_swallow"));
    evas_object_hide(m_settings_layout);
}

void SettingsUI::_del_selected_data_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData *itemData = static_cast<ItemData*>(data);
        Evas_Object *cb1 = elm_object_item_part_content_get(itemData->e_item, "cache_cb");
        Evas_Object *cb2 = elm_object_item_part_content_get(itemData->e_item, "cookies_cb");
        Evas_Object *cb3 = elm_object_item_part_content_get(itemData->e_item, "history_cb");
        std::string type;
        elm_check_state_get(cb1) ? type += "_CACHE" : "";
        elm_check_state_get(cb2) ? type += "_COOKIES" : "";
        elm_check_state_get(cb3) ? type += "_HISTORY" : "";
        itemData->settingsUI->deleteSelectedDataClicked(type);
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
    elm_genlist_clear(m_genList);
}

}
}
