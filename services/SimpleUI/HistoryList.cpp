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

#include "HistoryList.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"
#include "HistoryItem.h"
#include "EflTools.h"
#include <Evas.h>


namespace tizen_browser
{
namespace base_ui
{


HistoryList::HistoryList(std::shared_ptr<Evas_Object> mainWindow, Evas_Object* parentButton)
    : MenuButton(mainWindow, parentButton)
    ,m_genList(NULL)
    ,m_itemClass(NULL)
    ,m_lastFocusedItem(NULL)
    ,m_deleteSelected(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string edjFilePath = EDJE_DIR;
    edjFilePath.append("SimpleUI/HistoryItem.edj");
    elm_theme_extension_add(0, edjFilePath.c_str());

    itemWidth = atoi(edje_file_data_get(edjFilePath.c_str(),"item_width"));
    itemHeight = atoi(edje_file_data_get(edjFilePath.c_str(),"item_height"));
    parentItemHeight = atoi(edje_file_data_get(edjFilePath.c_str(),"parent_item_height"));
    itemsCounter = 0;
    parentItemsCounter = 0;

    m_genList = elm_genlist_add(m_window.get());
    elm_object_style_set(m_genList, "history");
    elm_genlist_homogeneous_set(m_genList, EINA_FALSE);
    elm_genlist_multi_select_set(m_genList, EINA_FALSE);
    elm_genlist_select_mode_set(m_genList, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genList, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genList, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    evas_object_smart_callback_add(m_genList, "item,focused", focusItem, this);
    evas_object_smart_callback_add(m_genList, "item,unfocused", unFocusItem, NULL);

    m_itemClass = elm_genlist_item_class_new();
    m_itemClass->item_style = "history_item";
    m_itemClass->func.text_get = &listItemTextGet;
    m_itemClass->func.content_get = &listItemContentGet;
    m_itemClass->func.state_get = 0;
    m_itemClass->func.del = 0;

    m_itemClassNoFavicon = elm_genlist_item_class_new();
    m_itemClassNoFavicon->item_style = "history_item_no_favicon";
    m_itemClassNoFavicon->func.text_get = &listItemTextGet;
    m_itemClassNoFavicon->func.content_get = &listItemContentGet;
    m_itemClassNoFavicon->func.state_get = 0;
    m_itemClassNoFavicon->func.del = 0;

    m_parentItemClass= elm_genlist_item_class_new();
    m_parentItemClass->item_style = "history_parent_item";
    m_parentItemClass->func.text_get = &listParentItemTextGet;
    m_parentItemClass->func.content_get = 0;
    m_parentItemClass->func.state_get = 0;
    m_parentItemClass->func.del = 0;
}

HistoryList::~HistoryList()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void HistoryList::addItem(const std::shared_ptr<tizen_browser::services::HistoryItem> item)
{
    BROWSER_LOGD("[%s:%d]:%s ", __PRETTY_FUNCTION__, __LINE__, item->getTitle().c_str());
    //first check if ther's a date item.
    GroupIterator parenIter = m_groupParent.find(item->getLastVisit().date());
    Elm_Object_Item* groupParent;

    ItemData * id = new ItemData;
    id->h_list = this;
    id->h_item = item.get();
    if(parenIter == m_groupParent.end()){
        groupParent = elm_genlist_item_append(m_genList,           //genlist
                                        m_parentItemClass,    //item Class
                                        //id.get(),                 //item data
                                        id,
                                        0,                    //parent item
                                        ELM_GENLIST_ITEM_GROUP,//item type
                                        paretn_item_clicked_cb,
                                        NULL                 //data passed to above function
                                        );
        elm_object_item_disabled_set(groupParent, EINA_TRUE);
        m_groupParent[item->getLastVisit().date()] = groupParent;
        ++parentItemsCounter;
    } else {
        groupParent = parenIter->second;
    }

    Elm_Object_Item* elmItem = elm_genlist_item_append(m_genList,            //genlist
                                                      item->getFavIcon()->dataSize ? m_itemClass : m_itemClassNoFavicon,          //item Class
                                                      //id.get(),        //item data
                                                      id,
                                                      groupParent,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,//item type
                                                      NULL,
                                                      NULL                  //data passed to above function
                                                     );
    id->e_item = elmItem;

//    m_items.push_back(item);
    ++itemsCounter;
}

void HistoryList::clearList()
{
	elm_genlist_clear(m_genList);
	itemsCounter = 0;
	parentItemsCounter = 0;
	m_groupParent.clear();
}

void HistoryList::addItems(tzSrv::HistoryItemVector items)
{
    clearList();
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    tzSrv::HistoryItemVectorConstIter end = items.end();
    for(tzSrv::HistoryItemVectorConstIter item = items.begin();  item!=end; item++){
        addItem(*item);
    }
}

Evas_Object* HistoryList::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return m_genList;
}

MenuButton::ListSize HistoryList::calculateSize()
{
    ListSize result;

    result.width = itemWidth;

    result.height = itemsCounter*itemHeight + parentItemsCounter*parentItemHeight + 16; //"16" is size of frame
    if (result.height > 868)
        result.height = 868;

    return result;
}

Evas_Object* HistoryList::getFirstFocus()
{
    return m_genList;
}

Evas_Object* HistoryList::listItemContentGet(void* data, Evas_Object* obj, const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData * id = static_cast<ItemData *>(data);
    if(!strcmp(part, "favicon") && id->h_item->getFavIcon())
    {
        return tizen_browser::tools::EflTools::getEvasImage(id->h_item->getFavIcon(), obj);
    }
    else if(!strcmp(part, "content_click"))
    {
        Evas_Object *content_click = elm_button_add(obj);
        elm_object_style_set(content_click, "invisible_button");
        evas_object_smart_callback_add(content_click, "clicked", HistoryList::item_clicked_cb, id);
        return content_click;
    }
    else if(!strcmp(part, "del_click"))
    {
        Evas_Object *del_click = elm_button_add(obj);
        elm_object_style_set(del_click, "invisible_button");
        evas_object_smart_callback_add(del_click, "clicked", HistoryList::item_delete_clicked_cb, id);
        return del_click;
    }
    return NULL;
}

char* HistoryList::listItemTextGet(void* data, Evas_Object* /* obj */, const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData * id = static_cast<ItemData *>(data);
    if(!strcmp(part, "page_title"))
    {
        if(!id->h_item->getTitle().empty()){
            return strdup(id->h_item->getTitle().c_str());
        }
    }
    else if(!strcmp(part, "page_url"))
        {
        if(!id->h_item->getUrl().empty()){
            return strdup(id->h_item->getUrl().c_str());
        }
    }
    else if(!strcmp(part, "page_time"))
    {
        std::string retstr = boost::posix_time::to_simple_string(id->h_item->getLastVisit().time_of_day());
        return strdup(retstr.c_str());
    }
    return strdup("");
}

void HistoryList::item_clicked_cb(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData * id = static_cast<ItemData *>(data);
    id->h_list->clickedHistoryItem(id->h_item->getUrl());
}

void HistoryList::item_delete_clicked_cb(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData * id = static_cast<ItemData *>(data);
    id->h_list->deleteHistoryItem(id->h_item->getUrl());
    elm_object_item_del(id->e_item);
    elm_genlist_item_update(id->e_item);
    BROWSER_LOGD("[%s:%d] Genlist count: %d", __PRETTY_FUNCTION__, __LINE__, elm_genlist_items_count(id->h_list->m_genList));
}

char* HistoryList::listParentItemTextGet(void* data, Evas_Object* /* obj */, const char* /* part */ )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ///\todo fix year out of range
    ItemData * id = static_cast<ItemData *>(data);
    std::string retstr = boost::gregorian::to_simple_string(id->h_item->getLastVisit().date());
    return strdup(retstr.c_str());

}

void HistoryList::paretn_item_clicked_cb(void* /* data */, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void HistoryList::focusItem(void* data, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,in", "content_click");

    HistoryList *self = static_cast<HistoryList*>(data);
    self->m_lastFocusedItem = item;
}

void HistoryList::unFocusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    ItemData * id = reinterpret_cast<ItemData *>(elm_object_item_data_get(item));
    M_ASSERT(id);
    elm_object_item_signal_emit(item, "mouse,out", "content_click");
    elm_object_item_signal_emit(item, "mouse,out", "del_click");
    id->h_list->m_deleteSelected = false;
}

void HistoryList::rightPressed()
{
    if(m_lastFocusedItem && elm_object_focus_get(elm_object_item_widget_get(m_lastFocusedItem))) {
        elm_object_item_signal_emit(m_lastFocusedItem, "mouse,in", "del_click");
        m_deleteSelected = true;
    }
}

void HistoryList::leftPressed()
{
    if(m_lastFocusedItem && elm_object_focus_get(elm_object_item_widget_get(m_lastFocusedItem))) {
        elm_object_item_signal_emit(m_lastFocusedItem, "mouse,out", "del_click");
        elm_object_item_signal_emit(m_lastFocusedItem, "mouse,in", "content_click");
        m_deleteSelected = false;
    }
}

void HistoryList::enterPressed()
{
    if(m_lastFocusedItem && elm_object_focus_get(elm_object_item_widget_get(m_lastFocusedItem))) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

        ItemData * id = reinterpret_cast<ItemData *>(elm_object_item_data_get(m_lastFocusedItem));
        M_ASSERT(id);
        if(m_deleteSelected) {
            deleteHistoryItem(id->h_item->getUrl());
            elm_object_item_del(m_lastFocusedItem);
            elm_genlist_item_update(m_lastFocusedItem);
        }
        else
            clickedHistoryItem(id->h_item->getUrl());
    }
}

} /* end of namespace base_ui */
} /* end of namespace tizen_browser */
