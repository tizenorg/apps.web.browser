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
#if PROFILE_MOBILE
#include "AbstractRotatable.h"
#endif
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"
#include "BookmarkFolder.h"
#include "FocusManager.h"

namespace tizen_browser{
namespace base_ui{

enum BookmarkManagerState {
    Default,
    Edit,
    Delete,
    Reorder
};

class BROWSER_EXPORT BookmarkManagerUI
        : public interfaces::AbstractContextMenu
        , public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
#if PROFILE_MOBILE
        , public tizen_browser::interfaces::AbstractRotatable
#endif
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

    void setFoldersId(unsigned int all, unsigned int special);
#if PROFILE_MOBILE
    void addNewFolder();
    virtual void orientationChanged() override;
#endif
    void onBackPressed();

    //AbstractContextMenu interface implementation
    virtual void showContextMenu() override;
    static void _cm_delete_clicked(void*, Evas_Object*, void*);
    static void _cm_share_clicked(void*, Evas_Object*, void*);
    static void _cm_reorder_clicked(void*, Evas_Object*, void*);
    static void _cm_edit_clicked(void*, Evas_Object*, void*);
    static void _cm_create_folder_clicked(void*, Evas_Object*, void*);

    void addCustomFolders(services::SharedBookmarkFolderList folders);
    void addCustomFolders(services::SharedBookmarkItemList folders);
    void addBookmarkItems(services::SharedBookmarkItem parent, services::SharedBookmarkItemList items);
    void addBookmarkItemCurrentFolder(services::SharedBookmarkItem item);

    boost::signals2::signal<void ()> closeBookmarkManagerClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemEdit;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemOrderEdited;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemDeleted;
    boost::signals2::signal<void (int)> customFolderClicked;
    boost::signals2::signal<void ()> allFolderClicked;
    //Special folder - Bookmark Bar on tv and Mobile on odroid
    boost::signals2::signal<void ()> specialFolderClicked;
#if PROFILE_MOBILE
    boost::signals2::signal<void (int)> newFolderItemClicked;
#endif

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
    void createNavigatorToolbar();
    void createGenlist();
    void createEmptyLayout();
    void createGengrid();
#if !PROFILE_MOBILE
    void createBottomContent();
#endif
    void createGenlistItemClasses();
    void createGengridItemClasses();
    void addCustomFolder(FolderData*);
    void addBookmarkItem(BookmarkData* item);
    void createFocusVector();

    void changeState(BookmarkManagerState state);
    void updateGenlistItems();
    void reoderBookmarkItems();
    void updateNoBookmarkText();
    void updateDeleteTopContent();

    static void _navigatorFolderClicked(void* data, Evas_Object* obj, void* event_info);
    static void _bookmarkItemClicked(void* data, Evas_Object*, void*);
    static void _bookmarkCustomFolderClicked(void * data, Evas_Object *, void *);
    static void _bookmarkAllFolderClicked(void * data, Evas_Object *, void *);
    static void _bookmarkMobileFolderClicked(void * data, Evas_Object *, void *);
    static void _close_clicked(void *data, Evas_Object *, void *);
    static void _cancel_clicked(void *data, Evas_Object *, void *);
    static void _delete_clicked(void *data, Evas_Object *, void *);
    static void _check_state_changed(void *data, Evas_Object *, void *);
    static void _genlist_bookmark_moved(void *data, Evas_Object *, void *);
    static void _select_all_down(void *data, Evas *, Evas_Object *, void *);
    static void _select_all_state_changed(void *data, Evas_Object *obj, void *);
#if PROFILE_MOBILE
    void createMenuDetails();
    static void _bookmarkNewFolderClicked(void * data, Evas_Object *, void *);
#endif
    static Evas_Object* listItemContentGet(void *data, Evas_Object *obj, const char *part);
    static void _grid_content_delete(void *data, Evas_Object *obj);
    static char* _genlist_bookmark_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object *_genlist_bookmark_content_get(void *data, Evas_Object *obj, const char *part);
    static char* _grid_all_folder_title_text_get(void *data, Evas_Object *obj, const char *part);
    static char* _grid_folder_title_text_get(void *data, Evas_Object *obj, const char *part);

    std::string m_edjFilePath;
    FocusManager m_focusManager;

    Evas_Object *m_parent;
    Evas_Object *b_mm_layout;
    Evas_Object *m_topContent;
    Evas_Object *m_cancel_button;
    Evas_Object *m_delete_button;
    Evas_Object *m_navigatorToolbar;
    Evas_Object *m_box;
    Evas_Object *m_genlist;
    Evas_Object *m_empty_layout;
    Evas_Object *m_select_all;
    Evas_Object *m_gengrid;
#if !PROFILE_MOBILE
    Evas_Object *m_bottom_content;

    const unsigned int GENGRID_ITEM_WIDTH = 404;
    const unsigned int GENGRID_ITEM_HEIGHT = 320;
#else
    const unsigned int GENGRID_ITEM_WIDTH = 337;
    const unsigned int GENGRID_ITEM_HEIGHT = 379;
    const unsigned int GENGRID_ITEM_WIDTH_LANDSCAPE = 308;
    const unsigned int GENGRID_ITEM_HEIGHT_LANDSCAPE = 326;

    Elm_Gengrid_Item_Class * m_folder_new_item_class;
#endif
    Elm_Gengrid_Item_Class * m_folder_all_item_class;
    Elm_Gengrid_Item_Class * m_folder_mobile_item_class;
    Elm_Gengrid_Item_Class * m_folder_custom_item_class;
    Elm_Genlist_Item_Class * m_bookmark_item_class;

    unsigned int m_all_folder_id;
    unsigned int m_special_folder_id;

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
