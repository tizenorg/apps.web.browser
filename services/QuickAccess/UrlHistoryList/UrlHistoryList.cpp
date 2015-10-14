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

namespace tizen_browser
{
namespace base_ui
{

UrlHistoryList::UrlHistoryList() :
		m_layout(nullptr)
{
	m_edjFilePath = EDJE_DIR;
	m_edjFilePath.append("MainUI/UrlHistoryList.edj");
	m_widgetListManager = make_shared<services::GenlistManager>();
}

UrlHistoryList::~UrlHistoryList()
{
}

void UrlHistoryList::show()
{
	if (m_layout)
	{
		evas_object_show(m_layout);
	}
}

Evas_Object* UrlHistoryList::getLayout()
{
	return m_layout;
}

void UrlHistoryList::createLayout(Evas_Object* parentLayout)
{
	m_layout = elm_layout_add(parentLayout);
	elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "url_history_list");

	Evas_Object* widgetList = m_widgetListManager->createWidget(m_layout);
}

void UrlHistoryList::onURLEntryEdit(const string& editedUrl,
		shared_ptr<services::HistoryItemVector> matchedEntries)
{
	Evas_Object* widgetList = m_widgetListManager->getWidget();
	if (matchedEntries->size() == 0)
	{
		m_widgetListManager->hideWidget();
	}
	else
	{
		elm_object_part_content_set(m_layout, "list_swallow", widgetList);
		m_widgetListManager->showWidget(editedUrl, matchedEntries);
		evas_object_show(widgetList);
	}
}

void UrlHistoryList::onMouseClick()
{
	m_widgetListManager->onMouseClick();
}

}/* namespace base_ui */
} /* namespace tizen_browser */
