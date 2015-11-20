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
    : m_bf_layout(nullptr)
    , m_parent(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("BookmarkFlowUI/BookmarkFlowUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

BookmarkFlowUI::~BookmarkFlowUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void BookmarkFlowUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

void BookmarkFlowUI::showUI()
{
    evas_object_show(m_bf_layout);
}

void BookmarkFlowUI::hideUI()
{
    evas_object_hide(m_bf_layout);
}

Evas_Object* BookmarkFlowUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_bf_layout)
      m_bf_layout = createBookmarkFlowLayout(m_parent);
    return m_bf_layout;
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

Evas_Object* BookmarkFlowUI::createBookmarkFlowLayout(Evas_Object* parent)
{
    m_bf_layout = elm_layout_add(parent);
    elm_layout_file_set(m_bf_layout, m_edjFilePath.c_str(), "bookmarkflow-layout");
    evas_object_size_hint_weight_set(m_bf_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_bf_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_bf_layout);

    createTitleArea();
    createContentsArea();

    return m_bf_layout;
}

void BookmarkFlowUI::createTitleArea()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_bf_layout);

    m_titleArea = elm_layout_add(m_bf_layout);
    elm_object_part_content_set(m_bf_layout, "title_area_swallow", m_titleArea);
    evas_object_size_hint_weight_set(m_titleArea, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_titleArea, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_titleArea);

    elm_layout_file_set(m_titleArea, m_edjFilePath.c_str(), "title_area-layout");

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
}

void BookmarkFlowUI::_save_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    M_UNUSED(data);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        BookmarkUpdate update;
        update.folder_id = 0;
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
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data != nullptr) {
        BookmarkFlowUI* bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->closeBookmarkFlowClicked();
    }
}

void BookmarkFlowUI::createContentsArea()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_bf_layout);

    m_contentsArea = elm_layout_add(m_bf_layout);
    elm_object_part_content_set(m_bf_layout, "contents_area_swallow", m_contentsArea);
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

    m_removeButton = elm_button_add(m_contentsArea);
    elm_object_style_set(m_removeButton, "invisible_button");
    evas_object_smart_callback_add(m_removeButton, "clicked", _remove_clicked, this);
    evas_object_show(m_removeButton);

    elm_object_part_content_set(m_contentsArea, "remove_click", m_removeButton);
}

void BookmarkFlowUI::_entry_focused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_focus_allow_set(bookmarkFlowUI->m_inputCancelButton, EINA_TRUE);
        elm_object_signal_emit(bookmarkFlowUI->m_contentsArea, "entry_focused", "ui");
        elm_object_signal_emit(bookmarkFlowUI->m_entry, "focused", "ui");
    }
}

void BookmarkFlowUI::_entry_unfocused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_focus_allow_set(bookmarkFlowUI->m_inputCancelButton, EINA_FALSE);
        elm_object_signal_emit(bookmarkFlowUI->m_contentsArea, "entry_unfocused", "ui");
        elm_object_signal_emit(bookmarkFlowUI->m_entry, "unfocused", "ui");
    }
}

void BookmarkFlowUI::_entry_changed(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        std::string text = elm_object_part_text_get(bookmarkFlowUI->m_entry, "elm.text");
        if (text.empty()) {
            elm_object_disabled_set(bookmarkFlowUI->m_saveButton, EINA_TRUE);
            elm_object_signal_emit(bookmarkFlowUI->m_titleArea, "save_dissabled", "ui");
        }
        else {
            elm_object_disabled_set(bookmarkFlowUI->m_saveButton, EINA_FALSE);
            elm_object_signal_emit(bookmarkFlowUI->m_titleArea, "save_enabled", "ui");
        }
    }
}

void BookmarkFlowUI::_inputCancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        elm_object_part_text_set(bookmarkFlowUI->m_entry, "elm.text", "");
        elm_object_disabled_set(bookmarkFlowUI->m_saveButton, EINA_TRUE);
        elm_object_signal_emit(bookmarkFlowUI->m_titleArea, "save_dissabled", "ui");
    }
}

void BookmarkFlowUI::_folder_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    M_UNUSED(data);
}

void BookmarkFlowUI::_remove_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data != nullptr) {
        BookmarkFlowUI*  bookmarkFlowUI = static_cast<BookmarkFlowUI*>(data);
        bookmarkFlowUI->removeBookmark();
        bookmarkFlowUI->closeBookmarkFlowClicked();
    }
}

}
}
