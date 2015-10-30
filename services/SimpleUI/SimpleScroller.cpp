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

#include "SimpleScroller.h"
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

SimpleScroller::SimpleScroller(Evas_Object* parent, Evas_Object *content)
{
    m_scroller = elm_scroller_add(parent);
    elm_scroller_propagate_events_set(m_scroller, EINA_TRUE);
    evas_object_smart_callback_add(m_scroller, "edge,top", _scrollTopReached, m_scroller);
    evas_object_smart_callback_add(m_scroller, "edge,bottom", _scrollBottomReached, m_scroller);
    evas_object_smart_callback_add(m_scroller, "scroll,down", _scrollDown, m_scroller);
    evas_object_smart_callback_add(m_scroller, "scroll,up", _scrollUp, m_scroller);
    elm_object_focus_allow_set(m_scroller, EINA_FALSE);
    elm_scroller_policy_set(m_scroller, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_AUTO);
    elm_scroller_content_min_limit(m_scroller, EINA_FALSE, EINA_FALSE);
    elm_object_content_set(m_scroller, content);
    evas_object_show(m_scroller);
}

Evas_Object* SimpleScroller::getEvasObjectPtr()
{
    return m_scroller;
}

void SimpleScroller::_scrollTopReached(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("Top");
    elm_object_signal_emit(reinterpret_cast<Evas_Object*>(data), "hide,top,shadow", "");
}

void SimpleScroller::_scrollBottomReached(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("Bottom");
    elm_object_signal_emit(reinterpret_cast<Evas_Object*>(data), "hide,bottom,shadow", "");
}

void SimpleScroller::_scrollDown(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("Down");
    elm_object_signal_emit(reinterpret_cast<Evas_Object*>(data), "show,top,shadow", "");
}

void SimpleScroller::_scrollUp(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("Up");
    elm_object_signal_emit(reinterpret_cast<Evas_Object*>(data), "show,bottom,shadow", "");
}

}
}
