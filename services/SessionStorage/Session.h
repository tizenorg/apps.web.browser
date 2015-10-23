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

#ifndef SESSION_H
#define SESSION_H

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <string>


namespace tizen_browser
{

namespace Session
{

class SqlStorage;

/**
 * Stores sesion data in storage.
 *
 * Only position on list and url are stored.
 *
 * If Session is invalid then getter functions return some random data,
 * and setter functions do nothing, so can be safely called,
 * no error nor exception will be returned.
 * \see Session::isValid()
 */
class Session
{

private:

    /**
     * This constructor is used by SqlStorage to create valid session.
     */
    Session(unsigned int sessionId
           ,const boost::posix_time::ptime& lastModificationTime
           ,const std::string& sessionName
           );
public:
    /**
     * This constructor to create invalid Session object,
     * such object cannot be used to store session information.
     * All methods calls do nothing.
     */
    Session();
    ~Session();
    Session(Session& ) = delete;
    Session& operator=(Session& )= delete;
    Session(Session&&);
    Session& operator=(Session&& );
    unsigned int sessionId() const;

    boost::posix_time::ptime lastModificationTime() const;
    void setLastModificationTime(const boost::posix_time::ptime& modificationTime = boost::posix_time::second_clock::local_time());

    const std::string& sessionName() const;
    void setSessionName(const std::string& newSessionName);
    void setTabTitle(const std::string& tabId, const std::string& title);
    std::string getUrlTitle(const std::string& url);

    /**
     * returns read only list of items.
     */
    const std::map<std::string, std::pair<std::string,std::string>>& items() const;

    /**
     * Adds or updates selected item and save changes in storage.
     *
     * If there is not item with given tabId it will be added to session.
     *
     */
    void updateItem(const std::string& tabId, const std::string& url, const std::string& title);

    /**
     * Removes item from session and save changes in storage.
     */
    void removeItem(const std::string& tabId);

    bool isValid() const;

private:
    unsigned int m_sessionId;
    boost::posix_time::ptime m_lastModificationTime;
    std::string m_sessinName;

    std::map<std::string, std::pair<std::string,std::string>> m_items;
    bool m_isValid;

    friend class SqlStorage;
};

typedef std::vector<Session> SessionsVector;

}//end namespace Session
}//end namespace tizen_browser

#endif // SESSION_H
