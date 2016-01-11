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
#include "AutoFillForm/AutoFillFormManager.h"

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

enum SettingsOptions {
    DEL_WEB_BRO,
    RESET_MOST_VIS,
    RESET_BRO,
    AUTO_FILL,
    CONTENT,
    PRIVACY
};

EXPORT_SERVICE(SettingsUI, "org.tizen.browser.settingsui")

SettingsUI::SettingsUI()
    : m_settings_layout(nullptr)
    , m_subpage_layout(nullptr)
    , m_items_layout(nullptr)
    , m_parent(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/SettingsMobileUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_setting_item_class = elm_gengrid_item_class_new();
    m_setting_item_class->item_style = "settings_button";
    m_setting_item_class->func.text_get = _gengrid_item_text_get;
    m_setting_item_class->func.content_get = nullptr;
    m_setting_item_class->func.state_get = nullptr;
    m_setting_item_class->func.del = nullptr;

    initializeButtonMap();
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

void SettingsUI::initializeButtonMap() {
    ItemData deleteWebBrowsing;
    //TODO Add translation API
    deleteWebBrowsing.buttonText="Delete Web browsing data";

    ItemData resetMostVisited;
    resetMostVisited.buttonText="Reset most visited site";

    ItemData resetBrowser;
    resetBrowser.buttonText="Reset browser";

    ItemData autoFill;
    autoFill.buttonText="Auto Fill data";

    ItemData content;
    content.buttonText="Content Settings";

    ItemData privacy;
    privacy.buttonText="Privacy";

    m_buttonsMap[SettingsOptions::DEL_WEB_BRO]=deleteWebBrowsing;
    m_buttonsMap[SettingsOptions::RESET_MOST_VIS]=resetMostVisited;
    m_buttonsMap[SettingsOptions::RESET_BRO]=resetBrowser;
    m_buttonsMap[SettingsOptions::AUTO_FILL]=autoFill;
    m_buttonsMap[SettingsOptions::CONTENT]=content;
    m_buttonsMap[SettingsOptions::PRIVACY]=privacy;
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
    Evas_Object* settings_layout = elm_layout_add(parent);
    elm_layout_file_set(settings_layout, m_edjFilePath.c_str(), "settings-layout");
    evas_object_size_hint_weight_set(settings_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(settings_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_actionBar = createActionBar(settings_layout);
    m_items_layout = createSettingsMobilePage(settings_layout);
    elm_object_tree_focus_allow_set(settings_layout, EINA_FALSE);

    return settings_layout;
}

Evas_Object* SettingsUI::createPageLayout(Evas_Object* parent, const std::string& layoutFile)
{
    Evas_Object* layout = elm_layout_add(parent);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), layoutFile.c_str());
    elm_object_part_content_set(parent, "settings_subpage_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_actionBar,"switch,content,Settings", "del_but");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    return layout;
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

char* SettingsUI::_gengrid_item_text_get(void* data, Evas_Object* /*obj*/, const char* part)
{
   M_ASSERT(data);
   if(!data)
       return nullptr;

   ItemData* it = static_cast<ItemData*>(data);

   if (strcmp(part, "button_text") == 0) {
       //TODO Implement translation API
       const char* item_name = it->buttonText.c_str();
       if (item_name)
          return strdup(item_name);
   }
   return nullptr;
}

Evas_Object* SettingsUI::createSettingsMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "settings_items");
    elm_object_part_content_set(settings_layout, "settings_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object *sign_in_button = elm_button_add(layout);
    elm_object_style_set(sign_in_button, "sign_in_button");
    elm_layout_content_set(layout, "sign_in_click", sign_in_button);

    //TODO Add translation API
    elm_object_translatable_part_text_set(sign_in_button, "text", "Sign in");

    Evas_Object* scroller = elm_gengrid_add(layout);
    evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_align_set(scroller, 0, 0);
    elm_gengrid_select_mode_set(scroller, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(scroller, EINA_FALSE);
    elm_gengrid_horizontal_set(scroller, EINA_FALSE);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_ON);
    elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_FALSE);
    elm_gengrid_item_size_set(scroller, 720 * efl_scale, 120 * efl_scale);

    elm_gengrid_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::DEL_WEB_BRO], _del_selected_data_menu_clicked_cb, this);
    elm_gengrid_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::RESET_MOST_VIS], _reset_mv_menu_clicked_cb, this);
    elm_gengrid_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::RESET_BRO], _reset_browser_menu_clicked_cb, this);
    elm_gengrid_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::AUTO_FILL], _auto_fill_data_menu_clicked_cb, this);
    elm_gengrid_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::CONTENT], _content_settings_menu_clicked_cb, this);
    elm_gengrid_item_append(scroller, m_setting_item_class, &m_buttonsMap[SettingsOptions::PRIVACY], _privacy_menu_clicked_cb, this);

    elm_object_part_content_set(layout, "list_swallow", scroller);
    evas_object_show(scroller);

    return layout;
}

Evas_Object* SettingsUI::createDelDataMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

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
    edje_object_signal_callback_add(edje, "mouse,clicked,1", "cache_cb_text_bg", __checkbox_label_click_cb, this);

    elm_object_style_set(cookies_checkbox, "custom_check");
    elm_layout_content_set(layout, "cookies_cb", cookies_checkbox);
    elm_check_state_set(cookies_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(edje, "mouse,clicked,1", "cookies_cb_text_bg", __checkbox_label_click_cb, this);

    elm_object_style_set(history_checkbox, "custom_check");
    elm_layout_content_set(layout, "history_cb", history_checkbox);
    elm_check_state_set(history_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(edje, "mouse,clicked,1", "history_cb_text_bg", __checkbox_label_click_cb, this);

    Evas_Object* password_checkbox = elm_check_add(layout);
    elm_object_style_set(password_checkbox, "custom_check");
    elm_layout_content_set(layout, "password_cb", password_checkbox);
    elm_check_state_set(password_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(elm_layout_edje_get(layout), "mouse,clicked,1", "password_cb_text_bg", __checkbox_label_click_cb, this);

    Evas_Object* formdata_checkbox = elm_check_add(layout);
    elm_object_style_set(formdata_checkbox, "custom_check");
    elm_layout_content_set(layout, "formdata_cb", formdata_checkbox);
    elm_check_state_set(formdata_checkbox, EINA_TRUE);
    edje_object_signal_callback_add(elm_layout_edje_get(layout), "mouse,clicked,1", "formdata_cb_text_bg", __checkbox_label_click_cb, this);

    Evas_Object *del_selected_data_button = elm_button_add(layout);
    elm_object_style_set(del_selected_data_button, "basic_button");
    evas_object_smart_callback_add(del_selected_data_button, "clicked", _del_selected_data_clicked_cb, this);
    elm_layout_content_set(layout, "del_selected_data_click", del_selected_data_button);

    return layout;
}

Evas_Object* SettingsUI::createRemoveMostVisitedMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "reset_most_visited");
    elm_object_part_content_set(settings_layout, "settings_subpage_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_actionBar,"switch,reset,most,visited", "del_but");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object *reset_mv_button = elm_button_add(layout);
    elm_object_style_set(reset_mv_button, "basic_button");
    evas_object_smart_callback_add(reset_mv_button, "clicked", _reset_mv_clicked_cb, this);
    elm_layout_content_set(layout, "reset_most_visited_click", reset_mv_button);

    return layout;
}

Evas_Object* SettingsUI::createRemoveBrowserDataMobilePage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = elm_layout_add(settings_layout);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "reset_browser");
    elm_object_part_content_set(settings_layout, "settings_subpage_swallow", layout);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_actionBar,"switch,reset,browser", "del_but");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    Evas_Object *reset_browser_button = elm_button_add(layout);
    elm_object_style_set(reset_browser_button, "basic_button");
    evas_object_smart_callback_add(reset_browser_button, "clicked", _reset_browser_clicked_cb, this);
    elm_layout_content_set(layout, "reset_browser_click", reset_browser_button);

    return layout;
}

Evas_Object* SettingsUI::createCheckBox(Evas_Object* layout, const std::string name, Edje_Signal_Cb func, void* data)
{
    Evas_Object* edje = elm_layout_edje_get(layout);
    Evas_Object* checkbox = elm_check_add(layout);
    elm_object_style_set(checkbox, "custom_check");
    elm_layout_content_set(layout, (name + "_cb").c_str(), checkbox);
    edje_object_signal_callback_add(edje, "mouse,clicked,1", (name + "_cb_text_bg").c_str(), func, data);
    return checkbox;
}

void SettingsUI::__checkbox_label_click_cb(void *data, Evas_Object*, const char*, const char *source)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);

        if(strcmp(source, "cache_cb_text_bg") == 0 ){
            Evas_Object *cache_check = elm_layout_content_get(self->m_subpage_layout, "cache_cb");
            elm_check_state_set(cache_check, !elm_check_state_get(cache_check));
        }
        else if (strcmp(source, "cookies_cb_text_bg") == 0 ){
            Evas_Object *cookies_check = elm_layout_content_get(self->m_subpage_layout, "cookies_cb");
            elm_check_state_set(cookies_check, !elm_check_state_get(cookies_check));
        }
        else if (strcmp(source, "history_cb_text_bg") == 0 ){
            Evas_Object *history_check = elm_layout_content_get(self->m_subpage_layout, "history_cb");
            elm_check_state_set(history_check, !elm_check_state_get(history_check));
        }
        else if (strcmp(source, "password_cb_text_bg") == 0 ){
            Evas_Object *password_check = elm_layout_content_get(self->m_subpage_layout, "password_cb");
            elm_check_state_set(password_check, !elm_check_state_get(password_check));
        }
        else if (strcmp(source, "formdata_cb_text_bg") == 0 ){
            Evas_Object *formdata_check = elm_layout_content_get(self->m_subpage_layout, "formdata_cb");
            elm_check_state_set(formdata_check, !elm_check_state_get(formdata_check));
        }
        else{
            BROWSER_LOGD("[%s:%d] - no matched source", __PRETTY_FUNCTION__, __LINE__);
        }
    }
}

Evas_Object* SettingsUI::createContentSettingsPage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = createPageLayout(settings_layout, "content_settings_mobile");

    elm_object_signal_emit(m_actionBar,"switch,content,Settings", "del_but");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    boost::optional<bool> sig = getWebEngineSettingsParam(basic_webengine::WebEngineSettings::PAGE_OVERVIEW);
    Eina_Bool flag = (sig && *sig) ? EINA_TRUE : EINA_FALSE;
    Evas_Object* overview_checkbox = createCheckBox(layout, "overview", __checkbox_content_settings_label_click_cb, this);
    elm_check_state_set(overview_checkbox, flag);

    sig = getWebEngineSettingsParam(basic_webengine::WebEngineSettings::LOAD_IMAGES);
    flag = (sig && *sig) ? EINA_TRUE : EINA_FALSE;
    Evas_Object* images_checkbox = createCheckBox(layout, "images", __checkbox_content_settings_label_click_cb, this);
    elm_check_state_set(images_checkbox, flag);

    sig = getWebEngineSettingsParam(basic_webengine::WebEngineSettings::ENABLE_JAVASCRIPT);
    flag = (sig && *sig) ? EINA_TRUE : EINA_FALSE;
    Evas_Object* javascript_checkbox = createCheckBox(layout, "javascript", __checkbox_content_settings_label_click_cb, this);
    elm_check_state_set(javascript_checkbox, flag);
    return layout;
}

Evas_Object* SettingsUI::createPrivacyPage(Evas_Object* settings_layout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = createPageLayout(settings_layout, "privacy_mobile");

    elm_object_signal_emit(m_actionBar,"switch,privacy,Settings", "del_but");
    elm_object_focus_set(elm_object_part_content_get(m_actionBar, "close_click"), EINA_TRUE);

    boost::optional<bool> sig = getWebEngineSettingsParam(basic_webengine::WebEngineSettings::REMEMBER_FROM_DATA);
    Eina_Bool flag = (sig && *sig) ? EINA_TRUE : EINA_FALSE;
    Evas_Object* form_data_checkbox = createCheckBox(layout, "form_data", __checkbox_privacy_label_click_cb, this);
    elm_check_state_set(form_data_checkbox, flag);

    sig = getWebEngineSettingsParam(basic_webengine::WebEngineSettings::REMEMBER_PASSWORDS);
    flag = (sig && *sig) ? EINA_TRUE : EINA_FALSE;
    Evas_Object* passwd_checkbox = createCheckBox(layout, "passwords", __checkbox_privacy_label_click_cb, this);
    elm_check_state_set(passwd_checkbox, flag);

    return layout;
}

void SettingsUI::__checkbox_content_settings_label_click_cb(void* data, Evas_Object*, const char*, const char* source)
{
    if (data) {
        auto self = static_cast<SettingsUI*>(data);

        if(strcmp(source, "overview_cb_text_bg") == 0 ) {
            Evas_Object *checkbox = elm_layout_content_get(self->m_subpage_layout, "overview_cb");
            Eina_Bool value = !elm_check_state_get(checkbox);
            elm_check_state_set(checkbox, value);
            self->setWebEngineSettingsParam(basic_webengine::WebEngineSettings::PAGE_OVERVIEW, static_cast<bool>(value));
        } else if (strcmp(source, "images_cb_text_bg") == 0 ) {
            Evas_Object *checkbox = elm_layout_content_get(self->m_subpage_layout, "images_cb");
            Eina_Bool value = !elm_check_state_get(checkbox);
            elm_check_state_set(checkbox, value);
            self->setWebEngineSettingsParam(basic_webengine::WebEngineSettings::LOAD_IMAGES, static_cast<bool>(value));
        } else if (strcmp(source, "javascript_cb_text_bg") == 0 ) {
            Evas_Object *checkbox = elm_layout_content_get(self->m_subpage_layout, "javascript_cb");
            Eina_Bool value = !elm_check_state_get(checkbox);
            elm_check_state_set(checkbox, value);
            self->setWebEngineSettingsParam(basic_webengine::WebEngineSettings::ENABLE_JAVASCRIPT, static_cast<bool>(value));
        } else {
            BROWSER_LOGD("[%s:%d] - no matched source", __PRETTY_FUNCTION__, __LINE__);
        }
    } else {
        BROWSER_LOGD("[%s:%d] Warning no data specified!", __PRETTY_FUNCTION__, __LINE__);
    }
}

void SettingsUI::__checkbox_privacy_label_click_cb(void* data, Evas_Object*, const char*, const char* source)
{
    if (data) {
        auto self = static_cast<SettingsUI*>(data);

        if(strcmp(source, "form_data_cb_text_bg") == 0 ) {
            Evas_Object *checkbox = elm_layout_content_get(self->m_subpage_layout, "form_data_cb");
            Eina_Bool value = !elm_check_state_get(checkbox);
            elm_check_state_set(checkbox, value);
            self->setWebEngineSettingsParam(basic_webengine::WebEngineSettings::REMEMBER_FROM_DATA, static_cast<bool>(value));
        } else if (strcmp(source, "passwords_cb_text_bg") == 0 ) {
            Evas_Object *checkbox = elm_layout_content_get(self->m_subpage_layout, "passwords_cb");
            Eina_Bool value = !elm_check_state_get(checkbox);
            elm_check_state_set(checkbox, value);
            self->setWebEngineSettingsParam(basic_webengine::WebEngineSettings::REMEMBER_PASSWORDS, static_cast<bool>(value));
        } else {
            BROWSER_LOGD("[%s:%d] - no matched source", __PRETTY_FUNCTION__, __LINE__);
        }
    } else {
        BROWSER_LOGD("[%s:%d] Warning no data specified!", __PRETTY_FUNCTION__, __LINE__);
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
        s_ui->onBackKey();
    }
}

void SettingsUI::initializeAutoFillManager()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_autoFillManager) {
         m_autoFillManager = std::unique_ptr<AutoFillFormManager>(new AutoFillFormManager());
         m_autoFillManager->listViewBackClicked.connect(boost::bind(&SettingsUI::destroyAutoFillManager, this));
         m_autoFillManager->init(m_parent);
         m_autoFillManager->showListView();
    }
}

void SettingsUI::destroyAutoFillManager()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_autoFillManager.reset();
}

void SettingsUI::_auto_fill_data_menu_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->initializeAutoFillManager();
    }
}

void SettingsUI::_del_selected_data_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        Evas_Object *cache_check = elm_layout_content_get(self->m_subpage_layout, "cache_cb");
        Evas_Object *cookies_check = elm_layout_content_get(self->m_subpage_layout, "cookies_cb");
        Evas_Object *history_check = elm_layout_content_get(self->m_subpage_layout, "history_cb");
        Evas_Object *password_check = elm_layout_content_get(self->m_subpage_layout, "password_cb");
        Evas_Object *formdata_check = elm_layout_content_get(self->m_subpage_layout, "formdata_cb");
        std::string type;
        elm_check_state_get(cache_check) ? type += "_CACHE" : "";
        elm_check_state_get(cookies_check) ? type += "_COOKIES" : "";
        elm_check_state_get(history_check) ? type += "_HISTORY" : "";
        elm_check_state_get(password_check) ? type += "_PASSWORD" : "";
        elm_check_state_get(formdata_check) ? type += "_FORMDATA" : "";

        self->deleteSelectedDataClicked(type);
    }
}

void SettingsUI::_del_selected_data_menu_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetItemsLayoutContent();
        self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
        self->m_subpage_layout = self->createDelDataMobilePage(self->m_settings_layout);
    }
}

void SettingsUI::_reset_mv_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetMostVisitedClicked();
    }
}

void SettingsUI::_reset_mv_menu_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetItemsLayoutContent();
        self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
        self->m_subpage_layout = self->createRemoveMostVisitedMobilePage(self->m_settings_layout);
    }
}

void SettingsUI::_reset_browser_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetBrowserClicked();
    }
}

void SettingsUI::_reset_browser_menu_clicked_cb(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        SettingsUI* self = static_cast<SettingsUI*>(data);
        self->resetItemsLayoutContent();
        self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
        self->m_subpage_layout = self->createRemoveBrowserDataMobilePage(self->m_settings_layout);
    }
}

void SettingsUI::_content_settings_menu_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<SettingsUI*>(data);
    self->resetItemsLayoutContent();
    self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
    self->m_subpage_layout = self->createContentSettingsPage(self->m_settings_layout);
}

void SettingsUI::_privacy_menu_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<SettingsUI*>(data);
    self->resetItemsLayoutContent();
    self->m_actionBar = self->createBackActionBar(self->m_settings_layout);
    self->m_subpage_layout = self->createPrivacyPage(self->m_settings_layout);
}

void SettingsUI::resetItemsLayoutContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_part_content_unset(this->m_settings_layout, "settings_swallow");
    evas_object_del(this->m_actionBar);
    evas_object_del(this->m_items_layout);
    evas_object_del(this->m_subpage_layout);
    this->m_subpage_layout = nullptr;
}

bool SettingsUI::isSubpage()
{
    return (m_subpage_layout != nullptr);
}

void SettingsUI::onBackKey()
{
    resetItemsLayoutContent();
    m_actionBar = createActionBar(m_settings_layout);
    m_items_layout = createSettingsMobilePage(m_settings_layout);
}
}
}
