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

#ifndef MAINUI_H
#define MAINUI_H

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

//TODO: This class name is not revelant to what this class actually does.
//Rename this class and file to "QuickAccessUI".
class BROWSER_EXPORT MainUI
        : public tizen_browser::core::AbstractService
{
public:
    MainUI();
    ~MainUI();
    void init(Evas_Object *main_layout);
    Evas_Object* getContent();
    void showMostVisited(std::shared_ptr<services::HistoryItemVector> vec);
    void showBookmarks(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > vec);
    void hide();
    virtual std::string getName();
    void openDetailPopup(std::shared_ptr<services::HistoryItem> currItem, std::shared_ptr<services::HistoryItemVector> prevItems);
    bool isDesktopMode() const;
    void setDesktopMode(bool mode);
    DetailPopup & getDetailPopup();

    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::HistoryItem>, int)> mostVisitedTileClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::HistoryItem>, bool)> openURLInNewTab;
    boost::signals2::signal<void (const std::string & )> mostVisitedClicked;
    boost::signals2::signal<void (const std::string & )> bookmarkClicked;
    boost::signals2::signal<void (const std::string & )> bookmarkManagerClicked;

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
    void showHistory();
    void showBookmarks();
    void clearItems();

    Evas_Object* createQuickAccessLayout(Evas_Object *parent);
    Evas_Object* createMostVisitedView(Evas_Object *parent);
    Evas_Object* createBookmarksView(Evas_Object *parent);
    Evas_Object* createBottomButton(Evas_Object *parent);
    Evas_Object* createTopButtons(Evas_Object *parent);

    static char* _grid_bookmark_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part);
    static void _thumbBookmarkClicked(void * data, Evas_Object * obj, void * event_info);
    static void _thumbClicked(void * data, Evas_Object * obj, void * event_info);
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
    std::vector<Evas_Object *> m_tiles;

    Elm_Gengrid_Item_Class * m_bookmark_item_class;
    DetailPopup m_detailPopup;
    services::HistoryItemVector m_historyItems;
    std::map<std::string,Elm_Object_Item*> m_map_bookmark_views;
    bool m_gengridSetup;
    std::string edjFilePath;
    bool m_desktopMode;

    static const int MAX_TILES_NUMBER;
    static const int BIG_TILE_INDEX;
    static const std::vector<std::string> TILES_NAMES;
};

}
}

#endif // BOOKMARKSUI_H
