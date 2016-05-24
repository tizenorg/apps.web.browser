/*
 * SettingsAFProfile_mob.h
 *
 *  Created on: May 30, 2016
 *      Author: knowac
 */

#ifndef SETTINGSAFCREATOR_MOB_H_
#define SETTINGSAFCREATOR_MOB_H_

#include "SettingsUI_mob.h"

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <Evas.h>
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "AutoFillForm/AutoFillFormItem.h"

namespace tizen_browser{
namespace base_ui{

class SettingsAFCreator
    : public SettingsUI
{
public:
    SettingsAFCreator(){};
    SettingsAFCreator(Evas_Object* parent, bool profile_exists);
    virtual ~SettingsAFCreator();
    virtual bool populateLayout(Evas_Object* genlist);
    virtual void updateButtonMap();
    Evas_Object* createScroller(Evas_Object *parent);
    bool loadProfile(void);
    void createNewAutoFillFormItem(Ewk_Autofill_Profile*);
private:
    Eina_Bool isEntryHasOnlySpace(const char* field);
    Eina_Bool applyEntryData(void);
    static void __done_button_cb(void* data, Evas_Object* obj, void* event_info);
    static void __cancel_button_cb(void* data, Evas_Object* obj, void* event_info);
    static void __entry_changed_cb(void* data, Evas_Object* obj, void* event_info);
    static void __entry_next_key_cb(void* data, Evas_Object* obj, void* event_info);
    static void __entry_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static void __entry_clear_button_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static void __editfield_changed_cb(void* data, Evas_Object* obj, void* event_info);
    enum menu_type
    {
        profile_composer_title_full_name = 0,
        profile_composer_title_company_name,
        profile_composer_title_address_line_1,
        profile_composer_title_address_line_2,
        profile_composer_title_city_town,
        profile_composer_title_country,
        profile_composer_title_post_code,
        profile_composer_title_country_region,
        profile_composer_title_phone,
        profile_composer_title_email,
        profile_composer_menu_end
    };

    struct genlistCallbackData {
        menu_type type;
        void *user_data;
        Evas_Object* editfield;
        Evas_Object* entry;
        Evas_Object* it;
    };
    void createInputLayout(Evas_Object* parent, char* fieldName, genlistCallbackData* cb_data);
    void addItems();
protected:
    std::map<unsigned, ItemData> m_buttonsMap;
    Evas_Object *m_mainLayout;
    Evas_Object *m_scroller;
    Evas_Object *m_box;
    Evas_Object *m_doneButton;
    Evas_Object *m_cancelButton;

    profileEditErrorcode m_editErrorcode;
    profileSaveErrorcode m_saveErrorcode;
    Elm_Genlist_Item_Class *m_editFieldItemClass;
    Elm_Entry_Filter_Limit_Size m_entryLimitSize;

    genlistCallbackData m_fullNameItemCallbackData;
    genlistCallbackData m_companyNameItemCallbackData;
    genlistCallbackData m_addressLine1ItemCallbackData;
    genlistCallbackData m_addressLine2ItemCallbackData;
    genlistCallbackData m_cityTownItemCallbackData;
    genlistCallbackData m_countryItemCallbackData;
    genlistCallbackData m_postCodeItemCallbackData;
    genlistCallbackData m_countyRegionItemCallbackData;
    genlistCallbackData m_phoneItemCallbackData;
    genlistCallbackData m_emailItemCallbackData;
//    std::string m_edjFilePath;
    std::shared_ptr<AutoFillFormItem> m_item;
    Ewk_Context* m_ewkContext;
    bool m_profile_exists;
};

}
}
#endif /* SETTINGSAFCREATOR_MOB_H_ */
