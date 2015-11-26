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

#ifndef READERUI_H
#define READERUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT ReaderUI
    : public tizen_browser::interfaces::AbstractUIComponent
    , public tizen_browser::core::AbstractService
{
public:
    ReaderUI();
    ~ReaderUI();
    void init(Evas_Object* parent);
    Evas_Object* getContent();
    void showUI();
    void hideUI();
    virtual std::string getName();
    void generateReaderContents(Evas_Object *web_view, const std::string& uri);
    std::string getOriginalURI();
    static void resultForArticleTag(Evas_Object* o,const char* result_value, void* user_data);
    static void resultForDivTag(Evas_Object* o, const char* result_value, void* user_data);
    static void generateTempFileAndLoad(const char* result_value, void* user_data);

    boost::signals2::signal<void (std::string)> readerContentsReady;
private:

    Evas_Object* m_currentWebview;
    Evas_Object *m_parent;
    std::string m_originalURI;
    std::string m_edjFilePath;
};

}
}

#endif // READERUI_H
