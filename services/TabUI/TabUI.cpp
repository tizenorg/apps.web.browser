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

TabUI::TabUI()
    : m_tab_layout(nullptr)
    , m_gengrid(nullptr)
    , m_parent(nullptr)
    , m_item_class(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("TabUI/TabUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    createTabItemClass();
    editMode = false;
    onOtherDevicesSwitch = false;
}

TabUI::~TabUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_gengrid_item_class_free(m_item_class);
}

void TabUI::createTabItemClass()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_item_class) {
        m_item_class = elm_gengrid_item_class_new();
        m_item_class->item_style = "tab_item";
        m_item_class->func.text_get = _grid_text_get;
        m_item_class->func.content_get =  _tab_grid_content_get;
        m_item_class->func.state_get = nullptr;
        m_item_class->func.del = nullptr;
    }
}

void TabUI::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
    //TODO:check if there is no some memory leak due to not destroying previous instance of this UI element.
    m_tab_layout = createTabUILayout(parent);
    evas_object_show(m_tab_layout);
}

Evas_Object* TabUI::createTabUILayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    Evas_Object* tab_layout = elm_layout_add(parent);
    elm_layout_file_set(tab_layout, m_edjFilePath.c_str(), "tab-layout");
    evas_object_size_hint_weight_set(tab_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(tab_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    //Create button bars and put them to layout's swallows
    Evas_Object* buttonBar = createTopButtons(tab_layout);
    elm_object_part_content_set(tab_layout, "top_bar", buttonBar);

    buttonBar = createActionBar(tab_layout);
    elm_object_part_content_set(tab_layout, "action_bar", buttonBar);

    //create gengrid containing tabs
    m_gengrid = elm_gengrid_add(tab_layout);
    elm_object_part_content_set(tab_layout, "tab_gengrid", m_gengrid);

    M_ASSERT(m_parent);
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(m_gengrid, EINA_TRUE);
    elm_gengrid_highlight_mode_set(m_gengrid, EINA_TRUE);
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);

    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
    elm_gengrid_item_size_set(m_gengrid, 364 * efl_scale, 320 * efl_scale);
    evas_object_event_callback_add(m_gengrid, EVAS_CALLBACK_MOUSE_IN, _focus_in, this);

    return tab_layout;
}


Evas_Object* TabUI::createActionBar(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    Evas_Object* actionBarLayout = elm_layout_add(parent);
    elm_layout_file_set(actionBarLayout, m_edjFilePath.c_str(), "action_bar_layout");
    evas_object_size_hint_weight_set(actionBarLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(actionBarLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(actionBarLayout);

    Evas_Object* button = elm_button_add(actionBarLayout);
    elm_object_style_set(button, "tab_button");
    evas_object_smart_callback_add(button, "clicked", _newtab_clicked, this);
    elm_object_part_content_set(actionBarLayout, "newtab_click", button);

    button = elm_button_add(actionBarLayout);
    elm_object_style_set(button, "tab_button");
    evas_object_smart_callback_add(button, "clicked", _newincognitotab_clicked, this);
    elm_object_part_content_set(actionBarLayout, "newincognitotab_click", button);

    button = elm_button_add(actionBarLayout);
    elm_object_style_set(button, "tab_button");
    evas_object_smart_callback_add(button, "clicked", _closetabs_clicked, this);
    elm_object_part_content_set(actionBarLayout, "closetabs_click", button);

    button = elm_button_add(actionBarLayout);
    elm_object_style_set(button, "tab_button");
    evas_object_smart_callback_add(button, "clicked", _close_clicked, this);
    elm_object_part_content_set(actionBarLayout, "close_click", button);

    return actionBarLayout;
}

void TabUI::_close_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI * tabUI = static_cast<TabUI*>(data);
        tabUI->closeTabUIClicked(std::string());
        tabUI->clearItems();
        tabUI->editMode = false;
        tabUI->onOtherDevicesSwitch = false;
    }
}

void TabUI::hide()
{
    evas_object_hide(elm_layout_content_get(m_tab_layout, "action_bar"));
    evas_object_hide(elm_layout_content_get(m_tab_layout, "top_bar"));
    evas_object_hide(elm_layout_content_get(m_tab_layout, "tab_gengrid"));
    evas_object_hide(m_tab_layout);
}

Evas_Object* TabUI::createTopButtons(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);

    Evas_Object* topLayout = elm_layout_add(parent);
    elm_layout_file_set(topLayout, m_edjFilePath.c_str(), "top_buttons_layout");
    evas_object_size_hint_weight_set(topLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(topLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Evas_Object *button = elm_button_add(topLayout);
    elm_object_style_set(button, "tab_button");
    evas_object_smart_callback_add(button, "clicked", _openedtabs_clicked, this);
    evas_object_show(button);
    elm_layout_content_set(topLayout, "openedtabs_click", button);

    button = elm_button_add(topLayout);
    elm_object_style_set(button, "tab_button");
    evas_object_smart_callback_add(button, "clicked", _onotherdevices_clicked, this);
    evas_object_show(button);
    elm_layout_content_set(topLayout, "onotherdevices_click", button);

    return topLayout;
}

void TabUI::_newtab_clicked(void * data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        tabUI->clearItems();
        tabUI->newTabClicked(std::string());
        tabUI->editMode = false;
        tabUI->onOtherDevicesSwitch = false;
    }

}
void TabUI::_openedtabs_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        if(tabUI->onOtherDevicesSwitch) {
            tabUI->onOtherDevicesSwitch = false;
            evas_object_show(tabUI->m_gengrid);
            elm_layout_text_set(elm_layout_content_get(tabUI->m_tab_layout, "action_bar"), "closetabs_text", "Close Tabs");
            tabUI->editMode = false;
        }
    }
}

void TabUI::_onotherdevices_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        if(!tabUI->onOtherDevicesSwitch) {
            tabUI->onOtherDevicesSwitch = true;
            evas_object_hide(tabUI->m_gengrid);
            elm_layout_text_set(elm_layout_content_get(tabUI->m_tab_layout, "action_bar"), "closetabs_text", "Close Tabs");
            tabUI->editMode = false;
        }
    }
}

void TabUI::_newincognitotab_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        tabUI->clearItems();
        tabUI->newIncognitoTabClicked(std::string());
        tabUI->editMode = false;
        tabUI->onOtherDevicesSwitch = false;
    }
}

void TabUI::_closetabs_clicked(void* data, Evas_Object* obj, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        TabUI* tabUI = static_cast<TabUI*>(data);
        if (!tabUI->editMode && !tabUI->onOtherDevicesSwitch) {
            tabUI->editMode = true;
            BROWSER_LOGD("[%s:%d] --------> edit mode: %d ", __PRETTY_FUNCTION__, __LINE__, tabUI->editMode);
            elm_layout_text_set(elm_layout_content_get(tabUI->m_tab_layout, "action_bar"), "closetabs_text", "Close all");
        } else if (tabUI->editMode && !tabUI->onOtherDevicesSwitch) {
            tabUI->closeAllTabs();
            elm_gengrid_realized_items_update(tabUI->m_gengrid);
        }
    }

}

void TabUI::addTabItem(std::shared_ptr<tizen_browser::basic_webengine::TabContent> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    TabItemData *itemData = new TabItemData();
    itemData->item = hi;
    itemData->tabUI = std::shared_ptr<tizen_browser::base_ui::TabUI>(this);
    Elm_Object_Item* tabView = elm_gengrid_item_append(m_gengrid, m_item_class, itemData, _thumbSelected, itemData);
    m_map_tab_views.insert(std::pair<std::string,Elm_Object_Item*>(hi->getTitle(),tabView));

    // unselect by default
    elm_gengrid_item_selected_set(tabView, EINA_FALSE);
    setEmptyGengrid(false);
}

void TabUI::addTabItems(std::vector<std::shared_ptr<tizen_browser::basic_webengine::TabContent>>items)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    for (auto it = items.begin(); it < items.end(); it++) {
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
}

void TabUI::_thumbSelected(void *data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data) {
        TabItemData *itemData = static_cast<TabItemData*>(data);
        if (!itemData->tabUI->editMode) {
            itemData->tabUI->clearItems();
            itemData->tabUI->tabClicked(itemData->item->getId());
        } else {
            itemData->tabUI->closeTabsClicked(itemData->item->getId());
            Elm_Object_Item* it = elm_gengrid_selected_item_get(itemData->tabUI->m_gengrid);
            elm_object_item_del(it);
            elm_gengrid_realized_items_update(itemData->tabUI->m_gengrid);
            int tabsNumber = *(itemData->tabUI->tabsCount());
            BROWSER_LOGD("%s:%d %s, items: %d", __FILE__, __LINE__, __func__, tabsNumber);
            if (!tabsNumber)
                itemData->tabUI->hide();
        }
    }
}

void TabUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    hide();
    elm_gengrid_clear(m_gengrid);
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

void TabUI::closeAllTabs()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* it = elm_gengrid_first_item_get(m_gengrid);
    while (it) {
        TabItemData *item = (TabItemData *)elm_object_item_data_get(it);
        item->tabUI->closeTabsClicked(item->item->getId());
        it = elm_gengrid_item_next_get(it);
    }
    hide();
}

void TabUI::setEmptyGengrid(bool setEmpty)
{
    if (setEmpty) {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", createNoHistoryLabel());
    } else {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", nullptr);
    }
}

void TabUI::_focus_in(void * data, Evas*, Evas_Object * obj, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Event_Mouse_In* ee  = (Evas_Event_Mouse_In*)event_info;
    int x, y;
    TabUI* tabUI = static_cast<TabUI*>(data);
    Elm_Object_Item* it = elm_gengrid_at_xy_item_get(tabUI->m_gengrid, ee->canvas.x, ee->canvas.y, &x, &y);
    if(it && tabUI->editMode)
        elm_object_item_signal_emit(it, "selected", "over3");
}

}
}
