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

#ifndef QUICKACCESS_H
#define QUICKACCESS_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"
#include "DetailPopup.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT QuickAccess
        : public tizen_browser::core::AbstractService
{
public:
    QuickAccess();
    ~QuickAccess();
    void init(Evas_Object *main_layout);
    Evas_Object* getContent();
    void showMostVisited(std::shared_ptr<services::HistoryItemVector> vec);
    void showBookmarks(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > vec);
    void hideUI();
    void showUI();
    virtual std::string getName();
    void openDetailPopup(std::shared_ptr<services::HistoryItem> currItem, std::shared_ptr<services::HistoryItemVector> prevItems);
    bool isDesktopMode() const;
    void setDesktopMode(bool mode);
    DetailPopup & getDetailPopup();
    void backButtonClicked();
    inline bool isMostVisitedActive() const;
    void refreshFocusChain();

    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::HistoryItem>, int)> mostVisitedTileClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::HistoryItem>, bool)> openURLInNewTab;
    boost::signals2::signal<void ()> mostVisitedClicked;
    boost::signals2::signal<void ()> bookmarkClicked;
    boost::signals2::signal<void ()> bookmarkManagerClicked;
    boost::signals2::signal<void ()> switchViewToWebPage;

    static const int MAX_THUMBNAIL_WIDTH;
    static const int MAX_THUMBNAIL_HEIGHT;

private:
    void createItemClasses();
    void addHistoryItem(std::shared_ptr<services::HistoryItem>);
    void addHistoryItems(std::shared_ptr<services::HistoryItemVector>);
    void addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem>);
    void addBookmarkItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >);
    void clearHistoryGenlist();
    void clearBookmarkGengrid();
    Evas_Object* createBookmarkGengrid(Evas_Object *parent);
    void showHistory();
    void showBookmarks();

    Evas_Object* createQuickAccessLayout(Evas_Object *parent);
    Evas_Object* createMostVisitedView(Evas_Object *parent);
    Evas_Object* createBookmarksView(Evas_Object *parent);
    Evas_Object* createBottomButton(Evas_Object *parent);
    Evas_Object* createTopButtons(Evas_Object *parent);

    static char* _grid_bookmark_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part);
    static void _thumbBookmarkClicked(void * data, Evas_Object * obj, void * event_info);
    static void _thumbHistoryClicked(void * data, Evas_Object * obj, void * event_info);
    void setEmptyView(bool empty);
    void showNoHistoryLabel();

    static void _mostVisited_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _bookmark_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _bookmark_manager_clicked(void * data, Evas_Object * obj, void * event_info);

    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object *m_bookmarksView;
    Evas_Object *m_mostVisitedView;
    Evas_Object *m_bookmarksButton;
    Evas_Object *m_mostVisitedButton;
    Evas_Object *m_bookmarkGengrid;
    Evas_Object *m_bookmarkManagerButton;
    bool m_after_history_thumb;
    std::vector<Evas_Object *> m_tiles;
    Eina_List* m_parentFocusChain;

    Elm_Gengrid_Item_Class * m_bookmark_item_class;
    DetailPopup m_detailPopup;
    services::HistoryItemVector m_historyItems;
    bool m_gengridSetup;
    std::string edjFilePath;
    bool m_desktopMode;

    static const int MAX_TILES_NUMBER;
    static const int BIG_TILE_INDEX;
    static const int TOP_RIGHT_TILE_INDEX;
    static const int BOTTOM_RIGHT_TILE_INDEX;
    static const std::vector<std::string> TILES_NAMES;

#if PROFILE_MOBILE
    Evas_Object *m_scroller;
    Evas_Object *m_centerLayout;
    static const int FIXED_SIZE_TILES_NUMBER = 3;
#endif
};

}
}

#endif // QUICKACCESS_H
