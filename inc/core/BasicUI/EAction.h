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

#ifndef EACTION_H
#define EACTION_H

#include <Evas.h>

#include "core/BasicUI/Action.h"

namespace tizen_browser
{
namespace base_ui
{

class EAction
{
public:
    EAction(sharedAction action);
    sharedAction getAction();
    static void callbackFunction(void *data, Evas_Object *obj, void *event_info);
private:
    sharedAction m_action;
};

}
}

#endif // EACTION_H
