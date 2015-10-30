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

#ifndef SIMPLESCROLLER_H
#define SIMPLESCROLLER_H

#include "Elementary.h"

namespace tizen_browser{
namespace base_ui{

class SimpleScroller
{
public:
    SimpleScroller(Evas_Object* parent, Evas_Object *content);
    //void setContent(Evas_Object *content);
    Evas_Object* getEvasObjectPtr();
private:
    Evas_Object *m_scroller;

    static void _scrollTopReached(void *data, Evas_Object *obj, void *event_info);
    static void _scrollBottomReached(void *data, Evas_Object *obj, void *event_info);
    static void _scrollDown(void *data, Evas_Object *obj, void *event_info);
    static void _scrollUp(void *data, Evas_Object *obj, void *event_info);
};

}
}

#endif // SIMPLESCROLLER_H
