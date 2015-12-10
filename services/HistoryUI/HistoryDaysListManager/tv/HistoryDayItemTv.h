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

#ifndef HistoryDayItemTv_H_
#define HistoryDayItemTv_H_

#include <memory>
#include <Elementary.h>
#include <string>
#include <vector>
#include "../HistoryDayItemDataTypedef.h"
#include "HistoryDaysListManagerEdjeTv.h"

namespace tizen_browser{
namespace base_ui{

class WebsiteHistoryItemTv;
typedef std::shared_ptr<WebsiteHistoryItemTv> WebsiteHistoryItemTvPtr;

class HistoryDayItemTv {
public:
    HistoryDayItemTv(HistoryDayItemDataPtr dayItemData);
    virtual ~HistoryDayItemTv();
    Evas_Object* init(Evas_Object* parent, HistoryDaysListManagerEdjeTvPtr edjeFiles);

private:
    void initBoxWebsites(HistoryDaysListManagerEdjeTvPtr edjeFiles);
    Evas_Object* createScrollerWebsites(Evas_Object* parent, HistoryDaysListManagerEdjeTvPtr edjeFiles);

    HistoryDayItemDataPtr m_dayItemData;
    std::vector<WebsiteHistoryItemTvPtr> m_websiteHistoryItems;

    Evas_Object* m_layoutDayColumn = nullptr;

    // vertical box: day label + websites history scroller
    Evas_Object* m_boxMainVertical = nullptr;

    Evas_Object* m_layoutHeader = nullptr;
    Evas_Object* m_boxHeader = nullptr;

    // box containing scroller
    Evas_Object* m_layoutBoxScrollerWebsites = nullptr;
    Evas_Object* m_boxScrollerWebsites;
    Evas_Object* m_scrollerWebsites;
    Evas_Object* m_layoutScrollerWebsites;
    Evas_Object* m_boxWebsites;
};

}
}

#endif /* HistoryDayItemTv_H_ */
