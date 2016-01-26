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

#include "BrowserImage.h"
#include "EflTools.h"
#include "BrowserLogger.h"
#include "TabService.h"
#include "Blob.h"
#include "TabId.h"
#include <web/web_tab.h>
#include "CapiWebErrorCodes.h"

namespace tizen_browser {
namespace services {

EXPORT_SERVICE(TabService, DOMAIN_TAB_SERVICE)

TabService::TabService()
{
    if (bp_tab_adaptor_initialize() < 0) {
        errorPrint("bp_tab_adaptor_initialize");
    }
}

TabService::~TabService()
{
    if (bp_tab_adaptor_deinitialize() < 0) {
        errorPrint("bp_tab_adaptor_deinitialize");
    }
}

int TabService::createTabId(int tabId) const
{
    int adaptorId = tabId;
    bp_tab_info_fmt info;
    std::memset(&info, 0, sizeof(bp_tab_info_fmt));

    if (!bp_tab_adaptor_easy_create(&adaptorId, &info)) {
        errorPrint("bp_tab_adaptor_create");
    }
    bp_tab_adaptor_easy_free(&info);
    return adaptorId;
}

boost::optional<int> TabService::convertTabId(std::string tabId) const
{
    try {
        boost::optional<int> tabIdConverted = std::stoi(tabId);
        return tabIdConverted;
    } catch (const std::exception& /*e*/) {
        BROWSER_LOGE("%s can't convert %s to tab id", __PRETTY_FUNCTION__,
                tabId.c_str());
        return boost::none;
    }
}

void TabService::errorPrint(std::string method) const
{
    int error_code = bp_tab_adaptor_get_errorcode();
    BROWSER_LOGE("%s error: %d (%s)", method.c_str(), error_code,
            tools::capiWebError::tabErrorToString(error_code).c_str());
}

tools::BrowserImagePtr TabService::getThumb(const basic_webengine::TabId& tabId)
{
    // TODO Check for improvements - low cache usage
    auto imageDatabase = getThumbDatabase(tabId);
    if(imageDatabase) {
        saveThumbCache(tabId, *imageDatabase);
        return *imageDatabase;
    }

    BROWSER_LOGD("%s [%d] generating thumb", __FUNCTION__, tabId.get());
    clearThumb(tabId);
    generateThumb(tabId);

    if (m_thumbMapSave[tabId.get()])
        return std::make_shared<tools::BrowserImage>();
    auto imageCache = getThumbCache(tabId);
    if (!imageCache) {
        // error, something went wrong and TabService didn't receive thumb
        // for desired ID through onThumbGenerated() slot
        BROWSER_LOGE("%s error: no thumb generated", __PRETTY_FUNCTION__);
        return std::make_shared<tools::BrowserImage>();
    }
    return *imageCache;
}

boost::optional<tools::BrowserImagePtr> TabService::getThumbCache(
        const basic_webengine::TabId& tabId)
{
    if (!thumbCached(tabId)) {
        return boost::none;
    }
    return m_thumbMap.find(tabId.get())->second;
}

void TabService::updateThumb(const basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("%s [%d]", __FUNCTION__, tabId.get());
    m_thumbMapSave[tabId.get()] = true;
    clearThumb(tabId);
    getThumb(tabId);
}

void TabService::clearThumb(const basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("%s [%d]", __FUNCTION__, tabId.get());
    clearFromDatabase(tabId);
    clearFromCache(tabId);
}

void TabService::fillThumbs(
        const std::vector<basic_webengine::TabContentPtr>& tabsContents)
{
    for (auto& tc : tabsContents) {
        auto thumbPtr = getThumb(tc->getId());
        m_thumbMapSave.insert(std::pair<int, bool>(tc->getId().get(), false));
        tc->setThumbnail(thumbPtr);
    }
}

void TabService::onThumbGenerated(const basic_webengine::TabId& tabId,
        tools::BrowserImagePtr imagePtr)
{
    if (m_thumbMapSave[tabId.get()]) {
        if(!thumbInDatabase(tabId)) {
            // prepare adaptor id before saving in db
            createTabId(tabId.get());
        }
        // TODO (m.kawonczyk) getBlobPNG() works fast on Z3. It should be improved for N4
        saveThumbDatabase(tabId, imagePtr);
    }
    saveThumbCache(tabId, imagePtr);
}

void TabService::saveThumbCache(const basic_webengine::TabId& tabId,
        tools::BrowserImagePtr imagePtr)
{
    m_thumbMap.insert(
            std::pair<int, tools::BrowserImagePtr>(tabId.get(), imagePtr));
}

bool TabService::thumbCached(const basic_webengine::TabId& tabId) const
{
    return m_thumbMap.find(tabId.get()) != m_thumbMap.end();
}

void TabService::clearFromCache(const basic_webengine::TabId& tabId) {
    m_thumbMap.erase(tabId.get());
}

void TabService::clearFromDatabase(const basic_webengine::TabId& tabId)
{
    if (!thumbInDatabase(tabId))
        return;
    if (bp_tab_adaptor_delete(tabId.get()) < 0) {
        errorPrint("bp_tab_adaptor_delete");
    }
}

void TabService::saveThumbDatabase(const basic_webengine::TabId& tabId,
        tools::BrowserImagePtr imagePtr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::unique_ptr<tools::Blob> thumb_blob = tools::EflTools::getBlobPNG(
            imagePtr);
    unsigned char* thumbData = std::move((unsigned char*) thumb_blob->getData());
    if (bp_tab_adaptor_set_snapshot(tabId.get(), imagePtr->getWidth(),
            imagePtr->getHeight(), thumbData, thumb_blob->getLength()) < 0) {
        errorPrint("bp_tab_adaptor_set_snapshot");
    }
}

bool TabService::thumbInDatabase(const basic_webengine::TabId& tabId) const
{
    int* ids;
    int count;
    if (bp_tab_adaptor_get_full_ids_p(&ids, &count) < 0) {
        errorPrint("bp_tab_adaptor_get_full_ids_p");
        return false;
    }
    bool exists = false;
    for(int i = 0; i < count; ++i) {
        if (ids[i] == tabId.get()) {
            exists = true;
            break;
        }
    }

    if(count)
        free(ids);

    return exists;
}

boost::optional<tools::BrowserImagePtr> TabService::getThumbDatabase(
        const basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!thumbInDatabase(tabId)) {
        return boost::none;
    }

    int w = 0, h = 0, l = 0;
    unsigned char *v = nullptr;
    if (bp_tab_adaptor_get_snapshot(tabId.get(), &w, &h, &v, &l)) {
        errorPrint("bp_tab_adaptor_get_snapshot");
        return boost::none;
    }

    tools::BrowserImagePtr image = std::make_shared<tools::BrowserImage>(w, h, l);
    // TODO check if we can use shared memory here
    image->setData((void*)v, false, tools::ImageType::ImageTypePNG);

    if (image->getSize() <= 0) {
        return boost::none;
    }

    return image;
}

} /* namespace base_ui */
} /* namespace tizen_browser */
