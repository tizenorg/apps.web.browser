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

#ifndef WEBSITEHISTORYITEMVISITITEMSTV_H_
#define WEBSITEHISTORYITEMVISITITEMSTV_H_

#include <Elementary.h>
#include <string>
#include <vector>
#include "../../HistoryDayItemDataTypedef.h"
#include <boost/signals2/signal.hpp>

namespace tizen_browser{
namespace base_ui{

class WebsiteHistoryItemVisitItemsTv
{
    struct LayoutButtonPair
    {
        Evas_Object* layout;
        Evas_Object* selectButton;
    };
public:
    WebsiteHistoryItemVisitItemsTv(
            const std::vector<WebsiteVisitItemDataPtr> websiteVisitItems);
    virtual ~WebsiteHistoryItemVisitItemsTv();
    Evas_Object* init(Evas_Object* parent, const std::string& edjeFilePath);
    void setFocusChain(Evas_Object* obj);

    static boost::signals2::signal<void(const WebsiteVisitItemDataPtr)>
    signalWebsiteVisitItemClicked;

private:
    LayoutButtonPair createLayoutVisitItem(Evas_Object* parent,
            const std::string& edjeFilePath,
            WebsiteVisitItemDataPtr websiteVisitItemData);
    Evas_Object* createLayoutVisitItemDate(Evas_Object* parent,
            const std::string& edjeFilePath,
            WebsiteVisitItemDataPtr websiteVisitItemData);
    Evas_Object* createLayoutVisitItemUrl(Evas_Object* parent,
            const std::string& edjeFilePath,
            WebsiteVisitItemDataPtr websiteVisitItemData);
    void initCallbacks();
    void deleteCallbacks();
    static void _buttonSelectClicked(void* data, Evas_Object* obj, void* event_info);

    std::vector<WebsiteVisitItemDataPtr> m_websiteVisitItems;

    std::vector<Evas_Object*> m_buttonsSelect;
    Evas_Object* m_layoutHistoryItemVisitItems;
    Evas_Object* m_boxMainVertical;
};

}
}

#endif /* WEBSITEHISTORYITEMVISITITEMSTV_H_ */
