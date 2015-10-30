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

#ifndef TABLIST_H
#define TABLIST_H

#include <boost/signals2/signal.hpp>
#include <Elementary.h>

#include "TabId.h"
#include "SimpleScroller.h"

#include "MenuButton.h"

namespace tizen_browser
{
namespace base_ui
{

class TabList:public MenuButton{

public:
    TabList(std::shared_ptr<Evas_Object> parent, Evas_Object* parentButton);
    void addItem(std::shared_ptr<tizen_browser::basic_webengine::TabContent>);
    void addItems(std::vector<std::shared_ptr<tizen_browser::basic_webengine::TabContent>>);
    void clearItems();
    void replaceItems(std::vector< std::shared_ptr< tizen_browser::basic_webengine::TabContent > > items);
    virtual Evas_Object *getContent();
    virtual ListSize calculateSize();
    virtual Evas_Object* getFirstFocus();
    boost::signals2::signal<void ()> newTabClicked;
    boost::signals2::signal<void (tizen_browser::basic_webengine::TabId)> tabClicked;
    boost::signals2::signal<void (tizen_browser::basic_webengine::TabId)> tabDelete;
    int getItemcCount();
    void setCurrentTabId(tizen_browser::basic_webengine::TabId currentTabId);
    void disableNewTabBtn(bool disabled);
    void onPopupShow();

    void rightPressed();
    void leftPressed();
    void enterPressed();
private:
    std::map<tizen_browser::basic_webengine::TabId, Elm_Object_Item*> m_items;
    tizen_browser::basic_webengine::TabId m_currentTabId;
    Evas_Object *m_list, *m_box, *m_newTabBtn;
    static void _item_clicked_cb(void* data, Evas_Object *obj, void *event_info);
    static void _newTab_clicked_cb(void* data, Evas_Object *obj, void *event_info);
    static void _deleteTab_clicked_cb(void* data, Evas_Object *obj, void *event_info);
    static void focusItem(void* data, Evas_Object* obj, void *event_info);
    static void unFocusItem(void* data, Evas_Object* obj, void *event_info);
    static void newButtonFocused(void* data, Evas_Object* obj, void *event_info);
    static void newButtonUnFocused(void* data, Evas_Object* obj, void *event_info);
    struct tab_data{
        tizen_browser::basic_webengine::TabId id;
        TabList *tab_list;
    };

    Elm_Object_Item *m_lastFocusedItem;
    bool m_deleteSelected;

    /**
     * Above this size tablist is scrolled.
     */
    static const int ScrollBorderValue;
};

}

}

#endif //TABLIST_H
