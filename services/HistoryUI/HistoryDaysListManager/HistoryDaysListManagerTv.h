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

#ifndef HISTORYDAYSLISTMANAGERTV_H_
#define HISTORYDAYSLISTMANAGERTV_H_

#include <memory>
#include <Elementary.h>
#include <string>
#include <vector>
#include "HistoryDayItemDataTypedef.h"
#include "HistoryDaysListManager.h"
#include "tv/HistoryDaysListManagerEdjeTv.h"

namespace tizen_browser {
namespace base_ui {

class HistoryDayItemTv;
typedef std::shared_ptr<HistoryDayItemTv> HistoryDayItemTvPtr;
class HistoryDeleteManager;
typedef std::shared_ptr<const HistoryDeleteManager> HistoryDeleteManagerPtrConst;

class HistoryDaysListManagerTv : public HistoryDaysListManager
{
public:
    HistoryDaysListManagerTv(HistoryDeleteManagerPtrConst deleteManager);
    virtual ~HistoryDaysListManagerTv();
    Evas_Object* createDaysList(Evas_Object* parentLayout) override;
    void addHistoryItems(const std::map<std::string, services::HistoryItemVector>&,
            HistoryPeriod period) override;
    void clear() override;
    void setFocusChain(Evas_Object* obj) override;

    void onHistoryDayItemHeaderFocus(const HistoryDayItemTv* focusedItem);
    void onWebsiteHistoryItemClicked(const WebsiteHistoryItemDataPtr websiteHistoryItemData);
    void onWebsiteHistoryItemVisitItemClicked(const WebsiteVisitItemDataPtr websiteVisitItemData);

private:
    void connectSignals();
    void appendDayItem(HistoryDayItemDataPtr dayItemData);
    int getHistoryItemIndex(const HistoryDayItemTv* item);
    void scrollToDayItem(const HistoryDayItemTv* item);

    HistoryDaysListManagerEdjeTvPtr m_edjeFiles;
    std::vector<HistoryDayItemTvPtr> m_dayItems;

    Evas_Object* m_scrollerDaysColumns = nullptr;
    Evas_Object* m_layoutScrollerDaysColumns = nullptr;
    Evas_Object* m_boxDaysColumns  = nullptr;

    HistoryDeleteManagerPtrConst m_historyDeleteManager;
};

}
}

#endif /* HISTORYDAYSLISTMANAGERTV_H_ */
