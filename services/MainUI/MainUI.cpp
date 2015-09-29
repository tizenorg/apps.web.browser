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

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "MainUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"
#include "Tools/GeneralTools.h"

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(MainUI, "org.tizen.browser.mainui")

const int MainUI::MAX_TILES_NUMBER = 5;
const int MainUI::MAX_THUMBNAIL_WIDTH = 840;
const int MainUI::MAX_THUMBNAIL_HEIGHT = 648;
const int MainUI::BIG_TILE_INDEX = 0;
const std::vector<std::string> MainUI::TILES_NAMES = {
    "elm.swallow.big",
    "elm.swallow.small_first",
    "elm.swallow.small_second",
    "elm.swallow.small_third",
    "elm.swallow.small_fourth"
};

typedef struct _HistoryItemData
{
        std::shared_ptr<tizen_browser::services::HistoryItem> item;
        std::shared_ptr<tizen_browser::base_ui::MainUI> mainUI;
} HistoryItemData;

typedef struct _BookmarkItemData
{
        std::shared_ptr<tizen_browser::services::BookmarkItem> item;
        std::shared_ptr<tizen_browser::base_ui::MainUI> mainUI;
} BookmarkItemData;

MainUI::MainUI()
    : m_parent(nullptr)
    , m_layout(nullptr)
    , m_bookmarksView(nullptr)
    , m_mostVisitedView(nullptr)
    , m_bookmarksButton(nullptr)
    , m_mostVisitedButton(nullptr)
    , m_bookmarkGengrid(nullptr)
    , m_bookmark_item_class(nullptr)
    , m_detailPopup(this)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    edjFilePath = EDJE_DIR;
    edjFilePath.append("MainUI/MainUI.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    MainUI::createItemClasses();
}

MainUI::~MainUI()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    elm_gengrid_item_class_free(m_bookmark_item_class);
}

void MainUI::init(Evas_Object* parent)
{
    M_ASSERT(parent);
    m_parent = parent;
}


Evas_Object* MainUI::getContent()
{
    M_ASSERT(m_parent);
    if (!m_layout) {
        m_layout = createQuickAccessLayout(m_parent);
    }
    return m_layout;
}

void MainUI::showMostVisited(std::shared_ptr< services::HistoryItemVector > vec)
{
    addHistoryItems(vec);
    showHistory();
    // update focus chain
    elm_object_focus_custom_chain_append(m_parent, m_mostVisitedButton, NULL);
    elm_object_focus_custom_chain_append(m_parent, m_bookmarksButton, NULL);
}

void MainUI::showBookmarks(std::vector< std::shared_ptr< tizen_browser::services::BookmarkItem > > vec)
{
    addBookmarkItems(vec);
    showBookmarks();
}

void MainUI::createItemClasses()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (!m_bookmark_item_class) {
        m_bookmark_item_class = elm_gengrid_item_class_new();
        m_bookmark_item_class->item_style = "grid_item";
        m_bookmark_item_class->func.text_get = _grid_bookmark_text_get;
        m_bookmark_item_class->func.content_get =  _grid_bookmark_content_get;
        m_bookmark_item_class->func.state_get = nullptr;
        m_bookmark_item_class->func.del = nullptr;
    }
}


Evas_Object* MainUI::createQuickAccessLayout(Evas_Object* parent)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_desktopMode = true;

    Evas_Object* layout = elm_layout_add(parent);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_mostVisitedView = createMostVisitedView(layout);
    m_bookmarksView   = createBookmarksView  (layout);

    showHistory();

    return layout;
}

Evas_Object* MainUI::createMostVisitedView (Evas_Object * parent)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    Evas_Object* mostVisitedLayout = elm_layout_add(parent);
    elm_layout_file_set(mostVisitedLayout, edjFilePath.c_str(), "mv_bookmarks");
    evas_object_size_hint_weight_set(mostVisitedLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (mostVisitedLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Evas_Object* topButtons = createTopButtons(mostVisitedLayout);
    elm_object_part_content_set(mostVisitedLayout, "elm.swallow.layoutTop", topButtons);

    return mostVisitedLayout;
}

Evas_Object* MainUI::createBookmarksView (Evas_Object * parent)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    Evas_Object *bookmarkViewLayout = elm_layout_add(parent);
    elm_layout_file_set(bookmarkViewLayout, edjFilePath.c_str(), "mv_bookmarks");
    evas_object_size_hint_weight_set(bookmarkViewLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bookmarkViewLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_bookmarkGengrid = createBookmarkGengrid(bookmarkViewLayout);
    elm_object_part_content_set(bookmarkViewLayout, "elm.swallow.grid", m_bookmarkGengrid);

    Evas_Object* topButtons = createTopButtons(bookmarkViewLayout);
    elm_object_part_content_set(bookmarkViewLayout, "elm.swallow.layoutTop", topButtons);

    Evas_Object* bottomButton = createBottomButton(bookmarkViewLayout);
    elm_object_part_content_set(bookmarkViewLayout, "elm.swallow.layoutBottom", bottomButton);

    return bookmarkViewLayout;
}

Evas_Object* MainUI::createBookmarkGengrid(Evas_Object *parent)
{
    Evas_Object *bookmarkGengrid = elm_gengrid_add(parent);

    elm_gengrid_align_set(bookmarkGengrid, 0, 0);
    elm_gengrid_select_mode_set(bookmarkGengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(bookmarkGengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(bookmarkGengrid, EINA_FALSE);

    elm_scroller_policy_set(bookmarkGengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(bookmarkGengrid, 0, 327);
    evas_object_size_hint_weight_set(bookmarkGengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bookmarkGengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_item_size_set(bookmarkGengrid, 364 * efl_scale, 320 * efl_scale);

    return bookmarkGengrid;
}

Evas_Object* MainUI::createTopButtons (Evas_Object *parent)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    Evas_Object *layoutTop = elm_layout_add(parent);
    elm_layout_file_set(layoutTop, edjFilePath.c_str(), "top_button_item");
    evas_object_size_hint_weight_set(layoutTop, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layoutTop, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(layoutTop);

    Evas_Object *mostVisitedButton = elm_button_add(layoutTop);
    elm_object_style_set(mostVisitedButton, "invisible_button");
    evas_object_smart_callback_add(mostVisitedButton, "clicked", _mostVisited_clicked, this);
    evas_object_show(mostVisitedButton);
    elm_layout_content_set(layoutTop, "mostvisited_click", mostVisitedButton);
    m_mostVisitedButton = mostVisitedButton;

    Evas_Object *bookmarksButton = elm_button_add(layoutTop);
    elm_object_style_set(bookmarksButton, "invisible_button");
    evas_object_smart_callback_add(bookmarksButton, "clicked", _bookmark_clicked, this);
    evas_object_show(bookmarksButton);
    elm_layout_content_set(layoutTop, "bookmark_click", bookmarksButton);
    m_bookmarksButton = bookmarksButton;

    return layoutTop;
}

Evas_Object* MainUI::createBottomButton(Evas_Object *parent)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    Evas_Object *layoutBottom = elm_layout_add(parent);
    elm_layout_file_set(layoutBottom, edjFilePath.c_str(), "bottom_button_item");

    evas_object_size_hint_weight_set(layoutBottom, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layoutBottom, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(layoutBottom);

    Evas_Object * bookmark_manager_button = elm_button_add(layoutBottom);
    elm_object_style_set(bookmark_manager_button, "invisible_button");
    evas_object_smart_callback_add(bookmark_manager_button, "clicked", _bookmark_manager_clicked, this);
    evas_object_show(bookmark_manager_button);

    elm_object_part_content_set(layoutBottom, "bookmarkmanager_click", bookmark_manager_button);

    return layoutBottom;
}

void MainUI::_mostVisited_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    MainUI* mainUI = reinterpret_cast<MainUI *>(data);
    mainUI->mostVisitedClicked();
}

void MainUI::_bookmark_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    MainUI* mainUI = reinterpret_cast<MainUI *>(data);
    mainUI->bookmarkClicked();
}

void MainUI::_bookmark_manager_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    MainUI*  mainUI = static_cast<MainUI *>(data);
    mainUI->bookmarkManagerClicked();
}

void MainUI::addHistoryItem(std::shared_ptr<services::HistoryItem> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    M_ASSERT(m_historyItems.size() < MAX_TILES_NUMBER);

    int tileNumber = m_historyItems.size();
    HistoryItemData *itemData = new HistoryItemData();
    itemData->item = hi;
    itemData->mainUI = std::shared_ptr<MainUI>(this);

    Evas_Object* tile = elm_button_add(m_mostVisitedView);
    if (tileNumber == BIG_TILE_INDEX)
        elm_object_style_set(tile, "big_tile");
    else
        elm_object_style_set(tile, "small_tile");
    evas_object_size_hint_weight_set(tile, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (tile, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(tile);
    elm_object_part_content_set(m_mostVisitedView, TILES_NAMES[tileNumber].c_str(), tile);
    m_tiles.push_back(tile);

    elm_layout_text_set(tile, "page_title", hi->getTitle().c_str());
    elm_layout_text_set(tile, "page_url", hi->getUrl().c_str());
    Evas_Object * thumb = tizen_browser::tools::EflTools::getEvasImage(hi->getThumbnail(), m_parent);
    elm_object_part_content_set(tile, "elm.thumbnail", thumb);
    evas_object_smart_callback_add(tile, "clicked", _thumbClicked, itemData);
    elm_object_focus_custom_chain_append(m_parent, tile, NULL);

    m_historyItems.push_back(hi);
}

void MainUI::addHistoryItems(std::shared_ptr<services::HistoryItemVector> items)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    clearHistoryGenlist();
    int i = 0;
    for (auto it = items->begin(); it != items->end(); ++it) {
        i++;
        if (i > MAX_TILES_NUMBER)
            break;
        addHistoryItem(*it);
    }
    if (i>0)
        setEmptyView(false);
}

void MainUI::addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem> bi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = bi;
        itemData->mainUI = std::shared_ptr<tizen_browser::base_ui::MainUI>(this);
        Elm_Object_Item* bookmarkView = elm_gengrid_item_append(m_bookmarkGengrid, m_bookmark_item_class, itemData, nullptr, this);
    m_map_bookmark_views.insert(std::pair<std::string,Elm_Object_Item*>(bi->getAddress(),bookmarkView));

    // unselect by default
    elm_gengrid_item_selected_set(bookmarkView, EINA_FALSE);
}

void MainUI::addBookmarkItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
    clearBookmarkGengrid();
    for (auto it = items.begin(); it != items.end(); ++it) {
         addBookmarkItem(*it);
    }
}


char* MainUI::_grid_bookmark_text_get(void *data, Evas_Object *, const char *part)
{
        BookmarkItemData *itemData = reinterpret_cast<BookmarkItemData*>(data);
        if (!strcmp(part, "page_title")) {
                return strdup(itemData->item->getTittle().c_str());
        }
        if (!strcmp(part, "page_url")) {
                return strdup(itemData->item->getAddress().c_str());
        }
        return strdup("");
}

Evas_Object * MainUI::_grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    BookmarkItemData *itemData = reinterpret_cast<BookmarkItemData*>(data);

    if (!strcmp(part, "elm.thumbnail")) {
        if (itemData->item->getThumbnail()) {
                Evas_Object * thumb = tizen_browser::tools::EflTools::getEvasImage(itemData->item->getThumbnail(), itemData->mainUI->m_parent);
                return thumb;
        }
        else {
                return nullptr;
        }
    }
    else if (!strcmp(part, "elm.thumbButton")) {
                Evas_Object *thumbButton = elm_button_add(obj);
                elm_object_style_set(thumbButton, "thumbButton");
                evas_object_smart_callback_add(thumbButton, "clicked", _thumbBookmarkClicked, data);
                return thumbButton;
    }
    return nullptr;
}

void MainUI::_thumbBookmarkClicked(void * data, Evas_Object * , void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
    itemData->mainUI->openURLInNewTab(itemData->item, itemData->mainUI->isDesktopMode());
}

void MainUI::_thumbClicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
    itemData->mainUI->mostVisitedTileClicked(itemData->item, DetailPopup::HISTORY_ITEMS_NO);
}

void MainUI::clearHistoryGenlist()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    for (auto it = m_tiles.begin(); it != m_tiles.end(); ++it) {
        evas_object_smart_callback_del(*it, "clicked", _thumbClicked);
        evas_object_del(*it);
    }

    m_tiles.clear();
    m_historyItems.clear();
}

void MainUI::showHistory()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    if (elm_layout_content_get(m_layout, "elm.swallow.content") == m_mostVisitedView)
        return;

    elm_layout_content_unset(m_layout, "elm.swallow.content");
    evas_object_hide(m_bookmarksView);
    elm_layout_content_set(m_layout, "elm.swallow.content", m_mostVisitedView);
    evas_object_show(m_mostVisitedView);

    elm_object_focus_set(m_mostVisitedButton, true);

    if (m_historyItems.empty()) {
        setEmptyView(true);
        return;
    }
    setEmptyView(false);
    evas_object_show(m_layout);
}

void MainUI::clearBookmarkGengrid()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    elm_gengrid_clear(m_bookmarkGengrid);
    m_map_bookmark_views.clear();
}

void MainUI::showBookmarks()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    if (elm_layout_content_get(m_layout, "elm.swallow.content") == m_bookmarksView && m_bookmarksView != nullptr)
        return;

    elm_layout_content_unset(m_layout, "elm.swallow.content");
    evas_object_hide(m_mostVisitedView);
    elm_layout_content_set(m_layout, "elm.swallow.content", m_bookmarksView);
    evas_object_show(m_bookmarksView);

    elm_object_focus_set(m_bookmarksButton, true);
    evas_object_show(m_layout);
}

void MainUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_show(m_layout);
    if (elm_layout_content_get(m_layout, "elm.swallow.content") == m_bookmarksView) {
        evas_object_show(m_bookmarksView);
    } else {
        evas_object_show(m_mostVisitedView);
    }
}

void MainUI::hideUI()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    evas_object_hide(m_layout);
    evas_object_hide(m_mostVisitedView);
    evas_object_hide(m_bookmarksView);
    clearHistoryGenlist();
    clearBookmarkGengrid();
}

void MainUI::openDetailPopup(std::shared_ptr<services::HistoryItem> currItem, std::shared_ptr<services::HistoryItemVector> prevItems)
{
    m_detailPopup.show(m_layout, currItem, prevItems);
}

void MainUI::showNoHistoryLabel()
{
    elm_layout_text_set(m_mostVisitedView, "elm.text.empty", "No visited site");
    elm_layout_signal_emit(m_mostVisitedView, "empty,view", "mainui");
}

void MainUI::setEmptyView(bool empty)
{
    BROWSER_LOGD("%s:%d %s, empty: %d", __FILE__, __LINE__, __func__, empty);
    if(empty) {
        showNoHistoryLabel();
    } else {
        elm_layout_signal_emit(m_mostVisitedView, "not,empty,view", "mainui");
    }
}

bool MainUI::isDesktopMode() const
{
    return m_desktopMode;
}

void MainUI::setDesktopMode(bool mode)
{
    m_desktopMode = mode;
}

DetailPopup& MainUI::getDetailPopup()
{
    return m_detailPopup;
}

}
}
