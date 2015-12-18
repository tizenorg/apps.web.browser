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

/*
 * BookmarkFlowUI.cpp
 *
 *  Created on: Nov 10, 2015
 *      Author: m.kawonczyk@samsung.com
 */

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <boost/format.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "BookmarkFlowUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(BookmarkFlowUI, BOOKMARK_FLOW_SERVICE)

BookmarkFlowUI::BookmarkFlowUI()
    : m_parent(nullptr)
    , m_layout(nullptr)
    , m_titleArea(nullptr)
#if PROFILE_MOBILE
    , m_contentsArea(nullptr)
    , m_removeButton(nullptr)
    , m_entry(nullptr)
    , m_saveButton(nullptr)
    , m_cancelButton(nullptr)
    , m_inputCancelButton(nullptr)
    , m_folderButton(nullptr)
#else
    , m_gengrid(nullptr)
    , m_bg(nullptr)
#endif
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("BookmarkFlowUI/BookmarkFlowUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
#if PROFILE_MOBILE
    createGenlistItemClasses();
#else
    createGengridItemClasses();
#endif
}

BookmarkFlowUI::~BookmarkFlowUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

#if PROFILE_MOBILE
    evas_object_smart_callback_del(m_saveButton, "clicked", _save_clicked);
    evas_object_smart_callback_del(m_cancelButton, "clicked", _cancel_clicked);
    evas_object_smart_callback_del(m_entry, "focused", _entry_focused);
    evas_object_smart_callback_del(m_entry, "unfocused", _entry_unfocused);
    evas_object_smart_callback_del(m_entry, "changed,user",_entry_changed);
    evas_object_smart_callback_del(m_inputCancelButton, "clicked", _inputCancel_clicked);
    evas_object_smart_callback_del(m_folderButton, "clicked", _folder_clicked);
    evas_object_smart_callback_del(m_removeButton, "clicked", _remove_clicked);

    evas_object_del(m_removeButton);
    evas_object_del(m_entry);
    evas_object_del(m_saveButton);
    evas_object_del(m_cancelButton);
    evas_object_del(m_inputCancelButton);
    evas_object_del(m_folderButton);
    evas_object_del(m_contentsArea);
#else
    if(m_folder_new_item_class)
        elm_gengrid_item_class_free(m_folder_new_item_class);
    if(m_folder_custom_item_class)
        elm_gengrid_item_class_free(m_folder_custom_item_class);

    elm_gengrid_clear(m_gengrid);
    evas_object_del(m_gengrid);

    popupDismissed.disconnect_all_slots();
    popupShown.disconnect_all_slots();
#endif
    evas_object_del(m_titleArea);
    evas_object_del(m_layout);

    closeBookmarkFlowClicked.disconnect_all_slots();
    saveBookmark.disconnect_all_slots();
    editBookmark.disconnect_all_slots();
    removeBookmark.disconnect_all_slots();
    addFolder.disconnect_all_slots();
}

void BookmarkFlowUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* BookmarkFlowUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_layout)
        m_layout = createBookmarkFlowLayout();
    return m_layout;
}

void BookmarkFlowUI::addCustomFolders(services::SharedBookmarkFolderList folders)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    for (auto it = folders.begin(); it != folders.end(); ++it) {
        if ((*it)->getName().compare("All") == 0)
            continue;
        FolderData *folderData = new FolderData();
        folderData->name = (*it)->getName();
        folderData->folder_id = (*it)->getId();
        folderData->bookmarkFlowUI.reset(this);
#if PROFILE_MOBILE
        listAddCustomFolder(folderData);
#else
        gridAddCustomFolder(folderData);
#endif
    }

#if PROFILE_MOBILE  
    elm_object_part_content_set(m_contentsArea, "dropdown_swallow", m_genlist);
    evas_object_show(m_genlist);
#else
    if (elm_gengrid_items_count(m_gengrid) < 10)
        elm_object_signal_emit(m_layout, "upto9", "ui");
    if (elm_gengrid_items_count(m_gengrid) < 7)
        elm_object_signal_emit(m_layout, "upto6", "ui");

    elm_object_part_content_set(m_layout, "folder_grid_swallow", m_gengrid);
    evas_object_show(m_gengrid);
#endif
}

#if PROFILE_MOBILE
void BookmarkFlowUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_parent, "show_popup", "ui");
    evas_object_show(m_layout);
}

void BookmarkFlowUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_parent, "hide_popup", "ui");
    evas_object_hide(m_layout);
    evas_object_hide(m_genlist);
    elm_object_signal_emit(m_contentsArea, "dropdown_swallow_hide", "ui");
    evas_object_hide(elm_object_part_content_get(m_contentsArea, "dropdown_swallow"));
    elm_genlist_clear(m_genlist);
    m_map_folders.clear();
}

void BookmarkFlowUI::setState(bool state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_state = state;
    if (m_state) {
        elm_object_signal_emit(m_contentsArea, "show_remove", "ui");
        elm_object_signal_emit(m_titleArea, "title_text_edit", "ui");
        evas_object_show(m_removeButton);
    }
    else {
        elm_object_signal_emit(m_contentsArea, "hide_remove", "ui");
        elm_object_signal_emit(m_titleArea, "title_text_add", "ui");
        evas_object_hide(m_removeButton);
    }
}

void BookmarkFlowUI::setTitle(const std::string& title)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, title.c_str());
    elm_object_part_text_set(m_entry, "elm.text", title.c_str());

    if (title.empty()) {
        elm_object_disabled_set(m_saveButton, EINA_TRUE);
        elm_object_signal_emit(m_titleArea, "save_dissabled", "ui");
    }
    else {
        elm_object_disabled_set(m_saveButton, EINA_FALSE);
        elm_object_signal_emit(m_titleArea, "save_enabled", "ui");
    }
}

void BookmarkFlowUI::setURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, url.c_str());
    elm_object_part_text_set(m_contentsArea, "site_url_text", url.c_str());
}

void BookmarkFlowUI::setFolder(unsigned int folder_id, const std::string& folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_genlist_item_item_class_update(m_map_folders[m_folder_id], m_folder_custom_item_class);
    m_folder_id = folder_id;
    elm_object_signal_emit(m_contentsArea, (m_folder_id == m_special_folder_id)
                           ? "folder_icon_special" : "folder_icon_normal", "ui");
    elm_object_part_text_set(m_contentsArea, "dropdown_text", folder_name.c_str());
    elm_genlist_item_item_class_update(m_map_folders[m_folder_id], m_folder_selected_item_class);
}

void BookmarkFlowUI::setSpecialFolderId(unsigned int special)
{
    m_special_folder_id = special;
}

void BookmarkFlowUI::_save_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_UNUSED(data);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        BookmarkUpdate update;
        update.folder_id = bookmarkFlowUI->m_folder_id;
        update.title = elm_object_part_text_get(bookmarkFlowUI->m_entry, "elm.text");
        if (!bookmarkFlowUI->m_state)
            bookmarkFlowUI->saveBookmark(update);
        else
            bookmarkFlowUI->editBookmark(update);
        bookmarkFlowUI->closeBookmarkFlowClicked();
    }
}

void BookmarkFlowUI::_cancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->closeBookmarkFlowClicked();
    }
}

void BookmarkFlowUI::createContentsArea()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_contentsArea = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout, "contents_area_swallow", m_contentsArea);
    evas_object_size_hint_weight_set(m_contentsArea, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_contentsArea, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_contentsArea);

    elm_layout_file_set(m_contentsArea, m_edjFilePath.c_str(), "contents-area-layout");

    m_entry = elm_entry_add(m_contentsArea);
    elm_object_style_set(m_entry, "title_input_entry");

    elm_entry_single_line_set(m_entry, EINA_TRUE);
    elm_entry_scrollable_set(m_entry, EINA_TRUE);
    elm_entry_input_panel_layout_set(m_entry, ELM_INPUT_PANEL_LAYOUT_URL);
    elm_object_part_content_set(m_contentsArea, "title_input_swallow", m_entry);

    evas_object_smart_callback_add(m_entry, "focused", _entry_focused, this);
    evas_object_smart_callback_add(m_entry, "unfocused", _entry_unfocused, this);
    evas_object_smart_callback_add(m_entry, "changed,user",_entry_changed, this);

    m_inputCancelButton = elm_button_add(m_contentsArea);
    elm_object_style_set(m_inputCancelButton, "invisible_button");
    evas_object_smart_callback_add(m_inputCancelButton, "clicked", _inputCancel_clicked, this);
    evas_object_show(m_inputCancelButton);

    elm_object_part_content_set(m_contentsArea, "input_cancel_click", m_inputCancelButton);

    m_folderButton = elm_button_add(m_contentsArea);
    elm_object_style_set(m_folderButton, "invisible_button");
    evas_object_smart_callback_add(m_folderButton, "clicked", _folder_clicked, this);
    evas_object_show(m_inputCancelButton);

    elm_object_part_content_set(m_contentsArea, "folder_button_click", m_folderButton);

    m_folder_dropdown_button = elm_button_add(m_contentsArea);
    elm_object_style_set(m_folder_dropdown_button, "invisible_button");
    evas_object_smart_callback_add(m_folder_dropdown_button, "clicked", _folder_dropdown_clicked, this);
    evas_object_show(m_folder_dropdown_button);

    elm_object_part_content_set(m_contentsArea, "folder_dropdown_click", m_folder_dropdown_button);

    m_removeButton = elm_button_add(m_contentsArea);
    elm_object_style_set(m_removeButton, "invisible_button");
    evas_object_smart_callback_add(m_removeButton, "clicked", _remove_clicked, this);
    evas_object_show(m_removeButton);

    elm_object_part_content_set(m_contentsArea, "remove_click", m_removeButton);
}

void BookmarkFlowUI::createGenlistItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_folder_custom_item_class = elm_genlist_item_class_new();
    m_folder_custom_item_class->item_style = "folder-custom-item";
    m_folder_custom_item_class->func.text_get = _folder_title_text_get;
    m_folder_custom_item_class->func.content_get = nullptr;
    m_folder_custom_item_class->func.state_get = nullptr;
    m_folder_custom_item_class->func.del = nullptr;

    m_folder_selected_item_class = elm_genlist_item_class_new();
    m_folder_selected_item_class->item_style = "folder-selected-item";
    m_folder_selected_item_class->func.text_get = _folder_title_text_get;
    m_folder_selected_item_class->func.content_get = nullptr;
    m_folder_selected_item_class->func.state_get = nullptr;
    m_folder_selected_item_class->func.del = nullptr;
}

void BookmarkFlowUI::createGenlist()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
    m_genlist = elm_genlist_add(m_contentsArea);
    elm_object_part_content_set(m_contentsArea, "dropdown_swallow", m_genlist);
    elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_genlist, 0, 384*efl_scale);
    elm_genlist_homogeneous_set(m_genlist, EINA_FALSE);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);
    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genlist, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genlist, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

void BookmarkFlowUI::listAddCustomFolder(FolderData* item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* custom_folder = elm_genlist_item_append(m_genlist, item->folder_id == m_folder_id ?
                                                             m_folder_selected_item_class : m_folder_custom_item_class, item,
                                                             nullptr, ELM_GENLIST_ITEM_NONE, _listCustomFolderClicked, item);
    m_map_folders.insert(std::pair<unsigned int, Elm_Object_Item*>(item->folder_id, custom_folder));
    elm_genlist_item_selected_set(custom_folder, EINA_FALSE);
}

void BookmarkFlowUI::_listCustomFolderClicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        FolderData* folderData = static_cast<FolderData*>(data);
        elm_genlist_item_item_class_update(folderData->bookmarkFlowUI->m_map_folders[folderData->bookmarkFlowUI->m_folder_id],
                folderData->bookmarkFlowUI->m_folder_custom_item_class);
        folderData->bookmarkFlowUI->m_folder_id = folderData->folder_id;
        elm_object_part_text_set(folderData->bookmarkFlowUI->m_contentsArea,
                                 "dropdown_text", folderData->name.c_str());
        elm_object_signal_emit(folderData->bookmarkFlowUI->m_contentsArea,
                               (folderData->bookmarkFlowUI->m_folder_id == folderData->bookmarkFlowUI->m_special_folder_id)
                               ? "folder_icon_special" : "folder_icon_normal", "ui");
        elm_object_signal_emit(folderData->bookmarkFlowUI->m_contentsArea, "dropdown_swallow_hide", "ui");
        evas_object_hide(folderData->bookmarkFlowUI->m_genlist);
        evas_object_hide(elm_object_part_content_get(folderData->bookmarkFlowUI->m_contentsArea, "dropdown_swallow"));
        elm_genlist_item_item_class_update(folderData->bookmarkFlowUI->m_map_folders[folderData->bookmarkFlowUI->m_folder_id],
                folderData->bookmarkFlowUI->m_folder_selected_item_class);
    }
}

void BookmarkFlowUI::_entry_focused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_focus_allow_set(bookmarkFlowUI->m_inputCancelButton, EINA_TRUE);
        elm_object_signal_emit(bookmarkFlowUI->m_contentsArea, "entry_focused", "ui");
        elm_object_signal_emit(bookmarkFlowUI->m_entry, "focused", "ui");
    }
}

void BookmarkFlowUI::_entry_unfocused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_focus_allow_set(bookmarkFlowUI->m_inputCancelButton, EINA_FALSE);
        elm_object_signal_emit(bookmarkFlowUI->m_contentsArea, "entry_unfocused", "ui");
        elm_object_signal_emit(bookmarkFlowUI->m_entry, "unfocused", "ui");
    }
}

void BookmarkFlowUI::_entry_changed(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        std::string text = elm_object_part_text_get(bookmarkFlowUI->m_entry, "elm.text");
        if (text.empty()) {
            elm_object_disabled_set(bookmarkFlowUI->m_saveButton, EINA_TRUE);
            elm_object_signal_emit(bookmarkFlowUI->m_titleArea, "save_dissabled", "ui");
        } else {
            elm_object_disabled_set(bookmarkFlowUI->m_saveButton, EINA_FALSE);
            elm_object_signal_emit(bookmarkFlowUI->m_titleArea, "save_enabled", "ui");
        }
    }
}

void BookmarkFlowUI::_inputCancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_part_text_set(bookmarkFlowUI->m_entry, "elm.text", "");
        elm_object_disabled_set(bookmarkFlowUI->m_saveButton, EINA_TRUE);
        elm_object_signal_emit(bookmarkFlowUI->m_titleArea, "save_dissabled", "ui");
    }
}

void BookmarkFlowUI::_folder_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->addFolder();
    }
}

void BookmarkFlowUI::_folder_dropdown_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        if (evas_object_visible_get(bookmarkFlowUI->m_genlist) == EINA_FALSE) {
            unsigned int count = elm_genlist_items_count(bookmarkFlowUI->m_genlist);
            if (count > bookmarkFlowUI->m_max_items)
                count = bookmarkFlowUI->m_max_items;
            elm_object_signal_emit(bookmarkFlowUI->m_contentsArea, (boost::format("%s_%u") % "dropdown_swallow_show" % count).str().c_str(), "ui");
            evas_object_show(bookmarkFlowUI->m_genlist);
            evas_object_show(elm_object_part_content_get(bookmarkFlowUI->m_contentsArea,"dropdown_swallow"));
        } else {
            elm_object_signal_emit(bookmarkFlowUI->m_contentsArea, "dropdown_swallow_hide", "ui");
            evas_object_hide(bookmarkFlowUI->m_genlist);
            evas_object_hide(elm_object_part_content_get(bookmarkFlowUI->m_contentsArea, "dropdown_swallow"));
        }
    }
}

void BookmarkFlowUI::_remove_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->removeBookmark();
        bookmarkFlowUI->closeBookmarkFlowClicked();
    }
}
#else
BookmarkFlowUI* BookmarkFlowUI::createPopup(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkFlowUI *bookmarkFlow = new BookmarkFlowUI();
    bookmarkFlow->m_parent = parent;
    return bookmarkFlow;
}

void BookmarkFlowUI::show()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    createBookmarkFlowLayout();
    evas_object_show(m_layout);
    evas_object_show(m_gengrid);
    m_focusManager.startFocusManager(m_gengrid);
    popupShown(this);
}

void BookmarkFlowUI::dismiss()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_focusManager.stopFocusManager();
    popupDismissed(this);
}

void BookmarkFlowUI::onBackPressed()
{
    dismiss();
}

void BookmarkFlowUI::createGengridItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_folder_new_item_class = elm_gengrid_item_class_new();
    m_folder_new_item_class->item_style = "folder-new-item";
    m_folder_new_item_class->func.text_get = nullptr;
    m_folder_new_item_class->func.content_get =  nullptr;
    m_folder_new_item_class->func.state_get = nullptr;
    m_folder_new_item_class->func.del = nullptr;

    m_folder_custom_item_class = elm_gengrid_item_class_new();
    m_folder_custom_item_class->item_style = "folder-custom-item";
    m_folder_custom_item_class->func.text_get = _folder_title_text_get;
    m_folder_custom_item_class->func.content_get =  nullptr;
    m_folder_custom_item_class->func.state_get = nullptr;
    m_folder_custom_item_class->func.del = nullptr;
}

void BookmarkFlowUI::createGengrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);
    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();

    m_gengrid = elm_gengrid_add(m_layout);
    elm_object_part_content_set(m_layout, "folder_grid_swallow", m_gengrid);
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
    elm_scroller_page_size_set(m_gengrid, 0, 750);
    elm_gengrid_item_size_set(m_gengrid, (208+18) * efl_scale, (208+18) * efl_scale);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(m_gengrid);
}

void BookmarkFlowUI::addNewFolder()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* new_folder = elm_gengrid_item_append(m_gengrid, m_folder_new_item_class,
                                                            NULL, _gridNewFolderClicked, this);
    elm_gengrid_item_selected_set(new_folder, EINA_FALSE);
}

void BookmarkFlowUI::_gridNewFolderClicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->addFolder();
        bookmarkFlowUI->dismiss();
    }
}

void BookmarkFlowUI::gridAddCustomFolder(FolderData* item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* custom_folder = elm_gengrid_item_append(m_gengrid, m_folder_custom_item_class,
                                                            item, _gridCustomFolderClicked, item);
    elm_gengrid_item_selected_set(custom_folder, EINA_FALSE);
}

void BookmarkFlowUI::_gridCustomFolderClicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        FolderData* folderData = static_cast<FolderData*>(data);
        BookmarkUpdate update;
        update.folder_id = folderData->folder_id;
        folderData->bookmarkFlowUI->saveBookmark(update);
        folderData->bookmarkFlowUI->dismiss();
    }
}

void BookmarkFlowUI::_bg_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->dismiss();
    }
}

void BookmarkFlowUI::createFocusVector()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_focusManager.addItem(m_gengrid);
    m_focusManager.setIterator();
}
#endif


char* BookmarkFlowUI::_folder_title_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr)) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        FolderData *folderData = static_cast<FolderData*>(data);
        const char *folder_text = "folder_text";
        if (!strncmp(folder_text, part, strlen(folder_text)))
            return strdup(folderData->name.c_str());
    }
    return strdup("");
}

Evas_Object* BookmarkFlowUI::createBookmarkFlowLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_layout = elm_layout_add(m_parent);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "bookmarkflow-layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    createTitleArea();
#if PROFILE_MOBILE
    createContentsArea();
    createGenlist();
#else
    m_bg = elm_button_add(m_layout);
    elm_object_style_set(m_bg, "invisible_button");
    evas_object_smart_callback_add(m_bg, "clicked", _bg_clicked, this);
    elm_object_part_content_set(m_layout, "bg_click", m_bg);
    createGengrid();
    createFocusVector();
#endif

    evas_object_show(m_layout);
    return m_layout;
}

void BookmarkFlowUI::createTitleArea()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_titleArea = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout, "title_area_swallow", m_titleArea);
    evas_object_size_hint_weight_set(m_titleArea, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_titleArea, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_titleArea);

    elm_layout_file_set(m_titleArea, m_edjFilePath.c_str(), "title-area-layout");
#if PROFILE_MOBILE
    m_saveButton = elm_button_add(m_titleArea);
    elm_object_style_set(m_saveButton, "invisible_button");
    evas_object_smart_callback_add(m_saveButton, "clicked", _save_clicked, this);
    evas_object_show(m_saveButton);

    elm_object_part_content_set(m_titleArea, "save_click", m_saveButton);

    m_cancelButton = elm_button_add(m_titleArea);
    elm_object_style_set(m_cancelButton, "invisible_button");
    evas_object_smart_callback_add(m_cancelButton, "clicked", _cancel_clicked, this);
    evas_object_show(m_cancelButton);

    elm_object_part_content_set(m_titleArea, "cancel_click", m_cancelButton);
#endif
}

}
}
