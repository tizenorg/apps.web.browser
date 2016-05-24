
#ifndef SETTINGSPRIVACY_MOB_H_
#define SETTINGSPRIVACY_MOB_H_

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

enum SettingsPrivacyOptions {
    COOKIES,
    SUGGESTIONS,
    SIGNIN_INFO,
    DEL_PER_DATA
};

class SettingsPrivacy
    : public SettingsUI
{
public:
    SettingsPrivacy(){};
    SettingsPrivacy(Evas_Object* parent);
    virtual ~SettingsPrivacy();
    virtual bool populateList(Evas_Object* genlist);
    virtual void updateButtonMap();
    static void _cookies_cb(void *data, Evas_Object*obj , void* event_info);
    static void _suggestions_cb(void *data, Evas_Object*obj , void* event_info);
    static void _signin_cb(void *data, Evas_Object*obj , void* event_info);
    static void _del_per_data_cb(void *data, Evas_Object*obj , void* event_info);
};

}
}

#endif /* SETTINGSPRIVACY_MOB_H_ */
