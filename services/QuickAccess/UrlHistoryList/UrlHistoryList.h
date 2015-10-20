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

class QuickAccess;
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
 * Needed to indicate who did the last url entry edition (user/other/other many times). Used
 * to indicate when widget can be hidden in a pretty way or an instant way.
 */
class EditedUrlStatesHelper
{
public:
    EditedUrlStatesHelper()
    {
    }
    void changeState(bool editedByUser)
    {
        if (editedByUser) {
            currentState = EditedUrlState::EDITED_BY_USER;
        } else {
            if (currentState == EditedUrlState::EDITED_BY_USER) {
                currentState = EditedUrlState::EDITED_OTHER_FIRST;
            } else {
                currentState = EditedUrlState::EDITED_OTHER_MANY_TIMES;
            }
        }
    }
    EditedUrlState getCurrentState()
    {
        return currentState;
    }
private:
    EditedUrlState currentState = EditedUrlState::EDITED_BY_USER;
};

/**
 * Manages list of url matches (URL from history). Manages top layout, creates
 * widget displaying url items.
 */
class UrlHistoryList
{
public:
    UrlHistoryList(QuickAccess* quickAccess);
    virtual ~UrlHistoryList();
    void createLayout(Evas_Object* parentLayout);
    Evas_Object* getLayout();

    // // on uri entry widget "changed,user" signal
    void onURLEntryEditedByUser(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);
    // on uri entry widget "changed" signal
    void onURLEntryEdited();

    void onMouseClick();

    boost::signals2::signal<void(shared_ptr<services::HistoryItem>, bool)> openURLInNewTab;

    int getVisibleItemsMax() const
    {
        return VISIBLE_ITEMS_MAX;
    }

    int getMinKeywordLength() const
    {
        return MIN_KEYWORD_LENGTH;
    }

private:
    void onListItemSelect(std::string content);
    void onListWidgetFocused();
    void onListWidgetUnfocused();

    EditedUrlStatesHelper editedUrlStatesHelper;

    // the maximum items number on a list
    const int VISIBLE_ITEMS_MAX = 12;
    // the minimum length of the keyword used to search matches
    const int MIN_KEYWORD_LENGTH = 3;
    Evas_Object* m_layout;
    string m_edjFilePath;

    GenlistManagerPtr m_genlistListManager = nullptr;
    QuickAccess* m_quickAccess;

};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* URLHISTORYLIST_H_ */
