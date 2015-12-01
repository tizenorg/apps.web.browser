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
  : m_currentWebview(NULL)
   ,m_parent(nullptr)

{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //m_edjFilePath = EDJE_DIR;
    //m_edjFilePath.append("ReaderUI/Reader.edj");
}

ReaderUI::~ReaderUI()
{

}

void ReaderUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   // M_ASSERT(parent);
    m_parent = parent;
}
Evas_Object* ReaderUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return NULL;
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
                                        if (htmlLength > 3500 )\
					{	htmlContent = artCollection[i].innerHTML; break; }\
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
        std::string str2="<article>";
        std::string str3="</article>";
        temp_string.insert(0,str2);
        temp_string.insert(temp_string.length(),str3);
	ReaderUI::generateTempFileAndLoad(temp_string.c_str(), user_data);
}

void ReaderUI::resultForDivTag(Evas_Object*  /*o*/,const char* result_value, void* user_data)
{

       // ReaderUI* thiz = static_cast<ReaderUI*>(user_data);
	if (!strcmp(result_value, "")) {
		BROWSER_LOGD(" No contents found  \n");
		return;
	}
        std::string temp_string(result_value);
        std::string str2="<div>";
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

	std::string file_scheme = "file:/", file_name = "//usr//apps//org.tizen.browser//data//temp.html";
        std::ofstream myfile;
        myfile.open(file_name.c_str());
        if (myfile.is_open())
		BROWSER_LOGD("######################## file opened #################\n");
        else
		BROWSER_LOGD("################# file open failed #################\n");
        
        myfile << final_content.c_str();
        myfile.close();
	thiz->readerContentsReady(file_scheme + file_name);
}

}
}
