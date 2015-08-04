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
    free_path_history();
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

    ret = bp_bookmark_adaptor_easy_create(&id, &info);
    if (ret < 0){
        BROWSER_LOGE("Error! Could not create new bookmark!");
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
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_URL, url.c_str(), 0);
    if (ids_count > 0){
        i = *ids;
    }
    free(ids);
    return i;
}

std::vector<std::shared_ptr<BookmarkItem> > BookmarkService::getBookmarks(int folder_id)
{
    BROWSER_LOGD("[%s:%d] folder_id = %d", __func__, __LINE__, folder_id);
    int *ids = nullptr;
    int ids_count = 0;
    int ret = bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id, 0, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0);
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
        std::string url = (bookmark_info.url ? bookmark_info.url : "");
        std::string title = (bookmark_info.title ? bookmark_info.title : "");

        std::shared_ptr<BookmarkItem> bookmark = std::make_shared<BookmarkItem>(url, title, std::string(""),(int) bookmark_info.parent, ids[i]);

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

std::vector<std::shared_ptr<BookmarkItem> > BookmarkService::getBookmarkFolders()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    int *ids = nullptr;
    int ids_count = 0;
    int ret = bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, -1, 1, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0);
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

        std::shared_ptr<BookmarkItem> bookmark = std::make_shared<BookmarkItem>(std::string(bookmark_info.url ? bookmark_info.url : ""),std::string(bookmark_info.title ? bookmark_info.title : ""), std::string(""),(int) bookmark_info.parent, ids[i]);
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
        if(!bookmark_info.is_operator)
           m_bookmarks.push_back(bookmark);
        else
           m_bookmarks.insert(m_bookmarks.begin(),bookmark);
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

int BookmarkService::get_root_folder_id(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    int root_id = 0;
    bp_bookmark_adaptor_get_root(&root_id);
    return root_id;
}

int BookmarkService::save_folder(const char *title, int *saved_folder_id, int parent_id, int by_operator)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    bp_bookmark_property_cond_fmt properties;
    properties.parent = parent_id;
    properties.type = 1;
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
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_TITLE, title, 0);
    free(ids);

    if (ret < 0)
        return -1;

    if (ids_count > 0) {
        return 0;
    }

    bp_bookmark_info_fmt info;
    info.type = 1;
    info.parent = parent_id;
    info.sequence = -1;
    info.is_operator = by_operator;
    info.access_count = -1;
    info.editable = 1;
    if (title != nullptr && strlen(title) > 0)
    {
        info.title = (char *)title;
    }
    ret = bp_bookmark_adaptor_easy_create(&id, &info);

    if (ret == 0) {
        ret = bp_bookmark_adaptor_set_sequence(id, -1); // max sequence
        if (ret == 0) {
           *saved_folder_id = id;
           BROWSER_LOGD("bmsvc_add_bookmark is success(id:%d)", *saved_folder_id);
           bp_bookmark_adaptor_publish_notification();
           return id;
        }
    }
    BROWSER_LOGD("bmsvc_add_bookmark is failed");
    return -1;
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
            if (ret < 0)
                return -1;
        }
        //This is a bookmark folder so check for any such folder with same title already exists
        else if (is_title_exist) {
            ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_TITLE, title, 0);
            free(ids);
            if (ret < 0)
                return -1;
        }

        if (ids_count > 0) {
            BROWSER_LOGD("same bookmark already exist");
            return 0;
        }
    }
    bp_bookmark_info_fmt info;
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

bool BookmarkService::is_in_folder(int parent_folder, const char *title)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");

    bp_bookmark_property_cond_fmt properties;
    properties.parent = parent_folder;
    properties.type = 1;
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
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_TITLE, title, 0);
    free(ids);
    return ((ret >= 0) && (ids_count > 0));
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

bool BookmarkService::get_folder_id(const char *title, int parent_id, int *folder_id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    bp_bookmark_property_cond_fmt properties;
    properties.parent = parent_id;
    properties.type = 1;
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
    int ret = bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds, BP_BOOKMARK_O_TITLE, title, 0);
    free(ids);
    bool result = ((ret >= 0) && (ids_count > 0));
    if (result) {
        *folder_id = ids[0];
    }
    return false;
}

bool BookmarkService::get_item_by_id(int id, BookmarkItem *item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("ID: %d", id);
    if (!item) {
        BROWSER_LOGE("item is nullptr");
        return false;
    }

    if (id == get_root_folder_id()) {
        item->setTittle("Bookmarks");
        item->setAddress("");
        item->setId(id);
        item->set_folder_flag(1);
        item->set_editable_flag(1);
        item->setDir(-1);
        return true;
    }
    bp_bookmark_info_fmt info;
    if (bp_bookmark_adaptor_get_info(id, (BP_BOOKMARK_O_TYPE | BP_BOOKMARK_O_PARENT | BP_BOOKMARK_O_SEQUENCE |
                                     BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |BP_BOOKMARK_O_TITLE), &info) == 0) {
        item->setId(id);
        item->set_folder_flag(static_cast<bool>(info.type));
        item->setDir(info.parent);
        item->set_editable_flag(static_cast<bool>(info.editable));

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

std::vector<BookmarkItem *>  BookmarkService::get_list_by_folder(const int folder_id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::vector<BookmarkItem *> list;
    BROWSER_LOGD("folder ID: %d", folder_id);
    int *ids = nullptr;
    int ids_count = -1;

    if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id, -1, -1, -1, BP_BOOKMARK_O_SEQUENCE, 1) < 0) {
        BROWSER_LOGD("bp_bookmark_adaptor_get_sequence_child_ids_p is failed");
        return list;
    }

    if (ids_count <= 0) {
        BROWSER_LOGD("bookmark list is empty");
        free(ids);
        return list;
    }

    bp_bookmark_info_fmt info;
    for(int i = 0; i < ids_count; i++) {
        if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
                BP_BOOKMARK_O_SEQUENCE |
                BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
                BP_BOOKMARK_O_TITLE | BP_BOOKMARK_O_IS_OPERATOR), &info) == 0) {
            BookmarkItem *item = new BookmarkItem();
            item->setId(ids[i]);

            item->set_folder_flag(static_cast<bool>(info.type));
            item->setDir(folder_id);
            item->set_editable_flag(static_cast<bool>(info.editable));

            if (info.url != nullptr && strlen(info.url) > 0)
                item->setAddress(info.url);

            if (info.title != nullptr && strlen(info.title) > 0)
                item->setTittle(info.title);

            list.push_back(item);
            bp_bookmark_adaptor_easy_free(&info);
        }
    }
    free(ids);
    return list;
}

bool BookmarkService::get_list_users_by_folder(const int folder_id, std::vector<BookmarkItem *> &list)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("folder ID: %d", folder_id);
    int *ids = nullptr;
    int ids_count = -1;
    if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id, -1, 0, -1, BP_BOOKMARK_O_SEQUENCE, 1) < 0) {
        BROWSER_LOGD("bp_bookmark_adaptor_get_ids_p is failed");
        return false;
    }

    if (ids_count <= 0) {
        BROWSER_LOGD("bookmark list is empty");
        free(ids);
        return false;
    }

    bp_bookmark_info_fmt info;
    for(int i = 0; i < ids_count; i++) {
        if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
                BP_BOOKMARK_O_SEQUENCE |
                BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
                BP_BOOKMARK_O_TITLE |
                BP_BOOKMARK_O_IS_OPERATOR), &info) == 0) {
            BookmarkItem *item = new BookmarkItem();
            item->setId(ids[i]);
            item->set_folder_flag(static_cast<bool>(info.type));
            item->setDir(folder_id);
            item->set_editable_flag(static_cast<bool>(info.editable));

            if (info.url != nullptr && strlen(info.url) > 0)
                item->setAddress(info.url);

            if (info.title != nullptr && strlen(info.title) > 0)
                item->setTittle(info.title);

            if (info.is_operator > 0) {
                delete item;
            } else {
                list.push_back(item);
            }
            bp_bookmark_adaptor_easy_free(&info);
        }
    }
    free(ids);
    return (list.empty() == false);
}

bool BookmarkService::get_list_by_dividing(const int folder_id, std::vector<BookmarkItem *> &list, int *list_item_count, int genlist_block_size)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("folder ID: %d", folder_id);

    int *ids = nullptr;
    int ids_count = -1;
    if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, genlist_block_size, *list_item_count, folder_id, -1, -1, -1,
                                      BP_BOOKMARK_O_SEQUENCE, 1) < 0) {
        BROWSER_LOGD("bp_bookmark_adaptor_get_ids_p is failed");
        return false;
    }

    if (ids_count <= 0) {
        BROWSER_LOGD("bookmark list is empty");
        free(ids);
        return false;
    }

    bp_bookmark_info_fmt info;
    for(int i = 0; i < ids_count; i++) {
        if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
                BP_BOOKMARK_O_SEQUENCE |
                BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
                BP_BOOKMARK_O_TITLE | BP_BOOKMARK_O_IS_OPERATOR), &info) == 0) {
            BookmarkItem *item = new BookmarkItem();
            item->setId(ids[i]);
            item->set_folder_flag(static_cast<bool>(info.type));
            item->setDir(folder_id);
            item->set_editable_flag(static_cast<bool>(info.editable));

            if (info.url != nullptr && strlen(info.url) > 0)
                item->setAddress(info.url);

            if (info.title != nullptr && strlen(info.title) > 0)
                item->setTittle(info.title);

            list.push_back(item);
            bp_bookmark_adaptor_easy_free(&info);
        }
    }
    free(ids);
    return true;
}

bool BookmarkService::get_list_users_by_dividing(const int folder_id, std::vector<BookmarkItem *> &list, int *list_item_count, int genlist_block_size)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("folder ID: %d", folder_id);
    int *ids = nullptr;
    int ids_count = -1;
    if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, genlist_block_size, *list_item_count, folder_id, -1, 0, 1,
                                      BP_BOOKMARK_O_SEQUENCE, 1) < 0) {
        BROWSER_LOGD("bp_bookmark_adaptor_get_ids_p is failed");
        return false;
    }

    if (ids_count <= 0) {
        BROWSER_LOGD("bookmark list is empty");
        free(ids);
        return false;
    }

    bp_bookmark_info_fmt info;
    BROWSER_LOGD("list.size() before : %d", list.size());
    BROWSER_LOGD("ids_count: %d", ids_count);
    for(int i = 0; i < ids_count; i++) {
        BROWSER_LOGD("list.size(): %d", list.size());
        BROWSER_LOGD("index : %d", i);
        if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
                BP_BOOKMARK_O_SEQUENCE |
                BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
                BP_BOOKMARK_O_TITLE |
                BP_BOOKMARK_O_IS_OPERATOR), &info) == 0) {
            BookmarkItem *item = new BookmarkItem();
            item->setId(ids[i]);
            item->set_folder_flag(static_cast<bool>(info.type));
            item->setDir(folder_id);
            item->set_editable_flag(static_cast<bool>(info.editable));

            if (info.url != nullptr && strlen(info.url) > 0)
                item->setAddress(info.url);

            if (info.title != nullptr && strlen(info.title) > 0)
                item->setTittle(info.title);

            if (info.is_operator > 0) {
                BROWSER_LOGD("this is operator bookmark");
                delete item;
            } else {
                BROWSER_LOGD("item is pushed");
                list.push_back(item);
            }
            bp_bookmark_adaptor_easy_free(&info);
        }
    }
    BROWSER_LOGD("list.size() after: %d", list.size());
    free(ids);
    return (list.empty() == false);
}

int BookmarkService::_get_folder_count(const int folder_id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("folder ID: %d", folder_id);

    int *ids = nullptr;
    int ids_count = -1;
    bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id, 1, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0);
    free(ids);
    return ids_count;
}


int BookmarkService::_get_bookmarks_count(const int folder_id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("folder ID: %d", folder_id);

    int *ids = nullptr;
    int ids_count = -1;
    bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id, 0, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0);
    free(ids);
    return ids_count;
}


int BookmarkService::_get_total_contents_count(const int folder_id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("folder ID: %d", folder_id);

    int *ids = nullptr;
    int ids_count = -1;
    bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id, 0, -1, -1, BP_BOOKMARK_O_SEQUENCE, 0);
    free(ids);
    return ids_count;
}

bool BookmarkService::get_list_operators(const int folder_id, std::vector<BookmarkItem *> &list)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("folder ID: %d", folder_id);
    int *ids = nullptr;
    int ids_count = -1;

    if (bp_bookmark_adaptor_get_ids_p(&ids, &ids_count, -1, 0, folder_id, -1, 1, -1, BP_BOOKMARK_O_SEQUENCE, 1) < 0) {
        BROWSER_LOGD("bp_bookmark_adaptor_get_ids_p is failed");
        return false;
    }

    if (ids_count <= 0) {
        BROWSER_LOGD("bookmark list is empty");
        free(ids);
        return false;
    }

    bp_bookmark_info_fmt info;
    for(int i = 0; i < ids_count; i++) {
        if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
                BP_BOOKMARK_O_SEQUENCE |
                BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
                BP_BOOKMARK_O_TITLE), &info) == 0) {
            BookmarkItem *item = new BookmarkItem();
            item->setId(ids[i]);
            item->set_folder_flag(static_cast<bool>(info.type));
            item->setDir(folder_id);
            item->set_editable_flag(static_cast<bool>(info.editable));

            if (info.url != nullptr && strlen(info.url) > 0)
                item->setAddress(info.url);

            if (info.title != nullptr && strlen(info.title) > 0)
                item->setTittle(info.title);

            list.push_back(item);
            bp_bookmark_adaptor_easy_free(&info);
        }
    }
    free(ids);
    return (list.empty() == false);
}

bool BookmarkService::get_list_by_keyword(const char *keyword, std::vector<BookmarkItem *> &list)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");

    if (!keyword || (strlen(keyword) == 0)) {
        BROWSER_LOGD("keyword is nullptr");
        return false;
    }

    std::string buf_str(keyword);
    buf_str = "%" + buf_str + "%";

    bp_bookmark_property_cond_fmt properties;
    properties.parent = -1;
    properties.type = 0;
    properties.is_operator = -1;
    properties.is_editable = -1;
    //conditions for querying
    bp_bookmark_rows_cond_fmt conds;
    conds.limit = -1;
    conds.offset = 0;
    conds.order_offset = BP_BOOKMARK_O_TITLE;
    conds.ordering = -1;
    conds.period_offset = BP_BOOKMARK_O_DATE_CREATED;
    conds.period_type = BP_BOOKMARK_DATE_ALL;

    int *ids = nullptr;
    int ids_count = -1;
    if (bp_bookmark_adaptor_get_cond_ids_p(&ids, &ids_count, &properties, &conds,
        (BP_BOOKMARK_O_TITLE | BP_BOOKMARK_O_URL), buf_str.c_str(), 1) < 0) {
        BROWSER_LOGE("bp_bookmark_adaptor_get_cond_ids_p is failed");
        return false;
    }

    if (ids_count <= 0) {
        BROWSER_LOGD("bookmark list is empty");
        free(ids);
        return false;
    }

    bp_bookmark_info_fmt info;
    for(int i = 0; i < ids_count; i++) {
        if (bp_bookmark_adaptor_get_info(ids[i], (BP_BOOKMARK_O_TYPE |
                BP_BOOKMARK_O_PARENT | BP_BOOKMARK_O_SEQUENCE |
                BP_BOOKMARK_O_IS_EDITABLE | BP_BOOKMARK_O_URL |
                BP_BOOKMARK_O_TITLE), &info) == 0) {
            BookmarkItem *item = new BookmarkItem();
            item->setId(ids[i]);
            item->set_folder_flag(static_cast<bool>(info.type));
            item->setDir(info.parent);
            item->set_editable_flag(static_cast<bool>(info.editable));

            if (info.url != nullptr && strlen(info.url) > 0)
                item->setAddress(info.url);

            if (info.title != nullptr && strlen(info.title) > 0)
                item->setTittle(info.title);

            list.push_back(item);
            bp_bookmark_adaptor_easy_free(&info);
        }
    }
    free(ids);
    return true;
}

void BookmarkService::destroy_list(std::vector<BookmarkItem *> &list)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");

    for(unsigned int i = 0 ; i < list.size() ; i++) {
        delete list[i];
    }
    list.clear();
}

bool BookmarkService::get_folder_depth_count(int *depth_count)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("depth_count: %d", *depth_count);
    return _get_depth_count_recursive(get_root_folder_id(), 0, depth_count);
}

bool BookmarkService::_get_depth_count_recursive(int folder_id, int cur_depth, int *depth_count)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("current_depth: %d, depth_count:%d", cur_depth, *depth_count);

    std::vector<BookmarkItem *> bookmark_list = get_list_by_folder(folder_id);
    if (!bookmark_list.empty()) {
        BROWSER_LOGE("get_list_by_folder is failed(folder id:%d)",folder_id);
        return false;
    }

    for(unsigned int j = 0 ; j < bookmark_list.size() ; j++) {
        if (bookmark_list[j]->is_folder()) {
            /* Folder item is found. get sub list */
            if ((cur_depth+1) > *depth_count)
                *depth_count = cur_depth+1;

            _get_depth_count_recursive(bookmark_list[j]->getId(), cur_depth+1, depth_count);
        }
    }
    destroy_list(bookmark_list);
    return true;
}

bool BookmarkService::set_thumbnail(int id, Evas_Object *thumbnail)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("id : %d", id);

    int w = 0;
    int h = 0;
    int stride = 0;
    int len = 0;

    //platform_Service ps.
    //ps.evas_image_size_get(thumbnail, &w, &h, &stride);
    len = stride * h;
    BROWSER_LOGD("thumbnail w=[%d], h=[%d], stride=[%d], len=[%d]", w, h, stride, len);

    if (len == 0)
        return false;

    void *thumbnail_data = evas_object_image_data_get(thumbnail, true);
    int ret = bp_bookmark_adaptor_set_snapshot(id, w, h, (const unsigned char *)thumbnail_data, len);
    if (ret < 0) {
        BROWSER_LOGE("bp_bookmark_adaptor_set_thumbnail is failed");
        return false;
    }
    return true;
}

Evas_Object *BookmarkService::get_thumbnail(int id, Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("id : %d", id);
    void *thumbnail_data = nullptr;
    int w = 0;
    int h = 0;
    int len = 0;
    Evas_Object *icon = nullptr;

    int ret = bp_bookmark_adaptor_get_snapshot(id, &w, &h, (unsigned char **)&thumbnail_data, &len);
    BROWSER_LOGE("len: %d, w:%d, h:%d", len, w, h);
    if (len > 0){
        /* gengrid event area has scaled with original image if it is evas image.
            therefore use elm_image*/
        icon = elm_image_add(parent);
        Evas_Object *icon_object = elm_image_object_get(icon);
        evas_object_image_colorspace_set(icon_object, EVAS_COLORSPACE_ARGB8888);
        evas_object_image_size_set(icon_object, w, h);
        evas_object_image_fill_set(icon_object, 0, 0, w, h);
        evas_object_image_filled_set(icon_object, true);
        evas_object_image_alpha_set(icon_object,true);

        void *target_image_data = evas_object_image_data_get(icon_object, true);
        memcpy(target_image_data, thumbnail_data, len);
        evas_object_image_data_set(icon_object, target_image_data);
    }

    return icon;
}

bool BookmarkService::set_favicon(int id, Evas_Object *favicon)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("id : %d", id);

    int w = 0;
    int h = 0;
    int stride = 0;
    int len = 0;

    //platform_Service ps.
    //ps.evas_image_size_get(favicon, &w, &h, &stride);
    len = stride * h;
    BROWSER_LOGD("favicon w=[%d], h=[%d], stride=[%d], len=[%d]", w, h, stride, len);

    if (len == 0)
        return false;

    void *favicon_data = evas_object_image_data_get(favicon, true);
    int ret = bp_bookmark_adaptor_set_icon(id, w, h, (const unsigned char *)favicon_data, len);
    if (ret < 0) {
        BROWSER_LOGE("bp_bookmark_adaptor_set_favicon is failed");
        return false;
    }
    return true;
}

Evas_Object *BookmarkService::get_favicon(int id, Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("id : %d", id);
    void *favicon_data = nullptr;
    int w = 0;
    int h = 0;
    int len = 0;

    Evas *e = nullptr;
    e = evas_object_evas_get(parent);
    if (!e) {
        BROWSER_LOGE("canvas is nullptr");
        return nullptr;
    }
    int ret = bp_bookmark_adaptor_get_icon(id, &w, &h, (unsigned char **)&favicon_data, &len);
    if (ret < 0) {
        BROWSER_LOGE("bp_bookmark_adaptor_set_favicon is failed");
        return nullptr;
    }

    Evas_Object *icon = nullptr;
    BROWSER_LOGE("len: %d, w:%d, h:%d", len, w, h);
    if (len > 0){
        icon = evas_object_image_filled_add(e);
        if (w == 0 || h == 0) {
            // Android bookmark.
            evas_object_image_memfile_set(icon, favicon_data, len, nullptr, nullptr);
        } else {
            // Tizen bookmark.
            evas_object_image_colorspace_set(icon, EVAS_COLORSPACE_ARGB8888);
            evas_object_image_size_set(icon, w, h);
            evas_object_image_fill_set(icon, 0, 0, w, h);
            evas_object_image_filled_set(icon, true);
            evas_object_image_alpha_set(icon,true);

            void *target_image_data = evas_object_image_data_get(icon, true);
            memcpy(target_image_data, favicon_data, len);
            evas_object_image_data_set(icon, target_image_data);
        }
    }
    return icon;
}

bool BookmarkService::set_webicon(int id, Evas_Object *webicon)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("id : %d", id);

    int w = 0;
    int h = 0;
    int stride = 0;
    int len = 0;

    //platform_Service ps;
    //ps.evas_image_size_get(webicon, &w, &h, &stride);
    len = stride * h;
    BROWSER_LOGD("webicon w=[%d], h=[%d], stride=[%d], len=[%d]", w, h, stride, len);

    if (len == 0)
        return false;

    void *webicon_data = evas_object_image_data_get(webicon, true);
    int ret = bp_bookmark_adaptor_set_webicon(id, w, h, (const unsigned char *)webicon_data, len);
    if (ret != 0)
        BROWSER_LOGE("set webicon is failed");
    return false;
}

Evas_Object *BookmarkService::get_webicon(int id, Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("id : %d", id);
    void *webicon_data = nullptr;
    int w = 0;
    int h = 0;
    int len = 0;

    int ret = bp_bookmark_adaptor_get_webicon(id, &w, &h, (unsigned char **)&webicon_data, &len);
    if (ret != 0) {
        BROWSER_LOGE("bp_bookmark_adaptor_set_webicon is failed");
        return nullptr;
    }

    Evas_Object *icon = nullptr;
    BROWSER_LOGE("len: %d, w:%d, h:%d", len, w, h);
    if (len > 0){
        Evas *e = evas_object_evas_get(parent);
        icon = evas_object_image_filled_add(e);

        evas_object_image_colorspace_set(icon, EVAS_COLORSPACE_ARGB8888);
        evas_object_image_size_set(icon, w, h);
        evas_object_image_fill_set(icon, 0, 0, w, h);
        evas_object_image_filled_set(icon, true);
        evas_object_image_alpha_set(icon,true);

        void *target_image_data = evas_object_image_data_get(icon, true);
        memcpy(target_image_data, webicon_data, len);
        evas_object_image_data_set(icon, target_image_data);
    }

    return icon;
}

bool BookmarkService::set_access_count(int id, int count)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");
    return !bp_bookmark_adaptor_set_access_count(id, count);
}

bool BookmarkService::get_access_count(int id, int *count)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");
    return !bp_bookmark_adaptor_get_access_count(id, count);
}

bool BookmarkService::increase_access_count(int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");
    int count;
    if(get_access_count(id, &count) == false) {
        BROWSER_LOGD("get_access_count is failed");
        return false;
    }

    if(set_access_count(id, count + 1) == false) {
        BROWSER_LOGD("set_access_count is failed");
        return false;
    }

    return true;
}

bool BookmarkService::set_last_sequence(int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");
    return (bp_bookmark_adaptor_set_sequence(id, -1) == -1);
}

const char* BookmarkService::get_path_info(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("%s", m_path_string.c_str());
    return m_path_string.c_str();
}

BookmarkService::folder_info *BookmarkService::get_path_by_index(unsigned int index)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("%s", m_path_history[index]->folder_name);
    return m_path_history[index];
}

int BookmarkService::get_path_size(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("%d", m_path_history.size());
    return m_path_history.size();
}

void BookmarkService::push_back_path(folder_info *item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("path size before push: %d", m_path_history.size());
    m_path_history.push_back(item);
}

void BookmarkService::pop_back_path(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("path size before pop: %d", m_path_history.size());
    m_path_history.pop_back();
}

void BookmarkService::clear_path_history(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");
    free_path_history();
    m_path_history.clear();
}

void BookmarkService::free_path_history(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");
    for(unsigned int i = 0 ; i < m_path_history.size() ; i++) {
        if (m_path_history[i]) {
            if (m_path_history[i]->folder_name)
                free(m_path_history[i]->folder_name);

            free(m_path_history[i]);
        }
    }
}

void BookmarkService::change_path_lang(void)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");
    m_path_string.clear();

    char *old_str = m_path_history[0]->folder_name;
    m_path_history[0]->folder_name = strdup("Bookmarks");
    free(old_str);

    for(int i = 0 ; i < get_path_size(); i++) {
        if (get_path_by_index(i)) {
            if (m_path_string.empty()) {
                m_path_string = m_path_history[0]->folder_name;
            } else {
                m_path_string += "/";
                m_path_string += get_path_by_index(i)->folder_name;
            }
        }
    }
    BROWSER_LOGD("str: %s", m_path_string.c_str());
}

void BookmarkService::path_into_sub_folder(int folder_id, const char *folder_name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");

    folder_info *item = (folder_info *)calloc(1, sizeof(folder_info));

    item->folder_id = folder_id;
    if (folder_id == get_root_folder_id()) {
        item->folder_name = strdup("Bookmarks");
    } else
        item->folder_name = strdup(folder_name);
    push_back_path(item);

    m_path_string.clear();
    for(int i = 0 ; i < get_path_size() ; i++) {
        if (get_path_by_index(i)) {
            if (m_path_string.empty()) {
                m_path_string = get_path_by_index(i)->folder_name;
            } else {
                m_path_string += "/";
                m_path_string += get_path_by_index(i)->folder_name;
            }
        }
    }
    char *path_string = (elm_entry_utf8_to_markup(m_path_string.c_str()));
    m_path_string = path_string;
    free(path_string);
    BROWSER_LOGD("m_path_string: %s", m_path_string.c_str());

    m_curr_folder = folder_id;
}

bool BookmarkService::path_to_upper_folder(int *curr_folder)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("");

    int current_depth = get_path_size();
    BROWSER_LOGD("current_depth: %d", current_depth);

    if (current_depth <= 0) {
        BROWSER_LOGE("[ERROR] top folder is not valid");
        return true;
    }

    int curr_depth = current_depth - 1;
    if (curr_depth > 0) {
        *curr_folder = get_path_by_index(curr_depth - 1)->folder_id;
        if (get_path_by_index(curr_depth) && get_path_by_index(curr_depth)->folder_name) {
            free(get_path_by_index(curr_depth)->folder_name);
            free(get_path_by_index(curr_depth));
        }
        pop_back_path();
    } else {
        /* Current folder is root folder */
        if (*curr_folder != get_root_folder_id()) {
            BROWSER_LOGE("[ERROR] top folder is not root folder");
            return true;
        }
        if (get_path_by_index(curr_depth) && get_path_by_index(curr_depth)->folder_name) {
            free(get_path_by_index(curr_depth)->folder_name);
            free(get_path_by_index(curr_depth));
        }
        pop_back_path();
        m_path_string.clear();
        return false;
    }

    m_path_string.clear();
    for(int i = 0 ; i < get_path_size() ; i++) {
        if (get_path_by_index(i)) {
            if (m_path_string.empty()) {
                m_path_string = get_path_by_index(i)->folder_name;
            } else {
                m_path_string += "/";
                m_path_string += get_path_by_index(i)->folder_name;
            }
        }
    }
    BROWSER_LOGD("m_path_string: %s", m_path_string.c_str());

    m_curr_folder = *curr_folder;
    return true;
}

} /* end of namespace services*/
} /* end of namespace tizen_browser */
