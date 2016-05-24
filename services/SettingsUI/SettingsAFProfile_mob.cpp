
#include "SettingsAFProfile_mob.h"

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
    ItemData test;
    m_profile = ewk_context_form_autofill_profile_get(ewk_context_default_get(), 1);
    if (!m_profile)
        test.buttonText = "Set my profile";
    else
        test.buttonText = ewk_autofill_profile_data_get(m_profile, EWK_PROFILE_NAME);
    m_buttonsMap[0] = test;
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
    if (!data)
        BROWSER_LOGD("Data does not exist");
    auto self = static_cast<SettingsAFProfile*>(data);
    if (self->m_profile)
        self->settingsAutofillProfileClicked(true);
    else
        self->settingsAutofillProfileClicked(false);
}
}
}
