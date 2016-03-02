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

#ifndef TABSERVICE_H_
#define TABSERVICE_H_

#define DOMAIN_TAB_SERVICE "org.tizen.browser.tabservice"

#include "core/ServiceManager/ServiceFactory.h"
#include "core/ServiceManager/service_macros.h"
#include <memory>
#include <map>
#include <boost/signals2/signal.hpp>
#include <boost/optional.hpp>
#include <web/web_tab.h>
#include "core/AbstractWebEngine/TabIdTypedef.h"
#include "core/Tools/BrowserImageTypedef.h"

namespace tizen_browser {
namespace services {

/**
 * Manages tab data by using web_tab.h API.
 */
class BROWSER_EXPORT TabService: public tizen_browser::core::AbstractService
{
public:
    TabService();
    virtual ~TabService();
    virtual std::string getName() = 0;

    /**
     * Overwrite image thumb for given id with new screenshot (in cache and
     * in database).
     */
    void updateThumb(const basic_webengine::TabId& tabId);

    /**
     * Remove image thumb for given id from the cache and database.
     */
    void clearThumb(const basic_webengine::TabId& tabId);

    /**
     * Set thumb images for given TabContent objects: get them from
     * cache or database or generate them by taking screenshots.
     */
    void fillThumbs(
            const std::vector<basic_webengine::TabContentPtr>& tabsContents);

    /**
     * Invoke bp_tab_adaptor_create.
     *
     * @param tabId If -1, new id will be created. Otherwise entry in database
     * will have given id.
     * @return id The id created by bp_tab_adaptor_create.
     */
    int createTabId(int tabId = -1) const;

    /**
     * Convert tab id (string) to int.
     *
     * @return boost::none if string cannot be converted
     */
    boost::optional<int> convertTabId(std::string tabId) const;

    /**
     * Slot to which generated image is passed.
     */
    void onThumbGenerated(const basic_webengine::TabId& tabId,
            tools::BrowserImagePtr imagePtr);
    boost::signals2::signal<void(basic_webengine::TabId)> generateThumb;

private:
    /**
     * Help method printing last bp_tab_error_defs error.
     */
    void errorPrint(std::string method) const;

    /**
     * Get image thumb for given id (from cache or database).
     * Create one, if it does not exist.
     */
    tools::BrowserImagePtr getThumb(const basic_webengine::TabId& tabId);

    /**
     * Get cached thumb for given tab id.
     *
     * @return Image or boost::none.
     */
    boost::optional<tools::BrowserImagePtr> getThumbCache(
            const basic_webengine::TabId& tabId);
    /**
     * Cache given thumb image with given tab id.
     */
    void saveThumbCache(const basic_webengine::TabId& tabId,
            tools::BrowserImagePtr imagePtr);
    /**
     * Check if thumb for given id is in a map.
     */
    bool thumbCached(const basic_webengine::TabId& tabId) const;
    /**
     * Remove image from cache for given tab id.
     */
    void clearFromCache(const basic_webengine::TabId& tabId);

    /**
     * Get thumb from database for given tab id.
     *
     * @return Image or boost::none.
     */
    boost::optional<tools::BrowserImagePtr> getThumbDatabase(
            const basic_webengine::TabId& tabId);
    /**
     * Save given thumb image with given tab id in a database.
     */
    void saveThumbDatabase(const basic_webengine::TabId& tabId,
            tools::BrowserImagePtr imagePtr);
    /**
     * Check if thumb for given id is in a database.
     */
    bool thumbInDatabase(const basic_webengine::TabId& tabId) const;
    /**
     * Remove image from a database for given tab id.
     *
     * @param ID created earlier by bp_tab_adaptor_create()
     */
    void clearFromDatabase(const basic_webengine::TabId& tabId);

    /**
     * Map caching images. Keys: tab ids, values: thumb images.
     */
    std::map<int, tools::BrowserImagePtr> m_thumbMap;

    /**
     * Map caching if thumb should be saved in database.
     * Only thumb generated after website fully loaded should be saved.
     * Keys: tab ids, values: false/true
     */
    std::map<int, bool> m_thumbMapSave;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* TABSERVICE_H_ */
