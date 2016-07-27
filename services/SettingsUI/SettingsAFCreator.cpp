/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include "SettingsAFCreator.h"

#include <memory>
#include "app_i18n.h"

#define TRIM_SPACE " \t\n\v"

namespace tizen_browser{
namespace base_ui{

inline std::string _trim(std::string& s, const std::string& drop = TRIM_SPACE)
{
    std::string r = s.erase(s.find_last_not_of(drop) + 1);
    return r.erase(0, r.find_first_not_of(drop));
}

SettingsAFCreator::SettingsAFCreator(Evas_Object* parent, bool profile_exists)
    : m_mainLayout(nullptr)
    , m_scroller(nullptr)
    , m_doneButton(nullptr)
    , m_cancelButton(nullptr)
    , m_editErrorcode(update_error_none)
    , m_saveErrorcode(save_error_none)
    , m_editFieldItemClass(nullptr)
    , m_item(nullptr)
    , m_ewkContext(ewk_context_default_get())
    , m_profile_exists(profile_exists)
{
    init(parent);
    loadProfile();
};

SettingsAFCreator::~SettingsAFCreator()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

bool SettingsAFCreator::loadProfile(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Ewk_Autofill_Profile* profile = nullptr;
    if (m_profile_exists)
        profile = ewk_context_form_autofill_profile_get(m_ewkContext, 1);

    createNewAutoFillFormItem(profile);

    return false;
}

void SettingsAFCreator::createNewAutoFillFormItem(Ewk_Autofill_Profile* profile)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!profile)
        m_item  = std::shared_ptr<AutoFillFormItem>(new AutoFillFormItem(nullptr));
    else {
        AutoFillFormItemData* item_data = new AutoFillFormItemData;
        if (!item_data) {
            BROWSER_LOGE("Malloc failed to get item_data");
            return;
        }
        memset(item_data, 0x00, sizeof(AutoFillFormItemData));
        item_data->profile_id = ewk_autofill_profile_id_get(profile);
        item_data->name = ewk_autofill_profile_data_get(profile, EWK_PROFILE_NAME);
        item_data->company = ewk_autofill_profile_data_get(profile, EWK_PROFILE_COMPANY);
        item_data->primary_address = ewk_autofill_profile_data_get(profile, EWK_PROFILE_ADDRESS1);
        item_data->secondary_address = ewk_autofill_profile_data_get(profile, EWK_PROFILE_ADDRESS2);
        item_data->city_town = ewk_autofill_profile_data_get(profile, EWK_PROFILE_CITY_TOWN);
        item_data->state_province_region = ewk_autofill_profile_data_get(profile, EWK_PROFILE_STATE_PROVINCE_REGION);
        item_data->post_code = ewk_autofill_profile_data_get(profile, EWK_PROFILE_ZIPCODE);
        item_data->country = ewk_autofill_profile_data_get(profile, EWK_PROFILE_COUNTRY);
        item_data->phone_number = ewk_autofill_profile_data_get(profile, EWK_PROFILE_PHONE);
        item_data->email_address = ewk_autofill_profile_data_get(profile, EWK_PROFILE_EMAIL);
        item_data->activation = false;
        item_data->compose_mode = profile_edit;

        m_item = std::shared_ptr<AutoFillFormItem>(new AutoFillFormItem(item_data));
        delete item_data;
        item_data = nullptr;
        return;
    }
}

bool SettingsAFCreator::populateLayout(Evas_Object* parent)
{
    m_entryLimitSize.max_char_count = 0;
    m_entryLimitSize.max_byte_count = AUTO_FILL_FORM_ENTRY_MAX_COUNT;
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/AutoFillMobileUI.edj");

    elm_object_signal_emit(m_actionBar, "show,buttons,signal", "but_vis");
    elm_object_signal_emit(m_actionBar, "hide,close,icon", "del_but");

    m_mainLayout = elm_layout_add(parent);
    elm_object_tree_focus_allow_set(m_mainLayout, EINA_TRUE);
    if (!m_mainLayout) {
        BROWSER_LOGE("createMainLayout failed");
        return nullptr;
    }
    elm_layout_file_set(m_mainLayout, m_edjFilePath.c_str(), "affcv-layout");
    evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_mainLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(m_mainLayout, "cancel_text", "IDS_BR_SK_CANCEL");
    elm_object_translatable_part_text_set(m_mainLayout, "done_text", "IDS_BR_SK_DONE");

    m_scroller = createScroller(m_mainLayout);
    evas_object_show(m_scroller);
    elm_object_part_content_set(m_mainLayout, "affcv_genlist", m_scroller);
    evas_object_show(m_mainLayout);

    if (m_item->getItemComposeMode() == profile_edit)
        elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Edit info");
    else
        elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Add info");

    m_cancelButton = elm_button_add(m_actionBar);
    if (!m_cancelButton) {
        BROWSER_LOGE("Failed to create m_cancelButton");
        return nullptr;
    }
    elm_object_style_set(m_cancelButton, "basic_button");
    evas_object_smart_callback_add(m_cancelButton, "clicked", __cancel_button_cb, this);
    elm_object_part_content_set(m_actionBar, "cancel_button", m_cancelButton);

    m_doneButton = elm_button_add(m_actionBar);
    if (!m_doneButton) {
        BROWSER_LOGE("Failed to create m_doneButton");
        return nullptr;
    }
    elm_object_style_set(m_doneButton, "basic_button");
    evas_object_smart_callback_add(m_doneButton, "clicked", __done_button_cb, this);
    elm_object_part_content_set(m_actionBar, "done_button", m_doneButton);
    elm_object_signal_emit(m_actionBar, "dim,done,button,signal", "");

    if (m_item->getItemComposeMode() == profile_create)
        elm_object_disabled_set(m_doneButton, EINA_TRUE);

    m_layout = m_mainLayout;
    return true;
}

void SettingsAFCreator::createInputLayout(Evas_Object* parent, char* fieldName,
                                                        genlistCallbackData* cb_data)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object* layout = elm_layout_add(parent);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "affcv_item");
    elm_object_part_text_set(layout, "field_name", fieldName);

    Evas_Object* editfield = elm_layout_add(layout);
    elm_layout_file_set(editfield, m_edjFilePath.c_str(), "edit-field");
    evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL, 0.0);
    evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND, 0.0);

    Evas_Object* entry = elm_entry_add(editfield);
    elm_object_style_set(entry, "entry_style");
    elm_entry_single_line_set(entry, EINA_TRUE);
    elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);

    cb_data->user_data = this;
    cb_data->editfield = editfield;
    cb_data->entry = entry;
    cb_data->it = layout;
#if defined(HW_MORE_BACK_KEY)
    eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
#endif
    evas_object_smart_callback_add(entry, "preedit,changed", __entry_changed_cb, cb_data);
    evas_object_smart_callback_add(entry, "changed", __entry_changed_cb, cb_data);
    evas_object_smart_callback_add(entry, "changed", __editfield_changed_cb, editfield);
    evas_object_smart_callback_add(entry, "activated", __entry_next_key_cb, cb_data);
    evas_object_smart_callback_add(entry, "clicked", __entry_clicked_cb, cb_data);
    elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_NEXT);
    elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &(m_entryLimitSize));

    elm_object_part_content_set(editfield, "editfield_entry", entry);

    Evas_Object *button = elm_button_add(editfield);
    elm_object_style_set(button, "basic_button");
    evas_object_smart_callback_add(button, "clicked", __entry_clear_button_clicked_cb, entry);
    elm_object_part_content_set(editfield, "entry_clear_button", button);

    if (!elm_entry_is_empty(entry)) {
        BROWSER_LOGE("entry is empty");
        elm_object_signal_emit(editfield, "show,clear,button,signal", "");
    }

    elm_object_part_content_set(layout, "entry_swallow", editfield);
    evas_object_show(layout);
}

void SettingsAFCreator::addItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    // full name
    m_fullNameItemCallbackData.type = profile_composer_title_full_name;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_FULL_NAME_ABB")), &m_fullNameItemCallbackData);
    elm_box_pack_end(m_box, m_fullNameItemCallbackData.it);
    if (m_item->getName() && strlen(m_item->getName()))
        elm_object_part_text_set(m_fullNameItemCallbackData.entry, "elm.text", m_item->getName());

    // company_name
    m_companyNameItemCallbackData.type = profile_composer_title_company_name;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_COMPANY_NAME_ABB")), &m_companyNameItemCallbackData);
    elm_box_pack_end(m_box, m_companyNameItemCallbackData.it);
    if (m_item->getCompany() && strlen(m_item->getCompany()))
        elm_object_part_text_set(m_companyNameItemCallbackData.entry, "elm.text", m_item->getCompany());

    // address 1
    m_addressLine1ItemCallbackData.type = profile_composer_title_address_line_1;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_ADDRESS_LINE_1_ABB")), &m_addressLine1ItemCallbackData);
    elm_box_pack_end(m_box, m_addressLine1ItemCallbackData.it);
    if (m_item->getPrimaryAddress() && strlen(m_item->getPrimaryAddress()))
        elm_object_part_text_set(m_addressLine1ItemCallbackData.entry, "elm.text", m_item->getPrimaryAddress());

    // address 2
    m_addressLine2ItemCallbackData.type = profile_composer_title_address_line_2;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_ADDRESS_LINE_2_ABB")), &m_addressLine2ItemCallbackData);
    elm_box_pack_end(m_box, m_addressLine2ItemCallbackData.it);
    if (m_item->getSecondaryAddress2() && strlen(m_item->getSecondaryAddress2()))
        elm_object_part_text_set(m_addressLine2ItemCallbackData.entry, "elm.text", m_item->getSecondaryAddress2());

    // city town
    m_cityTownItemCallbackData.type = profile_composer_title_city_town;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_CITY_TOWN_ABB")), &m_cityTownItemCallbackData);
    elm_box_pack_end(m_box, m_cityTownItemCallbackData.it);
    if (m_item->getCityTown() && strlen(m_item->getCityTown()))
        elm_object_part_text_set(m_cityTownItemCallbackData.entry, "elm.text", m_item->getCityTown());

    // country
    m_countryItemCallbackData.type = profile_composer_title_country;
    createInputLayout(m_box, strdup("Country"), &m_countryItemCallbackData);
    elm_box_pack_end(m_box, m_countryItemCallbackData.it);
    if (m_item->getCountry() && strlen(m_item->getCountry()))
        elm_object_part_text_set(m_countryItemCallbackData.entry, "elm.text", m_item->getCountry());

    // post code
    m_postCodeItemCallbackData.type = profile_composer_title_post_code;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_POSTCODE_ABB")), &m_postCodeItemCallbackData);
    elm_box_pack_end(m_box, m_postCodeItemCallbackData.it);
    Elm_Entry_Filter_Limit_Size m_entryLimitSize;
    Elm_Entry_Filter_Accept_Set m_entry_accept_set;
    m_entryLimitSize.max_char_count = 10;
    m_entry_accept_set.accepted = "0123456789";
    m_entry_accept_set.rejected = NULL;
    elm_entry_markup_filter_append(m_postCodeItemCallbackData.entry, elm_entry_filter_limit_size, &m_entryLimitSize);
    elm_entry_markup_filter_append(m_postCodeItemCallbackData.entry, elm_entry_filter_accept_set, &m_entry_accept_set);
    if (m_item->getPostCode() && strlen(m_item->getPostCode()))
        elm_object_part_text_set(m_postCodeItemCallbackData.entry, "elm.text", m_item->getPostCode());
    elm_entry_input_panel_layout_set(m_postCodeItemCallbackData.entry, ELM_INPUT_PANEL_LAYOUT_NUMBERONLY);
    elm_entry_prediction_allow_set(m_postCodeItemCallbackData.entry, EINA_FALSE);

    // country region
    m_countyRegionItemCallbackData.type = profile_composer_title_country_region;
    createInputLayout(m_box, strdup(_("IDS_BR_MBODY_COUNTRY_REGION")), &m_countyRegionItemCallbackData);
    elm_box_pack_end(m_box, m_countyRegionItemCallbackData.it);
    if (m_item->getStateProvince() && strlen(m_item->getStateProvince()))
        elm_object_part_text_set(m_countyRegionItemCallbackData.entry, "elm.text", m_item->getStateProvince());

    // phone
    m_phoneItemCallbackData.type = profile_composer_title_phone;
    createInputLayout(m_box, strdup(_("IDS_BR_BODY_PHONE")), &m_phoneItemCallbackData);
    elm_box_pack_end(m_box, m_phoneItemCallbackData.it);
    if (m_item->getPhoneNumber() && strlen(m_item->getPhoneNumber()))
        elm_object_part_text_set(m_phoneItemCallbackData.entry, "elm.text", m_item->getPhoneNumber());
    Elm_Entry_Filter_Accept_Set entry_accept_set;
    entry_accept_set.accepted = PHONE_FIELD_VALID_ENTRIES;
    entry_accept_set.rejected = NULL;
    elm_entry_markup_filter_append(m_phoneItemCallbackData.entry, elm_entry_filter_accept_set, &entry_accept_set);
    elm_entry_input_panel_layout_set(m_phoneItemCallbackData.entry, ELM_INPUT_PANEL_LAYOUT_PHONENUMBER);
    elm_entry_prediction_allow_set(m_phoneItemCallbackData.entry, EINA_FALSE);

    // email
    m_emailItemCallbackData.type = profile_composer_title_email;
    createInputLayout(m_box, strdup(_("IDS_BR_OPT_SENDURLVIA_EMAIL")), &m_emailItemCallbackData);
    elm_box_pack_end(m_box, m_emailItemCallbackData.it);
    if (m_item->getEmailAddress() && strlen(m_item->getEmailAddress()))
        elm_object_part_text_set(m_emailItemCallbackData.entry, "elm.text", m_item->getEmailAddress());
    elm_entry_input_panel_return_key_type_set(m_emailItemCallbackData.entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
    evas_object_smart_callback_add(m_emailItemCallbackData.entry, "activated", __done_button_cb, this);
    elm_entry_input_panel_layout_set(m_emailItemCallbackData.entry, ELM_INPUT_PANEL_LAYOUT_EMAIL);
    elm_entry_prediction_allow_set(m_emailItemCallbackData.entry, EINA_FALSE);
}

Evas_Object* SettingsAFCreator::createScroller(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *scroller = elm_scroller_add(parent);
    if (!scroller) {
        BROWSER_LOGE("elm_scroller_add failed");
        return nullptr;
    }

    m_box = elm_box_add(scroller);
    evas_object_size_hint_weight_set(m_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_box_align_set(m_box, 0.0, 0.0);
    elm_object_content_set(scroller, m_box);

    addItems();

    return scroller;
}

Eina_Bool SettingsAFCreator::isEntryHasOnlySpace(const char* field)
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

Eina_Bool SettingsAFCreator::applyEntryData(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    const char *full_name = elm_entry_entry_get(m_fullNameItemCallbackData.entry);

    if (!full_name)
        return EINA_FALSE;

    std::string full_name_str = std::string(full_name);
    full_name_str = _trim(full_name_str);
    full_name = full_name_str.c_str();

    if (full_name && strlen(full_name) && !isEntryHasOnlySpace(full_name))
        m_item->setName(full_name);
    else {
        elm_object_focus_set(m_cancelButton, EINA_TRUE); // Closing virtual keyboard by changing the focus*/
        return EINA_FALSE;
    }
    const char *company_name = elm_entry_entry_get(m_companyNameItemCallbackData.entry);
    m_item->setCompany(company_name);
    const char *primary_address = elm_entry_entry_get(m_addressLine1ItemCallbackData.entry);
    m_item->setPrimaryAddress(primary_address);
    const char *secondary_address = elm_entry_entry_get(m_addressLine2ItemCallbackData.entry);
    m_item->setSecondaryAddress2(secondary_address);
    const char *city_town = elm_entry_entry_get(m_cityTownItemCallbackData.entry);
    m_item->setCityTown(city_town);
    const char *county = elm_entry_entry_get(m_countryItemCallbackData.entry);
    m_item->setStateProvince(county);
    const char *post_code = elm_entry_entry_get(m_postCodeItemCallbackData.entry);
    m_item->setPostCode(post_code);
    const char *country = elm_entry_entry_get(m_countyRegionItemCallbackData.entry);
    m_item->setCountry(country);
    const char *phone = elm_entry_entry_get(m_phoneItemCallbackData.entry);
    m_item->setPhoneNumber(phone);
    const char *email = elm_entry_entry_get(m_emailItemCallbackData.entry);
    m_item->setEmailAddress(email);

    if (m_item->getItemComposeMode() == profile_edit) {

        m_editErrorcode = m_item->updateItem();
        if (m_editErrorcode == profile_edit_failed || m_editErrorcode == profile_already_exist) {
            BROWSER_LOGD("Update failed!");
            return EINA_FALSE;
        }
    } else {
        m_saveErrorcode = m_item->saveItem();
        if (m_saveErrorcode != profile_create_failed && m_saveErrorcode != duplicate_profile){
            BROWSER_LOGD("Cannot save autofill data");
            return EINA_FALSE;
        }
    }
    return EINA_TRUE;
}

void SettingsAFCreator::__editfield_changed_cb(void* data, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    Evas_Object *editfield = static_cast<Evas_Object*>(data);

    if (!elm_entry_is_empty(obj) && elm_object_focus_get(obj))
        elm_object_signal_emit(editfield, "show,clear,button,signal", "");
    else
        elm_object_signal_emit(editfield, "hide,clear,button,signal", "");
}

void SettingsAFCreator::__done_button_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    SettingsAFCreator *safc = static_cast<SettingsAFCreator*>(data);
    if (safc->applyEntryData() == EINA_FALSE)
        return;
    SettingsPrettySignalConnector::Instance().closeSettingsUIClicked();
}

void SettingsAFCreator::__cancel_button_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SettingsPrettySignalConnector::Instance().closeSettingsUIClicked();
}

void SettingsAFCreator::__entry_changed_cb(void* data, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    genlistCallbackData *cb_data = static_cast<genlistCallbackData*>(data);
    SettingsAFCreator *view = static_cast<SettingsAFCreator*>(cb_data->user_data);
    const char* text = elm_entry_entry_get(obj);
    if (text && strlen(text) > 0) {
        elm_object_signal_emit(cb_data->editfield, "show,clear,button,signal", "");
    }
    else {
        elm_object_signal_emit(cb_data->editfield, "hide,clear,button,signal", "");
    }

    if (!elm_entry_is_empty(view->m_fullNameItemCallbackData.entry)) {
        elm_object_signal_emit(view->m_actionBar, "show,buttons,signal", "but_vis");
        elm_object_disabled_set(view->m_doneButton, EINA_FALSE);
    } else {
        elm_object_signal_emit(view->m_actionBar, "dim,done,button,signal", "but_vis");
        elm_object_disabled_set(view->m_doneButton, EINA_TRUE);
    }

}

void SettingsAFCreator::__entry_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    Elm_Object_Item *item = callback_data->it;
    if (!item)
        BROWSER_LOGD("Item is null");
    elm_object_focus_set(callback_data->editfield, EINA_TRUE);
}

void SettingsAFCreator::__entry_clear_button_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    Evas_Object *entry = static_cast<Evas_Object*>(data);
    elm_entry_entry_set(entry, "");
}

void SettingsAFCreator::__entry_next_key_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data) {
        BROWSER_LOGE("data is null");
        return;
    }
    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    SettingsAFCreator *self = static_cast<SettingsAFCreator*>(callback_data->user_data);
    SettingsAFCreator::menu_type type = callback_data->type;
    Evas_Object *entry = nullptr;

    if (type == profile_composer_title_full_name) {
        entry = self->m_companyNameItemCallbackData.entry;
    } else if (type == profile_composer_title_company_name) {
        entry = self->m_addressLine1ItemCallbackData.entry;
    } else if (type == profile_composer_title_address_line_1) {
        entry = self->m_addressLine2ItemCallbackData.entry;
    } else if (type == profile_composer_title_address_line_2) {
        entry = self->m_cityTownItemCallbackData.entry;
    } else if (type == profile_composer_title_city_town) {
        entry = self->m_countryItemCallbackData.entry;
    } else if (type == profile_composer_title_country) {
        entry = self->m_postCodeItemCallbackData.entry;
    } else if (type == profile_composer_title_post_code) {
        entry = self->m_countyRegionItemCallbackData.entry;
    } else if (type == profile_composer_title_country_region) {
        entry = self->m_phoneItemCallbackData.entry;
    } else if (type == profile_composer_title_phone) {
        entry = self->m_emailItemCallbackData.entry;
    } else if (type == profile_composer_title_email) {
        BROWSER_LOGD("[%s:%d] It's last item to go", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    elm_object_focus_set(entry, EINA_TRUE);
    elm_entry_cursor_end_set(entry);
}
}
}
