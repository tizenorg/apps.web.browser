/*
 * SettingsManager.cpp
 *
 *  Created on: May 25, 2016
 *      Author: knowac
 */
#include "SettingsManager.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SettingsManager, "org.tizen.browser.settingsui")

SettingsManager::SettingsManager()
    : m_parent(nullptr)
{
};

SettingsManager::~SettingsManager()
{
};

void SettingsManager::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
}

std::shared_ptr<SettingsUI> SettingsManager::getView(const SettingsMainOptions& s)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    addView(s);
    return m_settingsViews[s];
}

SettingsUI* SettingsManager::getViewPtr(const SettingsMainOptions& s)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    addView(s);
    return m_settingsViews[s].get();
}

void SettingsManager::addView(const SettingsMainOptions& s)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_settingsViews.find(s) == m_settingsViews.end()) {
        switch (s) {
            case BASE:
                m_settingsViews[s] = std::make_shared<SettingsMain>(m_parent);
                break;
            case HOME:
                m_settingsViews[s] = std::make_shared<SettingsHomePage>(m_parent);
                break;
            case SEARCH:
                m_settingsViews[s] = std::make_shared<SettingsSearchEngine>(m_parent);
                break;
            case AUTO_FILL_PROFILE:
                m_settingsViews[s] = std::make_shared<SettingsAFProfile>(m_parent);
                break;
            case AUTO_FILL_CREATOR_WITH_PROFILE:
                m_settingsViews[s] = std::make_shared<SettingsAFCreator>(m_parent, true);
                break;
            case AUTO_FILL_CREATOR_WITHOUT_PROFILE:
                m_settingsViews[s] = std::make_shared<SettingsAFCreator>(m_parent, false);
                break;
            case PRIVACY:
                m_settingsViews[s] = std::make_shared<SettingsPrivacy>(m_parent);
                break;
            case ADVANCED:
                m_settingsViews[s] = std::make_shared<SettingsAdvanced>(m_parent);
                break;
            case DEL_PERSONAL_DATA:
                m_settingsViews[s] = std::make_shared<SettingsDelPersData>(m_parent);
                break;
            default:
                break;
        }
    } else {
        m_settingsViews[s]->updateButtonMap();
        if (m_settingsViews[s]->getGenlist())
           elm_genlist_realized_items_update(m_settingsViews[s]->getGenlist());
    }
}

}
}
