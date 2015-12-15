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
    void hide();
    Evas_Object *getContent();
    virtual std::string getName();

    void setFoldersId(unsigned int all, unsigned int special);
#if PROFILE_MOBILE
    void addNewFolder();
#endif
    //void addAllFolder(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >,  std::string);
    //void addSpecialFolder(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >,  std::string);
    void addCustomFolders(services::SharedBookmarkFolderList folders);
    void addCustomFolders(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >);

    boost::signals2::signal<void ()> closeBookmarkManagerClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemClicked;
    boost::signals2::signal<void (int)> customFolderClicked;
    boost::signals2::signal<void ()> allFolderClicked;
    //Special folder - Bookmark Bar on tv and Mobile on odroid
    boost::signals2::signal<void ()> specialFolderClicked;
#if PROFILE_MOBILE
    boost::signals2::signal<void ()> newFolderItemClicked;
#endif

private:
    typedef struct
    {
        std::string name;
        int count;
        unsigned int folder_id;
        std::shared_ptr<tizen_browser::base_ui::BookmarkManagerUI> bookmarkManagerUI;
    } FolderData;

    Evas_Object* createBookmarksLayout(Evas_Object* parent);
    void createTopContent();
    void createGengrid();
#if !PROFILE_MOBILE
    void createBottomContent();
#endif
    void createGengridItemClasses();
    void addCustomFolder(FolderData*);
    void createFocusVector();

    static void _bookmarkCustomFolderClicked(void * data, Evas_Object *, void *);
    static void _bookmarkAllFolderClicked(void * data, Evas_Object *, void *);
    static void _bookmarkMobileFolderClicked(void * data, Evas_Object *, void *);
    static void _close_clicked_cb(void *data, Evas_Object *, void *);
#if PROFILE_MOBILE
    void createMenuDetails();
    static void _bookmarkNewFolderClicked(void * data, Evas_Object *, void *);
#endif
    static Evas_Object* listItemContentGet(void *data, Evas_Object *obj, const char *part);
    static char* _grid_folder_title_text_get(void *data, Evas_Object *obj, const char *part);

    std::map<std::string,Elm_Object_Item*> m_map_bookmark;
    std::string m_edjFilePath;
    FocusManager m_focusManager;

    Evas_Object *m_parent;
    Evas_Object *b_mm_layout;
    Evas_Object *m_topContent;
    Evas_Object *m_gengrid;
#if !PROFILE_MOBILE
    Evas_Object *m_bottom_content;
#else
    Elm_Gengrid_Item_Class * m_folder_new_item_class;
#endif
    Elm_Gengrid_Item_Class * m_folder_all_item_class;
    Elm_Gengrid_Item_Class * m_folder_mobile_item_class;
    Elm_Gengrid_Item_Class * m_folder_custom_item_class;

    unsigned int m_all_folder_id;
    unsigned int m_special_folder_id;
};

}
}

#endif // BOOKMARKSUI_H
