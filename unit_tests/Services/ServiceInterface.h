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

#ifndef SERVICEINTERFACE_H
#define SERVICEINTERFACE_H

#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

class ServiceInterface : public  tizen_browser::core::AbstractService
{
public:
    ServiceInterface();
    virtual ~ServiceInterface();
    virtual void run() = 0;
    virtual void stop() = 0 ;
    virtual bool isRunning() = 0;
};

#endif // SERVICEINTERFACE_H
