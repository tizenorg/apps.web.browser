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

#include "BookmarksManager.h"
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

BookmarksManagerItem::BookmarksManagerItem()
    : action(std::make_shared<Action>())
{

}
BookmarksManagerItem::BookmarksManagerItem( std::shared_ptr<Action> action)
    : action(action)
{

}


BookmarksManagerItem::BookmarksManagerItem(const BookmarksManagerItem& source)
    : action(source.action)
{

}
BookmarksManagerItem::BookmarksManagerItem(const BookmarksManagerItem* source)
    : action(source->action)
{

}


BookmarksManagerItem::~BookmarksManagerItem()
{
    //elm_genlist_item_class_free(m_itemClass);??
}

BookmarksManager::BookmarksManager(std::shared_ptr< Evas_Object > mainWindow, Evas_Object* parentButton)
    : MenuButton(mainWindow, parentButton)
    , m_popup (nullptr)
    , m_genlist(nullptr)
    , m_internalPopup(nullptr)
    , m_itemClass(nullptr)
    , m_internalPopupVisible(false)
    , m_isPopupShown (false)
{
    std::string edjFilePath = EDJE_DIR;
    edjFilePath.append("SimpleUI/BookmarksManager.edj");
    elm_theme_extension_add(0, edjFilePath.c_str());

    m_genlist = elm_genlist_add(m_window.get());

    evas_object_smart_callback_add(m_genlist, "item,focused", focusItem, nullptr);
    evas_object_smart_callback_add(m_genlist, "item,unfocused", unFocusItem, nullptr);

    elm_object_style_set(m_genlist, "bookmarks_manager");
    elm_genlist_homogeneous_set(m_genlist, EINA_TRUE);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);

    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);

    m_itemClass = elm_genlist_item_class_new();
    m_itemClass->item_style = "bookmarks_manager_item";

    m_itemClass->func.text_get = &gridTextGet;
    m_itemClass->func.content_get = &gridOptionLabelGet;
    m_itemClass->func.state_get = 0;
    m_itemClass->func.del = 0;
}

BookmarksManager::~BookmarksManager()
{

}

Evas_Object* BookmarksManager::getContent()
{
    return m_genlist;
}

tizen_browser::base_ui::MenuButton::ListSize BookmarksManager::calculateSize()
{
    ListSize result;

    result.width = 490;
    result.height = 426;

    return result;
}

Evas_Object* BookmarksManager::getFirstFocus()
{
    return m_genlist;
}

void BookmarksManager::addAction(sharedAction action)
{
    std::shared_ptr<BookmarksManagerItem> bookmarks_managerItem = std::make_shared<BookmarksManagerItem>(action);
    BROWSER_LOGD("[%s] %x: %s", __func__ , bookmarks_managerItem.get(), bookmarks_managerItem->action->getText().c_str() );
    Elm_Object_Item* elmItem= elm_genlist_item_append(m_genlist,            //genlist
                                                      m_itemClass,          //item Class
                                                      bookmarks_managerItem.get(),        //item data
                                                      0,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,//item type
                                                      itemClicked,
                                                      this
                                                     );
    m_bookmarks_managerItemsMap[elmItem] = bookmarks_managerItem;
    BROWSER_LOGD("[%s:%d] \n\t %x:%s ", __PRETTY_FUNCTION__, __LINE__, elmItem, bookmarks_managerItem->action->getText().c_str());
    if(!bookmarks_managerItem->action->isEnabled()){
        elm_object_item_disabled_set(elmItem, EINA_TRUE);
    }
    if(bookmarks_managerItem->action->isCheckable()){
        elm_object_item_signal_emit(elmItem,
                          (bookmarks_managerItem->action->isChecked() ? "switch,on" : "switch,off")
                          , "BookmarksManagerModel");
    } else {
        elm_object_item_signal_emit(elmItem, "switch,hide", "BookmarksManagerModel");
    }
    action->enabledChanged.connect(boost::bind(&BookmarksManager::onEnabledChanged, this, action));
    m_actionButtonMap[action] = elmItem;
}

void BookmarksManager::onEnabledChanged(sharedAction action)
{
    BROWSER_LOGD("[%s]", __func__);
    refreshAction(action);
}

void BookmarksManager::refreshAction(sharedAction action)
{
    BROWSER_LOGD("[%s]", __func__);
    if(action->isEnabled())
        elm_object_item_disabled_set(m_actionButtonMap[action], EINA_FALSE);
    else
        elm_object_item_disabled_set(m_actionButtonMap[action], EINA_TRUE);
}

char* BookmarksManager::gridTextGet(void* data, Evas_Object* /*obj*/, const char* part)
{
    BROWSER_LOGD("[%s]", __func__);
    BookmarksManagerItem *bookmarks_managerItem =reinterpret_cast<BookmarksManagerItem*>(data);
    if(!strcmp(part, "optionName")){
        if(!bookmarks_managerItem->action->getText().empty()){
            return strdup(bookmarks_managerItem->action->getText().c_str());
        }
    }
    return strdup("");
}

Evas_Object* BookmarksManager::gridOptionLabelGet(void* data, Evas_Object* obj, const char* part)
{
    BookmarksManagerItem *bookmarks_managerItem =reinterpret_cast<BookmarksManagerItem*>(data);
    if(!strcmp(part, "optionValue")){
        if(bookmarks_managerItem->action->isEnabled() && bookmarks_managerItem->action->isCheckable()){
            Evas_Object *label = elm_label_add(obj);
            elm_object_text_set(label, (bookmarks_managerItem->action->isChecked() ? "On" :"Off" ));
            elm_object_style_set(label, "bookmarks_manager_label");
            return label;
        }
    }
    return nullptr;

}

void BookmarksManager::itemClicked(void* data, Evas_Object* /*o*/, void* event_info)
{
    BROWSER_LOGD("[%s]", __func__);

    BookmarksManager* bookmarks_manager = reinterpret_cast<BookmarksManager*>(data);
    Elm_Object_Item* elmItem = reinterpret_cast<Elm_Object_Item*>(event_info);
    BROWSER_LOGD("[%s:%d] \n\t %x", __PRETTY_FUNCTION__, __LINE__, elmItem);
    if(bookmarks_manager->m_bookmarks_managerItemsMap.count(elmItem)){
        std::shared_ptr<BookmarksManagerItem> bookmarks_managerItem(bookmarks_manager->m_bookmarks_managerItemsMap[elmItem]);// elm_object_item_data_get(elmItem)
        if(bookmarks_managerItem->action->isEnabled()){
            if(bookmarks_managerItem->action->isCheckable()) {
                bookmarks_manager->showInternalPopup(elmItem);
            } else{
                bookmarks_managerItem->action->trigger();
            }
        }
    }
}

void BookmarksManager::showPopup()
{
    BROWSER_LOGD("[%s:%d] this: %x ", __PRETTY_FUNCTION__, __LINE__, this);

    if (isPopupShown()){
        m_isPopupShown = false;
        unbindFocus();
        evas_object_hide(m_popup);
        return;
    }

    if(!m_popup) {
        m_popup = elm_popup_add(m_window.get());
        BROWSER_LOGD("[%s:%d] - new popup: %x ", __PRETTY_FUNCTION__, __LINE__, m_popup);
        evas_object_smart_callback_add(m_popup, "block,clicked", block_clicked, this);
      //  elm_object_content_set(m_popup, getContent());
        elm_object_part_text_set(m_popup, "title,text", "Add to Bookmark");

        Evas_Coord w,h,x,y;
        evas_object_geometry_get(m_window.get(), &x, &y, &w, &h);
        evas_object_geometry_set(m_popup,0,0,w,h);

      //elm_object_style_set(m_popup, "message_popup");
    }
    //evas_object_resize(m_popup, 480, 800);
    realShow(m_popup);
}

void BookmarksManager::realShow(Evas_Object* popup)
{
    elm_object_focus_next_object_set(m_parentButton, getFirstFocus(), ELM_FOCUS_DOWN);
    elm_object_focus_next_object_set(getFirstFocus(), m_parentButton, ELM_FOCUS_UP);

   // ListSize listSize = calculateSize();
  //  evas_object_size_hint_min_set(m_popup, listSize.width, listSize.height);
  //  evas_object_size_hint_max_set(m_popup, listSize.width, listSize.height);
   elm_popup_item_append(popup,"Folde 1",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 2",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 3",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 4",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 5",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 6",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 7",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 8",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 8",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 10",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 11",nullptr,nullptr,this);
   elm_popup_item_append(popup,"Folde 12",nullptr,nullptr,this);
   evas_object_show(popup);
    m_isPopupShown=true;
}

void BookmarksManager::unbindFocus()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_focus_next_object_set(m_parentButton, nullptr, ELM_FOCUS_DOWN);
}

bool BookmarksManager::isPopupShown()
{
    return m_isPopupShown;
}

void BookmarksManager::block_clicked(void*, Evas_Object *obj, void*)
{
    evas_object_hide(obj);
}

void BookmarksManager::hidePopup()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(m_popup);
    m_isPopupShown = false;
}

void BookmarksManager::showInternalPopup(Elm_Object_Item* listItem)
{
    BROWSER_LOGD("[%s]", __func__);
    m_internalPopupVisible = true;
    m_trackedItem = listItem;

    if(!m_internalPopup){
        m_internalPopup = elm_ctxpopup_add(m_window.get());
        evas_object_smart_callback_add(m_internalPopup, "dismissed", BookmarksManager::dismissedCtxPopup, this);
    }
    Evas_Object * buttonBox = elm_box_add(m_internalPopup);
    Evas_Object * radioOn = elm_radio_add(m_internalPopup);
    Evas_Object * radioOff = elm_radio_add(m_internalPopup);


    elm_object_text_set(radioOn, "On");
    elm_object_text_set(radioOff, "Off");

    elm_object_style_set(radioOn, "bookmarks_manager_radio");
    elm_object_style_set(radioOff, "bookmarks_manager_radio");

    elm_box_pack_end(buttonBox, radioOn);
    elm_box_pack_end(buttonBox, radioOff);

    m_checkState = (m_bookmarks_managerItemsMap[m_trackedItem]->action->isChecked() ? CheckStateOn : CheckStateOff );

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

    evas_object_geometry_get(elm_object_item_track(m_trackedItem), &x, &y, &w, &h);
    evas_object_move(m_internalPopup, x + 354, y);//354 is proper value

    elm_object_style_set(m_internalPopup, "bookmarks_manager_button");
    evas_object_show(m_internalPopup);
    elm_object_focus_set(radioOn, EINA_TRUE);
    elm_object_item_signal_emit(m_trackedItem, "selected", "BookmarksManagerModel");
    elm_object_signal_emit(m_genlist, "show_popup", "BookmarksManagerModel");
}

void BookmarksManager::dismissedCtxPopup(void* data, Evas_Object* obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s]", __func__);
    BookmarksManager* bookmarks_manager = static_cast<BookmarksManager*>(data);
    evas_object_hide(obj);
    bookmarks_manager->m_internalPopup = 0;
    elm_object_signal_emit(bookmarks_manager->m_genlist, "hide_popup", "BookmarksManagerModel");
    elm_object_item_signal_emit(bookmarks_manager->m_trackedItem, "unselected", "BookmarksManagerModel");

    elm_object_item_untrack(bookmarks_manager->m_trackedItem);
    bookmarks_manager->m_internalPopupVisible = false;
}

void BookmarksManager::radioChanged(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarksManager *bookmarks_manager = static_cast<BookmarksManager*>(data);
    std::shared_ptr<BookmarksManagerItem> bookmarks_managerItem(bookmarks_manager->m_bookmarks_managerItemsMap[bookmarks_manager->m_trackedItem]);
    bookmarks_managerItem->action->toggle();
    elm_object_item_signal_emit(bookmarks_manager->m_trackedItem,
                          (bookmarks_managerItem->action->isChecked() ? "switch,on" : "switch,off")
                          , "BookmarksManagerModel");
    elm_genlist_item_update(bookmarks_manager->m_trackedItem);
    elm_object_signal_emit(bookmarks_manager->m_internalPopup, "elm,state,hide", "elm");
}

void BookmarksManager::focusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,in", "option*");
}

void BookmarksManager::unFocusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,out", "option*");
}

bool BookmarksManager::canBeDismissed()
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
