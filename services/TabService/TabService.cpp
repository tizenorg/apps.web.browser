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

#include "BrowserLogger.h"
#include "TabService.h"
#include "Tools/BrowserImage.h"
#include "TabId.h"
#include "Config.h"

namespace tizen_browser {
namespace services {

EXPORT_SERVICE(TabService, DOMAIN_TAB_SERVICE)

TabService::TabService()
{
    config::DefaultConfig config;
    config.load("");
    THUMB_WIDTH = boost::any_cast<int>(
            config.get(CONFIG_KEY::TABSERVICE_THUMB_WIDTH));
    THUMB_HEIGHT = boost::any_cast<int>(
            config.get(CONFIG_KEY::TABSERVICE_THUMB_HEIGHT));
}

TabService::~TabService()
{
}

tools::BrowserImagePtr TabService::getThumb(basic_webengine::TabId id)
{
    if (!thumbExists(id)) {
        BROWSER_LOGD("Cached thumb for tab[%s] not found, updating cache",
                id.toString().c_str());

        generateThumb(id);

        if (!thumbExists(id)) {
            // error, something went wrong and TabService didn't receive thumb
            // for desired ID through onThumbGenerated() slot
            BROWSER_LOGE("@@ %s error: no tab generated", __PRETTY_FUNCTION__);
            return std::make_shared<tools::BrowserImage>();
        }
    }
    // thumb exists
    return thumbMap.find(id.toString())->second;
}

void TabService::updateThumb(basic_webengine::TabId id)
{
    clearThumb(id);
    getThumb(id);
}

void TabService::clearThumb(basic_webengine::TabId id)
{
    thumbMap.erase(id.toString());
}

void TabService::fillThumbs(
        const std::vector<basic_webengine::TabContentPtr>& tabsContents)
{
    for (auto& tc : tabsContents) {
        auto thumbPtr = getThumb(tc->getId());
        tc->setThumbnail(thumbPtr);
    }
}

void TabService::onThumbGenerated(basic_webengine::TabId tabId,
        tools::BrowserImagePtr imagePtr)
{
    thumbMap.insert(
            std::pair<std::string, tools::BrowserImagePtr>(tabId.toString(),
                    imagePtr));
}

bool TabService::thumbExists(const basic_webengine::TabId& id) const
{
    return thumbMap.find(id.toString()) != thumbMap.end();
}

} /* namespace base_ui */
} /* namespace tizen_browser */
