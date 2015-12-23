/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

/*
 * BookmarkDetailsUI.cpp
 *
 *  Created on: Dec 10, 2015
 *      Author: m.kawonczyk@samsung.com
 */


#include <Elementary.h>
#include <boost/format.hpp>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>
#include "app_i18n.h"
#include "BookmarkDetailsUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(BookmarkDetailsUI, "org.tizen.browser.bookmarkdetailsui")

struct ItemData
{
    tizen_browser::base_ui::BookmarkDetailsUI * m_bookmarkDetails;
    tizen_browser::services::BookmarkItem * h_item;
    Elm_Object_Item * e_item;
};

typedef struct
{
    std::shared_ptr<tizen_browser::services::BookmarkItem> item;
    std::shared_ptr<tizen_browser::base_ui::BookmarkDetailsUI> bookmarkDetailsUI;
} BookmarkItemData;

BookmarkDetailsUI::BookmarkDetailsUI()
    : m_parent(nullptr)
    , m_layout(nullptr)
    , m_top_content(nullptr)
    , m_gengrid(nullptr)
#if !PROFILE_MOBILE
    , m_bottom_content(nullptr)
#else
    , m_more_button(nullptr)
    , m_menu(nullptr)
    , m_edit_button(nullptr)
    , m_delete_button(nullptr)
    , m_remove_button(nullptr)
#endif
    , m_close_button(nullptr)
    , m_bookmark_item_class(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("BookmarkDetailsUI/BookmarkDetailsUI.edj");
    createGengridItemClasses();
}

BookmarkDetailsUI::~BookmarkDetailsUI()
{
    evas_object_smart_callback_del(m_close_button, "clicked", _close_button_clicked);
#if PROFILE_MOBILE
    evas_object_smart_callback_del(m_more_button, "clicked", _more_button_clicked);
    evas_object_smart_callback_del(m_edit_button, "clicked", _edit_button_clicked);
    evas_object_smart_callback_del(m_delete_button, "clicked", _delete_button_clicked);
    evas_object_smart_callback_del(m_remove_button, "clicked", _remove_button_clicked);
#endif

    evas_object_del(m_top_content);
    evas_object_del(m_close_button);
    evas_object_del(m_layout);
    evas_object_del(m_gengrid);
#if PROFILE_MOBILE
    evas_object_del(m_more_button);
    evas_object_del(m_menu);
    evas_object_del(m_edit_button);
    evas_object_del(m_delete_button);
    evas_object_del(m_remove_button);
#endif

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_bookmark_item_class)
        elm_gengrid_item_class_free(m_bookmark_item_class);
}

void BookmarkDetailsUI::createGengridItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_bookmark_item_class = elm_gengrid_item_class_new();
    m_bookmark_item_class->item_style = "grid_bookmark_item";
    m_bookmark_item_class->func.text_get = _grid_bookmark_text_get;
    m_bookmark_item_class->func.content_get =  _grid_bookmark_content_get;
    m_bookmark_item_class->func.state_get = nullptr;
    m_bookmark_item_class->func.del = nullptr;
}

void BookmarkDetailsUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

void BookmarkDetailsUI::showUI()
{
    evas_object_show(m_layout);
    m_focusManager.startFocusManager(m_gengrid);
    elm_object_signal_emit(m_layout, "hide_menu", "ui");
#if PROFILE_MOBILE
    evas_object_hide(m_menu);
    evas_object_hide(elm_object_part_content_get(m_layout, "more_swallow"));
#endif
}

void BookmarkDetailsUI::hideUI()
{
    evas_object_hide(m_layout);
    evas_object_hide(m_gengrid);
    elm_gengrid_clear(m_gengrid);
    m_map_bookmark.clear();
    m_focusManager.stopFocusManager();
}

Evas_Object* BookmarkDetailsUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_layout)
      m_layout = createLayout(m_parent);
    setEmpty(true);
    return m_layout;
}

void BookmarkDetailsUI::onBackPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    closeBookmarkDetailsClicked();
}

char* BookmarkDetailsUI::_grid_bookmark_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr))
    {
        BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__, part);
        BookmarkItemData *itemData = static_cast<BookmarkItemData*>(data);
        const char *part_name1 = "page_title";
        const char *part_name2 = "page_url";
        static const int part_name1_len = strlen(part_name1);
        static const int part_name2_len = strlen(part_name2);
        if (!strncmp(part_name1, part, part_name1_len))
        {
            return strdup(itemData->item->getTitle().c_str());
        }
        else if (!strncmp(part_name2, part, part_name2_len))
        {
            return strdup(itemData->item->getAddress().c_str());
        }
    }
    return strdup("");
}

Evas_Object * BookmarkDetailsUI::_grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part)
{
    if ((data != nullptr) && (obj != nullptr) && (part != nullptr))
    {
        BROWSER_LOGD("[%s:%d] part=%s", __PRETTY_FUNCTION__, __LINE__, part);
        BookmarkItemData *itemData = static_cast<BookmarkItemData*>(data);
        const char *part_name1 = "elm.thumbnail";
        static const int part_name1_len = strlen(part_name1);
        if (!strncmp(part_name1, part, part_name1_len))
        {
            std::shared_ptr<tizen_browser::tools::BrowserImage> image = itemData->item->getThumbnail();
            if (image)
            {
                return tizen_browser::tools::EflTools::getEvasImage(image, itemData->bookmarkDetailsUI->m_parent);
            }
        }
    }
    return nullptr;
}

void BookmarkDetailsUI::_bookmark_item_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkItemData * itemData = static_cast<BookmarkItemData*>(data);
        itemData->bookmarkDetailsUI->bookmarkItemClicked(itemData->item);
    }
}

void BookmarkDetailsUI::createFocusVector()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_focusManager.addItem(m_close_button);
    m_focusManager.addItem(m_gengrid);
    m_focusManager.setIterator();
}

void BookmarkDetailsUI::_close_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->onBackPressed();
    }
}
#if PROFILE_MOBILE
void BookmarkDetailsUI::_more_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        if (evas_object_visible_get(bookmarkDetailsUI->m_menu) == EINA_FALSE) {
            elm_object_signal_emit(bookmarkDetailsUI->m_layout, "show_menu", "ui");
            elm_object_signal_emit(bookmarkDetailsUI->m_top_content, "icon_more", "ui");
            evas_object_show(bookmarkDetailsUI->m_menu);
            evas_object_show(elm_object_part_content_get(bookmarkDetailsUI->m_layout,"more_swallow"));
        } else {
            elm_object_signal_emit(bookmarkDetailsUI->m_layout, "hide_menu", "ui");
            elm_object_signal_emit(bookmarkDetailsUI->m_top_content, "icon_less", "ui");
            evas_object_hide(bookmarkDetailsUI->m_menu);
            evas_object_hide(elm_object_part_content_get(bookmarkDetailsUI->m_layout,"more_swallow"));
        }
    }
}

void BookmarkDetailsUI::_edit_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->editFolderButtonClicked(bookmarkDetailsUI->getFolderName());
    }
}

void BookmarkDetailsUI::_delete_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->deleteFolderButtonClicked(bookmarkDetailsUI->getFolderName());
    }
}

void BookmarkDetailsUI::_remove_button_clicked(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //TODO: remove thumbnails: https://bugs.tizen.org/jira/browse/TM-122
}
#endif

std::string BookmarkDetailsUI::getFolderName()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string title = elm_object_part_text_get(m_top_content, "title_text");
    auto i = 0;
    auto pos = title.find_last_of("(");
    return title.substr(i, pos-i);
}

void BookmarkDetailsUI::setEmpty(bool isEmpty)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (isEmpty) {
        elm_object_signal_emit(m_layout, "show_no_favorites", "ui");
    } else {
        elm_object_signal_emit(m_layout, "hide_no_favorites", "ui");
    }
}

void BookmarkDetailsUI::createTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_top_content = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout, "top_content", m_top_content);
    evas_object_size_hint_weight_set(m_top_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_top_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_top_content);

    elm_layout_file_set(m_top_content, m_edjFilePath.c_str(), "top-content");

    m_close_button = elm_button_add(m_top_content);
    elm_object_style_set(m_close_button, "invisible_button");
    evas_object_smart_callback_add(m_close_button, "clicked", _close_button_clicked, this);
    evas_object_show(m_close_button);
    elm_object_part_content_set(m_top_content, "close_click", m_close_button);

    elm_object_focus_custom_chain_append(m_top_content, m_close_button, nullptr);
    elm_object_focus_set(m_close_button, EINA_TRUE);
    elm_object_tree_focus_allow_set(m_layout, EINA_TRUE);
    elm_object_focus_allow_set(m_close_button, EINA_TRUE);
#if PROFILE_MOBILE
    m_more_button = elm_button_add(m_top_content);
    elm_object_style_set(m_more_button, "invisible_button");
    evas_object_smart_callback_add(m_more_button, "clicked", _more_button_clicked, this);
    elm_object_part_content_set(m_top_content, "more_click", m_more_button);
    evas_object_show(m_more_button);

    elm_object_focus_custom_chain_append(m_top_content, m_more_button, nullptr);
    elm_object_focus_set(m_more_button, EINA_TRUE);
    elm_object_tree_focus_allow_set(m_layout, EINA_TRUE);
    elm_object_focus_allow_set(m_more_button, EINA_TRUE);
#endif
}

void BookmarkDetailsUI::createGengrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
    m_gengrid = elm_gengrid_add(m_layout);
    elm_object_part_content_set(m_layout, "elm.swallow.grid", m_gengrid);
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
#if PROFILE_MOBILE
    elm_scroller_bounce_set(m_gengrid, EINA_FALSE, EINA_TRUE);
    elm_object_scroll_lock_x_set(m_gengrid, EINA_TRUE);
    elm_gengrid_item_size_set(m_gengrid, (319+18) * efl_scale, (361+18) * efl_scale);
#else
    elm_object_style_set(m_gengrid, "back_ground");
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
    elm_gengrid_item_size_set(m_gengrid, 404 * efl_scale, 320 * efl_scale);
#endif
}

#if PROFILE_MOBILE
void BookmarkDetailsUI::createMenuDetails()
{
    m_menu = elm_box_add(m_layout);
    evas_object_size_hint_weight_set(m_menu, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_menu, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_edit_button = elm_button_add(m_menu);
    elm_object_style_set(m_edit_button, "more-button");
    evas_object_size_hint_weight_set(m_edit_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_edit_button, 0.0, 0.0);
    elm_object_part_text_set(m_edit_button, "elm.text", "Edit folder name");
    elm_box_pack_end(m_menu, m_edit_button);
    evas_object_smart_callback_add(m_edit_button, "clicked", _edit_button_clicked, this);
    elm_object_signal_emit(m_edit_button, "visible", "ui");

    evas_object_show(m_edit_button);
    elm_object_tree_focus_allow_set(m_edit_button, EINA_FALSE);

    m_delete_button = elm_button_add(m_menu);
    elm_object_style_set(m_delete_button, "more-button");
    evas_object_size_hint_weight_set(m_delete_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_delete_button, 0.0, 0.0);
    elm_object_part_text_set(m_delete_button, "elm.text", "Delete folder");
    elm_box_pack_end(m_menu, m_delete_button);
    evas_object_smart_callback_add(m_delete_button, "clicked", _delete_button_clicked, this);
    elm_object_signal_emit(m_delete_button, "visible", "ui");

    evas_object_show(m_delete_button);
    elm_object_tree_focus_allow_set(m_delete_button, EINA_FALSE);

    m_remove_button = elm_button_add(m_menu);
    elm_object_style_set(m_remove_button, "more-button");
    evas_object_size_hint_weight_set(m_remove_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_remove_button, 0.0, 0.0);
    elm_object_part_text_set(m_remove_button, "elm.text", "Remove Bookmarks");
    elm_box_pack_end(m_menu, m_remove_button);
    evas_object_smart_callback_add(m_remove_button, "clicked", _remove_button_clicked, this);

    evas_object_show(m_remove_button);
    elm_object_tree_focus_allow_set(m_remove_button, EINA_FALSE);

    elm_object_part_content_set(m_layout, "more_swallow", m_menu);
    evas_object_show(m_menu);
}
#else
void BookmarkDetailsUI::createBottomContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_bottom_content = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout, "bottom_content", m_bottom_content);
    evas_object_size_hint_weight_set(m_bottom_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_bottom_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_bottom_content);

    elm_layout_file_set(m_bottom_content, m_edjFilePath.c_str(), "bottom-content");
}
#endif

Evas_Object* BookmarkDetailsUI::createLayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_layout = elm_layout_add(parent);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "bookmark-details-layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    createTopContent();
    createGengrid();
#if PROFILE_MOBILE
    createMenuDetails();
#else
    createBottomContent();
#endif
    createFocusVector();
    return m_layout;
}

void BookmarkDetailsUI::addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem> hi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = hi;
    itemData->bookmarkDetailsUI.reset(this);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_bookmark_item_class,
                                                            itemData, _bookmark_item_clicked, itemData);
    m_map_bookmark.insert(std::pair<std::string,Elm_Object_Item*>(hi->getAddress(), BookmarkView));
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
}

void BookmarkDetailsUI::addBookmarks(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items, std::string folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    for (auto it = items.begin(); it != items.end(); ++it)
    {
        BROWSER_LOGD("%s %s",(*it)->getTitle().c_str(), (*it)->getAddress().c_str());
        addBookmarkItem(*it);
    }
    elm_object_part_text_set(m_top_content, "title_text",
#if PROFILE_MOBILE
                             (boost::format("%s(%d)") % folder_name.c_str() % items.size()).str().c_str());
#else
                             (boost::format("Bookmark manager > %s") % folder_name.c_str()).str().c_str());
#endif
    elm_object_part_content_set(m_layout, "elm.swallow.grid", m_gengrid);
    evas_object_show(m_gengrid);
#if PROFILE_MOBILE
    elm_box_unpack_all(m_menu);
    if (folder_name != _("IDS_BR_BODY_ALL") && folder_name != "Mobile") {
        evas_object_show(m_edit_button);
        evas_object_show(m_delete_button);
        elm_box_pack_end(m_menu, m_edit_button);
        elm_box_pack_end(m_menu, m_delete_button);
    } else {
        evas_object_hide(m_edit_button);
        evas_object_hide(m_delete_button);
    }
    elm_box_pack_end(m_menu, m_remove_button);
#else
    elm_object_part_text_set(m_bottom_content, "text", (boost::format("%d %s") % elm_gengrid_items_count(m_gengrid) %
                                                        (elm_gengrid_items_count(m_gengrid) == 1 ? "bookmark" : "bookmarks")).str().c_str());
#endif
    if (items.size() != 0)
        setEmpty(false);
}

}
}
