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

#include "FindOnPageUI.h"

#define FIND_WORD_MAX_COUNT 10000
#define FIND_ON_PAGE_MAX_TEXT 1000
#define FIND_ON_PAGE_ENTRY_STYLE "DEFAULT='font_size=30 color=#404040 ellipsis=1'"
#define BR_STRING_TEXT_FIELD_T "Text field"
#define BR_STRING_FIND_ON_PAGE "Find on page"
#define BR_STRING_CLEAR_ALL "Clear all"
#define BR_STRING_ACCESS_NEXT_BUT "Next"
#define BR_STRING_ACCESS_PREV_BUT "Previous"

#include <Elementary.h>
#include <vector>
#include <string>
#include <string.h>
#include <AbstractMainWindow.h>
#include "FindOnPageUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(FindOnPageUI, "org.tizen.browser.findonpageui")

FindOnPageUI::FindOnPageUI()
    : m_entry(NULL)
    , m_down_button(NULL)
    , m_up_button(NULL)
    , m_clear_button(NULL)
    , m_parent(nullptr)
    , m_total_count(0)
    , m_current_index(0)
    , m_input_word(NULL)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("FindOnPageUI/FindOnPage.edj");
}

FindOnPageUI::~FindOnPageUI(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_smart_callback_del(m_down_button, "clicked", __down_clicked_cb);
    evas_object_smart_callback_del(m_up_button, "clicked", __up_clicked_cb);

    evas_object_smart_callback_del(m_entry, "activated", __enter_key_cb);
    evas_object_smart_callback_del(m_entry, "changed", __entry_changed_cb);

    evas_object_del(m_fop_layout);
    eina_stringshare_del(m_input_word);
}

void FindOnPageUI::show()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_fop_layout)
        m_fop_layout = createFindOnPageUILayout(m_parent);
    showUI();
    show_ime();
}

void FindOnPageUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* FindOnPageUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_fop_layout)
        m_fop_layout = createFindOnPageUILayout(m_parent);
    return m_fop_layout;
}

void FindOnPageUI::show_ime(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_focus_set(m_entry, EINA_TRUE);
    elm_entry_cursor_end_set(m_entry);
}

void FindOnPageUI::set_text(const char *text)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_entry_entry_set(m_entry, "");
    char *markup_text = elm_entry_utf8_to_markup(text);
    elm_entry_entry_append(m_entry, markup_text);
    if (markup_text)
        free(markup_text);
}

void FindOnPageUI::clear_text(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_total_count = 0;
    m_current_index = 0;
    _set_count(0, 0);
    elm_entry_entry_set(m_entry, "");
}

void FindOnPageUI::unset_focus(void)
{
    elm_object_focus_set(m_entry, EINA_FALSE);
}

Evas_Object* FindOnPageUI::createFindOnPageUILayout(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    // Find on page layout.
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    Evas_Object *layout = elm_layout_add(parent);
    if (!layout) {
        BROWSER_LOGD("elm_layout_add failed");
        return NULL;
    }
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "find-on-page-layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    // Entry.
    Evas_Object *entry = elm_entry_add(layout);
    elm_object_style_set(entry, "fop_entry");
    elm_entry_single_line_set(entry, EINA_TRUE);
    elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_autocapital_type_set(entry, ELM_AUTOCAPITAL_TYPE_NONE);
    elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
    elm_entry_input_panel_enabled_set(entry, EINA_TRUE);
    elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);

    evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    evas_object_smart_callback_add(entry, "activated", __enter_key_cb, this);
    evas_object_smart_callback_add(entry, "changed", __entry_changed_cb, this);

    elm_object_part_text_set(entry, "elm.guide", (std::string("<font_size=28>") + BR_STRING_FIND_ON_PAGE + "</font>").c_str());

    Evas_Object *access = elm_access_object_get(elm_entry_textblock_get(entry));
    elm_access_info_set(access, ELM_ACCESS_TYPE, BR_STRING_TEXT_FIELD_T);
    elm_entry_text_style_user_push(entry, FIND_ON_PAGE_ENTRY_STYLE);

    static Elm_Entry_Filter_Limit_Size limit_filter_data;
    limit_filter_data.max_byte_count = 0;
    limit_filter_data.max_char_count = FIND_ON_PAGE_MAX_TEXT;
    elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);

    elm_object_part_content_set(layout, "elm.swallow.entry", entry);

    // Clear button
    Evas_Object *clear_button = elm_button_add(layout);
    elm_object_style_set(clear_button, "basic_button");
    elm_access_info_set(clear_button, ELM_ACCESS_INFO, BR_STRING_CLEAR_ALL);
    evas_object_smart_callback_add(clear_button, "clicked", __clear_clicked_cb, this);
    elm_object_part_content_set(layout, "clear_button_click", clear_button);

    // Down button.
    Evas_Object *down_button = elm_button_add(layout);
    elm_object_style_set(down_button, "basic_button");
    elm_access_info_set(down_button, ELM_ACCESS_INFO, BR_STRING_ACCESS_NEXT_BUT);
    evas_object_smart_callback_add(down_button, "clicked", __down_clicked_cb, this);
    elm_object_part_content_set(layout, "down_button_click", down_button);

    // Up button.
    Evas_Object *up_button = elm_button_add(layout);
    elm_object_style_set(up_button, "basic_button");
    elm_access_info_set(up_button, ELM_ACCESS_INFO, BR_STRING_ACCESS_PREV_BUT);
    evas_object_smart_callback_add(up_button, "clicked", __up_clicked_cb, this);
    elm_object_part_content_set(layout, "up_button_click", up_button);

    Evas_Object *close_click = elm_button_add(layout);
    elm_object_style_set(close_click, "basic_button");
    evas_object_smart_callback_add(close_click, "clicked", __close_clicked_cb, this);
    elm_object_part_content_set(layout, "close_click", close_click);

    m_entry = entry;
    m_clear_button = clear_button;
    m_down_button = down_button;
    m_up_button = up_button;
    m_fop_layout = layout;

    _set_count(0, 0);
    return layout;
}

void FindOnPageUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_show(m_fop_layout);
}

void FindOnPageUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_hide(m_fop_layout);
}

void FindOnPageUI::__close_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    FindOnPageUI *fop = static_cast<FindOnPageUI*>(data);
    fop->closeFindOnPageUIClicked();
}

void FindOnPageUI::_set_count(int index, int total)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("%d/%d", index, total);

    char text_buffer[16] = { 0, };
    int digit_count = 0;
    if (total > 999)
        digit_count = 4;
    else if (total > 99)
        digit_count = 3;
    else if (total > 9)
        digit_count = 2;
    else
        digit_count = 1;

    const char *elm_text = elm_entry_entry_get(m_entry);
    if (elm_text == NULL || strlen(elm_text) == 0) {
        // Show 0/0
        elm_object_signal_emit(m_fop_layout, "digit_1,signal", "");
    } else {
        // Change count layout size.
        sprintf(text_buffer, "digit_%d,signal", digit_count);
        elm_object_signal_emit(m_fop_layout, text_buffer, "");
    }

    sprintf(text_buffer, "%d/%d", index, total);
    elm_object_part_text_set(m_fop_layout, "elm.text.count", text_buffer);

    if (total <= 1) {
        BROWSER_LOGD("total %d", total);
        _disable_up_button(EINA_TRUE);
        _disable_down_button(EINA_TRUE);
        return;
    }

    _disable_up_button(EINA_FALSE);
    _disable_down_button(EINA_FALSE);
}

void FindOnPageUI::_disable_down_button(Eina_Bool disable)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_disabled_set(m_down_button, disable);
}

void FindOnPageUI::_disable_up_button(Eina_Bool disable)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_disabled_set(m_up_button, disable);
}

void FindOnPageUI::__text_found_cb(void *data, Evas_Object* /*obj*/, void *event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!data)
        return;

    FindOnPageUI *fop = static_cast<FindOnPageUI*>(data);

    int match_count = *(static_cast<int*>(event_info));
    BROWSER_LOGD("match_count=%d", match_count);
    fop->m_total_count = match_count;
    if(fop->m_current_index == 0)
        fop->m_current_index = 1;

    if (match_count == -1 || match_count >= FIND_WORD_MAX_COUNT)
        fop->m_total_count = FIND_WORD_MAX_COUNT-1;
    else
        fop->m_total_count = match_count;

    if (match_count == 0)
        fop->m_current_index = 0;

    if(fop->m_current_index > fop->m_total_count)
        fop->m_current_index = fop->m_total_count;
    fop->_set_count(fop->m_current_index, fop->m_total_count);
}

void FindOnPageUI::__entry_changed_cb(void *data, Evas_Object *obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    FindOnPageUI *fop = static_cast<FindOnPageUI*>(data);

    fop->m_current_index = 1;

    const char *elm_text = elm_entry_entry_get(obj);
    char *text = elm_entry_markup_to_utf8(elm_text);

    if(!text) {
        BROWSER_LOGD("[%s:%d] No input text to search! ", __PRETTY_FUNCTION__, __LINE__);
        return;
    }

    BROWSER_LOGD("text=[%s]", text);

    if (elm_text && strlen(elm_text))
        elm_object_signal_emit(fop->m_fop_layout, "show,clear,button,signal", "");
    else
        elm_object_signal_emit(fop->m_fop_layout, "hide,clear,button,signal", "");

    if (strlen(text) >= FIND_ON_PAGE_MAX_TEXT) {

        // TODO : Show Notification.

        char buf[FIND_ON_PAGE_MAX_TEXT + 1] = {0, };
        snprintf(buf, sizeof(buf) - 1, "%s", text);

        fop->set_text(buf);

        elm_entry_cursor_end_set(obj);
        free(text);
        return;
    }

    std::string input_uri_str;

    input_uri_str = std::string(text);
    unsigned int pos = input_uri_str.find("<preedit>");
    if (pos != std::string::npos)
        input_uri_str.erase(pos, strlen("<preedit>"));
    pos = input_uri_str.find("</preedit>");
    if (pos != std::string::npos)
        input_uri_str.erase(pos, strlen("</preedit>"));

    free(text);

    if ((!fop->m_input_word) || (input_uri_str.c_str() && strcmp(fop->m_input_word, input_uri_str.c_str()))) {
        eina_stringshare_replace(&fop->m_input_word, input_uri_str.c_str());
        struct FindData fd = {
            input_uri_str.c_str(),
            EINA_TRUE,
            __text_found_cb,
            data,
        };
        fop->startFindingWord(fd);
    }
}

void FindOnPageUI::__enter_key_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    // Unfocus the entry to hide IME when Done/Search key is pressed.
    FindOnPageUI *fop = static_cast<FindOnPageUI*>(data);
    // TODO : Check if IME is shown.
    elm_object_focus_set(fop->m_entry, EINA_FALSE);
}

void FindOnPageUI::__clear_clicked_cb(void *data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    FindOnPageUI *fop = static_cast<FindOnPageUI*>(data);
    elm_entry_entry_set(fop->m_entry, "");
    elm_object_focus_set(fop->m_entry, EINA_TRUE);
    elm_object_signal_emit(fop->m_fop_layout, "hide,clear,button,signal", "");
}

void FindOnPageUI::__down_clicked_cb(void *data, Evas_Object *obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    FindOnPageUI *fop = static_cast<FindOnPageUI*>(data);

    if (fop->m_total_count == 0)
        return;

    fop->m_current_index++;
    if (fop->m_current_index > fop->m_total_count)
        fop->m_current_index = 1;

    struct FindData fd = {
        fop->m_input_word,
        EINA_TRUE,
        __text_found_cb,
        data,
    };
    fop->startFindingWord(fd);

    elm_object_focus_set(obj, EINA_TRUE);
}

void FindOnPageUI::__up_clicked_cb(void *data, Evas_Object *obj, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    FindOnPageUI *fop = static_cast<FindOnPageUI*>(data);

    if (fop->m_total_count == 0)
        return;

    fop->m_current_index--;
    if (fop->m_current_index < 1)
        fop->m_current_index = fop->m_total_count;

    struct FindData fd = {
        fop->m_input_word,
        EINA_FALSE,
        __text_found_cb,
        data,
    };
    fop->startFindingWord(fd);

    elm_object_focus_set(obj, EINA_TRUE);
}

}
}
