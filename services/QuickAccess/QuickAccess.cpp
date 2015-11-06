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

#include "QuickAccess.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"
#include "Tools/GeneralTools.h"

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(QuickAccess, "org.tizen.browser.quickaccess")

const int QuickAccess::MAX_TILES_NUMBER = 5;
const int QuickAccess::MAX_THUMBNAIL_WIDTH = 840;
const int QuickAccess::MAX_THUMBNAIL_HEIGHT = 648;
const int QuickAccess::BIG_TILE_INDEX = 0;
const int QuickAccess::TOP_RIGHT_TILE_INDEX = 3;
const int QuickAccess::BOTTOM_RIGHT_TILE_INDEX = 4;

const std::vector<std::string> QuickAccess::TILES_NAMES = {
    "elm.swallow.big",
    "elm.swallow.small_first",
    "elm.swallow.small_second",
    "elm.swallow.small_third",
    "elm.swallow.small_fourth"
};

typedef struct _HistoryItemData
{
        std::shared_ptr<tizen_browser::services::HistoryItem> item;
        std::shared_ptr<tizen_browser::base_ui::QuickAccess> quickAccess;
} HistoryItemData;

typedef struct _BookmarkItemData
{
        std::shared_ptr<tizen_browser::services::BookmarkItem> item;
        std::shared_ptr<tizen_browser::base_ui::QuickAccess> quickAccess;
} BookmarkItemData;

QuickAccess::QuickAccess()
    : m_parent(nullptr)
    , m_layout(nullptr)
    , m_bookmarksView(nullptr)
    , m_mostVisitedView(nullptr)
    , m_bookmarksButton(nullptr)
    , m_mostVisitedButton(nullptr)
    , m_bookmarkGengrid(nullptr)
    , m_bookmarkManagerButton(nullptr)
    , m_after_history_thumb(false)
    , m_parentFocusChain(nullptr)
    , m_bookmark_item_class(nullptr)
    , m_detailPopup(this)
#if PROFILE_MOBILE
    , m_scroller(nullptr)
    , m_centerLayout(nullptr)
#endif
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    edjFilePath = EDJE_DIR;
    edjFilePath.append("QuickAccess/QuickAccess.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    QuickAccess::createItemClasses();
}

QuickAccess::~QuickAccess()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    elm_gengrid_item_class_free(m_bookmark_item_class);
    eina_list_free(m_parentFocusChain);
}

void QuickAccess::init(Evas_Object* parent)
{
    M_ASSERT(parent);
    m_parent = parent;
}


Evas_Object* QuickAccess::getContent()
{
    M_ASSERT(m_parent);
    if (!m_layout) {
        m_layout = createQuickAccessLayout(m_parent);
    }
    return m_layout;
}

void QuickAccess::showMostVisited(std::shared_ptr< services::HistoryItemVector > vec)
{
    addHistoryItems(vec);
    showHistory();
}

void QuickAccess::showBookmarks(std::vector< std::shared_ptr< tizen_browser::services::BookmarkItem > > vec)
{
    addBookmarkItems(vec);
    showBookmarks();
}

void QuickAccess::createItemClasses()
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


Evas_Object* QuickAccess::createQuickAccessLayout(Evas_Object* parent)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    m_desktopMode = true;

    Evas_Object* layout = elm_layout_add(parent);
    elm_layout_file_set(layout, edjFilePath.c_str(), "main_layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_mostVisitedView = createMostVisitedView(layout);
    m_bookmarksView   = createBookmarksView  (layout);

    return layout;
}

Evas_Object* QuickAccess::createMostVisitedView(Evas_Object * parent)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    Evas_Object* mostVisitedLayout = elm_layout_add(parent);
    elm_layout_file_set(mostVisitedLayout, edjFilePath.c_str(), "mv_bookmarks");
    evas_object_size_hint_weight_set(mostVisitedLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (mostVisitedLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Evas_Object* topButtons = createTopButtons(mostVisitedLayout);
    elm_object_part_content_set(mostVisitedLayout, "elm.swallow.layoutTop", topButtons);

#if PROFILE_MOBILE
    m_scroller = elm_scroller_add(mostVisitedLayout);
    evas_object_size_hint_weight_set(m_scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (m_scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_scroller_policy_set(m_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_bounce_set(m_scroller, EINA_FALSE, EINA_FALSE);
    elm_object_part_content_set(mostVisitedLayout, "center_swallow", m_scroller);
    evas_object_show(m_scroller);

    m_centerLayout = elm_layout_add(m_scroller);
    elm_layout_file_set(m_centerLayout, edjFilePath.c_str(), "center_layout");
    evas_object_size_hint_weight_set(m_centerLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (m_centerLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_content_set(m_scroller, m_centerLayout);
    evas_object_show(m_centerLayout);
#endif

    return mostVisitedLayout;
}

Evas_Object* QuickAccess::createBookmarksView (Evas_Object * parent)
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

Evas_Object* QuickAccess::createBookmarkGengrid(Evas_Object *parent)
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

Evas_Object* QuickAccess::createTopButtons (Evas_Object *parent)
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

Evas_Object* QuickAccess::createBottomButton(Evas_Object *parent)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    Evas_Object *layoutBottom = elm_layout_add(parent);
    elm_layout_file_set(layoutBottom, edjFilePath.c_str(), "bottom_button_item");

    evas_object_size_hint_weight_set(layoutBottom, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layoutBottom, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(layoutBottom);

    m_bookmarkManagerButton = elm_button_add(layoutBottom);
    elm_object_style_set(m_bookmarkManagerButton, "invisible_button");
    evas_object_smart_callback_add(m_bookmarkManagerButton, "clicked", _bookmark_manager_clicked, this);
    evas_object_show(m_bookmarkManagerButton);

    elm_object_part_content_set(layoutBottom, "bookmarkmanager_click", m_bookmarkManagerButton);

    return layoutBottom;
}


void QuickAccess::_mostVisited_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    QuickAccess* quickAccess = reinterpret_cast<QuickAccess *>(data);
    quickAccess->mostVisitedClicked();
}

void QuickAccess::_bookmark_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    QuickAccess* quickAccess = reinterpret_cast<QuickAccess *>(data);
    quickAccess->bookmarkClicked();
}

void QuickAccess::_bookmark_manager_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    QuickAccess*  quickAccess = static_cast<QuickAccess *>(data);
    quickAccess->bookmarkManagerClicked();
}

void QuickAccess::addHistoryItem(std::shared_ptr<services::HistoryItem> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    M_ASSERT(m_historyItems.size() < MAX_TILES_NUMBER);

    int tileNumber = m_historyItems.size();
    HistoryItemData *itemData = new HistoryItemData();
    itemData->item = hi;
    itemData->quickAccess = std::shared_ptr<QuickAccess>(this);

#if PROFILE_MOBILE
    Evas_Object* tile = elm_button_add(m_centerLayout);
#else
    Evas_Object* tile = elm_button_add(m_mostVisitedView);
#endif
    if (tileNumber == BIG_TILE_INDEX)
        elm_object_style_set(tile, "big_tile");
    else
        elm_object_style_set(tile, "small_tile");
    evas_object_size_hint_weight_set(tile, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (tile, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(tile);
#if PROFILE_MOBILE
    elm_object_part_content_set(m_centerLayout, TILES_NAMES[tileNumber].c_str(), tile);
#else
    elm_object_part_content_set(m_mostVisitedView, TILES_NAMES[tileNumber].c_str(), tile);
#endif
    m_tiles.push_back(tile);

    elm_layout_text_set(tile, "page_title", hi->getTitle().c_str());
    elm_layout_text_set(tile, "page_url", hi->getUrl().c_str());
    Evas_Object * thumb = tizen_browser::tools::EflTools::getEvasImage(hi->getThumbnail(), m_parent);
    elm_object_part_content_set(tile, "elm.thumbnail", thumb);
    evas_object_smart_callback_add(tile, "clicked", _thumbHistoryClicked, itemData);

    m_historyItems.push_back(hi);
}

void QuickAccess::addHistoryItems(std::shared_ptr<services::HistoryItemVector> items)
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

#if PROFILE_MOBILE
    if (items->size() > FIXED_SIZE_TILES_NUMBER) {
        elm_object_signal_emit(m_centerLayout, "set_fixed_size", "ui");
    } else {
        elm_object_signal_emit(m_centerLayout, "set_dynamic_size", "ui");
    }
#endif
}

void QuickAccess::addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem> bi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = bi;
    itemData->quickAccess = std::shared_ptr<tizen_browser::base_ui::QuickAccess>(this);
    elm_gengrid_item_append(m_bookmarkGengrid, m_bookmark_item_class, itemData, _thumbBookmarkClicked, itemData);
}

void QuickAccess::addBookmarkItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
    clearBookmarkGengrid();
    for (auto it = items.begin(); it != items.end(); ++it) {
         addBookmarkItem(*it);
    }
}


char* QuickAccess::_grid_bookmark_text_get(void *data, Evas_Object *, const char *part)
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

Evas_Object * QuickAccess::_grid_bookmark_content_get(void *data, Evas_Object*, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    BookmarkItemData *itemData = reinterpret_cast<BookmarkItemData*>(data);

    if (!strcmp(part, "elm.thumbnail")) {
        if (itemData->item->getThumbnail()) {
                Evas_Object * thumb = tizen_browser::tools::EflTools::getEvasImage(itemData->item->getThumbnail(), itemData->quickAccess->m_parent);
                return thumb;
        }
        else {
                return nullptr;
        }
    }

    return nullptr;
}

void QuickAccess::_thumbBookmarkClicked(void * data, Evas_Object * , void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
    itemData->quickAccess->openURLInNewTab(itemData->item, itemData->quickAccess->isDesktopMode());
    itemData->quickAccess->m_after_history_thumb = false;
}

void QuickAccess::_thumbHistoryClicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
    itemData->quickAccess->mostVisitedTileClicked(itemData->item, DetailPopup::HISTORY_ITEMS_NO);
    itemData->quickAccess->m_after_history_thumb = true;
}

void QuickAccess::clearHistoryGenlist()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    for (auto it = m_tiles.begin(); it != m_tiles.end(); ++it) {
        evas_object_smart_callback_del(*it, "clicked", _thumbHistoryClicked);
        evas_object_del(*it);
    }

    m_tiles.clear();
    m_historyItems.clear();
}

void QuickAccess::showHistory()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    if (elm_layout_content_get(m_layout, "view") == m_mostVisitedView)
        return;

    evas_object_hide(m_bookmarksView);
    elm_layout_content_unset(m_layout, "view");
    elm_layout_content_set(m_layout, "view", m_mostVisitedView);
    evas_object_show(m_mostVisitedView);


    if (m_historyItems.empty()) {
        setEmptyView(true);
        return;
    }
    setEmptyView(false);
    evas_object_show(m_layout);
    refreshFocusChain();
    elm_object_focus_set(m_mostVisitedButton, true);
}

void QuickAccess::clearBookmarkGengrid()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    elm_gengrid_clear(m_bookmarkGengrid);
}

void QuickAccess::showBookmarks()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    if (elm_layout_content_get(m_layout, "view") == m_bookmarksView && m_bookmarksView != nullptr)
        return;

    evas_object_hide(m_mostVisitedView);
    elm_layout_content_unset(m_layout, "view");
    elm_layout_content_set(m_layout, "view", m_bookmarksView);
    evas_object_show(m_bookmarksView);

    evas_object_show(m_layout);
    refreshFocusChain();
    elm_object_focus_set(m_bookmarksButton, true);
}

void QuickAccess::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_show(m_layout);
    if (elm_layout_content_get(m_layout, "view") == m_bookmarksView) {
        evas_object_show(m_bookmarksView);
    } else {
        evas_object_show(m_mostVisitedView);
    }
}

void QuickAccess::hideUI()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    evas_object_hide(m_layout);
    evas_object_hide(m_mostVisitedView);
    evas_object_hide(m_bookmarksView);
    clearHistoryGenlist();
    clearBookmarkGengrid();
}

void QuickAccess::openDetailPopup(std::shared_ptr<services::HistoryItem> currItem, std::shared_ptr<services::HistoryItemVector> prevItems)
{
    m_detailPopup.show(m_layout, m_parent, currItem, prevItems);
}

void QuickAccess::showNoHistoryLabel()
{
    elm_layout_text_set(m_mostVisitedView, "elm.text.empty", "No visited site");
    elm_layout_signal_emit(m_mostVisitedView, "empty,view", "quickaccess");
}

void QuickAccess::setEmptyView(bool empty)
{
    BROWSER_LOGD("%s:%d %s, empty: %d", __FILE__, __LINE__, __func__, empty);
    if(empty) {
        showNoHistoryLabel();
    } else {
        elm_layout_signal_emit(m_mostVisitedView, "not,empty,view", "quickaccess");
    }
}

bool QuickAccess::isDesktopMode() const
{
    return m_desktopMode;
}

void QuickAccess::setDesktopMode(bool mode)
{
    m_desktopMode = mode;
}

DetailPopup& QuickAccess::getDetailPopup()
{
    return m_detailPopup;
}

void QuickAccess::backButtonClicked()
{
    if (m_detailPopup.isVisible()) {
        m_detailPopup.hide();
    } else {
        hideUI();
        switchViewToWebPage();
    }
}

bool QuickAccess::isMostVisitedActive() const
{
    return evas_object_visible_get(m_mostVisitedView);
}

void QuickAccess::refreshFocusChain()
{
    if (!isMostVisitedActive() && m_after_history_thumb) {
        m_after_history_thumb = false;
        return;
    }
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!m_parentFocusChain) {
        m_parentFocusChain = eina_list_clone(elm_object_focus_custom_chain_get(m_parent));
    } else {
        elm_object_focus_custom_chain_unset(m_parent);
        elm_object_focus_custom_chain_set(m_parent, eina_list_clone(m_parentFocusChain));
    }

    elm_object_focus_custom_chain_append(m_parent, m_mostVisitedButton, NULL);
    elm_object_focus_custom_chain_append(m_parent, m_bookmarksButton, NULL);
    if (isMostVisitedActive()) {
        for (auto tile = m_tiles.begin(); tile != m_tiles.end(); ++tile) {
                elm_object_focus_custom_chain_append(m_parent, *tile, NULL);
        }
        // prevent from moving outside of screen
        elm_object_focus_next_object_set(m_tiles[BIG_TILE_INDEX], m_tiles[BIG_TILE_INDEX], ELM_FOCUS_LEFT);
        elm_object_focus_next_object_set(m_tiles[TOP_RIGHT_TILE_INDEX], m_tiles[TOP_RIGHT_TILE_INDEX], ELM_FOCUS_RIGHT);
        elm_object_focus_next_object_set(m_tiles[BOTTOM_RIGHT_TILE_INDEX], m_tiles[BOTTOM_RIGHT_TILE_INDEX], ELM_FOCUS_RIGHT);
    } else {
        elm_object_focus_custom_chain_append(m_parent, m_bookmarkGengrid, NULL);
        elm_object_focus_custom_chain_append(m_parent, m_bookmarkManagerButton, NULL);
        // prevent from moving outside of screen
        elm_object_focus_next_object_set(m_bookmarkGengrid, m_bookmarkGengrid, ELM_FOCUS_LEFT);
        elm_object_focus_next_object_set(m_bookmarkGengrid, m_bookmarkGengrid, ELM_FOCUS_RIGHT);
    }
}

}
}
