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
#include <boost/signals2/signal.hpp>

namespace tizen_browser{
namespace base_ui{

class WebsiteHistoryItemTv;
typedef std::shared_ptr<WebsiteHistoryItemTv> WebsiteHistoryItemTvPtr;
class HistoryDeleteManager;
typedef std::shared_ptr<const HistoryDeleteManager> HistoryDeleteManagerPtrConst;

class HistoryDayItemTv {
public:
    HistoryDayItemTv(HistoryDayItemDataPtr dayItemData,
            HistoryDeleteManagerPtrConst historyDeleteManager);
    virtual ~HistoryDayItemTv();
    Evas_Object* init(Evas_Object* parent, HistoryDaysListManagerEdjeTvPtr edjeFiles);
    void setFocusChain(Evas_Object* obj);
    Evas_Object* getLayoutDayColumn() const {return m_layoutMain;}

    // invoked when main layout is already removed. prevents from second
    // evas_object_del() on main layout in destructor
    void setEflObjectsAsDeleted();
    HistoryDeleteManagerPtrConst getDeleteManager() const {return m_historyDeleteManager;}
    Evas_Object* getImageClear() {return m_imageClear;}

    static boost::signals2::signal<void(const HistoryDayItemTv*)> signalHeaderFocus;
    static boost::signals2::signal<void(const HistoryDayItemTv*)>
    signalButtonFocus;
    static boost::signals2::signal<void(const HistoryDayItemDataPtr)>
    signaButtonClicked;
private:
    void initBoxWebsites(HistoryDaysListManagerEdjeTvPtr edjeFiles);
    Evas_Object* createScrollerWebsites(Evas_Object* parent,
            HistoryDaysListManagerEdjeTvPtr edjeFiles);
    Evas_Object* createImageClear(Evas_Object* parent,
            const std::string& edjeFilePath);
    void initCallbacks();
    void deleteCallbacks();
    static void _layoutHeaderFocused(void* data, Evas_Object* obj,
            void* event_info);
    static void _buttonSelectClicked(void *data, Evas_Object *obj, void *event_info);
    static void _buttonSelectFocused(void *data, Evas_Object *obj, void *event_info);
    static void _buttonSelectUnfocused(void *data, Evas_Object *obj, void *event_info);

    /// used to indicate, if efl object were already deleted
    bool m_eflObjectsDeleted = false;

    HistoryDayItemDataPtr m_dayItemData;
    std::vector<WebsiteHistoryItemTvPtr> m_websiteHistoryItems;

    // main layout: all children widgets will have this as their parent
    Evas_Object* m_layoutMain = nullptr;
    Evas_Object* m_buttonSelect = nullptr;
    Evas_Object* m_imageClear = nullptr;

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

    HistoryDeleteManagerPtrConst m_historyDeleteManager;
};

}
}

#endif /* HistoryDayItemTv_H_ */
