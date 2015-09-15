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
#include <Eina.h>
#include <memory>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

#include "BookmarkItem.h"
#include "AddBookmarkPopup.h"
#include "NewFolderPopup.h"
#include "../BookmarkManagerUI/AddNewFolderPopup.h"
#include "services/HistoryService/HistoryItem.h"

namespace tizen_browser{
namespace base_ui{

class SimpleUI;
enum ItemType {
#ifdef READER_MODE_ENABLED
    READER_MODE,
#endif
    BOOKMARK_MANAGER,
    HISTORY,
    SCREEN_ZOOM,
#ifdef START_MINIBROWSER_ENABLED
    START_MINIBROWSER,
#endif
    FOCUS_MODE,
    VIEW_MOBILE_WEB,
    VIEW_DESKTOP_WEB,
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

    //AbstractUIComponent interface methods
    void init(Evas_Object* parent);
    Evas_Object* getContent();
    void showUI();
    void hideUI();

    void setDesktopMode(bool desktopMode) {m_desktopMode = desktopMode;}

    void show(Evas_Object *main_layout, bool desktopMode);
    //TODO: remove this function after new view managment introduction.
    void showCurrentTab();
    virtual std::string getName();
    void addItems();
    //TODO: remove this function after new view managment introduction.
    void hide();
    void clearItems();
    void getBookmarkFolderList(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > );
    void setFavIcon(std::shared_ptr<tizen_browser::tools::BrowserImage> favicon);
    void setWebTitle(const std::string& title);
    void setURL(const std::string& url);
    void changeBookmarkStatus(bool data);
    void createToastPopup(const char* text);

    boost::signals2::signal<void ()> addToBookmarkClicked;
    boost::signals2::signal<void (int)> AddBookmarkInput;
    boost::signals2::signal<void (const char*, int)> BookmarkFolderCreated;
    //TODO: remove redundant argument from this signal.
    boost::signals2::signal<void (std::string)> bookmarkManagerClicked;
    boost::signals2::signal<void (std::string)> BookmarkFoldersListImport;
    boost::signals2::signal<void (std::string)> historyUIClicked;
    boost::signals2::signal<void (const std::string&)> settingsClicked;
    //TODO: remove redundant argument from this signal.
    boost::signals2::signal<void (std::string)> closeMoreMenuClicked;
    boost::signals2::signal<void ()> switchToMobileMode;
    boost::signals2::signal<void ()> switchToDesktopMode;
    boost::signals2::signal<bool ()> isBookmark;
    boost::signals2::signal<void ()> deleteBookmark;
private:
    Elm_Gengrid_Item_Class* createItemClass();
    Evas_Object* createMoreMenuLayout(Evas_Object* parent);
    Evas_Object* createGengrid(Evas_Object* parent);
    static char* _grid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_content_get(void *data, Evas_Object *obj, const char *part);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);
    static void _exitClicked();

    void setDocIcon();
    void newFolderPopup(std::string);
    void NewFolderCreate(Evas_Object * popup_content);
    void CancelClicked(Evas_Object * popup_content);

    void AddBookmarkPopupCalled();
    void addToBookmarks(int folder_id);

    static void _star_clicked(void *data, Evas_Object *obj, void *event_info);
    static void _close_clicked(void *data, Evas_Object *obj, void *event_info);
    static void _timeout(void *data, Evas_Object *obj, void *event_info);

    static void __cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
    static void __cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);

    Evas_Object *m_current_tab_bar;
    std::shared_ptr<tizen_browser::base_ui::NewFolderPopup> m_new_folder_popup;
    Evas_Object *m_mm_layout;
    std::shared_ptr<tizen_browser::base_ui::AddBookmarkPopup> m_add_bookmark_popup;
    Evas_Object *m_gengrid;
    Evas_Object *m_parent;
    Evas_Object *m_toastPopup;
    Evas_Object *m_icon;
    Evas_Object *m_bookmarkIcon;
    Elm_Gengrid_Item_Class * m_item_class;
    std::map<ItemType,Elm_Object_Item*> m_map_menu_views;
    std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > m_map_bookmark_folder_list;
    std::string m_edjFilePath;
    std::string m_folderName;
    bool m_gengridSetup;
    bool m_desktopMode;
    Eina_Bool m_isBookmark;
};

}
}

#endif // BOOKMARKSUI_H
