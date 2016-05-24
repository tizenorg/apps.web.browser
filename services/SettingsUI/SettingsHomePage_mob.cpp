
#include "SettingsHomePage_mob.h"

namespace tizen_browser{
namespace base_ui{

const std::string SettingsHomePage::DEF_HOME_PAGE = "http://www.samsung.com";

SettingsHomePage::SettingsHomePage(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    init(parent);
};

SettingsHomePage::~SettingsHomePage(){
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

std::string SettingsHomePage::getCurrentPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_current = requestCurrentPage();
    BROWSER_LOGD("[%s:%s] ", __PRETTY_FUNCTION__, (*m_current).c_str());
    if(m_current && !(*m_current).empty())
        return *m_current;
    return SettingsHomePage::DEF_HOME_PAGE;
}

void SettingsHomePage::updateButtonMap()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData defaultPage;
    defaultPage.buttonText = "Default";
    defaultPage.subText = DEF_HOME_PAGE;
    defaultPage.sui = this;
    defaultPage.id = DEFAULT;

    ItemData current;
    current.buttonText = "Current page";
    current.subText = getCurrentPage();
    current.sui = this;
    current.id = CURRENT;

    ItemData quick;
    quick.buttonText = "Quick access";
    quick.sui = this;
    quick.id = QUICK_ACCESS;

    ItemData most;
    most.buttonText = "Most visited websites";
    most.sui = this;
    most.id = MOST_VIS;

    ItemData other;
    other.buttonText = "Other";
    other.subText = "http://wwww.samsung.com";
    other.sui = this;
    other.id = OTHER;

    m_buttonsMap[SettingsHomePageOptions::DEFAULT] = defaultPage;
    m_buttonsMap[SettingsHomePageOptions::CURRENT] = current;
    m_buttonsMap[SettingsHomePageOptions::QUICK_ACCESS] = quick;
    m_buttonsMap[SettingsHomePageOptions::MOST_VIS] = most;
    m_buttonsMap[SettingsHomePageOptions::OTHER] = other;
}

bool SettingsHomePage::populateList(Evas_Object* genlist)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Home page");
    updateButtonMap();
    m_itemsMap[SettingsHomePageOptions::DEFAULT] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::DEFAULT], _default_cb);
    m_itemsMap[SettingsHomePageOptions::CURRENT] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::CURRENT], _current_cb);
    m_itemsMap[SettingsHomePageOptions::QUICK_ACCESS] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::QUICK_ACCESS], _quick_cb);
    m_itemsMap[SettingsHomePageOptions::MOST_VIS] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::MOST_VIS], _most_visited_cb);
    m_itemsMap[SettingsHomePageOptions::OTHER] =
        appendGenlist(genlist, m_setting_check_radio_item_class, &m_buttonsMap[SettingsHomePageOptions::OTHER], _other_cb);
    return true;
}

void SettingsHomePage::_default_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self =  static_cast<SettingsHomePage*>(data);
    self->homePageChanged(SettingsHomePage::DEF_HOME_PAGE);
}

void SettingsHomePage::_current_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self =  static_cast<SettingsHomePage*>(data);
    self->homePageChanged(self->getCurrentPage());
}

void SettingsHomePage::_quick_cb(void */*data*/, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsHomePage::_most_visited_cb(void */*data*/, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsHomePage::_other_cb(void */*data*/, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

}
}
