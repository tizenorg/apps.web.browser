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

#include "ZoomList.h"
#include "BrowserAssert.h"
#include "BrowserLogger.h"
#include <stdio.h>
#include <stdlib.h>

namespace tizen_browser
{
namespace base_ui
{


ZoomList::ZoomList(std::shared_ptr< Evas_Object > mainWindow, Evas_Object* parentButton)
    : MenuButton(mainWindow, parentButton)
    , m_trackFirst(0)
    , m_items(0)
{
    BROWSER_LOGD("[%s:%d] this: %x", __PRETTY_FUNCTION__, __LINE__, this);
    m_list = elm_list_add(m_window.get());

    evas_object_smart_callback_add(m_list, "item,focused", focusItem, this);
    evas_object_smart_callback_add(m_list, "item,unfocused", unFocusItem, this);
    evas_object_smart_callback_add(m_list, "activated", _item_clicked_cb, this);

    elm_object_style_set(m_list, "zoom_list");
    elm_list_mode_set(m_list, ELM_LIST_EXPAND);
    evas_object_size_hint_align_set(m_list, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(m_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_list_go(m_list);
    evas_object_show(m_list);
}

ZoomList::~ZoomList()
{
}


void ZoomList::addItem(const char* text, zoom_type value)
{
    zoom_data *data = new zoom_data();
    data->value = value;
    data->zoom_list = this;

    elm_list_item_append(m_list, text, NULL, NULL, ZoomList::_item_clicked_cb, (void *)data);
    elm_list_go(m_list);

    m_items++;
}

void ZoomList::setZoom(zoom_type value)
{
    Elm_Object_Item * item = elm_list_first_item_get(m_list);
    while (item != NULL)
    {
        zoom_type *elem_zoom = (zoom_type*)elm_object_item_data_get(item);
        edje_object_signal_emit(elm_list_item_object_get(item), "unchecked", "");
        if (*elem_zoom == value)
        {
            edje_object_signal_emit(elm_list_item_object_get(item), "default_checked", "");
            current_zoom = value;
        }
        item = elm_list_item_next(item);
    }
}

Evas_Object* ZoomList::getContent()
{
    evas_object_resize(m_list, 390, 0);
    return m_list;
}

MenuButton::ListSize ZoomList::calculateSize()
{
    ListSize result;

    result.width = 390;
    result.height = 590;

    return result;
}

void ZoomList::_item_clicked_cb(void* /*data*/, Evas_Object */*obj*/, void *event_info)
{
    Elm_Object_Item *item = static_cast<Elm_Object_Item *>(event_info);
    zoom_data *zoom = static_cast<zoom_data*>(elm_object_item_data_get(item));
    M_ASSERT(zoom);
    if (zoom->value != zoom->zoom_list->current_zoom)
    {
        zoom->zoom_list->zoomChanged(zoom->value);
        edje_object_signal_emit(elm_list_item_object_get(item), "checked", "");
        item = elm_list_first_item_get(zoom->zoom_list->m_list);
        while (item != NULL)
        {
            zoom_type *elem_zoom = (zoom_type*)elm_object_item_data_get(item);
            if (*elem_zoom == zoom->zoom_list->current_zoom)
            {
                edje_object_signal_emit(elm_list_item_object_get(item), "unchecked", "");
                zoom->zoom_list->current_zoom = zoom->value;
                break;
            }
            item = elm_list_item_next(item);
        }
    }
    zoom->zoom_list->hidePopup();
}

Evas_Object* ZoomList::getFirstFocus()
{
    return m_list;
}

void ZoomList::focusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,in", "over2");
}

void ZoomList::unFocusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,out", "over2");
}

}

}
