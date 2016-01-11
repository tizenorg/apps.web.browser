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

#include "app_i18n.h"
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
    , m_horizontalScroller(nullptr)
    , m_box(nullptr)
    , m_bookmarksView(nullptr)
    , m_mostVisitedView(nullptr)
    , m_bookmarksButton(nullptr)
    , m_mostVisitedButton(nullptr)
    , m_bookmarkGengrid(nullptr)
    , m_bookmarkManagerButton(nullptr)
    , m_after_history_thumb(false)
    , m_parentFocusChain(nullptr)
    , m_currPage(-1)    // init with no page selected
    , m_bookmark_item_class(nullptr)
    , m_detailPopup(this)
#if PROFILE_MOBILE
    , m_index(nullptr)
    , m_verticalScroller(nullptr)
    , m_centerLayout(nullptr)
    , m_bookmarkManagerTileclass(nullptr)
#endif
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    edjFilePath = EDJE_DIR;
    edjFilePath.append("QuickAccess/QuickAccess.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    QuickAccess::createItemClasses();
}

QuickAccess::~QuickAccess()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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

void QuickAccess::createItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_bookmark_item_class) {
        m_bookmark_item_class = elm_gengrid_item_class_new();
        m_bookmark_item_class->item_style = "grid_item";
        m_bookmark_item_class->func.text_get = _grid_bookmark_text_get;
        m_bookmark_item_class->func.content_get =  _grid_bookmark_content_get;
        m_bookmark_item_class->func.state_get = nullptr;
        m_bookmark_item_class->func.del = nullptr;
    }
#if PROFILE_MOBILE
    if (!m_bookmarkManagerTileclass) {
        m_bookmarkManagerTileclass = elm_gengrid_item_class_new();
        m_bookmarkManagerTileclass->item_style = "bookmark_manager";
        m_bookmarkManagerTileclass->func.text_get = nullptr;
        m_bookmarkManagerTileclass->func.content_get = nullptr;
        m_bookmarkManagerTileclass->func.state_get = nullptr;
        m_bookmarkManagerTileclass->func.del = nullptr;
    }
#endif
}


Evas_Object* QuickAccess::createQuickAccessLayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    m_desktopMode = false;
#else
    m_desktopMode = true;
#endif

    Evas_Object* layout = elm_layout_add(parent);
    elm_layout_file_set(layout, edjFilePath.c_str(), "main_layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(layout);

#if !PROFILE_MOBILE
    Evas_Object* topButtons = createTopButtons(layout);
    elm_object_part_content_set(layout, "buttons", topButtons);
    elm_object_tree_focus_allow_set(topButtons, EINA_TRUE);

    Evas_Object* bottomButton = createBottomButton(layout);
    elm_object_part_content_set(layout, "bottom_layout", bottomButton);
    elm_object_tree_focus_allow_set(bottomButton, EINA_TRUE);

#else
    m_index = elm_index_add(layout);
    evas_object_size_hint_weight_set(m_index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_index, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_style_set(m_index, "browser_pagecontrol");
    elm_index_horizontal_set(m_index, EINA_TRUE);
    elm_index_autohide_disabled_set(m_index, EINA_TRUE);
    elm_object_part_content_set(layout, "buttons", m_index);

    elm_index_item_append(m_index, "1", NULL, (void *) QuickAccess::MOST_VISITED_PAGE);
    elm_index_item_append(m_index, "2", NULL, (void *) QuickAccess::BOOKMARK_PAGE);
    elm_index_level_go(m_index, 0);
#endif

    m_horizontalScroller = elm_scroller_add(layout);
    elm_scroller_loop_set(m_horizontalScroller, EINA_FALSE, EINA_FALSE);
    evas_object_size_hint_weight_set(m_horizontalScroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_horizontalScroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_scroller_page_relative_set(m_horizontalScroller, 1.0, 0.0);
    elm_scroller_policy_set(m_horizontalScroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_scroll_limit_set(m_horizontalScroller, 1, 0);
    elm_scroller_bounce_set(m_horizontalScroller, EINA_TRUE, EINA_FALSE);
    elm_object_scroll_lock_y_set(m_horizontalScroller, EINA_TRUE);
    elm_object_part_content_set(layout, "view", m_horizontalScroller);
    evas_object_smart_callback_add(m_horizontalScroller, "scroll", _horizontalScroller_scroll, this);

    m_box = elm_box_add(m_horizontalScroller);
    elm_box_horizontal_set(m_box, EINA_TRUE);
    elm_object_content_set(m_horizontalScroller, m_box);
    evas_object_show(m_box);

    m_mostVisitedView = createMostVisitedView(m_box);
    elm_box_pack_end(m_box, m_mostVisitedView);
    m_bookmarksView   = createBookmarksView  (m_box);
    elm_box_pack_end(m_box, m_bookmarksView);

    return layout;
}

Evas_Object* QuickAccess::createMostVisitedView(Evas_Object * parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* mostVisitedLayout = elm_layout_add(parent);
    elm_layout_file_set(mostVisitedLayout, edjFilePath.c_str(), "page_layout");
    evas_object_size_hint_weight_set(mostVisitedLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (mostVisitedLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(mostVisitedLayout);

#if PROFILE_MOBILE
    m_verticalScroller = elm_scroller_add(mostVisitedLayout);
    evas_object_size_hint_weight_set(m_verticalScroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (m_verticalScroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_scroller_policy_set(m_verticalScroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_bounce_set(m_verticalScroller, EINA_FALSE, EINA_FALSE);
    elm_object_part_content_set(mostVisitedLayout, "center_swallow", m_verticalScroller);
    evas_object_show(m_verticalScroller);

    m_centerLayout = elm_layout_add(m_verticalScroller);
    elm_layout_file_set(m_centerLayout, edjFilePath.c_str(), "center_layout");
    evas_object_size_hint_weight_set(m_centerLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (m_centerLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_content_set(m_verticalScroller, m_centerLayout);
    evas_object_show(m_centerLayout);
#endif

    return mostVisitedLayout;
}

Evas_Object* QuickAccess::createBookmarksView (Evas_Object * parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *bookmarkViewLayout = elm_layout_add(parent);
    elm_layout_file_set(bookmarkViewLayout, edjFilePath.c_str(), "page_layout");
    evas_object_size_hint_weight_set(bookmarkViewLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bookmarkViewLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(bookmarkViewLayout);

    m_bookmarkGengrid = createBookmarkGengrid(bookmarkViewLayout);
    elm_object_part_content_set(bookmarkViewLayout, "elm.swallow.grid", m_bookmarkGengrid);
    evas_object_show(m_bookmarkGengrid);

    return bookmarkViewLayout;
}

Evas_Object* QuickAccess::createBookmarkGengrid(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object *bookmarkGengrid = elm_gengrid_add(parent);

    elm_gengrid_select_mode_set(bookmarkGengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(bookmarkGengrid, EINA_FALSE);
    elm_scroller_policy_set(bookmarkGengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    evas_object_size_hint_weight_set(bookmarkGengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bookmarkGengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

#if !PROFILE_MOBILE
    elm_scroller_page_size_set(bookmarkGengrid, 0, 327);
    elm_gengrid_item_size_set(bookmarkGengrid, 364 * efl_scale, 320 * efl_scale);
    elm_gengrid_align_set(bookmarkGengrid, 0.5, 0.5);
#else
    elm_scroller_page_size_set(bookmarkGengrid, 0, 1100);
    elm_gengrid_align_set(bookmarkGengrid, 0.5, 0.0);
    elm_gengrid_item_size_set(bookmarkGengrid, 337 * efl_scale, 379 * efl_scale);
#endif
    elm_scroller_bounce_set(bookmarkGengrid, EINA_FALSE, EINA_FALSE);

    return bookmarkGengrid;
}

#if !PROFILE_MOBILE
Evas_Object* QuickAccess::createTopButtons (Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *layoutTop = elm_layout_add(parent);
    elm_layout_file_set(layoutTop, edjFilePath.c_str(), "category_menu");
    evas_object_size_hint_weight_set(layoutTop, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layoutTop, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(layoutTop);

    Evas_Object *mostVisitedButton = elm_button_add(layoutTop);
    elm_object_style_set(mostVisitedButton, "category_button");
    elm_object_translatable_part_text_set(mostVisitedButton, "btn_txt", "IDS_BR_TAB_MOST_VISITED");
    evas_object_smart_callback_add(mostVisitedButton, "clicked", _mostVisited_clicked, this);
    evas_object_smart_callback_add(mostVisitedButton, "mouse,in", _category_btn_mouse_in, nullptr);
    evas_object_smart_callback_add(mostVisitedButton, "mouse,out", _category_btn_mouse_out, this);
    evas_object_smart_callback_add(mostVisitedButton, "focused", _category_btn_mouse_in, nullptr);
    evas_object_smart_callback_add(mostVisitedButton, "unfocused", _category_btn_mouse_out, this);
    evas_object_show(mostVisitedButton);
    elm_layout_content_set(layoutTop, "most_visited_btn", mostVisitedButton);
    m_mostVisitedButton = mostVisitedButton;

    Evas_Object *bookmarksButton = elm_button_add(layoutTop);
    elm_object_style_set(bookmarksButton, "category_button");
    elm_object_translatable_part_text_set(bookmarksButton, "btn_txt", "IDS_BR_OPT_BOOKMARK");
    evas_object_smart_callback_add(bookmarksButton, "clicked", _bookmark_clicked, this);
    evas_object_smart_callback_add(bookmarksButton, "mouse,in", _category_btn_mouse_in, nullptr);
    evas_object_smart_callback_add(bookmarksButton, "mouse,out", _category_btn_mouse_out, this);
    evas_object_smart_callback_add(bookmarksButton, "focused", _category_btn_mouse_in, nullptr);
    evas_object_smart_callback_add(bookmarksButton, "unfocused", _category_btn_mouse_out, this);
    evas_object_show(bookmarksButton);
    elm_layout_content_set(layoutTop, "bookmark_btn", bookmarksButton);
    m_bookmarksButton = bookmarksButton;

    return layoutTop;
}

Evas_Object* QuickAccess::createBottomButton(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *layoutBottom = elm_layout_add(parent);
    elm_layout_file_set(layoutBottom, edjFilePath.c_str(), "bottom_button");

    evas_object_size_hint_weight_set(layoutBottom, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layoutBottom, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(layoutBottom);

    m_bookmarkManagerButton = elm_button_add(layoutBottom);
    elm_object_style_set(m_bookmarkManagerButton, "browser_text_button");
    // TODO: add translation for Bookmark manager
    elm_object_translatable_part_text_set(m_bookmarkManagerButton, "btn_txt", "Bookmark manager");
    evas_object_event_callback_add(m_bookmarkManagerButton, EVAS_CALLBACK_SHOW, _bookmark_btn_show, (void*) &m_currPage);
    evas_object_smart_callback_add(m_bookmarkManagerButton, "mouse,in", _category_btn_mouse_in, nullptr);
    evas_object_smart_callback_add(m_bookmarkManagerButton, "mouse,out", _category_btn_mouse_out, this);
    evas_object_smart_callback_add(m_bookmarkManagerButton, "focused", _category_btn_mouse_in, nullptr);
    evas_object_smart_callback_add(m_bookmarkManagerButton, "unfocused", _category_btn_mouse_out, this);
    evas_object_smart_callback_add(m_bookmarkManagerButton, "clicked", _bookmark_manager_clicked, this);

    elm_object_part_content_set(layoutBottom, "bookmark_manager_btn", m_bookmarkManagerButton);
    evas_object_hide(m_bookmarkManagerButton);

    return layoutBottom;
}
#endif

void QuickAccess::_mostVisited_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = reinterpret_cast<QuickAccess *>(data);
    self->showMostVisited();
    elm_scroller_page_show(self->m_horizontalScroller, MOST_VISITED_PAGE, 0);
}

void QuickAccess::_bookmark_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = reinterpret_cast<QuickAccess *>(data);
    self->showBookmarks();
    elm_scroller_page_show(self->m_horizontalScroller, BOOKMARK_PAGE, 0);
}

void QuickAccess::_bookmark_manager_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    QuickAccess*  quickAccess = static_cast<QuickAccess *>(data);
    quickAccess->bookmarkManagerClicked();
}

void QuickAccess::_horizontalScroller_scroll(void* data, Evas_Object* /*scroller*/, void* /*event_info*/)
{
    auto self = static_cast<QuickAccess*>(data);
    int page_no;

    elm_scroller_current_page_get(self->m_horizontalScroller, &page_no, NULL);
    if (self->m_currPage != page_no) {
        switch (page_no) {
            case MOST_VISITED_PAGE:
                self->showMostVisited();
                break;
            case BOOKMARK_PAGE:
                self->showBookmarks();
                break;
            default:
                BROWSER_LOGD("[%s:%d] Warning: unknown page value!", __PRETTY_FUNCTION__, __LINE__);
        }
    }
}

#if !PROFILE_MOBILE
void QuickAccess::_category_btn_mouse_in(void* /*data*/, Evas_Object* obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(obj, "btn,focused", "border");
    elm_object_focus_set(obj, EINA_TRUE);
}

void QuickAccess::_category_btn_mouse_out(void* data, Evas_Object* obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<QuickAccess*>(data);
    if (((obj == self->m_mostVisitedButton) && (self->m_currPage == MOST_VISITED_PAGE)) ||
            ((obj == self->m_bookmarksButton) && (self->m_currPage == BOOKMARK_PAGE))   ){
        elm_object_signal_emit(obj, "btn,selected", "border");
    } else {
        elm_object_signal_emit(obj, "btn,normal", "border");
    }
    elm_object_focus_set(obj, EINA_FALSE);
}


/**
 * This function is need only in one case. When browser is starting and first page is QuickAccess.
 * Then to avoid showing BookmarkManagerButton we hide it.
 * When there will be no signal to show this button in app_resume it could be removed.
 */
void QuickAccess::_bookmark_btn_show(void* data, Evas* /*e*/, Evas_Object* obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    int page_no = *(int*)data;
    if (page_no == MOST_VISITED_PAGE) {
        evas_object_hide(obj);
    }
}
#endif

void QuickAccess::addMostVisitedItem(std::shared_ptr<services::HistoryItem> hi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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
    evas_object_smart_callback_add(tile, "clicked", _thumbMostVisitedClicked, itemData);

    m_historyItems.push_back(hi);
}

void QuickAccess::setMostVisitedItems(std::shared_ptr<services::HistoryItemVector> items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    clearMostVisitedGenlist();
    int i = 0;
    for (auto it = items->begin(); it != items->end(); ++it) {
        i++;
        if (i > MAX_TILES_NUMBER)
            break;
        addMostVisitedItem(*it);
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
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = bi;
    itemData->quickAccess = std::shared_ptr<tizen_browser::base_ui::QuickAccess>(this);
    elm_gengrid_item_append(m_bookmarkGengrid, m_bookmark_item_class, itemData, _thumbBookmarkClicked, itemData);
}

void QuickAccess::setBookmarksItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
    clearBookmarkGengrid();
#if PROFILE_MOBILE
    addBookmarkManagerTile();
#endif
    for (auto it = items.begin(); it != items.end(); ++it) {
         addBookmarkItem(*it);
    }
}

#if PROFILE_MOBILE
void QuickAccess::addBookmarkManagerTile()
{
    elm_gengrid_item_append(m_bookmarkGengrid, m_bookmarkManagerTileclass, this, _bookmark_manager_clicked, this);
}

void QuickAccess::setIndexPage(const uintptr_t page) const
{
    Elm_Object_Item* it = elm_index_item_find(m_index, (void *)page);
    if (it != NULL) {
        elm_index_item_selected_set(it, EINA_TRUE);
    }
}
#endif

char* QuickAccess::_grid_bookmark_text_get(void *data, Evas_Object *, const char *part)
{
        BookmarkItemData *itemData = reinterpret_cast<BookmarkItemData*>(data);
        if (!strcmp(part, "page_title")) {
                return strdup(itemData->item->getTitle().c_str());
        }
        if (!strcmp(part, "page_url")) {
                return strdup(itemData->item->getAddress().c_str());
        }
        return strdup("");
}

Evas_Object * QuickAccess::_grid_bookmark_content_get(void *data, Evas_Object*, const char *part)
{
    BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__, part);
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
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
    itemData->quickAccess->openURLInNewTab(itemData->item, itemData->quickAccess->isDesktopMode());
    itemData->quickAccess->m_after_history_thumb = false;
}

void QuickAccess::_thumbMostVisitedClicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
#if PROFILE_MOBILE
    itemData->quickAccess->openURLInNewTab(itemData->item, false);
#else
    itemData->quickAccess->mostVisitedTileClicked(itemData->item, DetailPopup::HISTORY_ITEMS_NO);
    itemData->quickAccess->m_after_history_thumb = true;
#endif
}

void QuickAccess::clearMostVisitedGenlist()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    for (auto it = m_tiles.begin(); it != m_tiles.end(); ++it) {
        evas_object_smart_callback_del(*it, "clicked", _thumbMostVisitedClicked);
        evas_object_del(*it);
    }

    m_tiles.clear();
    m_historyItems.clear();
}

void QuickAccess::showMostVisited()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_currPage = QuickAccess::MOST_VISITED_PAGE;
#if PROFILE_MOBILE
    elm_object_part_text_set(m_layout, "screen_title", "Most Visited");
    setIndexPage(QuickAccess::MOST_VISITED_PAGE);
#else
    evas_object_hide(m_bookmarkManagerButton);
    refreshFocusChain();
    // Setting focus is needed for the first time we open QuickAccess
    elm_object_focus_set(m_mostVisitedButton, EINA_TRUE);
    // Unfocused signal to previous category button is send before m_currPage is change
    // we must change state of other buttons manually
    elm_object_signal_emit(m_bookmarksButton, "btn,normal", "border");
    // Category buttons overlay each other so we need to raise active one.
    evas_object_raise(m_mostVisitedButton);
#endif
    setEmptyView(m_historyItems.empty());
}

void QuickAccess::clearBookmarkGengrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_gengrid_clear(m_bookmarkGengrid);
}

void QuickAccess::showBookmarks()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_currPage = QuickAccess::BOOKMARK_PAGE;
#if PROFILE_MOBILE
    elm_object_translatable_part_text_set(m_layout, "screen_title", "IDS_BR_OPT_BOOKMARK");
    setIndexPage(QuickAccess::BOOKMARK_PAGE);
#else
    evas_object_show(m_bookmarkManagerButton);
    refreshFocusChain();
    elm_object_focus_set(m_bookmarksButton, EINA_TRUE);
    elm_object_signal_emit(m_mostVisitedButton, "btn,normal", "border");
    evas_object_raise(m_bookmarksButton);
#endif
}

void QuickAccess::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_show(m_layout);
    getMostVisitedItems();
    getBookmarksItems();
    _horizontalScroller_scroll(this, NULL, NULL);
}

void QuickAccess::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(m_layout);
    clearMostVisitedGenlist();
    clearBookmarkGengrid();
    m_currPage = -1; // Reset current page no.
}

void QuickAccess::openDetailPopup(std::shared_ptr<services::HistoryItem> currItem, std::shared_ptr<services::HistoryItemVector> prevItems)
{
    m_detailPopup.show(m_layout, m_parent, currItem, prevItems);
}

void QuickAccess::showNoMostVisitedLabel()
{
    elm_layout_text_set(m_mostVisitedView, "elm.text.empty", "No visited site");
    elm_layout_signal_emit(m_mostVisitedView, "empty,view", "quickaccess");
}

void QuickAccess::setEmptyView(bool empty)
{
    BROWSER_LOGD("[%s:%d], empty: %d", __PRETTY_FUNCTION__, __LINE__, empty);
    if(empty) {
        showNoMostVisitedLabel();
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

bool QuickAccess::canBeBacked(int tabCount)
{
    if (m_detailPopup.isVisible())
        return true;
    else
        return (tabCount != 0);
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
    return m_currPage == MOST_VISITED_PAGE;
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
    elm_object_focus_next_object_set(m_mostVisitedButton, m_bookmarksButton, ELM_FOCUS_RIGHT);
    elm_object_focus_next_object_set(m_bookmarksButton, m_mostVisitedButton, ELM_FOCUS_LEFT);
    if (isMostVisitedActive()) {
        if (!m_tiles.empty()) {
            for (auto tile = m_tiles.begin(); tile != m_tiles.end(); ++tile) {
                    elm_object_focus_custom_chain_append(m_parent, *tile, NULL);
            }
            // prevent from moving outside of screen
            elm_object_focus_next_object_set(m_tiles[BIG_TILE_INDEX], m_tiles[BIG_TILE_INDEX], ELM_FOCUS_LEFT);
            elm_object_focus_next_object_set(m_tiles[TOP_RIGHT_TILE_INDEX], m_tiles[TOP_RIGHT_TILE_INDEX], ELM_FOCUS_RIGHT);
            elm_object_focus_next_object_set(m_tiles[BOTTOM_RIGHT_TILE_INDEX], m_tiles[BOTTOM_RIGHT_TILE_INDEX], ELM_FOCUS_RIGHT);
            // set focus chain for category buttons
            elm_object_focus_next_object_set(m_mostVisitedButton, m_tiles[BIG_TILE_INDEX], ELM_FOCUS_DOWN);
            elm_object_focus_next_object_set(m_bookmarksButton, m_tiles[BIG_TILE_INDEX], ELM_FOCUS_DOWN);
        }
    } else {
        if (elm_gengrid_items_count(m_bookmarkGengrid)) {
            elm_object_focus_custom_chain_append(m_parent, m_bookmarkGengrid, NULL);
            // prevent from moving outside of screen
            elm_object_focus_next_object_set(m_bookmarkGengrid, m_bookmarkGengrid, ELM_FOCUS_LEFT);
            elm_object_focus_next_object_set(m_bookmarkGengrid, m_bookmarkGengrid, ELM_FOCUS_RIGHT);
            // set focus chain for category buttons
            elm_object_focus_next_object_set(m_mostVisitedButton, m_bookmarkGengrid, ELM_FOCUS_DOWN);
            elm_object_focus_next_object_set(m_bookmarksButton, m_bookmarkGengrid, ELM_FOCUS_DOWN);
            elm_object_focus_next_object_set(m_bookmarkGengrid, m_mostVisitedButton, ELM_FOCUS_UP);
        }
        elm_object_focus_custom_chain_append(m_parent, m_bookmarkManagerButton, NULL);
        if (!elm_gengrid_items_count(m_bookmarkGengrid)) {
            elm_object_focus_next_object_set(m_mostVisitedButton, m_bookmarkManagerButton, ELM_FOCUS_DOWN);
            elm_object_focus_next_object_set(m_bookmarksButton, m_bookmarkManagerButton, ELM_FOCUS_DOWN);
            elm_object_focus_next_object_set(m_bookmarkManagerButton, m_mostVisitedButton, ELM_FOCUS_UP);
        } else {
            elm_object_focus_next_object_set(m_bookmarkGengrid, m_bookmarkManagerButton, ELM_FOCUS_DOWN);
            elm_object_focus_next_object_set(m_bookmarkManagerButton, m_bookmarkGengrid, ELM_FOCUS_UP);
        }
    }
}

}
}
