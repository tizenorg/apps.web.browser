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
#include "AutoProfileDeleteView.h"
#include "AutoFillFormListView.h"
#include "AutoFillFormItem.h"
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

AutoProfileDeleteView::AutoProfileDeleteView(AutoFillFormManager* manager)
    : m_manager(manager)
    , m_mainLayout(NULL)
    , m_genlist(NULL)
    , m_checked_count(0)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/AutoFillMobileUI.edj");
}

AutoProfileDeleteView::~AutoProfileDeleteView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_genlist) {
        elm_genlist_clear(m_genlist);
        evas_object_del(m_genlist);
    }
    if (m_mainLayout) {
        evas_object_hide(m_mainLayout);
        evas_object_del(m_mainLayout);
    }
}

void AutoProfileDeleteView::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);


    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_mainLayout = createMainLayout(parent);
    if (!m_mainLayout) {
        BROWSER_LOGE("createMainLayout failed");
        return;
    }

    Evas_Object* back_button = elm_button_add(m_mainLayout);
    if (!back_button) {
        BROWSER_LOGE("Failed to create back_button");
        return;
    }
    elm_object_style_set(back_button, "basic_button");
    evas_object_smart_callback_add(back_button, "clicked", __back_button_cb, this);
    elm_object_part_content_set(m_mainLayout, "back_button", back_button);

    Evas_Object *del_btn = elm_button_add(m_mainLayout);
    elm_object_style_set(del_btn, "basic_button");
    evas_object_smart_callback_add(del_btn, "clicked", __delete_button_cb, this);
    elm_object_part_content_set(m_mainLayout, "del_button", del_btn);

    elm_object_signal_emit(m_mainLayout, "dim,del,button,signal", "");
    elm_object_disabled_set(del_btn, true);

    evas_object_show(m_mainLayout);
}

Evas_Object *AutoProfileDeleteView::createMainLayout(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *layout = elm_layout_add(parent);
    if (!layout) {
        BROWSER_LOGD("elm_layout_add failed");
        return NULL;
    }
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "affdv-layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(layout, "select_all_text", "IDS_BR_OPT_SELECT_ALL");
    elm_object_translatable_part_text_set(layout, "del_text", "IDS_BR_SK_DELETE");

    Evas_Object* checkbox = elm_check_add(layout);
    if (!checkbox) {
        BROWSER_LOGE("Failed to add check");
        return NULL;
    }
    elm_object_style_set(checkbox, "custom_check");
    Eina_Bool checked = false;
    elm_check_state_pointer_set(checkbox, &checked);
    evas_object_propagate_events_set(checkbox, EINA_FALSE);
    elm_object_part_content_set(layout, "select_all_checkbox", checkbox);

    Evas_Object* button = elm_button_add(layout);
    if (!button) {
        BROWSER_LOGE("Failed to add button");
        return NULL;
    }
    elm_object_style_set(button, "basic_button");
    evas_object_smart_callback_add(button, "clicked", __select_all_checkbox_changed_cb, this);
    elm_object_part_content_set(layout, "select_all_checkbox_button", button);

    m_genlist = createGenlist(layout);
    if (!m_genlist) {
        BROWSER_LOGE("elm_genlist_add failed");
        return NULL;
    }
    evas_object_show(m_genlist);
    elm_object_part_content_set(layout, "affdv_genlist", m_genlist);

    return layout;
}

Evas_Object *AutoProfileDeleteView::createGenlist(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *genlist = elm_genlist_add(parent);
    if (!genlist) {
        BROWSER_LOGE("elm_genlist_add failed");
        return NULL;
    }

    m_itemClass = elm_genlist_item_class_new();
    if (!m_itemClass) {
        BROWSER_LOGE("elm_genlist_item_class_new for description_item_class failed");
        return EINA_FALSE;
    }
    m_itemClass->item_style = "affdv_item";
    m_itemClass->func.content_get = __content_get_cb;

    m_itemClass->func.text_get = __text_get_cb;
    m_itemClass->func.state_get = NULL;
    m_itemClass->func.del = NULL;

    appendGenlist(genlist);

    return genlist;
}

Eina_Bool AutoProfileDeleteView::appendGenlist(Evas_Object *genlist)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    unsigned int item_count = m_manager->getAutoFillFormItemCount();
    if (item_count > 0) {
        for (unsigned int i = 0; i < item_count; i++) {
           genlistCallbackData* item_callback_data = new genlistCallbackData;
           item_callback_data->menu_index = i;
           item_callback_data->user_data = this;
           item_callback_data->it = elm_genlist_item_append(genlist, m_itemClass,
                                        item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_selected_cb, item_callback_data);
        }
    }

    return EINA_TRUE;
}

void AutoProfileDeleteView::__select_all_checkbox_changed_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoProfileDeleteView *apdv = static_cast<AutoProfileDeleteView*>(data);
    Evas_Object* sel_all_checkbox = elm_object_part_content_get(apdv->m_mainLayout, "select_all_checkbox");
    Eina_Bool sel_all_state = elm_check_state_get(sel_all_checkbox);
    elm_check_state_set(sel_all_checkbox, !sel_all_state);

    Elm_Object_Item *it = elm_genlist_first_item_get(apdv->m_genlist);
    while (it) {
         Evas_Object* checkbox = elm_object_item_part_content_get(it, "checkbox");
         Eina_Bool state = elm_check_state_get(checkbox);
         if (state == sel_all_state)
              elm_check_state_set(checkbox, !sel_all_state);
         it = elm_genlist_item_next_get(it);
    }

    if (sel_all_state)
        apdv->m_checked_count = 0;
    else
        apdv->m_checked_count = elm_genlist_items_count(apdv->m_genlist);

    if (apdv->m_checked_count >= 1) {
        elm_object_signal_emit(apdv->m_mainLayout, "show,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(apdv->m_mainLayout, "del_button"), false);
    }
    else {
        elm_object_signal_emit(apdv->m_mainLayout, "dim,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(apdv->m_mainLayout, "del_button"), true);
    }
}

void AutoProfileDeleteView::__back_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoProfileDeleteView *view = static_cast<AutoProfileDeleteView*>(data);
    view->hide();
}

void AutoProfileDeleteView::__genlist_item_selected_cb(void* data, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    AutoProfileDeleteView *apdv = static_cast<AutoProfileDeleteView*>(callback_data->user_data);

    Elm_Object_Item *item = static_cast<Elm_Object_Item*>(event_info);
    elm_genlist_item_selected_set(item, EINA_FALSE);
    Evas_Object *checkbox = elm_object_item_part_content_get(item, "checkbox");
    Eina_Bool state = elm_check_state_get(checkbox);
    elm_check_state_set(checkbox, !state);
    if (state)
        apdv->m_checked_count--;
    else
        apdv->m_checked_count++;

    BROWSER_LOGD("----Checked count : [%d]---- ", apdv->m_checked_count);
    Evas_Object* sel_all_checkbox = elm_object_part_content_get(apdv->m_mainLayout, "select_all_checkbox");
    if (apdv->m_checked_count == elm_genlist_items_count(apdv->m_genlist))
        elm_check_state_set(sel_all_checkbox, true);
    else
        elm_check_state_set(sel_all_checkbox, false);

    if (apdv->m_checked_count >= 1) {
        elm_object_signal_emit(apdv->m_mainLayout, "show,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(apdv->m_mainLayout, "del_button"), false);
    }
    else {
        elm_object_signal_emit(apdv->m_mainLayout, "dim,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(apdv->m_mainLayout, "del_button"), true);
    }
}

void AutoProfileDeleteView::__delete_button_cb(void* data,Evas_Object* /*obj*/,void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoProfileDeleteView *apdv = static_cast<AutoProfileDeleteView*>(data);
    if (apdv->m_checked_count == elm_genlist_items_count(apdv->m_genlist))
        apdv->deleteAllItems();
    else
        apdv->deleteSelectedItems();
}

void AutoProfileDeleteView::deleteAllItems(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_manager->deleteAllAutoFillFormItems();
    hide();
    m_manager->refreshListView();
}

void AutoProfileDeleteView::deleteSelectedItems(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    genlistCallbackData *cb_data = NULL;
    Elm_Object_Item *it = elm_genlist_first_item_get(m_genlist);
    Evas_Object *checkbox;
    int del_count =0;

    while (it) {
        checkbox = elm_object_item_part_content_get(it, "checkbox");
        cb_data = static_cast<genlistCallbackData*>(elm_object_item_data_get(it));
        if (elm_check_state_get(checkbox)) {
            AutoFillFormItem *item = m_manager->getItemList()[cb_data->menu_index - del_count];
            m_manager->deleteAutoFillFormItem(item);
            del_count++;
        }
        it = elm_genlist_item_next_get(it);
    }
    BROWSER_LOGD("Total items deleted %d",del_count);
    hide();
    m_manager->refreshListView();
}

char *AutoProfileDeleteView::__text_get_cb(void* data, Evas_Object* /*obj*/, const char *part)
{
    BROWSER_LOGD("part[%s]", part);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    AutoProfileDeleteView *view = static_cast<AutoProfileDeleteView*>(callback_data->user_data);

    if (!strcmp(part, "item_title")) {
        const char *item_full_name = view->getEachItemFullName((unsigned int)callback_data->menu_index);
        if (item_full_name)
           return strdup(item_full_name);
    }
    return NULL;
}

const char *AutoProfileDeleteView::getEachItemFullName(unsigned int index)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_manager->getAutoFillFormItemCount() == 0)
        return NULL;
    return (m_manager->getItemList())[index]->getName();
}

Evas_Object *AutoProfileDeleteView::__content_get_cb(void* /*data*/, Evas_Object* obj, const char *part)
{
    BROWSER_LOGD("part[%s]", part);

    if (!strcmp(part, "checkbox")) {
        Evas_Object *checkbox = elm_check_add(obj);
        if (!checkbox) {
            BROWSER_LOGE("Failed to add check");
            return NULL;
        }
        elm_object_style_set(checkbox, "custom_check");
        Eina_Bool checked = false;
        elm_check_state_pointer_set(checkbox, &checked);
        evas_object_propagate_events_set(checkbox, EINA_FALSE);
        return checkbox;
    }
    return NULL;
}

void AutoProfileDeleteView::hide(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(m_mainLayout);
}

}
}
