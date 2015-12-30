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
//    , m_layout(nullptr)
//    , m_top_content(nullptr)
//    , m_gengrid(nullptr)
#if !PROFILE_MOBILE
//    , m_bottom_content(nullptr)
#else
//    , m_more_button(nullptr)
//    , m_menu_bg_button(nullptr)
//    , m_menu(nullptr)
//    , m_edit_button(nullptr)
//    , m_delete_button(nullptr)
//    , m_remove_button(nullptr)
//    , m_cancel_top_button(nullptr)
//    , m_remove_top_button(nullptr)
    , m_delete_count(0)
    , m_remove_bookmark_mode(false)
#endif
//    , m_close_button(nullptr)
    //    , m_bookmark_item_class(nullptr)
    , m_rotation_state(0)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    for (int state = 0; state < TAB_SIZE; ++state)
        m_layout[state] = nullptr;

    m_edjFilePath[0] = EDJE_DIR;
    m_edjFilePath[0].append("BookmarkDetailsUI/Vertical.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath[0].c_str());
#if PROFILE_MOBILE
    m_edjFilePath[1] = EDJE_DIR;
    m_edjFilePath[1].append("BookmarkDetailsUI/Landscape.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath[1].c_str());
#endif
    createGengridItemClasses();
}

BookmarkDetailsUI::~BookmarkDetailsUI()
{
    for (int state = 0; state < TAB_SIZE; ++state) {
        evas_object_smart_callback_del(m_close_button[state], "clicked", _close_button_clicked);
#if PROFILE_MOBILE
        evas_object_smart_callback_del(m_more_button[state], "clicked", _more_button_clicked);
        evas_object_smart_callback_del(m_menu_bg_button[state], "clicked", _menu_bg_button_clicked);
        evas_object_smart_callback_del(m_edit_button[state], "clicked", _edit_button_clicked);
        evas_object_smart_callback_del(m_delete_button[state], "clicked", _delete_button_clicked);
        evas_object_smart_callback_del(m_remove_button[state], "clicked", _remove_button_clicked);
        evas_object_smart_callback_del(m_cancel_top_button[state], "clicked", _cancel_top_button_clicked);
        evas_object_smart_callback_del(m_remove_top_button[state], "clicked", _remove_top_button_clicked);
#endif

        evas_object_del(m_top_content[state]);
        evas_object_del(m_close_button[state]);
        evas_object_del(m_layout[state]);
        evas_object_del(m_gengrid[state]);
#if PROFILE_MOBILE
        evas_object_del(m_more_button[state]);
        evas_object_del(m_menu_bg_button[state]);
        evas_object_del(m_menu[state]);
        evas_object_del(m_edit_button[state]);
        evas_object_del(m_delete_button[state]);
        evas_object_del(m_remove_button[state]);
        evas_object_del(m_cancel_top_button[state]);
        evas_object_del(m_remove_top_button[state]);
#endif
        if(m_bookmark_item_class[state])
            elm_gengrid_item_class_free(m_bookmark_item_class[state]);
    }
}

void BookmarkDetailsUI::createGengridItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (int state = 0; state < TAB_SIZE; ++state) {
        m_bookmark_item_class[state] = elm_gengrid_item_class_new();
        m_bookmark_item_class[state]->item_style = state == 0 ? "grid_bookmark_item_0" : "grid_bookmark_item_1";
        m_bookmark_item_class[state]->func.text_get = _grid_bookmark_text_get;
        m_bookmark_item_class[state]->func.content_get =  _grid_bookmark_content_get;
        m_bookmark_item_class[state]->func.state_get = nullptr;
        m_bookmark_item_class[state]->func.del = nullptr;
    }
}

void BookmarkDetailsUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

void BookmarkDetailsUI::showUI()
{
    for (int state = 0; state < TAB_SIZE; ++state) {
        m_focusManager[state].startFocusManager(m_gengrid[state]);
        elm_object_signal_emit(m_layout[state], "hide_menu", "ui");
#if PROFILE_MOBILE
        evas_object_hide(m_menu[state]);
        evas_object_hide(elm_object_part_content_get(m_layout[state], "more_swallow"));
        evas_object_hide(m_menu_bg_button[state]);
        evas_object_hide(elm_object_part_content_get(m_layout[state], "more_bg"));
#endif
    }
    evas_object_show(m_layout[!m_rotation_state]);
    evas_object_hide(m_layout[!m_rotation_state]);
    evas_object_show(m_layout[m_rotation_state]);
}

void BookmarkDetailsUI::hideUI()
{
    for (int state = 0; state < TAB_SIZE; ++state) {
        evas_object_hide(m_layout[state]);
        evas_object_hide(m_gengrid[state]);
        elm_gengrid_clear(m_gengrid[state]);
#if PROFILE_MOBILE
        elm_object_signal_emit(m_top_content[state], "icon_less", "ui");
        m_map_bookmark[state].clear();
#endif
        m_focusManager[state].stopFocusManager();
    }
}

Evas_Object* BookmarkDetailsUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    for (int state = 0; state < TAB_SIZE; ++state)
        if (!m_layout[state])
            m_layout[state] = createLayout(m_parent, state);
    setEmpty(true);
    return m_layout[m_rotation_state];
}

void BookmarkDetailsUI::onBackPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
#if PROFILE_MOBILE
    if (m_remove_bookmark_mode)
        resetRemovalMode();
#endif
    closeBookmarkDetailsClicked();
}

#if PROFILE_MOBILE
void BookmarkDetailsUI::setLandscape(bool state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_rotation_state = state ? 0 : 1;
}

void BookmarkDetailsUI::resetContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bool show_menu = (evas_object_visible_get(m_menu[!m_rotation_state]) == EINA_TRUE);
    evas_object_hide(m_layout[!m_rotation_state]);
    evas_object_hide(m_gengrid[!m_rotation_state]);
    evas_object_show(m_layout[m_rotation_state]);
    evas_object_show(m_gengrid[m_rotation_state]);
    if (show_menu) {
        elm_object_signal_emit(m_layout[!m_rotation_state], "hide_menu", "ui");
        elm_object_signal_emit(m_top_content[!m_rotation_state], "icon_less", "ui");
        evas_object_hide(m_menu[!m_rotation_state]);
        evas_object_hide(elm_object_part_content_get(m_layout[!m_rotation_state], "more_swallow"));

        elm_object_signal_emit(m_layout[m_rotation_state], "show_menu", "ui");
        elm_object_signal_emit(m_top_content[m_rotation_state], "icon_more", "ui");
        evas_object_show(m_menu[m_rotation_state]);
        evas_object_show(elm_object_part_content_get(m_layout[m_rotation_state], "more_swallow"));
    }
}
#endif

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

void BookmarkDetailsUI::_bookmark_item_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkItemData * itemData = static_cast<BookmarkItemData*>(data);
#if PROFILE_MOBILE
        if (itemData->bookmarkDetailsUI->m_remove_bookmark_mode) {
            itemData->bookmarkDetailsUI->m_delete_count -= itemData->bookmarkDetailsUI->m_map_delete[itemData->item->getAddress()] ? 1 : -1;
            itemData->bookmarkDetailsUI->m_map_delete[itemData->item->getAddress()] =
                    !itemData->bookmarkDetailsUI->m_map_delete[itemData->item->getAddress()];
            for (int state = 0; state < TAB_SIZE; ++state) {
                elm_object_item_signal_emit(itemData->bookmarkDetailsUI->
                                        m_map_bookmark[state][itemData->item->getAddress()], "check_box_click", "ui");
                elm_object_part_text_set(itemData->bookmarkDetailsUI->m_top_content[state], "title_text", (boost::format("%d %s")
                                        % itemData->bookmarkDetailsUI->m_delete_count % _("IDS_BR_OPT_SELECTED")).str().c_str());
            }
        }
        else
            itemData->bookmarkDetailsUI->bookmarkItemClicked(itemData->item);
#else
        itemData->bookmarkDetailsUI->bookmarkItemClicked(itemData->item);
#endif
    }
    else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::createFocusVector(int state)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_focusManager[state].addItem(m_close_button[state]);
    m_focusManager[state].addItem(m_gengrid[state]);
    m_focusManager[state].setIterator();
}

void BookmarkDetailsUI::_close_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->onBackPressed();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}
#if PROFILE_MOBILE
void BookmarkDetailsUI::_more_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        for (int state = 0; state < TAB_SIZE; ++state) {
            if (evas_object_visible_get(bookmarkDetailsUI->m_menu[state]) == EINA_FALSE) {
                elm_object_signal_emit(bookmarkDetailsUI->m_layout[state], "show_menu", "ui");
                elm_object_signal_emit(bookmarkDetailsUI->m_top_content[state], "icon_more", "ui");
                evas_object_show(bookmarkDetailsUI->m_menu[state]);
                evas_object_show(elm_object_part_content_get(bookmarkDetailsUI->m_layout[state], "more_swallow"));
            } else {
                elm_object_signal_emit(bookmarkDetailsUI->m_layout[state], "hide_menu", "ui");
                elm_object_signal_emit(bookmarkDetailsUI->m_top_content[state], "icon_less", "ui");
                evas_object_hide(bookmarkDetailsUI->m_menu[state]);
                evas_object_hide(elm_object_part_content_get(bookmarkDetailsUI->m_layout[state], "more_swallow"));
            }
        }
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_menu_bg_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        for (int state = 0; state < TAB_SIZE; ++state)
            elm_object_signal_emit(bookmarkDetailsUI->m_top_content[state], "icon_less", "ui");
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_edit_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->editFolderButtonClicked(bookmarkDetailsUI->getFolderName());
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_delete_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->deleteFolderButtonClicked(bookmarkDetailsUI->getFolderName());
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_remove_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->m_map_delete.clear();
        bookmarkDetailsUI->m_remove_bookmark_mode = true;

        for (int state = 0 ; state < TAB_SIZE; ++state) {
            for (auto it = bookmarkDetailsUI->m_map_bookmark[state].begin(); it != bookmarkDetailsUI->m_map_bookmark[state].end(); ++it) {
                elm_object_item_signal_emit(it->second, "check_box_click", "ui");
                if (state == 0)
                    bookmarkDetailsUI->m_map_delete.insert(std::pair<std::string, bool>(it->first, false));
            }
            elm_object_signal_emit(bookmarkDetailsUI->m_top_content[state], "icon_less", "ui");
            elm_object_signal_emit(bookmarkDetailsUI->m_layout[state], "hide_menu", "ui");
            elm_object_signal_emit(bookmarkDetailsUI->m_top_content[state], "removal_mode", "ui");
            elm_object_part_text_set(bookmarkDetailsUI->m_top_content[state], "title_text", (boost::format("%d %s")
                                     % bookmarkDetailsUI->m_delete_count % _("IDS_BR_OPT_SELECTED")).str().c_str());
            evas_object_hide(bookmarkDetailsUI->m_menu[state]);
            evas_object_hide(elm_object_part_content_get(bookmarkDetailsUI->m_layout[state], "more_swallow"));
        }
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_cancel_top_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);
        bookmarkDetailsUI->resetRemovalMode();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkDetailsUI::_remove_top_button_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkDetailsUI* bookmarkDetailsUI = static_cast<BookmarkDetailsUI*>(data);

        std::vector<std::shared_ptr<services::BookmarkItem>> bookmarks;
        bookmarks.clear();

        for (int state = 0; state < TAB_SIZE; ++state) {
            for (auto it = bookmarkDetailsUI->m_map_delete.begin(); it != bookmarkDetailsUI->m_map_delete.end(); ++it)
                if (it->second) {
                    BookmarkItemData * itemData = static_cast<BookmarkItemData*>(elm_object_item_data_get(
                                                                                 bookmarkDetailsUI->m_map_bookmark[state][it->first]));
                    if (state == 0)
                        bookmarks.push_back(itemData->item);
                }
        }
        bookmarkDetailsUI->removeFoldersButtonClicked(bookmarks);
        bookmarkDetailsUI->resetRemovalMode(false);
        for (int state = 0; state < TAB_SIZE; ++state) {
            for (auto it = bookmarkDetailsUI->m_map_delete.begin(); it != bookmarkDetailsUI->m_map_delete.end(); ++it)
                if (it->second) {
                    elm_object_item_del(bookmarkDetailsUI->m_map_bookmark[state][it->first]);
                    bookmarkDetailsUI->m_map_bookmark[state].erase(it->first);
                }
            elm_object_part_text_set(bookmarkDetailsUI->m_top_content[state], "title_text", (boost::format("%s(%d)")
                                    % bookmarkDetailsUI->m_folder_name.c_str()
                                    % elm_gengrid_items_count(bookmarkDetailsUI->m_gengrid[state])).str().c_str());
        }
        bookmarkDetailsUI->m_map_delete.clear();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}
#endif

std::string BookmarkDetailsUI::getFolderName()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string title = elm_object_part_text_get(m_top_content[m_rotation_state], "title_text");
    auto i = 0;
    auto pos = title.find_last_of("(");
    return title.substr(i, pos-i);
}

void BookmarkDetailsUI::setEmpty(bool isEmpty, int state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (isEmpty)
        elm_object_signal_emit(m_layout[state], "show_no_favorites", "ui");
    else
        elm_object_signal_emit(m_layout[state], "hide_no_favorites", "ui");
}

void BookmarkDetailsUI::createTopContent(int state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_top_content[state] = elm_layout_add(m_layout[state]);
    elm_object_part_content_set(m_layout[state], "top_content", m_top_content[state]);
    evas_object_size_hint_weight_set(m_top_content[state], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_top_content[state], EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_top_content[state]);

    elm_layout_file_set(m_top_content[state], m_edjFilePath[state].c_str(), "top-content");

    m_close_button[state] = elm_button_add(m_top_content[state]);
    elm_object_style_set(m_close_button[state], "invisible_button");
    evas_object_smart_callback_add(m_close_button[state], "clicked", _close_button_clicked, this);
    evas_object_show(m_close_button[state]);
    elm_object_part_content_set(m_top_content[state], "close_click", m_close_button[state]);

    elm_object_focus_custom_chain_append(m_top_content[state], m_close_button[state], nullptr);
    elm_object_focus_set(m_close_button[state], EINA_TRUE);
    elm_object_tree_focus_allow_set(m_layout[state], EINA_TRUE);
    elm_object_focus_allow_set(m_close_button[state], EINA_TRUE);
#if PROFILE_MOBILE
    m_more_button[state] = elm_button_add(m_top_content[state]);
    elm_object_style_set(m_more_button[state], "invisible_button");
    evas_object_smart_callback_add(m_more_button[state], "clicked", _more_button_clicked, this);
    elm_object_part_content_set(m_top_content[state], "more_click", m_more_button[state]);
    evas_object_show(m_more_button[state]);

    elm_object_focus_custom_chain_append(m_top_content[state], m_more_button[state], nullptr);
    elm_object_focus_set(m_more_button[state], EINA_TRUE);
    elm_object_tree_focus_allow_set(m_layout[state], EINA_TRUE);
    elm_object_focus_allow_set(m_more_button[state], EINA_TRUE);

    m_cancel_top_button[state] = elm_button_add(m_top_content[state]);
    elm_object_style_set(m_cancel_top_button[state], "invisible_button");
    evas_object_smart_callback_add(m_cancel_top_button[state], "clicked", _cancel_top_button_clicked, this);
    elm_object_part_content_set(m_top_content[state], "cancel_click_2", m_cancel_top_button[state]);

    m_remove_top_button[state] = elm_button_add(m_top_content[state]);
    elm_object_style_set(m_remove_top_button[state], "invisible_button");
    evas_object_smart_callback_add(m_remove_top_button[state], "clicked", _remove_top_button_clicked, this);
    elm_object_part_content_set(m_top_content[state], "remove_click_2", m_remove_top_button[state]);
#endif
}

void BookmarkDetailsUI::createGengrid(int state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
    m_gengrid[state] = elm_gengrid_add(m_layout[state]);
    elm_object_part_content_set(m_layout[state], "elm.swallow.grid", m_gengrid[state]);
    elm_gengrid_align_set(m_gengrid[state], 0, 0);
    elm_gengrid_select_mode_set(m_gengrid[state], ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid[state], EINA_FALSE);
#if PROFILE_MOBILE
    elm_scroller_bounce_set(m_gengrid[state], EINA_FALSE, EINA_TRUE);
    elm_object_scroll_lock_x_set(m_gengrid[state], EINA_TRUE);
    if (state == 1)
        elm_gengrid_item_size_set(m_gengrid[state], (290+18) * efl_scale, (308+18) * efl_scale);
    else
        elm_gengrid_item_size_set(m_gengrid[state], (319+18) * efl_scale, (361+18) * efl_scale);
#else
    elm_object_style_set(m_gengrid[state], "back_ground");
    elm_scroller_policy_set(m_gengrid[state], ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid[state], 0, 327);
    evas_object_size_hint_weight_set(m_gengrid[state], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid[state], EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_horizontal_set(m_gengrid[state], EINA_FALSE);
    elm_gengrid_item_size_set(m_gengrid[state], 404 * efl_scale, 320 * efl_scale);
#endif
}

#if PROFILE_MOBILE
void BookmarkDetailsUI::createMenuDetails(int state)
{
    m_menu_bg_button[state] = elm_button_add(m_layout[state]);
    elm_object_style_set(m_menu_bg_button[state], "invisible_button");
    evas_object_smart_callback_add(m_menu_bg_button[state], "clicked", _menu_bg_button_clicked, this);
    elm_object_part_content_set(m_layout[state], "more_bg_click", m_menu_bg_button[state]);

    m_menu[state] = elm_box_add(m_layout[state]);
    evas_object_size_hint_weight_set(m_menu[state], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_menu[state], EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_edit_button[state] = elm_button_add(m_menu[state]);
    elm_object_style_set(m_edit_button[state], "more-button");
    evas_object_size_hint_weight_set(m_edit_button[state], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_edit_button[state], 0.0, 0.0);
    elm_object_part_text_set(m_edit_button[state], "elm.text", "Edit folder name");
    elm_box_pack_end(m_menu[state], m_edit_button[state]);
    evas_object_smart_callback_add(m_edit_button[state], "clicked", _edit_button_clicked, this);

    evas_object_show(m_edit_button[state]);
    elm_object_tree_focus_allow_set(m_edit_button[state], EINA_FALSE);

    m_delete_button[state] = elm_button_add(m_menu[state]);
    elm_object_style_set(m_delete_button[state], "more-button");
    evas_object_size_hint_weight_set(m_delete_button[state], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_delete_button[state], 0.0, 0.0);
    elm_object_part_text_set(m_delete_button[state], "elm.text", "Delete folder");
    elm_box_pack_end(m_menu[state], m_delete_button[state]);
    evas_object_smart_callback_add(m_delete_button[state], "clicked", _delete_button_clicked, this);

    evas_object_show(m_delete_button[state]);
    elm_object_tree_focus_allow_set(m_delete_button[state], EINA_FALSE);

    m_remove_button[state] = elm_button_add(m_menu[state]);
    elm_object_style_set(m_remove_button[state], "more-shadow-button");
    evas_object_size_hint_weight_set(m_remove_button[state], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_remove_button[state], 0.0, 0.0);
    elm_object_part_text_set(m_remove_button[state], "elm.text", "Remove Bookmarks");
    elm_box_pack_end(m_menu[state], m_remove_button[state]);
    evas_object_smart_callback_add(m_remove_button[state], "clicked", _remove_button_clicked, this);

    evas_object_show(m_remove_button[state]);
    elm_object_tree_focus_allow_set(m_remove_button[state], EINA_FALSE);

    elm_object_part_content_set(m_layout[state], "more_swallow", m_menu[state]);
    evas_object_show(m_menu[state]);
}

void BookmarkDetailsUI::resetRemovalMode(bool clear)
{
    if (clear)
        m_map_delete.clear();
    m_delete_count = 0;
    m_remove_bookmark_mode = false;
    for (int state = 0; state < TAB_SIZE; ++state) {
        elm_object_signal_emit(m_top_content[state], "default_mode", "ui");
        if (clear)
            elm_object_part_text_set(m_top_content[state], "title_text", (boost::format("%s(%d)") % m_folder_name.c_str()
                                 % elm_gengrid_items_count(m_gengrid[state])).str().c_str());
        for (auto it = m_map_bookmark[state].begin(); it != m_map_bookmark[state].end(); ++it)
            elm_object_item_signal_emit(it->second, "check_box_invisible", "ui");
    }
}
#else
void BookmarkDetailsUI::createBottomContent(int state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout[state]);

    m_bottom_content[state] = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout[state], "bottom_content", m_bottom_content[state]);
    evas_object_size_hint_weight_set(m_bottom_content[state], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_bottom_content[state], EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_bottom_content[state]);

    elm_layout_file_set(m_bottom_content[state], m_edjFilePath[state].c_str(), "bottom-content");
}
#endif

Evas_Object* BookmarkDetailsUI::createLayout(Evas_Object* parent, int state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_layout[state] = elm_layout_add(parent);
    elm_layout_file_set(m_layout[state], m_edjFilePath[state].c_str(), "bookmark-details-layout");
    evas_object_size_hint_weight_set(m_layout[state], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout[state], EVAS_HINT_FILL, EVAS_HINT_FILL);

    createTopContent(state);
    createGengrid(state);
#if PROFILE_MOBILE
    createMenuDetails(state);
#else
    createBottomContent(state);
#endif
    createFocusVector(state);
    return m_layout[state];
}

void BookmarkDetailsUI::addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem> hi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = hi;
    itemData->bookmarkDetailsUI.reset(this);
    for (int state = 0; state < TAB_SIZE; ++state) {
        Elm_Object_Item* bookmarkView = elm_gengrid_item_append(m_gengrid[state], m_bookmark_item_class[state],
                                                            itemData, _bookmark_item_clicked, itemData);
#if PROFILE_MOBILE
        m_map_bookmark[state].insert(std::pair<std::string,Elm_Object_Item*>(hi->getAddress(), bookmarkView));
#endif
        elm_gengrid_item_selected_set(bookmarkView, EINA_FALSE);
    }
}

void BookmarkDetailsUI::addBookmarks(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items, std::string folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_folder_name = folder_name;
    for (auto it = items.begin(); it != items.end(); ++it)
        addBookmarkItem(*it);
    for (int state = 0; state < TAB_SIZE; ++state) {
        elm_object_part_text_set(m_top_content[state], "title_text",
#if PROFILE_MOBILE
                                (boost::format("%s(%d)") % m_folder_name.c_str() % items.size()).str().c_str());
#else
                                (boost::format("Bookmark manager > %s") % m_folder_name.c_str()).str().c_str());
#endif
        elm_object_part_content_set(m_layout[state], "elm.swallow.grid", m_gengrid[state]);
#if PROFILE_MOBILE
        elm_box_unpack_all(m_menu[state]);
        //TODO: missing translation
        if (m_folder_name != _("IDS_BR_BODY_ALL") && m_folder_name != "Mobile") {
            evas_object_show(m_edit_button[state]);
            evas_object_show(m_delete_button[state]);
            elm_box_pack_end(m_menu[state], m_edit_button[state]);
            elm_box_pack_end(m_menu[state], m_delete_button[state]);
        } else {
            evas_object_hide(m_edit_button[state]);
            evas_object_hide(m_delete_button[state]);
        }
        elm_box_pack_end(m_menu[state], m_remove_button[state]);
#else
        elm_object_part_text_set(m_bottom_content[state], "text", (boost::format("%d %s") % elm_gengrid_items_count(m_gengrid[state]) %
                                                        (elm_gengrid_items_count(m_gengrid[state]) == 1 ? "bookmark" : "bookmarks")).str().c_str());
#endif
        if (items.size() != 0)
            setEmpty(false);
        evas_object_show(m_gengrid[state]);
    }
}

}
}

