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
#include <stdio.h>
#include <vector>
#include <AbstractMainWindow.h>

#include "SettingsUI_mob.h"
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
    , m_subpage_layout(nullptr)
    , m_items_layout(nullptr)
    , m_parent(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/SettingsMobileUI.edj");
}

SettingsUI::~SettingsUI()
{

}

void SettingsUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* SettingsUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_settings_layout)
        m_settings_layout = createSettingsUILayout(m_parent);
    return m_settings_layout;
}

void SettingsUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_show(m_settings_layout);
    evas_object_show(m_actionBar);
}

void SettingsUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(m_settings_layout);
    evas_object_hide(m_actionBar);
}

Evas_Object* SettingsUI::createSettingsUILayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    Evas_Object* settings_layout = elm_layout_add(parent);
    elm_layout_file_set(settings_layout, m_edjFilePath.c_str(), "settings-layout");
    evas_object_size_hint_weight_set(settings_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(settings_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_actionBar = createActionBar(settings_layout);
    m_items_layout = createSettingsMobilePage(settings_layout);
    elm_object_tree_focus_allow_set(settings_layout, EINA_FALSE);

    return settings_layout;
}

Evas_Object* SettingsUI::createActionBar(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object* actionBar = elm_layout_add(settings_layout);
    elm_object_part_content_set(settings_layout, "actionbar_swallow", actionBar);
    evas_object_size_hint_weight_set(actionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(actionBar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_layout_file_set(actionBar, m_edjFilePath.c_str(), "action_bar");
    Evas_Object *close_click_btn = elm_button_add(actionBar);
    elm_object_style_set(close_click_btn, "basic_button");
    evas_object_smart_callback_add(close_click_btn, "clicked", SettingsUI::close_clicked_cb, this);
    elm_object_part_content_set(actionBar, "close_click", close_click_btn);

    return actionBar;
}

Evas_Object* SettingsUI::createBackActionBar(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object* actionBar = elm_layout_add(settings_layout);
    elm_object_part_content_set(settings_layout, "actionbar_swallow", actionBar);
    evas_object_size_hint_weight_set(actionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(actionBar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_layout_file_set(actionBar, m_edjFilePath.c_str(), "action_bar");
    Evas_Object *close_click_btn = elm_button_add(actionBar);
    elm_object_style_set(close_click_btn, "basic_button");
    evas_object_smart_callback_add(close_click_btn, "clicked", SettingsUI::back_clicked_cb, this);
    elm_object_part_content_set(actionBar, "close_click", close_click_btn);

    return actionBar;
}

Evas_Object* SettingsUI::createSettingsMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    ItemData *id = new ItemData;
    id->settingsUI = this;

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "settings_items");
    elm_object_part_content_set(settings_layout, "settings_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object *del_selected_data_button = elm_button_add(layout);
    elm_object_style_set(del_selected_data_button, "basic_button");
    evas_object_smart_callback_add(del_selected_data_button, "clicked", _del_selected_data_menu_clicked_cb, (void*)id);
    elm_layout_content_set(layout, "del_web_bro_click", del_selected_data_button);

    Evas_Object *reset_mv_button = elm_button_add(layout);
    elm_object_style_set(reset_mv_button, "basic_button");
    evas_object_smart_callback_add(reset_mv_button, "clicked", _reset_mv_menu_clicked_cb, (void*)id);
    elm_layout_content_set(layout, "reset_most_visited_click", reset_mv_button);

    Evas_Object *reset_browser_button = elm_button_add(layout);
    elm_object_style_set(reset_browser_button, "basic_button");
    evas_object_smart_callback_add(reset_browser_button, "clicked", _reset_browser_menu_clicked_cb, (void*)id);
    elm_layout_content_set(layout, "reset_browser_click", reset_browser_button);


    return layout;
}

Evas_Object* SettingsUI::createDelDataMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    ItemData *id = new ItemData;
    id->settingsUI = this;

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "delete_browsing_data_mobile");
    elm_object_part_content_set(settings_layout, "settings_subpage_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_actionBar,"switch,delete,web,data", "del_but");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object* edje = elm_layout_edje_get(layout);
    Evas_Object* cache_checkbox = elm_check_add(layout);
    Evas_Object* cookies_checkbox = elm_check_add(layout);
    Evas_Object* history_checkbox = elm_check_add(layout);

    elm_object_style_set(cache_checkbox, "custom_check");
    elm_layout_content_set(layout, "cache_cb", cache_checkbox);
    elm_check_state_set(cache_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(edje, "mouse,clicked,1", "cache_cb_text_bg", __checkbox_label_click_cb, (void*)id);

    elm_object_style_set(cookies_checkbox, "custom_check");
    elm_layout_content_set(layout, "cookies_cb", cookies_checkbox);
    elm_check_state_set(cookies_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(edje, "mouse,clicked,1", "cookies_cb_text_bg", __checkbox_label_click_cb, (void*)id);

    elm_object_style_set(history_checkbox, "custom_check");
    elm_layout_content_set(layout, "history_cb", history_checkbox);
    elm_check_state_set(history_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(edje, "mouse,clicked,1", "history_cb_text_bg", __checkbox_label_click_cb, (void*)id);

    Evas_Object* password_checkbox = elm_check_add(layout);
    elm_object_style_set(password_checkbox, "custom_check");
    elm_layout_content_set(layout, "password_cb", password_checkbox);
    elm_check_state_set(password_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(elm_layout_edje_get(layout), "mouse,clicked,1", "password_cb_text_bg", __checkbox_label_click_cb, (void*)id);

    Evas_Object* formdata_checkbox = elm_check_add(layout);
    elm_object_style_set(formdata_checkbox, "custom_check");
    elm_layout_content_set(layout, "formdata_cb", formdata_checkbox);
    elm_check_state_set(formdata_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(elm_layout_edje_get(layout), "mouse,clicked,1", "formdata_cb_text_bg", __checkbox_label_click_cb, (void*)id);

    Evas_Object *del_selected_data_button = elm_button_add(layout);
    elm_object_style_set(del_selected_data_button, "basic_button");
    evas_object_smart_callback_add(del_selected_data_button, "clicked", _del_selected_data_clicked_cb, (void*)id);
    elm_layout_content_set(layout, "del_selected_data_click", del_selected_data_button);

    return layout;
}

Evas_Object* SettingsUI::createRemoveMostVisitedMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    ItemData *id = new ItemData;
    id->settingsUI = this;

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "reset_most_visited");
    elm_object_part_content_set(settings_layout, "settings_subpage_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_actionBar,"switch,reset,most,visited", "del_but");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object *reset_mv_button = elm_button_add(layout);
    elm_object_style_set(reset_mv_button, "basic_button");
    evas_object_smart_callback_add(reset_mv_button, "clicked", _reset_mv_clicked_cb, (void*)id);
    elm_layout_content_set(layout, "reset_most_visited_click", reset_mv_button);

    return layout;
}

Evas_Object* SettingsUI::createRemoveBrowserDataMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    ItemData *id = new ItemData;
    id->settingsUI = this;

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "reset_browser");
    elm_object_part_content_set(settings_layout, "settings_subpage_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_actionBar,"switch,reset,browser", "del_but");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object *reset_browser_button = elm_button_add(layout);
    elm_object_style_set(reset_browser_button, "basic_button");
    evas_object_smart_callback_add(reset_browser_button, "clicked", _reset_browser_clicked_cb, (void*)id);
    elm_layout_content_set(layout, "reset_browser_click", reset_browser_button);

    return layout;
}

void SettingsUI::__checkbox_label_click_cb(void *data, Evas_Object*, const char*, const char *source)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData *id = static_cast<ItemData*>(data);

        if(strcmp(source, "cache_cb_text_bg") == 0 ){
            Evas_Object *cache_check = elm_layout_content_get(id->settingsUI->m_subpage_layout, "cache_cb");
            elm_check_state_set(cache_check, !elm_check_state_get(cache_check));
        }
        else if (strcmp(source, "cookies_cb_text_bg") == 0 ){
            Evas_Object *cookies_check = elm_layout_content_get(id->settingsUI->m_subpage_layout, "cookies_cb");
            elm_check_state_set(cookies_check, !elm_check_state_get(cookies_check));
        }
        else if (strcmp(source, "history_cb_text_bg") == 0 ){
            Evas_Object *history_check = elm_layout_content_get(id->settingsUI->m_subpage_layout, "history_cb");
            elm_check_state_set(history_check, !elm_check_state_get(history_check));
        }
        else if (strcmp(source, "password_cb_text_bg") == 0 ){
            Evas_Object *password_check = elm_layout_content_get(id->settingsUI->m_subpage_layout, "password_cb");
            elm_check_state_set(password_check, !elm_check_state_get(password_check));
        }
        else if (strcmp(source, "formdata_cb_text_bg") == 0 ){
            Evas_Object *formdata_check = elm_layout_content_get(id->settingsUI->m_subpage_layout, "formdata_cb");
            elm_check_state_set(formdata_check, !elm_check_state_get(formdata_check));
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
        SettingsUI * s_ui = static_cast<SettingsUI*>(data);
        s_ui->closeSettingsUIClicked();
    }
}

void SettingsUI::back_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI * s_ui = static_cast<SettingsUI*>(data);
        s_ui->resetItemsLayoutContent();
        s_ui->m_actionBar = s_ui->createActionBar(s_ui->m_settings_layout);
        s_ui->m_items_layout = s_ui->createSettingsMobilePage(s_ui->m_settings_layout);
    }
}

void SettingsUI::_del_selected_data_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData *id = static_cast<ItemData*>(data);
        Evas_Object *cache_check = elm_layout_content_get(id->settingsUI->m_subpage_layout, "cache_cb");
        Evas_Object *cookies_check = elm_layout_content_get(id->settingsUI->m_subpage_layout, "cookies_cb");
        Evas_Object *history_check = elm_layout_content_get(id->settingsUI->m_subpage_layout, "history_cb");
        Evas_Object *password_check = elm_layout_content_get(id->settingsUI->m_subpage_layout, "password_cb");
        Evas_Object *formdata_check = elm_layout_content_get(id->settingsUI->m_subpage_layout, "formdata_cb");
        std::string type;
        elm_check_state_get(cache_check) ? type += "_CACHE" : "";
        elm_check_state_get(cookies_check) ? type += "_COOKIES" : "";
        elm_check_state_get(history_check) ? type += "_HISTORY" : "";
        elm_check_state_get(password_check) ? type += "_PASSWORD" : "";
        elm_check_state_get(formdata_check) ? type += "_FORMDATA" : "";

        id->settingsUI->deleteSelectedDataClicked(type);
    }
}

void SettingsUI::_del_selected_data_menu_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData *id = static_cast<ItemData*>(data);
        id->settingsUI->resetItemsLayoutContent();
        id->settingsUI->m_actionBar = id->settingsUI->createBackActionBar(id->settingsUI->m_settings_layout);
        id->settingsUI->m_subpage_layout = id->settingsUI->createDelDataMobilePage(id->settingsUI->m_settings_layout);
    }
}

void SettingsUI::_reset_mv_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* itemData = static_cast<ItemData*>(data);
        itemData->settingsUI->resetMostVisitedClicked();
    }
}

void SettingsUI::_reset_mv_menu_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* id = static_cast<ItemData*>(data);
        id->settingsUI->resetItemsLayoutContent();
        id->settingsUI->m_actionBar = id->settingsUI->createBackActionBar(id->settingsUI->m_settings_layout);
        id->settingsUI->m_subpage_layout = id->settingsUI->createRemoveMostVisitedMobilePage(id->settingsUI->m_settings_layout);
    }
}

void SettingsUI::_reset_browser_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* itemData = static_cast<ItemData*>(data);
        itemData->settingsUI->resetBrowserClicked();
    }
}

void SettingsUI::_reset_browser_menu_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* id = static_cast<ItemData*>(data);
        id->settingsUI->resetItemsLayoutContent();
        id->settingsUI->m_actionBar = id->settingsUI->createBackActionBar(id->settingsUI->m_settings_layout);
        id->settingsUI->m_subpage_layout = id->settingsUI->createRemoveBrowserDataMobilePage(id->settingsUI->m_settings_layout);
    }
}

void SettingsUI::resetItemsLayoutContent(){
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_part_content_unset(this->m_settings_layout, "settings_swallow");
    evas_object_del(this->m_actionBar);
    evas_object_del(this->m_items_layout);
    evas_object_del(this->m_subpage_layout);
}

}
}
