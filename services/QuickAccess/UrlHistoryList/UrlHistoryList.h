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

#ifndef URLHISTORYLIST_H_
#define URLHISTORYLIST_H_

#include <memory>
#include <Evas.h>

#include "services/HistoryService/HistoryItem.h"

using namespace std;

namespace tizen_browser
{

namespace services
{
class WidgetListManager;
typedef shared_ptr<WidgetListManager> WidgetListManagerPtr;
}

namespace base_ui
{

/**
 * Manages list of url matches (URL from history). Manages top layout, creates
 * widget displaying url items.
 */
class UrlHistoryList
{
public:
	UrlHistoryList();
	virtual ~UrlHistoryList();
	void show();
	void createLayout(Evas_Object* parentLayout);
	Evas_Object *getLayout();

	/**
	 * \brief entered url is edited (edited before acceptation)
	 */
	void onURLEntryEdit(const string& editedUrl,
			shared_ptr<services::HistoryItemVector> matchedEntries);
	void onMouseClick();

private:

	Evas_Object* m_layout;
	string m_edjFilePath;

	services::WidgetListManagerPtr m_widgetListManager;

};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* URLHISTORYLIST_H_ */
