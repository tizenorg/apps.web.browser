/*
 * SettingsManager.h
 *
 *  Created on: May 25, 2016
 *      Author: knowac
 */

#ifndef SETTINGSMANAGER_H_
#define SETTINGSMANAGER_H_

#include "AbstractService.h"
#include "service_macros.h"
#include "SettingsUI_mob.h"
#include "SettingsMain_mob.h"
#include "SettingsHomePage_mob.h"
#include "SettingsSearchEngine_mob.h"
#include "SettingsPrivacy_mob.h"
#include "SettingsAdvanced_mob.h"
#include "SettingsDelPersData_mob.h"
#include "SettingsAFProfile_mob.h"
#include "SettingsAFCreator_mob.h"
#include "service_macros.h"
#include <string>
#include <map>
#include <Evas.h>
#include <Elementary.h>
#include <memory>

namespace tizen_browser{
namespace base_ui{

typedef std::shared_ptr<SettingsUI> SetPtr;
class BROWSER_EXPORT SettingsManager
    : public tizen_browser::core::AbstractService
{
public:
    SettingsManager();
    ~SettingsManager();
    void init(Evas_Object* parent);
    SetPtr getView(const SettingsMainOptions& s);
    SettingsUI* getViewPtr(const SettingsMainOptions& s);
    void addView(const SettingsMainOptions& s);
    std::string getName();
    SetPtr operator[](const SettingsMainOptions& s){ return m_settingsViews[s];};
private:
    std::map<SettingsMainOptions,SetPtr> m_settingsViews;
    Evas_Object* m_parent;
};
}
}

#endif /* SETTINGSMANAGER_H_ */
