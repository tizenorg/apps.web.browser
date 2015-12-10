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

#ifndef HISTORYDAYSLISTMANAGERMOB_H_
#define HISTORYDAYSLISTMANAGERMOB_H_

#include <memory>
#include <Elementary.h>
#include <string>
#include <vector>
#include "HistoryDaysListManager.h"
#include "HistoryDayItemDataTypedef.h"

namespace tizen_browser {
namespace base_ui {

class HistoryDayItemMob;
typedef std::shared_ptr<HistoryDayItemMob> HistoryDayItemMobPtr;
enum class HistoryPeriod;

class HistoryDaysListManagerMob : public HistoryDaysListManager
{
public:
    HistoryDaysListManagerMob();
    virtual ~HistoryDaysListManagerMob();
    Evas_Object* createDaysList(Evas_Object* parentLayout) override;
    void addHistoryItems(const std::map<std::string, services::HistoryItemVector>&,
            HistoryPeriod period) override;
    void clear() override;

private:
    void appendDayItem(HistoryDayItemDataPtr dayItemData);

    std::vector<HistoryDayItemMobPtr> m_dayItems;

    Evas_Object* m_scrollerMain = nullptr;
    Evas_Object* m_layoutScroller = nullptr;
    Evas_Object* m_boxMain  = nullptr;

    std::string m_daysListEdjFilePath;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* HISTORYDAYSLISTMANAGERMOB_H_ */
