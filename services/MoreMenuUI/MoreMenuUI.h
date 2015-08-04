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

#ifndef MOREMENUUI_H
#define MOREMENUUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
//#include "../SimpleUI/SimpleUI.h"

#include "BookmarkItem.h"
#include "AddBookmarkPopup.h"
#include "NewFolderPopup.h"
#include "../BookmarkManagerUI/AddNewFolderPopup.h"
#include "services/HistoryService/HistoryItem.h"

namespace tizen_browser{
namespace base_ui{

class SimpleUI;
enum ItemType {
    READER_MODE,
    BOOKMARK_MANAGER,
    HISTORY,
    SCREEN_ZOOM,
    START_MINIBROWSER,
    FOCUS_MODE,
    VIEW_MOBILE_WEB,
    SHARE,
    SETTINGS,
    EXIT_BROWSER
} item;

class BROWSER_EXPORT MoreMenuUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    MoreMenuUI();
    ~MoreMenuUI();
    void show(Evas_Object *main_layout);
    void showCurrentTab(const std::shared_ptr<tizen_browser::services::HistoryItem> item);
    virtual std::string getName();
    void addItems();
    void hide();
    void clearItems();
    void getBookmarkFolderList(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > );

    boost::signals2::signal<void (std::string)> historyUIClicked;
    boost::signals2::signal<void (const char*, int, int)> BookmarkFolderCreated;
    boost::signals2::signal<void (std::string)> closeMoreMenuClicked;
    boost::signals2::signal<void (std::string)> BookmarkFoldersListImport;
    boost::signals2::signal<void (const std::string&)> settingsClicked;
    boost::signals2::signal<void (int)> AddBookmarkInput;
private:
    static char* _grid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_content_get(void *data, Evas_Object *obj, const char *part);
    static void _itemSelected(void * data, Evas_Object * obj, void * event_info);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);

    static Evas_Object* listItemContentGet(void *data, Evas_Object *obj, const char *part);
    static char*        listItemTextGet(void *data, Evas_Object *obj, const char *part);

    void newFolderPopup(std::string);
    void NewFolderCreate(Evas_Object * popup_content);
    void CancelClicked(Evas_Object * popup_content);

    void AddBookmarkPopupCalled();
    void addToBookmarks(int folder_id);
    static void item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void star_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void close_clicked_cb(void *data, Evas_Object *obj, void *event_info);

    static void __cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
    static void __cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);

private:
    Evas_Object *m_genList;
    std::shared_ptr<tizen_browser::base_ui::NewFolderPopup > m_popup;
    Evas_Object *m_mm_layout;
    std::shared_ptr<tizen_browser::base_ui::AddBookmarkPopup > popup;
    Elm_Genlist_Item_Class *m_itemClass;
    Evas_Object *m_gengrid;
    Evas_Object *m_parent;
    Elm_Gengrid_Item_Class * m_item_class;
    std::map<ItemType,Elm_Object_Item*> m_map_menu_views;
    std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > m_map_bookmark_folder_list;
    std::string edjFilePath;
    std::string m_folderName;
    bool m_gengridSetup;

    static void focusItem(void* data, Evas_Object* obj, void* event_info);
    static void unFocusItem(void* data, Evas_Object* obj, void* event_info);
};

}
}

#endif // BOOKMARKSUI_H
