#ifndef SETTINGSDELPERSDATA_MOB_H_
#define SETTINGSDELPERSDATA_MOB_H_

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

class SettingsDelPersData
    : public SettingsUI
{
public:
    SettingsDelPersData(){};
    SettingsDelPersData(Evas_Object* parent);
    virtual ~SettingsDelPersData();
    virtual bool populateList(Evas_Object* genlist);
    virtual void updateButtonMap();
    virtual Evas_Object* createNormalCheckBox(Evas_Object*, ItemData*);
    static void grid_item_check_changed(void* data, Evas_Object* obj, void*);
    static void __cancel_button_cb(void* data, Evas_Object*, void*);
    static void __delete_button_cb(void* data, Evas_Object*, void*);
protected:
    std::map<unsigned, ItemData> m_buttonsMap;
    Evas_Object* m_cancelButton;
    Evas_Object* m_deleteButton;
};

}
}
#endif /* SETTINGSDELPERSDATA_MOB_H_ */
