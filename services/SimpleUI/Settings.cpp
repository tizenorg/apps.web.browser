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

#include "Settings.h"
#include <memory>
#include <string>
#include <iostream>
#include <Eina.h>
#include <Evas.h>
#include "BrowserLogger.h"

namespace tizen_browser
{
namespace base_ui
{

SettingsItem::SettingsItem()
    : action(std::make_shared<Action>())
{

}
SettingsItem::SettingsItem( std::shared_ptr<Action> action)
    : action(action)
{

}


SettingsItem::SettingsItem(const SettingsItem& source)
    : action(source.action)
{

}
SettingsItem::SettingsItem(const SettingsItem* source)
    : action(source->action)
{

}


SettingsItem::~SettingsItem()
{
    //elm_genlist_item_class_free(m_itemClass);??
}

Settings::Settings(std::shared_ptr< Evas_Object > mainWindow, Evas_Object* parentButton)
    : MenuButton(mainWindow, parentButton)
    , m_genlist(0)
    , m_itemClass(0)
    , m_internalPopup(0)
    , m_internalPopupVisible(false)
    , m_pointerModeEnabled(true)
{
    std::string edjFilePath = EDJE_DIR;
    edjFilePath.append("SimpleUI/Settings.edj");
    elm_theme_extension_add(0, edjFilePath.c_str());

    m_genlist = elm_genlist_add(m_window.get());

    evas_object_smart_callback_add(m_genlist, "item,focused", focusItem, NULL);
    evas_object_smart_callback_add(m_genlist, "item,unfocused", unFocusItem, NULL);

    elm_object_style_set(m_genlist, "settings");
    elm_genlist_homogeneous_set(m_genlist, EINA_TRUE);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);

    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);

    m_itemClass = elm_genlist_item_class_new();
    m_itemClass->item_style = "settings_item";

    m_itemClass->func.text_get = &gridTextGet;
    m_itemClass->func.content_get = &gridOptionLabelGet;
    m_itemClass->func.state_get = 0;
    m_itemClass->func.del = 0;
}

Settings::~Settings()
{

}

Evas_Object* Settings::getContent()
{
    return m_genlist;
}

tizen_browser::base_ui::MenuButton::ListSize Settings::calculateSize()
{
    ListSize result;

    result.width = 490;
    result.height = 426;

    return result;
}

Evas_Object* Settings::getFirstFocus()
{
    return m_genlist;
}

void Settings::addAction(sharedAction action)
{
    std::shared_ptr<SettingsItem> settingsItem = std::make_shared<SettingsItem>(action);
    BROWSER_LOGD("[%s] %x: %s", __func__ , settingsItem.get(), settingsItem->action->getText().c_str() );
    Elm_Object_Item* elmItem= elm_genlist_item_append(m_genlist,            //genlist
                                                      m_itemClass,          //item Class
                                                      settingsItem.get(),        //item data
                                                      0,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,//item type
                                                      itemClicked,
                                                      this
                                                     );
    m_settingsItemsMap[elmItem] = settingsItem;
    BROWSER_LOGD("[%s:%d] \n\t %x:%s ", __PRETTY_FUNCTION__, __LINE__, elmItem, settingsItem->action->getText().c_str());
    if(!settingsItem->action->isEnabled()){
        elm_object_item_disabled_set(elmItem, EINA_TRUE);
    }
    if(settingsItem->action->isCheckable()){
        elm_object_item_signal_emit(elmItem,
                          (settingsItem->action->isChecked() ? "switch,on" : "switch,off")
                          , "SettingsModel");
    } else {
        elm_object_item_signal_emit(elmItem, "switch,hide", "SettingsModel");
    }
    action->enabledChanged.connect(boost::bind(&Settings::onEnabledChanged, this, action));
    m_actionButtonMap[action] = elmItem;
}

void Settings::onEnabledChanged(sharedAction action)
{
    BROWSER_LOGD("[%s]", __func__);
    refreshAction(action);
}

void Settings::refreshAction(sharedAction action)
{
    BROWSER_LOGD("[%s]", __func__);
    if(action->isEnabled())
        elm_object_item_disabled_set(m_actionButtonMap[action], EINA_FALSE);
    else
        elm_object_item_disabled_set(m_actionButtonMap[action], EINA_TRUE);
}

char* Settings::gridTextGet(void* data, Evas_Object* /*obj*/, const char* part)
{
    BROWSER_LOGD("[%s]", __func__);
    SettingsItem *settingsItem =reinterpret_cast<SettingsItem*>(data);
    if(!strcmp(part, "optionName")){
        if(!settingsItem->action->getText().empty()){
            return strdup(settingsItem->action->getText().c_str());
        }
    }
    return strdup("");
}

Evas_Object* Settings::gridOptionLabelGet(void* data, Evas_Object* obj, const char* part)
{
    SettingsItem *settingsItem =reinterpret_cast<SettingsItem*>(data);
    if(!strcmp(part, "optionValue")){
        if(settingsItem->action->isEnabled() && settingsItem->action->isCheckable()){
            Evas_Object *label = elm_label_add(obj);
            elm_object_text_set(label, (settingsItem->action->isChecked() ? "On" :"Off" ));
            elm_object_style_set(label, "settings_label");
            return label;
        }
    }
    return NULL;

}

void Settings::itemClicked(void* data, Evas_Object* /*o*/, void* event_info)
{
    BROWSER_LOGD("[%s]", __func__);

    Settings* settings = reinterpret_cast<Settings*>(data);
    Elm_Object_Item* elmItem = reinterpret_cast<Elm_Object_Item*>(event_info);
    BROWSER_LOGD("[%s:%d] \n\t %x", __PRETTY_FUNCTION__, __LINE__, elmItem);
    if(settings->m_settingsItemsMap.count(elmItem)){
        std::shared_ptr<SettingsItem> settingsItem(settings->m_settingsItemsMap[elmItem]);// elm_object_item_data_get(elmItem)
        if(settingsItem->action->isEnabled()){
            if(settingsItem->action->isCheckable()) {
                settings->showInternalPopup(elmItem);
            } else{
                settingsItem->action->trigger();
            }
        }
    }
}

void Settings::showInternalPopup(Elm_Object_Item* listItem)
{
    BROWSER_LOGD("[%s]", __func__);
    m_internalPopupVisible = true;
    m_trackedItem = listItem;

    if(!m_internalPopup){
        m_internalPopup = elm_ctxpopup_add(m_window.get());
        evas_object_smart_callback_add(m_internalPopup, "dismissed", Settings::dismissedCtxPopup, this);
    }
    Evas_Object * buttonBox = elm_box_add(m_internalPopup);
    Evas_Object * radioOn = elm_radio_add(m_internalPopup);
    Evas_Object * radioOff = elm_radio_add(m_internalPopup);


    elm_object_text_set(radioOn, "On");
    elm_object_text_set(radioOff, "Off");

    elm_object_style_set(radioOn, "settings_radio");
    elm_object_style_set(radioOff, "settings_radio");

    elm_box_pack_end(buttonBox, radioOn);
    elm_box_pack_end(buttonBox, radioOff);

    m_checkState = (m_settingsItemsMap[m_trackedItem]->action->isChecked() ? CheckStateOn : CheckStateOff );

    elm_radio_state_value_set(radioOn, CheckStateOn);//true
    elm_radio_value_pointer_set(radioOn, reinterpret_cast<int*>(&m_checkState));

    elm_radio_state_value_set(radioOff, CheckStateOff);//false
    elm_radio_value_pointer_set(radioOff, reinterpret_cast<int*>(&m_checkState));

    elm_radio_group_add(radioOff, radioOn);

    evas_object_smart_callback_add(radioOn, "changed", radioChanged, this);
    evas_object_smart_callback_add(radioOff, "changed", radioChanged, this);

    evas_object_size_hint_weight_set(buttonBox, EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(buttonBox, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(buttonBox);
    evas_object_show(radioOn);
    evas_object_show(radioOff);


    elm_object_content_set(m_internalPopup, buttonBox);
    elm_ctxpopup_direction_priority_set(m_internalPopup,
                                        ELM_CTXPOPUP_DIRECTION_DOWN,
                                        ELM_CTXPOPUP_DIRECTION_DOWN,
                                        ELM_CTXPOPUP_DIRECTION_DOWN,
                                        ELM_CTXPOPUP_DIRECTION_DOWN);
    Evas_Coord w,h,x,y;

#if PLATFORM(TIZEN)
    evas_object_geometry_get(elm_object_item_track(m_trackedItem), &x, &y, &w, &h);

    BROWSER_LOGD("[%s:%d] \n\tx:%d; y:%d; w:%d; h:%d  ", __PRETTY_FUNCTION__, __LINE__,
                                 x,    y,    w,    h);
    evas_object_move(m_internalPopup, x + 354, y);//354 is proper value
#endif

    elm_object_style_set(m_internalPopup, "settings_button");
    evas_object_show(m_internalPopup);
    if(!m_pointerModeEnabled)
        elm_object_focus_set(radioOn, EINA_TRUE);
    elm_object_item_signal_emit(m_trackedItem, "selected", "SettingsModel");
    elm_object_signal_emit(m_genlist, "show_popup", "SettingsModel");
}

void Settings::dismissedCtxPopup(void* data, Evas_Object* obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s]", __func__);
    Settings* settings = static_cast<Settings*>(data);
    evas_object_hide(obj);
    settings->m_internalPopup = 0;
    elm_object_signal_emit(settings->m_genlist, "hide_popup", "SettingsModel");
    elm_object_item_signal_emit(settings->m_trackedItem, "unselected", "SettingsModel");

#if PLATFORM(TIZEN)
    elm_object_item_untrack(settings->m_trackedItem);
#endif
    settings->m_internalPopupVisible = false;
}

void Settings::radioChanged(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Settings *settings = static_cast<Settings*>(data);
    std::shared_ptr<SettingsItem> settingsItem(settings->m_settingsItemsMap[settings->m_trackedItem]);
    settingsItem->action->toggle();
    elm_object_item_signal_emit(settings->m_trackedItem,
                          (settingsItem->action->isChecked() ? "switch,on" : "switch,off")
                          , "SettingsModel");
    elm_genlist_item_update(settings->m_trackedItem);
    elm_object_signal_emit(settings->m_internalPopup, "elm,state,hide", "elm");
}

void Settings::focusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,in", "option*");
}

void Settings::unFocusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,out", "option*");
}

void Settings::setPointerModeEnabled (bool enabled)
{
    m_pointerModeEnabled = enabled;
}

bool Settings::canBeDismissed()
{
    BROWSER_LOGD("[%s]", __func__);
    bool internalPopupVisible = m_internalPopup &&  m_internalPopupVisible;
    if(internalPopupVisible) {
        elm_ctxpopup_dismiss(m_internalPopup);
        return false;
    }
    return true;
}

}//namespace base_ui

}//namespace tizen_browser
