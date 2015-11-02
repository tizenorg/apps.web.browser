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

#include "browser_config.h"
#include "Config.h"
#include <tzplatform_config.h>

namespace tizen_browser
{
namespace config
{

void DefaultConfig::load(const std::string &)
{
    const char *db_path = NULL;
    db_path = tzplatform_getenv(TZ_USER_APP);

    m_data["main_service_name"] = std::string("org.tizen.browser.base_UI");
    //m_data["favorite_service_name"] = std::string("org.tizen.browser.service.favorite.browserProvider");
    m_data["favorite_service_name"] = std::string("org.tizen.browser.favoriteservice");
    m_data["DB_BOOKMARK"] = std::string(".browser.bookmark.db");
    m_data["DB_SETTINGS"] = std::string(".browser.settings.db");
    m_data["DB_HISTORY"] = std::string(".browser.history.db");
    m_data["DB_SESSION"] = std::string(".browser.session.db");

    m_data["TOOLTIP_DELAY"] = 0.05; // time from mouse in to tooltip show
    m_data["TOOLTIP_HIDE_TIMEOUT"] = 2.0; // time from tooltip show to tooltip hide
    m_data["RELOAD_WEBVIEW_TIMEOUT"] = 0.01; // time for webpage to initialize
    m_data["TAB_LIMIT"] = 10;    // max number of open tabs
    m_data["FAVORITES_LIMIT"] = 40;    // max number of added favorites

#   include "ConfigValues.h"

    m_data["resourcedb/dir"] = std::string(db_path)+"/"+std::string("org.tizen.browser/data/");

    m_data["mobile_scale"] = 1.98;
}

void DefaultConfig::store(const std::string & )
{
}

boost::any DefaultConfig::get(const std::string & key)
{
	return m_data[key];
}

void DefaultConfig::set(const std::string & key, const boost::any & value)
{
	m_data[key] = value;
}

bool DefaultConfig::isMobileProfile() const
{
    return boost::any_cast<std::string>(m_data.at("profile")) == MOBILE;
}

} /* end of namespace config */
} /* end of namespace tizen_browser */
