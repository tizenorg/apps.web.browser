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

/*
 * TabThumbCache.h
 *
 *  Created on: May 28, 2014
 *      Author: pchmielewski
 */

#ifndef TABTHUMBCACHE_H_
#define TABTHUMBCACHE_H_

#include <memory>
#include <map>
#include <string>
#include "Tools/BrowserImage.h"
#include "TabId.h"

namespace tizen_browser {
namespace services {

class TabThumbCache {
public:

    virtual ~TabThumbCache();

    static TabThumbCache* getInstance();

    std::shared_ptr<tizen_browser::tools::BrowserImage> getThumb(tizen_browser::basic_webengine::TabId id);
    void updateThumb(tizen_browser::basic_webengine::TabId id);
    void clearThumb(tizen_browser::basic_webengine::TabId id);
private:
    TabThumbCache();
    static TabThumbCache* instance;
    std::map<std::string, std::shared_ptr<tizen_browser::tools::BrowserImage>> thumbMap;
};

} /* namespace services */
} /* namespace tizen_browser */

#endif /* TABTHUMBCACHE_H_ */
