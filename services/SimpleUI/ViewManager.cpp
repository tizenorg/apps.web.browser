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

/*
 * ViewManager.cpp
 *
 *  Created on: Sep 11, 2015
 *      Author: m.lapinski@samsung.com
 */

#include <Elementary.h>

#include "ViewManager.h"
#include "core/BrowserLogger.h"
#include "core/ServiceManager/Debug/BrowserAssert.h"

namespace tizen_browser{
namespace base_ui{

ViewManager::ViewManager(Evas_Object* parentWindow)
   :m_mainLayout(nullptr)
   ,m_previousTop(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parentWindow);
    m_mainLayout = elm_layout_add(parentWindow);
    evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (m_mainLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    if(!elm_object_style_set (m_mainLayout,"content-back"))
        BROWSER_LOGD("[%s:%d]  elm_object_style_set falied.",__PRETTY_FUNCTION__, __LINE__);
    elm_win_resize_object_add(parentWindow, m_mainLayout);
    evas_object_show(m_mainLayout);
}

void ViewManager::popStackTo(interfaces::AbstractUIComponent* view)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(view);

    if(!m_viewStack.empty())
        m_previousTop = m_viewStack.top();

    while(!m_viewStack.empty())
    {
        if (m_viewStack.top() == view)
            break;
        m_viewStack.pop();
    }
    updateLayout();
}

void ViewManager::popTheStack()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_viewStack.empty())
    {
        m_previousTop = m_viewStack.top();
        m_viewStack.pop();
        updateLayout();
    }
}

void ViewManager::pushViewToStack(interfaces::AbstractUIComponent* view)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(view);
    m_viewStack.push(view);
    updateLayout();
}


void ViewManager::updateLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object* swallowed = elm_layout_content_get(m_mainLayout, "elm.swallow.content");

    if (!m_viewStack.empty())
    {
        if (m_viewStack.top()->getContent() == swallowed)
        {
            BROWSER_LOGD("[%s:%d] Top of stack is already visible!!!",
                         __PRETTY_FUNCTION__, __LINE__);
            return;
        }
        if(m_previousTop)
            m_previousTop->hideUI();
        elm_layout_content_unset(m_mainLayout, "elm.swallow.content");
        elm_layout_content_set(m_mainLayout, "elm.swallow.content", m_viewStack.top()->getContent());
        m_viewStack.top()->showUI();
    }
    else
    {
        BROWSER_LOGD("[%s:%d] Stack is empty!!!",__PRETTY_FUNCTION__, __LINE__);
        if(m_previousTop)
             m_previousTop->hideUI();

        elm_layout_content_unset(m_mainLayout, "elm.swallow.content");
        elm_layout_content_set(m_mainLayout, "elm.swallow.content", nullptr);
    }
}

Evas_Object* ViewManager::getContent()
{
    M_ASSERT(m_mainLayout);
    return m_mainLayout;
}

}//namespace base_ui
}//names1pace tizen_browser
