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
 *
 *
 */

#ifndef WEB_ENGINE_SETTINGS_H
#define WEB_ENGINE_SETTINGS_H

namespace tizen_browser {
namespace basic_webengine {

enum class WebEngineSettings {
    PAGE_OVERVIEW,
    LOAD_IMAGES,
    ENABLE_JAVASCRIPT,
    REMEMBER_FROM_DATA,
    REMEMBER_PASSWORDS
};

// string parameters mapping
const std::map<WebEngineSettings, std::string> PARAMS_NAMES = {
    {WebEngineSettings::PAGE_OVERVIEW, "page_overview"},
    {WebEngineSettings::LOAD_IMAGES, "load_images"},
    {WebEngineSettings::ENABLE_JAVASCRIPT, "enable_javascript"},
    {WebEngineSettings::REMEMBER_FROM_DATA, "remember_form_data"},
    {WebEngineSettings::REMEMBER_PASSWORDS, "remember_passwords"}
};

// array used to iterate over
const std::array<WebEngineSettings, 5> ALL_WEBENGINE_SETTINGS = {
    WebEngineSettings::PAGE_OVERVIEW,
    WebEngineSettings::LOAD_IMAGES,
    WebEngineSettings::ENABLE_JAVASCRIPT,
    WebEngineSettings::REMEMBER_FROM_DATA,
    WebEngineSettings::REMEMBER_PASSWORDS
};

}
}

#endif
