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

#include <boost/algorithm/string.hpp>
#include <services/HistoryService/HistoryMatchFinder/HistoryMatchFinder.h>
#include <core/BrowserLogger.h>

namespace tizen_browser {
namespace services {

void HistoryMatchFinder::removeMismatches(
        const shared_ptr<HistoryItemVector>& historyItems,
        const vector<string>& keywords) const
{
    for (auto itItem = historyItems->begin(); itItem != historyItems->end();) {
        if (!stringMatchesKeywords((*itItem)->getUrl(), keywords.begin(),
                keywords.end())) {
            // remove url not matching all keywords
            itItem = historyItems->erase(itItem);
        } else {
            ++itItem;
        }
    }
}

bool HistoryMatchFinder::stringMatchesKeywords(const string& url,
        const vector<string>::const_iterator itKeywordsBegin,
        const vector<string>::const_iterator itKeywordsEnd) const
{
    for (auto it = itKeywordsBegin; it != itKeywordsEnd; ++it)
        if (!simplePatternMatch(url, *it))
            return false;
    return true;
}

void HistoryMatchFinder::splitKeywordsString(const string& keywordsString,
        vector<string>& resultKeywords) const
{
    boost::algorithm::split(resultKeywords, keywordsString,
            boost::is_any_of("\t "), boost::token_compress_on);
    // remove empty elements
    for (auto it = resultKeywords.begin(); it != resultKeywords.end();) {
        if (it->empty()) {
            it = resultKeywords.erase(it);
        } else {
            ++it;
        }
    }
}

void HistoryMatchFinder::downcaseStrings(vector<string>& resultKeywords) const
{
    for (auto& str : resultKeywords) {
        boost::algorithm::to_lower(str);
    }
}

unsigned HistoryMatchFinder::getLongest(const vector<string>& strVec) const
{
    unsigned posLongest = 0;
    for (auto it = strVec.begin() + 1; it != strVec.end(); ++it)
        if (it->length() > strVec.at(posLongest).length())
            posLongest = distance(strVec.begin(), it);
    return posLongest;
}

bool HistoryMatchFinder::simplePatternMatch(const string &text,
        const string &pattern) const
{
    if (pattern.empty() || text.empty()) {
        return pattern.empty() && text.empty();
    }
    for (auto it = text.begin(); it != text.end(); ++it) {
        const auto mismatches = mismatch(pattern.begin(), pattern.end(), it);
        if (mismatches.first == pattern.end()) {
            return true;
        }
    }
    return false;
}

} /* namespace services */
} /* namespace tizen_browser */
