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

#ifndef WEBSITEHISTORYITEMTITLETV_H_
#define WEBSITEHISTORYITEMTITLETV_H_

#include <Elementary.h>
#include <string>
#include "../../HistoryDayItemDataTypedef.h"
#include <boost/signals2/signal.hpp>

namespace tizen_browser{
namespace base_ui{

class WebsiteHistoryItemTitleTv
{
public:
    WebsiteHistoryItemTitleTv(WebsiteHistoryItemDataPtr websiteHistoryItemData);
    virtual ~WebsiteHistoryItemTitleTv();
    Evas_Object* init(Evas_Object* parent, const std::string& edjeFilePath);
    void setFocusChain(Evas_Object* obj);

    static boost::signals2::signal<void(const WebsiteHistoryItemDataPtr)>
    signalWebsiteHistoryItemClicked;

private:
    Evas_Object* createLayoutIcon(Evas_Object* parent,
            const std::string& edjeFilePath);
    Evas_Object* createLayoutSummary(Evas_Object* parent,
            const std::string& edjeFilePath);
    void initCallbacks();
    void deleteCallbacks();
    static void _buttonSelectClicked(void* data, Evas_Object* obj, void* event_info);

    WebsiteHistoryItemDataPtr m_websiteHistoryItemData;

    Evas_Object* m_buttonSelect;
    Evas_Object* m_layoutHistoryItemTitle;
    Evas_Object* m_boxMainHorizontal;
};

}
}

#endif /* WEBSITEHISTORYITEMTITLETV_H_ */