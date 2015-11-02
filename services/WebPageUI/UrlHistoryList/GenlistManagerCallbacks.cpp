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
#include "GenlistManagerCallbacks.h"
#include "GenlistItemsManager.h"
#include <Ecore_Input.h>

namespace tizen_browser {
namespace base_ui {

GenlistManager* GenlistManagerCallbacks::genlistManager = nullptr;

GenlistManagerCallbacks::GenlistManagerCallbacks()
{
}

GenlistManagerCallbacks::~GenlistManagerCallbacks()
{
}

void GenlistManagerCallbacks::_genlist_edge_top(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    auto manager = static_cast<GenlistManager*>(data);
    // spaces added for 'slide in' effect are not longer needed
    manager->removeSpaces();
}

void GenlistManagerCallbacks::_genlist_edge_bottom(void* data,
        Evas_Object* /*obj*/, void* /*event_info*/)
{
    auto manager = static_cast<GenlistManager*>(data);
    if (manager->getWidgetPreviouslyHidden()) {
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

void GenlistManagerCallbacks::_item_selected(void* data, Evas_Object* /*obj*/,
        void* /*event_info*/)
{
    const UrlPair* const item = reinterpret_cast<UrlPair*>(data);
    if (item) {
        if (genlistManager) {
            genlistManager->signalItemSelected(item->urlOriginal);
            genlistManager->hideWidgetPretty();
        }
    }
}

Eina_Bool GenlistManagerCallbacks::_object_event(void* /*data*/,
        Evas_Object* /*obj*/, Evas_Object* /*src*/, Evas_Callback_Type /*type*/,
        void* event_info)
{
    if (!genlistManager)
        return EINA_FALSE;

    Ecore_Event_Key *ev = static_cast<Ecore_Event_Key *>(event_info);
    const std::string keyName = ev->keyname;
    if (keyName.compare("Down") == 0 || keyName.compare("Up") == 0) {
        genlistManager->removeSpaces();
        genlistManager->setWidgetPreviouslyHidden(false);
        GenlistItemsManagerPtr itemsManager = genlistManager->getItemsManager();
        if (!itemsManager->getItem(GenlistItemType::ITEM_CURRENT)) {
            itemsManager->assignItem(GenlistItemType::ITEM_CURRENT,
                    GenlistItemType::ITEM_FIRST);
        }
        if (keyName.compare("Down") == 0) {
            itemsManager->shiftItemDown(GenlistItemType::ITEM_CURRENT);
        } else {
            itemsManager->shiftItemUp(GenlistItemType::ITEM_CURRENT);
        }
        genlistManager->signalItemFocusChange();
    }
    return EINA_FALSE;
}

} /* namespace base_ui */
} /* namespace tizen_browser */
