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

#include "GenlistManager.h"
#include "GenlistManagerCallbacks.h"
#include "UrlMatchesStyler.h"

namespace tizen_browser {
namespace services {

GenlistManager::GenlistManager()
{
    m_urlMatchesStyler = make_shared<UrlMatchesStyler>();

    m_historyItemClass = elm_genlist_item_class_new();
    m_historyItemClass->item_style = "url_historylist_grid_item";
    m_historyItemClass->func.text_get = nullptr;
    m_historyItemClass->func.content_get = m_contentGet;
    m_historyItemClass->func.state_get = nullptr;
    m_historyItemClass->func.del = nullptr;

    m_historyItemSpaceClass = elm_genlist_item_class_new();
    m_historyItemSpaceClass->item_style = "url_historylist_grid_item_space";
    m_historyItemSpaceClass->func.text_get = nullptr;
    m_historyItemSpaceClass->func.content_get = nullptr;
    m_historyItemSpaceClass->func.state_get = nullptr;
    m_historyItemSpaceClass->func.del = nullptr;
}

GenlistManager::~GenlistManager()
{
    elm_genlist_item_class_free(m_historyItemClass);
}

void GenlistManager::clearWidget()
{
    elm_genlist_clear(m_genlist);
    elm_genlist_clear(m_genlist);
    elm_genlist_clear(m_genlist);
    elm_genlist_clear(m_genlist);
}

void GenlistManager::onMouseFocusChange(bool mouseInsideWidget)
{
    this->mouseInsideWidget = mouseInsideWidget;
}

Evas_Object* GenlistManager::createWidget(Evas_Object* parentLayout)
{
    if (!widgetExists()) {
        m_parentLayout = parentLayout;
        m_genlist = elm_genlist_add(parentLayout);
        evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND,
        EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL,
        EVAS_HINT_FILL);
        if (!genlistShowScrollbar) {
            elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF,
                    ELM_SCROLLER_POLICY_OFF);
        }

        evas_object_smart_callback_add(m_genlist, "scroll,anim,stop",
                GenlistManagerCallbacks::cb_genlistAnimStop, this);
        evas_object_smart_callback_add(m_genlist, "edge,top",
                GenlistManagerCallbacks::cb_genlistEdgeTop, this);
        evas_object_smart_callback_add(m_genlist, "edge,bottom",
                GenlistManagerCallbacks::cb_genlistEdgeBottom, this);

        evas_object_smart_callback_add(m_genlist, "activated",
                GenlistManagerCallbacks::cb_genlistActivated, this);
        evas_object_smart_callback_add(m_genlist, "pressed",
                GenlistManagerCallbacks::cb_genlistPressed, this);
        evas_object_smart_callback_add(m_genlist, "selected",
                GenlistManagerCallbacks::cb_genlistSelected, this);

        evas_object_event_callback_add(m_genlist, EVAS_CALLBACK_MOUSE_IN,
                GenlistManagerCallbacks::cb_genlistMouseIn, this);
        evas_object_event_callback_add(m_genlist, EVAS_CALLBACK_MOUSE_OUT,
                GenlistManagerCallbacks::cb_genlistMouseOut, this);
        evas_object_smart_callback_add(m_genlist, "unselected",
                GenlistManagerCallbacks::cb_genlistUnselected, this);
        evas_object_smart_callback_add(m_genlist, "focused",
                GenlistManagerCallbacks::cb_genlistFocused, this);
    }
    return m_genlist;
}

Evas_Object* GenlistManager::getWidget()
{
    if (!widgetExists())
        createWidget(m_parentLayout);
    return m_genlist;
}

void GenlistManager::showWidget(const string& editedUrl,
        shared_ptr<services::HistoryItemVector> matchedEntries)
{
    clearWidget();
    prepareUrlsVector(editedUrl, matchedEntries);

    m_itemUrlFirst = m_itemUrlLast = nullptr;
    Elm_Object_Item* itemAppended;
    for (auto it : m_readyUrls) {
        itemAppended = elm_genlist_item_append(m_genlist, m_historyItemClass,
                it.get(), nullptr, ELM_GENLIST_ITEM_NONE, nullptr, this);
        if (!m_itemUrlFirst)
            m_itemUrlFirst = itemAppended;
    }
    m_itemUrlLast = itemAppended;

    if (widgetPreviouslyHidden) {
        widgetPreviouslyHidden = false;
        startScrollIn();
    }
}

void GenlistManager::hideWidget()
{
    if (widgetPreviouslyHidden)
        return;
    startScrollOut();
    widgetPreviouslyHidden = true;
}

bool GenlistManager::isWidgetHidden()
{
    return widgetPreviouslyHidden;
}

void GenlistManager::onMouseClick()
{
    if (!mouseInsideWidget) {
        hideWidget();
    }
}

void GenlistManager::startScrollIn()
{
    if (m_itemUrlFirst) {
        addSpaces();
        elm_genlist_item_show(m_itemSpaceLast, ELM_GENLIST_ITEM_SCROLLTO_TOP);
        elm_genlist_item_bring_in(m_itemUrlFirst,
                ELM_GENLIST_ITEM_SCROLLTO_TOP);
    }
}

void GenlistManager::startScrollOut()
{
    addSpaces();
    if (m_itemSpaceFirst) {
        elm_genlist_item_bring_in(m_itemSpaceFirst,
                ELM_GENLIST_ITEM_SCROLLTO_TOP);
    }
}

void GenlistManager::setLastEdgeTop(bool edgeTop)
{
    lastEdgeTop = edgeTop;
}

bool GenlistManager::getLastEdgeTop()
{
    return lastEdgeTop;
}

void GenlistManager::addSpaces()
{
    if (m_itemUrlLast) {
        m_itemSpaceFirst = m_itemSpaceLast = nullptr;
        Elm_Object_Item* itemAppended;
        for (auto i = 0; i < historyItemsVisibleMax; ++i) {
            // append spaces to the last url item, so they can be easily cleared
            itemAppended = elm_genlist_item_append(m_genlist,
                    m_historyItemSpaceClass, nullptr, m_itemUrlLast,
                    ELM_GENLIST_ITEM_NONE, nullptr, this);
            if (!m_itemSpaceFirst)
                m_itemSpaceFirst = itemAppended;
        }
        m_itemSpaceLast = itemAppended;
    }
}

void GenlistManager::removeSpaces()
{
    if (m_itemUrlLast) {
        elm_genlist_item_subitems_clear(m_itemUrlLast);
    }
    m_itemSpaceFirst = m_itemSpaceLast = nullptr;
}

Evas_Object* GenlistManager::m_contentGet(void *data, Evas_Object *obj,
        const char *part)
{
    Evas_Object* label = elm_label_add(obj);
    if (strcmp(part, "matched_url") == 0) {
        const string * const item = reinterpret_cast<string*>(data);
        if (item) {
            elm_object_text_set(label, item->c_str());
            evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND,
            EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(label, EVAS_HINT_FILL,
            EVAS_HINT_FILL);
        }
    }
    return label;
}

void GenlistManager::prepareUrlsVector(const string& editedUrl,
        shared_ptr<services::HistoryItemVector> matchedEntries)
{
    // free previously used urls. IMPORTANT: it has to be assured that previous
    // genlist items are not using these pointers.
    m_readyUrls.clear();
    for (auto it : *matchedEntries) {
        m_readyUrls.push_back(
                make_shared < string
                        > (m_urlMatchesStyler->getUrlHighlightedMatches(
                                it->getUrl(), editedUrl)));
    }
}

} /* namespace services */
} /* namespace tizen_browser */
