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

#ifndef HistoryDayItemMob_H_
#define HistoryDayItemMob_H_

#include <memory>
#include <Elementary.h>
#include <vector>
#include "../HistoryDayItemDataTypedef.h"
#include "../HistoryDaysListManagerEdje.h"

namespace tizen_browser {
namespace base_ui {

class WebsiteHistoryItemMob;
typedef std::shared_ptr<WebsiteHistoryItemMob> WebsiteHistoryItemMobPtr;

class HistoryDayItemMob
{
public:
    HistoryDayItemMob(HistoryDayItemDataPtr dayItemData);
    virtual ~HistoryDayItemMob();
    Evas_Object* init(Evas_Object* parent,
            HistoryDaysListManagerEdjePtr edjeFiles);

    /**
     * @brief invoked when main layout is already removed.
     * prevents from second evas_object_del() on main layout in destructor
     */
    void setEflObjectsAsDeleted();

private:
    Evas_Object* createBoxWebsites(Evas_Object* parent,
            HistoryDaysListManagerEdjePtr edjeFiles);

    /// used to indicate, if efl object were already deleted
    bool m_eflObjectsDeleted;

    HistoryDayItemDataPtr m_dayItemData;
    std::vector<WebsiteHistoryItemMobPtr> m_websiteHistoryItems;

    Evas_Object* m_layoutMain;
    // vertical box: day label + websites history scroller
    Evas_Object* m_boxMainVertical;

    Evas_Object* m_layoutHeader;
    Evas_Object* m_boxHeader;

    Evas_Object* m_layoutBoxWebsites;
    Evas_Object* m_boxWebsites;
};

}
}

#endif /* BROWSER_MERGING_SERVICES_HISTORYUI_HISTORYDAYSLISTMANAGER_MOB_HISTORYDAYITEMMOB_H_ */
