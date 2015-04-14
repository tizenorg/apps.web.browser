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

#include "Session.h"
#include "SqlStorage.h"

namespace tizen_browser
{

namespace Session
{


Session::Session()
    :m_sessionId(0)
    ,m_lastModificationTime()
    ,m_sessinName("")
    ,m_isValid(false)
{
}

Session::Session(unsigned int sessionId
                        , const boost::posix_time::ptime& lastModificationTime
                        , const std::string& sessionName)
    :m_sessionId(sessionId)
    ,m_lastModificationTime(lastModificationTime)
    ,m_sessinName(sessionName)
    ,m_isValid(true)
{
}

Session::~Session()
{
}

Session::Session(Session&& source)
 :m_sessionId(std::move(source.m_sessionId))
 , m_lastModificationTime(std::move(source.m_lastModificationTime))
 , m_sessinName(std::move(source.m_sessinName))
 , m_isValid(std::move(source.m_isValid))
{
}

Session & Session::operator=(Session&& other)
{
    if (this != &other) {
	m_sessionId = std::move(other.m_sessionId);
        m_lastModificationTime = std::move(other.m_lastModificationTime);
        m_sessinName = std::move(other.m_sessinName);
        m_isValid = std::move(other.m_isValid);
    }
    return *this;
}

unsigned int Session::sessionId() const
{
    return m_sessionId;
}

boost::posix_time::ptime Session::lastModificationTime() const
{
    return m_lastModificationTime;
}

void Session::setLastModificationTime(const boost::posix_time::ptime& modificationTime)
{
    if( m_isValid & ( m_lastModificationTime != modificationTime )){
        SqlStorage* storage = SqlStorage::getInstance();
        if(storage){
            storage->updateSessionTimeStamp(*this, modificationTime);
        }
    }
}


const std::string& Session::sessionName() const
{
    return m_sessinName;
}


void Session::setSessionName(const std::string& newSessionName)
{
    if(m_isValid && ( m_sessinName != newSessionName )){
        m_sessinName = newSessionName;
        SqlStorage* instance = SqlStorage::getInstance();
        if(instance){
            instance->updateSessionName(*this, newSessionName);
        }
    }
}


const std::map< std::string, std::string >& Session::items() const
{
    return m_items;
}

bool Session::isValid() const
{
    return m_isValid;
}

void Session::updateItem(const std::string& tabId, const std::string& url)
{
    if(m_isValid){
        m_items[tabId] = url;
        SqlStorage* const storage = SqlStorage::getInstance();
        if(storage){
            storage->updateSession(*this);
        }
    }
}

void Session::removeItem(const std::string& tabId)
{
    if(m_isValid){
        m_items.erase(tabId);
        SqlStorage* const storage = SqlStorage::getInstance();
        if(storage){
            storage->removeItem(*this, tabId);
        }
    }
}

}//end namespace Session
}//end namespace tizen_browser
