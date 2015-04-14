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

#ifndef PLATFORMINPUTMANAGER_H
#define PLATFORMINPUTMANAGER_H

#include <boost/signals2/signal.hpp>
#include <boost/thread/thread.hpp>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Eina.h>
#include <Elementary.h>

#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

namespace tizen_browser
{
namespace services
{

class BROWSER_EXPORT PlatformInputManager : public tizen_browser::core::AbstractService
{
public:
    /**
     * @brief Default constructor with variable initialization.
     */
    PlatformInputManager();

    /**
     * @brief Initialization of the object, adding event filter, setting cursor to be always visible.
     */
    void init(Evas_Object *mainWindow);

    /**
     * Signals emitted after certain button on keyboard/remote controller press.
     */
    boost::signals2::signal<void ()> menuPressed;
    boost::signals2::signal<void ()> returnPressed;
    boost::signals2::signal<void ()> enterPressed;
    boost::signals2::signal<void ()> leftPressed;
    boost::signals2::signal<void ()> rightPressed;

    /**
     * @brief Returns current service's name.
     */
    virtual std::string getName();

    /**
     * @brief Enables/disables pointer mode.
     */
    void setPointerModeEnabled(bool enabled);

    /**
     * @brief Returns if poiner mode is enabled.
     */
    bool getPointerModeEnabled() const;

private:
    enum LastPressedKey {ARROW, RETURN, OTHER_KEY};

    /**
     * @brief Struct holding parameters of mouse movement.
     * It is used in pointer mode to simulate mouse move after pressing arrows.
     */
    struct MouseMovementParams {
        bool moveMousePointer;
        int xMod, yMod;
        int counter, speed;
    };

    /**
     * @brief It process every input event and handles it if necessary.
     */
    static Eina_Bool __filter(void */*data*/, void */*loop_data*/, int /*type*/, void */*event*/);

    /**
     * @brief This callback prevent mouse cursor from disappearing.
     */
    static Eina_Bool __handler_FOCUS_IN(void *data, int type, void *event);

    /**
     * @brief Moves mouse cursor on screen according to passed params. Called in timer.
     */
    static Eina_Bool mouseMove(void *data);

    /**
     * @brief Allows to simulate mouse button presses and releases.
     * @param button Button identifier from Xlib.h
     * @param event_type Event identifier from Xlib.h
     */
    void mouseButtonManipulate(int button, int event_type);

    Ecore_X_Window m_xWindow;
    Ecore_X_Window m_devicemgr_win;
    Ecore_X_Atom m_atomDevicemgrInputWindow;
    Ecore_X_Atom m_atomAlwaysCursorOn;
    MouseMovementParams m_currentMouseMovementParams;
    Ecore_Timer *m_mouseMoveTimer;
    bool m_moveMousePointer;
    LastPressedKey m_lastPressedKey;
    bool m_pointerModeEnabled;
};

}
}

#endif // PLATFORMINPUTMANAGER_H
