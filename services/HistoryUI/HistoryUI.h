/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#ifndef HISTORYUI_H
#define HISTORYUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT HistoryUI
    : public tizen_browser::interfaces::AbstractUIComponent
    , public tizen_browser::core::AbstractService
{
public:
    HistoryUI();
    ~HistoryUI();
    void init(Evas_Object *main_layout);
    virtual std::string getName();
    void addHistoryItem(std::shared_ptr<services::HistoryItem>);
    void addHistoryItems(std::shared_ptr<services::HistoryItemVector>);
    void removeHistoryItem(const std::string& uri);
    void clearItems();
    void hide();
    void showActionBar();
    void show(Evas_Object *main_layout);
    void addItems();
    boost::signals2::signal<void (const std::string&)> closeHistoryUIClicked;
    boost::signals2::signal<void (const std::string&)> clearHistoryClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::HistoryItem>)> historyItemClicked;
    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::HistoryItem>)> historyDeleteClicked;
private:
    static char* _grid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _history_grid_content_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _grid_content_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object* _listActionBarContentGet(void *data, Evas_Object *obj, const char *part);
    static void _clearHistory_clicked(void *data, Evas_Object *obj, void *event_info);
    static char* _listTodayTextGet(void* data, Evas_Object* obj , const char* part);
    static void _close_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void _history_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);

    void setEmptyGengrid(bool setEmpty);

    Evas_Object *m_history_layout;
    Evas_Object *m_actionBar;
    Evas_Object *m_genListToday;
    Elm_Genlist_Item_Class *m_itemClassToday;
    Evas_Object *m_gengrid;
    Evas_Object *m_parent;
    Elm_Gengrid_Item_Class * m_item_class;
    std::map<std::string,Elm_Object_Item*> m_map_history_views;
    std::string m_edjFilePath;
    Evas_Object *createNoHistoryLabel();
};

}
}

#endif // BOOKMARKSUI_H
