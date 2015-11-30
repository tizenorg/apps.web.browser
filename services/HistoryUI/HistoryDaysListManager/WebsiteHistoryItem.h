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

#ifndef HISTORYWEBSITEHISTORYITEM_H_
#define HISTORYWEBSITEHISTORYITEM_H_

#include <string>
#include <vector>
#include <Elementary.h>
#include "HistoryDayItemDataTypedef.h"

namespace tizen_browser {
namespace base_ui {

class WebsiteHistoryVisitItem;
typedef std::shared_ptr<WebsiteHistoryVisitItem> WebsiteHistoryVisitItemPtr;

/**
 * Represents the website item: header (image + website name), seperator,
 * list of the website's visit items.
 */
class WebsiteHistoryItem
{
public:
    WebsiteHistoryItem(WebsiteHistoryItemDataPtr websiteHistoryItemData);
    virtual ~WebsiteHistoryItem();
    Evas_Object* init(Evas_Object* parent);

private:
    WebsiteHistoryItemDataPtr m_websiteHistoryItemData;
    std::vector<WebsiteHistoryVisitItemPtr> m_websiteHistoryVisitItems;

    std::string m_websiteTitle;

    Evas_Object* m_layout = nullptr;
    // box including website header, separator and list of the visits items
    Evas_Object* m_boxMain = nullptr;

    Evas_Object* m_boxHeader = nullptr;
    Evas_Object* m_labelWebsiteTitle = nullptr;

    Evas_Object* m_boxVisitItems = nullptr;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* HISTORYWEBSITEHISTORYITEM_H_ */
