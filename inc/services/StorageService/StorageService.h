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

#ifndef STORAGESERVICE_H
#define STORAGESERVICE_H

#include "core/ServiceManager/ServiceFactory.h"
#include "core/ServiceManager/service_macros.h"
#include "services/StorageService/SettingsStorage.h"
#include "services/StorageService/SessionStorage.h"
#include "services/StorageService/FoldersStorage.h"

#define DOMAIN_STORAGE_SERVICE "org.tizen.browser.storageservice"

namespace tizen_browser {
namespace services {

class BROWSER_EXPORT StorageService : public tizen_browser::core::AbstractService
{
public:
    StorageService();
    virtual ~StorageService();
    virtual std::string getName() = 0;

    storage::SettingsStorage& getSettingsStorage() { return m_settingsStorage; }
    storage::SessionStorage& getSessionStorage() { return storage::SessionStorage::getInstance(); }
    storage::FoldersStorage& getFoldersStorage() { return m_foldersStorage; }

private:
    storage::SettingsStorage m_settingsStorage;
    storage::FoldersStorage m_foldersStorage;
};



}
}

#endif // STORAGESERVICE_H
