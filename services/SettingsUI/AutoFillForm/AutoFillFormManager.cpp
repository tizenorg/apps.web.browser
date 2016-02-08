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
#include "AutoFillFormItem.h"
#include "AutoFillFormListView.h"
#include "AutoProfileDeleteView.h"
#include "AutoFillFormComposeView.h"
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

AutoFillFormManager::AutoFillFormManager(void)
    : m_listView(NULL)
    , m_composer(NULL)
    , m_deleteView(NULL)
    , m_ewkContext(ewk_context_default_get())
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_AutoFillFormItemList = loadEntireItemList();
}

AutoFillFormManager::~AutoFillFormManager(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_listView)
        delete m_listView;
    m_listView = NULL;

    if (m_deleteView)
        delete m_deleteView;
    m_deleteView = NULL;

    if(m_composer)
        delete m_composer;
    m_composer = NULL;
}

void AutoFillFormManager::refreshListView()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    loadEntireItemList();
    if (m_listView)
        m_listView->refreshView();
}

std::vector<AutoFillFormItem *> AutoFillFormManager::loadEntireItemList(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_AutoFillFormItemList.clear();

    Eina_List *entire_item_list = ewk_context_form_autofill_profile_get_all(m_ewkContext);

    Eina_List *list = NULL;
    void *item_data = NULL;

    EINA_LIST_FOREACH(entire_item_list, list, item_data) {
        if (item_data) {
            Ewk_Autofill_Profile *profile = static_cast<Ewk_Autofill_Profile*>(item_data);
            AutoFillFormItem *item = createNewAutoFillFormItem(profile);
            if (item)
                m_AutoFillFormItemList.push_back(item);
        }
    }

    BROWSER_LOGD("----------- List size : [%d] ---------",  m_AutoFillFormItemList.size());
    return m_AutoFillFormItemList;
}

Eina_Bool AutoFillFormManager::addItemToList(AutoFillFormItem *item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_AutoFillFormItemList.push_back(item);
    return EINA_TRUE;
}

void AutoFillFormManager::rotateLandscape(){
    if(m_listView)
        m_listView->rotateLandscape();
    if(m_composer)
        m_composer->rotateLandscape();
    if(m_deleteView)
        m_deleteView->rotateLandscape();
}

void AutoFillFormManager::rotatePortrait(){
    if(m_listView)
        m_listView->rotatePortrait();
    if(m_composer)
        m_composer->rotatePortrait();
    if(m_deleteView)
        m_deleteView->rotatePortrait();
}

Eina_Bool AutoFillFormManager::deleteAutoFillFormItem(AutoFillFormItem *item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (unsigned int i = 0; i < m_AutoFillFormItemList.size(); i++) {
        if (m_AutoFillFormItemList[i]->getProfileId() == item->getProfileId()) {
            if (ewk_context_form_autofill_profile_remove(m_ewkContext, item->getProfileId()) == EINA_FALSE)
                return EINA_FALSE;

            m_AutoFillFormItemList.erase(m_AutoFillFormItemList.begin() + i);
        }
    }

    return EINA_TRUE;
}

Eina_Bool AutoFillFormManager::deleteAllAutoFillFormItems(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_AutoFillFormItemList.clear();

    Eina_List *entire_item_list = ewk_context_form_autofill_profile_get_all(m_ewkContext);

    Eina_List *list = NULL;
    void *item_data = NULL;

    EINA_LIST_FOREACH(entire_item_list, list, item_data) {
        if (item_data) {
            Ewk_Autofill_Profile *profile = static_cast<Ewk_Autofill_Profile*>(item_data);
            ewk_context_form_autofill_profile_remove(m_ewkContext, ewk_autofill_profile_id_get(profile));
        }
    }

    return EINA_TRUE;
}

unsigned int AutoFillFormManager::getAutoFillFormItemCount(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return m_AutoFillFormItemList.size();
}

AutoFillFormItem *AutoFillFormManager::createNewAutoFillFormItem(Ewk_Autofill_Profile *profile)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoFillFormItem *item = NULL;
    if (!profile)
        item = new AutoFillFormItem(NULL);
    else {
        AutoFillFormItemData *item_data = new AutoFillFormItemData;
        if (!item_data) {
            BROWSER_LOGE("Malloc failed to get item_data");
            return NULL;
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

        item = new AutoFillFormItem(item_data);
        free(item_data);
    }

    return item;
}

AutoFillFormListView *AutoFillFormManager::showListView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_listView)
        delete m_listView;

    m_listView = new AutoFillFormListView(this);
    m_listView->show(m_parent);

    return m_listView;
}

AutoFillFormComposeView *AutoFillFormManager::showComposer(AutoFillFormItem *item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_composer)
        delete m_composer;

    m_composer = new AutoFillFormComposeView(this, item);
    m_composer->show(m_parent);

    return m_composer;
}

AutoProfileDeleteView *AutoFillFormManager::showDeleteView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_deleteView)
        delete m_deleteView;

    m_deleteView = new AutoProfileDeleteView(this);
    m_deleteView->show(m_parent);

    return m_deleteView;
}

Eina_Bool AutoFillFormManager::deleteListView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_listView)
        delete m_listView;
    m_listView = NULL;

    return EINA_TRUE;
}

Eina_Bool AutoFillFormManager::deleteComposer(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_composer)
        delete m_composer;
    m_composer = NULL;

    return EINA_TRUE;
}

Eina_Bool AutoFillFormManager::deleteDeleteView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_deleteView)
        delete m_deleteView;
    m_deleteView = NULL;

    return EINA_TRUE;
}


void AutoFillFormManager::seeAllData(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    for (unsigned int i = 0; i < m_AutoFillFormItemList.size(); i++) {
        BROWSER_LOGD("m_AutoFillFormItemList[%d] item - start", i);
        BROWSER_LOGD("************************************************************************************");
        BROWSER_LOGD("m_AutoFillFormItemList[%d].id[%d]", i, m_AutoFillFormItemList[i]->getProfileId());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_name[%s]", i, m_AutoFillFormItemList[i]->getName());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_company[%s]", i, m_AutoFillFormItemList[i]->getCompany());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_primary_address[%s]", i, m_AutoFillFormItemList[i]->getPrimaryAddress());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_secondary_address[%s]", i, m_AutoFillFormItemList[i]->getSecondaryAddress2());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_city_town[%s]", i, m_AutoFillFormItemList[i]->getCityTown());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_state_province_region[%s]", i, m_AutoFillFormItemList[i]->getStateProvince());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_post_code[%s]", i, m_AutoFillFormItemList[i]->getPostCode());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_country_region[%s]", i, m_AutoFillFormItemList[i]->getCountry());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_phone_number[%s]", i, m_AutoFillFormItemList[i]->getPhoneNumber());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_email_address[%s]", i, m_AutoFillFormItemList[i]->getEmailAddress());
        BROWSER_LOGD("m_AutoFillFormItemList[%d].m_activation[%d]", i, m_AutoFillFormItemList[i]->getActivation());
        BROWSER_LOGD("************************************************************************************");
        BROWSER_LOGD("m_AutoFillFormItemList[%d] item - end", i);
    }

    return;
}

}
}
