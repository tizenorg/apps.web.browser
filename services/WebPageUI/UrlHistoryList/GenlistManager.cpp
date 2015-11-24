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
#include "Config.h"

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

    GenlistManagerCallbacks::setGenlistManager(this);

    config::DefaultConfig config;
    config.load("");
    GENLIST_SHOW_SCROLLBAR = boost::any_cast<bool>(
            config.get(CONFIG_KEY::URLHISTORYLIST_SHOW_SCROLLBAR));
#if PROFILE_MOBILE
    double mobile_scale = boost::any_cast<double>(config.get("mobile_scale"));
    ITEM_H = boost::any_cast<int>(
            config.get(CONFIG_KEY::URLHISTORYLIST_ITEM_HEIGHT));
    ITEM_H *= mobile_scale;
#else
    ITEM_H = boost::any_cast<int>(
            config.get(CONFIG_KEY::URLHISTORYLIST_ITEM_HEIGHT));
#endif
    ITEMS_VISIBLE_NUMBER_MAX = boost::any_cast<int>(
            config.get(CONFIG_KEY::URLHISTORYLIST_ITEMS_VISIBLE_NUMBER_MAX));
    m_historyItemsVisibleCurrent = ITEMS_VISIBLE_NUMBER_MAX;
}

GenlistManager::~GenlistManager()
{
    elm_genlist_item_class_free(m_historyItemClass);
}

void GenlistManager::clearWidget()
{
    if (m_genlist && elm_genlist_items_count(m_genlist))
        elm_genlist_clear(m_genlist);
    m_itemsManager->clear();
}

void GenlistManager::onMouseFocusChange(bool mouseInsideWidget)
{
    this->m_mouseInsideWidget = mouseInsideWidget;
}

bool GenlistManager::isMouseInsideWidget()
{
    return this->m_mouseInsideWidget;
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
        elm_scroller_bounce_set(m_genlist, EINA_FALSE, EINA_FALSE);
        elm_scroller_movement_block_set(m_genlist,
                ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);

        if (!GENLIST_SHOW_SCROLLBAR) {
            elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF,
                    ELM_SCROLLER_POLICY_OFF);
        }
        elm_object_event_callback_add(m_genlist,
                GenlistManagerCallbacks::_object_event, this);

        evas_object_event_callback_add(m_genlist, EVAS_CALLBACK_MOUSE_IN,
                GenlistManagerCallbacks::_genlist_mouse_in, this);
        evas_object_event_callback_add(m_genlist, EVAS_CALLBACK_MOUSE_OUT,
                GenlistManagerCallbacks::_genlist_mouse_out, this);
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

    adjustWidgetHeight();
}

string GenlistManager::getItemUrl(std::initializer_list<GenlistItemType> types) const
{
    for (auto t : types) {
        if (!m_itemsManager->getItem( { t }))
            continue;
        void* data = elm_object_item_data_get(m_itemsManager->getItem( { t }));
        if (!data)
            continue;
        const UrlPair* const urlPair = reinterpret_cast<UrlPair*>(data);
        if (!urlPair)
            continue;
        return urlPair->urlOriginal;
    }
    return "";
}

void GenlistManager::hideWidget()
{
    if(getSingleBlockHide()) {
        setSingleBlockHide(false);
        return;
    }
    clearWidget();
    evas_object_hide(getWidget());
}

void GenlistManager::onMouseClick()
{
    // Handle mouse click in the end, to indicate if the cursor is inside
    // widget. Callback for the mouse focus comes after onMouseClick()).
    // Hence a timer is needed.
    m_timerMouseClickHandle = ecore_timer_add(0.0,
            GenlistManager::timerMouseClickHandle, this);
}

void GenlistManager::clearTimerMouseClickHandle()
{
    ecore_timer_del(m_timerMouseClickHandle);
    m_timerMouseClickHandle = nullptr;
}

Eina_Bool GenlistManager::timerMouseClickHandle(void* data)
{
    auto self = reinterpret_cast<GenlistManager*>(data);
    if(!self) return EINA_FALSE;

    self->clearTimerMouseClickHandle();

    if (!self->isMouseInsideWidget()) {
        self->hideWidget();
    } else {
        // don't hide widget, when curosr is inside
        self->setSingleBlockHide(true);
    }
    return EINA_FALSE;
}

void GenlistManager::adjustWidgetHeight()
{
    const int LIST_ITEMS_NUMBER = elm_genlist_items_count(m_genlist);
    if (LIST_ITEMS_NUMBER == 0)
        return;

    m_historyItemsVisibleCurrent = ITEMS_VISIBLE_NUMBER_MAX;
    if (LIST_ITEMS_NUMBER < m_historyItemsVisibleCurrent)
        m_historyItemsVisibleCurrent = LIST_ITEMS_NUMBER;

    Evas_Coord w, h;
    evas_object_geometry_get(m_genlist, nullptr, nullptr, &w, nullptr);
    h = ITEM_H * m_historyItemsVisibleCurrent;

    evas_object_resize(m_genlist, w, h);
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
