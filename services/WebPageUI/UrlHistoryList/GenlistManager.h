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

#ifndef GENLISTMANAGER_H_
#define GENLISTMANAGER_H_

#include <Elementary.h>
#include "services/HistoryService/HistoryItem.h"
#include <boost/signals2/signal.hpp>

using namespace std;

namespace tizen_browser {
namespace base_ui {

class GenlistManagerCallbacks;
class GenlistItemsManager;
enum class GenlistItemType
;
typedef shared_ptr<GenlistItemsManager> GenlistItemsManagerPtr;
class UrlMatchesStyler;
typedef shared_ptr<UrlMatchesStyler> UrlMatchesStylerPtr;

typedef struct UrlPair_s
{
    UrlPair_s(string a, string b) :
            urlOriginal(a), urlHighlighted(b)
    {
    }
    string urlOriginal;
    /**
     * Url plus styling tags.
     */
    string urlHighlighted;
} UrlPair;

class GenlistManager
{
    friend class GenlistManagerCallbacks;
public:
    GenlistManager();
    ~GenlistManager();

    Evas_Object* createWidget(Evas_Object* parentLayout);
    Evas_Object* getWidget();
    GenlistItemsManagerPtr getItemsManager();

    void showWidget(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);
    /**
     * Hide widget by scrolling out.
     */
    void hideWidgetPretty();
    /**
     * Hide widget by evas_object_hide.
     */
    void hideWidgetInstant();
    void setWidgetPreviouslyHidden(bool previouslyHidden);
    bool getWidgetPreviouslyHidden();
    void onMouseClick();

    /**
     * Add empty list elements to allow scroll in effect.
     */
    void addSpaces();
    void removeSpaces();
    /**
     * Clear all elements from a genlist.
     */
    void clearWidget();
    /**
     * When set to true, the next hide attempt will be blocked.
     */
    void setSingleBlockHide(bool block);
    bool getSingleBlockHide();

    boost::signals2::signal<void(string)> signalItemSelected;
    boost::signals2::signal<void()> signalWidgetFocused;
    boost::signals2::signal<void()> signalWidgetUnfocused;
    boost::signals2::signal<void()> signalItemFocusChange;

    const char* getItemUrl(GenlistItemType type);

private:
    static Evas_Object* m_itemClassContentGet(void *data, Evas_Object *obj,
            const char *part);

    bool widgetExists() {return m_genlist != nullptr;}

    void prepareUrlsVector(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);

    void onMouseFocusChange(bool mouseInsideWidget);

    /**
     * Adjust widget's height to item's number.
     */
    void adjustWidgetHeight();
    void startScrollIn();
    void startScrollOut();

    Evas_Object* m_parentLayout = nullptr;
    Evas_Object* m_genlist = nullptr;

    const bool GENLIST_SHOW_SCROLLBAR = false;
    // don't know how to get from edc:
    const int HISTORY_ITEM_H = 82;
    const int HISTORY_ITEMS_VISIBLE_MAX = 5;
    // don't know how to calculate:
    const int GENLIST_H = HISTORY_ITEM_H * HISTORY_ITEMS_VISIBLE_MAX;

    /*
     * Set to true, whenever hide request occurs. Set to false, whenever show
     * request occurs. Needed to indicate when genlist should slide in.
     */
    bool m_widgetPreviouslyHidden = true;
    bool m_singleHideBlock = false;
    /*
     * If mouse click received and mouse is outside widget, hide it.
     */
    bool mouseInsideWidget = true;

    Elm_Gengrid_Item_Class* m_historyItemClass;
    Elm_Gengrid_Item_Class* m_historyItemSpaceClass;

    GenlistItemsManagerPtr m_itemsManager;

    /*
     * keeps shared pointers to strings, that are ready to be displayed, so they can be
     * passed through EFL, until they're not needed. IMPORTANT: it has to be
     * assured, that list is not cleared until all EFL items has created their
     * labels from these pointers in m_contentGet(). in case of segfaults, delete copy of pointers
     * manually in m_contentGet().
     */
    vector<shared_ptr<UrlPair>> m_readyUrlPairs;
    UrlMatchesStylerPtr m_urlMatchesStyler;

};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* GENLISTMANAGER_H_ */
