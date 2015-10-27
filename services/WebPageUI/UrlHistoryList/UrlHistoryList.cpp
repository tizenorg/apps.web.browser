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
#include "UrlHistoryList.h"
#include "GenlistManager.h"
#include "BrowserLogger.h"
#include "GenlistItemsManager.h"

namespace tizen_browser {
namespace base_ui {

UrlHistoryList::UrlHistoryList() :
        m_layout(nullptr)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("WebPageUI/UrlHistoryList.edj");
    m_genlistListManager = make_shared<GenlistManager>();
    m_genlistListManager->signalItemSelected.connect(
            boost::bind(&UrlHistoryList::onListItemSelect, this, _1));
    m_genlistListManager->signalWidgetFocused.connect(
            boost::bind(&UrlHistoryList::onListWidgetFocused, this));
    m_genlistListManager->signalWidgetUnfocused.connect(
            boost::bind(&UrlHistoryList::onListWidgetUnfocused, this));
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
    evas_object_show(widgetList);
    evas_object_show(m_layout);
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

void UrlHistoryList::saveEntryEditedContent()
{
    m_entryEditedContent = elm_entry_entry_get(m_entry);
}

void UrlHistoryList::restoreEntryEditedContent()
{
    elm_entry_entry_set(m_entry, m_entryEditedContent.c_str());
}

void UrlHistoryList::saveEntryURLContent()
{
    m_entryURLContent = elm_entry_entry_get(m_entry);
}

void UrlHistoryList::restoreEntryURLContent()
{
    elm_entry_entry_set(m_entry, m_entryURLContent.c_str());
}

Evas_Object* UrlHistoryList::getGenlist()
{
    return m_genlistListManager->getWidget();
}

void UrlHistoryList::hideWidgetPretty()
{
    m_genlistListManager->hideWidgetPretty();
}

void UrlHistoryList::onURLEntryEditedByUser(const string& editedUrl,
        shared_ptr<services::HistoryItemVector> matchedEntries)
{
    if (matchedEntries->size() == 0) {
        m_genlistListManager->hideWidgetPretty();
    } else {
        Evas_Object* widgetList = m_genlistListManager->getWidget();
        m_genlistListManager->showWidget(editedUrl, matchedEntries);
        evas_object_show(m_parent);
        evas_object_show(widgetList);
        evas_object_show(m_layout);
    }
}

void UrlHistoryList::onItemFocusChange()
{
    elm_entry_entry_set(m_entry,
            m_genlistListManager->getItemUrl(GenlistItemType::ITEM_CURRENT));
}

void UrlHistoryList::onMouseClick()
{
    m_genlistListManager->onMouseClick();
}

void UrlHistoryList::onListItemSelect(std::string content)
{
    openURLInNewTab (make_shared<services::HistoryItem>(content));hideWidgetPretty();
}

void UrlHistoryList::onListWidgetFocused()
{
    saveEntryURLContent();
}

void UrlHistoryList::onListWidgetUnfocused()
{
    m_genlistListManager->hideWidgetPretty();
    restoreEntryURLContent();
}

void UrlHistoryList::_uri_entry_editing_changed_user(void* data,
        Evas_Object* /* obj */, void* /* event_info */)
{
    UrlHistoryList* self = reinterpret_cast<UrlHistoryList*>(data);
    self->saveEntryEditedContent();
}

void UrlHistoryList::_uri_entry_unfocused(void* data, Evas_Object* /* obj */,
        void* /* event_info */)
{

    UrlHistoryList* self = reinterpret_cast<UrlHistoryList*>(data);
    self->hideWidgetPretty();
}

}/* namespace base_ui */
} /* namespace tizen_browser */
