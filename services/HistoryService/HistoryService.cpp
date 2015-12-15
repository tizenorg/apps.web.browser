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

#include <string>
#include <BrowserAssert.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/date.hpp>
#include <boost/date_time/date_defs.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "ServiceManager.h"
#include "HistoryService.h"
#include "HistoryItem.h"
#include "AbstractWebEngine.h"

#include "EflTools.h"

#include "Tools/GeneralTools.h"
#include "Tools/StringTools.h"
#include "HistoryServiceTools.h"
#include "Tools/CapiWebErrorCodes.h"

namespace tizen_browser
{
namespace services
{

EXPORT_SERVICE(HistoryService, DOMAIN_HISTORY_SERVICE)

const int SEARCH_LIKE = 1;

HistoryService::HistoryService() : m_testDbMod(false)
{
    BROWSER_LOGD("HistoryService");
}

HistoryService::~HistoryService()
{
}

void HistoryService::setStorageServiceTestMode(bool testmode) {
	m_testDbMod = testmode;
}

void HistoryService::errorPrint(std::string method) const
{
    int error_code = bp_history_adaptor_get_errorcode();
    BROWSER_LOGE("%s error: %d (%s)", method.c_str(), error_code,
            tools::capiWebError::historyErrorToString(error_code).c_str());
}

int HistoryService::getHistoryItemsCount(){
    int *ids = nullptr;
    int count=0;
    bp_history_rows_cond_fmt conds;
    conds.limit = 20;  //no of rows to get negative means no limitation
    conds.offset = -1;   //the first row's index
    conds.order_offset =BP_HISTORY_O_DATE_CREATED; // property to sort
    conds.ordering = 1; //way of ordering 0 asc 1 desc
    conds.period_offset = BP_HISTORY_O_DATE_CREATED;
    conds.period_type = BP_HISTORY_DATE_TODAY;
    if (bp_history_adaptor_get_cond_ids_p(&ids, &count, &conds, 0, nullptr, 0)
            < 0) {
        errorPrint("bp_history_adaptor_get_cond_ids_p");
    }

    BROWSER_LOGD("[%s:%d] History Count %d", __PRETTY_FUNCTION__, __LINE__, count);
    return count;
}

bool HistoryService::isDuplicate(const char* url) const
{
    M_ASSERT(url);
    int *ids=nullptr;
    int count=0;
    bp_history_rows_cond_fmt conds;
    conds.limit = 20;  //no of rows to get negative means no limitation
    conds.offset = -1;   //the first row's index
    conds.order_offset =BP_HISTORY_O_DATE_CREATED; // property to sort
    conds.ordering = 1; //way of ordering 0 asc 1 desc
    conds.period_offset = BP_HISTORY_O_DATE_CREATED;
    conds.period_type = BP_HISTORY_DATE_TODAY;
    if(bp_history_adaptor_get_cond_ids_p(&ids ,&count, &conds, 0, nullptr, 0) < 0 ) {
        errorPrint("bp_history_adaptor_get_cond_ids_p");
    }

    bp_history_offset offset = (BP_HISTORY_O_URL | BP_HISTORY_O_DATE_CREATED);

    for (int i = 0; i < count; i++) {
        bp_history_info_fmt history_info;
        bp_history_adaptor_get_info(ids[i], offset, &history_info);
        if (!history_info.url) {
            BROWSER_LOGD("Warning: history entry without url!");
        } else if (!strcmp(history_info.url, url)) {
            int freq;
            bp_history_adaptor_get_frequency(ids[i], &freq);
            bp_history_adaptor_set_frequency(ids[i], freq + 1);
            bp_history_adaptor_set_date_visited(ids[i],-1);
            return true;
        }
    }
    return false;
}

std::shared_ptr<HistoryItemVector> HistoryService::getHistoryAll()
{
    return getHistoryItems(BP_HISTORY_DATE_ALL);
}

std::shared_ptr<HistoryItemVector> HistoryService::getHistoryToday()
{
    return getHistoryItems(BP_HISTORY_DATE_TODAY);
}

std::shared_ptr<HistoryItemVector> HistoryService::getHistoryYesterday()
{
    return getHistoryItems(BP_HISTORY_DATE_YESTERDAY);
}

std::shared_ptr<HistoryItemVector> HistoryService::getHistoryLastWeek()
{
    return getHistoryItems(BP_HISTORY_DATE_LAST_7_DAYS);
}

std::shared_ptr<HistoryItemVector> HistoryService::getHistoryLastMonth()
{
    return getHistoryItems(BP_HISTORY_DATE_LAST_MONTH);
}
std::shared_ptr<HistoryItemVector> HistoryService::getHistoryOlder()
{
    return getHistoryItems(BP_HISTORY_DATE_OLDER);
}

std::shared_ptr<HistoryItemVector> HistoryService::getMostVisitedHistoryItems()
{
    std::shared_ptr<HistoryItemVector> ret_history_list(new HistoryItemVector);

    int *ids=nullptr;
    int count=-1;
    bp_history_rows_cond_fmt conds;
    conds.limit = 5;  //no of rows to get negative means no limitation
    conds.offset = -1;   //the first row's index
    conds.order_offset = BP_HISTORY_O_FREQUENCY; // property to sort
    conds.ordering = 1; //way of ordering 0 asc 1 desc
    conds.period_offset = BP_HISTORY_O_DATE_CREATED;
    //TODO: consider to change below line to BP_HISTORY_DATE_LAST_MONTH
    conds.period_type = BP_HISTORY_DATE_ALL; // set from which period most visited sites are generated

    if(bp_history_adaptor_get_cond_ids_p(&ids ,&count, &conds, 0, nullptr, 0) < 0 ) {
        errorPrint("bp_history_adaptor_get_cond_ids_p");
    }

    bp_history_offset offset = (BP_HISTORY_O_URL | BP_HISTORY_O_TITLE | BP_HISTORY_O_FREQUENCY | BP_HISTORY_O_FAVICON | BP_HISTORY_O_DATE_CREATED | BP_HISTORY_O_THUMBNAIL);

    int freq_arr[1000];
    for(int i = 0; i< count; i++){
        int freq;
        if (0 == bp_history_adaptor_get_frequency(ids[i], &freq))
        {
            freq_arr[i] = freq;
        }
    }

    int index_array[6];
    int j=0;
    int maximum = freq_arr[0];
    int position = 0;

    for(int k=1; k<=5;k++){
        if(k > count || count == 0)
            break;

    maximum = freq_arr[0];
    position = 0;

    for(int i =1;i<count;i++){
        if (freq_arr[i] > maximum)
            {
                maximum  = freq_arr[i];
                position = i;
            }
         }
        index_array[j++] = position;
        freq_arr[position] = -1;
    }

    for(int i = 0; i < j; i++){
        bp_history_info_fmt history_info;
        bp_history_adaptor_get_info(ids[index_array[i]],offset,&history_info);

        std::shared_ptr<HistoryItem> history = std::make_shared<HistoryItem>(std::string(history_info.url));
        history->setUrl(std::string(history_info.url ? history_info.url : ""));
        history->setTitle(std::string(history_info.title ? history_info.title : ""));

        //thumbnail
        if (history_info.thumbnail_length != -1) {
            std::shared_ptr<tizen_browser::tools::BrowserImage> hi = std::make_shared<tizen_browser::tools::BrowserImage>();
            hi->imageType = tools::BrowserImage::ImageTypePNG;
            hi->width = history_info.thumbnail_width;
            hi->height = history_info.thumbnail_height;
            hi->dataSize = history_info.thumbnail_length;
            hi->imageData = (void*)malloc(history_info.thumbnail_length);
            memcpy(hi->imageData, (void*)history_info.thumbnail, history_info.thumbnail_length);
            history->setThumbnail(hi);
        } else {
            BROWSER_LOGD("history thumbnail lenght is -1");
        }
        if(history_info.frequency > 0)
            ret_history_list->push_back(history);
    }

    free(ids);
    return ret_history_list;
}

void HistoryService::cleanMostVisitedHistoryItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    int *ids=nullptr;
    int count=-1;
    bp_history_rows_cond_fmt conds;
    conds.limit = -1;  // no of rows to get negative means no limitation
    conds.offset = -1;   // the first row's index
    conds.order_offset = BP_HISTORY_O_ALL; // property to sort
    conds.ordering = 1; // way of ordering 0 asc 1 desc
    conds.period_offset = BP_HISTORY_O_DATE_CREATED;
    conds.period_type = BP_HISTORY_DATE_ALL; // set from which period most visited sites are generated

    if(bp_history_adaptor_get_cond_ids_p(&ids ,&count, &conds, 0, nullptr, 0) < 0 ) {
        errorPrint("bp_history_adaptor_get_cond_ids_p");
        return;
    }

    for(int i = 0; i < count; i++){
            bp_history_adaptor_set_frequency(ids[i], 0);
    }
    BROWSER_LOGD("Deleted Most Visited Sites!");
}

std::shared_ptr<HistoryItemVector> HistoryService::getHistoryItemsByKeyword(
        const std::string & keyword, int maxItems)
{
    std::string search("%" + keyword + "%");    // add SQL 'any character' signs

    std::shared_ptr<HistoryItemVector> items(new HistoryItemVector);
    int *ids=nullptr;
    int count=-1;
    bp_history_rows_cond_fmt conds;
    conds.limit = maxItems;  //no of rows to get negative means no limitation
    conds.offset = -1;   //the first row's index
    conds.order_offset = BP_HISTORY_O_DATE_VISITED; // property to sort
    conds.ordering = 1; //way of ordering 0 asc 1 desc
    conds.period_offset = BP_HISTORY_O_DATE_VISITED;
    conds.period_type = BP_HISTORY_DATE_ALL;

    if(bp_history_adaptor_get_cond_ids_p(&ids ,&count, &conds, BP_HISTORY_O_URL, search.c_str(), SEARCH_LIKE) < 0) {
        errorPrint("bp_history_adaptor_get_cond_ids_p");
        return items;
    }

    bp_history_offset offset = (BP_HISTORY_O_URL | BP_HISTORY_O_TITLE | BP_HISTORY_O_DATE_VISITED);
    for(int i = 0; i < count; i++) {
        bp_history_info_fmt history_info;
        bp_history_adaptor_get_info(ids[i], offset, &history_info);

        std::shared_ptr<HistoryItem> history = std::make_shared<HistoryItem>(std::string(history_info.url));
        history->setTitle(std::string(history_info.title ? history_info.title : ""));

        items->push_back(history);
    }

    free(ids);
    return items;
}

void HistoryService::addHistoryItem(std::shared_ptr<HistoryItem> his,std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail){
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    his->setFavIcon(his->getFavIcon());
    his->setThumbnail(thumbnail);

    if (isDuplicate(his->getUrl().c_str()))
        return;

    int id = -1;
    if(bp_history_adaptor_create(&id) < 0) {
        errorPrint("bp_history_adaptor_create");
    }

    int *ids=nullptr;
    int count=-1;
    bp_history_rows_cond_fmt conds;
    conds.limit = 20;  //no of rows to get negative means no limitation
    conds.offset = -1;   //the first row's index
    conds.order_offset =BP_HISTORY_O_DATE_CREATED; // property to sort
    conds.ordering = 1; //way of ordering 0 asc 1 desc
    conds.period_offset = BP_HISTORY_O_DATE_CREATED;
    conds.period_type = BP_HISTORY_DATE_TODAY;

    if(bp_history_adaptor_get_cond_ids_p(&ids ,&count, &conds, 0, nullptr, 0) < 0) {
        errorPrint("bp_history_adaptor_get_cond_ids_p");
    }

    bp_history_adaptor_set_url(id, (his->getUrl()).c_str());
    bp_history_adaptor_set_title(id, (his->getTitle()).c_str());
    bp_history_adaptor_set_date_visited(id,-1);
    bp_history_adaptor_set_frequency(id, 1);

    if (thumbnail) {
       std::unique_ptr<tizen_browser::tools::Blob> thumb_blob = tizen_browser::tools::EflTools::getBlobPNG(thumbnail);
       unsigned char * thumb = std::move((unsigned char*)thumb_blob->getData());
       bp_history_adaptor_set_snapshot(id, thumbnail->width, thumbnail->height, thumb, thumb_blob->getLength());
    }

    std::shared_ptr<tizen_browser::tools::BrowserImage> favicon = his->getFavIcon();
    if (favicon) {
       std::unique_ptr<tizen_browser::tools::Blob> favicon_blob = tizen_browser::tools::EflTools::getBlobPNG(favicon);
       unsigned char * fav = std::move((unsigned char*)favicon_blob->getData());
       bp_history_adaptor_set_icon(id, favicon->width, favicon->height, fav, favicon_blob->getLength());
    }

    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    historyAdded(his);
}

void HistoryService::clearAllHistory()
{
    bp_history_adaptor_reset();
    history_list.clear();
    historyAllDeleted();
}

int HistoryService::getHistoryId(const std::string & url)
{
    bp_history_rows_cond_fmt conds;
    conds.limit = -1;
    conds.offset = 0;
    conds.order_offset = BP_HISTORY_O_DATE_CREATED;
    conds.ordering = 0;
    conds.period_offset = BP_HISTORY_O_DATE_CREATED;
    conds.period_type = BP_HISTORY_DATE_ALL;
    int *ids = nullptr;
    int ids_count = 0;
    if(bp_history_adaptor_get_cond_ids_p(&ids ,&ids_count, &conds, BP_HISTORY_O_URL, url.c_str(), 0) < 0) {
        errorPrint("bp_history_adaptor_get_cond_ids_p");
    } else if (ids_count != 0) {
        int i = *ids;
        free(ids);
        return i;
    }
    return 0;
}

void HistoryService::clearURLHistory(const std::string & url)
{
    int id = getHistoryId(url);
    if (id!=0)
        bp_history_adaptor_delete(id);
    if(0 == getHistoryItemsCount())
        historyEmpty(true);
    historyDeleted(url);
}

std::shared_ptr<HistoryItem> HistoryService::getHistoryItem(int * ids, int idNumber)
{
    bp_history_offset offset = (BP_HISTORY_O_URL | BP_HISTORY_O_TITLE | BP_HISTORY_O_FAVICON | BP_HISTORY_O_DATE_VISITED);
    bp_history_info_fmt history_info;
    bp_history_adaptor_get_info(ids[idNumber], offset, &history_info);

    int date;
    bp_history_adaptor_get_date_created(ids[idNumber], &date);

    struct tm *item_time_info;
    time_t item_time = (time_t) date;
    item_time_info = localtime(&item_time);

    int m_year = item_time_info->tm_year;
    int m_month = item_time_info->tm_mon + 1;
    int m_month_day = item_time_info->tm_mday;
    int min = item_time_info->tm_min;
    int hour = item_time_info->tm_hour;
    int sec = item_time_info->tm_sec;

    m_year = 2000 + m_year % 100;

    std::shared_ptr<HistoryItem> history = std::make_shared <HistoryItem> (std::string(history_info.url));
    boost::gregorian::date d(m_year, m_month, m_month_day);
    boost::posix_time::ptime t(d, boost::posix_time::time_duration(hour, min, sec));
    history->setLastVisit(t);
    history->setUrl(std::string(history_info.url ? history_info.url : ""));
    history->setTitle(std::string(history_info.title ? history_info.title : ""));

    //thumbail
    std::shared_ptr<tizen_browser::tools::BrowserImage> hi = std::make_shared<tizen_browser::tools::BrowserImage>();
    hi->imageType = tizen_browser::tools::BrowserImage::ImageType::ImageTypePNG;
    hi->width = history_info.thumbnail_width;
    hi->height = history_info.thumbnail_height;
    hi->dataSize = history_info.thumbnail_length;
    hi->imageData = (void*) malloc(history_info.thumbnail_length);
    memcpy(hi->imageData, (void*) history_info.thumbnail, history_info.thumbnail_length);
    history->setThumbnail(hi);

    return history;
}

std::shared_ptr<HistoryItem> HistoryService::getCurrentTab()
{
    int *ids=nullptr;
    int count = -1;
    bp_history_rows_cond_fmt conds;
    conds.limit = 20;  //no of rows to get negative means no limitation
    conds.offset = -1;   //the first row's index
    conds.order_offset = BP_HISTORY_O_DATE_VISITED; // property to sort
    conds.ordering = 1; //way of ordering 0 asc 1 desc
    conds.period_offset = BP_HISTORY_O_DATE_VISITED;
    conds.period_type = BP_HISTORY_DATE_TODAY;

    if(bp_history_adaptor_get_cond_ids_p(&ids , &count, &conds, 0, nullptr, 0) < 0) {
        errorPrint("bp_history_adaptor_get_cond_ids_p");
    }

    return getHistoryItem(ids);
}

std::shared_ptr<HistoryItemVector> HistoryService::getHistoryItems(bp_history_date_defs period)
{
    std::shared_ptr<HistoryItemVector> ret_history_list(new HistoryItemVector);

    int *ids=nullptr;
    int count=-1;
    bp_history_rows_cond_fmt conds;
    conds.limit = 20;  //no of rows to get negative means no limitation
    conds.offset = -1;   //the first row's index
    conds.order_offset = BP_HISTORY_O_DATE_VISITED; // property to sort
    conds.ordering = 1; //way of ordering 0 asc 1 desc
    conds.period_offset = BP_HISTORY_O_DATE_VISITED;
    conds.period_type = period;

    if(bp_history_adaptor_get_cond_ids_p(&ids ,&count, &conds, 0, nullptr, 0) < 0) {
        errorPrint("bp_history_adaptor_get_cond_ids_p");
    }

    for(int i = 0; i< count; i++) {
        ret_history_list->push_back(getHistoryItem(ids, i));
    }
    free(ids);
    return ret_history_list;
}

std::shared_ptr<HistoryItemVector> HistoryService::getHistoryItemsByURL(
        const std::string& url, int maxItems)
{
    return getHistoryItemsByKeyword(tools::extractDomain(url), maxItems);
}

std::shared_ptr<HistoryItemVector> HistoryService::getHistoryItemsByKeywordsString(
        const std::string& keywordsString, const int maxItems,
        const unsigned int minKeywordLength)
{
    if (keywordsString.empty())
        return std::make_shared<HistoryItemVector>();

    std::vector<std::string> keywords;
    tools::string_tools::splitString(keywordsString, keywords);

    // the longer the keyword is, the faster search will be
    const unsigned longestKeywordPos = tools::string_tools::getLongest(keywords);
    std::string longestKeyword = keywords.at(longestKeywordPos);
    boost::algorithm::to_lower(longestKeyword);

    // assumption: search starts when longest keyword is at least
    // minKeywordLength characters long
    if (longestKeyword.length() < minKeywordLength) {
        return std::make_shared<HistoryItemVector>();
    }

    // get all results for the longest keyword
    std::shared_ptr<HistoryItemVector> historyItems = getHistoryItemsByKeyword(
            longestKeyword, -1);

    if (keywords.size() > 1) {
        // longestKeywordPos is already handled
        keywords.erase(keywords.begin() + longestKeywordPos);
        tools::string_tools::downcase(keywords);
        removeMismatches(historyItems, keywords);
    }

    if (maxItems != -1) {
        if (historyItems->size() > static_cast<unsigned int>(maxItems)) {
            historyItems->erase(historyItems->begin() + maxItems,
                    historyItems->end());
        }
    }
    return historyItems;
}

}
}