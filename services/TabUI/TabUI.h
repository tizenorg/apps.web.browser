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
#include "TabId.h"
namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT TabUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    TabUI();
    ~TabUI();
    void show(Evas_Object *main_layout);
    virtual std::string getName();
    void showActionBar();
    void showTopButtons();
    void clearItems();
    void hide();

    void addTabItem(std::shared_ptr<tizen_browser::basic_webengine::TabContent>);
    void addTabItems(std::vector<std::shared_ptr<tizen_browser::basic_webengine::TabContent> > items);

    boost::signals2::signal<void (const tizen_browser::basic_webengine::TabId&)> tabClicked;
    boost::signals2::signal<void (const std::string & )> newTabClicked;
    boost::signals2::signal<void (const std::string & )> newIncognitoTabClicked;
    boost::signals2::signal<void (const std::string & )> closeTabsClicked;
    boost::signals2::signal<void (const std::string & )> openedTabsClicked;
    boost::signals2::signal<void (const std::string & )> onOtherDevicesClicked;
    boost::signals2::signal<void (const std::string & )> closeTabUIClicked;
private:
    static Evas_Object* listActionBarContentGet(void *data, Evas_Object *obj, const char *part);
    static Evas_Object* listTopButtonItemsContentGet(void *data, Evas_Object *obj, const char *part);

    static char* _grid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _tab_grid_content_get(void *data, Evas_Object *obj, const char *part);
    static void _itemSelected(void * data, Evas_Object * obj, void * event_info);
    static void _item_deleted(void *data, Evas_Object *obj);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);
    static void _deleteBookmark(void *data, Evas_Object *obj, void *event_info);
    static void close_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    void setEmptyGengrid(bool setEmpty);

    static void _openedtabs_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _newtab_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _newincognitotab_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _closetabs_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _onotherdevices_clicked(void * data, Evas_Object * obj, void * event_info);
private:
    Evas_Object *m_tab_layout;
    Evas_Object *m_genListActionBar;
    Elm_Genlist_Item_Class *m_itemClassActionBar;
    Evas_Object *m_genListTop;
    Elm_Genlist_Item_Class *m_itemClassTop;
    Evas_Object *m_gengrid;
    Evas_Object *m_parent;
    
    Elm_Gengrid_Item_Class * m_item_class;
    std::map<std::string,Elm_Object_Item*> m_map_tab_views;
    bool m_gengridSetup;
    std::string m_edjFilePath;
    Evas_Object *createNoHistoryLabel();

    static void focusItem(void* data, Evas_Object* obj, void* event_info);
    static void unFocusItem(void* data, Evas_Object* obj, void* event_info);
};

}
}

#endif // BOOKMARKSUI_H
