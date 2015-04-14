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

#include "SearchBox.h"
#include "BrowserLogger.h"

namespace tizen_browser
{
namespace base_ui
{

SearchBox::SearchBox(Evas_Object *parent)
{
    std::string edjFilePath = EDJE_DIR;
    edjFilePath.append("SimpleUI/SearchBox.edj");
    elm_theme_extension_add(NULL, edjFilePath.c_str());

    m_entry_layout = elm_layout_add(parent);
    if(!elm_layout_file_set(m_entry_layout, edjFilePath.c_str(), "search_box"))
        throw std::runtime_error("Layout file not found: " + edjFilePath);

    m_webSearchEntry = elm_entry_add(m_entry_layout);

    elm_object_style_set(m_webSearchEntry, "search_entry");

    elm_entry_single_line_set(m_webSearchEntry, EINA_TRUE);
    elm_entry_scrollable_set(m_webSearchEntry, EINA_TRUE);

    search_caseSensitive = false;

    elm_object_translatable_part_text_set(m_webSearchEntry, "elm.guide", "Search");

    evas_object_smart_callback_add(m_webSearchEntry, "changed,user", SearchBox::searchNext, this);

    //Add Case sensitive checkbox
    m_caseSensitive = elm_check_add(m_entry_layout);
    elm_object_part_text_set(m_caseSensitive, "default", "Match case");
    elm_object_part_text_set(m_caseSensitive, "on", "Match case");
    elm_object_part_text_set(m_caseSensitive, "off", "Match case");
    elm_check_state_set(m_caseSensitive, EINA_FALSE);
    elm_object_part_content_set(m_entry_layout, "search_opts_case", m_caseSensitive);

    //Add prev and next buttons
    m_searchPrev = elm_button_add(m_entry_layout);
    elm_object_style_set(m_searchPrev, "default_button");
    elm_object_part_text_set(m_searchPrev, "default", "Prev");
    elm_object_part_content_set(m_entry_layout, "search_opts_prev", m_searchPrev);

    m_searchNext = elm_button_add(m_entry_layout);
    elm_object_style_set(m_searchNext, "default_button");
    elm_object_part_text_set(m_searchNext, "default", "Next");
    elm_object_part_content_set(m_entry_layout, "search_opts_next", m_searchNext);

    evas_object_smart_callback_add(m_searchNext, "clicked", SearchBox::searchNext, this);
    evas_object_smart_callback_add(m_searchPrev, "clicked", SearchBox::searchPrev, this);
    evas_object_smart_callback_add(m_caseSensitive, "changed", SearchBox::caseSensitiveChanged, this);

    elm_object_part_content_set(m_entry_layout, "search_entry_swallow", m_webSearchEntry);

    hide();
}

Evas_Object* SearchBox::getContent()
{
    return m_entry_layout;
}

void SearchBox::hide()
{
    elm_object_signal_emit(m_entry_layout, "elm,state,hide", "elm");
    elm_object_signal_emit(m_searchNext, "elm,state,hide", "elm");
    elm_object_signal_emit(m_searchPrev, "elm,state,hide", "elm");
    elm_object_signal_emit(m_webSearchEntry, "elm,state,hide", "elm");
    elm_entry_entry_set(m_webSearchEntry, "");
//    evas_object_hide(m_caseSensitive);
    visible = false;
}

void SearchBox::show()
{
    elm_object_signal_emit(m_entry_layout, "elm,state,show", "elm");
    elm_object_signal_emit(m_searchNext, "elm,state,show", "elm");
    elm_object_signal_emit(m_searchPrev, "elm,state,show", "elm");
    elm_object_signal_emit(m_webSearchEntry, "elm,state,show", "elm");
//    evas_object_show(m_caseSensitive);
    visible = true;
}

bool SearchBox::is_visible()
{
    return visible;
}

void SearchBox::searchNext(void *data, Evas_Object */*obj*/, void */*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SearchBox *self = reinterpret_cast<SearchBox*>(data);
    std::string entry(elm_entry_markup_to_utf8(elm_entry_entry_get(self->m_webSearchEntry)));

    if (self->search_caseSensitive)
        self->textChanged(entry, EWK_FIND_OPTIONS_SHOW_HIGHLIGHT);
    else
        self->textChanged(entry, EWK_FIND_OPTIONS_SHOW_HIGHLIGHT | EWK_FIND_OPTIONS_CASE_INSENSITIVE);

}

void SearchBox::searchPrev(void *data, Evas_Object */*obj*/, void */*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SearchBox *self = reinterpret_cast<SearchBox*>(data);
    std::string entry(elm_entry_markup_to_utf8(elm_entry_entry_get(self->m_webSearchEntry)));

    if (self->search_caseSensitive)
        self->textChanged(entry, EWK_FIND_OPTIONS_SHOW_HIGHLIGHT | EWK_FIND_OPTIONS_BACKWARDS);
    else
        self->textChanged(entry, EWK_FIND_OPTIONS_SHOW_HIGHLIGHT | EWK_FIND_OPTIONS_CASE_INSENSITIVE | EWK_FIND_OPTIONS_BACKWARDS);

}

void SearchBox::caseSensitiveChanged(void *data, Evas_Object */*obj*/, void */*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SearchBox *self = reinterpret_cast<SearchBox*>(data);
    std::string entry(elm_entry_markup_to_utf8(elm_entry_entry_get(self->m_webSearchEntry)));
    self->search_caseSensitive = elm_check_state_get(self->m_caseSensitive);

    if (self->search_caseSensitive)
        self->textChanged(entry, EWK_FIND_OPTIONS_SHOW_HIGHLIGHT);
    else
        self->textChanged(entry, EWK_FIND_OPTIONS_SHOW_HIGHLIGHT | EWK_FIND_OPTIONS_CASE_INSENSITIVE);
}


}

}

