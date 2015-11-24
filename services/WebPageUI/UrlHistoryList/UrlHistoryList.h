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

/**
 * Manages list of url matches (URL from history). Manages top layout, creates
 * widget displaying url items.
 */
class UrlHistoryList
{
public:
    UrlHistoryList(WPUStatesManagerPtrConst webPageUiStatesMgr);
    virtual ~UrlHistoryList();

    void setMembers(Evas_Object* parent, Evas_Object* chainObject);
    Evas_Object* getContent();
    Evas_Object* getEditedEntry();
    Evas_Object* getGenlist();

    /**
     * On uri entry widget "changed,user" signal.
     *
     * @param matchedEntries The entries matches for editedUrl
     */
    void onURLEntryEditedByUser(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);

    /**
     * On genlist's item focus change.
     */
    void onItemFocusChange();
    void onMouseClick();

    void hideWidget();

    /**
     * @return True if widget is focused.
     */
    bool widgetFocused() const;
    void onListWidgetFocusChange(bool focused);
    void listWidgetFocusChangeTimerStart();

    void saveEntryAsEditedContent();
    void saveEntryAsURLContent();
    void restoreEntryEditedContent();
    void restoreEntryURLContent();

    boost::signals2::signal<void(shared_ptr<services::HistoryItem>)> openURLInNewTab;
    boost::signals2::signal<void (const std::string&)> uriChanged;

    int getItemsNumberMax() const
    {
        return ITEMS_NUMBER_MAX;
    }

    int getKeywordLengthMin() const
    {
        return KEYWORD_LENGTH_MIN;
    }

private:
    void createLayout(Evas_Object* parentLayout);
    void onItemSelect(std::string content);

    static Eina_Bool onListWidgetFocusChangeDelayed(void *data);
    static void _uri_entry_editing_changed_user(void* data, Evas_Object* obj, void* event_info);
    static void _uri_entry_unfocused(void* data, Evas_Object* obj, void* event_info);

    GenlistManagerPtr m_genlistManager;
    WPUStatesManagerPtrConst m_webPageUiStatesMgr;

    // the maximum items number on a list
    int ITEMS_NUMBER_MAX;
    // the minimum length of the keyword used to search matches
    int KEYWORD_LENGTH_MIN;
    Evas_Object* m_parent = nullptr;
    // entry widget from which change signals are received
    Evas_Object* m_entry = nullptr;
    Evas_Object* m_layout = nullptr;
    string m_edjFilePath;

    // content of the edited entry, needed to restore edited value
    string m_entryEditedContent;
    // content of the entry before edition: needed to restore original URL value
    string m_entryURLContent;

    bool m_widgetFocused = false;
    static Ecore_Timer* m_widgetFocusChangeDelayedTimer;

};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* URLHISTORYLIST_H_ */
