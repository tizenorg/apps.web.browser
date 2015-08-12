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

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

const int SMALL_TILES_ROWS = 2;

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
    : m_gengrid(nullptr)
    , m_genListTop(nullptr)
    , m_genListBottom(nullptr)
    , m_parent(nullptr)
    , m_big_item_class(nullptr)
    , m_small_item_class(nullptr)
    , m_bookmark_item_class(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    edjFilePath = EDJE_DIR;
    edjFilePath.append("MainUI/MainUI.edj");
}

MainUI::~MainUI()
{
}

void MainUI::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    m_layout = elm_layout_add(parent);
    elm_layout_file_set(m_layout, edjFilePath.c_str(), "mv_bookmarks");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_layout);
    m_parent = m_layout;

    m_genListLeft = elm_genlist_add(m_layout);
    elm_object_part_content_set(m_layout, "elm.swallow.left", m_genListLeft);
    elm_genlist_homogeneous_set(m_genListLeft, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListLeft, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListLeft, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListLeft, ELM_LIST_LIMIT);
    evas_object_size_hint_weight_set(m_genListLeft, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    m_genListCenter = elm_genlist_add(m_layout);
    elm_object_part_content_set(m_layout, "elm.swallow.center", m_genListCenter);
    elm_genlist_homogeneous_set(m_genListCenter, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListCenter, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListCenter, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListCenter, ELM_LIST_LIMIT);
    evas_object_size_hint_weight_set(m_genListCenter, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    m_genListRight = elm_genlist_add(m_layout);
    elm_object_part_content_set(m_layout, "elm.swallow.right", m_genListRight);
    elm_genlist_homogeneous_set(m_genListRight, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListRight, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListRight, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListRight, ELM_LIST_LIMIT);
    evas_object_size_hint_weight_set(m_genListRight, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

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

    /*evas_object_smart_callback_add(m_genList, "item,focused", focusItem, this);
    evas_object_smart_callback_add(m_genList, "item,unfocused", unFocusItem, nullptr);*/


    m_gengrid = elm_gengrid_add(m_layout);
    //elm_object_part_content_set(m_layout, "elm.swallow.grid", m_gengrid);

    evas_object_smart_callback_add(m_gengrid, "item,focused", focusItem, nullptr);
    evas_object_smart_callback_add(m_gengrid, "item,unfocused", unFocusItem, nullptr);
    evas_object_smart_callback_add(m_gengrid, "activated", _itemSelected, this);

	if (!m_bookmark_item_class) {
            m_bookmark_item_class = elm_gengrid_item_class_new();
            m_bookmark_item_class->item_style = "grid_item";
            m_bookmark_item_class->func.text_get = _grid_bookmark_text_get;
            m_bookmark_item_class->func.content_get =  _grid_bookmark_content_get;
            m_bookmark_item_class->func.state_get = nullptr;
            m_bookmark_item_class->func.del = nullptr;
        }

    M_ASSERT(m_parent);
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
    //elm_gengrid_highlight_mode_set(m_gengrid, EINA_TRUE);
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_item_size_set(m_gengrid, 364 * efl_scale, 320 * efl_scale);

    showTopButtons();
    showBottomButton();
}


void MainUI::showTopButtons()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    m_genListTop = elm_genlist_add(m_layout);
    elm_object_part_content_set(m_layout, "elm.swallow.genlistTop", m_genListTop);
    elm_genlist_homogeneous_set(m_genListTop, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListTop, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListTop, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListTop, ELM_LIST_LIMIT);
    //elm_genlist_decorate_mode_set(m_genListTop, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genListTop, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    /*evas_object_smart_callback_add(m_genList, "item,focused", focusItem, this);
    evas_object_smart_callback_add(m_genList, "item,unfocused", unFocusItem, nullptr);*/

    m_itemClassTop = elm_genlist_item_class_new();
    m_itemClassTop->item_style = "top_button_item";
    m_itemClassTop->func.text_get = nullptr; // &listTopItemTextGet;
    m_itemClassTop->func.content_get = &listTopItemContentGet;
    m_itemClassTop->func.state_get = 0;
    m_itemClassTop->func.del = 0;

    ItemData * id = new ItemData;
    id->mainUI = this;
    Elm_Object_Item* elmItem = elm_genlist_item_append(m_genListTop,            //genlist
                                                       m_itemClassTop,          //item Class
                                                      id,
                                                      nullptr,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,//item type
                                                      nullptr,
                                                      nullptr                  //data passed to above function
                                                     );
    id->e_item = elmItem;
    ItemData * id2 = new ItemData;
    id2->mainUI = this;
    Elm_Object_Item* elmItem2 = elm_genlist_item_append(m_genListTop,            //genlist
                                                       m_itemClassTop,          //item Class
                                                      id2,
                                                      nullptr,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,//item type
                                                      nullptr,
                                                      nullptr                  //data passed to above function
                                                     );
    id2->e_item = elmItem2;
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void MainUI::showBottomButton()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    m_genListBottom = elm_genlist_add(m_layout);
    elm_genlist_homogeneous_set(m_genListBottom, EINA_FALSE);
    elm_genlist_multi_select_set(m_genListBottom, EINA_FALSE);
    elm_genlist_select_mode_set(m_genListBottom, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genListBottom, ELM_LIST_LIMIT);
    //elm_genlist_decorate_mode_set(m_genListBottom, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genListBottom, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    /*evas_object_smart_callback_add(m_genList, "item,focused", focusItem, this);
    evas_object_smart_callback_add(m_genList, "item,unfocused", unFocusItem, nullptr);*/

    m_itemClassBottom = elm_genlist_item_class_new();
    m_itemClassBottom->item_style = "bottom_button_item";
    m_itemClassBottom->func.text_get = nullptr;
    m_itemClassBottom->func.content_get = &listItemBottomContentGet;
    m_itemClassBottom->func.state_get = 0;
    m_itemClassBottom->func.del = 0;

    ItemData * id = new ItemData;
    id->mainUI = this;
    Elm_Object_Item* elmItem = elm_genlist_item_append(m_genListBottom,            //genlist
                                                       m_itemClassBottom,          //item Class
                                                      id,
                                                      nullptr,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,//item type
                                                      nullptr,
                                                      nullptr                  //data passed to above function
                                                     );
    id->e_item = elmItem;
}

Evas_Object* MainUI::listTopItemContentGet(void* data, Evas_Object* obj, const char* part)
{
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        if(!strcmp(part, "mostvisited_click"))
        {
                Evas_Object *mvButton = elm_button_add(obj);
                elm_object_style_set(mvButton, "invisible_button");
                evas_object_smart_callback_add(mvButton, "clicked", tizen_browser::base_ui::MainUI::_mostVisited_clicked, data);
                return mvButton;
        }
        else if(!strcmp(part, "bookmark_click"))
        {
		Evas_Object *bmButton = elm_button_add(obj);
                elm_object_style_set(bmButton, "invisible_button");
                evas_object_smart_callback_add(bmButton, "clicked", tizen_browser::base_ui::MainUI::_bookmark_clicked, data);
		return bmButton;
	}
        return nullptr;
}

void MainUI::_mostVisited_clicked(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
	ItemData* itemData = reinterpret_cast<ItemData *>(data);
	itemData->mainUI->mostVisitedClicked(std::string());
}

void MainUI::_bookmark_clicked(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
	ItemData* itemData = reinterpret_cast<ItemData *>(data);
	itemData->mainUI->bookmarkClicked(std::string());
}

void MainUI::_bookmark_manager_clicked(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
	ItemData* itemData = reinterpret_cast<ItemData *>(data);
	itemData->mainUI->bookmarkManagerClicked(std::string());
}

Evas_Object* MainUI::listItemBottomContentGet(void* data, Evas_Object* obj, const char* part)
{
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
	if(!strcmp(part, "bookmarkmanager_click"))
	{
		Evas_Object *bmButton = elm_button_add(obj);
        	elm_object_style_set(bmButton, "invisible_button");
        	evas_object_smart_callback_add(bmButton, "clicked", tizen_browser::base_ui::MainUI::_bookmark_manager_clicked, data);
        	return bmButton;
	}
	return nullptr;
}

void MainUI::addHistoryItem(std::shared_ptr<tizen_browser::services::HistoryItem> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (m_map_history_views.size() >= 5)
	return;

    HistoryItemData *itemData = new HistoryItemData();
    itemData->item = hi;
    itemData->mainUI = std::shared_ptr<tizen_browser::base_ui::MainUI>(this);
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

    setEmptyGengrid(false);
}

void MainUI::addHistoryItems(std::vector<std::shared_ptr<tizen_browser::services::HistoryItem> > items)
{
         BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
	 int i = 0;
         for (auto it = items.begin(); it != items.end(); ++it) {
		 i++;
		 if (i > 5) break;
                 addHistoryItem(*it);
         }
}

void MainUI::addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem> bi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = bi;
        itemData->mainUI = std::shared_ptr<tizen_browser::base_ui::MainUI>(this);
        Elm_Object_Item* bookmarkView = elm_gengrid_item_append(m_gengrid, m_bookmark_item_class, itemData, nullptr, this);
    m_map_bookmark_views.insert(std::pair<std::string,Elm_Object_Item*>(bi->getAddress(),bookmarkView));

    // unselect by default
    elm_gengrid_item_selected_set(bookmarkView, EINA_FALSE);

    setEmptyGengrid(false);
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
		return strdup(itemData->item->getUrl().c_str());
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
		evas_object_smart_callback_add(thumbButton, "clicked", tizen_browser::base_ui::MainUI::_thumbSelected, data);
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
                evas_object_smart_callback_add(thumbButton, "clicked", tizen_browser::base_ui::MainUI::_thumbSelected, data);
                return thumbButton;
    }
    return nullptr;
}


void MainUI::_itemSelected(void * data, Evas_Object * /* obj */, void * event_info)
{
	Elm_Object_Item * selected = reinterpret_cast<Elm_Object_Item *>(event_info);
	HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(elm_object_item_data_get(selected));
	MainUI * self = reinterpret_cast<MainUI *>(data);

	self->historyClicked(itemData->item);
}

void MainUI::_thumbSelected(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
	HistoryItemData * itemData = reinterpret_cast<HistoryItemData *>(data);
	itemData->mainUI->historyClicked(itemData->item);
}

void MainUI::clearHistoryGenlist()
{
    elm_genlist_clear(m_genListRight);
    elm_genlist_clear(m_genListLeft);
    elm_genlist_clear(m_genListCenter);
    m_map_history_views.clear();
    evas_object_hide(elm_object_part_content_get(m_layout, "elm.swallow.left"));
    evas_object_hide(elm_object_part_content_get(m_layout, "elm.swallow.right"));
    evas_object_hide(elm_object_part_content_get(m_layout, "elm.swallow.center"));
}

void MainUI::showHistoryGenlist()
{
    evas_object_hide(elm_object_part_content_get(m_layout, "elm.swallow.grid"));
    elm_object_part_content_unset(m_layout, "elm.swallow.grid");
    evas_object_hide(elm_object_part_content_get(m_layout, "elm.swallow.genlistBottom"));
    elm_object_part_content_unset(m_layout, "elm.swallow.genlistBottom");
    elm_object_part_content_set(m_layout, "elm.swallow.left", m_genListLeft);
    elm_object_part_content_set(m_layout, "elm.swallow.right", m_genListRight);
    elm_object_part_content_set(m_layout, "elm.swallow.center", m_genListCenter);
}

void MainUI::clearBookmarkGengrid()
{
    elm_gengrid_clear(m_gengrid);
    m_map_bookmark_views.clear();
    evas_object_hide(elm_layout_content_get(m_layout, "elm.swallow.grid"));
}

void MainUI::showBookmarkGengrid()
{
    evas_object_hide(elm_object_part_content_get(m_layout, "elm.swallow.left"));
    evas_object_hide(elm_object_part_content_get(m_layout, "elm.swallow.right"));
    evas_object_hide(elm_object_part_content_get(m_layout, "elm.swallow.center"));
    elm_object_part_content_unset(m_layout, "elm.swallow.left");
    elm_object_part_content_unset(m_layout, "elm.swallow.right");
    elm_object_part_content_unset(m_layout, "elm.swallow.center");
    elm_object_part_content_set(m_layout, "elm.swallow.grid", m_gengrid);
    elm_object_part_content_set(m_layout, "elm.swallow.genlistBottom", m_genListBottom);
}

void MainUI::hide()
{
   BROWSER_LOGD("MainUI Hide");
   evas_object_hide(elm_layout_content_get(m_layout, "elm.swallow.genlistTop"));
   evas_object_hide(elm_layout_content_get(m_layout, "elm.swallow.genlistBottom"));
   evas_object_hide(m_layout);
   clearItems();
}

void MainUI::clearItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    clearHistoryGenlist();
    clearBookmarkGengrid();
    elm_genlist_clear(m_genListTop);
    elm_genlist_clear(m_genListBottom);
}

Evas_Object* MainUI::createNoHistoryLabel()
{
    Evas_Object *label = elm_label_add(m_parent);
    elm_object_text_set(label, "No favorite websites.");
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    return label;
}

void MainUI::setEmptyGengrid(bool setEmpty)
{
    if(setEmpty) {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", createNoHistoryLabel());
    } else {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", nullptr);
    }
}

void MainUI::focusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,in", "over2");

    // selected manually
    elm_gengrid_item_selected_set(item, EINA_TRUE);
}

void MainUI::unFocusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,out", "over2");

    // unselected manually
    elm_gengrid_item_selected_set(item, EINA_FALSE);
}

}
}
