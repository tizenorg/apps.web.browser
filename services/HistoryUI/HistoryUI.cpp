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

#include "HistoryUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"


namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(HistoryUI, "org.tizen.browser.historyui")

typedef struct _HistoryItemData
{
	std::shared_ptr<tizen_browser::services::HistoryItem> item;
	std::shared_ptr<tizen_browser::base_ui::HistoryUI> historyUI;
} HistoryItemData;

struct ItemData{
        tizen_browser::base_ui::HistoryUI* historyUI;
        Elm_Object_Item * e_item;
};

static std::vector<HistoryItemData*> m_history_item_data;

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())
HistoryUI::HistoryUI()
    : m_gengrid(NULL)
    , m_parent(NULL)
    , m_item_class(NULL)
    , m_gengridSetup(false)
    , m_history_layout(NULL)
    , m_historyitem_layout(NULL)
    , m_genListActionBar(NULL)
    , m_genListToday(NULL)
    , m_itemClassToday(NULL)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    edjFilePath = EDJE_DIR;
    edjFilePath.append("HistoryUI/History.edj");
}

HistoryUI::~HistoryUI()
{
}

void HistoryUI::show(Evas_Object* parent)
{
    elm_theme_extension_add(NULL, edjFilePath.c_str());
    m_history_layout = elm_layout_add(parent);
    elm_layout_file_set(m_history_layout, edjFilePath.c_str(), "history-layout");
    evas_object_size_hint_weight_set(m_history_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_history_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_history_layout);

    showActionBar();

    m_gengrid = elm_gengrid_add(m_history_layout);
    elm_object_part_content_set(m_history_layout, "history_gengird", m_gengrid);

    /*evas_object_smart_callback_add(m_gengrid, "item,focused", focusItem, NULL);
    evas_object_smart_callback_add(m_gengrid, "item,unfocused", unFocusItem, NULL);
    evas_object_smart_callback_add(m_gengrid, "activated", _itemSelected, this);*/

      if (!m_item_class) {
            m_item_class = elm_gengrid_item_class_new();
            m_item_class->item_style = "history_item";
            m_item_class->func.text_get = _grid_text_get;
            m_item_class->func.content_get =  _history_grid_content_get;
            m_item_class->func.state_get = NULL;
            m_item_class->func.del = NULL;
        }

    M_ASSERT(m_parent);
        elm_gengrid_align_set(m_gengrid, 0, 0);
        elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
        elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
        elm_gengrid_horizontal_set(m_gengrid, EINA_TRUE);
        elm_gengrid_highlight_mode_set(m_gengrid, EINA_TRUE);
        elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
        elm_scroller_page_size_set(m_gengrid, 0, 580);

        evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_gengrid_item_size_set(m_gengrid, 580 * efl_scale, 580 * efl_scale);

    addItems();

}

void HistoryUI::showActionBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(NULL, edjFilePath.c_str());
    m_genListActionBar = elm_genlist_add(m_history_layout);
    elm_object_part_content_set(m_history_layout, "action_bar_history_genlist", m_genListActionBar);
    elm_genlist_homogeneous_set(m_genListActionBar, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListActionBar, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListActionBar, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListActionBar, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genListActionBar, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genListActionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    m_itemClassActionBar = elm_genlist_item_class_new();
    m_itemClassActionBar->item_style = "action_bar_history_items";
    m_itemClassActionBar->func.text_get = NULL; // &listTopItemTextGet;
    m_itemClassActionBar->func.content_get = &listActionBarContentGet;
    m_itemClassActionBar->func.state_get = 0;
    m_itemClassActionBar->func.del = 0;

    ItemData * id = new ItemData;
    id->historyUI = this;
    Elm_Object_Item* elmItem = elm_genlist_item_append(m_genListActionBar,            //genlist
                                                       m_itemClassActionBar,          //item Class
                                                      id,
                                                      NULL,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,//item type
                                                      NULL,
                                                      NULL                  //data passed to above function
                                                     );
    id->e_item = elmItem;
    ItemData * id2 = new ItemData;
    id2->historyUI = this;
    Elm_Object_Item* elmItem2 = elm_genlist_item_append(m_genListActionBar,            //genlist
                                                       m_itemClassActionBar,          //item Class
                                                      id2,
                                                      NULL,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,//item type
                                                      NULL,
                                                      NULL                  //data passed to above function
                                                     );
    id2->e_item = elmItem2;
}

Evas_Object* HistoryUI::listActionBarContentGet(void* data, Evas_Object* obj , const char* part)
{
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        if(!strcmp(part, "clearhistory_click"))
        {
                Evas_Object *clearHistoryButton = elm_button_add(obj);
                elm_object_style_set(clearHistoryButton, "history_button");
                evas_object_smart_callback_add(clearHistoryButton, "clicked", tizen_browser::base_ui::HistoryUI::_clearhistory_clicked, data);
                return clearHistoryButton;
        }
	else if(!strcmp(part, "close_click"))
    	{
        	Evas_Object *close_click = elm_button_add(obj);
	        elm_object_style_set(close_click, "history_button");
	        evas_object_smart_callback_add(close_click, "clicked", HistoryUI::close_clicked_cb, data);
	        return close_click;
	}
          
        return NULL;
}

void HistoryUI::close_clicked_cb(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData * id = static_cast<ItemData *>(data);
    id->historyUI->closeHistoryUIClicked(std::string());
    id->historyUI->clearItems();
}	

char* HistoryUI::listTodayTextGet(void* data, Evas_Object* obj , const char* part)
{
     BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    HistoryItemData * id = static_cast<HistoryItemData *>(data);
    if(!strcmp(part, "history_url_text"))
    {
        if(!id->item->getUrl().empty()){
	    std::string str = id->item->getTitle() + " - " + id->item->getUrl();
            return strdup(str.c_str());
        }
    }
    return NULL;
}

void HistoryUI::_clearhistory_clicked(void * data, Evas_Object * /* obj */, void * event_info)
{
        /*
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        ItemData* itemData = reinterpret_cast<ItemData *>(data);
        itemData->tabUI->clearItems();
        itemData->tabUI->newTabClicked(std::string());
        */
}

void HistoryUI::addItems()
{
         BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
         for (size_t i = 0; i < 1; i++) {
             HistoryItemData *itemData = new HistoryItemData();
             itemData->historyUI = std::shared_ptr<tizen_browser::base_ui::HistoryUI>(this);
             Elm_Object_Item* historyView = elm_gengrid_item_append(m_gengrid, m_item_class, itemData, NULL, this);
             elm_gengrid_item_selected_set(historyView, EINA_FALSE);
         }
         BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
}

void HistoryUI::addHistoryItem(std::shared_ptr<tizen_browser::services::HistoryItem> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    HistoryItemData *itemData = new HistoryItemData();
    itemData->item = hi;
    itemData->historyUI = std::shared_ptr<tizen_browser::base_ui::HistoryUI>(this);
    m_history_item_data.push_back(itemData);
    setEmptyGengrid(false);
}

void HistoryUI::addHistoryItems(std::vector<std::shared_ptr<tizen_browser::services::HistoryItem> > items)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_history_item_data.clear();
    for (auto it = items.begin(); it != items.end(); ++it)
	addHistoryItem(*it);
}

char* HistoryUI::_grid_text_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    HistoryItemData *itemData = reinterpret_cast<HistoryItemData*>(data);
    const char* item_name = NULL;
    if (!strcmp(part, "menu_label")) {
            item_name = "Today"; 
	    return strdup(item_name);
    }
    return NULL;
}

Evas_Object * HistoryUI::_history_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] part : %s", __PRETTY_FUNCTION__, __LINE__, part);
    HistoryItemData * id = static_cast<HistoryItemData *>(data);
    if(!strcmp(part, "history_genlist"))
    {
    	id->historyUI->m_genListToday = elm_genlist_add(id->historyUI->m_history_layout);

    	elm_genlist_homogeneous_set(id->historyUI->m_genListToday, EINA_FALSE);
	elm_genlist_multi_select_set(id->historyUI->m_genListToday, EINA_FALSE);
	elm_genlist_select_mode_set(id->historyUI->m_genListToday, ELM_OBJECT_SELECT_MODE_ALWAYS);
        elm_genlist_mode_set(id->historyUI->m_genListToday, ELM_LIST_LIMIT);
        elm_genlist_decorate_mode_set(id->historyUI->m_genListToday, EINA_TRUE);
        evas_object_size_hint_weight_set(id->historyUI->m_genListToday, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	id->historyUI->m_itemClassToday = elm_genlist_item_class_new();
	id->historyUI->m_itemClassToday->item_style = "history_url_items";
	id->historyUI->m_itemClassToday->func.text_get =  &listTodayTextGet;
	id->historyUI->m_itemClassToday->func.content_get = NULL;
	id->historyUI->m_itemClassToday->func.state_get = 0;
	id->historyUI->m_itemClassToday->func.del = 0;

	for(auto it = m_history_item_data.begin(); it != m_history_item_data.end(); it++) {
		Elm_Object_Item* historyView = elm_genlist_item_append(id->historyUI->m_genListToday, id->historyUI->m_itemClassToday, *it, NULL, ELM_GENLIST_ITEM_NONE, NULL, id->historyUI.get());
		id->historyUI->m_map_history_views.insert(std::pair<std::string,Elm_Object_Item*>((*it)->item->getUrl(), historyView));
	}

	return id->historyUI->m_genListToday;
    }
    return NULL;
}

void HistoryUI::removeHistoryItem(const std::string& uri)
{
    BROWSER_LOGD("[%s] uri=%s", __func__, uri.c_str());
    if(m_map_history_views.find(uri) == m_map_history_views.end()) {
        return;
    }

    Elm_Object_Item* historyView = m_map_history_views.at(uri);
    elm_object_item_del(historyView);
    m_map_history_views.erase(uri);

    setEmptyGengrid(0 == m_map_history_views.size());
}

void  HistoryUI::_item_deleted(void * /* data */, Evas_Object * /* obj */)
{

}

void HistoryUI::_itemSelected(void * data, Evas_Object * /* obj */, void * event_info)
{
        Elm_Object_Item * selected = reinterpret_cast<Elm_Object_Item *>(event_info);
        HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(elm_object_item_data_get(selected));
        HistoryUI * self = reinterpret_cast<HistoryUI *>(data);

        self->historyItemClicked(itemData->item);
}

void HistoryUI::_deleteHistory(void *data, Evas_Object * /* obj */, void * /* event_info */)
{
        HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
        //itemData->historysUI->historyDeleteClicked(itemData->item);
}

void HistoryUI::_thumbSelected(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
        HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
       // itemData->historysUI->historyClicked(itemData->item);
}

Evas_Object* HistoryUI::createNoHistorysLabel()
{
    Evas_Object *label = elm_label_add(m_parent);
    elm_object_text_set(label, "No favorite websites.");
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    return label;
}

void HistoryUI::setEmptyGengrid(bool setEmpty)
{
    if(setEmpty) {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", createNoHistorysLabel());
    } else {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", NULL);
    }
}

void HistoryUI::hide()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(elm_layout_content_get(m_history_layout, "m_genListToday"));
    evas_object_hide(elm_layout_content_get(m_history_layout, "m_gengrid"));
    evas_object_hide(elm_layout_content_get(m_history_layout, "m_genListActionBar"));
    evas_object_hide(m_history_layout);
}

void HistoryUI::clearItems()
{
    hide();
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_genlist_clear(m_genListToday);
    elm_gengrid_clear(m_gengrid);
    elm_genlist_clear(m_genListActionBar);
    m_map_history_views.clear();
    m_history_item_data.clear();
    setEmptyGengrid(true);
}

void HistoryUI::focusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,in", "over2");

    // selected manually
    elm_gengrid_item_selected_set(item, EINA_TRUE);
}

void HistoryUI::unFocusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,out", "over2");

    // unselected manually
    elm_gengrid_item_selected_set(item, EINA_FALSE);
}

}
}
