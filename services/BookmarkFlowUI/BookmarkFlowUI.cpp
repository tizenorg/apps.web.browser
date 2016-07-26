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
    , m_title_area(nullptr)
    , m_box(nullptr)
    , m_entry_layout(nullptr)
    , m_contents_area(nullptr)
    , m_remove_button(nullptr)
    , m_entry(nullptr)
    , m_done_button(nullptr)
    , m_cancel_button(nullptr)
    , m_input_cancel_button(nullptr)
    , m_folder_button(nullptr)
    , m_add_to_qa(false)
    //TODO: Asign nullptr to genlist_item_classes
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("BookmarkFlowUI/BookmarkFlowUI.edj");
    createGenlistItemClasses();
}

BookmarkFlowUI::~BookmarkFlowUI()
{
    evas_object_smart_callback_del(m_done_button, "clicked", _done_clicked);
    evas_object_smart_callback_del(m_cancel_button, "clicked", _cancel_clicked);
    evas_object_smart_callback_del(m_entry, "focused", _entry_focused);
    evas_object_smart_callback_del(m_entry, "unfocused", _entry_unfocused);
    evas_object_smart_callback_del(m_entry, "changed,user", _entry_changed);
    evas_object_smart_callback_del(m_input_cancel_button, "clicked", _input_cancel_clicked);
    evas_object_smart_callback_del(m_folder_button, "clicked", _folder_clicked);
    evas_object_smart_callback_del(m_remove_button, "clicked", _remove_clicked);

    evas_object_del(m_remove_button);
    evas_object_del(m_entry);
    evas_object_del(m_done_button);
    evas_object_del(m_save);
    evas_object_del(m_save_box);
    evas_object_del(m_cancel_button);
    evas_object_del(m_cancel);
    evas_object_del(m_cancel_box);
    evas_object_del(m_input_cancel_button);
    evas_object_del(m_folder_button);
    evas_object_del(m_contents_area);

    //TODO: add other destructors
    if (m_entry_item_class)
        elm_genlist_item_class_free(m_entry_item_class);
    evas_object_del(m_title_area);
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

void BookmarkFlowUI::addCustomFolders(services::SharedBookmarkFolderList /*folders*/)
{
/*    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    for (auto it = folders.begin(); it != folders.end(); ++it) {
        if ((*it)->getId() == m_all_folder_id)
            continue;
        FolderData *folderData = new FolderData();
        folderData->name = (*it)->getName();
        folderData->folder_id = (*it)->getId();
        folderData->bookmarkFlowUI.reset(this);
        listAddCustomFolder(folderData);
    }

    elm_object_part_content_set(m_contents_area, "dropdown_swallow", m_genlist);
    evas_object_show(m_genlist);
    elm_object_item_signal_emit(elm_genlist_last_item_get(m_genlist), "invisible", "ui");*/
}

void BookmarkFlowUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_parent, "show_popup", "ui");
    evas_object_show(m_layout);
    //elm_object_signal_emit(m_contents_area, "close_icon_show", "ui");
    resetContent();
}

void BookmarkFlowUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_parent, "hide_popup", "ui");
    evas_object_hide(m_layout);
    //evas_object_hide(m_genlist);
    //elm_object_signal_emit(m_contents_area, "dropdown_swallow_hide", "ui");
    //evas_object_hide(elm_object_part_content_get(m_contents_area, "dropdown_swallow"));
    //m_map_folders.clear();
}

void BookmarkFlowUI::setState(bool state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_state = state;
    if (m_state)
        elm_object_translatable_part_text_set(m_layout, "elm.text.title", _("IDS_BR_HEADER_EDIT_BOOKMARK"));
    else
        elm_object_translatable_part_text_set(m_layout, "elm.text.title", _("IDS_BR_OPT_ADD_BOOKMARK"));
}

void BookmarkFlowUI::setTitle(const std::string& title)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, title.c_str());
    elm_object_part_text_set(m_entry, "elm.text", elm_entry_utf8_to_markup(title.c_str()));

    if (title.empty()) {
        //elm_object_disabled_set(m_done_button, EINA_TRUE);
        elm_object_signal_emit(m_done_button, "elm,state,disabled", "elm");
    } else {
        //elm_object_disabled_set(m_done_button, EINA_FALSE);
        elm_object_signal_emit(m_done_button, "elm,state,enabled", "elm");
    }
}

void BookmarkFlowUI::setURL(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] %s", __PRETTY_FUNCTION__, __LINE__, url.c_str());
    elm_object_part_text_set(m_contents_area, "site_url_text", url.c_str());
}

void BookmarkFlowUI::setFolder(unsigned int /*folder_id*/, const std::string& /*folder_name*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
//    elm_genlist_item_item_class_update(m_map_folders[m_folder_id], m_folder_custom_item_class);
//    m_folder_id = folder_id;
//    elm_object_signal_emit(m_contents_area, (m_folder_id == m_special_folder_id)
//                           ? "folder_icon_special" : "folder_icon_normal", "ui");
//    elm_object_part_text_set(m_contents_area, "dropdown_text", elm_entry_utf8_to_markup(folder_name.c_str()));
//    elm_genlist_item_item_class_update(m_map_folders[m_folder_id], m_folder_selected_item_class);
}

void BookmarkFlowUI::setFoldersId(unsigned int all, unsigned int special)
{
    m_all_folder_id = all;
    m_special_folder_id = special;
}

void BookmarkFlowUI::resetContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> rotated = isRotated();
    if (rotated) {
        if (*rotated) {
//            elm_scroller_page_size_set(m_genlist, 0, ELM_SCALE_SIZE(GENLIST_HEIGHT_LANDSCAPE));
            m_max_items = MAX_ITEMS_LANDSCAPE;
            elm_object_signal_emit(m_contents_area, "dropdown_text_landscape", "ui");
        } else {
//            elm_scroller_page_size_set(m_genlist, 0, ELM_SCALE_SIZE(GENLIST_HEIGHT));
            m_max_items = MAX_ITEMS;
            elm_object_signal_emit(m_contents_area, "dropdown_text_portrait", "ui");
        }
//        if (evas_object_visible_get(m_genlist) == EINA_TRUE) {
//            unsigned int count = elm_genlist_items_count(m_genlist);
//            if (count > m_max_items)
//                count = m_max_items;
//            elm_object_signal_emit(m_contents_area, (boost::format("%s_%u") % "dropdown_swallow_show" % count).str().c_str(), "ui");
//            evas_object_show(m_genlist);
//            evas_object_show(elm_object_part_content_get(m_contents_area, "dropdown_swallow"));
//        }
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}

void BookmarkFlowUI::_done_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_UNUSED(data);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        BookmarkUpdate update;
        update.folder_id = 0;
        //TODO: change bookmarkFlowUI->m_folder_id;
        update.title = elm_entry_markup_to_utf8(elm_object_part_text_get(bookmarkFlowUI->m_entry, "elm.text"));
        if (!bookmarkFlowUI->m_state)
            bookmarkFlowUI->saveBookmark(update);
        else
            bookmarkFlowUI->editBookmark(update);
        bookmarkFlowUI->closeBookmarkFlowClicked();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_cancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->closeBookmarkFlowClicked();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::createContentsArea()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_genlist = elm_genlist_add(m_layout);
    elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_genlist_homogeneous_set(m_genlist, EINA_FALSE);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);
    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);
    evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_part_content_set(m_layout, "elm.swallow.content", m_genlist);
    evas_object_show(m_genlist);

    elm_genlist_item_append(m_genlist, m_entry_item_class, this, nullptr, ELM_GENLIST_ITEM_NONE, nullptr, this);

    elm_genlist_item_append(m_genlist, m_group_item_class, _("IDS_BR_BODY_LOCATION_M_INFORMATION"), nullptr,
        ELM_GENLIST_ITEM_NONE, nullptr, _("IDS_BR_BODY_LOCATION_M_INFORMATION"));

    FolderData *folderData = new FolderData();
    folderData->name = "Bookmarks";
    folderData->folder_id = 0;
    folderData->bookmarkFlowUI.reset(this);

    elm_genlist_item_append(m_genlist, m_folder_item_class, folderData, nullptr, ELM_GENLIST_ITEM_NONE,
        _folder_selector_clicked, folderData);

    elm_genlist_item_append(m_genlist, m_add_to_qa_item_class, this, nullptr, ELM_GENLIST_ITEM_NONE,
        _qa_clicked, this);
}

void BookmarkFlowUI::_folder_selector_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        //BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        //TODO: Add selector functionality
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_qa_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->m_add_to_qa = !(elm_check_state_get(bookmarkFlowUI->m_qa_checkbox) == EINA_TRUE);
        elm_check_state_set(bookmarkFlowUI->m_qa_checkbox, bookmarkFlowUI->m_add_to_qa ? EINA_TRUE : EINA_FALSE);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::createGenlistItemClasses()
{
    //TODO: Create function to remove code duplicates
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_entry_item_class = elm_genlist_item_class_new();
    m_entry_item_class->item_style = "full";
    m_entry_item_class->func.text_get = nullptr;
    m_entry_item_class->func.content_get = _genlist_entry_content_get;
    m_entry_item_class->func.state_get = nullptr;
    m_entry_item_class->func.del = nullptr;
    m_entry_item_class->decorate_all_item_style = "edit_default";

    m_group_item_class = elm_genlist_item_class_new();
    m_group_item_class->item_style = "group_index";
    m_group_item_class->func.text_get = _genlist_text_get;
    m_group_item_class->func.content_get = nullptr;
    m_group_item_class->func.state_get = nullptr;
    m_group_item_class->func.del = nullptr;
    m_group_item_class->decorate_all_item_style = "edit_default";

    m_folder_item_class = elm_genlist_item_class_new();
    m_folder_item_class->item_style = "type1";
    m_folder_item_class->func.text_get = _genlist_folder_text_get;
    m_folder_item_class->func.content_get =  _genlist_folder_content_get;
    m_folder_item_class->func.state_get = nullptr;
    m_folder_item_class->func.del = nullptr;
    m_folder_item_class->decorate_all_item_style = "edit_default";

    m_add_to_qa_item_class = elm_genlist_item_class_new();
    m_add_to_qa_item_class->item_style = "type1";
    m_add_to_qa_item_class->func.text_get = _genlist_add_to_qa_text_get;
    m_add_to_qa_item_class->func.content_get =  _genlist_add_to_qa_content_get;
    m_add_to_qa_item_class->func.state_get = nullptr;
    m_add_to_qa_item_class->func.del = nullptr;
    m_add_to_qa_item_class->decorate_all_item_style = "edit_default";
}

void BookmarkFlowUI::createGenlist()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_genlist = elm_genlist_add(m_contents_area);
    elm_object_part_content_set(m_contents_area, "dropdown_swallow", m_genlist);
    elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_genlist_homogeneous_set(m_genlist, EINA_TRUE);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);
    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);
    evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

void BookmarkFlowUI::listAddCustomFolder(FolderData* /*item*/)
{
//    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
//    Elm_Object_Item* custom_folder = elm_genlist_item_append(m_genlist, item->folder_id == m_folder_id ?
//                                                             m_folder_selected_item_class : m_folder_custom_item_class, item,
//                                                             nullptr, ELM_GENLIST_ITEM_NONE, _listCustomFolderClicked, item);
//    m_map_folders.insert(std::pair<unsigned int, Elm_Object_Item*>(item->folder_id, custom_folder));
//    elm_genlist_item_selected_set(custom_folder, EINA_FALSE);
}

void BookmarkFlowUI::_listCustomFolderClicked(void */*data*/, Evas_Object *, void *)
{
//    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
//    if (data != nullptr) {
//        FolderData* folderData = static_cast<FolderData*>(data);
//        elm_genlist_item_item_class_update(folderData->bookmarkFlowUI->m_map_folders[folderData->bookmarkFlowUI->m_folder_id],
//                folderData->bookmarkFlowUI->m_folder_custom_item_class);
//        folderData->bookmarkFlowUI->m_folder_id = folderData->folder_id;
//        elm_object_part_text_set(folderData->bookmarkFlowUI->m_contents_area,
//                                 "dropdown_text", elm_entry_utf8_to_markup(folderData->name.c_str()));
//        elm_object_signal_emit(folderData->bookmarkFlowUI->m_contents_area,
//                               (folderData->bookmarkFlowUI->m_folder_id == folderData->bookmarkFlowUI->m_special_folder_id)
//                               ? "folder_icon_special" : "folder_icon_normal", "ui");
//        elm_object_signal_emit(folderData->bookmarkFlowUI->m_contents_area, "dropdown_swallow_hide", "ui");
//        evas_object_hide(folderData->bookmarkFlowUI->m_genlist);
//        evas_object_hide(elm_object_part_content_get(folderData->bookmarkFlowUI->m_contents_area, "dropdown_swallow"));
//        elm_genlist_item_item_class_update(folderData->bookmarkFlowUI->m_map_folders[folderData->bookmarkFlowUI->m_folder_id],
//                folderData->bookmarkFlowUI->m_folder_selected_item_class);
//    } else
//        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_entry_focused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_signal_emit(bookmarkFlowUI->m_entry_layout, "elm,state,focused", "");
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_entry_unfocused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_signal_emit(bookmarkFlowUI->m_entry_layout, "elm,state,unfocused", "");
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_entry_changed(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        if (elm_entry_is_empty(bookmarkFlowUI->m_entry)) {
            elm_object_signal_emit(bookmarkFlowUI->m_done_button, "elm,state,disabled", "elm");
            elm_object_signal_emit(bookmarkFlowUI->m_entry_layout, "elm,action,hide,button", "");
        } else {
            elm_object_signal_emit(bookmarkFlowUI->m_done_button, "elm,state,enabled", "elm");
            elm_object_signal_emit(bookmarkFlowUI->m_entry_layout, "elm,action,show,button", "");
        }
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_input_cancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_entry_entry_set(bookmarkFlowUI->m_entry, "");
        elm_object_focus_set(bookmarkFlowUI->m_entry, EINA_TRUE);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_folder_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->addFolder(0);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_folder_dropdown_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        if (evas_object_visible_get(bookmarkFlowUI->m_genlist) == EINA_FALSE) {
            unsigned int count = elm_genlist_items_count(bookmarkFlowUI->m_genlist);
            if (count > bookmarkFlowUI->m_max_items)
                count = bookmarkFlowUI->m_max_items;
            elm_object_signal_emit(bookmarkFlowUI->m_contents_area, (boost::format("%s_%u") % "dropdown_swallow_show" % count).str().c_str(), "ui");
            evas_object_show(bookmarkFlowUI->m_genlist);
            evas_object_show(elm_object_part_content_get(bookmarkFlowUI->m_contents_area,"dropdown_swallow"));
        } else {
            elm_object_signal_emit(bookmarkFlowUI->m_contents_area, "dropdown_swallow_hide", "ui");
            evas_object_hide(bookmarkFlowUI->m_genlist);
            evas_object_hide(elm_object_part_content_get(bookmarkFlowUI->m_contents_area, "dropdown_swallow"));
        }
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkFlowUI::_remove_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->removeBookmark();
        bookmarkFlowUI->closeBookmarkFlowClicked();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

char* BookmarkFlowUI::_genlist_entry_text_get(void *data, Evas_Object *, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if ((data != nullptr) && (part != nullptr)) {
        if (!strcmp(part, "elm.text"))
            return strdup("elm.text");
//        if (!strcmp(part, "elm.text.multiline"))
//            return strdup("elm.text.multiline");
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

Evas_Object *BookmarkFlowUI::_genlist_entry_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if ((data != nullptr) && (part != nullptr)) {
        BookmarkFlowUI *bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        if (!strcmp(part, "elm.swallow.content")) {
            Evas_Object* layout = elm_layout_add(obj);
            elm_layout_file_set(layout, bookmarkFlowUI->m_edjFilePath.c_str(), "genlist_entry_custom_layout");
            evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

            bookmarkFlowUI->m_entry_layout = elm_layout_add(obj);
            elm_layout_theme_set(bookmarkFlowUI->m_entry_layout, "layout", "editfield", "multiline");
            evas_object_size_hint_weight_set(bookmarkFlowUI->m_entry_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(bookmarkFlowUI->m_entry_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

            bookmarkFlowUI->m_entry = elm_entry_add(obj);
            elm_entry_single_line_set(bookmarkFlowUI->m_entry, EINA_TRUE);
            elm_entry_scrollable_set(bookmarkFlowUI->m_entry, EINA_TRUE);
            evas_object_size_hint_weight_set(bookmarkFlowUI->m_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(bookmarkFlowUI->m_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);

            evas_object_smart_callback_add(bookmarkFlowUI->m_entry, "changed", _entry_changed, bookmarkFlowUI);
            evas_object_smart_callback_add(bookmarkFlowUI->m_entry, "focused", _entry_focused, bookmarkFlowUI);
            evas_object_smart_callback_add(bookmarkFlowUI->m_entry, "unfocused", _entry_unfocused, bookmarkFlowUI);

            elm_object_part_content_set(bookmarkFlowUI->m_entry_layout, "elm.swallow.content", bookmarkFlowUI->m_entry);

            bookmarkFlowUI->m_input_cancel_button = elm_button_add(obj);
            elm_object_style_set(bookmarkFlowUI->m_input_cancel_button, "editfield_clear");
            elm_object_focus_allow_set(bookmarkFlowUI->m_input_cancel_button, EINA_FALSE);

            evas_object_smart_callback_add(bookmarkFlowUI->m_input_cancel_button, "clicked", _input_cancel_clicked, bookmarkFlowUI);

            elm_object_part_content_set(bookmarkFlowUI->m_entry_layout, "elm.swallow.button", bookmarkFlowUI->m_input_cancel_button);

            elm_object_part_text_set(layout, "elm.text", "DUPA");
            elm_object_part_content_set(layout, "elm.swallow.content", bookmarkFlowUI->m_entry_layout);

            return layout;
        }
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

char* BookmarkFlowUI::_genlist_text_get(void *data, Evas_Object *, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if ((data != nullptr) && (part != nullptr)) {
        if (!strcmp(part, "elm.text"))
            return strdup(elm_entry_utf8_to_markup(static_cast<char*>(data)));
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

char* BookmarkFlowUI::_genlist_folder_text_get(void *data, Evas_Object *, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        FolderData *folderData = static_cast<FolderData*>(data);
        if (!strcmp(part, "elm.text"))
            return strdup(elm_entry_utf8_to_markup(folderData->name.c_str()));
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

Evas_Object *BookmarkFlowUI::_genlist_folder_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        FolderData *folderData = static_cast<FolderData*>(data);
        if (!strcmp(part, "elm.swallow.icon")) {
            Evas_Object* layout = elm_layout_add(obj);
            elm_layout_file_set(layout, folderData->bookmarkFlowUI->m_edjFilePath.c_str(), "folder_image");
            return layout;
        }
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

char* BookmarkFlowUI::_genlist_add_to_qa_text_get(void *, Evas_Object *, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (part) {
        if (!strcmp(part, "elm.text"))
            return strdup(elm_entry_utf8_to_markup(_("IDS_BR_OPT_ADD_TO_QUICK_ACCESS")));
    } else
        BROWSER_LOGE("[%s:%d] Part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

Evas_Object *BookmarkFlowUI::_genlist_add_to_qa_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        BookmarkFlowUI *bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        if (!strcmp(part, "elm.swallow.end")) {
            bookmarkFlowUI->m_qa_checkbox = elm_check_add(obj);
            evas_object_propagate_events_set(bookmarkFlowUI->m_qa_checkbox, EINA_FALSE);
            elm_check_state_set(bookmarkFlowUI->m_qa_checkbox, bookmarkFlowUI->m_add_to_qa ? EINA_TRUE : EINA_FALSE);
            evas_object_smart_callback_add(bookmarkFlowUI->m_qa_checkbox, "changed", _add_to_qa_state_changed, bookmarkFlowUI);
            evas_object_show(bookmarkFlowUI->m_qa_checkbox);
            return bookmarkFlowUI->m_qa_checkbox;
        }
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

void BookmarkFlowUI::_add_to_qa_state_changed(void *data, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        BookmarkFlowUI *bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->m_add_to_qa = elm_check_state_get(obj) == EINA_TRUE;
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

Evas_Object* BookmarkFlowUI::createBookmarkFlowLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_layout = elm_layout_add(m_parent);
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_layout);
    elm_layout_theme_set(m_layout, "naviframe", "item/basic", "default");

    createTopContent();
    createContentsArea();
//    createGenlist();

    return m_layout;
}

void BookmarkFlowUI::createTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_layout);

    m_cancel_button = elm_button_add(m_layout);
    elm_object_part_content_set(m_layout, "title_left_btn", m_cancel_button);
    elm_object_style_set(m_cancel_button, "naviframe/title_left");
    elm_object_text_set(m_cancel_button, _("IDS_TPLATFORM_ACBUTTON_CANCEL_ABB"));
    evas_object_smart_callback_add(m_cancel_button, "clicked", _cancel_clicked, this);
    evas_object_size_hint_weight_set(m_cancel_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_cancel_button, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_layout, "elm,state,title_left_btn,show", "elm");

    m_done_button = elm_button_add(m_layout);
    elm_object_part_content_set(m_layout, "title_right_btn", m_done_button);
    elm_object_style_set(m_done_button, "naviframe/title_right");
    elm_object_text_set(m_done_button, _("IDS_TPLATFORM_ACBUTTON_DONE_ABB"));
    evas_object_smart_callback_add(m_done_button, "clicked", _done_clicked, this);
    evas_object_size_hint_weight_set(m_done_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_done_button, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_signal_emit(m_layout, "elm,state,title_right_btn,show", "elm");
}

}
}
