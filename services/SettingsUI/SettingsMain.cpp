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

#include "SettingsMain.h"

namespace tizen_browser{
namespace base_ui{

SettingsMain::SettingsMain(Evas_Object* parent)
{
    init(parent);
};

SettingsMain::~SettingsMain()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsMain::updateButtonMap()
{
    // TODO Missing translations
    ItemData homePage;
    homePage.buttonText = "Home Page";
    homePage.subText = "Default";
    homePage.sui = this;
    homePage.id = HOME;

    ItemData search;
    search.buttonText = "Default search engine";
    search.subText = "Google";
    search.sui = this;
    search.id = SEARCH;

    ItemData autofill;
    autofill.buttonText = "My Auto fill profile";
    autofill.subText = "Manage your Auto fill profile.";
    autofill.sui = this;
    autofill.id = AUTO_FILL_PROFILE;

    ItemData zoom;
    zoom.buttonText = "Manual zoom";
    zoom.subText = "Override website requests to control the zoom level.";
    zoom.sui = this;
    zoom.id = ZOOM;

    ItemData privacy;
    privacy.buttonText="Privacy";
    privacy.sui = this;
    privacy.id = PRIVACY;

    ItemData advanced;
    advanced.buttonText="Advanced";
    advanced.sui = this;
    advanced.id = ADVANCED;

    m_buttonsMap[SettingsMainOptions::HOME] = homePage;
    m_buttonsMap[SettingsMainOptions::SEARCH] = search;
    m_buttonsMap[SettingsMainOptions::AUTO_FILL_PROFILE] = autofill;
    m_buttonsMap[SettingsMainOptions::ZOOM] = zoom;
    m_buttonsMap[SettingsMainOptions::PRIVACY] = privacy;
    m_buttonsMap[SettingsMainOptions::ADVANCED] = advanced;
}

bool SettingsMain::populateList(Evas_Object* genlist)
{
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Settings");
    updateButtonMap();
    appendGenlist(genlist, m_setting_double_item_class, &m_buttonsMap[SettingsMainOptions::HOME], _home_page_cb);
    appendGenlist(genlist, m_setting_double_item_class, &m_buttonsMap[SettingsMainOptions::SEARCH], _search_engine_cb);
    appendGenlist(genlist, m_setting_double_item_class, &m_buttonsMap[SettingsMainOptions::AUTO_FILL_PROFILE], _auto_fill_cb);
    appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsMainOptions::ZOOM], _zoom_cb);
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsMainOptions::PRIVACY], _privacy_cb);
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsMainOptions::ADVANCED], _advanced_cb);
    return true;
}

Evas_Object* SettingsMain::createOnOffCheckBox(Evas_Object* obj, ItemData* itd)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto check = elm_check_add(obj);
    elm_object_style_set(check, "on&off");
    elm_check_state_set(check, getOriginalZoomState());
    evas_object_smart_callback_add(check, "changed", grid_item_check_changed, itd);
    return check;
}

Eina_Bool SettingsMain::getOriginalZoomState()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> sig =
        SettingsPrettySignalConnector::Instance().
            getWebEngineSettingsParam(basic_webengine::WebEngineSettings::PAGE_OVERVIEW);

    return (sig && *sig) ? EINA_TRUE : EINA_FALSE;
}

void SettingsMain::_home_page_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SettingsPrettySignalConnector::Instance().settingsHomePageClicked();
}

void SettingsMain::_search_engine_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SettingsPrettySignalConnector::Instance().settingsBaseShowRadioPopup();
}

void SettingsMain::_zoom_cb(void *, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto el = elm_genlist_selected_item_get(obj);
    auto check = elm_object_item_part_content_get(el, "elm.swallow.end");
    auto value = !elm_check_state_get(check);

    elm_check_state_set(check, value);
    SettingsPrettySignalConnector::Instance().
        setWebEngineSettingsParam(
            basic_webengine::WebEngineSettings::PAGE_OVERVIEW, static_cast<bool>(value));
}

void SettingsMain::_advanced_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SettingsPrettySignalConnector::Instance().settingsAdvancedClicked();
}

void SettingsMain::_auto_fill_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SettingsPrettySignalConnector::Instance().settingsAutofillClicked();
}

void SettingsMain::_privacy_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SettingsPrettySignalConnector::Instance().settingsPrivacyClicked();
}

void SettingsMain::grid_item_check_changed(void*, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    auto value = !elm_check_state_get(obj);

    elm_check_state_set(obj, value);
    SettingsPrettySignalConnector::Instance().
        setWebEngineSettingsParam(
            basic_webengine::WebEngineSettings::PAGE_OVERVIEW, static_cast<bool>(value));
}
}
}
