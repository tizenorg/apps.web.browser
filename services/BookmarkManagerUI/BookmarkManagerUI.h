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

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"
#include "BookmarkFolder.h"
#include "FocusManager.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT BookmarkManagerUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    BookmarkManagerUI();
    ~BookmarkManagerUI();
    //AbstractUIComponent interface methods
    void init(Evas_Object *parent);
    void showUI();
    void hideUI();
    Evas_Object *getContent();

    virtual std::string getName();
    void addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem>);
    void addBookmarkItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >);
    void addAllFolder(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >,  std::string);
    void addCustomFolders(services::SharedBookmarkFolderList folders);
    void addCustomFolders(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >);

#if PROFILE_MOBILE
    Evas_Object* getDetailsContent();
    void addNewFolder();
    void addMobileFolder(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >,  std::string);
    void showDetailsUI();
    void addDetails(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >,  std::string);
#endif
    void hide();
    void clearItems();
    Evas_Object* createNoHistoryLabel();
    void setEmptyGengrid(bool setEmpty);

    boost::signals2::signal<void ()> closeBookmarkManagerClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemClicked;
    boost::signals2::signal<void (int)> customFolderClicked;
    boost::signals2::signal<void ()> allFolderClicked;
#if PROFILE_MOBILE
    boost::signals2::signal<void ()> mobileFolderClicked;
    boost::signals2::signal<void ()> newFolderItemClicked;
    boost::signals2::signal<void (std::string)> editFolderButtonClicked;
    boost::signals2::signal<void (std::string)> deleteFolderButtonClicked;
#endif

private:
    typedef struct
    {
        std::string name;
        int count;
        int folder_id;
        std::shared_ptr<tizen_browser::base_ui::BookmarkManagerUI> bookmarkManagerUI;
    } FolderData;

    Evas_Object* createBookmarksLayout(Evas_Object* parent);
    void createGenGrid();
    void showTopContent();
    void createGengridItemClasses();
    Evas_Object *getGenList();
    Evas_Object *getGenGrid();

    static char*         _grid_bookmark_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part);

    static void _bookmarkItemClicked(void * data, Evas_Object * obj, void * event_info);
    static void _bookmark_thumbSelected(void * data, Evas_Object *, void *);

    void addCustomFolder(FolderData*);
    static char* _grid_folder_title_text_get(void *data, Evas_Object *obj, const char *part);
    static void _bookmarkCustomFolderClicked(void * data, Evas_Object *, void *);
    static void _bookmarkAllFolderClicked(void * data, Evas_Object *, void *);
#if PROFILE_MOBILE
    void addDetailsItem(std::shared_ptr<tizen_browser::services::BookmarkItem>);
    void createFolderDetailsGenGrid();
    void createFolderDetailsTopContent();
    Evas_Object* createFolderDetailsLayout(Evas_Object* parent);
    void hideFolderDetailsUI();
    void setDetailsEmptyGengrid(bool);
    void createMenuDetails();
    std::string getDetailFolderName();
    static char* _grid_title_text_get(void *data, Evas_Object *obj, const char *part);
    static void _bookmarkNewFolderClicked(void * data, Evas_Object *, void *);
    static void _bookmarkMobileFolderClicked(void * data, Evas_Object *, void *);
    static void _back_details_clicked_cb(void *data, Evas_Object *, void *);
    static void _more_details_clicked_cb(void *data, Evas_Object *, void *);
    static void _editButton_clicked(void *data, Evas_Object *, void *);
    static void _deleteButton_clicked(void *data, Evas_Object *, void *);
    static void _removeButton_clicked(void *data, Evas_Object *, void *);
#endif

    static Evas_Object* listItemContentGet(void *data, Evas_Object *obj, const char *part);
    static char*        listItemTextGet(void *data, Evas_Object *, const char *part);

    static void close_clicked_cb(void *data, Evas_Object *, void *);

    void createFocusVector();

private:
    Evas_Object *m_topContent;
    Evas_Object *b_mm_layout;
    Elm_Genlist_Item_Class *m_itemClass;
    Evas_Object *m_gengrid;
    Evas_Object *m_parent;
    Elm_Gengrid_Item_Class * m_bookmark_item_class;
    std::map<std::string,Elm_Object_Item*> m_map_bookmark;
    std::string edjFilePath;
    bool m_gengridSetup;
    FocusManager m_focusManager;

    Elm_Gengrid_Item_Class * m_folder_all_item_class;
    Elm_Gengrid_Item_Class * m_folder_custom_item_class;
#if PROFILE_MOBILE
    Elm_Gengrid_Item_Class * m_details_item_class;
    Elm_Gengrid_Item_Class * m_folder_new_item_class;
    Elm_Gengrid_Item_Class * m_folder_mobile_item_class;

    Evas_Object *m_details_topContent;
    Evas_Object *b_details_layout;
    Evas_Object *m_details_gengrid;
    Evas_Object *m_menu_details;
    Evas_Object *m_editButton;
    Evas_Object *m_deleteButton;
    Evas_Object *m_removeButton;
#endif
};

}
}

#endif // BOOKMARKSUI_H
