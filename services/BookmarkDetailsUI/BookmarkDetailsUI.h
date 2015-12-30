/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
 * BookmarkDetailsUI.h
 *
 *  Created on: Dec 10, 2015
 *      Author: m.kawonczyk@samsung.com
 */


#ifndef BOOKMARKDETAILSUI_H
#define BOOKMARKDETAILSUI_H

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
#include "app_i18n.h"

#if PROFILE_MOBILE
#define TAB_SIZE 2
#else
#define TAB_SIZE 1
#endif

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT BookmarkDetailsUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    BookmarkDetailsUI();
    ~BookmarkDetailsUI();
    //AbstractUIComponent interface methods
    void init(Evas_Object *parent);
    void showUI();
    void hideUI();
    void hide();
    Evas_Object *getContent();
    virtual std::string getName();
    void onBackPressed();

    void addBookmarks(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >,  std::string);

    boost::signals2::signal<void ()> closeBookmarkDetailsClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkItemClicked;
#if PROFILE_MOBILE
    boost::signals2::signal<void (std::string)> editFolderButtonClicked;
    boost::signals2::signal<void (std::string)> deleteFolderButtonClicked;
    boost::signals2::signal<void (std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem>>)> removeFoldersButtonClicked;

    void setLandscape(bool state);
    void resetContent();
#endif

private:
    void addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem>);
    void createFocusVector(int state = 0);
    void createGengridItemClasses();
    std::string getFolderName();
    void setEmpty(bool isEmpty, int state = 0);

    Evas_Object* createLayout(Evas_Object* parent, int state = 0);
    void createTopContent(int state = 0);
    void createGengrid(int state = 0);
    void createBottomContent(int state = 0);
#if PROFILE_MOBILE
    void createMenuDetails(int state = 0);
    void resetRemovalMode(bool clear = true);
    static void _more_button_clicked(void *data, Evas_Object *, void *);
    static void _menu_bg_button_clicked(void *data, Evas_Object *, void *);
    static void _edit_button_clicked(void *data, Evas_Object *, void *);
    static void _delete_button_clicked(void *data, Evas_Object *, void *);
    static void _remove_button_clicked(void *data, Evas_Object *, void *);
    static void _cancel_top_button_clicked(void *data, Evas_Object *, void *);
    static void _remove_top_button_clicked(void *data, Evas_Object *, void *);
#endif
    static char* _grid_bookmark_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object* _grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part);
    static void _bookmark_item_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _bookmark_thumbSelected(void * data, Evas_Object *, void *);
    static void _close_button_clicked(void *data, Evas_Object *, void *);

    Evas_Object *m_parent;
    std::string m_edjFilePath[TAB_SIZE];
    FocusManager m_focusManager[TAB_SIZE];
    Evas_Object *m_layout[TAB_SIZE];
    Evas_Object *m_top_content[TAB_SIZE];
    Evas_Object *m_gengrid[TAB_SIZE];
#if !PROFILE_MOBILE
    Evas_Object *m_bottom_content[TAB_SIZE];
#else
    std::map<std::string, Elm_Object_Item*> m_map_bookmark[TAB_SIZE];
    std::map<std::string, bool> m_map_delete;

    Evas_Object *m_more_button[TAB_SIZE];
    Evas_Object *m_menu_bg_button[TAB_SIZE];
    Evas_Object *m_menu[TAB_SIZE];
    Evas_Object *m_edit_button[TAB_SIZE];
    Evas_Object *m_delete_button[TAB_SIZE];
    Evas_Object *m_remove_button[TAB_SIZE];
    Evas_Object *m_cancel_top_button[TAB_SIZE];
    Evas_Object *m_remove_top_button[TAB_SIZE];
    unsigned int m_delete_count;
    bool m_remove_bookmark_mode;
#endif
    Evas_Object *m_close_button[TAB_SIZE];

    Elm_Gengrid_Item_Class * m_bookmark_item_class[TAB_SIZE];
    std::string m_folder_name;
    unsigned int m_rotation_state;
};

}
}

#endif // BOOKMARKDETAILSUI_H
