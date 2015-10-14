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

#ifndef WIDGETLISTMANAGER_H_
#define WIDGETLISTMANAGER_H_

#include <memory>
#include <Evas.h>
#include "services/HistoryService/HistoryItem.h"

namespace tizen_browser
{
namespace services
{

/**
 * Interface for classes managing list-like widgets.
 * TODO: consider if it could be used in whole application, not only for URL
 * list.
 */
class WidgetListManager
{
public:
	virtual ~WidgetListManager()
	{
	}
	virtual Evas_Object* createWidget(Evas_Object* parentLayout) = 0;
	virtual Evas_Object* getWidget() = 0;
	virtual void hideWidget() = 0;
	virtual void showWidget(const std::string& editedUrl,
			std::shared_ptr<services::HistoryItemVector> matchedEntries) = 0;
	virtual void onMouseClick() = 0;
};

} /* namespace services */
} /* namespace tizen_browser */

#endif /* WIDGETLISTMANAGER_H_ */
