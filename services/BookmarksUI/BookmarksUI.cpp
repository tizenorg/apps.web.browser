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

#include "BookmarksUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"


namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(BookmarksUI, "org.tizen.browser.bookmarksui")


typedef struct _BookmarkItemData
{
	std::shared_ptr<tizen_browser::services::BookmarkItem> item;
	std::shared_ptr<tizen_browser::base_ui::BookmarksUI> bookmarksUI;
} BookmarkItemData;

BookmarksUI::BookmarksUI()
    : m_gengrid(NULL)
    , m_parent(NULL)
    , m_item_class(NULL)
	, m_gengridSetup(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

BookmarksUI::~BookmarksUI()
{

}

void BookmarksUI::init(Evas_Object* p)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = p;
    m_gengrid = elm_gengrid_add(m_parent);

    evas_object_smart_callback_add(m_gengrid, "item,focused", focusItem, NULL);
    evas_object_smart_callback_add(m_gengrid, "item,unfocused", unFocusItem, NULL);
    evas_object_smart_callback_add(m_gengrid, "activated", _itemSelected, this);

      if (!m_item_class) {
            m_item_class = elm_genlist_item_class_new();
            m_item_class->item_style = "bookmark_item";
            m_item_class->func.text_get = NULL;
            m_item_class->func.content_get =  _grid_content_get;
            m_item_class->func.state_get = NULL;
            m_item_class->func.del = NULL;
        }
}


Evas_Object * BookmarksUI::getContent()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    M_ASSERT(m_parent);

    if(m_map_bookmark_views.size() == 0) {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", createNoBookmarksLabel());
    }


    if(!m_gengridSetup) {
        std::string edjFilePath = EDJE_DIR;
        edjFilePath.append("BookmarksUI/Bookmarks.edj");
        elm_theme_extension_add(NULL, edjFilePath.c_str());


        elm_object_style_set(m_gengrid, "bookmarks");

        elm_gengrid_align_set(m_gengrid, 0, 0);
        elm_gengrid_item_size_set(m_gengrid, 395, 357);
        elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
        elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
        elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
        elm_gengrid_highlight_mode_set(m_gengrid, EINA_TRUE);
        elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
        elm_scroller_page_size_set(m_gengrid, 0, 327);

        evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

        m_gengridSetup = true;
    }
    return m_gengrid;
}

void BookmarksUI::addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem> bi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = bi;
	itemData->bookmarksUI = std::shared_ptr<tizen_browser::base_ui::BookmarksUI>(this);
	Elm_Object_Item* bookmarkView = elm_gengrid_item_append(m_gengrid, m_item_class, itemData, NULL, this);
    m_map_bookmark_views.insert(std::pair<std::string,Elm_Object_Item*>(bi->getAddress(),bookmarkView));

    // unselect by default
    elm_gengrid_item_selected_set(bookmarkView, EINA_FALSE);

    setEmptyGengrid(false);
}

void BookmarksUI::addBookmarkItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
	 BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
	 for (auto it = items.begin(); it != items.end(); ++it) {
		 addBookmarkItem(*it);
	 }
}

void BookmarksUI::removeBookmarkItem(const std::string& uri)
{
    BROWSER_LOGD("[%s] uri=%s", __func__, uri.c_str());
    if(m_map_bookmark_views.find(uri) == m_map_bookmark_views.end()) {
        return;
    }

    Elm_Object_Item* bookmarkView = m_map_bookmark_views.at(uri);
    elm_object_item_del(bookmarkView);
    m_map_bookmark_views.erase(uri);

    setEmptyGengrid(0 == m_map_bookmark_views.size());
}

void  BookmarksUI::_item_deleted(void * /* data */, Evas_Object * /* obj */)
{

}

Evas_Object * BookmarksUI::_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    BookmarkItemData *itemData = reinterpret_cast<BookmarkItemData*>(data);

    if (!strcmp(part, "elm.thumbnail")) {
	    if (itemData->item->getThumbnail()) {
    		Evas_Object * thumb = tizen_browser::tools::EflTools::getEvasImage(itemData->item->getThumbnail(), itemData->bookmarksUI->m_parent);
    		return thumb;
    	}
    	else {
    		return NULL;
    	}
    }
    else if (!strcmp(part, "favicon")) {
	Evas_Object * favicon = NULL;
        if(itemData->item->getFavicon().get()){
            favicon = tizen_browser::tools::EflTools::getEvasImage(itemData->item->getFavicon(), obj);
        }
        return favicon;
    }
    else if (!strcmp(part, "elm.label")) {
		Evas_Object *label = elm_label_add(obj);
		elm_object_style_set(label, "bookmarks_label");
		elm_object_text_set(label, itemData->item->getTittle().c_str());
		return label;
    }
    else if (!strcmp(part, "elm.deleteButton")) {
		Evas_Object *deleteButton = elm_button_add(obj);
		elm_object_style_set(deleteButton, "deleteButton");
		evas_object_smart_callback_add(deleteButton, "clicked", tizen_browser::base_ui::BookmarksUI::_deleteBookmark, data);
		return deleteButton;
    }
    else if (!strcmp(part, "elm.thumbButton")) {
		Evas_Object *thumbButton = elm_button_add(obj);
		elm_object_style_set(thumbButton, "thumbButton");
		evas_object_smart_callback_add(thumbButton, "clicked", tizen_browser::base_ui::BookmarksUI::_thumbSelected, data);
		return thumbButton;
    }
    return NULL;
}

void BookmarksUI::_itemSelected(void * data, Evas_Object * /* obj */, void * event_info)
{
	Elm_Object_Item * selected = reinterpret_cast<Elm_Object_Item *>(event_info);
	BookmarkItemData * itemData = reinterpret_cast<BookmarkItemData *>(elm_object_item_data_get(selected));
	BookmarksUI * self = reinterpret_cast<BookmarksUI *>(data);

	self->bookmarkClicked(itemData->item);
}

void BookmarksUI::_deleteBookmark(void *data, Evas_Object * /* obj */, void * /* event_info */)
{
	BookmarkItemData * itemData = reinterpret_cast<BookmarkItemData *>(data);
	itemData->bookmarksUI->bookmarkDeleteClicked(itemData->item);
}

void BookmarksUI::_thumbSelected(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
	BookmarkItemData * itemData = reinterpret_cast<BookmarkItemData *>(data);
	itemData->bookmarksUI->bookmarkClicked(itemData->item);
}

Evas_Object* BookmarksUI::createNoBookmarksLabel()
{
    Evas_Object *label = elm_label_add(m_parent);
    elm_object_text_set(label, "No favorite websites.");
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    return label;
}

void BookmarksUI::setEmptyGengrid(bool setEmpty)
{
    if(setEmpty) {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", createNoBookmarksLabel());
    } else {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", NULL);
    }
}

void BookmarksUI::deleteAllItems()
{
    BROWSER_LOGD("Deleting all items from gengrid");
    elm_gengrid_clear(m_gengrid);
    m_map_bookmark_views.clear();
    setEmptyGengrid(true);
}

void BookmarksUI::focusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,in", "over2");

    // selected manually
    elm_gengrid_item_selected_set(item, EINA_TRUE);
}

void BookmarksUI::unFocusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,out", "over2");

    // unselected manually
    elm_gengrid_item_selected_set(item, EINA_FALSE);
}

std::shared_ptr<tizen_browser::services::BookmarkItem> BookmarksUI::getSelectedBookmarkItem()
{
    Elm_Object_Item * selected = elm_gengrid_selected_item_get(m_gengrid);

    if (!selected) {
            BROWSER_LOGD("none selected");
        return std::make_shared<tizen_browser::services::BookmarkItem>();
    }
	BookmarkItemData * itemData = reinterpret_cast<BookmarkItemData *>(elm_object_item_data_get(selected));
	return itemData->item;
}


}
}
