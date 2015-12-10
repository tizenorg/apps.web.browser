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

#ifndef WEBSITEHISTORYITEMTV_H_
#define WEBSITEHISTORYITEMTV_H_

#include <Elementary.h>
#include <string>
#include <vector>
#include <memory>
#include "../../HistoryDayItemDataTypedef.h"
#include "../HistoryDaysListManagerEdjeTv.h"


namespace tizen_browser{
namespace base_ui{

class WebsiteHistoryItemTitleTv;
typedef std::shared_ptr<WebsiteHistoryItemTitleTv> WebsiteHistoryItemTitleTvPtr;
class WebsiteHistoryItemVisitItemsTv;
typedef std::shared_ptr<WebsiteHistoryItemVisitItemsTv> WebsiteHistoryItemVisitItemsTvPtr;

class WebsiteHistoryItemTv
{
public:
    WebsiteHistoryItemTv(WebsiteHistoryItemDataPtr websiteHistoryItemData);
    virtual ~WebsiteHistoryItemTv();
    Evas_Object* init(Evas_Object* parent,
            HistoryDaysListManagerEdjeTvPtr edjeFiles);

private:
    Evas_Object* createBoxMainHorizontal(Evas_Object* parent,
            HistoryDaysListManagerEdjeTvPtr edjeFiles);

    WebsiteHistoryItemDataPtr m_websiteHistoryItemData;
    WebsiteHistoryItemTitleTvPtr m_websiteHistoryItemTitle;
    WebsiteHistoryItemVisitItemsTvPtr m_websiteHistoryItemVisitItems;

    Evas_Object* m_layoutHistoryItem = nullptr;
    Evas_Object* m_boxMainHorizontal = nullptr;

};

}
}

#endif /* WEBSITEHISTORYITEMTV_H_ */
