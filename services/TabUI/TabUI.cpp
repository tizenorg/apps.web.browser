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
#include <string>
#include <string.h>
#include <AbstractMainWindow.h>

#include "TabUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"


namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(TabUI, "org.tizen.browser.tabui")

typedef struct _TabItemData
{
    std::shared_ptr<tizen_browser::basic_webengine::TabContent> item;
    std::shared_ptr<tizen_browser::base_ui::TabUI> tabUI;
} TabItemData;

struct ItemData
{
    tizen_browser::base_ui::TabUI* tabUI;
    Elm_Object_Item * e_item;
};

TabUI::TabUI()
    : m_tab_layout(nullptr)
    , m_genListActionBar(nullptr)
    , m_itemClassActionBar(nullptr)
    , m_genListTop(nullptr)
    , m_itemClassTop(nullptr)
    , m_gengrid(nullptr)
    , m_parent(nullptr)
    , m_item_class(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("TabUI/TabUI.edj");
}

TabUI::~TabUI()
{

}

void TabUI::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //m_parent = p;
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_tab_layout = elm_layout_add(parent);
    elm_layout_file_set(m_tab_layout, m_edjFilePath.c_str(), "tab-layout");
    evas_object_size_hint_weight_set(m_tab_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_tab_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_tab_layout);

    showActionBar();
    showTopButtons();

    m_gengrid = elm_gengrid_add(m_tab_layout);
    elm_object_part_content_set(m_tab_layout, "tab_gengird", m_gengrid);

    evas_object_smart_callback_add(m_gengrid, "item,focused", focusItem, nullptr);
    evas_object_smart_callback_add(m_gengrid, "item,unfocused", unFocusItem, nullptr);
    evas_object_smart_callback_add(m_gengrid, "activated", _itemSelected, this);

    if (!m_item_class) {
        m_item_class = elm_gengrid_item_class_new();
        m_item_class->item_style = "tab_item";
        m_item_class->func.text_get = _grid_text_get;
        m_item_class->func.content_get =  _tab_grid_content_get;
        m_item_class->func.state_get = nullptr;
        m_item_class->func.del = nullptr;
    }

    M_ASSERT(m_parent);
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
    elm_gengrid_highlight_mode_set(m_gengrid, EINA_TRUE);
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);

    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
    elm_gengrid_item_size_set(m_gengrid, 364 * efl_scale, 320 * efl_scale);
}


void TabUI::showActionBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_genListActionBar = elm_genlist_add(m_tab_layout);
    elm_object_part_content_set(m_tab_layout, "action_bar_genlist", m_genListActionBar);
    elm_genlist_homogeneous_set(m_genListActionBar, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListActionBar, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListActionBar, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListActionBar, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genListActionBar, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genListActionBar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

//  evas_object_smart_callback_add(m_genList, "item,focused", focusItem, this);
//  evas_object_smart_callback_add(m_genList, "item,unfocused", unFocusItem, nullptr);*/

    m_itemClassActionBar = elm_genlist_item_class_new();
    m_itemClassActionBar->item_style = "action_bar_items";
    m_itemClassActionBar->func.text_get = nullptr; // &listTopItemTextGet;
    m_itemClassActionBar->func.content_get = &listActionBarContentGet;
    m_itemClassActionBar->func.state_get = nullptr;
    m_itemClassActionBar->func.del = nullptr;

    ItemData *id = new ItemData;
    id->tabUI = this;
    Elm_Object_Item* elmItem = elm_genlist_item_append(m_genListActionBar,    //genlist
                                                       m_itemClassActionBar,  //item Class
                                                       id,
                                                       nullptr,               //parent item
                                                       ELM_GENLIST_ITEM_NONE, //item type
                                                       nullptr,
                                                       nullptr                //data passed to above function
                                                      );
    id->e_item = elmItem;

    ItemData *id2 = new ItemData;
    id2->tabUI = this;
    Elm_Object_Item* elmItem2 = elm_genlist_item_append(m_genListActionBar,    //genlist
                                                        m_itemClassActionBar,  //item Class
                                                        id2,
                                                        nullptr,               //parent item
                                                        ELM_GENLIST_ITEM_NONE, //item type
                                                        nullptr,
                                                        nullptr                //data passed to above function
                                                       );
    id2->e_item = elmItem2;

    ItemData *id3 = new ItemData;
    id3->tabUI = this;
    Elm_Object_Item* elmItem3 = elm_genlist_item_append(m_genListActionBar  ,  //genlist
                                                        m_itemClassActionBar,  //item Class
                                                        id3,
                                                        nullptr,               //parent item
                                                        ELM_GENLIST_ITEM_NONE, //item type
                                                        nullptr,
                                                        nullptr                //data passed to above function
                                                       );
    id3->e_item = elmItem3;

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

Evas_Object* TabUI::listActionBarContentGet(void* data, Evas_Object* obj , const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (obj && part) {
        const char *part_name1 = "newtab_click";
        static const int part_name1_len = strlen(part_name1);
        const char *part_name2 = "newincognitotab_clic";
        static const int part_name2_len = strlen(part_name2);
        const char *part_name3 = "closetabs_click";
        static const int part_name3_len = strlen(part_name3);
        const char *part_name4 = "close_click";
        static const int part_name4_len = strlen(part_name4);

        if (!strncmp(part_name1, part, part_name1_len)) {
            Evas_Object *newtabButton = elm_button_add(obj);
            elm_object_style_set(newtabButton, "tab_button");
            evas_object_smart_callback_add(newtabButton, "clicked", tizen_browser::base_ui::TabUI::_newtab_clicked, data);
            return newtabButton;
        }
        if (!strncmp(part_name2, part, part_name2_len)) {
            Evas_Object *newincognitotabButton = elm_button_add(obj);
            elm_object_style_set(newincognitotabButton, "tab_button");
            evas_object_smart_callback_add(newincognitotabButton, "clicked", tizen_browser::base_ui::TabUI::_newincognitotab_clicked, data);
            return newincognitotabButton;
        }
        if (!strncmp(part_name3, part, part_name3_len)) {
            Evas_Object *closetabsButton = elm_button_add(obj);
            elm_object_style_set(closetabsButton, "tab_button");
            evas_object_smart_callback_add(closetabsButton, "clicked", tizen_browser::base_ui::TabUI::_closetabs_clicked, data);
            return closetabsButton;
        }
        if (!strncmp(part_name4, part, part_name4_len)) {
            Evas_Object *close_click = elm_button_add(obj);
            elm_object_style_set(close_click, "tab_button");
            evas_object_smart_callback_add(close_click, "clicked", TabUI::close_clicked_cb, data);
            return close_click;
        }
    }
    return nullptr;
}

void TabUI::close_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData * id = static_cast<ItemData*>(data);
        id->tabUI->closeTabUIClicked(std::string());
        id->tabUI->clearItems();
    }
}

void TabUI::hide()
{
    evas_object_hide(elm_layout_content_get(m_tab_layout, "action_bar_genlist"));
    evas_object_hide(elm_layout_content_get(m_tab_layout, "top_bar_genlist"));
    evas_object_hide(elm_layout_content_get(m_tab_layout, "tab_gengird"));
    evas_object_hide(m_tab_layout);
}

void TabUI::showTopButtons()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_genListTop = elm_genlist_add(m_tab_layout);
    elm_object_part_content_set(m_tab_layout, "top_bar_genlist", m_genListTop);
    elm_genlist_homogeneous_set(m_genListTop, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListTop, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListTop, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListTop, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genListTop, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genListTop, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

//  evas_object_smart_callback_add(m_genList, "item,focused", focusItem, this);
//  evas_object_smart_callback_add(m_genList, "item,unfocused", unFocusItem, nullptr);

    m_itemClassTop = elm_genlist_item_class_new();
    m_itemClassTop->item_style = "top_buttons";
    m_itemClassTop->func.text_get = nullptr;
    m_itemClassTop->func.content_get = &listTopButtonItemsContentGet;
    m_itemClassTop->func.state_get = nullptr;
    m_itemClassTop->func.del = nullptr;

    ItemData *id = new ItemData;
    id->tabUI = this;
    Elm_Object_Item *elmItem = elm_genlist_item_append(m_genListTop,          //genlist
                                                       m_itemClassTop,        //item Class
                                                       id,
                                                       nullptr,               //parent item
                                                       ELM_GENLIST_ITEM_NONE, //item type
                                                       nullptr,
                                                       nullptr                //data passed to above function
                                                      );
    id->e_item = elmItem;
    ItemData *id2 = new ItemData;
    id2->tabUI = this;
    Elm_Object_Item *elmItem2 = elm_genlist_item_append(m_genListTop,          //genlist
                                                        m_itemClassTop,        //item Class
                                                        id2,
                                                        nullptr,               //parent item
                                                        ELM_GENLIST_ITEM_NONE, //item type
                                                        nullptr,
                                                        nullptr                //data passed to above function
                                                       );
    id2->e_item = elmItem2;

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

Evas_Object* TabUI::listTopButtonItemsContentGet(void* data, Evas_Object* obj , const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj && part) {
        const char *part_name1 = "openedtabs_button";
        static const int part_name1_len = strlen(part_name1);
        const char *part_name2 = "onotherdevices_button";
        static const int part_name2_len = strlen(part_name2);

        if (!strncmp(part_name1, part, part_name1_len)) {
            Evas_Object *openedtabsButton = elm_button_add(obj);
            elm_object_style_set(openedtabsButton, "tab_button");
            evas_object_smart_callback_add(openedtabsButton, "clicked", tizen_browser::base_ui::TabUI::_openedtabs_clicked, data);
            return openedtabsButton;
        }
        if (!strncmp(part_name2, part, part_name2_len)) {
            Evas_Object *onotherdevicesButton = elm_button_add(obj);
            elm_object_style_set(onotherdevicesButton, "tab_button");
            evas_object_smart_callback_add(onotherdevicesButton, "clicked", tizen_browser::base_ui::TabUI::_onotherdevices_clicked, data);
            return onotherdevicesButton;
        }
    }
    return nullptr;
}

void TabUI::_newtab_clicked(void * data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* itemData = static_cast<ItemData*>(data);
        itemData->tabUI->clearItems();
        itemData->tabUI->newTabClicked(std::string());
    }

}
void TabUI::_openedtabs_clicked(void*, Evas_Object*, void*)
{
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void TabUI::_onotherdevices_clicked(void*, Evas_Object*, void*)
{
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void TabUI::_newincognitotab_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* itemData = static_cast<ItemData*>(data);
        itemData->tabUI->clearItems();
        itemData->tabUI->newIncognitoTabClicked(std::string());
    }
}

void TabUI::_closetabs_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        ItemData* itemData = static_cast<ItemData*>(data);
        itemData->tabUI->closeTabsClicked(std::string());
    }
}

void TabUI::addTabItem(std::shared_ptr<tizen_browser::basic_webengine::TabContent> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (m_map_tab_views.size() >= 10)
        return;
    TabItemData *itemData = new TabItemData();
    itemData->item = hi;
    itemData->tabUI = std::shared_ptr<tizen_browser::base_ui::TabUI>(this);
    Elm_Object_Item* tabView = elm_gengrid_item_append(m_gengrid, m_item_class, itemData, nullptr, this);
    m_map_tab_views.insert(std::pair<std::string,Elm_Object_Item*>(hi->getTitle(),tabView));

    // unselect by default
    elm_gengrid_item_selected_set(tabView, EINA_FALSE);
    setEmptyGengrid(false);
}

void TabUI::addTabItems(std::vector<std::shared_ptr<tizen_browser::basic_webengine::TabContent>>items)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    //limiting number of added items to 10
    auto eit = std::min(items.end(), items.begin() + 10);
    for (auto it = items.begin(); it != eit; ++it) {
        addTabItem(*it);
    }
}

char* TabUI::_grid_text_get(void *data, Evas_Object*, const char *part)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data && part) {
        TabItemData *itemData = static_cast<TabItemData*>(data);
        const char *part_name1 = "tab_title";
        static const int part_name1_len = strlen(part_name1);
        const char *part_name2 = "tab_url";
        static const int part_name2_len = strlen(part_name2);

        if (!strncmp(part_name1, part, part_name1_len)) {
            return strdup(itemData->item->getTitle().c_str());
        }
        if (!strncmp(part_name2, part, part_name2_len)) {
            //return strdup(itemData->item->getUrl().c_str());
            return strdup("");
        }
    }
    return strdup("");
}

Evas_Object * TabUI::_tab_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    if (data && obj && part) {
        TabItemData *itemData = static_cast<TabItemData*>(data);
        const char *part_name1 = "tab_thumbnail";
        static const int part_name1_len = strlen(part_name1);
        const char *part_name2 = "tab_thumbButton";
        static const int part_name2_len = strlen(part_name2);

        if (!strncmp(part_name2, part, part_name2_len)) {
            Evas_Object *thumbButton = elm_button_add(obj);
            elm_object_style_set(thumbButton, "tab_thumbButton");
            evas_object_smart_callback_add(thumbButton, "clicked", tizen_browser::base_ui::TabUI::_thumbSelected, data);
            return thumbButton;
        }
        if (!strncmp(part_name1, part, part_name1_len)) {
            if (itemData->item->getThumbnail()) {
                return tizen_browser::tools::EflTools::getEvasImage(itemData->item->getThumbnail(), itemData->tabUI->m_parent);
            }
        }
    }
    return nullptr;
}

void TabUI::_itemSelected(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
//  self->historyClicked(itemData->item);
}

void TabUI::_thumbSelected(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data) {
        TabItemData *itemData = static_cast<TabItemData*>(data);
        itemData->tabUI->clearItems();
        itemData->tabUI->tabClicked(itemData->item->getId());
    }
}

void TabUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    hide();
    elm_gengrid_clear(m_gengrid);
    elm_genlist_clear(m_genListActionBar);
    elm_genlist_clear(m_genListTop);
    m_map_tab_views.clear();
}

Evas_Object* TabUI::createNoHistoryLabel()
{
    Evas_Object *label = elm_label_add(m_parent);
    elm_object_text_set(label, "No favorite websites.");
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    return label;
}

void TabUI::setEmptyGengrid(bool setEmpty)
{
    if (setEmpty) {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", createNoHistoryLabel());
    } else {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", nullptr);
    }
}

void TabUI::focusItem(void*, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (event_info) {
        Elm_Object_Item *item = static_cast<Elm_Object_Item*>(event_info);
        elm_object_item_signal_emit( item, "mouse,in", "over2");

        // selected manually
        elm_gengrid_item_selected_set(item, EINA_TRUE);
    }
}

void TabUI::unFocusItem(void*, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (event_info) {
        Elm_Object_Item *item = static_cast<Elm_Object_Item*>(event_info);
        elm_object_item_signal_emit( item, "mouse,out", "over2");

        // unselected manually
        elm_gengrid_item_selected_set(item, EINA_FALSE);
    }
}

}
}
