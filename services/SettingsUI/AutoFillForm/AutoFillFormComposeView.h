/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AUTOFILLFORMCOMPOSEVIEW_H_
#define AUTOFILLFORMCOMPOSEVIEW_H_

#include "AutoFillFormItem.h"

class AutoFillFormManager;
class AutoFillFormComposeView {
public:
    AutoFillFormComposeView(AutoFillFormManager* manager, AutoFillFormItem *item = NULL);
    ~AutoFillFormComposeView(void);

    void show(Evas_Object *parent);
    void hide();
private:
    typedef enum _menu_type
    {
        profile_composer_title_full_name = 0,
        profile_composer_title_company_name,
        profile_composer_title_address_line_1,
        profile_composer_title_address_line_2,
        profile_composer_title_city_town,
        profile_composer_title_county_region,
        profile_composer_title_post_code,
        profile_composer_title_country,
        profile_composer_title_phone,
        profile_composer_title_email,

        profile_composer_menu_end
    } menu_type;

    typedef struct _genlistCallbackData {
        menu_type type;
        void *user_data;
        Evas_Object *entry;
        Elm_Object_Item *it;
    } genlistCallbackData;

    Evas_Object *createMainLayout(Evas_Object *parent);
    Evas_Object *createGenlist(Evas_Object *parent);
    Eina_Bool isEntryHasOnlySpace(const char *);
    Eina_Bool applyEntryData(void);

    static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);
    static char *__text_get_cb(void *data, Evas_Object *obj, const char *part);
    static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
    static void __done_button_cb(void *data, Evas_Object *obj, void *event_info);
    static void __cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
    static void __entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
    static void __entry_next_key_cb(void *data, Evas_Object *obj, void *eventInfo);
    static void __entry_clicked_cb(void *data, Evas_Object *obj, void *eventInfo);
    static void __entry_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void __editfield_changed_cb(void *data, Evas_Object *obj, void *event_info);
    AutoFillFormItem *m_itemForCompose;
    AutoFillFormManager *m_manager;

    Evas_Object *m_mainLayout;
    Evas_Object *m_genlist;
    Evas_Object *m_doneButton;
    Evas_Object *m_cancelButton;
    Evas_Object *m_entryFullName;
    Evas_Object *m_entryCompanyName;
    Evas_Object *m_entryAddressLine1;
    Evas_Object *m_entryAddressLine2;
    Evas_Object *m_entryCityTown;
    Evas_Object *m_entryCounty;
    Evas_Object *m_entryPostCode;
    Evas_Object *m_entryCountry;
    Evas_Object *m_entryPhone;
    Evas_Object *m_entryEmail;

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
    std::string m_edjFilePath;
};

#endif /* AUTOFILLFORMCOMPOSEVIEW_H_ */
