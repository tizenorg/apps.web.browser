/*
 * SettingsMain_mob.h
 *
 *  Created on: May 24, 2016
 *      Author: knowac
 */

#ifndef SETTINGSMAIN_MOB_H_
#define SETTINGSMAIN_MOB_H_

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

enum SettingsMainOptions {
    BASE,
    HOME,
    SEARCH,
    AUTO_FILL_PROFILE,
    AUTO_FILL_CREATOR_WITHOUT_PROFILE,
    AUTO_FILL_CREATOR_WITH_PROFILE,
    PRIVACY,
    ZOOM,
    ADVANCED,
    DEL_PERSONAL_DATA
};

class SettingsMain
    : public SettingsUI
{
public:
    SettingsMain(){};
    SettingsMain(Evas_Object* parent);
    virtual ~SettingsMain();
    virtual bool populateList(Evas_Object* genlist);
    Evas_Object* createOnOffCheckBox(Evas_Object* obj, ItemData* itd);
    Eina_Bool getOriginalState(int id);
    virtual void updateButtonMap();
    virtual void connectSignals(){};
    virtual void disconnectSignals(){};
    static void _home_page_cb(void *data, Evas_Object*obj , void* event_info);
    static void _search_engine_cb(void *data, Evas_Object*obj , void* event_info);
    static void _zoom_cb(void *data, Evas_Object*obj , void* event_info);
    static void _advanced_cb(void *data, Evas_Object*obj , void* event_info);
    static void _auto_fill_cb(void* data, Evas_Object* obj, void* event_info);
    static void _privacy_cb(void* data, Evas_Object* obj, void * event_info);
    static void grid_item_check_changed(void* data, Evas_Object* obj, void*);
};

}
}

#endif /* SETTINGSMAIN_MOB_H_ */
