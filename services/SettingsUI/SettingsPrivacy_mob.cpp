
#include "SettingsPrivacy_mob.h"

namespace tizen_browser{
namespace base_ui{


SettingsPrivacy::SettingsPrivacy(Evas_Object* parent){
    init(parent);
    updateButtonMap();
};

SettingsPrivacy::~SettingsPrivacy(){
}

void SettingsPrivacy::updateButtonMap() {
    ItemData cookies;
    cookies.buttonText = "Accept cookies";
    cookies.subText = "Allow sites to save and read cookies.";
    cookies.sui = this;

    ItemData suggestions;
    suggestions.buttonText = "Suggest searches";
    suggestions.subText = "Set the device to suggest queries and sites in the web address bar as you type.";
    suggestions.sui = this;

    ItemData signIn;
    signIn.buttonText = "Save sign-in info";
    signIn.subText = " Set your device to show a pop-up with the option to save your username and password when you enter user credentials for websites.";
    signIn.sui = this;

    ItemData delPerData;
    delPerData.buttonText = "Delete personal data";
    delPerData.sui = this;

    m_buttonsMap[SettingsPrivacyOptions::COOKIES]=cookies;
    m_buttonsMap[SettingsPrivacyOptions::SUGGESTIONS]=suggestions;
    m_buttonsMap[SettingsPrivacyOptions::SIGNIN_INFO]=signIn;
    m_buttonsMap[SettingsPrivacyOptions::DEL_PER_DATA]=delPerData;
}

bool SettingsPrivacy::populateList(Evas_Object* genlist){
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Privacy");

    appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsPrivacyOptions::COOKIES], _cookies_cb);
    appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsPrivacyOptions::SUGGESTIONS], _suggestions_cb);
    appendGenlist(genlist, m_setting_check_on_of_item_class, &m_buttonsMap[SettingsPrivacyOptions::SIGNIN_INFO], _signin_cb);
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[SettingsPrivacyOptions::DEL_PER_DATA], _del_per_data_cb);
    return true;
}

void SettingsPrivacy::_cookies_cb(void */*data*/, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsPrivacy::_suggestions_cb(void */*data*/, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsPrivacy::_signin_cb(void */*data*/, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsPrivacy::_del_per_data_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<SettingsPrivacy*>(data);
    self->settingsDelPersDataClicked();
}

}
}
