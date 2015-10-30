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
 *
 *
 */

#ifndef SESSIONSTORAGE_H
#define SESSIONSTORAGE_H
#include <string>

#include "ServiceManager/ServiceFactory.h"
#include "ServiceManager/service_macros.h"
#include "SqlStorage.h"


#define DOMAIN_SESSION_STORAE_SERVICE "org.tizen.browser.sessionStorageService"

namespace tizen_browser
{

namespace services
{


class BROWSER_EXPORT SessionStorage: public tizen_browser::core::AbstractService
{
public:
    SessionStorage();
    virtual std::string getName();
    virtual ~SessionStorage();

    tizen_browser::Session::SqlStorage* getStorage();

};

}//end namespace Session
}//end namespace tizen_browser

#endif // SESSIONSTORAGE_H
