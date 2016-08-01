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

#ifndef BOOKMARKMANAGERUI_H
#define BOOKMARKMANAGERUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractContextMenu.h"
#include "AbstractUIComponent.h"
#include "AbstractRotatable.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"
#include "BookmarkFolder.h"

namespace tizen_browser{
namespace base_ui{

enum class BookmarkManagerState {
    Default,
    SelectFolder,
    Edit,
    Delete,
    Reorder
};

class BROWSER_EXPORT BookmarkManagerUI
        : public interfaces::AbstractContextMenu
        , public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
        , public tizen_browser::interfaces::AbstractRotatable
{
public:
    BookmarkManagerUI();
    ~BookmarkManagerUI();
    //AbstractUIComponent interface methods
    void init(Evas_Object *parent);
    void showUI();
    void hideUI();
    void hide();
    Evas_Object *getContent();
    virtual std::string getName();

    void orientationChanged() {};
    void onBackPressed();

    //AbstractContextMenu interface implementation
    virtual void showContextMenu() override;

    void addBookmarkItems(services::SharedBookmarkItem parent,
                          services::SharedBookmarkItemList items,
                          BookmarkManagerState state = BookmarkManagerState::Default);
    void addBookmarkItemCurrentFolder(services::SharedBookmarkItem item);

    boost::signals2::signal<void ()> showHistory;
    boost::signals2::signal<void ()> closeBookmarkManagerClicked;
    boost::signals2::signal<void (services::SharedBookmarkItem)> folderSelected;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemEdit;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemOrderEdited;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemDeleted;
    boost::signals2::signal<void (int)> newFolderItemClicked;

private:
    typedef struct
    {
        std::string name;
        int count;
        unsigned int folder_id;
        BookmarkManagerUI* bookmarkManagerUI;
    } FolderData;

    typedef struct
    {
        services::SharedBookmarkItem bookmarkItem;
        BookmarkManagerUI* bookmarkManagerUI;
    } BookmarkData;

    Evas_Object* createBookmarksLayout(Evas_Object* parent);
    void createTopContent();
    void createModulesToolbar();
    void createNavigatorToolbar();
    void createGenlist();
    void createEmptyLayout();
    void createGenlistItemClasses();
    void addBookmarkItem(BookmarkData* item);

    void changeState(BookmarkManagerState state);
    void reoderBookmarkItems();
    void updateNoBookmarkText();
    void updateDeleteClick(int id);
    void updateDeleteTopContent();

    static void _navigatorFolderClicked(void* data, Evas_Object* obj, void* event_info);
    static void _modules_bookmarks_clicked(void* data, Evas_Object* obj, void* event_info);
    static void _modules_history_clicked(void* data, Evas_Object* obj, void* event_info);
    static void _bookmarkItemClicked(void* data, Evas_Object*, void*);
    static void _cancel_clicked(void *data, Evas_Object *, void *);
    static void _accept_clicked(void *data, Evas_Object *, void *);
    static void _prev_clicked(void *data, Evas_Object *, void *);
    static void _check_state_changed(void *data, Evas_Object *, void *);
    static void _genlist_bookmark_moved(void *data, Evas_Object *, void *);
    static void _select_all_down(void *data, Evas *, Evas_Object *, void *);
    static void _select_all_state_changed(void *data, Evas_Object *obj, void *);
    static void _cm_delete_clicked(void*, Evas_Object*, void*);
    static void _cm_share_clicked(void*, Evas_Object*, void*);
    static void _cm_reorder_clicked(void*, Evas_Object*, void*);
    static void _cm_edit_clicked(void*, Evas_Object*, void*);
    static void _cm_create_folder_clicked(void*, Evas_Object*, void*);
    static char* _genlist_bookmark_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object *_genlist_bookmark_content_get(void *data, Evas_Object *obj, const char *part);

    std::string m_edjFilePath;

    Evas_Object *m_parent;
    Evas_Object *b_mm_layout;
    Evas_Object *m_content;
    Evas_Object *m_cancel_button;
    Evas_Object *m_accept_button;
    Evas_Object *m_prev_button;
    Evas_Object *m_modulesToolbar;
    Evas_Object *m_navigatorToolbar;
    Evas_Object *m_box;
    Evas_Object *m_genlist;
    Evas_Object *m_empty_layout;
    Evas_Object *m_select_all;

    Elm_Genlist_Item_Class * m_bookmark_item_class;

    services::SharedBookmarkItemList m_folder_path;
    BookmarkManagerState m_state;
    bool m_reordered;
    std::map<unsigned int, Elm_Object_Item*> m_map_bookmark;
    std::map<unsigned int, bool> m_map_delete;
    unsigned int m_delete_count;
};

}
}

#endif // BOOKMARKSUI_H
