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
#include <Evas.h>
#include "URIEntry.h"
#include "BrowserLogger.h"
#include "MenuButton.h"
#include <algorithm>
#include <boost/regex.hpp>
#include "BrowserAssert.h"

namespace tizen_browser {
namespace base_ui {

#define GUIDE_TEXT_FOCUSED "Search or URL"
#define GUIDE_TEXT_UNFOCUSED "Search or URL - Press [A] to enter"

const std::string keynameSelect = "Select";
const std::string keynameClear = "Clear";
const std::string keynameKP_Enter = "KP_Enter";
const std::string keynameReturn = "Return";
const std::string keynameEsc = "XF86Back";

URIEntry::URIEntry()
    : m_parent(nullptr)
    , m_entry(NULL)
    , m_favicon(0)
    , m_entry_layout(NULL)
    , m_entrySelectedAllFirst(false)
{
    std::string edjFilePath = EDJE_DIR;
    edjFilePath.append("WebPageUI/URIEntry.edj");
    elm_theme_extension_add(NULL, edjFilePath.c_str());
}

URIEntry::~URIEntry()
{}

void URIEntry::init(Evas_Object* parent)
{
    m_parent = parent;
}

Evas_Object* URIEntry::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    if (!m_entry_layout) {
        m_entry_layout = elm_layout_add(m_parent);
        std::string edjFilePath = EDJE_DIR;
        edjFilePath.append("WebPageUI/URIEntry.edj");
        Eina_Bool layoutSetResult = elm_layout_file_set(m_entry_layout, edjFilePath.c_str(), "uri_entry_layout");
        if (!layoutSetResult)
            throw std::runtime_error("Layout file not found: " + edjFilePath);

        m_entry = elm_entry_add(m_entry_layout);
        elm_object_style_set(m_entry, "uri_entry");

        elm_entry_single_line_set(m_entry, EINA_TRUE);
        elm_entry_scrollable_set(m_entry, EINA_TRUE);
        elm_entry_input_panel_layout_set(m_entry, ELM_INPUT_PANEL_LAYOUT_URL);

        setUrlGuideText(GUIDE_TEXT_UNFOCUSED);

        evas_object_smart_callback_add(m_entry, "activated", URIEntry::activated, this);
        evas_object_smart_callback_add(m_entry, "aborted", URIEntry::aborted, this);
        evas_object_smart_callback_add(m_entry, "preedit,changed", URIEntry::preeditChange, this);
        evas_object_smart_callback_add(m_entry, "changed", URIEntry::_uri_entry_editing_changed, this);
        evas_object_smart_callback_add(m_entry, "changed,user", URIEntry::_uri_entry_editing_changed_user, this);
        evas_object_smart_callback_add(m_entry, "focused", URIEntry::focused, this);
        evas_object_smart_callback_add(m_entry, "unfocused", URIEntry::unfocused, this);
        evas_object_smart_callback_add(m_entry, "clicked", _uri_entry_clicked, this);
        evas_object_event_callback_priority_add(m_entry, EVAS_CALLBACK_KEY_DOWN, 2 * EVAS_CALLBACK_PRIORITY_BEFORE, URIEntry::_fixed_entry_key_down_handler, this);

        elm_object_part_content_set(m_entry_layout, "uri_entry_swallow", m_entry);

        m_entryBtn = elm_button_add(m_entry_layout);

        evas_object_smart_callback_add(m_entryBtn, "focused", URIEntry::focusedBtn, this);
        evas_object_smart_callback_add(m_entryBtn, "unfocused", URIEntry::unfocusedBtn, this);

        elm_object_style_set(m_entryBtn, "entry_btn");
        evas_object_smart_callback_add(m_entryBtn, "clicked", _uri_entry_btn_clicked, this);

        elm_object_part_content_set(m_entry_layout, "uri_entry_btn", m_entryBtn);
    }
    return m_entry_layout;
}

void URIEntry::changeUri(const std::string& newUri)
{
    BROWSER_LOGD("%s: newUri=%s", __func__, newUri.c_str());
    m_URI = newUri;
    if (m_URI.empty()) {
        elm_entry_entry_set(m_entry, elm_entry_utf8_to_markup(""));
        m_pageTitle = std::string();
    }
}

void URIEntry::setFavIcon(std::shared_ptr< tizen_browser::tools::BrowserImage > favicon)
{
    BROWSER_LOGD("[%s:%d] faviconType:%d ", __PRETTY_FUNCTION__, __LINE__, favicon->imageType);
    if (favicon->imageType != tools::BrowserImage::ImageTypeNoImage) {
        m_favicon = tizen_browser::tools::EflTools::getEvasImage(favicon, m_entry_layout);
        evas_object_image_fill_set(m_favicon, 0, 0, 36, 36);
        evas_object_resize(m_favicon, 36, 36);
        elm_object_part_content_set(m_entry_layout, "fav_icon", m_favicon);
        setCurrentFavIcon();
    } else {
        setDocIcon();
    }
}

void URIEntry::setCurrentFavIcon()
{
    m_currentIconType = IconTypeFav;
    elm_object_signal_emit(m_entry_layout, "show_favicon", "model");
}

void URIEntry::setSearchIcon()
{
    m_currentIconType = IconTypeSearch;
    elm_object_signal_emit(m_entry_layout, "set_search_icon", "model");
}

void URIEntry::setDocIcon()
{
    m_currentIconType = IconTypeDoc;
    elm_object_signal_emit(m_entry_layout, "set_doc_icon", "model");
}

void URIEntry::setPageTitle(const std::string& title)
{
    BROWSER_LOGD("%s", __func__);
    m_pageTitle = title;
    elm_entry_entry_set(m_entry, elm_entry_utf8_to_markup(m_pageTitle.c_str()));
}

void URIEntry::setURI(const std::string& uri)
{
    BROWSER_LOGD("%s, URI: %s", __func__, uri.c_str());
    m_URI = uri;
}

void URIEntry::showPageTitle()
{
    BROWSER_LOGD("%s, Page title: %s", __func__, m_pageTitle.c_str());
    if (!m_pageTitle.empty())
        elm_entry_entry_set(m_entry, elm_entry_utf8_to_markup(m_pageTitle.c_str()));
    else if (!m_URI.empty())
        elm_entry_entry_set(m_entry, elm_entry_utf8_to_markup(m_URI.c_str()));
    else
        elm_entry_entry_set(m_entry, elm_entry_utf8_to_markup(""));
}

URIEntry::IconType URIEntry::getCurrentIconTyep()
{
    return m_currentIconType;
}

void URIEntry::selectWholeText()
{
    m_oryginalEntryText = elm_entry_markup_to_utf8(elm_entry_entry_get(m_entry));
    if (!m_entrySelectedAllFirst && !m_oryginalEntryText.empty()) {
        elm_entry_select_all(m_entry);
        elm_entry_cursor_end_set(m_entry);
        m_entrySelectedAllFirst = true;
    }
}

void URIEntry::_uri_entry_clicked(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    URIEntry* self = static_cast<URIEntry*>(data);
    self->selectWholeText();
}

void URIEntry::_uri_entry_btn_clicked(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    URIEntry* self = static_cast<URIEntry*>(data);
    elm_object_focus_set(self->m_entry, EINA_TRUE);
    self->selectWholeText();

    elm_object_signal_emit(self->m_entry_layout, "mouse,in", "over");
}

void URIEntry::activated(void* /* data */, Evas_Object* /* obj */, void* /*event_info*/)
{
    BROWSER_LOGD("%s", __func__);
}

void URIEntry::aborted(void* data, Evas_Object* /* obj */, void* /*event_info*/)
{
    BROWSER_LOGD("%s", __func__);
    URIEntry* self = reinterpret_cast<URIEntry*>(data);
    self->editingCanceled();
}

void URIEntry::preeditChange(void* /* data */, Evas_Object* /* obj */, void* /*event_info*/)
{
    BROWSER_LOGD("%s", __func__);
}

void URIEntry::_uri_entry_editing_changed_user(void* data, Evas_Object* /* obj */, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    URIEntry* self = reinterpret_cast<URIEntry*>(data);
    std::string entry(elm_entry_markup_to_utf8(elm_entry_entry_get(self->m_entry)));
    if ((entry.find("http://") == 0)
            || (entry.find("https://") == 0)
            || (entry.find(".") != std::string::npos)) {
        self->setDocIcon();
    } else {//if(entry.find(" ") != std::string::npos){
        self->setSearchIcon();
    }
    self->uriEntryEditingChangedByUser(std::make_shared<std::string>(entry));
}

void URIEntry::_uri_entry_editing_changed(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    URIEntry* self = static_cast<URIEntry*>(data);
    self->uriEntryEditingChanged();
}

void URIEntry::setUrlGuideText(const char* txt) const
{
#if PLATFORM(TIZEN)
    elm_object_translatable_part_text_set(m_entry, "elm.guide", txt);
#else
    elm_object_part_text_set(m_entry, "elm.guide", txt);
#endif
}

void URIEntry::unfocused(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s", __func__);
    URIEntry* self = static_cast<URIEntry*>(data);
    self->m_entrySelectedAllFirst = false;
    self->setUrlGuideText(GUIDE_TEXT_UNFOCUSED);
    elm_object_signal_emit(self->m_entry_layout, "mouse,out", "over");
    elm_entry_entry_set(self->m_entry, elm_entry_utf8_to_markup(self->m_pageTitle.c_str()));
}

void URIEntry::focused(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    URIEntry* self = static_cast<URIEntry*>(data);
    self->setUrlGuideText(GUIDE_TEXT_FOCUSED);
    elm_object_signal_emit(self->m_entry_layout, "mouse,in", "over");
    elm_entry_entry_set(self->m_entry, elm_entry_utf8_to_markup(self->m_URI.c_str()));
    BROWSER_LOGD("%s, URI: %s", __func__, self->m_URI.c_str());
}

void URIEntry::_fixed_entry_key_down_handler(void* data, Evas* /*e*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("%s", __func__);
    Evas_Event_Key_Down* ev = static_cast<Evas_Event_Key_Down*>(event_info);
    if (!data || !ev || !ev->keyname)
        return;
    URIEntry* self = static_cast<URIEntry*>(data);

    if (keynameClear == ev->keyname) {
        elm_entry_entry_set(self->m_entry, "");
        return;
    }
    if (keynameSelect == ev->keyname
            || keynameReturn == ev->keyname
            || keynameKP_Enter == ev->keyname) {
        self->editingCompleted();
        return;
    }
    if (keynameEsc == ev->keyname) {
        self->editingCanceled();
        elm_object_focus_set(self->m_entryBtn, EINA_TRUE);
        return;
    }
}

void URIEntry::editingCompleted()
{
    char* text = elm_entry_markup_to_utf8(elm_entry_entry_get(m_entry));
    std::string userString(text);
    free(text);

    elm_entry_input_panel_hide(m_entry);
    uriChanged(rewriteURI(userString));
    elm_object_focus_set(m_entryBtn, EINA_TRUE);
}

std::string URIEntry::rewriteURI(const std::string& url)
{
    BROWSER_LOGD("%s: %s", __PRETTY_FUNCTION__, url.c_str());
    boost::regex urlRegex(R"(^(https?|ftp)://[^\s/$.?#].[^\s]*$)");

    if (!url.empty() && url != "about:blank" && url != "about:home") {
        if (boost::regex_match(url, urlRegex))
            return url;
        else if (boost::regex_match(std::string("http://") + url, urlRegex) &&  url.find(".") != std::string::npos)
            return std::string("http://") + url;
        else {
            std::string searchString("http://www.google.com/search?q=");
            searchString += url;
            std::replace(searchString.begin(), searchString.end(), ' ', '+');
            BROWSER_LOGD("[%s:%d] Search string: %s", __PRETTY_FUNCTION__, __LINE__, searchString.c_str());
            return searchString;
        }
    }

    return url;
}


void URIEntry::editingCanceled()
{
    BROWSER_LOGD("[%s:%d] oryinal URL: %s ", __PRETTY_FUNCTION__, __LINE__, m_oryginalEntryText.c_str());
    if (!m_oryginalEntryText.empty()) {
        elm_entry_entry_set(m_entry, elm_entry_utf8_to_markup(m_oryginalEntryText.c_str()));
        m_oryginalEntryText = "";
    }
    elm_entry_input_panel_hide(m_entry);
    setCurrentFavIcon();
}

void URIEntry::AddAction(sharedAction action)
{
    m_actions.push_back(action);
}

std::list<sharedAction> URIEntry::actions() const
{
    return m_actions;
}

void URIEntry::clearFocus()
{
    elm_object_focus_set(m_entry, EINA_FALSE);
}

void URIEntry::setFocus()
{
    elm_object_focus_set(m_entryBtn, EINA_TRUE);
}

bool URIEntry::hasFocus() const
{
    return elm_object_focus_get(m_entry) == EINA_TRUE ? true : false;
}

void URIEntry::focusedBtn(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    URIEntry* self = static_cast<URIEntry*>(data);
    elm_object_signal_emit(self->m_entry_layout, "mouse,in", "over");
}

void URIEntry::unfocusedBtn(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    URIEntry* self = static_cast<URIEntry*>(data);
    elm_object_signal_emit(self->m_entry_layout, "mouse,out", "over");
    //elm_entry_entry_set(self->m_entry, elm_entry_utf8_to_markup(self->m_pageTitle.c_str()));
}

void URIEntry::setDisabled(bool disabled)
{
    if (disabled) {
        clearFocus();
    }
    elm_object_disabled_set(getContent(), disabled ? EINA_TRUE : EINA_FALSE);
}

}
}
