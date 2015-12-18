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

/*
 * BookmarkFlowUI.cpp
 *
 *  Created on: Nov 10, 2015
 *      Author: m.kawonczyk@samsung.com
 */

#ifndef BOOKMARKFLOWUI_H
#define BOOKMARKFLOWUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#if PROFILE_MOBILE
#include "AbstractUIComponent.h"
#else
#include "AbstractPopup.h"
#endif
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"
#include "BookmarkFolder.h"
#include "FocusManager.h"

#define BOOKMARK_FLOW_SERVICE "org.tizen.browser.bookmarkflowui"
#define M_UNUSED(x) (void)(x)

namespace tizen_browser{
namespace base_ui{

struct BookmarkUpdate {
    unsigned int folder_id;
    std::string title;
};

class BROWSER_EXPORT BookmarkFlowUI
#if PROFILE_MOBILE
        : public tizen_browser::interfaces::AbstractUIComponent
#else
        : public interfaces::AbstractPopup
#endif
        , public tizen_browser::core::AbstractService
{
public:
    BookmarkFlowUI();
    ~BookmarkFlowUI();
    //AbstractUIComponent interface methods
    void init(Evas_Object *parent);
    Evas_Object *getContent();
    void addCustomFolders(services::SharedBookmarkFolderList folders);

#if PROFILE_MOBILE
    void showUI();
    void hideUI();
    void setState(bool state);
    void setTitle(const std::string& title);
    void setURL(const std::string& title);
    void setFolder(unsigned int folder_id, const std::string& folder_name);
    void setSpecialFolderId(unsigned int special);
#else
    static BookmarkFlowUI* createPopup(Evas_Object* parent);
    void show();
    void dismiss();
    void onBackPressed();
    void addNewFolder();
#endif
    virtual std::string getName();
    void hide();

    boost::signals2::signal<void ()> closeBookmarkFlowClicked;
    boost::signals2::signal<void (BookmarkUpdate)> saveBookmark;
    boost::signals2::signal<void (BookmarkUpdate)> editBookmark;
    boost::signals2::signal<void ()> removeBookmark;
    boost::signals2::signal<void ()> addFolder;

private:
    typedef struct
    {
        std::string name;
        unsigned int folder_id;
        std::shared_ptr<tizen_browser::base_ui::BookmarkFlowUI> bookmarkFlowUI;
    } FolderData;

    Evas_Object* createBookmarkFlowLayout();
    void createTitleArea();

    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object *m_titleArea;
    std::string m_edjFilePath;
    bool m_state;

    static char* _folder_title_text_get(void* data, Evas_Object*, const char* part);
#if PROFILE_MOBILE
    void createContentsArea();
    void createGenlistItemClasses();
    void createGenlist();
    void listAddCustomFolder(FolderData* item);

    static void _save_clicked(void* data, Evas_Object*, void*);
    static void _cancel_clicked(void* data, Evas_Object*, void*);
    static void _entry_focused(void* data, Evas_Object*, void*);
    static void _entry_unfocused(void* data, Evas_Object*, void*);
    static void _entry_changed(void* data, Evas_Object*, void*);
    static void _inputCancel_clicked(void* data, Evas_Object*, void*);
    static void _folder_clicked(void* data, Evas_Object*, void*);
    static void _folder_dropdown_clicked(void* data, Evas_Object*, void*);
    static void _remove_clicked(void* data, Evas_Object*, void*);
    static void _listCustomFolderClicked(void* data, Evas_Object*, void*);

    Evas_Object *m_contentsArea;
    Evas_Object *m_removeButton;
    Evas_Object *m_entry;
    Evas_Object *m_saveButton;
    Evas_Object *m_cancelButton;
    Evas_Object *m_inputCancelButton;
    Evas_Object *m_folderButton;
    Evas_Object *m_folder_dropdown_button;
    Evas_Object *m_genlist;

    Elm_Genlist_Item_Class *m_folder_custom_item_class;
    Elm_Genlist_Item_Class *m_folder_selected_item_class;
    std::map<unsigned int, Elm_Object_Item*> m_map_folders;
    unsigned int m_folder_id;
    unsigned int m_special_folder_id;
#else
    void createGengridItemClasses();
    void createGengrid();
    void gridAddCustomFolder(FolderData* item);
    void createFocusVector();

    static void _gridNewFolderClicked(void* data, Evas_Object*, void*);
    static void _gridCustomFolderClicked(void* data, Evas_Object*, void*);
    static void _bg_clicked(void* data, Evas_Object*, void*);

    Evas_Object *m_gengrid;
    Evas_Object *m_bg;
    Elm_Gengrid_Item_Class *m_folder_new_item_class;
    Elm_Gengrid_Item_Class *m_folder_custom_item_class;

    FocusManager m_focusManager;
#endif
};

}
}

#endif // BOOKMARKFLOWUI_H
