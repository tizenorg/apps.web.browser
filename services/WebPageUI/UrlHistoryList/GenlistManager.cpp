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

#include "BrowserLogger.h"
#include "GenlistManager.h"
#include "GenlistManagerCallbacks.h"
#include "UrlMatchesStyler.h"
#include "GenlistItemsManager.h"

namespace tizen_browser {
namespace base_ui {

GenlistManager::GenlistManager()
{
    m_urlMatchesStyler = make_shared<UrlMatchesStyler>();
    m_itemsManager = make_shared<GenlistItemsManager>();

    m_historyItemClass = elm_genlist_item_class_new();
    m_historyItemClass->item_style = "url_historylist_grid_item";
    m_historyItemClass->func.text_get = nullptr;
    m_historyItemClass->func.content_get = m_itemClassContentGet;
    m_historyItemClass->func.state_get = nullptr;
    m_historyItemClass->func.del = nullptr;

    m_historyItemSpaceClass = elm_genlist_item_class_new();
    m_historyItemSpaceClass->item_style = "url_historylist_grid_item_space";
    m_historyItemSpaceClass->func.text_get = nullptr;
    m_historyItemSpaceClass->func.content_get = nullptr;
    m_historyItemSpaceClass->func.state_get = nullptr;
    m_historyItemSpaceClass->func.del = nullptr;

    GenlistManagerCallbacks::setGenlistManager(this);
}

GenlistManager::~GenlistManager()
{
    elm_genlist_item_class_free(m_historyItemClass);
}

void GenlistManager::clearWidget()
{
    if (m_genlist && elm_genlist_items_count(m_genlist))
        elm_genlist_clear(m_genlist);
}

void GenlistManager::onMouseFocusChange(bool mouseInsideWidget)
{
    this->mouseInsideWidget = mouseInsideWidget;
}

void GenlistManager::setSingleBlockHide(bool block)
{
    m_singleHideBlock = block;
}

bool GenlistManager::getSingleBlockHide()
{
    return m_singleHideBlock;
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
        if (!GENLIST_SHOW_SCROLLBAR) {
            elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF,
                    ELM_SCROLLER_POLICY_OFF);
        }
        elm_object_event_callback_add(m_genlist,
                GenlistManagerCallbacks::_object_event, this);
        evas_object_smart_callback_add(m_genlist, "edge,top",
                GenlistManagerCallbacks::_genlist_edge_top, this);
        evas_object_smart_callback_add(m_genlist, "edge,bottom",
                GenlistManagerCallbacks::_genlist_edge_bottom, this);

        evas_object_event_callback_add(m_genlist, EVAS_CALLBACK_MOUSE_IN,
                GenlistManagerCallbacks::_genlist_mouse_in, this);
        evas_object_event_callback_add(m_genlist, EVAS_CALLBACK_MOUSE_OUT,
                GenlistManagerCallbacks::_genlist_mouse_out, this);

        evas_object_smart_callback_add(m_genlist, "focused",
                GenlistManagerCallbacks::_genlist_focused, this);
        evas_object_smart_callback_add(m_genlist, "unfocused",
                GenlistManagerCallbacks::_genlist_unfocused, this);
    }
    return m_genlist;
}

Evas_Object* GenlistManager::getWidget()
{
    if (!widgetExists())
        createWidget(m_parentLayout);
    return m_genlist;
}

GenlistItemsManagerPtr GenlistManager::getItemsManager()
{
    return m_itemsManager;
}

void GenlistManager::showWidget(const string& editedUrl,
        shared_ptr<services::HistoryItemVector> matchedEntries)
{
    clearWidget();
    prepareUrlsVector(editedUrl, matchedEntries);

    m_itemsManager->setItems( { GenlistItemType::ITEM_FIRST,
            GenlistItemType::ITEM_LAST }, nullptr);
    Elm_Object_Item* itemAppended = nullptr;
    for (auto it : m_readyUrlPairs) {
        itemAppended = elm_genlist_item_append(m_genlist, m_historyItemClass,
                it.get(), nullptr, ELM_GENLIST_ITEM_NONE,
                GenlistManagerCallbacks::_item_selected, it.get());
        m_itemsManager->setItemsIfNullptr( { GenlistItemType::ITEM_FIRST },
                itemAppended);
    }
    m_itemsManager->setItems( { GenlistItemType::ITEM_LAST }, itemAppended);

    if (getWidgetPreviouslyHidden()) {
        setWidgetPreviouslyHidden(false);
        startScrollIn();
    }
}

const char* GenlistManager::getItemUrl(GenlistItemType type)
{
    if (!m_itemsManager->getItem( { type }))
        return "";
    void* data = elm_object_item_data_get(m_itemsManager->getItem( { type }));
    if (!data)
        return "";
    const UrlPair* const urlPair = reinterpret_cast<UrlPair*>(data);
    if (!urlPair)
        return "";
    return urlPair->urlOriginal.c_str();
}

void GenlistManager::hideWidgetPretty()
{
    if (getSingleBlockHide()) {
        setSingleBlockHide(false);
        return;
    }

    m_itemsManager->setItems( { GenlistItemType::ITEM_CURRENT }, nullptr);

    if (getWidgetPreviouslyHidden()) {
        hideWidgetInstant();
        return;
    }
    startScrollOut();
    setWidgetPreviouslyHidden(true);
}

void GenlistManager::hideWidgetInstant()
{
    if (m_genlist)
        evas_object_hide(m_genlist);
}

bool GenlistManager::getWidgetPreviouslyHidden()
{
    return m_widgetPreviouslyHidden;
}

void GenlistManager::setWidgetPreviouslyHidden(bool previouslyHidden)
{
    this->m_widgetPreviouslyHidden = previouslyHidden;
}

void GenlistManager::onMouseClick()
{
    if (!mouseInsideWidget) {
        hideWidgetPretty();
    }
}

void GenlistManager::startScrollIn()
{
    if (m_itemsManager->getItem(GenlistItemType::ITEM_FIRST)) {
        addSpaces();
        elm_genlist_item_show(
                m_itemsManager->getItem(GenlistItemType::ITEM_SPACE_LAST),
                ELM_GENLIST_ITEM_SCROLLTO_TOP);
        elm_genlist_item_bring_in(
                m_itemsManager->getItem(GenlistItemType::ITEM_FIRST),
                ELM_GENLIST_ITEM_SCROLLTO_TOP);
    }
}

void GenlistManager::startScrollOut()
{
    addSpaces();
    Elm_Object_Item* first = m_itemsManager->getItem(
            GenlistItemType::ITEM_SPACE_FIRST);
    if (first) {
        elm_genlist_item_bring_in(first, ELM_GENLIST_ITEM_SCROLLTO_TOP);
    }
}

void GenlistManager::addSpaces()
{
    if (m_itemsManager->getItem(GenlistItemType::ITEM_LAST)) {
        m_itemsManager->setItems( { GenlistItemType::ITEM_SPACE_FIRST,
                GenlistItemType::ITEM_SPACE_LAST }, nullptr);
        Elm_Object_Item* itemAppended = nullptr;
        for (auto i = 0; i < HISTORY_ITEMS_VISIBLE_MAX; ++i) {
            // append spaces to the last url item, so they can be easily cleared
            itemAppended = elm_genlist_item_append(m_genlist,
                    m_historyItemSpaceClass, nullptr,
                    m_itemsManager->getItem(GenlistItemType::ITEM_LAST),
                    ELM_GENLIST_ITEM_NONE, nullptr, nullptr);
            elm_object_focus_allow_set(itemAppended, EINA_FALSE);
            m_itemsManager->setItemsIfNullptr( {
                    GenlistItemType::ITEM_SPACE_FIRST }, itemAppended);
        }
        m_itemsManager->setItems( { GenlistItemType::ITEM_SPACE_LAST },
                itemAppended);
    }
}

void GenlistManager::removeSpaces()
{
    if (m_itemsManager->getItem(GenlistItemType::ITEM_LAST)) {
        elm_genlist_item_subitems_clear(
                m_itemsManager->getItem(GenlistItemType::ITEM_LAST));
    }
    m_itemsManager->setItems( { GenlistItemType::ITEM_SPACE_FIRST,
            GenlistItemType::ITEM_SPACE_LAST }, nullptr);
}

Evas_Object* GenlistManager::m_itemClassContentGet(void* data, Evas_Object* obj,
        const char* part)
{
    Evas_Object* label = elm_label_add(obj);
    if (strcmp(part, "matched_url") == 0) {
        const UrlPair* const item = reinterpret_cast<UrlPair*>(data);
        if (item) {
            elm_object_text_set(label, item->urlHighlighted.c_str());
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
    m_readyUrlPairs.clear();
    for (auto it : *matchedEntries) {
        UrlPair newUrlPair(it->getUrl(),
                m_urlMatchesStyler->getUrlHighlightedMatches(it->getUrl(),
                        editedUrl));
        m_readyUrlPairs.push_back(make_shared < UrlPair > (newUrlPair));
    }
}

} /* namespace base_ui */
} /* namespace tizen_browser */
