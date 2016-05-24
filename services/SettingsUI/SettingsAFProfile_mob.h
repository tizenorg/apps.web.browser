/*
 * SettingsAFProfile_mob.h
 *
 *  Created on: May 30, 2016
 *      Author: knowac
 */

#ifndef SETTINGSAFPROFILE_MOB_H_
#define SETTINGSAFPROFILE_MOB_H_

#include "SettingsUI_mob.h"

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <Evas.h>
#include "BrowserLogger.h"
#include "Tools/EflTools.h"

#include <ewk_chromium.h>

namespace tizen_browser{
namespace base_ui{

class SettingsAFProfile
    : public SettingsUI
{
public:
    SettingsAFProfile(){};
    SettingsAFProfile(Evas_Object* parent);
    virtual ~SettingsAFProfile();
    virtual bool populateList(Evas_Object* genlist);
    virtual void updateButtonMap();
    static void _select_profile_cb(void* data, Evas_Object*, void*);
protected:
    std::map<unsigned, ItemData> m_buttonsMap;
    Ewk_Autofill_Profile* m_profile;
};

}
}
#endif /* SETTINGSAFPROFILE_MOB_H_ */
