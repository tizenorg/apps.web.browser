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

#include "TabList.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"
#include "EflTools.h"
#include <stdio.h>
#include <stdlib.h>

namespace tizen_browser
{
namespace base_ui
{

const int TabList::ScrollBorderValue = 8;

TabList::TabList(std::shared_ptr< Evas_Object > parent, Evas_Object* parentButton)
    : MenuButton(parent, parentButton)
    , m_list(NULL)
    , m_box(NULL)
    , m_deleteSelected(false)
{
    m_box = elm_box_add(m_window.get());
    evas_object_size_hint_align_set(m_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(m_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(m_box);

    m_newTabBtn = elm_button_add(m_box);
    elm_object_style_set(m_newTabBtn, "new_tab_elem");
    evas_object_smart_callback_add(m_newTabBtn, "clicked", TabList::_newTab_clicked_cb, (void*)this);
    //evas_object_smart_callback_add(m_newTabBtn, "focused", TabList::newButtonFocused, this);
    //evas_object_smart_callback_add(m_newTabBtn, "unfocused", TabList::newButtonUnFocused, this);
    evas_object_show(m_newTabBtn);

    m_list = elm_list_add(m_box);
    evas_object_size_hint_align_set(m_list, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(m_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_list_mode_set(m_list, ELM_LIST_SCROLL);
    elm_list_multi_select_set(m_list, EINA_FALSE);
    elm_list_select_mode_set(m_list, ELM_OBJECT_SELECT_MODE_ALWAYS);

    evas_object_smart_callback_add(m_list, "item,focused", TabList::focusItem, this);
    evas_object_smart_callback_add(m_list, "item,unfocused", TabList::unFocusItem, this);

    elm_object_focus_next_object_set(m_newTabBtn, m_list, ELM_FOCUS_DOWN);

    elm_object_style_set(m_list, "tab_list");

    elm_box_pack_end(m_box, m_newTabBtn);
    elm_box_pack_end(m_box, m_list);

    elm_list_go(m_list);

    //elm_object_focus_next_object_set(m_newTabBtn, m_list, ELM_FOCUS_DOWN);

    evas_object_show(m_list);
}

void TabList::addItem(std::shared_ptr<tizen_browser::basic_webengine::TabContent> tab)
{
    tab_data *data = new tab_data();
    data->id = tab->getId();
    data->tab_list = this;

    Evas_Object *end = elm_button_add(m_list);
    Evas_Object *icon = elm_button_add(m_list);


    elm_object_focus_allow_set(end, EINA_TRUE);
    elm_object_focus_allow_set(icon, EINA_TRUE);

    elm_object_style_set(end, "invisible_button");
    elm_object_style_set(icon, "invisible_button");

    Evas_Object * thumbnail = tizen_browser::tools::EflTools::getEvasImage(tab->getThumbnail(), icon);
    if (thumbnail)
        elm_object_part_content_set(icon, "e.swallow.icon", thumbnail);
    else
        BROWSER_LOGD("[%s] Empty thumbnail added to tabs vector", __func__);

    evas_object_smart_callback_add(icon, "clicked", TabList::_item_clicked_cb, (void *)data);
    evas_object_smart_callback_add(end, "clicked", TabList::_deleteTab_clicked_cb, (void *)data);
    Elm_Object_Item *item = elm_list_item_append(m_list, tab->getTitle().c_str(), icon, end, NULL, data);

    elm_object_focus_next_object_set(elm_list_item_object_get(item),
                                     elm_object_item_part_content_get(item, "elm.swallow.end"), ELM_FOCUS_RIGHT);

    m_items[tab->getId()] = item;

    elm_list_go(m_list);
}

void TabList::addItems(std::vector<std::shared_ptr<tizen_browser::basic_webengine::TabContent>> items)
{
    clearItems();
    for (std::vector<std::shared_ptr<tizen_browser::basic_webengine::TabContent> >::iterator it = items.begin(); it != items.end(); ++it) {
        addItem(*it);
    }
}

void TabList::clearItems()
{
    m_items.clear();

    /*
     * Calling elm_list_clear in this case makes that list cannot receive focus anymore.
     * So deleting item by item is necessary.
     */
    Elm_Object_Item *item;
    while((item = elm_list_first_item_get(m_list)))
        elm_object_item_del(item);
}

void TabList::replaceItems(std::vector< std::shared_ptr< basic_webengine::TabContent > > items)
{
    clearItems();
    addItems(items);
}

int TabList::getItemcCount()
{
    return m_items.size();
}

Evas_Object* TabList::getContent()
{
    return m_box;
}

MenuButton::ListSize TabList::calculateSize()
{
    elm_object_focus_allow_set(m_list, m_items.size() ? EINA_TRUE : EINA_FALSE);

    ListSize result;

    result.width = 390;

    result.height = 89 * (getItemcCount()+1) + 8;

    int x, y, w, h;
    Evas_Object *focusDown = elm_object_focus_next_object_get(m_newTabBtn, ELM_FOCUS_DOWN);
    evas_object_geometry_get(focusDown, &x, &y, &w, &h);
    BROWSER_LOGD("%d %d %d %d\n", x, y, w, h);

    const int listMaxHeight = atoi(edje_file_data_get((std::string(EDJE_DIR) + "SimpleUI/TabItem.edj").c_str(),"list_max_height"));
    if (result.height > listMaxHeight)
        result.height = listMaxHeight;

    return result;
}

void TabList::_item_clicked_cb(void* data, Evas_Object */*obj*/, void */*event_info*/)
{
    tab_data *tab = static_cast<tab_data*>(data);
    tab->tab_list->tabClicked(tab->id);
    tab->tab_list->hidePopup();
}

void TabList::_newTab_clicked_cb(void* data, Evas_Object */*obj*/, void */*event_info*/)
{
    TabList *self = reinterpret_cast<TabList*>(data);
    self->newTabClicked();
    self->hidePopup();
}

void TabList::_deleteTab_clicked_cb(void* data, Evas_Object */*obj*/, void */*event_info*/)
{
    tab_data *tab = static_cast<tab_data*>(data);
    tab->tab_list->tabDelete(tab->id);
    tab->tab_list->hidePopup();
}

void TabList::setCurrentTabId(tizen_browser::basic_webengine::TabId currentTabId)
{
    m_currentTabId = currentTabId;
}

void TabList::disableNewTabBtn(bool disabled)
{
    elm_object_disabled_set(m_newTabBtn, disabled);
}

void TabList::onPopupShow()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if(m_items.find(m_currentTabId) != m_items.end())
            elm_object_item_signal_emit(m_items[m_currentTabId], "highlight,selected", "");
}

Evas_Object* TabList::getFirstFocus()
{
    BROWSER_LOGD("[%s:%d] m_newTabBtn: %x ", __PRETTY_FUNCTION__, __LINE__, m_newTabBtn);
    return m_newTabBtn;
}

void TabList::focusItem(void* data, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TabList *self = reinterpret_cast<TabList*>(data);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    BROWSER_LOGD("[%s:%d] Item: %s ", __PRETTY_FUNCTION__, __LINE__, elm_object_item_part_text_get(item,"elm.text"));
    //elm_object_signal_emit(elm_object_item_ item, "mouse,in", "elm.swallow.icon");
    elm_object_item_signal_emit( item, "mouse,in", "elm.swallow.icon");
    //elm.swallow.end
    //Evas_Object* delIcon = elm_object_part_content_get(item, "elm.swallow.end");
    self->m_lastFocusedItem = item;
}

void TabList::unFocusItem(void* data, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TabList *self = reinterpret_cast<TabList*>(data);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    BROWSER_LOGD("[%s:%d] Item: %s ", __PRETTY_FUNCTION__, __LINE__, elm_object_item_part_text_get(item,"elm.text"));
    //elm_object_signal_emit(elm_object_item_ item, "mouse,in", "elm.swallow.icon");
    elm_object_item_signal_emit(item, "mouse,out", "elm.swallow.end");
    elm_object_item_signal_emit(item, "mouse,out", "elm.swallow.icon");

    self->m_deleteSelected = false;
}

void TabList::newButtonFocused(void* /*data*/, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void TabList::newButtonUnFocused(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TabList *self = reinterpret_cast<TabList*>(data);
    //EFL BUG workaround: it looks like button do not get "unfocus" signal each time.
    elm_object_signal_emit(self->m_newTabBtn, "elm,action,unfocus","elm");
}

void TabList::rightPressed()
{
    if(m_lastFocusedItem && elm_object_focus_get(elm_object_item_widget_get(m_lastFocusedItem))) {
        elm_object_item_signal_emit(m_lastFocusedItem, "mouse,out", "elm.swallow.icon");
        elm_object_item_signal_emit(m_lastFocusedItem, "mouse,in", "elm.swallow.end");
        m_deleteSelected = true;
    }
}

void TabList::leftPressed()
{
    if(m_lastFocusedItem && elm_object_focus_get(elm_object_item_widget_get(m_lastFocusedItem))) {
        elm_object_item_signal_emit(m_lastFocusedItem, "mouse,out", "elm.swallow.end");
        elm_object_item_signal_emit(m_lastFocusedItem, "mouse,in", "elm.swallow.icon");
        m_deleteSelected = false;
    }
}

void TabList::enterPressed()
{
    if(m_lastFocusedItem && elm_object_focus_get(elm_object_item_widget_get(m_lastFocusedItem))) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

        tab_data *td = static_cast<tab_data*>(elm_object_item_data_get(m_lastFocusedItem));
        M_ASSERT(td);

        if(m_deleteSelected) {
            tabDelete(td->id);
        }
        else {
           tabClicked(td->id);
        }
        hidePopup();
    }
}

}

}
