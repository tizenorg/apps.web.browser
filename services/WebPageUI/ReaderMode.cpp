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
#include "ReaderMode.h"
#include "BrowserLogger.h"
#include "MenuButton.h"
#include <algorithm>
#include <boost/regex.hpp>
#include "BrowserAssert.h"

namespace tizen_browser {
namespace base_ui {

/*#define GUIDE_TEXT_FOCUSED "Search or URL"
#define GUIDE_TEXT_UNFOCUSED "Search or URL - Press [A] to enter"

const std::string keynameSelect = "Select";
const std::string keynameClear = "Clear";
const std::string keynameKP_Enter = "KP_Enter";
const std::string keynameReturn = "Return";
const std::string keynameEsc = "XF86Back";*/

ReaderMode::ReaderMode()
    : m_parent(nullptr)
    , m_reader_layout(NULL)
    , m_fontIncreaseBtn(NULL)
    , m_fontDecreaseBtn(NULL)
{
    std::string edjFilePath = EDJE_DIR;
    edjFilePath.append("WebPageUI/ReaderMode.edj");
    elm_theme_extension_add(NULL, edjFilePath.c_str());
}

ReaderMode::~ReaderMode()
{}

void ReaderMode::init(Evas_Object* parent)
{
    m_parent = parent;
}

Evas_Object* ReaderMode::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    if (!m_reader_layout) {
        m_reader_layout = elm_layout_add(m_parent);
        std::string edjFilePath = EDJE_DIR;
        edjFilePath.append("WebPageUI/ReaderMode.edj");
        Eina_Bool layoutSetResult = elm_layout_file_set(m_reader_layout, edjFilePath.c_str(), "reader_layout");
        if (!layoutSetResult)
            throw std::runtime_error("Layout file not found: " + edjFilePath);

        m_fontIncreaseBtn = elm_button_add(m_reader_layout);

        //evas_object_smart_callback_add(m_fontIncreaseBtn, "focused", ReaderMode::focusedBtn, this);
        //evas_object_smart_callback_add(m_fontIncreaseBtn, "unfocused", ReaderMode::unfocusedBtn, this);

        elm_object_style_set(m_fontIncreaseBtn, "basic_button");
        evas_object_smart_callback_add(m_fontIncreaseBtn, "clicked", _font_increase_button_clicked, this);

        elm_object_part_content_set(m_reader_layout, "font_up_button_click", m_fontIncreaseBtn);

        m_fontDecreaseBtn = elm_button_add(m_reader_layout);

        //evas_object_smart_callback_add(_fontDecreaseBtn, "focused", ReaderMode::focusedBtn, this);
        //evas_object_smart_callback_add(_fontDecreaseBtn, "unfocused", ReaderMode::unfocusedBtn, this);

        elm_object_style_set(_fontDecreaseBtn, "basic_button");
        evas_object_smart_callback_add(_fontDecreaseBtn, "clicked", _font_decrease_button_clicked, this);

        elm_object_part_content_set(m_reader_layout, "font_up_button_click", _fontDecreaseBtn);
    }
    return m_reader_layout;
}

void ReaderMode::_font_increase_button_clicked(void* /*data*/, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void ReaderMode::_font_decrease_button_clicked(void* /*data*/, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

}
}
