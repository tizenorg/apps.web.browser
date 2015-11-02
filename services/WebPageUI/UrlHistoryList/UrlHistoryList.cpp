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

#include <Elementary.h>
#include <ecore-1/Ecore.h>
#include "UrlHistoryList.h"
#include "GenlistManager.h"
#include "BrowserLogger.h"
#include "GenlistItemsManager.h"
#include "WebPageUI/WebPageUIStatesManager.h"

namespace tizen_browser {
namespace base_ui {

Ecore_Timer* UrlHistoryList::m_widgetFocusChangeDelayedTimer = nullptr;

UrlHistoryList::UrlHistoryList(WPUStatesManagerPtrConst webPageUiStatesMgr)
    : m_genlistListManager(make_shared<GenlistManager>())
    , m_webPageUiStatesMgr(webPageUiStatesMgr)
    , m_layout(nullptr)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("WebPageUI/UrlHistoryList.edj");
    m_genlistListManager->signalItemSelected.connect(
            boost::bind(&UrlHistoryList::onItemSelect, this, _1));
    m_genlistListManager->signalItemFocusChange.connect(
            boost::bind(&UrlHistoryList::onItemFocusChange, this));
}

UrlHistoryList::~UrlHistoryList()
{
}

void UrlHistoryList::setMembers(Evas_Object* parent, Evas_Object* editedEntry)
{
    m_parent = parent;
    m_entry = editedEntry;

    evas_object_smart_callback_add(m_entry, "changed,user",
            UrlHistoryList::_uri_entry_editing_changed_user, this);
    evas_object_smart_callback_add(m_entry, "unfocused",
            UrlHistoryList::_uri_entry_unfocused, this);
}

void UrlHistoryList::createLayout(Evas_Object* parentLayout)
{
    if (m_layout)
        return;

    m_layout = elm_layout_add(parentLayout);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "url_history_list");
    Evas_Object* widgetList = m_genlistListManager->createWidget(m_layout);
    m_genlistListManager->hideWidgetPretty();
    elm_object_part_content_set(m_layout, "list_swallow", widgetList);
    evas_object_hide(widgetList);
    evas_object_hide(m_layout);
}

Evas_Object* UrlHistoryList::getContent()
{
    if (!m_layout)
        createLayout(m_parent);
    return m_layout;
}

Evas_Object* UrlHistoryList::getEditedEntry()
{
    return m_entry;
}

void UrlHistoryList::saveEntryAsEditedContent()
{
    m_entryEditedContent = elm_entry_entry_get(m_entry);
}

void UrlHistoryList::restoreEntryEditedContent()
{
    elm_entry_entry_set(m_entry, m_entryEditedContent.c_str());
    elm_entry_cursor_end_set(m_entry);
}

Evas_Object* UrlHistoryList::getGenlist()
{
    return m_genlistListManager->getWidget();
}

void UrlHistoryList::hideWidgetPretty()
{
    m_genlistListManager->hideWidgetPretty();
}

bool UrlHistoryList::widgetFocused() const
{
    return m_widgetFocused;
}

void UrlHistoryList::onURLEntryEditedByUser(const string& editedUrl,
        shared_ptr<services::HistoryItemVector> matchedEntries)
{
    if (matchedEntries->size() == 0) {
        m_genlistListManager->hideWidgetPretty();
    } else {
        Evas_Object* widgetList = m_genlistListManager->getWidget();
        m_genlistListManager->showWidget(editedUrl, matchedEntries);
        evas_object_show(widgetList);
        evas_object_show(m_layout);
    }
}

void UrlHistoryList::onMouseClick()
{
    m_genlistListManager->onMouseClick();
}

void UrlHistoryList::onItemFocusChange()
{
    elm_entry_entry_set(m_entry, m_genlistListManager->getItemUrl( {
            GenlistItemType::ITEM_CURRENT }).c_str());
}

void UrlHistoryList::onItemSelect(std::string content)
{
    if (m_webPageUiStatesMgr->equals(WPUState::QUICK_ACCESS)) {
        openURLInNewTab (make_shared<services::HistoryItem>(content));
    } else {
        uriChanged(content);
    }
    hideWidgetPretty();
}

void UrlHistoryList::onListWidgetFocusChange(bool focused)
{
    m_widgetFocused = focused;
    if (focused) {
        string itemUrl = m_genlistListManager->getItemUrl( {
                GenlistItemType::ITEM_CURRENT, GenlistItemType::ITEM_FIRST });
        elm_entry_entry_set(m_entry, itemUrl.c_str());
    } else {
        restoreEntryEditedContent();
    }
}

void UrlHistoryList::listWidgetFocusChangeTimerStart()
{
    m_widgetFocused = true;
    // block 'hideWidgetPretty()' invoked in _uri_entry_unfocused
    m_genlistListManager->setSingleBlockHide(true);
    UrlHistoryList::m_widgetFocusChangeDelayedTimer = ecore_timer_add(0.1,
            UrlHistoryList::onListWidgetFocusChangeDelayed, this);
}

Eina_Bool UrlHistoryList::onListWidgetFocusChangeDelayed(void* data)
{
    if (UrlHistoryList::m_widgetFocusChangeDelayedTimer) {
        ecore_timer_del(UrlHistoryList::m_widgetFocusChangeDelayedTimer);
        UrlHistoryList::m_widgetFocusChangeDelayedTimer = nullptr;
    }

    auto self = reinterpret_cast<UrlHistoryList*>(data);
    self->onListWidgetFocusChange(true);
    return EINA_FALSE;
}

void UrlHistoryList::_uri_entry_editing_changed_user(void* data,
        Evas_Object* /* obj */, void* /* event_info */)
{
    UrlHistoryList* self = reinterpret_cast<UrlHistoryList*>(data);
    self->saveEntryAsEditedContent();
}

void UrlHistoryList::_uri_entry_unfocused(void* data, Evas_Object* /* obj */,
        void* /* event_info */)
{
    UrlHistoryList* self = reinterpret_cast<UrlHistoryList*>(data);
    self->hideWidgetPretty();
}

}/* namespace base_ui */
} /* namespace tizen_browser */
