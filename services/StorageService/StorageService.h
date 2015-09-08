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

#ifndef __STORAGESERVICE_H
#define __STORAGESERVICE_H

#include <memory>

#include "ServiceFactory.h"
#include "service_macros.h"
#include "BrowserImage.h"
#include "Config.h"
#include "BookmarksStorage.h"
#include "BookmarksSQLite.h"

#define DOMAIN_STORAGE_SERVICE "org.tizen.browser.storageservice"

namespace tizen_browser
{
namespace services
{

class BROWSER_EXPORT StorageService: public tizen_browser::core::AbstractService
{
public:
    StorageService();
    virtual ~StorageService();
    virtual std::string getName();

    /**
     * @throws StorageException on error
     */
    int getSettingsInt(const std::string & key, const int defaultValue) const;

    /**
     * @throws StorageException on error
     */
    double getSettingsDouble(const std::string & key, const double defaultValue) const;

    /**
     * @throws StorageException on error
     */
    const std::string getSettingsText(const std::string & key, const std::string & defaultValue) const;

    /**
     * @throws StorageException on error
     */
    void setSettingsInt(const std::string & key, int value) const;

    /**
     * @throws StorageException on error
     */
    void setSettingsDouble(const std::string & key, double value) const;

    /**
     * @throws StorageException on error
     */
    void setSettingsString(const std::string & key, std::string value) const;

    /**
     * @throws StorageException on error
     */
    static void checkAndCreateTable(const std::string & db_str, const std::string & tablename, const std::string & ddl);

    /**
     * @throws StorageException on error
     */
    static void checkAndCreateTable(storage::SQLTransactionScope * transactionScope, const std::string & tablename, const std::string & ddl);

    void init(bool testmode = false);

    /**
     * @brief Returns shared_ptr to BookmarksStorage;
     *
     * @return std::shared_ptr< tizen_browser::services::BookmarksStorage >
     */
    std::shared_ptr<BookmarksStorage> getBookmarkStorage();

private:
    /**
     * @throws StorageExceptionInitialization on error
     */
    void initDatabaseSettings(const std::string & db_str);

    /**
     * @throws StorageExceptionInitialization on error
     */
    void setSettingsValue(const std::string & key, storage::FieldPtr field) const;

    bool m_dbSettingsInitialised;
    config::DefaultConfig config;
    std::string DB_SETTINGS;

//    std::shared_ptr<RssStorage> m_rssStorage;
    std::shared_ptr<BookmarksSQLite> m_bookmarks;

    bool m_isInitialized;
};

}
}

#endif
