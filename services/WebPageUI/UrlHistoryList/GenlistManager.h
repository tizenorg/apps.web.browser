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
enum class GenlistItemType;
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
    void hideWidget();

    void onMouseClick();

    /**
     * Clear all elements from a genlist.
     */
    void clearWidget();
    /**
     * When set to true, the next hide attempt will be blocked. E.g. widget
     * should not be hidden on a mouse click, when cursor is inside widget.
     */
    void setSingleBlockHide(bool block);
    bool getSingleBlockHide();

    boost::signals2::signal<void(string)> signalItemSelected;
    boost::signals2::signal<void()> signalItemFocusChange;

    /**
     * Get url from item of a given type.
     * @param types The types of list items: url will be searched in these item types.
     * @return Url from the first item from the list, which has valid url. Empty if neither of items has url assigned.
     */
    string getItemUrl(std::initializer_list<GenlistItemType> types) const;

    void clearTimerMouseClickHandle();
    bool isMouseInsideWidget();

private:
    static Evas_Object* m_itemClassContentGet(void *data, Evas_Object *obj,
            const char *part);

    bool widgetExists() {return m_genlist != nullptr;}

    void prepareUrlsVector(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);

    /**
     * Cursor focus change. Needed to indicate, if widget should be hidden on
     * a mouse click.
     */
    void onMouseFocusChange(bool mouseInsideWidget);

    /**
     * Adjust widget's height to item's number.
     */
    void adjustWidgetHeight();

    static Eina_Bool timerMouseClickHandle(void *data);

    Evas_Object* m_parentLayout = nullptr;
    Evas_Object* m_genlist = nullptr;

    bool GENLIST_SHOW_SCROLLBAR;
    // don't know how to get from edc:
    int ITEM_H;
    int ITEMS_VISIBLE_NUMBER_MAX;
    // currently visible items number
    int m_historyItemsVisibleCurrent;

    /**
     * Used in setSingleBlockHide().
     */
    bool m_singleHideBlock = false;
    /**
     * Used in onMouseFocusChange().
     */
    bool m_mouseInsideWidget = false;

    Elm_Gengrid_Item_Class* m_historyItemClass;

    GenlistItemsManagerPtr m_itemsManager;

    /*
     * keeps shared pointers to strings, which are ready to be displayed, so they can be
     * passed through EFL, until they're not needed. IMPORTANT: it has to be
     * assured, that list is not cleared until all EFL items has created their
     * labels from these pointers in m_contentGet(). in case of segfaults, delete copy of pointers
     * manually in m_contentGet().
     */
    vector<shared_ptr<UrlPair>> m_readyUrlPairs;
    UrlMatchesStylerPtr m_urlMatchesStyler;

    /**
     * Used to invoke timerMouseClickHandle()
     */
    Ecore_Timer* m_timerMouseClickHandle = nullptr;

};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* GENLISTMANAGER_H_ */
