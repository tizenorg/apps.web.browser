/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include "SettingsAdvanced.h"

namespace tizen_browser{
namespace base_ui{

SettingsAdvanced::SettingsAdvanced(Evas_Object* parent)
{
    init(parent);
    updateButtonMap();
};

SettingsAdvanced::~SettingsAdvanced()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsAdvanced::updateButtonMap()
{
    // TODO Missing translations
    ItemData enable_js;
    enable_js.buttonText = "Enable JavaScript";
    enable_js.subText = "Allow sites to run JavaScript.";
    enable_js.sui = this;
    enable_js.id = ENABLE_JS;

    ItemData block_popups;
    block_popups.buttonText = "Block pop-ups";
    block_popups.subText = "Block pop-ups on webpages.";
    block_popups.sui = this;
    block_popups.id = BLOCK_POPUPS;

    ItemData save_content;
    save_content.buttonText = "Save content to";
    save_content.subText = "Device";
    save_content.sui = this;
    save_content.id = SAVE_CONTENT;

    ItemData manage_web_data;
    manage_web_data.buttonText = "Manage website data";
    manage_web_data.subText = "Set advanced settings for individual websites.";
    manage_web_data.sui = this;
    manage_web_data.id = MANAGE_WEB_DATA;

    m_buttonsMap[SettingsAdvancedOptions::ENABLE_JS] = enable_js;
    m_buttonsMap[SettingsAdvancedOptions::BLOCK_POPUPS] = block_popups;
    m_buttonsMap[SettingsAdvancedOptions::SAVE_CONTENT] = save_content;
    m_buttonsMap[SettingsAdvancedOptions::MANAGE_WEB_DATA] = manage_web_data;
}

bool SettingsAdvanced::populateList(Evas_Object* genlist)
{
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Advanced");

    appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsAdvancedOptions::ENABLE_JS], _enable_js_cb);
    appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsAdvancedOptions::BLOCK_POPUPS], _block_popups_cb);
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsAdvancedOptions::SAVE_CONTENT], _save_content_cb);
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsAdvancedOptions::MANAGE_WEB_DATA], _manage_web_data_cb);
    return true;
}

Evas_Object* SettingsAdvanced::createOnOffCheckBox(Evas_Object* obj, ItemData* itd)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto check = elm_check_add(obj);
    elm_object_style_set(check, "on&off");
    elm_check_state_set(check, getOriginalState(itd->id));
    evas_object_smart_callback_add(check, "changed", grid_item_check_changed, itd);
    return check;
}

Eina_Bool SettingsAdvanced::getOriginalState(int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> sig;
    switch (id){
        case ENABLE_JS:
            sig = SettingsPrettySignalConnector::Instance().getWebEngineSettingsParam(basic_webengine::WebEngineSettings::ENABLE_JAVASCRIPT);
            break;
        case BLOCK_POPUPS:
            sig = SettingsPrettySignalConnector::Instance().getWebEngineSettingsParam(basic_webengine::WebEngineSettings::SCRIPTS_CAN_OPEN_PAGES);
            break;
        default:
            sig = false;
            break;
    }
    return (sig && *sig) ? EINA_TRUE : EINA_FALSE;
}

void SettingsAdvanced::_enable_js_cb(void *, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsAdvanced::_block_popups_cb(void *, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsAdvanced::_save_content_cb(void *, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsAdvanced::_manage_web_data_cb(void *, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsAdvanced::grid_item_check_changed(void* data, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto itd = static_cast<ItemData*>(data);
    auto value = elm_check_state_get(obj);
    switch (itd->id){
        case ENABLE_JS:
            elm_check_state_set(obj, value);
            SettingsPrettySignalConnector::Instance().setWebEngineSettingsParam(basic_webengine::WebEngineSettings::ENABLE_JAVASCRIPT, static_cast<bool>(value));
            break;
        case BLOCK_POPUPS:
            elm_check_state_set(obj, value);
            SettingsPrettySignalConnector::Instance().setWebEngineSettingsParam(basic_webengine::WebEngineSettings::SCRIPTS_CAN_OPEN_PAGES, static_cast<bool>(value));
            break;
        default:
            break;
    }
}
}
}
