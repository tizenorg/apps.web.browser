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

#include "ServiceFactory.h"
#include "service_macros.h"
#include <memory>
#include <map>
#include <boost/signals2/signal.hpp>
#include <web/web_tab.h>
#include "TabIdTypedef.h"
#include "BrowserImageTypedef.h"

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
    virtual std::string getName();

    /**
     * Get image thumb for given id. Create one, if it does not exist.
     */
    tools::BrowserImagePtr getThumb(basic_webengine::TabId id);

    /**
     * Overwrite image thumb for given id with new screenshot.
     */
    void updateThumb(basic_webengine::TabId id);

    /**
     * Remove image thumb for given id from the map.
     */
    void clearThumb(basic_webengine::TabId id);

    /**
     * Set thumb images for given TabContent objects.
     */
    void fillThumbs(
            const std::vector<basic_webengine::TabContentPtr>& tabsContents);

    /**
     * Slot to which generated image is passed.
     */
    void onThumbGenerated(basic_webengine::TabId tabId,
            tools::BrowserImagePtr imagePtr);
    boost::signals2::signal<void(basic_webengine::TabId)> generateThumb;

    int getThumbWidth() const {return THUMB_WIDTH;}
    int getThumbHeight() const {return THUMB_HEIGHT;}

private:
    /**
     * Check if thumb for given id is already generated (ready to use).
     */
    bool thumbExists(const basic_webengine::TabId& id) const;

    std::map<std::string, tools::BrowserImagePtr> thumbMap;
    int THUMB_WIDTH;
    int THUMB_HEIGHT;

};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* TABSERVICE_H_ */
