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

#ifndef TABUI_H
#define TABUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "TabIdTypedef.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT TabUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    TabUI();
    ~TabUI();
    //AbstractUIComponent interface implementation
    void showUI();
    void hideUI();
    void init(Evas_Object *parent);
    Evas_Object* getContent();

    virtual std::string getName();

    void addTabItems(std::vector<basic_webengine::TabContentPtr> items);
    bool isEditMode();
    void onBackKey();

    boost::signals2::signal<void (const tizen_browser::basic_webengine::TabId&)> tabClicked;
    boost::signals2::signal<void ()> newTabClicked;
    boost::signals2::signal<void ()> newIncognitoTabClicked;
    boost::signals2::signal<void (const tizen_browser::basic_webengine::TabId&)> closeTabsClicked;
    boost::signals2::signal<void (const std::string & )> openedTabsClicked;
    boost::signals2::signal<void (const std::string & )> onOtherDevicesClicked;
    boost::signals2::signal<void ()> closeTabUIClicked;
    boost::signals2::signal<int () > tabsCount;
    boost::signals2::signal<bool (const tizen_browser::basic_webengine::TabId& )> isIncognito;

private:

    static char* _grid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _tab_grid_content_get(void *data, Evas_Object *obj, const char *part);
    static void _del_item(void* /*data*/, Evas_Object* obj);
    static void _itemSelected(void * data, Evas_Object * obj, void * event_info);
    static void _item_deleted(void *data, Evas_Object *obj);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);
    static void _deleteBookmark(void *data, Evas_Object *obj, void *event_info);
    static void _close_clicked(void *data, Evas_Object *obj, void *event_info);
    void setEmptyGengrid(bool setEmpty);

    static void _openedtabs_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _newtab_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _newincognitotab_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _closetabs_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _close_tab_clicked(void *data, Evas_Object*, void*);
    static void _onotherdevices_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _focus_in(void * data, Evas*, Evas_Object * obj, void * event_info);

    void createTabUILayout();
    Evas_Object* createActionBar(Evas_Object* parent);
    Evas_Object* createGengrid(Evas_Object* parent);
    Evas_Object* createTopButtons(Evas_Object* parent);
    Evas_Object* createBottomBar(Evas_Object* parent);
    Evas_Object* createNoHistoryLabel();
    void createTabItemClass();
    void closeAllTabs();
    void addTabItem(basic_webengine::TabContentPtr);

    Evas_Object *m_tab_layout;
    Evas_Object *m_gengrid;
    Evas_Object *m_parent;
    bool editMode;
    bool onOtherDevicesSwitch;

    Elm_Gengrid_Item_Class * m_item_class;
    std::map<std::string,Elm_Object_Item*> m_map_tab_views;
    bool m_gengridSetup;
    std::string m_edjFilePath;
};

}
}

#endif // BOOKMARKSUI_H
