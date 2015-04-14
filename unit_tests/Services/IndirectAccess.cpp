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

#include <stdexcept>
#include "BrowserLogger.h"

// for tests...
#include "Lifecycle.h"
#include "ServiceManager.h"
#include "ServiceInterface.h"


int main() try
{
    BEGIN()
#ifndef NDEBUG
    //Initialization of logger module
    tizen_browser::logger::Logger::getInstance().init();
    tizen_browser::logger::Logger::getInstance().setLogTag("IndirectAccess.cpp");
#endif
	BROWSER_LOGD("BROWSER IS SAYING HELLO");
    tizen_browser::core::ServiceManager *svm = &tizen_browser::core::ServiceManager::getInstance();



    std::shared_ptr<ServiceInterface> service1
            = std::dynamic_pointer_cast
                <
                    ServiceInterface,
                    tizen_browser::core::AbstractService
                >
                (svm->getService("org.tizen.browser.TestServiceOne"));



    std::shared_ptr<ServiceInterface> service2
            = std::dynamic_pointer_cast
                <
                    ServiceInterface,
                    tizen_browser::core::AbstractService
                >
                (svm->getService("org.tizen.browser.TestServiceTwo"));

    if(service1){
        BROWSER_LOGD(service1->getName().c_str());
        BROWSER_LOGD("%p", service1.get() );
        service1->isRunning();

        service1->isRunning();
        service1->run();
        service1->isRunning();
        service1->stop();
        service1->isRunning();
    }
    if(service2){
        BROWSER_LOGD(service2->getName().c_str() );
        BROWSER_LOGD("%p" , service2.get() );
        service2->isRunning();

        service2->isRunning();
        service2->run();
        service2->isRunning();
        service2->stop();
        service2->isRunning();
    }

    END()

} catch (std::exception & e)
{
    std::cerr << "UNHANDLED EXCEPTION " << e.what() << std::endl ;
} catch (...)
{
    std::cerr << "UNHANDLED EXCEPTION" << std::endl;
}

