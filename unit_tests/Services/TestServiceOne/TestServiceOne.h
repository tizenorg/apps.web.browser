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

#ifndef TESTMASTER_H
#define TESTMASTER_H

#include <string>

#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "../ServiceInterface.h"
#include "Lifecycle.h"


class BROWSER_EXPORT TestServiceOne : public ServiceInterface , ShowLifeCycle<TestServiceOne>
{
public:
    TestServiceOne();
    //TestMaster(const AbstractService::AgrsMap& args);
    virtual ~TestServiceOne();
    void run();
    void stop();
    bool isRunning();
    virtual std::string getName();
private:
    bool m_isRunning;
};


EXPORT_SERVICE(TestServiceOne, "org.tizen.browser.TestServiceOne")


#endif // TESTMASTER_H
