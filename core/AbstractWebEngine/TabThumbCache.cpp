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
 * TabThumbCache.cpp
 *
 *  Created on: May 28, 2014
 *      Author: pchmielewski
 */

#include "browser_config.h"
#include <stdexcept>
#include "AbstractService.h"
#include "AbstractWebEngine.h"
#include "TabThumbCache.h"
#include "BrowserConstants.h"
#include "ServiceManager.h"

using namespace tizen_browser::config;
using namespace tizen_browser::core;
using namespace tizen_browser::basic_webengine;
using namespace tizen_browser::tools;

namespace tizen_browser {
namespace services {

TabThumbCache* TabThumbCache::instance = NULL;

TabThumbCache::TabThumbCache() {
    // TODO Auto-generated constructor stub

}

TabThumbCache::~TabThumbCache() {
    // TODO Auto-generated destructor stub
}

TabThumbCache* TabThumbCache::getInstance(){
    if(instance == NULL){
        instance = new TabThumbCache();
    }
    return instance;
}

std::shared_ptr<BrowserImage> TabThumbCache::getThumb(TabId id){
    BROWSER_LOGD("[%s] Cached tab thumbnails: %d", __func__, thumbMap.size());
    BROWSER_LOGD("[%s] Getting cached thumb for Tab[%s]", __func__, id.toString().c_str());
    std::map<std::string,std::shared_ptr<BrowserImage>>::iterator it;
    it = thumbMap.find(id.toString());
    if(it == thumbMap.end()){
        BROWSER_LOGD("[%s] Cached thumb for Tab[%s] not found, updating cache ...", __func__, id.toString().c_str());
        std::shared_ptr<AbstractWebEngine<Evas_Object>> m_webEngine =
            std::dynamic_pointer_cast
            <AbstractWebEngine<Evas_Object>,AbstractService>
            (ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));
        std::shared_ptr<BrowserImage> result = m_webEngine->getSnapshotData(id,THUMB_WIDTH,THUMB_HEIGHT);
        thumbMap.insert(std::pair<std::string,std::shared_ptr<BrowserImage>>(id.toString(),result));
        BROWSER_LOGD("[%s] Cached tab thumbnails: %d", __func__, thumbMap.size());
        return result;
    } else {
        BROWSER_LOGD("[%s] Cached thumb for Tab[%s] found", __func__, id.toString().c_str());
        return it->second;
    }
}

void TabThumbCache::updateThumb(tizen_browser::basic_webengine::TabId id)
{
    clearThumb(id);
    getThumb(id);
}

void TabThumbCache::clearThumb(TabId id){
    thumbMap.erase(id.toString());
}

} /* namespace services */
} /* namespace tizen_browser */
