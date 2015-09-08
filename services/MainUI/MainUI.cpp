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

const int SMALL_TILES_ROWS = 2;
const int MAX_TILES_NUMBER = 5;
const int MainUI::MAX_TILE_WIDTH = 784;
const int MainUI::MAX_TILE_HEIGHT = 498;

EXPORT_SERVICE(MainUI, "org.tizen.browser.mainui")

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

struct ItemData{
        tizen_browser::base_ui::MainUI * mainUI;
        const char* button_name;
        Elm_Object_Item * e_item;
};

MainUI::MainUI()
    : m_parent(nullptr)
    , m_layout(nullptr)
    , m_bookmarksButton(nullptr)
    , m_mostVisitedButton(nullptr)
    , m_bookmarksView(nullptr)
    , m_mostVisitedView(nullptr)
    , m_bookmarkGengrid(nullptr)
    , m_genListLeft(nullptr)
    , m_genListCenter(nullptr)
    , m_genListRight(nullptr)
    , m_big_item_class(nullptr)
    , m_small_item_class(nullptr)
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
    elm_genlist_item_class_free(m_big_item_class);
    elm_genlist_item_class_free(m_small_item_class);
    elm_gengrid_item_class_free(m_bookmark_item_class);
}

void MainUI::createItemClasses()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (!m_big_item_class) {
       m_big_item_class = elm_genlist_item_class_new();
       m_big_item_class->item_style = "big_grid_item";
       m_big_item_class->func.text_get = _grid_text_get;
       m_big_item_class->func.content_get =  _grid_content_get;
       m_big_item_class->func.state_get = nullptr;
       m_big_item_class->func.del = nullptr;
    }

    if (!m_small_item_class) {
        m_small_item_class = elm_genlist_item_class_new();
        m_small_item_class->item_style = "small_grid_item";
        m_small_item_class->func.text_get = _grid_text_get;
        m_small_item_class->func.content_get =  _grid_content_get;
        m_small_item_class->func.state_get = nullptr;
        m_small_item_class->func.del = nullptr;
    }

    if (!m_bookmark_item_class) {
        m_bookmark_item_class = elm_gengrid_item_class_new();
        m_bookmark_item_class->item_style = "grid_item";
        m_bookmark_item_class->func.text_get = _grid_bookmark_text_get;
        m_bookmark_item_class->func.content_get =  _grid_bookmark_content_get;
        m_bookmark_item_class->func.state_get = nullptr;
        m_bookmark_item_class->func.del = nullptr;
    }
}

void MainUI::show(Evas_Object* parent)
{
    //FIXME: this may be source of memory leak this object is not deleted anywhere
    m_layout = createQuickAccessLayout(parent);
    evas_object_show(m_layout);

    m_parent = parent;
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

    evas_object_show(m_mostVisitedView);
    //TODO: uncoment this after cleaning up the mess in whole app window.
    //evas_object_show(m_bookmarksView);
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

    Evas_Object* genListLeft = elm_genlist_add(mostVisitedLayout);

    elm_genlist_homogeneous_set(genListLeft, EINA_FALSE);
    elm_genlist_multi_select_set(genListLeft, EINA_FALSE);
    elm_genlist_select_mode_set(genListLeft, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(genListLeft, ELM_LIST_LIMIT);
    evas_object_size_hint_weight_set(genListLeft, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_object_part_content_set(mostVisitedLayout, "elm.swallow.left", genListLeft);
    m_genListLeft = genListLeft;

    Evas_Object* genListCenter = elm_genlist_add(mostVisitedLayout);
    elm_genlist_homogeneous_set(genListCenter, EINA_FALSE);
    elm_genlist_multi_select_set(genListCenter, EINA_FALSE);
    elm_genlist_select_mode_set(genListCenter, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(genListCenter, ELM_LIST_LIMIT);
    evas_object_size_hint_weight_set(genListCenter, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_object_part_content_set(mostVisitedLayout, "elm.swallow.center", genListCenter);
    m_genListCenter = genListCenter;

    Evas_Object* genListRight = elm_genlist_add(mostVisitedLayout);
    elm_genlist_homogeneous_set(genListRight, EINA_FALSE);
    elm_genlist_multi_select_set(genListRight, EINA_FALSE);
    elm_genlist_select_mode_set(genListRight, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(genListRight, ELM_LIST_LIMIT);
    evas_object_size_hint_weight_set(genListRight, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_object_part_content_set(mostVisitedLayout, "elm.swallow.right", genListRight);
    m_genListRight=genListRight;

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

    Evas_Object *bookmarkGengrid = elm_gengrid_add(bookmarkViewLayout);

    elm_gengrid_align_set(bookmarkGengrid, 0, 0);
    elm_gengrid_select_mode_set(bookmarkGengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(bookmarkGengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(bookmarkGengrid, EINA_FALSE);

    elm_scroller_policy_set(bookmarkGengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(bookmarkGengrid, 0, 327);
    evas_object_size_hint_weight_set(bookmarkGengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bookmarkGengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_item_size_set(bookmarkGengrid, 364 * efl_scale, 320 * efl_scale);
    evas_object_show(bookmarkGengrid);

    elm_object_part_content_set(bookmarkViewLayout, "elm.swallow.grid", bookmarkGengrid);
    m_bookmarkGengrid = bookmarkGengrid;

    Evas_Object* topButtons = createTopButtons(bookmarkViewLayout);
    elm_object_part_content_set(bookmarkViewLayout, "elm.swallow.layoutTop", topButtons);

    Evas_Object* bottomButton = createBottomButton(bookmarkViewLayout);
    elm_object_part_content_set(bookmarkViewLayout, "elm.swallow.layoutBottom", bottomButton);

    return bookmarkViewLayout;
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

void MainUI::_mostVisited_clicked(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    MainUI* mainUI = reinterpret_cast<MainUI *>(data);
    mainUI->mostVisitedClicked(std::string());
}

void MainUI::_bookmark_clicked(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    MainUI* mainUI = reinterpret_cast<MainUI *>(data);
    mainUI->bookmarkClicked(std::string());
}

void MainUI::_bookmark_manager_clicked(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    MainUI*  mainUI = static_cast<MainUI *>(data);
    mainUI->bookmarkManagerClicked(std::string());
}

void MainUI::addHistoryItem(std::shared_ptr<services::HistoryItem> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (m_map_history_views.size() >= MAX_TILES_NUMBER)
        return;

    HistoryItemData *itemData = new HistoryItemData();
    itemData->item = hi;
    itemData->mainUI = std::shared_ptr<MainUI>(this);
    Elm_Object_Item* historyView = nullptr;

    if (m_map_history_views.empty())
    {
        BROWSER_LOGD("%s:%d %s  m_map_history_views.size %d", __FILE__, __LINE__, __func__, m_map_history_views.size());
        historyView = elm_genlist_item_append(m_genListLeft, m_big_item_class, itemData, nullptr, ELM_GENLIST_ITEM_NONE, nullptr, this);
    } else if (m_map_history_views.size() <= SMALL_TILES_ROWS) {
        BROWSER_LOGD("%s:%d %s  m_map_history_views.size %d", __FILE__, __LINE__, __func__, m_map_history_views.size());
        historyView = elm_genlist_item_append(m_genListCenter, m_small_item_class, itemData, nullptr, ELM_GENLIST_ITEM_NONE, nullptr, this);
    } else {
        BROWSER_LOGD("%s:%d %s  m_map_history_views.size %d", __FILE__, __LINE__, __func__, m_map_history_views.size());
        historyView = elm_genlist_item_append(m_genListRight, m_small_item_class, itemData, nullptr, ELM_GENLIST_ITEM_NONE, nullptr, this);
    }

    m_map_history_views.insert(std::pair<std::string,Elm_Object_Item*>(hi->getUrl(),historyView));
}

void MainUI::addHistoryItems(std::shared_ptr<services::HistoryItemVector> items)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
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
         BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
         for (auto it = items.begin(); it != items.end(); ++it) {
                 addBookmarkItem(*it);
         }
}

char* MainUI::_grid_text_get(void *data, Evas_Object *obj, const char *part)
{
    HistoryItemData *itemData = reinterpret_cast<HistoryItemData*>(data);
    if (!strcmp(part, "page_title")) {
        return strdup(itemData->item->getTitle().c_str());
    }
    if (!strcmp(part, "page_url")) {
        return tools::clearURL(itemData->item->getUrl());
    }
    return strdup("");
}

char* MainUI::_grid_bookmark_text_get(void *data, Evas_Object *obj, const char *part)
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

Evas_Object * MainUI::_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    HistoryItemData *itemData = reinterpret_cast<HistoryItemData*>(data);

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
		evas_object_smart_callback_add(thumbButton, "clicked", _thumbSelected, data);
		return thumbButton;
    }
    return nullptr;
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
                evas_object_smart_callback_add(thumbButton, "clicked", _thumbSelected, data);
                return thumbButton;
    }
    return nullptr;
}

void MainUI::_thumbSelected(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
    itemData->mainUI->mostVisitedTileClicked(itemData->item, DetailPopup::HISTORY_ITEMS_NO);
}

void MainUI::clearHistoryGenlist()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    elm_genlist_clear(m_genListRight);
    elm_genlist_clear(m_genListLeft);
    elm_genlist_clear(m_genListCenter);
    m_map_history_views.clear();
}

void MainUI::showHistory()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    if (elm_layout_content_get(m_layout, "elm.swallow.content") == m_mostVisitedView)
        return;

    //TODO: remove these "evas_object_hide" and "evas_object_show" after cleaning up the mess in whole app window.
    elm_layout_content_unset(m_layout, "elm.swallow.content");
    evas_object_hide(m_bookmarksView);
    elm_layout_content_set(m_layout, "elm.swallow.content", m_mostVisitedView);
    evas_object_show(m_mostVisitedView);

    elm_object_focus_set(m_mostVisitedButton, true);

    if (m_map_history_views.empty()) {
        setEmptyView(true);
        return;
    }
    setEmptyView(false);
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

    if (elm_layout_content_get(m_layout, "elm.swallow.content") == m_bookmarksView)
        return;

    //TODO: remove these "evas_object_hide" and "evas_object_show" after cleaning up the mess in whole app window.
    elm_layout_content_unset(m_layout, "elm.swallow.content");
    evas_object_hide(m_mostVisitedView);
    elm_layout_content_set(m_layout, "elm.swallow.content", m_bookmarksView);
    evas_object_show(m_bookmarksView);

    elm_object_focus_set(m_bookmarksButton, true);
}

void MainUI::hide()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    //TODO: remove these "evas_object_hide" after cleaning up the mess in whole app window.
    evas_object_hide(m_layout);
    evas_object_hide(m_mostVisitedView);
    evas_object_hide(m_bookmarksView);
    clearItems();
}

void MainUI::clearItems()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    clearHistoryGenlist();
    clearBookmarkGengrid();
}

void MainUI::openDetailPopup(std::shared_ptr<services::HistoryItem> currItem, std::shared_ptr<services::HistoryItemVector> prevItems)
{
    m_detailPopup.show(m_layout, currItem, prevItems);
}

void MainUI::showNoHistoryLabel()
{
    evas_object_hide(elm_object_part_content_get(m_mostVisitedView, "elm.swallow.left"));
    evas_object_hide(elm_object_part_content_get(m_mostVisitedView, "elm.swallow.right"));
    evas_object_hide(elm_object_part_content_get(m_mostVisitedView, "elm.swallow.center"));

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
