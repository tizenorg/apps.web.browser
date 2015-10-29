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

#ifndef URLHISTORYLIST_H_
#define URLHISTORYLIST_H_

#include <memory>
#include <Evas.h>

#include "services/HistoryService/HistoryItem.h"
#include <boost/signals2/signal.hpp>

using namespace std;

namespace tizen_browser {
namespace base_ui {

class WebPageUIStatesManager;
typedef std::shared_ptr<const WebPageUIStatesManager> WPUStatesManagerPtrConst;
class GenlistManager;
typedef shared_ptr<GenlistManager> GenlistManagerPtr;

enum class EditedUrlState
{
    // url was edited by a user (by typing)
    EDITED_BY_USER,
    // url was edited in other way than typing (but for the first time after previous user edition)
    EDITED_OTHER_FIRST,
    // url was edited in other way than typing (and previously was not edited by the user)
    EDITED_OTHER_MANY_TIMES
};

/**
 * Manages list of url matches (URL from history). Manages top layout, creates
 * widget displaying url items.
 */
class UrlHistoryList
{
public:
    UrlHistoryList(WPUStatesManagerPtrConst webPageUiStatesMgr);
    virtual ~UrlHistoryList();
    Evas_Object* getContent();
    Evas_Object* getEditedEntry();
    void saveEntryEditedContent();
    void restoreEntryEditedContent();
    void saveEntryURLContent();
    void restoreEntryURLContent();
    Evas_Object* getGenlist();

    // remove if unused
    void hideWidgetPretty();

    void setMembers(Evas_Object* parent, Evas_Object* chainObject);

    // // on uri entry widget "changed,user" signal
    void onURLEntryEditedByUser(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);

    void onItemFocusChange();
    void onMouseClick();

    boost::signals2::signal<void(shared_ptr<services::HistoryItem>)> openURLInNewTab;
    boost::signals2::signal<void (const std::string&)> uriChanged;

    int getVisibleItemsMax() const
    {
        return VISIBLE_ITEMS_MAX;
    }

    int getMinKeywordLength() const
    {
        return MIN_KEYWORD_LENGTH;
    }

private:
    void createLayout(Evas_Object* parentLayout);
    void onListItemSelect(std::string content);
    void onListWidgetFocused();
    void onListWidgetUnfocused();

    static void _uri_entry_editing_changed_user(void* data, Evas_Object* obj, void* event_info);
    static void _uri_entry_unfocused(void* data, Evas_Object* obj, void* event_info);

    WPUStatesManagerPtrConst m_webPageUiStatesMgr = nullptr;

    // the maximum items number on a list
    const int VISIBLE_ITEMS_MAX = 12;
    // the minimum length of the keyword used to search matches
    const int MIN_KEYWORD_LENGTH = 3;
    Evas_Object* m_parent = nullptr;
    // entry widget from which change signals are received
    Evas_Object* m_entry = nullptr;
    // content of the entry, needed to restore edited value
    string m_entryEditedContent;
    // content of the entry before edition: needed to restore original URL value
    string m_entryURLContent;
    Evas_Object* m_layout = nullptr;
    string m_edjFilePath;

    GenlistManagerPtr m_genlistListManager = nullptr;

};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* URLHISTORYLIST_H_ */
