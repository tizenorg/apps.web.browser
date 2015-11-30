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

#ifndef WEBSITEHISTORYVISITITEM_H_
#define WEBSITEHISTORYVISITITEM_H_

#include <string>
#include <Elementary.h>
#include "HistoryDayItemDataTypedef.h"

namespace tizen_browser {
namespace base_ui {

/**
 * Represtents the website visit item: website visit title, website url,
 * visite time.
 */
class WebsiteHistoryVisitItem
{
public:
    WebsiteHistoryVisitItem(WebsiteVisitItemDataPtr visitItemData);
    virtual ~WebsiteHistoryVisitItem();
    Evas_Object* init(Evas_Object* parent);

private:
    WebsiteVisitItemDataPtr m_websiteVisitItemData;

    std::string m_websiteTitle;
    std::string m_websiteUrl;
    std::string m_websiteTimestamp;

    Evas_Object* m_layout = nullptr;
    // box including website header, separator and list of the visits items
    Evas_Object* m_boxMain = nullptr;
    Evas_Object* m_tableVisit = nullptr;

    Evas_Object* m_labelTitle = nullptr;
    Evas_Object* m_labelUrl = nullptr;
    Evas_Object* m_labelTimestamp = nullptr;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* WEBSITEHISTORYVISITITEM_H_ */
