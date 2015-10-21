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
#include "../QuickAccess.h"

namespace tizen_browser {
namespace base_ui {

UrlHistoryList::UrlHistoryList(QuickAccess* quickAccess) :
        m_layout(nullptr), m_quickAccess(quickAccess)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("QuickAccess/UrlHistoryList.edj");
    m_genlistListManager = make_shared<GenlistManager>();
    m_genlistListManager->signalItemSelected.connect(
            boost::bind(&UrlHistoryList::onListItemSelect, this, _1));
    m_genlistListManager->signalWidgetFocused.connect(
            boost::bind(&UrlHistoryList::onListWidgetFocused, this));
    m_genlistListManager->signalWidgetUnfocused.connect(
            boost::bind(&UrlHistoryList::onListWidgetUnfocused, this));
}

UrlHistoryList::~UrlHistoryList()
{
}

Evas_Object* UrlHistoryList::getLayout()
{
    return m_layout;
}

void UrlHistoryList::createLayout(Evas_Object* parentLayout)
{
    m_layout = elm_layout_add(parentLayout);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "url_history_list");

    m_genlistListManager->createWidget(m_layout);
}

void UrlHistoryList::onURLEntryEditedByUser(const string& editedUrl,
        shared_ptr<services::HistoryItemVector> matchedEntries)
{
    editedUrlStatesHelper.changeState(true);

    if (matchedEntries->size() == 0) {
        m_genlistListManager->hideWidgetPretty();
    } else {
        Evas_Object* widgetList = m_genlistListManager->getWidget();
        elm_object_part_content_set(m_layout, "list_swallow", widgetList);
        m_genlistListManager->showWidget(editedUrl, matchedEntries);
        evas_object_show(widgetList);
    }
}

void UrlHistoryList::onURLEntryEdited()
{
    editedUrlStatesHelper.changeState(false);
    if (editedUrlStatesHelper.getCurrentState()
            == EditedUrlState::EDITED_OTHER_FIRST) {
        m_genlistListManager->hideWidgetPretty();
    } else {
        // in this situation scroll will not work, it has to be hidden instantly
        m_genlistListManager->hideWidgetInstant();
    }
}

void UrlHistoryList::onMouseClick()
{
    m_genlistListManager->onMouseClick();
}

void UrlHistoryList::onListItemSelect(std::string content)
{
    openURLInNewTab(make_shared < services::HistoryItem > (content),
            m_quickAccess->isDesktopMode());
}

void UrlHistoryList::onListWidgetFocused()
{
    // will be used soon: in a proper focus-chain solution
}

void UrlHistoryList::onListWidgetUnfocused()
{
    // will be used soon: in a proper focus-chain solution
}

}/* namespace base_ui */
} /* namespace tizen_browser */
