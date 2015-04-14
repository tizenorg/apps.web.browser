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

#ifndef HISTORYLIST_H
#define HISTORYLIST_H
#include <memory>
#include <map>
#include <string>
#include <list>

#include <boost/signals2/signal.hpp>
#include <Evas.h>
#include <Elementary.h>

#include "MenuButton.h"

#include "HistoryItem.h"

namespace tizen_browser
{
namespace base_ui
{

namespace tzSrv = tizen_browser::services;

class HistoryList : public MenuButton
{
public:
    HistoryList(std::shared_ptr<Evas_Object> mainWindow, Evas_Object* parentButton);
    ~HistoryList();
    void addItems(tzSrv::HistoryItemVector items);
    void addItem(const std::shared_ptr<tizen_browser::services::HistoryItem> item);
    virtual Evas_Object *getContent();
    virtual ListSize calculateSize();
    virtual Evas_Object* getFirstFocus();
    boost::signals2::signal<void (const std::string & )> clickedHistoryItem;
    boost::signals2::signal<void (const std::string & )> deleteHistoryItem;
    void clearList();

    void rightPressed();
    void leftPressed();
    void enterPressed();

private:

    static Evas_Object* listItemContentGet(void *data, Evas_Object *obj, const char *part);
    static char*        listItemTextGet(void *data, Evas_Object *obj, const char *part);
    static char*        listParentItemTextGet(void *data, Evas_Object *obj, const char *part);

    static void paretn_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void item_delete_clicked_cb(void *data, Evas_Object *obj, void *event_info);

private:
    struct ItemData{
        tizen_browser::base_ui::HistoryList * h_list;
        tizen_browser::services::HistoryItem * h_item;
        Elm_Object_Item * e_item;
    };
    Evas_Object *m_genList;
    Elm_Genlist_Item_Class *m_itemClass;
    Elm_Genlist_Item_Class *m_itemClassNoFavicon;
    Elm_Genlist_Item_Class *m_parentItemClass;
    int itemsCounter;
    int parentItemsCounter;
    int itemHeight;
    int itemWidth;
    int parentItemHeight;
    std::map<boost::gregorian::date, Elm_Object_Item*> m_groupParent;
    typedef std::map<boost::gregorian::date, Elm_Object_Item*>::iterator GroupIterator;

    static void focusItem(void* data, Evas_Object* obj, void* event_info);
    static void unFocusItem(void* data, Evas_Object* obj, void* event_info);

    Elm_Object_Item *m_lastFocusedItem;
    bool m_deleteSelected;
};

} /* end of namespace base_ui */
} /* end of namespace tizen_browser */

#endif // HISTORYLIST_H
