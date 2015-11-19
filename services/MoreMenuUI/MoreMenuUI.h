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
#include "services/HistoryService/HistoryItem.h"
#include "FocusManager.h"

#include <Ecore_Wayland.h>

#define M_UNUSED(x) (void)(x)

namespace tizen_browser{
namespace base_ui{

class SimpleUI;
enum ItemType {
#if PROFILE_MOBILE
    ADD_TO_BOOKMARK,
    READER_MODE,
    SHARE,
    HISTORY,
    BOOKMARK_MANAGER,
    FIND_ON_PAGE,
    SETTINGS
#else
#ifdef READER_MODE_ENABLED
    READER_MODE,
#endif
    BOOKMARK_MANAGER,
    HISTORY,
    SCREEN_ZOOM,
#ifdef START_MINIBROWSER_ENABLED
    START_MINIBROWSER,
#endif
    VIEW_MOBILE_WEB,
    VIEW_DESKTOP_WEB,
    SHARE,
    SETTINGS,
    EXIT_BROWSER
#endif
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

    void showCurrentTab();
    virtual std::string getName();
    void setFavIcon(std::shared_ptr<tizen_browser::tools::BrowserImage> favicon);
    void setWebTitle(const std::string& title);
    void setURL(const std::string& url);
    void setHomePageInfo();

    void changeBookmarkStatus(bool data);
    void enableAddToBookmarkButton(bool data);
    void createToastPopup(const char* text);
    void setFocus(Eina_Bool focusable);
#if PROFILE_MOBILE
    void blockThumbnails(bool blockThumbnails);
    void setIsBookmark(bool isBookmark);
    void shouldShowFindOnPage(bool show);
    boost::signals2::signal<void ()> findOnPageClicked;
#endif

    boost::signals2::signal<void (int)> addToBookmarkClicked;
    boost::signals2::signal<void ()> bookmarkManagerClicked;
    boost::signals2::signal<void ()> historyUIClicked;
    boost::signals2::signal<void ()> settingsClicked;
    boost::signals2::signal<void ()> closeMoreMenuClicked;
    boost::signals2::signal<void ()> zoomUIClicked;
    boost::signals2::signal<void ()> switchToMobileMode;
    boost::signals2::signal<void ()> switchToDesktopMode;
    boost::signals2::signal<bool ()> isBookmark;
    boost::signals2::signal<void ()> deleteBookmark;
#if PROFILE_MOBILE
    boost::signals2::signal<void (bool)> bookmarkFlowClicked;
#endif
private:
    Elm_Gengrid_Item_Class* createItemClass();
    void createMoreMenuLayout();
#if PROFILE_MOBILE
    void deleteMoreMenuLayout();
#endif
    void createGengrid();
    void addItems();
    void addItem(ItemType item);
    void clearItems();
    static char* _grid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_content_get(void *data, Evas_Object *obj, const char *part);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);
    static void _exitClicked();
    void createFocusVector();

    void setDocIcon();

    static void _star_clicked(void *data, Evas_Object *obj, void *event_info);
    static void _close_clicked(void *data, Evas_Object *obj, void *event_info);

    static void _timeout(void *data, Evas_Object *obj, void *event_info);

    static void __cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
    static void __cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
    Evas_Object *m_current_tab_bar;
    Evas_Object *m_mm_layout;
    Evas_Object *m_gengrid;
    Evas_Object *m_parent;
    Evas_Object *m_toastPopup;
    Evas_Object *m_icon;
    Evas_Object *m_bookmarkIcon;
    Evas_Object *m_bookmarkButton;
    Elm_Gengrid_Item_Class * m_item_class;
    std::map<ItemType,Elm_Object_Item*> m_map_menu_views;
    std::string m_edjFilePath;
    bool m_gengridSetup;
    bool m_desktopMode;
    Eina_Bool m_isBookmark;
    FocusManager m_focusManager;
#if PROFILE_MOBILE
    bool m_shouldShowFindOnPage;
    bool m_blockThumbnails;
#endif
};

}
}

#endif // BOOKMARKSUI_H
