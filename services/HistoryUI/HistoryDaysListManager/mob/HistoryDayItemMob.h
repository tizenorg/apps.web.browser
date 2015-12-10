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

#ifndef HISTORYDAYITEMMOB_H_
#define HISTORYDAYITEMMOB_H_

#include <memory>
#include <Elementary.h>
#include <string>
#include <vector>
#include "../HistoryDayItemDataTypedef.h"

namespace tizen_browser {
namespace base_ui {

class WebsiteHistoryItemMob;
typedef std::shared_ptr<WebsiteHistoryItemMob> WebsiteHistoryItemPtr;

/**
 * Represents the day item: day header, plus boxes with website history items.
 */
class HistoryDayItemMob
{
public:
    HistoryDayItemMob(HistoryDayItemDataPtr dayItemData);
    virtual ~HistoryDayItemMob();
    Evas_Object* init(Evas_Object* parent);

private:
    HistoryDayItemDataPtr m_dayItemData;
    std::vector<WebsiteHistoryItemPtr> m_websiteHistoryItems;

    Evas_Object* m_layout = nullptr;

    // box including day header and websites box
    Evas_Object* m_boxMain = nullptr;
    // box including day label
    Evas_Object* m_boxHeader;
    // box including website boxes
    Evas_Object* m_boxWebsites;

    Evas_Object* m_labelDay = nullptr;

    std::string m_day;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* HISTORYDAYITEMMOB_H_ */
