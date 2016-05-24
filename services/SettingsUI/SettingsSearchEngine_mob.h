/*
 * SettingsSearchEngine_mob.h
 *
 *  Created on: May 30, 2016
 *      Author: knowac
 */

#ifndef SETTINGSSEARCHENGINE_MOB_H_
#define SETTINGSSEARCHENGINE_MOB_H_

#include "SettingsUI_mob.h"

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <Evas.h>
#include "BrowserLogger.h"
#include "Tools/EflTools.h"

namespace tizen_browser{
namespace base_ui{

class SettingsSearchEngine
    : public SettingsUI
{
public:
    SettingsSearchEngine(){};
    SettingsSearchEngine(Evas_Object* parent);
    virtual ~SettingsSearchEngine();
    virtual bool populateList(Evas_Object* genlist);
    virtual void updateButtonMap();
    static void _test_page_cb(void* data, Evas_Object*, void*);
protected:
    std::map<unsigned, ItemData> m_buttonsMap;
};

}
}
#endif /* SETTINGSSEARCHENGINE_MOB_H_ */
