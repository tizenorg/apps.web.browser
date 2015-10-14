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

#include "BrowserLogger.h"
#include "WidgetListManager.h"

using namespace std;

namespace tizen_browser {
namespace services {

class GenlistManagerCallbacks;
class UrlMatchesStyler;
typedef shared_ptr<UrlMatchesStyler> UrlMatchesStylerPtr;

class GenlistManager: public WidgetListManager
{
    friend class GenlistManagerCallbacks;
public:
    GenlistManager();
    virtual ~GenlistManager();

    virtual Evas_Object* createWidget(Evas_Object* parentLayout);
    virtual Evas_Object* getWidget();

    virtual void showWidget(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);
    virtual void hideWidget();
    void onMouseClick();

    bool isWidgetHidden();

    /**
     * Add empty list elements to allow scroll in effect.
     */
    void addSpaces();
    void removeSpaces();

    void clearWidget();

private:

    static Evas_Object* m_contentGet(void *data, Evas_Object *obj,
            const char *part);
    bool widgetExists()
    {
        return m_genlist != nullptr;
    }
    void prepareUrlsVector(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);
    void startScrollIn();
    void startScrollOut();
    void setLastEdgeTop(bool edgeTop);
    bool getLastEdgeTop();
    void onMouseFocusChange(bool mouseInsideWidget);

    Evas_Object* m_parentLayout = nullptr;
    Evas_Object* m_genlist = nullptr;
    const bool genlistShowScrollbar = false;

    // don't know how to get from edc:
    const int historyItemH = 82;
    const int historyItemsVisibleMax = 5;
    // don't know how to calculate:
    const int genlistH = historyItemH * historyItemsVisibleMax;

    /*
     * Set to true, whenever hide request occurs. Set to false, whenever show
     * request occurs. Needed to indicate when genlist should slide in.
     */
    bool widgetPreviouslyHidden = true;
    /*
     * If mouse click received and mouse is outside widget, hide it.
     */
    bool mouseInsideWidget = true;
    /*
     * needed to indicate direction of the scroll in 'anim,stop' callback
     */
    bool lastEdgeTop = true;

    Elm_Gengrid_Item_Class * m_historyItemClass;
    Elm_Gengrid_Item_Class * m_historyItemSpaceClass;
    Elm_Object_Item* m_itemUrlFirst = nullptr;
    Elm_Object_Item* m_itemUrlLast = nullptr;
    Elm_Object_Item* m_itemSpaceFirst = nullptr;
    Elm_Object_Item* m_itemSpaceLast = nullptr;

    /*
     * keeps shared pointers to strings, that are ready to be displayed, so they can be
     * passed through EFL, until they're not needed. IMPORTANT: it has to be
     * assured, that list is not cleared until all EFL items has created their
     * labels from these pointers in m_contentGet(). in case of segfaults, delete copy of pointers
     * manually in m_contentGet().
     */
    vector<shared_ptr<string>> m_readyUrls;
    UrlMatchesStylerPtr m_urlMatchesStyler;

};

} /* namespace services */
} /* namespace tizen_browser */

#endif /* GENLISTMANAGER_H_ */
