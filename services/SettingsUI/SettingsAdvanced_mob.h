
#ifndef SETTINGSADVANCED_MOB_H_
#define SETTINGSADVANCED_MOB_H_

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

enum SettingsAdvancedOptions {
    ENABLE_JS,
    BLOCK_POPUPS,
    SAVE_CONTENT,
    MANAGE_WEB_DATA
};

class SettingsAdvanced
    : public SettingsUI
{
public:
    SettingsAdvanced(){};
    SettingsAdvanced(Evas_Object* parent);
    virtual ~SettingsAdvanced();
    virtual bool populateList(Evas_Object* genlist);
    virtual void updateButtonMap();
    Evas_Object* createOnOffCheckBox(Evas_Object* obj, ItemData*);
    Eina_Bool getOriginalState(int id);
    static void _enable_js_cb(void *data, Evas_Object*obj , void* event_info);
    static void _block_popups_cb(void *data, Evas_Object*obj , void* event_info);
    static void _save_content_cb(void *data, Evas_Object*obj , void* event_info);
    static void _manage_web_data_cb(void *data, Evas_Object*obj , void* event_info);
    static void grid_item_check_changed(void *data, Evas_Object *obj, void *event_info);
};

}
}

#endif /* SETTINGSADVANCED_MOB_H_ */
