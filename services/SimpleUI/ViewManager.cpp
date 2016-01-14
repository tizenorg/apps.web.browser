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
#include <Ecore.h>
#include <Ecore_Wayland.h>
#include <string>

#include "ViewManager.h"
#include "core/BrowserLogger.h"
#include "core/ServiceManager/Debug/BrowserAssert.h"

namespace tizen_browser{
namespace base_ui{

ViewManager::ViewManager()
   : m_mainLayout(nullptr)
   , m_parentWindow(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void ViewManager::init(Evas_Object* parentWindow)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parentWindow);
    m_mainLayout = elm_layout_add(parentWindow);
    evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set (m_mainLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Eina_Bool ret = elm_layout_file_set(m_mainLayout,
                                        (std::string(EDJE_DIR)
                                        + std::string("SimpleUI/ViewManager.edj")).c_str(),
                                        "main_layout");
    if (!ret)
        BROWSER_LOGD("[%s:%d]  elm_layout_file_set falied !!!",__PRETTY_FUNCTION__, __LINE__);

    elm_win_resize_object_add(parentWindow, m_mainLayout);
    evas_object_show(m_mainLayout);
}

ViewManager::~ViewManager()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_del(m_mainLayout);
}

void ViewManager::popStackTo(interfaces::AbstractUIComponent* view)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(view);
    interfaces::AbstractUIComponent* previousView = m_viewStack.top();

    while(!m_viewStack.empty() && m_viewStack.top() != view)
    {
        m_viewStack.pop();
    }
    updateLayout(previousView);
    BROWSER_LOGD("[%s:%d] new top: %p", __PRETTY_FUNCTION__, __LINE__, topOfStack());
}

void ViewManager::popTheStack()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(!m_viewStack.empty())
    {
        interfaces::AbstractUIComponent* previousView = m_viewStack.top();
        m_viewStack.pop();
        updateLayout(previousView);
    }
    BROWSER_LOGD("[%s:%d] new top: %p", __PRETTY_FUNCTION__, __LINE__, topOfStack());
}

void ViewManager::pushViewToStack(interfaces::AbstractUIComponent* view)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    M_ASSERT(view);
    if (topOfStack() == view)
    {
       BROWSER_LOGD("[%s:%d] View %p is already on stack !!!",
                          __PRETTY_FUNCTION__, __LINE__, view);
       return;
    }
    interfaces::AbstractUIComponent* previousView = topOfStack();
    m_viewStack.push(view);
    updateLayout(previousView);
    BROWSER_LOGD("[%s:%d] new top: %p", __PRETTY_FUNCTION__, __LINE__, topOfStack());
}


void ViewManager::updateLayout(interfaces::AbstractUIComponent* previousView)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Object* swallowed = elm_layout_content_get(m_mainLayout, "content");
    if (!m_viewStack.empty())
    {
        if (m_viewStack.top()->getContent() == swallowed)
        {
            BROWSER_LOGD("[%s:%d] Top of stack is already visible!!!",
                         __PRETTY_FUNCTION__, __LINE__);
            return;
        }
        if(previousView)
            previousView->hideUI();
        elm_layout_content_unset(m_mainLayout, "content");
        elm_layout_content_set(m_mainLayout, "content", m_viewStack.top()->getContent());

        m_viewStack.top()->showUI();
    }
    else
    {
        BROWSER_LOGD("[%s:%d] Stack is empty!!!",__PRETTY_FUNCTION__, __LINE__);
        if(previousView)
             previousView->hideUI();

        elm_layout_content_unset(m_mainLayout, "content");
        elm_layout_content_set(m_mainLayout, "content", nullptr);
    }
}

Evas_Object* ViewManager::getContent()
{
    M_ASSERT(m_mainLayout);
    return m_mainLayout;
}


interfaces::AbstractUIComponent* ViewManager::topOfStack()
{
    if(!m_viewStack.empty())
        return m_viewStack.top();
    else
        return nullptr;
}

void ViewManager::decreaseWindow()
{
    if (*isLandscape())
        elm_object_signal_emit(getContent(), "open_landscape_ime", "ui");
    else
        elm_object_signal_emit(getContent(), "open_portrait_ime", "ui");
}

void ViewManager::enlargeWindow()
{
    elm_object_signal_emit(getContent(), "hidden_ime", "ui");
}


}//namespace base_ui
}//names1pace tizen_browser
