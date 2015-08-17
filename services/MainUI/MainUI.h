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

#ifndef MAINUI_H
#define MAINUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT MainUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    MainUI();
    ~MainUI();
    void show(Evas_Object *main_layout);
    void hide();
    virtual std::string getName();
    void clearHistoryGenlist();
    void clearBookmarkGengrid();
    void showHistoryGenlist();
    void showBookmarkGengrid();
    void showTopButtons();
    void showBottomButton();
    void clearItems();

    void addHistoryItem(std::shared_ptr<tizen_browser::services::HistoryItem>);
    void addHistoryItems(std::vector<std::shared_ptr<tizen_browser::services::HistoryItem> >);
    void addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem>);
    void addBookmarkItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >);

    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::HistoryItem>)> historyClicked;
    boost::signals2::signal<void (const std::string & )> mostVisitedClicked;
    boost::signals2::signal<void (const std::string & )> bookmarkClicked;
    boost::signals2::signal<void (const std::string & )> bookmarkManagerClicked;
private:
    static Evas_Object* listItemBottomContentGet(void *data, Evas_Object *obj, const char *part);
    static Evas_Object* listTopItemContentGet(void *data, Evas_Object *obj, const char *part);
    //static char*        listItemTextGet(void *data, Evas_Object *obj, const char *part);

    static char* _grid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_content_get(void *data, Evas_Object *obj, const char *part);
    static char* _grid_bookmark_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part);
    static void _itemSelected(void * data, Evas_Object * obj, void * event_info);
    static void _item_deleted(void *data, Evas_Object *obj);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);
    static void _deleteBookmark(void *data, Evas_Object *obj, void *event_info);
    void setEmptyGengrid(bool setEmpty);

    static void _mostVisited_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _bookmark_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _bookmark_manager_clicked(void * data, Evas_Object * obj, void * event_info);
private:
    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object *m_layoutTop;
    Evas_Object *m_gengrid;
    Evas_Object *m_genListLeft;
    Evas_Object *m_genListCenter;
    Evas_Object *m_genListRight;
    Evas_Object *m_genListBottom;
    Elm_Genlist_Item_Class *m_itemClassBottom;
    Elm_Gengrid_Item_Class * m_big_item_class;
    Elm_Gengrid_Item_Class * m_small_item_class;
    Elm_Gengrid_Item_Class * m_bookmark_item_class;
    std::multimap<std::string,Elm_Object_Item*> m_map_history_views;
    std::map<std::string,Elm_Object_Item*> m_map_bookmark_views;
    bool m_gengridSetup;
    std::string edjFilePath;
    Evas_Object *createNoHistoryLabel();

    static void focusItem(void* data, Evas_Object* obj, void* event_info);
    static void unFocusItem(void* data, Evas_Object* obj, void* event_info);
};

}
}

#endif // BOOKMARKSUI_H
