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

#ifndef SETTINGSPRETTYSIGNALCONNECTOR_H_
#define SETTINGSPRETTYSIGNALCONNECTOR_H_

#include <boost/signals2/signal.hpp>
#include <map>
#include <memory>
#include "../../core/AbstractWebEngine/WebEngineSettings.h"

namespace tizen_browser{
namespace base_ui{

enum SettingsDelPersDataOptions {
    SELECT_ALL,
    BROWSING_HISTORY,
    CACHE,
    COOKIES_AND_SITE,
    PASSWORDS,
    DEL_PERS_AUTO_FILL,
    LOCATION
};

class SettingsPrettySignalConnector
{
public:
    static SettingsPrettySignalConnector& Instance()
    {
        static SettingsPrettySignalConnector instance;
        return instance;
    }
    SettingsPrettySignalConnector(SettingsPrettySignalConnector const&) = delete;
    void operator=(SettingsPrettySignalConnector const&) = delete;

    boost::signals2::signal<void ()> settingsPrivacyClicked;
    boost::signals2::signal<void ()> settingsDelPersDataClicked;
    boost::signals2::signal<void (const std::map<SettingsDelPersDataOptions, bool>&)> deleteSelectedDataClicked;
    boost::signals2::signal<void ()> closeSettingsUIClicked;
    boost::signals2::signal<bool (const basic_webengine::WebEngineSettings&)> getWebEngineSettingsParam;
    boost::signals2::signal<void (const basic_webengine::WebEngineSettings&, bool)> setWebEngineSettingsParam;
    boost::signals2::signal<void ()> settingsBaseClicked;
    boost::signals2::signal<void ()> settingsHomePageClicked;
    boost::signals2::signal<void ()> settingsAutofillClicked;
    boost::signals2::signal<void ()> settingsAdvancedClicked;
    boost::signals2::signal<void (bool)> settingsAutofillProfileClicked;
    boost::signals2::signal<void (const std::string&)> homePageChanged;
    boost::signals2::signal<std::string ()> requestCurrentPage;
    boost::signals2::signal<bool ()> isLandscape;
    boost::signals2::signal<void (unsigned)> showSettings;
    boost::signals2::signal<void ()> settingsBaseShowRadioPopup;

private:
    SettingsPrettySignalConnector(){};
};

}
}

#endif /* SETTINGSPRETTYSIGNALCONNECTOR_H_ */
