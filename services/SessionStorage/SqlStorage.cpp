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

#include <boost/format.hpp>
#include <string>
#include "SqlStorage.h"
#include <Field.h>
#include <SQLDatabase.h>
#include <SQLDatabaseImpl.h>
#include <DriverManager.h>
#include <StorageException.h>
#include <StorageExceptionInitialization.h>

namespace{
    const std::string TABLE_SESSION = "SESSION_TABLE";
    const std::string COL_SESSION_ID = "id";
    const std::string COL_SESSION_NAME = "name";
    const std::string COL_SESSION_DATE = "modification_date";
    const std::string DDL_CREATE_TABLE_SESSION
                            = " CREATE TABLE " + TABLE_SESSION
                            + " ( " + COL_SESSION_ID + " INTEGER PRIMARY KEY, "
                            + "   " + COL_SESSION_DATE + " DATETIME NOT NULL, "
                            + "   " + COL_SESSION_NAME + " TEXT "
                            + " ); ";
    const std::string INDEX_SESSION_DATE = "SESSION_MODIFICATION_INDEX";
    const std::string DDL_CREATE_INDEX_SESSION_DATE
                            = " CREATE INDEX IF NOT EXISTS " + INDEX_SESSION_DATE
                            + "    ON " + TABLE_SESSION + " ( " + COL_SESSION_DATE + " ); "
                            ;

    const std::string TABLE_URL = "URL_TABLE";
    const std::string COL_URL_SESION_ID = "session_id";
    const std::string COL_URL_TABID = "position"; // position in list.
    const std::string COL_URL_URL = "url";
    const std::string COL_URL_TITLE = "title";
    const std::string CONSTRAINT_URL_PK = TABLE_URL + "_PK";
    const std::string CONSTRAINT_URL_SESSION_FK = TABLE_URL + "_" + COL_URL_SESION_ID + "_FK";
    const std::string DDL_CREATE_TABLE_URL
                            = " CREATE TABLE " + TABLE_URL
                            + " ( " + COL_URL_SESION_ID + " INTEGER, "
                            + "   " + COL_URL_TABID + " TEXT, "
                            + "   " + COL_URL_URL + " TEXT, "
                            + "   " + COL_URL_TITLE + " TEXT, "
                            + " CONSTRAINT " + CONSTRAINT_URL_PK
                            + "     PRIMARY KEY ( " + COL_URL_SESION_ID + " , " + COL_URL_TABID + " ) "
                            + "     ON CONFLICT REPLACE, "
                            + " CONSTRAINT " + CONSTRAINT_URL_SESSION_FK
                            + "     FOREIGN KEY ( " + COL_URL_SESION_ID + " ) "
                            + "     REFERENCES " + TABLE_SESSION + " ( " + COL_SESSION_ID + " ) "
                            + "     ON DELETE CASCADE "
                            + " ); ";

    const std::string TABLE_FOLDER = "FOLDER_TABLE";
    const std::string COL_FOLDER_ID = "folder_id";
    const std::string COL_FOLDER_NAME = "name";
    const std::string CONSTRAINT_TABLE_PK = TABLE_FOLDER + "_PK";
    const std::string DDL_CREATE_TABLE_FOLDER
                            = " CREATE TABLE " + TABLE_FOLDER
                            + " ( " + COL_FOLDER_ID + " INTEGER, "
                            + "   " + COL_FOLDER_NAME + " TEXT,"
                            + " CONSTRAINT " + CONSTRAINT_TABLE_PK
                            + "      PRIMARY KEY ( " + COL_FOLDER_ID + " ) "
                            + "      ON CONFLICT REPLACE "
                            + " ); ";

}

namespace tizen_browser
{

namespace Session
{

SqlStorage::SqlStorage()
    : m_isInitialized(false)
{

}
SqlStorage::~SqlStorage()
{
}


bool SqlStorage::init()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_isInitialized){
        return true;
    }

    config.load("not used text");
    std::string resourceDbDir(boost::any_cast < std::string > (config.get("resourcedb/dir")));
    std::string sessionDb(boost::any_cast < std::string > (config.get("DB_SESSION")));

    m_dbString = resourceDbDir + sessionDb;
    bool status = initSessionDatabase();
    if( status ) {
        m_isInitialized = true;
        return true;
    }
    return false;
}

SqlStorage* SqlStorage::getInstance()
{
    static SqlStorage instance;
    if( instance.init() ){
        return &instance;
    }
    return 0;
}


bool SqlStorage::initSessionDatabase()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
    try {
        tizen_browser::services::StorageService::checkAndCreateTable(&scope,
                                                                     TABLE_SESSION,
                                                                     DDL_CREATE_TABLE_SESSION);
        tizen_browser::services::StorageService::checkAndCreateTable(&scope
                                                                     ,TABLE_URL
                                                                     ,DDL_CREATE_TABLE_URL);
        tizen_browser::services::StorageService::checkAndCreateTable(&scope,
                                                                     TABLE_FOLDER,
                                                                     DDL_CREATE_TABLE_FOLDER);
        scope.database()->exec(DDL_CREATE_INDEX_SESSION_DATE);
        return true;
    } catch (storage::StorageException &e) {
        BROWSER_LOGE("[ERROR] Session Storage initalization Error code: %d: %s"
                    , __PRETTY_FUNCTION__
                    , __LINE__
                    ,  e.getErrorCode()
                    , e.getMessage());
        return false;
    }
}

Session SqlStorage::createSession(const std::string& name)
{
    boost::format addSessionQueryString("INSERT OR REPLACE INTO %1% ( %2%, %3% ) VALUES ( ? , ? );");
    addSessionQueryString % TABLE_SESSION % COL_SESSION_DATE % COL_SESSION_NAME;
    unsigned int sessionId=0;
    boost::posix_time::ptime currentTime(boost::posix_time::second_clock::local_time());
    std::string sessionName( name.empty()
                             ? boost::posix_time::to_iso_string(currentTime)
                             : name
                           );
    try{
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        storage::SQLQuery addSessionQuery(db->prepare(addSessionQueryString.str()));
        addSessionQuery.bindText(1, boost::posix_time::to_iso_string(currentTime));
        addSessionQuery.bindText(2, sessionName);
        addSessionQuery.exec();
        sessionId = db->lastInsertId();
    }catch (storage::StorageException &e){
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        return Session();
    }
    return Session(sessionId, currentTime, sessionName);
}

Session SqlStorage::getLastSession()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getLastSessionString("SELECT %1%, %2%, %3% FROM %4% ORDER BY %3% DESC LIMIT 1,1;");
    getLastSessionString % COL_SESSION_ID % COL_SESSION_NAME % COL_SESSION_DATE % TABLE_SESSION;
    unsigned int sessionId;
    std::string sessionName;
    boost::posix_time::ptime sessionTime;
    try{
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getLastSessionQuery(connection->prepare(getLastSessionString.str()));

        getLastSessionQuery.exec();

        if(getLastSessionQuery.hasNext()){
            sessionId = getLastSessionQuery.getInt(0);
            sessionName = getLastSessionQuery.getString(1);
            sessionTime = boost::posix_time::from_iso_string(getLastSessionQuery.getString(2));
            Session session(sessionId, sessionTime, sessionName);
            readSession(session, connection);
            return session;
        }
    }catch( storage::StorageException &e){
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return Session();
}

void SqlStorage::updateSession(tizen_browser::Session::Session& session)
{
    if (session.isValid()) {
        boost::posix_time::ptime currentTime(boost::posix_time::second_clock::local_time());
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        clearSession(session, connection);
        for (auto i=session.items().begin(), end=session.items().end(); i != end; i++ ) {
            updateSession(session, i->first, connection);
        }
        updateSessionTimeStamp(session, currentTime, connection);
        session.m_lastModificationTime = currentTime;
    }
}

void SqlStorage::updateSession(tizen_browser::Session::Session& session, const std::string& itemId)
{
    if (session.isValid()) {
        boost::posix_time::ptime currentTime(boost::posix_time::second_clock::local_time());
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        updateSession(session, itemId, connection);
        updateSessionTimeStamp(session, currentTime, connection);
        session.m_lastModificationTime = currentTime;
    }
}

void SqlStorage::updateSession(Session& session,
                               const std::string& itemId,
                               std::shared_ptr<storage::SQLDatabase> connection)
{
    boost::format updateUrlString("INSERT OR REPLACE INTO %1% ( %2% , %3% , %4%, %5% ) VALUES ( ? , ? , ? , ? );");
    updateUrlString % TABLE_URL  % COL_URL_SESION_ID % COL_URL_TABID % COL_URL_URL % COL_URL_TITLE;

    try {
        storage::SQLQuery updateUrlQuery(connection->prepare(updateUrlString.str()));
        updateUrlQuery.bindInt(1, session.sessionId());
        updateUrlQuery.bindText(2, itemId);
        updateUrlQuery.bindText(3, session.items().at(itemId).first);
        updateUrlQuery.bindText(4, session.items().at(itemId).second);
        updateUrlQuery.exec();
    } catch(storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

void SqlStorage::updateSessionTimeStamp(Session& session
                                       ,boost::posix_time::ptime accessTime)
{
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
    std::shared_ptr<storage::SQLDatabase> connection = scope.database();
    updateSessionTimeStamp(session, accessTime, connection);
}

void SqlStorage::updateSessionTimeStamp(
            Session& session
            ,const boost::posix_time::ptime& accessTime
            ,std::shared_ptr< storage::SQLDatabase > connection
            )
{
    try{
        boost::format updateSessionStrig("UPDATE %1% SET %2% = ? WHERE %3% = ?");
        updateSessionStrig % TABLE_SESSION % COL_SESSION_DATE % COL_SESSION_ID;
        storage::SQLQuery updateSessionQuery(connection->prepare(updateSessionStrig.str()));
        updateSessionQuery.bindText(1, boost::posix_time::to_iso_string(accessTime));
        updateSessionQuery.bindInt(2, session.m_sessionId);
        updateSessionQuery.exec();
        session.m_lastModificationTime = accessTime;
    }catch( storage::StorageException &e){
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }

}

void SqlStorage::removeItem(Session& sessionToDelFrom, const std::string& itemId)
{
    if(sessionToDelFrom.isValid()){
        boost::format deleteItemFromSessionString("DELETE FROM %1% WHERE %2% = ? AND %3% = ?;");
        deleteItemFromSessionString % TABLE_URL % COL_URL_SESION_ID % COL_URL_TABID;

        boost::posix_time::ptime currentTime(boost::posix_time::second_clock::local_time());
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();


        try{
            storage::SQLQuery clearSessionQurey(connection->prepare(deleteItemFromSessionString.str()));
            clearSessionQurey.bindInt(1, sessionToDelFrom.sessionId());
            clearSessionQurey.bindText(1, itemId);
            clearSessionQurey.exec();
        }catch (storage::StorageException &e){
            BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        }

        updateSessionTimeStamp(sessionToDelFrom, currentTime, connection);
        sessionToDelFrom.m_lastModificationTime = currentTime;
    }
}

void SqlStorage::clearSession(const Session& session,
                              std::shared_ptr< storage::SQLDatabase > connection)
{
    boost::format clearSessionString("DELETE FROM %1% WHERE %2% = ?;");
    clearSessionString % TABLE_URL % COL_URL_SESION_ID;

    try{
        storage::SQLQuery clearSessionQurey(connection->prepare(clearSessionString.str()));
        clearSessionQurey.bindInt(1, session.sessionId());
        clearSessionQurey.exec();
    }catch (storage::StorageException &e){
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

void SqlStorage::updateSessionName(Session& session, const std::string& newName)
{
    if(session.isValid()){
        boost::format updateSessionNameString("UPDATE %1%  SET %2% = ?  WHERE %3% = ?" );
        updateSessionNameString % TABLE_SESSION % COL_SESSION_NAME % COL_SESSION_ID;

        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();

        try{
            storage::SQLQuery updateSessionNameQuery(connection->prepare(updateSessionNameString.str()));
            updateSessionNameQuery.bindText(1, newName);
            updateSessionNameQuery.bindInt(2, session.sessionId());
            updateSessionNameQuery.exec();
        }catch( storage::StorageException &e){
            BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());

        }
    }
}

std::string SqlStorage::getUrlTitle(const std::string& url)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getTitleString("SELECT %1% FROM %2% WHERE %3% = ?;");
    getTitleString % COL_URL_TITLE % TABLE_URL % COL_URL_URL;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getTitleQuery(connection->prepare(getTitleString.str()));
        getTitleQuery.bindText(1, url);
        getTitleQuery.exec();

        return getTitleQuery.getString(0);

    } catch(storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return std::string();
}

services::SharedBookmarkFolder SqlStorage::getFolder(unsigned int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string name = getFolderName(id);
    services::SharedBookmarkFolder folder;
    if (name != "")
        folder = std::make_shared<services::BookmarkFolder>(id, name, 0);
    return folder;
}

services::SharedBookmarkFolderList SqlStorage::getFolders()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    services::SharedBookmarkFolderList folders;
    int foldersCount = getFoldersCount();
    if (foldersCount != 0) {
        boost::format getFoldersString("SELECT %1%, %2% FROM %3% ;");
        getFoldersString % COL_FOLDER_ID % COL_FOLDER_NAME % TABLE_FOLDER;
        try {
            storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
            std::shared_ptr<storage::SQLDatabase> connection = scope.database();
            storage::SQLQuery getFoldersQuery(connection->prepare(getFoldersString.str()));
            getFoldersQuery.exec();
            for (int i = 0; i < foldersCount; ++i) {
                //todo: bookmark count
                services::SharedBookmarkFolder bookmark = std::make_shared<services::BookmarkFolder>(getFoldersQuery.getInt(0), getFoldersQuery.getString(1), 0);
                folders.push_back(bookmark);
                getFoldersQuery.next();
            }
        } catch (storage::StorageException& e) {
            BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        }
    }
    return folders;
}

unsigned int SqlStorage::getFoldersCount()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getCountString("SELECT COUNT (*) FROM %1% ;");
    getCountString % TABLE_FOLDER;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getCountQuery(connection->prepare(getCountString.str()));
        getCountQuery.exec();
        return getCountQuery.getInt(0);
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

unsigned int SqlStorage::addFolder(const std::string& name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format addFolderQueryString("INSERT OR REPLACE INTO %1% ( %2% ) VALUES ( ? );");
    addFolderQueryString % TABLE_FOLDER % COL_FOLDER_NAME;
    boost::posix_time::ptime currentTime(boost::posix_time::second_clock::local_time());
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> db = scope.database();
        storage::SQLQuery addFolderQuery(db->prepare(addFolderQueryString.str()));
        addFolderQuery.bindText(1, name);
        addFolderQuery.exec();
        return db->lastInsertId();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

void SqlStorage::updateFolderName(unsigned int id, const std::string& newName)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format updateFolderNameString("UPDATE %1%  SET %2% = ? WHERE %3% = ?" );
    updateFolderNameString % TABLE_FOLDER % COL_FOLDER_NAME % COL_FOLDER_ID;
    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
    std::shared_ptr<storage::SQLDatabase> connection = scope.database();
    try {
        storage::SQLQuery updateFolderNameQuery(connection->prepare(updateFolderNameString.str()));
        updateFolderNameQuery.bindText(1, newName);
        updateFolderNameQuery.bindInt(2, id);
        updateFolderNameQuery.exec();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

void SqlStorage::removeFolder(unsigned int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format deleteFolderString("DELETE FROM %1% WHERE %2% = ?;");
    deleteFolderString % TABLE_FOLDER % COL_FOLDER_ID;

    storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
    std::shared_ptr<storage::SQLDatabase> connection = scope.database();
    try {
        storage::SQLQuery deleteFolderQurey(connection->prepare(deleteFolderString.str()));
        deleteFolderQurey.bindInt(1, id);
        deleteFolderQurey.exec();
    } catch (storage::StorageException &e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

bool SqlStorage::ifFolderExists(const std::string& name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getCountString("SELECT COUNT (*) FROM %1% WHERE %2% = ?;");
    getCountString % TABLE_FOLDER % COL_FOLDER_NAME;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getCountQuery(connection->prepare(getCountString.str()));
        getCountQuery.bindText(1, name);
        getCountQuery.exec();
        int number = getCountQuery.getInt(0);
        return number != 0;
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return true;
}

unsigned int SqlStorage::getFolderId(const std::string& name)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getIdString("SELECT %1% FROM %2% WHERE %3% = ?;");
    getIdString % COL_FOLDER_ID % TABLE_FOLDER % COL_FOLDER_NAME;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getIdQuery(connection->prepare(getIdString.str()));
        getIdQuery.bindText(1, name);
        getIdQuery.exec();
        return getIdQuery.getInt(0);
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return 0;
}

std::string SqlStorage::getFolderName(unsigned int id)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::format getNameString("SELECT %1% FROM %2% WHERE %3% = ?;");
    getNameString % COL_FOLDER_NAME % TABLE_FOLDER % COL_FOLDER_ID;
    try {
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();
        storage::SQLQuery getNameQuery(connection->prepare(getNameString.str()));
        getNameQuery.bindInt(1, id);
        getNameQuery.exec();

        return getNameQuery.getString(0);
    } catch (storage::StorageException& e) {
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
    return std::string();
}

void SqlStorage::readSession(Session& session, std::shared_ptr< storage::SQLDatabase > connection)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(session.isValid()){
        boost::format getSessionDataString("SELECT %1%, %2% FROM %3% WHERE %4% = ? ");
        getSessionDataString % COL_URL_TABID % COL_URL_URL % TABLE_URL % COL_URL_SESION_ID;

        storage::SQLQuery getSessionDataQuery = connection->prepare(getSessionDataString.str());
        getSessionDataQuery.bindInt(1, session.sessionId());

        getSessionDataQuery.exec();
        while(getSessionDataQuery.hasNext()){
            std::cout << __FUNCTION__ <<":"<< &session <<":"<< session.sessionId()<<":"<< getSessionDataQuery.getString(0) << std::endl;
            session.m_items[getSessionDataQuery.getString(0)].first = getSessionDataQuery.getString(1);
            getSessionDataQuery.next();
        }
    }
}

SessionsVector SqlStorage::getAllSessions()
{
    SessionsVector sessionContainer;

    boost::format getSessionsString("SELECT %1%, %2%, %3% FROM %4% ORDER BY %3% DESC;");
    getSessionsString % COL_SESSION_ID % COL_SESSION_NAME % COL_SESSION_DATE % TABLE_SESSION;
    unsigned int sessionId;
    std::string sessionName;
    boost::posix_time::ptime sessionTime;
    try{
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();

        storage::SQLQuery getSessionsQuery(connection->prepare(getSessionsString.str()));

        getSessionsQuery.exec();

        while(getSessionsQuery.hasNext()){
            sessionId = getSessionsQuery.getInt(0);
            sessionName = getSessionsQuery.getString(1);
            sessionTime = boost::posix_time::from_iso_string(getSessionsQuery.getString(2));
            Session session(sessionId, sessionTime, sessionName);
            readSession(session, connection);
            sessionContainer.push_back(std::move(session));
            getSessionsQuery.next();
        }
        return sessionContainer;
    }catch( storage::StorageException &e){
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        return SessionsVector();
    }
}

void SqlStorage::deleteSession(Session& session)
{
    if(session.isValid()){
        boost::format deleteSessionString("DELETE FROM %1% WHERE %2% = ?;");
        deleteSessionString % TABLE_SESSION % COL_SESSION_ID;
        try{
            storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
            std::shared_ptr<storage::SQLDatabase> connection = scope.database();

            storage::SQLQuery deleteSessionQuery(connection->prepare(deleteSessionString.str()));
            deleteSessionQuery.bindInt(1, session.sessionId());
            deleteSessionQuery.exec();
        }catch( storage::StorageException &e){
            BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
        }
        session.m_isValid = false;
    }
}

void SqlStorage::deleteAllSessions()
{
    boost::format deleteSessionString("DELETE FROM %1%;");
    deleteSessionString % TABLE_SESSION;
    try{
        storage::SQLTransactionScope scope(storage::DriverManager::getDatabase(m_dbString));
        std::shared_ptr<storage::SQLDatabase> connection = scope.database();

        storage::SQLQuery deleteSessionQuery(connection->prepare(deleteSessionString.str()));
        deleteSessionQuery.exec();
    }catch( storage::StorageException &e){
        BROWSER_LOGD("[%s:%d] SQLException (%d): %s ", __PRETTY_FUNCTION__, __LINE__, e.getErrorCode(), e.getMessage());
    }
}

}//end namespace Session
}//end namespace tizen_browser
