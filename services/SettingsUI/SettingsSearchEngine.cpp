/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include "SettingsSearchEngine.h"

namespace tizen_browser{
namespace base_ui{

SettingsSearchEngine::SettingsSearchEngine(Evas_Object* parent)
{
    init(parent);
    updateButtonMap();
};

SettingsSearchEngine::~SettingsSearchEngine()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsSearchEngine::updateButtonMap()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // TODO Missing all
    ItemData test;
    test.buttonText = "TEST";
    m_buttonsMap[0] = test;
}

bool SettingsSearchEngine::populateList(Evas_Object* genlist)
{
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Default search engine");
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[0], _test_page_cb);
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[0], _test_page_cb);
    return true;
}

void SettingsSearchEngine::_test_page_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    // TODO Implementation
}
}
}
