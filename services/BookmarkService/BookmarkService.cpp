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
#include "BookmarkService.h"

#include <string>
#include <boost/any.hpp>
#include <BrowserAssert.h>

#include "ServiceManager.h"
#include "service_macros.h"
#include "BrowserLogger.h"
#include "AbstractWebEngine.h"
#include "AbstractMainWindow.h"
#include "EflTools.h"
#include "GeneralTools.h"

#include <web/web_bookmark.h>



namespace tizen_browser{
namespace services{

EXPORT_SERVICE(BookmarkService, "org.tizen.browser.favoriteservice")

BookmarkService::BookmarkService()
{
    int ret = bp_bookmark_adaptor_initialize();
    if (ret<0){
        BROWSER_LOGE("Error! Could not initialize bookmark service!");
        return;
    }
}

BookmarkService::~BookmarkService()
{
    bp_bookmark_adaptor_deinitialize();
}

/*private*/ std::shared_ptr<tizen_browser::services::StorageService> BookmarkService::getStorageManager() {
    return m_storageManager;
}

void BookmarkService::synchronizeBookmarks()
{
    m_bookmarks.clear();
}

std::shared_ptr<BookmarkItem> BookmarkService::addToBookmarks(
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

    int bookmark_id = -1;
    int ret = bp_bookmark_adaptor_create(&bookmark_id);
    if (ret<0){
        BROWSER_LOGE("Error! Could not create new bookmark!");
        return bookmark;
    }
    int *ids = NULL;
    int ids_count = 0;
    ret = bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, -1, -1, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0);
    if (ret<0){
        BROWSER_LOGE("Error! Could not get ids!");
        return std::make_shared<BookmarkItem>();
    }

    bp_bookmark_adaptor_set_url(bookmark_id, address.c_str());

    bp_bookmark_adaptor_set_title(bookmark_id, tittle.c_str());
	if (thumbnail) {
	    std::unique_ptr<tizen_browser::tools::Blob> thumb_blob = tizen_browser::tools::EflTools::getBlobPNG(thumbnail);
	    unsigned char * thumb = std::move((unsigned char*)thumb_blob->getData());
	    bp_bookmark_adaptor_set_snapshot(bookmark_id, thumbnail->width, thumbnail->height, thumb, thumb_blob->getLength());
	}
	if (favicon) {
	    std::unique_ptr<tizen_browser::tools::Blob> favicon_blob = tizen_browser::tools::EflTools::getBlobPNG(favicon);
	    unsigned char * fav = std::move((unsigned char*)favicon_blob->getData());
	    bp_bookmark_adaptor_set_icon(bookmark_id, favicon->width, favicon->height, fav, favicon_blob->getLength());
	}

    m_bookmarks.push_back(bookmark);
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bookmarkAdded(bookmark);
    return bookmark;
}

bool BookmarkService::deleteBookmark(const std::string & url)
{
    int id = getBookmarkId(url);
    if (id!=0)
        bp_bookmark_adaptor_delete(id);
    bookmarkDeleted(url);
    return true;
}

int BookmarkService::countBookmarksAndSubFolders()
{
    return m_bookmarks.size();
}

bool BookmarkService::bookmarkExists(const std::string & url)
{
    return 0 != getBookmarkId(url);
}

int BookmarkService::getBookmarkId(const std::string & url)
{
    bp_bookmark_property_cond_fmt properties;
    bp_bookmark_rows_cond_fmt conds;
    properties.parent = -1;
    properties.type = 0;
    properties.is_operator = 0;
    properties.is_editable = -1;
    conds.limit = -1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;
    int *ids = 0;
    int ids_count = 0;
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count,
            &properties, &conds, BP_BOOKMARK_O_URL, url.c_str(), 0);
    if (ids_count!=0){
        int i = *ids;
        free(ids);
        return i;
    }
    return 0;
}

std::vector<std::shared_ptr<BookmarkItem> > BookmarkService::getBookmarks()
{
    int *ids = NULL;
    int ids_count = 0;
    int ret = bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, -1, -1, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0);
    if (ret<0){
        BROWSER_LOGE("Error! Could not get ids!");
        return std::vector<std::shared_ptr<BookmarkItem>>();
    }

    m_bookmarks.clear();

    BROWSER_LOGD("Bookmark items: %d", ids_count);

    for(int i = 0; i<ids_count; i++)
    {
        bp_bookmark_info_fmt bookmark_info;
        bp_bookmark_adaptor_get_easy_all(ids[i], &bookmark_info);
        std::string url = bookmark_info.url ? bookmark_info.url : "";
        std::string title = bookmark_info.title ? bookmark_info.title : "";

        std::shared_ptr<BookmarkItem> bookmark = std::make_shared<BookmarkItem>(url,title, std::string(""), 0);

        std::shared_ptr<tizen_browser::tools::BrowserImage> bi = std::make_shared<tizen_browser::tools::BrowserImage>();
        bi->imageType = tizen_browser::tools::BrowserImage::ImageType::ImageTypePNG;
        bi->width = bookmark_info.thumbnail_width;
        bi->height = bookmark_info.thumbnail_height;
        bi->dataSize = bookmark_info.thumbnail_length;
        bi->imageData = (void*)malloc(bookmark_info.thumbnail_length);
        memcpy(bi->imageData, (void*)bookmark_info.thumbnail, bookmark_info.thumbnail_length);
        bookmark->setThumbnail(bi);

        std::shared_ptr<tizen_browser::tools::BrowserImage> fav = std::make_shared<tizen_browser::tools::BrowserImage>();
        unsigned char *image_bytes;
        bp_bookmark_adaptor_get_icon(ids[i], &fav->width, &fav->height, &image_bytes, &fav->dataSize);
        fav->imageType = tizen_browser::tools::BrowserImage::ImageType::ImageTypePNG;

        fav->imageData = (void*)malloc(bookmark_info.favicon_length);
        memcpy(fav->imageData, (void*)image_bytes, bookmark_info.favicon_length);
        bookmark->setFavicon(fav);

        m_bookmarks.push_back(bookmark);
    }
    free(ids);
    return m_bookmarks;
}

bool BookmarkService::deleteAllBookmarks()
{
    bp_bookmark_adaptor_reset();
    m_bookmarks.clear();
    bookmarksDeleted();
    return true;
}


} /* end of namespace services*/
} /* end of namespace tizen_browser */
