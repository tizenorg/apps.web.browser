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

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "SettingsUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(SettingsUI, "org.tizen.browser.settingsui")

struct ItemData{
        tizen_browser::base_ui::SettingsUI* settingsUI;
        Elm_Object_Item * e_item;
};

SettingsUI::SettingsUI()
    : m_settings_layout(NULL)
    , m_genListActionBar(NULL)
    , m_parent(NULL)
    , m_itemClassActionBar(NULL)
    , m_item_class(NULL)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    edjFilePath = EDJE_DIR;
    edjFilePath.append("SettingsUI/SettingsUI.edj");
}

SettingsUI::~SettingsUI()
{

}

void SettingsUI::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //m_parent = p;
    elm_theme_extension_add(NULL, edjFilePath.c_str());
    m_settings_layout = elm_layout_add(parent);
    elm_layout_file_set(m_settings_layout, edjFilePath.c_str(), "settings-layout");
    evas_object_size_hint_weight_set(m_settings_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_settings_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_settings_layout);

    showActionBar();
    showSettingsGenlist();
}


void SettingsUI::showActionBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(NULL, edjFilePath.c_str());
    m_genListActionBar = elm_genlist_add(m_settings_layout);
    elm_object_part_content_set(m_settings_layout, "actionbar_swallow", m_genListActionBar);
    elm_genlist_homogeneous_set(m_genListActionBar, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListActionBar, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListActionBar, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListActionBar, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genListActionBar, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genListActionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    m_itemClassActionBar = elm_genlist_item_class_new();
    m_itemClassActionBar->item_style = "settings_action_bar_items";
    m_itemClassActionBar->func.text_get = NULL;
    m_itemClassActionBar->func.content_get = &listActionBarContentGet;
    m_itemClassActionBar->func.state_get = 0;
    m_itemClassActionBar->func.del = 0;

    ItemData * id = new ItemData;
    id->settingsUI = this;
    Elm_Object_Item* elmItem = elm_genlist_item_append(m_genListActionBar,            //genlist
                                                       m_itemClassActionBar,          //item Class
                                                      id,
                                                      NULL,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,//item type
                                                      NULL,
                                                      NULL                  //data passed to above function
                                                     );
    id->e_item = elmItem; 

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

}

void SettingsUI::showSettingsGenlist()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(NULL, edjFilePath.c_str());
    m_genList = elm_genlist_add(m_settings_layout);
    elm_object_part_content_set(m_settings_layout, "settings_genlist_swallow", m_genList);
    elm_genlist_homogeneous_set(m_genList, EINA_FALSE);
    elm_genlist_multi_select_set(m_genList, EINA_FALSE);
    elm_genlist_select_mode_set(m_genList, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genList, ELM_LIST_SCROLL);
    elm_genlist_decorate_mode_set(m_genList, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_genlist_scroller_policy_set(m_genList, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_AUTO);

    m_item_class = elm_genlist_item_class_new();
    m_item_class->item_style = "settings_items";
    m_item_class->func.text_get = NULL;
    m_item_class->func.content_get = &listSettingsGenlistContentGet;
    m_item_class->func.state_get = 0;
    m_item_class->func.del = 0;

    ItemData * id = new ItemData;
    id->settingsUI = this;
    Elm_Object_Item* elmItem = elm_genlist_item_append(m_genList,            //genlist
                                                       m_item_class,          //item Class
                                                      id,
                                                      NULL,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,//item type
                                                      NULL,
                                                      NULL                  //data passed to above function
                                                     );
    id->e_item = elmItem;

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

}

Evas_Object* SettingsUI::listActionBarContentGet(void* data, Evas_Object* obj , const char* part)
{
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        if(!strcmp(part, "close_click"))
	{
		Evas_Object *close_click = elm_button_add(obj);
		elm_object_style_set(close_click, "basic_button");
		evas_object_smart_callback_add(close_click, "clicked", SettingsUI::close_clicked_cb, data);
		return close_click;
	}
        return NULL;
}

Evas_Object* SettingsUI::listSettingsGenlistContentGet(void* data, Evas_Object* obj , const char* part)
{
        BROWSER_LOGD("[%s:%d] Part %s", __PRETTY_FUNCTION__, __LINE__, part);
        if(!strcmp(part, "del_selected_data_click"))
        {
                Evas_Object *button = elm_button_add(obj);
                elm_object_style_set(button, "basic_button");
                evas_object_smart_callback_add(button, "clicked", SettingsUI::_del_selected_data_clicked_cb, data);
                return button;
        }
	else if(!strcmp(part, "reset_mv_click"))
        {
                Evas_Object *button = elm_button_add(obj);
                elm_object_style_set(button, "basic_button");
                evas_object_smart_callback_add(button, "clicked", SettingsUI::_reset_mv_clicked_cb, data);
                return button;
        }
	else if(!strcmp(part, "reset_browser_click"))
        {
                Evas_Object *button = elm_button_add(obj);
                elm_object_style_set(button, "basic_button");
                evas_object_smart_callback_add(button, "clicked", SettingsUI::_reset_browser_clicked_cb, data);
                return button;
        }
	else if(!strcmp(part, "cache_cb") || !strcmp(part, "cookies_cb") || !strcmp(part, "history_cb"))
        {
		Evas_Object *checkbox = elm_check_add(obj);
                elm_object_style_set(checkbox, "on&off");
                evas_object_smart_callback_add(checkbox, "changed", __check_changed_cb, data);
                elm_check_state_set(checkbox, EINA_TRUE);
                return checkbox;
	}
	else if(!strcmp(part, "accept_all_rb") || !strcmp(part, "ask_rb") || !strcmp(part, "sr_disable_rb") || !strcmp(part, "bs_enable_rb")
		|| !strcmp(part, "bs_disable_rb") || !strcmp(part, "ts_enable_rb") || !strcmp(part, "ts_disable_rb"))
        {
                Evas_Object *rb = elm_radio_add(obj);
                elm_object_style_set(rb, "on&off");
                //evas_object_smart_callback_add(rb, "changed", __radio_changed_cb, data);
                return rb;
        }
        return NULL;
}

void SettingsUI::__check_changed_cb(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //ItemData * id = static_cast<ItemData *>(data);
}

void SettingsUI::close_clicked_cb(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData * id = static_cast<ItemData *>(data);
    id->settingsUI->closeSettingsUIClicked(std::string());
    id->settingsUI->clearItems();
}

void SettingsUI::hide()
{
        evas_object_hide(elm_layout_content_get(m_settings_layout, "action_bar_swallow"));
        evas_object_hide(elm_layout_content_get(m_settings_layout, "settings_genlist_swallow"));
        evas_object_hide(m_settings_layout);
}

void SettingsUI::_del_selected_data_clicked_cb(void * data, Evas_Object * /* obj */, void * event_info)
{
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
	ItemData* itemData = reinterpret_cast<ItemData *>(data);
	Evas_Object *cb1 = elm_object_item_part_content_get(itemData->e_item, "cache_cb");
	Evas_Object *cb2 = elm_object_item_part_content_get(itemData->e_item, "cookies_cb");
	Evas_Object *cb3 = elm_object_item_part_content_get(itemData->e_item, "history_cb");
	std::string type;
	elm_check_state_get(cb1) ? type += "_CACHE" : "";
	elm_check_state_get(cb2) ? type += "_COOKIES" : "";
	elm_check_state_get(cb3) ? type += "_HISTORY" : "";
        itemData->settingsUI->deleteSelectedDataClicked(type);
}

void SettingsUI::_reset_mv_clicked_cb(void * data, Evas_Object * /* obj */, void * event_info)
{
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
	ItemData* itemData = reinterpret_cast<ItemData *>(data);
        itemData->settingsUI->resetMostVisitedClicked(std::string());
}

void SettingsUI::_reset_browser_clicked_cb(void * data, Evas_Object * /* obj */, void * event_info)
{
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        ItemData* itemData = reinterpret_cast<ItemData *>(data);
        itemData->settingsUI->resetBrowserClicked(std::string());
}

void SettingsUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    hide();
    elm_genlist_clear(m_genListActionBar);
    elm_genlist_clear(m_genList);
}

void SettingsUI::focusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,in", "over2");

    // selected manually
    elm_gengrid_item_selected_set(item, EINA_TRUE);
}

void SettingsUI::unFocusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,out", "over2");

    // unselected manually
    elm_gengrid_item_selected_set(item, EINA_FALSE);
}

}
}
