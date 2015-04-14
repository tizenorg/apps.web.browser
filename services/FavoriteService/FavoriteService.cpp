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
 *
 * Created on: Apr, 2014
 *     Author: k.dobkowski
 */

#include "browser_config.h"
#include "FavoriteService.h"

#include <string>
#include <boost/any.hpp>
#include <BrowserAssert.h>

#include "ServiceManager.h"
//#include "service_macros.h"
#include "BrowserLogger.h"
#include "AbstractWebEngine.h"
#include "AbstractMainWindow.h"
#include "StorageService.h"
#include "StorageException.h"
#include "StorageExceptionInitialization.h"
#include "EflTools.h"
#include "GeneralTools.h"



namespace tizen_browser{
namespace services{

EXPORT_SERVICE(FavoriteService, "org.tizen.browser.service.favorite.sqlite")

FavoriteService::FavoriteService() : m_testDbMod(false)
{
    config.load("whatever");
}

FavoriteService::~FavoriteService()
{
}

/*private*/ std::shared_ptr<tizen_browser::services::StorageService> FavoriteService::getStorageManager() {
    if (!m_storageManager) {
        m_storageManager = std::dynamic_pointer_cast<
                tizen_browser::services::StorageService,
                tizen_browser::core::AbstractService>(
                tizen_browser::core::ServiceManager::getInstance().getService(
                        DOMAIN_STORAGE_SERVICE));
    }

    M_ASSERT(m_storageManager);
    m_storageManager->init(m_testDbMod);

    return m_storageManager;
}

void FavoriteService::synchronizeBookmarks()
{
    m_bookmarks.clear();
// #if PLATFORM(TIZEN)
//     favorites_bookmark_foreach(_bookmark_callback, this);
// #endif
    //addHardcodedItemsForTDC();
}

std::shared_ptr<BookmarkItem> FavoriteService::addToBookmarks(
                                                const std::string & address,
                                                const std::string & tittle,
                                                const std::string & note,
                                                std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail,
                                                std::shared_ptr<tizen_browser::tools::BrowserImage> favicon,
                                                unsigned int dirId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::shared_ptr<BookmarkItem> bookmark = std::make_shared<BookmarkItem>(address, tittle, note, dirId);
    bookmark->setThumbnail(thumbnail);
    bookmark->setFavicon(favicon);
    getStorageManager()->getBookmarkStorage()->addBookmarkItem(bookmark);
    BROWSER_LOGD("[%s:%d] id:%d, tittle:\"%s\", url:\"%s\"", __PRETTY_FUNCTION__, __LINE__,
                                                           bookmark->getId(),
                                                           bookmark->getTittle().c_str(),
                                                           bookmark->getAddress().c_str() );
    m_bookmarks.push_back(bookmark);
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bookmarkAdded(bookmark);
    return bookmark;
}

bool FavoriteService::deleteBookmark(const std::string & url)
{

    //BROWSER_LOGI("Bookmark deleted from platform service, id: %d [%s]", id, url.c_str());
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for(std::vector <std::shared_ptr<BookmarkItem> >::iterator it(m_bookmarks.begin()), end(m_bookmarks.end()); it != end; ++it){
        if ((*it)->getAddress() == url){
            BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
            getStorageManager()->getBookmarkStorage()->deleteBookmark(*it);
            m_bookmarks.erase(it);
            break;
        }
    }
    bookmarkDeleted(url);
    return true;
}

int FavoriteService::countBookmarksAndSubFolders()
{
    return m_bookmarks.size();
}

bool FavoriteService::bookmarkExists(const std::string & url)
{
    return 0 != getBookmarkId(url);
}

int FavoriteService::getBookmarkId(const std::string & url)
{
/// \todo Need to change this method of finding bookmark
    for(std::vector <std::shared_ptr<tizen_browser::services::BookmarkItem> >::iterator it(m_bookmarks.begin()), end(m_bookmarks.end()); it != end; ++it)
    if ((*it)->getAddress().compare(url) == 0)
        return (*it)->getId();
    return 0;
}

std::vector<std::shared_ptr<BookmarkItem> > FavoriteService::getBookmarks()
{
    m_bookmarks.clear();
    m_bookmarks = getStorageManager()->getBookmarkStorage()->getBookmarksAll();
    for(auto iter=m_bookmarks.begin(), end=m_bookmarks.end(); iter<end; iter++){
        (*iter)->setFavicon(getStorageManager()->getFavicon((*iter)->getAddress()));
    }
    return m_bookmarks;
}

bool FavoriteService::deleteAllBookmarks()
{

    getStorageManager()->getBookmarkStorage()->clearBookmarks();
    m_bookmarks.clear();
    bookmarksDeleted();
    return true;
}

void FavoriteService::addFakeBookmark(int fake_id, const std::string & address, const std::string & tittle,
                         const std::string & thumbnail_path, const std::string &  favicon_path, Evas * evas)
{

    if(!bookmarkExists(address)){
    Evas_Object * thumb = evas_object_image_add(evas);
    Evas_Object * favicon = evas_object_image_add(evas);
    evas_object_image_file_set(thumb, thumbnail_path.c_str(), NULL);
    evas_object_image_file_set(favicon, favicon_path.c_str(), NULL);

    std::shared_ptr<BookmarkItem> bookmark = std::make_shared<BookmarkItem>();
    std::shared_ptr<tizen_browser::tools::BrowserImage> thumb_image = tizen_browser::tools::EflTools::getBrowserImage(thumb);
    std::shared_ptr<tizen_browser::tools::BrowserImage> favicon_image = tizen_browser::tools::EflTools::getBrowserImage(favicon);

    bookmark->setId(fake_id);
    bookmark->setAddress(address);
    bookmark->setTittle(tittle);

    if (thumb)
      bookmark->setThumbnail(thumb_image);

    if(favicon)
          bookmark->setFavicon(favicon_image);

    BROWSER_LOGI("Added to bookmarks: %s, %s, id: %d", bookmark->getAddress().c_str(), bookmark->getTittle().c_str(), fake_id);
    m_bookmarks.insert(m_bookmarks.begin(), bookmark);
    }
}

void FavoriteService::addHardcodedItemsForTDC()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);

    std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
    std::dynamic_pointer_cast
    <
    tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
    tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));
    Evas * evas = evas_object_evas_get(mainUi->getMainWindow().get());

    std::vector<std::shared_ptr<BookmarkItem> > m_tmp;

    std::string thumbnail_path =  boost::any_cast <std::string> (config.get("resource/dir")) + std::string("/samsung.png");
    std::string favicon_path = boost::any_cast <std::string> (config.get("resource/dir")) + std::string("/samsung.ico");
    addFakeBookmark(9999, std::string("http://www.samsung.com/us/"), std::string("Samsung US"), thumbnail_path, favicon_path, evas);

    thumbnail_path = boost::any_cast <std::string> (config.get("resource/dir")) + std::string("/tizen.png");
    favicon_path = boost::any_cast <std::string> (config.get("resource/dir")) + std::string("/tizen.ico");
    addFakeBookmark(9998, std::string("https://www.tizen.org/"), std::string("Tizen"), thumbnail_path, favicon_path, evas);

    thumbnail_path = boost::any_cast <std::string> (config.get("resource/dir")) + std::string("/google.png");
    favicon_path = boost::any_cast <std::string> (config.get("resource/dir")) + std::string("/google.ico");
    addFakeBookmark(9997, std::string("https://www.google.com/"), std::string("Google"), thumbnail_path, favicon_path, evas);
}

void FavoriteService::setStorageServiceTestMode(bool testmode) {
    m_testDbMod = testmode;
}


} /* end of namespace services*/
} /* end of namespace tizen_browser */
