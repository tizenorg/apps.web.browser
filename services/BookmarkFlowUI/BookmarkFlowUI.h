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
#include "FocusManager.h"
#endif
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"
#include "BookmarkFolder.h"
#include "app_i18n.h"

#define BOOKMARK_FLOW_SERVICE "org.tizen.browser.bookmarkflowui"
#define M_UNUSED(x) (void)(x)

namespace tizen_browser{
namespace base_ui{

struct BookmarkUpdate {
    unsigned int folder_id;
    std::string title;
};

class BROWSER_EXPORT BookmarkFlowUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    BookmarkFlowUI();
    ~BookmarkFlowUI();
    //AbstractUIComponent interface methods
    void init(Evas_Object *parent);
    Evas_Object *getContent();
    void addCustomFolders(services::SharedBookmarkFolderList folders);

    void showUI();
    void hideUI();
    void setState(bool state);
    void setTitle(const std::string& title);
    void setURL(const std::string& title);
    void setFolder(unsigned int folder_id, const std::string& folder_name);
    void setFoldersId(unsigned int all, unsigned int special);
    void resetContent();

    virtual std::string getName();
    void hide();

    boost::signals2::signal<void ()> closeBookmarkFlowClicked;
    boost::signals2::signal<void (BookmarkUpdate)> saveBookmark;
    boost::signals2::signal<void (BookmarkUpdate)> editBookmark;
    boost::signals2::signal<void ()> removeBookmark;
    boost::signals2::signal<void (int)> addFolder;
    boost::signals2::signal<bool ()> isRotated;

private:
    typedef struct
    {
        std::string name;
        unsigned int folder_id;
        std::shared_ptr<tizen_browser::base_ui::BookmarkFlowUI> bookmarkFlowUI;
    } FolderData;

    Evas_Object* createBookmarkFlowLayout();
    void createTopContent();

    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object *m_title_area;
    std::string m_edjFilePath;
    bool m_state;
    unsigned int m_all_folder_id;

    static Evas_Object *_genlist_entry_content_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object *_genlist_folder_content_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object *_genlist_add_to_qa_content_get(void *data, Evas_Object *obj, const char *part);
    static char* _genlist_entry_text_get(void* data, Evas_Object*, const char* part);
    static char* _genlist_text_get(void* data, Evas_Object*, const char* part);
    static char* _genlist_folder_text_get(void* data, Evas_Object*, const char* part);
    static char* _genlist_add_to_qa_text_get(void* data, Evas_Object*, const char* part);

    void createContentsArea();
    void createGenlistItemClasses();
    void createGenlist();
    void listAddCustomFolder(FolderData* item);

    static void _done_clicked(void* data, Evas_Object*, void*);
    static void _cancel_clicked(void* data, Evas_Object*, void*);
    static void _entry_focused(void* data, Evas_Object*, void*);
    static void _entry_unfocused(void* data, Evas_Object*, void*);
    static void _entry_changed(void* data, Evas_Object*, void*);
    static void _input_cancel_clicked(void* data, Evas_Object*, void*);
    static void _folder_clicked(void* data, Evas_Object*, void*);
    static void _folder_dropdown_clicked(void* data, Evas_Object*, void*);
    static void _remove_clicked(void* data, Evas_Object*, void*);
    static void _listCustomFolderClicked(void* data, Evas_Object*, void*);
    static void _add_to_qa_state_changed(void* data, Evas_Object*, void*);
    static void _folder_selector_clicked(void* data, Evas_Object*, void*);
    static void _qa_clicked(void* data, Evas_Object*, void*);

    Evas_Object *m_box;
    Evas_Object *m_entry_layout;
    Evas_Object *m_contents_area;
    Evas_Object *m_remove_button;
    Evas_Object *m_entry;
    Evas_Object *m_qa_checkbox;
    Evas_Object *m_save_box;
    Evas_Object *m_save;
    Evas_Object *m_done_button;
    Evas_Object *m_cancel_box;
    Evas_Object *m_cancel;
    Evas_Object *m_cancel_button;
    Evas_Object *m_input_cancel_button;
    Evas_Object *m_folder_button;
    Evas_Object *m_folder_dropdown_button;
    Evas_Object *m_genlist;

    Elm_Genlist_Item_Class *m_label_item_class;
    Elm_Genlist_Item_Class *m_entry_item_class;
    Elm_Genlist_Item_Class *m_group_item_class;
    Elm_Genlist_Item_Class *m_folder_item_class;
    Elm_Genlist_Item_Class *m_add_to_qa_item_class;

    std::map<unsigned int, Elm_Object_Item*> m_map_folders;
    unsigned int m_folder_id;
    unsigned int m_special_folder_id;
    unsigned int m_max_items;
    const unsigned int MAX_ITEMS = 4;
    const unsigned int MAX_ITEMS_LANDSCAPE = 2;
    const unsigned int GENLIST_HEIGHT = 384;
    const unsigned int GENLIST_HEIGHT_LANDSCAPE = 192;
    bool m_add_to_qa;
};

}
}

#endif // BOOKMARKFLOWUI_H
