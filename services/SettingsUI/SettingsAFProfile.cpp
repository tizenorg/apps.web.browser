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


#include "SettingsAFProfile.h"

namespace tizen_browser{
namespace base_ui{

SettingsAFProfile::SettingsAFProfile(Evas_Object* parent)
    : m_profile(nullptr)
{
    init(parent);
    updateButtonMap();
};

SettingsAFProfile::~SettingsAFProfile()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsAFProfile::updateButtonMap()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // TODO Missing translations
    ItemData profileName;
    m_profile = ewk_context_form_autofill_profile_get(ewk_context_default_get(), 1);
    if (!m_profile)
        profileName.buttonText = "Set my profile";
    else
        profileName.buttonText = ewk_autofill_profile_data_get(m_profile, EWK_PROFILE_NAME);
    m_buttonsMap[0] = profileName;
}

bool SettingsAFProfile::populateList(Evas_Object* genlist)
{
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "My Auto fill profile");
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[0], _select_profile_cb);
    return true;
}

void SettingsAFProfile::_select_profile_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    auto self = static_cast<SettingsAFProfile*>(data);
    if (self->m_profile)
        SettingsPrettySignalConnector::Instance().settingsAutofillProfileClicked(true);
    else
        SettingsPrettySignalConnector::Instance().settingsAutofillProfileClicked(false);
}
}
}
