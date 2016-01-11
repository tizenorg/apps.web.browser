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

#include "AutoFillFormManager.h"
#include "AutoFillFormComposeView.h"
#include "AutoFillFormListView.h"
#include "BrowserLogger.h"
#include "app_i18n.h"

#include <Ecore.h>
#include <Elementary.h>
#include <Evas.h>

#define TRIM_SPACE " \t\n\v"

namespace tizen_browser{
namespace base_ui{

inline std::string _trim(std::string& s, const std::string& drop = TRIM_SPACE)
{
    std::string r = s.erase(s.find_last_not_of(drop) + 1);
    return r.erase(0, r.find_first_not_of(drop));
}

AutoFillFormComposeView::AutoFillFormComposeView(AutoFillFormManager* manager, AutoFillFormItem *item)
    : m_itemForCompose(NULL)
    , m_manager(manager)
    , m_mainLayout(NULL)
    , m_genlist(NULL)
    , m_doneButton(NULL)
    , m_cancelButton(NULL)
    , m_entryFullName(NULL)
    , m_entryCompanyName(NULL)
    , m_entryAddressLine1(NULL)
    , m_entryAddressLine2(NULL)
    , m_entryCityTown(NULL)
    , m_entryCounty(NULL)
    , m_entryPostCode(NULL)
    , m_entryCountry(NULL)
    , m_entryPhone(NULL)
    , m_entryEmail(NULL)
    , m_editErrorcode(update_error_none)
    , m_saveErrorcode(save_error_none)
    , m_editFieldItemClass(NULL)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (item && item->getItemComposeMode() == profile_edit)
        m_itemForCompose = item;
    else
        m_itemForCompose = new AutoFillFormItem(NULL);

    m_entryLimitSize.max_char_count = 0;
    m_entryLimitSize.max_byte_count = AUTO_FILL_FORM_ENTRY_MAX_COUNT;
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/AutoFillMobileUI.edj");
}

AutoFillFormComposeView::~AutoFillFormComposeView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_editFieldItemClass)
        elm_genlist_item_class_free(m_editFieldItemClass);

    if (m_itemForCompose && m_itemForCompose->getItemComposeMode() == profile_create)
        delete m_itemForCompose;
    m_itemForCompose = NULL;

    if (m_genlist) {
        elm_genlist_clear(m_genlist);
        evas_object_del(m_genlist);
    }
    if (m_mainLayout) {
        evas_object_hide(m_mainLayout);
        evas_object_del(m_mainLayout);
    }
}

void AutoFillFormComposeView::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_mainLayout = elm_layout_add(parent);
    if (!m_mainLayout) {
        BROWSER_LOGE("createMainLayout failed");
        return;
    }
    elm_layout_file_set(m_mainLayout, m_edjFilePath.c_str(), "affcv-layout");
    evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_mainLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_genlist = createGenlist(m_mainLayout);
    evas_object_show(m_genlist);
    elm_object_part_content_set(m_mainLayout, "affcv_genlist", m_genlist);

    evas_object_show(m_mainLayout);

    if (m_itemForCompose->getItemComposeMode() == profile_edit)
        elm_object_part_text_set(m_mainLayout, "title_text", "Auto Fill Edit");
    else
        elm_object_part_text_set(m_mainLayout, "title_text", "Auto Fill Compose");

    m_cancelButton = elm_button_add(m_mainLayout);
    if (!m_cancelButton) {
        BROWSER_LOGE("Failed to create m_cancelButton");
        return;
    }
    elm_object_style_set(m_cancelButton, "basic_button");
    evas_object_smart_callback_add(m_cancelButton, "clicked", __cancel_button_cb, this);
    elm_object_part_content_set(m_mainLayout, "cancel_button", m_cancelButton);

    m_doneButton = elm_button_add(m_mainLayout);
    if (!m_doneButton) {
        BROWSER_LOGE("Failed to create m_doneButton");
        return;
    }
    elm_object_style_set(m_doneButton, "basic_button");
    evas_object_smart_callback_add(m_doneButton, "clicked", __done_button_cb, this);
    elm_object_part_content_set(m_mainLayout, "done_button", m_doneButton);
        elm_object_signal_emit(m_mainLayout, "dim,done,button,signal", "");

    if (m_itemForCompose->getItemComposeMode() == profile_create)
        elm_object_disabled_set(m_doneButton, EINA_TRUE);
}

Evas_Object *AutoFillFormComposeView::createGenlist(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *genlist = elm_genlist_add(parent);
    if (!genlist) {
        BROWSER_LOGE("elm_genlist_add failed");
        return NULL;
    }

    Elm_Genlist_Item_Class *edit_field_item_class = elm_genlist_item_class_new();
    if (!edit_field_item_class) {
        BROWSER_LOGE("elm_genlist_item_class_new for edit_field_item_class failed");
        return EINA_FALSE;
    }
    edit_field_item_class->item_style = "affcv_item";
    edit_field_item_class->func.text_get = __text_get_cb;
    edit_field_item_class->func.content_get = __content_get_cb;
    m_editFieldItemClass = edit_field_item_class;

    m_fullNameItemCallbackData.type = profile_composer_title_full_name;
    m_fullNameItemCallbackData.user_data = this;
    m_fullNameItemCallbackData.it = elm_genlist_item_append(genlist, m_editFieldItemClass,
                        &m_fullNameItemCallbackData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    m_companyNameItemCallbackData.type = profile_composer_title_company_name;
    m_companyNameItemCallbackData.user_data = this;
    m_companyNameItemCallbackData.it = elm_genlist_item_append(genlist, m_editFieldItemClass,
                        &m_companyNameItemCallbackData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    m_addressLine1ItemCallbackData.type = profile_composer_title_address_line_1;
    m_addressLine1ItemCallbackData.user_data = this;
    m_addressLine1ItemCallbackData.it = elm_genlist_item_append(genlist, m_editFieldItemClass,
                        &m_addressLine1ItemCallbackData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    m_addressLine2ItemCallbackData.type = profile_composer_title_address_line_2;
    m_addressLine2ItemCallbackData.user_data = this;
    m_addressLine2ItemCallbackData.it = elm_genlist_item_append(genlist, m_editFieldItemClass,
                        &m_addressLine2ItemCallbackData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    m_cityTownItemCallbackData.type = profile_composer_title_city_town;
    m_cityTownItemCallbackData.user_data = this;
    m_cityTownItemCallbackData.it = elm_genlist_item_append(genlist, m_editFieldItemClass,
                        &m_cityTownItemCallbackData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    m_countryItemCallbackData.type = profile_composer_title_country;
    m_countryItemCallbackData.user_data = this;
    m_countryItemCallbackData.it = elm_genlist_item_append(genlist, m_editFieldItemClass,
                        &m_countryItemCallbackData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    m_postCodeItemCallbackData.type = profile_composer_title_post_code;
    m_postCodeItemCallbackData.user_data = this;
    m_postCodeItemCallbackData.it = elm_genlist_item_append(genlist, m_editFieldItemClass,
                        &m_postCodeItemCallbackData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    m_phoneItemCallbackData.type = profile_composer_title_phone;
    m_phoneItemCallbackData.user_data = this;
    m_phoneItemCallbackData.it = elm_genlist_item_append(genlist, m_editFieldItemClass,
                        &m_phoneItemCallbackData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    m_emailItemCallbackData.type = profile_composer_title_email;
    m_emailItemCallbackData.user_data = this;
    m_emailItemCallbackData.it = elm_genlist_item_append(genlist, m_editFieldItemClass,
                        &m_emailItemCallbackData, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    return genlist;
}

Eina_Bool AutoFillFormComposeView::isEntryHasOnlySpace(const char* field)
{
    Eina_Bool only_has_space = EINA_FALSE;
    unsigned int space_count = 0;
    unsigned int str_len = strlen(field);

    for (unsigned int i = 0 ; i < str_len ; i++) {
        if (field[i] == ' ')
            space_count++;
    }
    if (space_count == str_len)
        only_has_space = EINA_TRUE;

    return only_has_space;
}

Eina_Bool AutoFillFormComposeView::applyEntryData(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    const char *full_name = elm_entry_entry_get(m_entryFullName);

    if (!full_name)
        return EINA_FALSE;

    std::string full_name_str = std::string(full_name);
    full_name_str = _trim(full_name_str);
    full_name = full_name_str.c_str();

    if (full_name && strlen(full_name) && !isEntryHasOnlySpace(full_name))
        m_itemForCompose->setName(full_name);
    else {
        elm_object_focus_set(m_cancelButton, EINA_TRUE); // Closing virtual keyboard by changing the focus*/
        return EINA_FALSE;
    }
    const char *company_name = elm_entry_entry_get(m_entryCompanyName);
    m_itemForCompose->setCompany(company_name);
    const char *primary_address = elm_entry_entry_get(m_entryAddressLine1);
    m_itemForCompose->setPrimaryAddress(primary_address);
    const char *secondary_address = elm_entry_entry_get(m_entryAddressLine2);
    m_itemForCompose->setSecondaryAddress2(secondary_address);
    const char *city_town = elm_entry_entry_get(m_entryCityTown);
    m_itemForCompose->setCityTown(city_town);
    const char *county = elm_entry_entry_get(m_entryCounty);
    m_itemForCompose->setStateProvince(county);
    const char *post_code = elm_entry_entry_get(m_entryPostCode);
    m_itemForCompose->setPostCode(post_code);
    const char *country = elm_entry_entry_get(m_entryCountry);
    m_itemForCompose->setCountry(country);
    const char *phone = elm_entry_entry_get(m_entryPhone);
    m_itemForCompose->setPhoneNumber(phone);
    const char *email = elm_entry_entry_get(m_entryEmail);
    m_itemForCompose->setEmailAddress(email);

    if (m_itemForCompose->getItemComposeMode() == profile_edit) {

        m_editErrorcode = m_itemForCompose->updateItem();
        if (m_editErrorcode == profile_edit_failed || m_editErrorcode == profile_already_exist) {
            BROWSER_LOGD("Update failed!");
            return EINA_FALSE;
        }
    } else {
        m_saveErrorcode = m_itemForCompose->saveItem();
        if (m_saveErrorcode != profile_create_failed && m_saveErrorcode != duplicate_profile)
            m_manager->addItemToList(m_itemForCompose);
    }

    return EINA_TRUE;
}

char *AutoFillFormComposeView::__text_get_cb(void* data, Evas_Object* /*obj*/, const char *part)
{
    BROWSER_LOGD("part[%s]", part);

    genlistCallbackData *cb_data = static_cast<genlistCallbackData*>(data);
    AutoFillFormComposeView::menu_type type = cb_data->type;

    if (!strcmp(part, "field_name")) {
        if (type == profile_composer_title_full_name)
            return strdup("Full Name" /*BR_STRING_AUTO_FILL_DATA_FULL_NAME*/);
        else if (type == profile_composer_title_company_name)
            return strdup(_("IDS_BR_BODY_COMPANY_NAME_ABB") /*BR_STRING_AUTO_FILL_DATA_COMPANY_NAME*/);
        else if (type == profile_composer_title_address_line_1)
            return strdup(_("IDS_BR_BODY_ADDRESS_LINE_1_ABB") /*BR_STRING_AUTO_FILL_DATA_ADDRESS_LINE_1*/);
        else if (type == profile_composer_title_address_line_2)
            return strdup(_("IDS_BR_BODY_ADDRESS_LINE_2_ABB") /*BR_STRING_AUTO_FILL_DATA_ADDRESS_LINE_2*/);
        else if (type == profile_composer_title_city_town)
            return strdup("City town" /*BR_STRING_TOWN_AUTO_FILL_CITY_COUNTY*/);
        else if (type == profile_composer_title_county_region)
            return strdup("Region" /*BR_STRING_AUTO_FILL_DATA_COUNTRY_REGION*/);
        else if (type == profile_composer_title_post_code)
            return strdup("Post code"/*BR_STRING_AUTO_FILL_DATA_POST_CODE*/);
        else if (type == profile_composer_title_country)
            return strdup("Country" /*BR_STRING_AUTO_FILL_DATA_COUNTRY*/);
        else if (type == profile_composer_title_phone)
            return strdup(_("IDS_BR_BODY_PHONE") /*BR_STRING_AUTO_FILL_DATA_PHONE*/);
        else if (type == profile_composer_title_email)
            return strdup(_("IDS_BR_OPT_SENDURLVIA_EMAIL") /*BR_STRING_AUTO_FILL_DATA_EMAIL*/);
    }

    return NULL;
}

Evas_Object *AutoFillFormComposeView::__content_get_cb(void* data, Evas_Object* obj, const char *part)
{
    BROWSER_LOGD("part[%s]", part);

    genlistCallbackData *cb_data = static_cast<genlistCallbackData*>(data);
    AutoFillFormComposeView *view = static_cast<AutoFillFormComposeView*>(cb_data->user_data);
    AutoFillFormComposeView::menu_type type = cb_data->type;

    if (!strcmp(part, "entry_swallow")) {

        Evas_Object *editfield = elm_layout_add(obj);
        elm_layout_file_set(editfield, view->m_edjFilePath.c_str(), "edit-field");
        evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL, 0.0);
        evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND, 0.0);

        Evas_Object *entry = elm_entry_add(editfield);
        elm_object_style_set(entry, "entry_style");
        elm_entry_single_line_set(entry, EINA_TRUE);
        elm_entry_scrollable_set(entry, EINA_TRUE);
        elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
        elm_entry_text_style_user_push(entry, "DEFAULT='font_size=30 color=#404040 ellipsis=1'");

#if defined(HW_MORE_BACK_KEY)
        eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
#endif
        evas_object_smart_callback_add(entry, "preedit,changed", __entry_changed_cb, cb_data);
        evas_object_smart_callback_add(entry, "changed", __entry_changed_cb, cb_data);
        evas_object_smart_callback_add(entry, "changed", __editfield_changed_cb, editfield);
        evas_object_smart_callback_add(entry, "activated", __entry_next_key_cb, cb_data);
        evas_object_smart_callback_add(entry, "clicked", __entry_clicked_cb, cb_data);
        elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_NEXT);
        elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &(view->m_entryLimitSize));

        if (type == profile_composer_title_full_name) {
            if (view->m_itemForCompose->getName() && strlen(view->m_itemForCompose->getName()))
                elm_object_part_text_set(entry, "elm.text", view->m_itemForCompose->getName());
            view->m_entryFullName = cb_data->entry = entry;
        } else if (type == profile_composer_title_company_name) {
            if (view->m_itemForCompose->getCompany() && strlen(view->m_itemForCompose->getCompany()))
                elm_object_part_text_set(entry, "elm.text", view->m_itemForCompose->getCompany());
            view->m_entryCompanyName = cb_data->entry = entry;
        } else if (type == profile_composer_title_address_line_1) {
            if (view->m_itemForCompose->getPrimaryAddress() && strlen(view->m_itemForCompose->getPrimaryAddress()))
                elm_object_part_text_set(entry, "elm.text", view->m_itemForCompose->getPrimaryAddress());
            view->m_entryAddressLine1 = cb_data->entry = entry;
        } else if (type == profile_composer_title_address_line_2) {
            if (view->m_itemForCompose->getSecondaryAddress2() && strlen(view->m_itemForCompose->getSecondaryAddress2()))
                elm_object_part_text_set(entry, "elm.text", view->m_itemForCompose->getSecondaryAddress2());
            view->m_entryAddressLine2 = cb_data->entry = entry;
        } else if (type == profile_composer_title_city_town) {
            if (view->m_itemForCompose->getCityTown() && strlen(view->m_itemForCompose->getCityTown()))
                elm_object_part_text_set(entry, "elm.text", view->m_itemForCompose->getCityTown());
            view->m_entryCityTown = cb_data->entry = entry;
        } else if (type == profile_composer_title_county_region) {
            if (view->m_itemForCompose->getStateProvince() && strlen(view->m_itemForCompose->getStateProvince()))
                elm_object_part_text_set(entry, "elm.text", view->m_itemForCompose->getStateProvince());
            view->m_entryCounty = cb_data->entry = entry;
        } else if (type == profile_composer_title_post_code) {
            Elm_Entry_Filter_Limit_Size m_entryLimitSize;
            Elm_Entry_Filter_Accept_Set m_entry_accept_set;
            m_entryLimitSize.max_char_count = 10;
            m_entry_accept_set.accepted = "0123456789";
            m_entry_accept_set.rejected = NULL;
            elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &m_entryLimitSize);
            elm_entry_markup_filter_append(entry, elm_entry_filter_accept_set, &m_entry_accept_set);

            if (view->m_itemForCompose->getPostCode() && strlen(view->m_itemForCompose->getPostCode()))
                elm_object_part_text_set(entry, "elm.text", view->m_itemForCompose->getPostCode());
            elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NUMBERONLY);
            elm_entry_prediction_allow_set(entry, EINA_FALSE);
            view->m_entryPostCode = cb_data->entry = entry;
        } else if (type == profile_composer_title_country) {
            if (view->m_itemForCompose->getCountry() && strlen(view->m_itemForCompose->getCountry()))
                elm_object_part_text_set(entry, "elm.text", view->m_itemForCompose->getCountry());
            view->m_entryCountry = cb_data->entry = entry;
        } else if (type == profile_composer_title_phone) {
            if (view->m_itemForCompose->getPhoneNumber() && strlen(view->m_itemForCompose->getPhoneNumber()))
                elm_object_part_text_set(entry, "elm.text", view->m_itemForCompose->getPhoneNumber());
            Elm_Entry_Filter_Accept_Set entry_accept_set;
            entry_accept_set.accepted = PHONE_FIELD_VALID_ENTRIES;
            entry_accept_set.rejected = NULL;
            elm_entry_markup_filter_append(entry, elm_entry_filter_accept_set, &entry_accept_set);
            elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_PHONENUMBER);
            elm_entry_prediction_allow_set(entry, EINA_FALSE);
            view->m_entryPhone = cb_data->entry = entry;
        } else if (type == profile_composer_title_email) {
            if (view->m_itemForCompose->getEmailAddress() && strlen(view->m_itemForCompose->getEmailAddress()))
                elm_object_part_text_set(entry, "elm.text", view->m_itemForCompose->getEmailAddress());
            elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
            evas_object_smart_callback_add(entry, "activated", __done_button_cb, view);
            elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_EMAIL);
            elm_entry_prediction_allow_set(entry, EINA_FALSE);
            view->m_entryEmail = cb_data->entry = entry;
        }

        elm_object_part_content_set(editfield, "editfield_entry", entry);

        Evas_Object *button = elm_button_add(editfield);
        elm_object_style_set(button, "basic_button");
        evas_object_smart_callback_add(button, "clicked", __entry_clear_button_clicked_cb, entry);
        elm_object_part_content_set(editfield, "entry_clear_button", button);

        if (!elm_entry_is_empty(entry)) {
            BROWSER_LOGE("entry is empty");
            elm_object_signal_emit(editfield, "show,clear,button,signal", "");
        }

        return editfield;
    }

    return NULL;
}

void AutoFillFormComposeView::__editfield_changed_cb(void* data, Evas_Object* obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object *editfield = static_cast<Evas_Object*>(data);

    if (!elm_entry_is_empty(obj) && elm_object_focus_get(obj))
        elm_object_signal_emit(editfield, "show,clear,button,signal", "");
    else
        elm_object_signal_emit(editfield, "hide,clear,button,signal", "");
}

void AutoFillFormComposeView::__done_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    AutoFillFormComposeView *view = static_cast<AutoFillFormComposeView*>(data);

    if (elm_entry_is_empty(view->m_entryFullName)) {
        elm_object_focus_set(view->m_cancelButton, EINA_TRUE); // Closing virtual keyboard by changing the focus
        return;
    }

    if (view->applyEntryData() == EINA_FALSE)
        return;

    elm_object_focus_set(view->m_cancelButton, EINA_TRUE);
    evas_object_hide(view->m_mainLayout);
    view->m_manager->refreshListView();
}

void AutoFillFormComposeView::__cancel_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    AutoFillFormComposeView *view = static_cast<AutoFillFormComposeView*>(data);
    view->hide();
}

void AutoFillFormComposeView::__entry_changed_cb(void* data, Evas_Object* obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    genlistCallbackData *cb_data = static_cast<genlistCallbackData*>(data);
    AutoFillFormComposeView *view = static_cast<AutoFillFormComposeView*>(cb_data->user_data);
    const char* text = elm_entry_entry_get(obj);
    //view->show_notification(text, obj, AUTO_FILL_FORM_ENTRY_MAX_COUNT);
    if (text && strlen(text) > 0) {
        elm_object_signal_emit(cb_data->it, "show,clear,button,signal", "");
    }
    else {
        elm_object_signal_emit(cb_data->it, "hide,clear,button,signal", "");
    }

    if (!elm_entry_is_empty(view->m_entryFullName)) {
        elm_object_signal_emit(view->m_mainLayout, "show,done,button,signal", "");
        elm_object_disabled_set(view->m_doneButton, EINA_FALSE);
    } else {
        elm_object_signal_emit(view->m_mainLayout, "dim,done,button,signal", "");
        elm_object_disabled_set(view->m_doneButton, EINA_TRUE);
    }
    return;
}

void AutoFillFormComposeView::__entry_clicked_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    Elm_Object_Item *item = callback_data->it;
    Evas_Object *entry = elm_object_item_part_content_get(item, "entry_swallow");
    elm_object_focus_set(entry, EINA_TRUE);
    //elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
}

void AutoFillFormComposeView::__entry_clear_button_clicked_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object *entry = static_cast<Evas_Object*>(data);
    elm_entry_entry_set(entry, "");
}

void AutoFillFormComposeView::__entry_next_key_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    AutoFillFormComposeView *view_this = static_cast<AutoFillFormComposeView*>(callback_data->user_data);
    AutoFillFormComposeView::menu_type type = callback_data->type;
    Evas_Object *entry = NULL;
    //Elm_Object_Item *item = NULL;

    if (type == profile_composer_title_full_name) {
        entry = elm_object_item_part_content_get(
                    view_this->m_companyNameItemCallbackData.it, "entry_swallow");
        //item = view_this->m_companyNameItemCallbackData.it;
    } else if (type == profile_composer_title_company_name) {
        entry = elm_object_item_part_content_get(
                    view_this->m_addressLine1ItemCallbackData.it, "entry_swallow");
        //item = view_this->m_addressLine1ItemCallbackData.it;
    } else if (type == profile_composer_title_address_line_1) {
        entry = elm_object_item_part_content_get(
                    view_this->m_addressLine2ItemCallbackData.it, "entry_swallow");
        //item = view_this->m_addressLine2ItemCallbackData.it;
    } else if (type == profile_composer_title_address_line_2) {
        entry = elm_object_item_part_content_get(
                    view_this->m_cityTownItemCallbackData.it, "entry_swallow");
        //item = view_this->m_cityTownItemCallbackData.it;
    } else if (type == profile_composer_title_city_town) {
        entry = elm_object_item_part_content_get(
                    view_this->m_countryItemCallbackData.it, "entry_swallow");
        //item = view_this->m_countryItemCallbackData.it;
    } else if (type == profile_composer_title_country) {
        entry = elm_object_item_part_content_get(
                    view_this->m_postCodeItemCallbackData.it, "entry_swallow");
        //item = view_this->m_postCodeItemCallbackData.it;
    } else if (type == profile_composer_title_post_code) {
        entry = elm_object_item_part_content_get(
                    view_this->m_phoneItemCallbackData.it, "entry_swallow");
        //item = view_this->m_phoneItemCallbackData.it;
    }
    else if (type == profile_composer_title_county_region) {
        entry = elm_object_item_part_content_get(
                    view_this->m_postCodeItemCallbackData.it, "entry_swallow");
    }
    else if (type == profile_composer_title_phone) {
        entry = elm_object_item_part_content_get(
                    view_this->m_emailItemCallbackData.it, "entry_swallow");
        //item = view_this->m_emailItemCallbackData.it;
    } else if (type == profile_composer_title_email) {
        BROWSER_LOGD("It's last item to go");
        return;
    }
    elm_object_focus_set(entry, EINA_TRUE);
    elm_entry_cursor_end_set(entry);
    //elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
}

void AutoFillFormComposeView::hide()
{
    evas_object_hide(m_mainLayout);
}

}
}
