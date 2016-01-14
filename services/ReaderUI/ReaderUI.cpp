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

#if defined(USE_EWEBKIT)
#include <ewk_chromium.h>
#endif

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <vector>
#include <string>
#include <string.h>
#include <AbstractMainWindow.h>

#include "ReaderUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"

#include <iostream>
#include <fstream>

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(ReaderUI, "org.tizen.browser.readerui")

ReaderUI::ReaderUI()
  : m_currentWebview(nullptr)
  , m_fontIncreaseBtn(nullptr)
  , m_readerLayout(nullptr)
  , m_fontDecreaseBtn(nullptr)
  , m_parent(nullptr)

{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

ReaderUI::~ReaderUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void ReaderUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
    elm_object_part_content_set(m_parent, "reader_bar_swallow", getContent());
}

Evas_Object* ReaderUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    if (!m_readerLayout) {
        m_readerLayout = elm_layout_add(m_parent);
        std::string edjFilePath = EDJE_DIR;
        edjFilePath.append("WebPageUI/ReaderMode.edj");
        elm_theme_extension_add(NULL, edjFilePath.c_str());
        Eina_Bool layoutSetResult = elm_layout_file_set(m_readerLayout, edjFilePath.c_str(), "reader_layout");
        if (!layoutSetResult)
            throw std::runtime_error("Layout file not found: " + edjFilePath);

        m_fontIncreaseBtn = elm_button_add(m_readerLayout);

        //evas_object_smart_callback_add(m_fontIncreaseBtn, "focused", ReaderMode::focusedBtn, this);
        //evas_object_smart_callback_add(m_fontIncreaseBtn, "unfocused", ReaderMode::unfocusedBtn, this);

        elm_object_style_set(m_fontIncreaseBtn, "basic_button");
        evas_object_smart_callback_add(m_fontIncreaseBtn, "clicked", _font_increase_button_clicked, this);

        elm_object_part_content_set(m_readerLayout, "font_up_button_click", m_fontIncreaseBtn);

        m_fontDecreaseBtn = elm_button_add(m_readerLayout);

        //evas_object_smart_callback_add(_fontDecreaseBtn, "focused", ReaderMode::focusedBtn, this);
        //evas_object_smart_callback_add(_fontDecreaseBtn, "unfocused", ReaderMode::unfocusedBtn, this);

        elm_object_style_set(m_fontDecreaseBtn, "basic_button");
        evas_object_smart_callback_add(m_fontDecreaseBtn, "clicked", _font_decrease_button_clicked, this);

        elm_object_part_content_set(m_readerLayout, "font_down_button_click", m_fontDecreaseBtn);
    }
    return m_readerLayout;
}

void ReaderUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void ReaderUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void ReaderUI::generateReaderContents(Evas_Object *web_view, const std::string& uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_currentWebview = web_view;
    m_originalURI = uri;
    const char* script ="var artCollection = document.getElementsByTagName('article'); \
                         var artLength = artCollection.length; \
                         var htmlContent, finalContent; \
                         for (var i = 0; i < artLength; i++) { \
                             var htmlLength = artCollection[i].innerHTML.length; \
                             if (htmlLength > 3500 ) { \
                                 htmlContent = artCollection[i].innerHTML; break; \
                             } \
                         } \
                         finalContent = htmlContent;";
    bool result = ewk_view_script_execute(m_currentWebview, script, resultForArticleTag, this);
    if (!result) {
        BROWSER_LOGD(" ewk_view_script_execute() failed \n");
    }
}

std::string ReaderUI::getOriginalURI()
{
    return m_originalURI;
}

void ReaderUI::resultForArticleTag(Evas_Object* /*o*/, const char* result_value, void* user_data)
{
    BROWSER_LOGD("[%s:%d] string for article: %s", __PRETTY_FUNCTION__, __LINE__, result_value);
    ReaderUI* thiz = static_cast<ReaderUI*>(user_data);
    if (!strcmp(result_value, ""))
    {
        BROWSER_LOGD("<article> not found, trying with <div> tag contents\n");
        const char* script = "var divCollection = document.getElementsByTagName('div'); \
                              var divLength = divCollection.length; \
                              var htmlContent, finalContent; \
                              for (var i = 0; i < divLength; i++) { \
                                        var className = divCollection[i].className; \
                                        var id = divCollection[i].id; \
                                        if (className.indexOf('article') != -1 || className.indexOf('story') != -1 || className.indexOf('content') != -1 || className.indexOf('main') != -1 || className.indexOf('page') != -1 || className.indexOf('container') != -1 ||\
                                            (id.indexOf('story') != -1 ||id.indexOf('article') != -1 || id.indexOf('content') != -1 || id.indexOf('main') != -1) || id.indexOf('page') != -1 || id.indexOf('container') != -1){ \
                                                      var htmlContent = divCollection[i].innerHTML; break; \
                                        } \
                              } \
                              finalContent = htmlContent;";
        bool result = ewk_view_script_execute(thiz->m_currentWebview, script, ReaderUI::resultForDivTag, thiz);
            if (!result) {
            BROWSER_LOGD(" ewk_view_script_execute() failed \n");
        }
        return;
    }
    std::string temp_string(result_value);
    std::string str2="<article id='main_id' style='font-size:20px'>";
    std::string str3="</article>";
    temp_string.insert(0, str2);
    temp_string.insert(temp_string.length(), str3);
    ReaderUI::generateTempFileAndLoad(temp_string.c_str(), user_data);
}

void ReaderUI::resultForDivTag(Evas_Object* /*o*/,const char* result_value, void* user_data)
{
    if (!strcmp(result_value, "")) {
        BROWSER_LOGD(" No contents found  \n");
        ReaderUI* thiz = static_cast<ReaderUI*>(user_data);
        thiz->readerContentsUnavailable();
        return;
    }
    std::string temp_string(result_value);
    std::string str2="<div id='main_id' style='font-size:20px'>";
    std::string str3="</div>";
    temp_string.insert(0,str2);
    temp_string.insert(temp_string.length(), str3);
    ReaderUI::generateTempFileAndLoad(temp_string.c_str(), user_data);
}

void ReaderUI::generateTempFileAndLoad(const char* result_value, void* user_data)
{
    ReaderUI* thiz = static_cast<ReaderUI*>(user_data);
    std::string basic_header_start = "<html><body>";
    std::string basic_header_end = "</body></html>";
    std::string final_content = basic_header_start + std::string(result_value) + basic_header_end;

    std::string file_scheme = "file:/", file_name = "//home//owner//apps_rw//org.tizen.browser//data//temp.html";
    std::ofstream myfile;
    myfile.open(file_name.c_str());
    if (myfile.is_open())
        BROWSER_LOGD("[%s:%d] File open successfull", __PRETTY_FUNCTION__, __LINE__);
    else
        BROWSER_LOGD("[%s:%d] Unable to open file", __PRETTY_FUNCTION__, __LINE__);
    myfile << final_content.c_str();
    myfile.close();
    thiz->readerContentsReady(file_scheme + file_name);
}

void ReaderUI::setReaderView(Evas_Object* view)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_currentWebview = view;
}

void ReaderUI::_font_increase_button_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    ReaderUI* thiz;
    if (data) {
        thiz = static_cast<ReaderUI*>(data);
    }
    else {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        return;
    }

    const char* script  = "var p=document.getElementById('main_id');\
    var max=50;\
    var s; \
      if(p.style.fontSize) { \
         s = parseInt(p.style.fontSize);\
      } else { \
         s = 20; \
      } \
      if(s!=max) { \
         s += 3;\
      }\
      p.style.fontSize = s+'px';\
    ";

    bool result = ewk_view_script_execute(thiz->m_currentWebview, script, ReaderUI::resultForIncrease, thiz);
    if (!result) {
        BROWSER_LOGD(" ewk_view_script_execute() failed \n");
    }
}

void ReaderUI::_font_decrease_button_clicked(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    ReaderUI* thiz;
    if (data) {
        thiz = static_cast<ReaderUI*>(data);
    }
    else {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        return;
    }

    const char* script  = "var p=document.getElementById('main_id');\
    var min=20;\
    var s; \
      if(p.style.fontSize) { \
         s = parseInt(p.style.fontSize);\
      } else { \
         s = 20; \
      } \
      if(s!=min) { \
         s -= 3;\
      }\
      p.style.fontSize = s+'px';\
    ";

    //ewk_view_script_execute(thiz->m_currentWebview, script, ReaderMode::resultForIncrease, thiz);
    bool result = ewk_view_script_execute(thiz->m_currentWebview, script, ReaderUI::resultForDecrease, thiz);
    if (!result) {
        BROWSER_LOGD(" ewk_view_script_execute() failed \n");
    }

}

void ReaderUI::resultForIncrease(Evas_Object*  /*o*/,const char* /* result_value */, void* /* user_data */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

}

void ReaderUI::resultForDecrease(Evas_Object*  /*o*/,const char* /* result_value */, void* /* user_data */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

}
}
