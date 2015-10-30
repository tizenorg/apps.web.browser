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

#ifndef ZOOMLIST_H
#define ZOOMLIST_H

#include <boost/signals2/signal.hpp>
#include <Elementary.h>
#include <map>
#include "MenuButton.h"

namespace tizen_browser
{
namespace base_ui
{

typedef enum {
    ZOOM_TYPE_50 = 50,
    ZOOM_TYPE_75 = 75,
    ZOOM_TYPE_100 = 100,
    ZOOM_TYPE_150 = 150,
    ZOOM_TYPE_200 = 200,
    ZOOM_TYPE_300 = 300,
} zoom_type;


class ZoomList:public MenuButton{

public:
    ZoomList(std::shared_ptr< Evas_Object > mainWindow, Evas_Object* parentButton);
    ~ZoomList();
    void addItem(const char* text, zoom_type value);
    void setZoom(zoom_type value);
    virtual Evas_Object *getContent();
    virtual ListSize calculateSize();
    boost::signals2::signal<void (int)> zoomChanged;

    virtual Evas_Object* getFirstFocus();

private:
    Evas_Object *m_list;
    Evas_Object *m_trackFirst;
    zoom_type current_zoom;
    int m_items;
    static void _item_clicked_cb(void* data, Evas_Object *obj, void *event_info);
    struct zoom_data{
        zoom_type value;
        ZoomList *zoom_list;
    };

    static void focusItem(void* data, Evas_Object* obj, void* event_info);
    static void unFocusItem(void* data, Evas_Object* obj, void* event_info);
};

}

}

#endif //ZOOMLIST_H
