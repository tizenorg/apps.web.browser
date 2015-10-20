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

#include "BrowserLogger.h"
#include <services/QuickAccess/UrlHistoryList/GenlistManagerCallbacks.h>

namespace tizen_browser {
namespace base_ui {

GenlistManager* GenlistManagerCallbacks::genlistManager = nullptr;

GenlistManagerCallbacks::GenlistManagerCallbacks()
{
}

GenlistManagerCallbacks::~GenlistManagerCallbacks()
{
}

void GenlistManagerCallbacks::_genlist_edge_top(void *data, Evas_Object* /*obj*/,
        void* /*event_info*/)
{
    auto manager = static_cast<GenlistManager*>(data);
    manager->setLastEdgeTop(false);
    // spaces added for 'slide in' effect are not longer needed
    manager->removeSpaces();
}

void GenlistManagerCallbacks::_genlist_edge_bottom(void *data, Evas_Object* /*obj*/,
        void* /*event_info*/)
{
    auto manager = static_cast<GenlistManager*>(data);
    manager->setLastEdgeTop(true);
    if (manager->isWidgetHidden()) {
        manager->clearWidget();
        evas_object_hide(manager->getWidget());
    }
}

void GenlistManagerCallbacks::_genlist_mouse_in(void* data, Evas* /*e*/,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    auto manager = static_cast<GenlistManager*>(data);
    manager->onMouseFocusChange(true);
}
void GenlistManagerCallbacks::_genlist_mouse_out(void* data, Evas* /*e*/,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    auto manager = static_cast<GenlistManager*>(data);
    manager->onMouseFocusChange(false);
}

void GenlistManagerCallbacks::_genlist_focused(void* /*data*/, Evas_Object* /*obj*/,
        void* /*event_info*/)
{
    if(genlistManager)
    {
        genlistManager->signalWidgetFocused();
    }
}

void GenlistManagerCallbacks::_genlist_unfocused(void* /*data*/, Evas_Object* /*obj*/,
        void* /*event_info*/)
{
    if(genlistManager)
    {
        genlistManager->signalWidgetUnfocused();
    }
}

void GenlistManagerCallbacks::_item_selected(void* data, Evas_Object* /*obj*/,
        void* /*event_info*/)
{
    const UrlPair* const item = reinterpret_cast<UrlPair*>(data);
    if (item) {
        if(genlistManager)
        {
            genlistManager->signalItemSelected(item->urlOriginal);
            genlistManager->hideWidgetPretty();
        }
    }
}

} /* namespace base_ui */
} /* namespace tizen_browser */
