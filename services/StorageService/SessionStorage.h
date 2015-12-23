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

#ifndef SQLSTORAGE_H
#define SQLSTORAGE_H
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>

#include "SQLDatabase.h"
#include "Session.h"
#include "BookmarkFolder.h"
#include <memory>

namespace tizen_browser {
namespace storage {


class  SessionStorage
{
    /**
     * Singleton, protect against being created in wrong place.
     */
    SessionStorage();

    /**
     * Initialise SessionStorage.
     *
     * Checks if all needed tables are created and if not creates them.
     *
     * Returns true if there is no error.
     */
    bool init();
public:
    ~SessionStorage();
    static SessionStorage& getInstance();
    Session createSession(const std::string& name = "");
    /**
     * Return newes session in storage.
     */
    Session getLastSession();
    ///\todo implement getSession(name); if needed.
    //Session getSession(const std::string& name);
    SessionsVector getAllSessions();

    /**
     * Get title of a tab.
     */
    std::string getUrlTitle(const std::string& url);

    /**
     * Store/update all items in storage
     */
    void updateSession(Session& session);

    /**
     * Update ony one item form session.
     */
    void updateSession(Session& session, const std::string& itemId);

    /**
     * Updates session last update time.
     */
    void updateSessionTimeStamp(Session& session, boost::posix_time::ptime accessTime = boost::posix_time::second_clock::local_time());

    /**
     * Change session name.
     */
    void updateSessionName(Session& session, const std::string& newName);

    /**
     * Deletes item form session, and stores changes.
     */
    void removeItem(Session& sessionToDelFrom, const std::string& itemId);

    /**
     * \brief Delete session form storage.
     */
     void deleteSession(Session& session);

     /**
      * Delete all sessions in storage.
      */
     void deleteAllSessions();
private:
    bool initSessionDatabase();
    void updateSessionTimeStamp(Session& session,
                                const boost::posix_time::ptime& accessTime,
                                std::shared_ptr<storage::SQLDatabase> connection);
    void clearSession(const Session& session, std::shared_ptr<storage::SQLDatabase> connection);
    void updateSession(Session& session,
                       const std::string& itemId,
                       std::shared_ptr<storage::SQLDatabase> connection);
    void readSession(Session& session, std::shared_ptr<storage::SQLDatabase> connection);
    bool m_isInitialized;

    std::string m_dbString;

};

}//end namespace storage
}//end namespace tizen_browser

#endif // SQLSTORAGE_H
