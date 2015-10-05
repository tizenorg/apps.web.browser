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

#ifndef BOOKSMARK_MANAGER_H
#define BOOKSMARK_MANAGER_H
#include <Evas.h>
#include <Elementary.h>
#include <string>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <deque>

#include "Action.h"
#include "MenuButton.h"

namespace tizen_browser
{
namespace base_ui
{

struct BookmarksManagerItem{
    std::shared_ptr<Action> action;
    BookmarksManagerItem();
    BookmarksManagerItem(std::shared_ptr<Action> action);
    BookmarksManagerItem(const BookmarksManagerItem& source);
    BookmarksManagerItem(const BookmarksManagerItem* source);
    ~BookmarksManagerItem();
};


class BookmarksManager: public MenuButton
{
public:
    BookmarksManager(std::shared_ptr< Evas_Object > mainWindow, Evas_Object* parentButton);
    ~BookmarksManager();
    virtual Evas_Object *getContent();
    virtual ListSize calculateSize();
    virtual Evas_Object* getFirstFocus();
    void addAction(sharedAction action);
    void onEnabledChanged(sharedAction action);
    void refreshAction(sharedAction action);
    void showPopup();
    void hidePopup();
    bool isPopupShown();
    virtual bool canBeDismissed();
private:
    static char        *gridTextGet(void *data, Evas_Object* obj, const char *part);
    static Evas_Object *gridOptionLabelGet(void* data, Evas_Object* /*obj*/, const char* part);
    static void itemClicked(void *data, Evas_Object *o, void *event_info);
    static void block_clicked(void *data, Evas_Object *obj, void *event_info);

    void showInternalPopup(Elm_Object_Item* parent);
    static void dismissedCtxPopup(void* data, Evas_Object* obj, void* event_info);
    static void radioChanged(void *data, Evas_Object *obj, void *event_info);

private:
    Evas_Object *m_popup;
    Evas_Object *m_genlist;
    Evas_Object *m_internalPopup;
    Elm_Object_Item *m_trackedItem;
    Elm_Genlist_Item_Class *m_itemClass;
    bool m_internalPopupVisible;
    std::map<Elm_Object_Item*, std::shared_ptr<BookmarksManagerItem>> m_bookmarks_managerItemsMap;
    std::map<sharedAction, Elm_Object_Item*> m_actionButtonMap;
    enum CheckState{
        CheckStateOn=1
       ,CheckStateOff=2
    };
    CheckState m_checkState;
    void realShow(Evas_Object* popup);
    void unbindFocus();
    bool m_isPopupShown;

    static void focusItem(void* data, Evas_Object* obj, void* event_info);
    static void unFocusItem(void* data, Evas_Object* obj, void* event_info);
};

}//namespace base_ui

}//namespace tizen_browser

#endif // BOOKSMARK_MANAGER_H
