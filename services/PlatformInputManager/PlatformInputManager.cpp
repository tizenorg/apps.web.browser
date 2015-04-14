
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

#include <X11/Xlib.h>
#include <utilX.h>
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

PlatformInputManager::PlatformInputManager() :
    m_mouseMoveTimer(NULL),
    m_moveMousePointer(false),
    m_lastPressedKey(OTHER_KEY),
    m_pointerModeEnabled(true)
{

}

void PlatformInputManager::init(Evas_Object* mainWindow)
{
    M_ASSERT(mainWindow);
    ecore_event_filter_add(NULL, __filter, NULL, this);
    m_xWindow = elm_win_xwindow_get(mainWindow);

    // This snippet is needed to show mouse pointer all the time, because by default it is hidden after few seconds
    m_atomDevicemgrInputWindow = ecore_x_atom_get(E_PROP_DEVICEMGR_INPUTWIN);
    m_atomAlwaysCursorOn = ecore_x_atom_get(E_PROP_NOT_CURSOR_HIDE);
    if(!ecore_x_window_prop_window_get(ecore_x_window_root_first_get(), m_atomDevicemgrInputWindow, &m_devicemgr_win, 1))
        BROWSER_LOGD("Failed to get device manager input window!");
    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, __handler_FOCUS_IN, this);
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

        bool wasArrow = false;
        /**
         * In pointer mode arrow key events cause mouse pointer to move.
         * For that each arrow key is handled by us.
         * Pointer movement is realized in timer to achieve smooth animation of pointer's move.
         */
        if(self->m_pointerModeEnabled) {
            wasArrow = true;
            if (!keyName.compare("KEY_LEFT")) {
                self->m_currentMouseMovementParams.xMod = -1;
                self->m_currentMouseMovementParams.yMod = 0;
            }
            else if (!keyName.compare("KEY_RIGHT")) {
                self->m_currentMouseMovementParams.xMod = 1;
                self->m_currentMouseMovementParams.yMod = 0;
            }
            else if (!keyName.compare("KEY_UP")) {
                self->m_currentMouseMovementParams.xMod = 0;
                self->m_currentMouseMovementParams.yMod = -1;
            }
            else if (!keyName.compare("KEY_DOWN")) {
                self->m_currentMouseMovementParams.xMod = 0;
                self->m_currentMouseMovementParams.yMod = 1;
            }
            else
                wasArrow = false;

            /**
            * If last pressed key was arrow we would like to convert Return key to mouse click
            * assuming that user was moving cursor and now want to "click" chosen element.
            * In other cases Return key is handled normally allowing for example to accecpt typed url.
            */
            if(!keyName.compare("KEY_ENTER")) {
                if(self->m_lastPressedKey == ARROW) {
                    self->m_lastPressedKey = RETURN;
                    self->mouseButtonManipulate(Button1, ButtonPress);
                    return EINA_FALSE;
                }
            }

            if(wasArrow) {
                self->m_lastPressedKey = ARROW;
                if(!(self->m_mouseMoveTimer)) {
                    self->m_currentMouseMovementParams.moveMousePointer = true;
                    self->m_currentMouseMovementParams.counter = 0;
                    self->m_currentMouseMovementParams.speed = 1;
                    self->m_mouseMoveTimer = ecore_timer_add(MOUSE_POINTER_MOVE_DELAY, &PlatformInputManager::mouseMove, self);
                }
                return EINA_FALSE;
            }
        }

        self->m_lastPressedKey= OTHER_KEY;

        /**
         * Because MENU button launches org.tizen.menu
         * we use blue 'D' button on remote control or F4 on keyboard as substitution of MENU button
         */
        if(!keyName.compare("KEY_MENU") || !keyName.compare("KEY_BLUE")) {
            self->menuPressed();
            return EINA_FALSE;
        }

        if(!keyName.compare("KEY_CHANNELUP")){
            /**
             * Converting ChannelUp Button on remote control and Page Up key on keyboard to mouse's wheel up move.
             * This is used to achieve same effect of scrolling web page using mouse, remote control and keyboard.
             */
            self->mouseButtonManipulate(Button4, ButtonPress); // Simulate mouse wheel up movement
            self->mouseButtonManipulate(Button4, ButtonRelease);
            return EINA_FALSE;
        }

        if(!keyName.compare("KEY_CHANNELDOWN")){
            /**
             * Same as above ChannelDown and Page Down are replaced by mouse's wheel down movement.
             */
            self->mouseButtonManipulate(Button5, ButtonPress); // Simulate mouse wheel down movement
            self->mouseButtonManipulate(Button5, ButtonRelease);
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
    } else if(type == ECORE_EVENT_KEY_UP) {
        M_ASSERT(event);
        Ecore_Event_Key *ev = static_cast<Ecore_Event_Key *>(event);

        if(!ev->keyname)
            return EINA_TRUE;

        BROWSER_LOGD("Released key: %s", ev->keyname);
        const std::string keyName = ev->keyname;

        /**
         * When arrow key is released thread realizing pointer movement is stopped.
         */
        if(self->m_pointerModeEnabled && (!keyName.compare("KEY_LEFT")||!keyName.compare("KEY_UP")
        || !keyName.compare("KEY_RIGHT")||!keyName.compare("KEY_DOWN"))) {
            if(self->m_mouseMoveTimer) {
                self->m_currentMouseMovementParams.moveMousePointer = false;
                ecore_timer_del(self->m_mouseMoveTimer);
                self->m_mouseMoveTimer = NULL;
            }
            return EINA_FALSE;
        }
        /**
         * If Return key is being released we send event that mouse button was released (in pointer mode).
         */
        if(self->m_lastPressedKey == RETURN && self->m_pointerModeEnabled && !keyName.compare("KEY_ENTER")) {
            self->m_lastPressedKey = OTHER_KEY;
            self->mouseButtonManipulate(Button1, ButtonRelease);
            return EINA_FALSE;
        }
    }
    return EINA_TRUE;
}

Eina_Bool PlatformInputManager::mouseMove(void *data)
{
    int x, y;
    PlatformInputManager *self = static_cast<PlatformInputManager*>(data);

    if(self->m_currentMouseMovementParams.moveMousePointer) {
        ecore_x_pointer_xy_get(self->m_xWindow, &x, &y);
        x += self->m_currentMouseMovementParams.xMod * self->m_currentMouseMovementParams.speed;
        y += self->m_currentMouseMovementParams.yMod * self->m_currentMouseMovementParams.speed;
        ecore_x_pointer_warp(self->m_xWindow, x, y);
        ecore_x_flush();

        ++(self->m_currentMouseMovementParams.counter);
        if(self->m_currentMouseMovementParams.counter == MOUSE_POINTER_STEPS) {
            self->m_currentMouseMovementParams.counter = 0;
            ++(self->m_currentMouseMovementParams.speed);
        }
        return EINA_TRUE;
    }
    return EINA_FALSE;
}

Eina_Bool PlatformInputManager::__handler_FOCUS_IN(void *data, int /*type*/, void */*event*/)
{
    PlatformInputManager * self = reinterpret_cast<PlatformInputManager *>(data);
    ecore_x_window_prop_window_set(self->m_devicemgr_win, self->m_atomAlwaysCursorOn, &(self->m_xWindow), 1);
    return EINA_TRUE;
}

void PlatformInputManager::mouseButtonManipulate(int button, int event_type)
{
    XEvent event;

    Display *display = static_cast<Display*>(ecore_x_display_get());

    memset(&event, 0x00, sizeof(event));

    event.type = event_type;

    if (event_type == ButtonRelease)
        event.xbutton.state = 0x100;

    event.xbutton.button = button;
    event.xbutton.same_screen = True;

    XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);

    event.xbutton.subwindow = event.xbutton.window;

    while (event.xbutton.subwindow) {
        event.xbutton.window = event.xbutton.subwindow;

        XQueryPointer(display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    }

    XSendEvent(display, PointerWindow, True, 0xfff, &event);

    XFlush(display);
}

void PlatformInputManager::setPointerModeEnabled(bool enabled)
{
    m_pointerModeEnabled = enabled;
}

bool PlatformInputManager::getPointerModeEnabled() const
{
    return m_pointerModeEnabled;
}

}
}
