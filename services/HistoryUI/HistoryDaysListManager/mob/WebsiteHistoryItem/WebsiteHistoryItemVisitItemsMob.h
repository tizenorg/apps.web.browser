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

#ifndef WEBSITEHISTORYITEMVISITITEMSMOB_H_
#define WEBSITEHISTORYITEMVISITITEMSMOB_H_

#include <Elementary.h>
#include <string>
#include <vector>
#include "../../HistoryDayItemDataTypedef.h"
#include <boost/signals2/signal.hpp>

namespace tizen_browser {
namespace base_ui {

class WebsiteHistoryItemVisitItemsMob
{
    struct LayoutVisitItemObjects
    {
        Evas_Object* layout = nullptr;
        Evas_Object* buttonSelect = nullptr;
    };
    struct VisitItemObjects
    {
        WebsiteVisitItemDataPtr websiteVisitItemData;
        struct LayoutVisitItemObjects layoutVisitItemObjects;
    };
public:
    WebsiteHistoryItemVisitItemsMob(
            const std::vector<WebsiteVisitItemDataPtr> websiteVisitItems);
    virtual ~WebsiteHistoryItemVisitItemsMob();
    Evas_Object* init(Evas_Object* parent, const std::string& edjeFilePath);
    /**
     * @brief invoked when main layout is already removed.
     * prevents from second evas_object_del() on main layout in destructor
     */
    void setEflObjectsAsDeleted();

    static boost::signals2::signal<void(const WebsiteVisitItemDataPtr)>
    signalButtonClicked;

private:
    LayoutVisitItemObjects createLayoutVisitItem(Evas_Object* parent,
            const std::string& edjeFilePath,
            WebsiteVisitItemDataPtr websiteVisitItemData);

    void initCallbacks();
    static void _buttonSelectClicked(void* data, Evas_Object* obj, void* event_info);

    /// used to indicate, if efl object were already deleted
    bool m_eflObjectsDeleted;

    std::vector<VisitItemObjects> m_websiteVisitItems;
    Evas_Object* m_layoutMain;
    Evas_Object* m_boxMainVertical;
};

}
}

#endif /* WEBSITEHISTORYITEMVISITITEMSMOB_H_ */
