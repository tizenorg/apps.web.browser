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
#include "AutoFillFormListView.h"
#include "AutoFillFormItem.h"
#include "AutoProfileDeleteView.h"
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

AutoFillFormListView::AutoFillFormListView(AutoFillFormManager *affm)
    : m_manager(affm)
    , m_mainLayout(nullptr)
    , m_genlist(nullptr)
    , m_itemClass(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SettingsUI/AutoFillMobileUI.edj");
}

AutoFillFormListView::~AutoFillFormListView(void)
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

Eina_Bool AutoFillFormListView::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_mainLayout = createMainLayout(parent);
    if (!m_mainLayout) {
        BROWSER_LOGE("createMainLayout failed");
        return EINA_FALSE;
    }

    Evas_Object* back_button = elm_button_add(m_mainLayout);
    if (!back_button) {
        BROWSER_LOGE("Failed to create back_button");
        return EINA_FALSE;
    }
    elm_object_style_set(back_button, "basic_button");
    evas_object_smart_callback_add(back_button, "clicked", __back_button_cb, this);
    elm_object_part_content_set(m_mainLayout, "back_button", back_button);

    Evas_Object *add_btn = elm_button_add(m_mainLayout);
    elm_object_style_set(add_btn, "basic_button");
    evas_object_smart_callback_add(add_btn, "clicked", __add_profile_button_cb, this);
    elm_object_part_content_set(m_mainLayout, "add_profile_button", add_btn);

    Evas_Object *del_btn = elm_button_add(m_mainLayout);
    elm_object_style_set(del_btn, "basic_button");
    evas_object_smart_callback_add(del_btn, "clicked", __delete_profile_button_cb, this);
    elm_object_part_content_set(m_mainLayout, "delete_profile_button", del_btn);

    evas_object_show(m_mainLayout);

    return EINA_TRUE;
}

void AutoFillFormListView::refreshView(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_genlist_clear(m_genlist);
    appendGenlist(m_genlist);
}

void AutoFillFormListView::rotateLandscape()
{
    if(m_mainLayout)
        elm_object_signal_emit(m_mainLayout,"rotation,landscape", "rot");
}

void AutoFillFormListView::rotatePortrait()
{
    if(m_mainLayout)
        elm_object_signal_emit(m_mainLayout,"rotation,portrait", "rot");
}

Evas_Object *AutoFillFormListView::createMainLayout(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *layout = elm_layout_add(parent);
    if (!layout) {
        BROWSER_LOGD("elm_layout_add failed");
        return NULL;
    }
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "afflv-layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(layout, "title_text", "IDS_BR_BODY_AUTO_FILL_FORMS_T_TTS");
    elm_object_translatable_part_text_set(layout, "profile_text", "IDS_BR_HEADER_PROFILES");
    elm_object_translatable_part_text_set(layout, "add_profile_text", "IDS_BR_OPT_ADD");
    elm_object_translatable_part_text_set(layout, "delete_profile_text", "IDS_BR_SK_DELETE_ABB");

    m_genlist = createGenlist(layout);
    if (!m_genlist) {
        BROWSER_LOGE("elm_genlist_add failed");
        return NULL;
    }
    evas_object_show(m_genlist);
    elm_object_part_content_set(layout, "afflv_genlist", m_genlist);

    return layout;
}

Evas_Object *AutoFillFormListView::createGenlist(Evas_Object *parent)
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
    m_itemClass->item_style = "afflv_item";
    m_itemClass->func.content_get = NULL;

    m_itemClass->func.text_get = __text_get_cb;
    m_itemClass->func.state_get = NULL;
    m_itemClass->func.del = NULL;

    appendGenlist(genlist);

    return genlist;
}

const char *AutoFillFormListView::getEachItemFullName(unsigned int index)
{
    if (m_manager->getAutoFillFormItemCount() == 0)
        return NULL;
    return (m_manager->getItemList())[index]->getName();
}

Eina_Bool AutoFillFormListView::appendGenlist(Evas_Object *genlist)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    unsigned int item_count = m_manager->getAutoFillFormItemCount();
    BROWSER_LOGD("Item count : [%d]", item_count);
    if (item_count > 0) {
        for (unsigned int i = 0; i < item_count; i++) {
            genlistCallbackData* item_callback_data = new genlistCallbackData;
            item_callback_data->menu_index = i;
            item_callback_data->user_data = this;
            item_callback_data->it = elm_genlist_item_append(genlist, m_itemClass,
            item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, item_callback_data);
        }
        elm_object_signal_emit(m_mainLayout, "show,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(m_mainLayout, "del_button"), false);
    }
    else {
        elm_object_signal_emit(m_mainLayout, "dim,del,button,signal", "");
        elm_object_disabled_set(elm_object_part_content_get(m_mainLayout, "del_button"), true);
    }

    return EINA_TRUE;
}

char *AutoFillFormListView::__text_get_cb(void* data, Evas_Object* /*obj*/, const char *part)
{
    BROWSER_LOGD("part[%s]", part);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    AutoFillFormListView *view = static_cast<AutoFillFormListView*>(callback_data->user_data);

    if (!strcmp(part, "item_title")) {
        const char *item_full_name = view->getEachItemFullName((unsigned int)callback_data->menu_index);
        if (item_full_name)
           return strdup(item_full_name);
    }
    return NULL;

}

void AutoFillFormListView::__add_profile_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    /* create new profile */
    AutoFillFormListView *list_view = static_cast<AutoFillFormListView*>(data);
    list_view->m_manager->showComposer();
}

void AutoFillFormListView::__delete_profile_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoFillFormListView *afflv = static_cast<AutoFillFormListView*>(data);
    afflv->m_manager->showDeleteView();
}

void AutoFillFormListView::__back_button_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    AutoFillFormListView *list_view = static_cast<AutoFillFormListView*>(data);
    list_view->hide();
    list_view->m_manager->listViewBackClicked();
}

void AutoFillFormListView::__genlist_item_clicked_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    genlistCallbackData *callback_data = static_cast<genlistCallbackData*>(data);
    AutoFillFormListView *view = static_cast<AutoFillFormListView*>(callback_data->user_data);

    elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
    view->m_manager->showComposer((view->m_manager->getItemList())[callback_data->menu_index]);
}

void AutoFillFormListView::hide()
{
    evas_object_hide(m_mainLayout);
}

}
}
