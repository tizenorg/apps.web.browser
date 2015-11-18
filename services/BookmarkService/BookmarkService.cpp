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
#include <Elementary.h>

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
#include "Tools/CapiWebErrorCodes.h"

namespace tizen_browser{
namespace services{

EXPORT_SERVICE(BookmarkService, "org.tizen.browser.favoriteservice")

BookmarkService::BookmarkService()
{
    if(bp_bookmark_adaptor_initialize() < 0) {
        errorPrint("bp_bookmark_adaptor_initialize");
        return;
    }
}

BookmarkService::~BookmarkService()
{
    bp_bookmark_adaptor_deinitialize();
}

void BookmarkService::errorPrint(std::string method) const
{
    int error_code = bp_bookmark_adaptor_get_errorcode();
    BROWSER_LOGE("%s error: %d (%s)", method.c_str(), error_code,
            tools::capiWebError::bookmarkErrorToString(error_code).c_str());
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
    bp_bookmark_property_cond_fmt properties;
    properties.parent = -1;
    properties.type = 0;
    properties.is_operator = -1;
    properties.is_editable = -1;
    //conditions for querying
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = 1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;

    int id = -1;
    int *ids = nullptr;
    int ids_count = -1;
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, address.c_str(), 0);
    free(ids);
    if (ret < 0){
        BROWSER_LOGE("Error! Could not get ids!");
        return std::make_shared<BookmarkItem>();
    }

    bp_bookmark_info_fmt info;

    std::memset(&info, 0, sizeof(bp_bookmark_info_fmt));
    info.type = 0;
    info.parent = dirId;
    info.sequence = -1;
    info.access_count = -1;
    info.editable = 1;

    if (!address.empty()) {
        info.url = (char*) address.c_str();
    }
    if (!tittle.empty())
        info.title = (char*) tittle.c_str();

    if (bp_bookmark_adaptor_easy_create(&id, &info) < 0) {
        errorPrint("bp_bookmark_adaptor_easy_create");
        return std::make_shared<BookmarkItem>();
    }

    // max sequence
    ret = bp_bookmark_adaptor_set_sequence(id, -1);

    if(thumbnail)
    {
        std::unique_ptr<tizen_browser::tools::Blob> thumb_blob = tizen_browser::tools::EflTools::getBlobPNG(thumbnail);
        unsigned char * thumb = std::move((unsigned char*)thumb_blob->getData());
        bp_bookmark_adaptor_set_snapshot(id, thumbnail->width, thumbnail->height, thumb, thumb_blob->getLength());
    }
    if(favicon)
    {
        std::unique_ptr<tizen_browser::tools::Blob> favicon_blob = tizen_browser::tools::EflTools::getBlobPNG(favicon);
        unsigned char * fav = std::move((unsigned char*)favicon_blob->getData());
        bp_bookmark_adaptor_set_icon(id, favicon->width, favicon->height, fav, favicon_blob->getLength());
    }
    std::shared_ptr<BookmarkItem> bookmark = std::make_shared<BookmarkItem>(address, tittle, note, dirId, id);
    bookmark->setThumbnail(thumbnail);
    bookmark->setFavicon(favicon);
#if PROFILE_MOBILE
    bookmark->set_folder_flag(EINA_FALSE);
#endif
    m_bookmarks.push_back(bookmark);
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

int BookmarkService::countBookmarks()
{
    return m_bookmarks.size();
}

bool BookmarkService::bookmarkExists(const std::string & url)
{
    return 0 != getBookmarkId(url);
}

int BookmarkService::getBookmarkId(const std::string & url)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bp_bookmark_property_cond_fmt properties;
    properties.parent = -1;
    properties.type = 0;
    properties.is_operator = 0;
    properties.is_editable = -1;
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = -1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;
    int *ids = nullptr;
    int ids_count = 0;
    int i = 0;
    bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, url.c_str(), 0);
    if (ids_count > 0){
        i = *ids;
    }
    free(ids);
    return i;
}

std::string BookmarkService::getBookmarkFolderName(int folder_id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    char* folder_name;
    switch(folder_id){
     case ALL_BOOKMARKS_ID:
        return "All";
     case ROOT_FOLDER_ID:
#if PROFILE_MOBILE
        return "Mobile";
#else
        return "Bookmark"; // default folder name of the TV profile is not defined yet
#endif
    }

    if( bp_bookmark_adaptor_get_title(folder_id, &folder_name) < 0){
        errorPrint("bp_bookmark_adaptor_get_title");
        return("Mobile");
    }
    return folder_name;
}

std::vector<std::shared_ptr<BookmarkItem> > BookmarkService::getBookmarks(int folder_id)
{
    BROWSER_LOGD("[%s:%d] folder_id = %d", __func__, __LINE__, folder_id);
    int *ids = nullptr;
    int ids_count = 0;
#if PROFILE_MOBILE
    if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id,
            ALL_TYPE, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0) < 0) {
#else
    if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id,
            BOOKMARK_TYPE, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0) < 0) {
#endif
        errorPrint("bp_bookmark_adaptor_get_ids_p");
        return std::vector<std::shared_ptr<BookmarkItem>>();
    }

    m_bookmarks.clear();
    BROWSER_LOGD("Bookmark items: %d", ids_count);

    for(int i = 0; i<ids_count; i++)
    {
        bp_bookmark_info_fmt bookmark_info;
        if (bp_bookmark_adaptor_get_easy_all(ids[i], &bookmark_info) == 0) {
            std::string url = (bookmark_info.url ? bookmark_info.url : "");
            std::string title = (bookmark_info.title ? bookmark_info.title : "");

            std::shared_ptr<BookmarkItem> bookmark = std::make_shared<BookmarkItem>(url, title, std::string(""),(int) bookmark_info.parent, ids[i]);

            if (bookmark_info.thumbnail_length != -1) {
                std::shared_ptr<tizen_browser::tools::BrowserImage> bi = std::make_shared<tizen_browser::tools::BrowserImage>();
                bi->imageType = tizen_browser::tools::BrowserImage::ImageType::ImageTypePNG;
                bi->width = bookmark_info.thumbnail_width;
                bi->height = bookmark_info.thumbnail_height;
                bi->dataSize = bookmark_info.thumbnail_length;
                bi->imageData = (void*)malloc(bookmark_info.thumbnail_length);
                memcpy(bi->imageData, (void*)bookmark_info.thumbnail, bookmark_info.thumbnail_length);
                bookmark->setThumbnail(bi);
            } else {
                BROWSER_LOGD("bookmark thumbnail size is -1");
            }

            if (bookmark_info.favicon_length != -1) {
                std::shared_ptr<tizen_browser::tools::BrowserImage> fav = std::make_shared<tizen_browser::tools::BrowserImage>();
                unsigned char *image_bytes;
                bp_bookmark_adaptor_get_icon(ids[i], &fav->width, &fav->height, &image_bytes, &fav->dataSize);
                fav->imageType = tizen_browser::tools::BrowserImage::ImageType::ImageTypePNG;

                fav->imageData = (void*)malloc(bookmark_info.favicon_length);
                memcpy(fav->imageData, (void*)image_bytes, bookmark_info.favicon_length);
                bookmark->setFavicon(fav);
            } else {
                BROWSER_LOGD("bookmark favicon size is -1");
            }
#if PROFILE_MOBILE
            bool is_folder = (bookmark_info.type > 0 ? EINA_TRUE : EINA_FALSE);
            bookmark->set_folder_flag(is_folder);
#endif
            m_bookmarks.push_back(bookmark);
        } else {
            BROWSER_LOGD("bp_bookmark_adaptor_get_easy_all error");
        }
    }
    free(ids);
    return m_bookmarks;
}

bool BookmarkService::deleteAllBookmarks()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bp_bookmark_adaptor_reset();
    m_bookmarks.clear();
    bookmarksDeleted();
    return true;
}

bool BookmarkService::delete_by_id(int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("id[%d]", id);
    if (bp_bookmark_adaptor_delete(id) < 0)
        return false;
    else {
        bp_bookmark_adaptor_publish_notification();
        return true;
    }
}

bool BookmarkService::delete_by_id_notify(int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("id[%d]", id);

    BookmarkItem item;
    get_item_by_id(id, &item);
    return delete_by_id(id);
}

bool BookmarkService::delete_by_uri(const char *uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("uri[%s]", uri);
    bp_bookmark_property_cond_fmt properties;
    properties.parent = -1;
    properties.type = 0;
    properties.is_operator = -1;
    properties.is_editable = -1;
    //conditions for querying
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = 1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;

    int *ids = nullptr;
    int ids_count = -1;
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, uri, 0);
    bool result = false;
    if (ret >= 0 && ids_count > 0)
    {
        delete_by_id_notify(ids[0]);
        free(ids);
    }

    return result;
}

int BookmarkService::update_bookmark(int id, const char *title, const char *uri, int parent_id, int order,
                                     bool is_duplicate_check_needed, bool is_URI_check_needed)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    bool is_URI_exist = (uri != nullptr && strlen(uri) > 0);
    bool is_title_exist = (title != nullptr && strlen(title) > 0);
    int ret = -1;
    if (is_duplicate_check_needed) {
        bp_bookmark_property_cond_fmt properties;
        properties.parent = -1;
        properties.type = 0;
        properties.is_operator = -1;
        properties.is_editable = -1;
        //conditions for querying
        bp_bookmark_rows_cond_fmt conds;
        conds.limit = 1;
        conds.offset = 0;
        conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
        conds.ordering = 0;
        conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
        conds.period_type = BP_BOOKMARK_DATE_ALL;
        int *ids = nullptr;
        int ids_count = -1;
        //This is a bookmark item so check for any such URL already exists
        if (is_URI_exist && is_URI_check_needed) {
            ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, uri, 0);
            free(ids);
            if (ret < 0) {
                errorPrint("bp_bookmark_adaptor_get_cond_ids_p");
                return -1;
            }
        }

        if (ids_count > 0) {
            BROWSER_LOGD("same bookmark already exist");
            return 0;
        }
    }
    bp_bookmark_info_fmt info;

    std::memset(&info, 0, sizeof(bp_bookmark_info_fmt));
    info.type = -1;
    info.parent = parent_id;
    info.sequence = order;
    info.editable = -1;
    if (is_URI_exist)
        info.url = (char *)uri;
    if (is_title_exist)
        info.title = (char *)title;

    ret = bp_bookmark_adaptor_easy_create(&id, &info);
    if (ret == 0) {
        BROWSER_LOGD("bp_bookmark_adaptor_easy_create is success");
        bp_bookmark_adaptor_publish_notification();
        return 1;
    }
    int errcode = bp_bookmark_adaptor_get_errorcode();
    BROWSER_LOGD("bp_bookmark_adaptor_easy_create is failed[%d]", errcode);
    return -1;
}

int BookmarkService::update_bookmark_notify(int id, const char *title, const char *uri, int parent_id, int order,
                                            bool is_duplicate_check_needed, bool is_URI_check_needed)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");
    int ret = update_bookmark(id, title, uri, parent_id, order, is_duplicate_check_needed, is_URI_check_needed);
    return ret;
}

bool BookmarkService::is_in_bookmark(const char *uri)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("uri[%s]", uri);

    int id = 0;
    return get_id(uri, &id);
}

bool BookmarkService::get_id(const char *uri, int *bookmark_id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    bp_bookmark_property_cond_fmt properties;
    properties.parent = -1;
    properties.type = 0;
    properties.is_operator = -1;
    properties.is_editable = -1;
    //conditions for querying
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = 1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_SEQUENCE;
    conds.ordering = 0;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;

    int *ids = nullptr;
    int ids_count = -1;
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, uri, 0);
    free(ids);
    bool result = ((ret >= 0) && (ids_count > 0));
    if (result) {
        *bookmark_id = ids[0];
    }
    return result;
}

bool BookmarkService::get_item_by_id(int id, BookmarkItem *item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("ID: %d", id);
    if (!item) {
        BROWSER_LOGE("item is nullptr");
        return false;
    }

    if (id == 0) {
        item->setTittle("Bookmarks");
        item->setAddress("");
        item->setId(id);
        item->setDir(-1);
        return true;
    }
    bp_bookmark_info_fmt info;
    if (bp_bookmark_adaptor_get_info(id, (BP_BOOKMARK_O_TYPE | BP_BOOKMARK_O_PARENT | BP_BOOKMARK_O_SEQUENCE |
                                     BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |BP_BOOKMARK_O_TITLE), &info) == 0) {
        item->setId(id);
        item->setDir(info.parent);

        if (info.url != nullptr && strlen(info.url) > 0)
            item->setAddress(info.url);

        if (info.title != nullptr && strlen(info.title) > 0)
            item->setTittle(info.title);

        bp_bookmark_adaptor_easy_free(&info);
        return true;
    }
    BROWSER_LOGD("bp_bookmark_adaptor_get_easy_all is failed");
    return false;
}

} /* end of namespace services*/
} /* end of namespace tizen_browser */
