
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

#include "PlatformInputManager.h"

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Input.h>
#include <unistd.h>

#include "BrowserAssert.h"
#include "BrowserLogger.h"

#define E_PROP_DEVICEMGR_INPUTWIN "DeviceMgr Input Window"
#define E_PROP_NOT_CURSOR_HIDE "E_NOT_CURSOR_HIDE"

#define MOUSE_POINTER_MOVE_DELAY 0.015f
#define MOUSE_POINTER_STEPS 10

namespace tizen_browser
{
namespace services
{

EXPORT_SERVICE(PlatformInputManager, "org.tizen.browser.platforminputmanager")

PlatformInputManager::PlatformInputManager()
{

}

void PlatformInputManager::init(Evas_Object* mainWindow)
{
    M_ASSERT(mainWindow);
    //Suppress compilation warning
    (void) mainWindow;
    ecore_event_filter_add(NULL, __filter, NULL, this);
}

Eina_Bool PlatformInputManager::__filter(void *data, void */*loop_data*/, int type, void *event)
{
    PlatformInputManager *self = static_cast<PlatformInputManager*>(data);

    if (type == ECORE_EVENT_KEY_DOWN) {
        M_ASSERT(event);
        Ecore_Event_Key *ev = static_cast<Ecore_Event_Key *>(event);

        if(!ev->keyname)
            return EINA_TRUE;

        BROWSER_LOGD("Pressed key: %s", ev->keyname);
        const std::string keyName = ev->keyname;

        /**
         * Because MENU button launches org.tizen.menu
         * we use blue 'D' button on remote control or F4 on keyboard as substitution of MENU button
         */
        if(!keyName.compare("KEY_MENU") || !keyName.compare("KEY_BLUE")) {
            self->menuPressed();
            return EINA_FALSE;
        }

        if(!keyName.compare("KEY_RETURN"))
            self->returnPressed();
        else if(!keyName.compare("KEY_LEFT"))
            self->leftPressed();
        else if(!keyName.compare("KEY_RIGHT"))
            self->rightPressed();
        else if(!keyName.compare("KEY_ENTER"))
            self->enterPressed();
#if !PROFILE_MOBILE
#ifdef HW_BACK_KEY
        else if(!keyName.compare("F11") || !keyName.compare("XF86Back"))
            self->backPressed();
#endif
#endif
        else if(!keyName.compare("Escape"))
            self->escapePressed();
        else if(!keyName.compare("XF86Red"))    // F4 - Red
            self->redPressed();
        else if(!keyName.compare("XF86Green"))  // F5 - Green
            self->greenPressed();
        else if(!keyName.compare("XF86Yellow")) // F6 - Yellow
            self->yellowPressed();
        else if(!keyName.compare("XF86Blue"))   // F7 - Blue
            self->bluePressed();
#if PROFILE_MOBILE
        else if(!keyName.compare("XF86Menu"))
            self->menuButtonPressed();
#endif

    } else if(type == ECORE_EVENT_KEY_UP) {
        M_ASSERT(event);
        Ecore_Event_Key *ev = static_cast<Ecore_Event_Key *>(event);

        if(!ev->keyname)
            return EINA_TRUE;

        BROWSER_LOGD("Released key: %s", ev->keyname);
    } else if (type == ECORE_EVENT_MOUSE_BUTTON_DOWN) {
        self->mouseClicked();
    }
    return EINA_TRUE;
}

}
}
