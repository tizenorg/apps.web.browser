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

#ifndef HISTORYDAYSLISTMANAGER_H_
#define HISTORYDAYSLISTMANAGER_H_

#include <services/HistoryService/HistoryItemTypedef.h>
#include <Elementary.h>
#include <map>
#include <string>

namespace tizen_browser {
namespace base_ui {

enum class HistoryPeriod;

class HistoryDaysListManager
{
public:
    virtual ~HistoryDaysListManager() {}
    virtual Evas_Object* createDaysList(Evas_Object* parentLayout) = 0;
    virtual void addHistoryItems(
            const std::map<std::string, services::HistoryItemVector>&,
            HistoryPeriod period) = 0;
    // clear everything including efl objects (result: empty list)
    virtual void clear() = 0;
};

}
}

#endif /* HISTORYDAYSLISTMANAGER_H_ */
