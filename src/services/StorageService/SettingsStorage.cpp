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

#include <string>
#include <core/ServiceManager/Debug/BrowserAssert.h>
#include <boost/any.hpp>

#include "services/StorageService/Field.h"
#include "core/BrowserLogger.h"
#include "services/StorageService/DriverManager.h"
#include "core/Tools/Blob.h"
#include "services/StorageService/StorageException.h"
#include "services/StorageService/StorageExceptionInitialization.h"
#include "core/Tools/EflTools.h"
#include "core/Config/Config.h"
#include "services/StorageService/SettingsStorage.h"
#include "services/StorageService/DBTools.h"

namespace
{
// ------ Database SETTINGS ------
const std::string TABLE_SETTINGS = "SETTINGS";
const std::string COL_SETTINGS_KEY = "KEY";
const std::string COL_SETTINGS_VALUE_INT = "VALUE_INT";
const std::string COL_SETTINGS_VALUE_DOUBLE = "VALUE_DOUBLE";
const std::string COL_SETTINGS_VALUE_TEXT = "VALUE_TEXT";

const std::string DDL_CREATE_TABLE_SETTINGS = "CREATE TABLE " + TABLE_SETTINGS
                                              + " (" + COL_SETTINGS_KEY + " TEXT PRIMARY KEY, "
                                              + COL_SETTINGS_VALUE_INT + " INTEGER, "
                                              + COL_SETTINGS_VALUE_DOUBLE + " DOUBLE, "
                                              + COL_SETTINGS_VALUE_TEXT + " TEXT)";

const std::string INSERT_TABLE_SETTINGS_INT_VALUE = "insert or replace into " + TABLE_SETTINGS + " ("
                                                    + COL_SETTINGS_KEY + ", "
                                                    + COL_SETTINGS_VALUE_INT
                                                    + ") values (?,?)";

const std::string INSERT_TABLE_SETTINGS_DOUBLE_VALUE = "insert or replace into " + TABLE_SETTINGS + " ("
                                                       + COL_SETTINGS_KEY + ", "
                                                       + COL_SETTINGS_VALUE_DOUBLE
                                                       + ") values (?,?)";

const std::string INSERT_TABLE_SETTINGS_TEXT_VALUE = "insert or replace into " + TABLE_SETTINGS + " ("
                                                     + COL_SETTINGS_KEY + ", "
                                                     + COL_SETTINGS_VALUE_TEXT
                                                     + ") values (?,?)";

const std::string DELETE_SETTING = "delete from " + TABLE_SETTINGS + " where "
                                   + COL_SETTINGS_KEY + "=?";

const std::string DELETE_ALL_SETTINGS = "delete from " + TABLE_SETTINGS;

const std::string SQL_FIND_VALUE_INT_SETTINGS = "select " + COL_SETTINGS_VALUE_INT + " from " + TABLE_SETTINGS
                                                + " where " + COL_SETTINGS_KEY + "=?";

const std::string SQL_FIND_VALUE_DOUBLE_SETTINGS = "select " + COL_SETTINGS_VALUE_DOUBLE + " from " + TABLE_SETTINGS
                                                   + " where " + COL_SETTINGS_KEY + "=?";

const std::string SQL_FIND_VALUE_TEXT_SETTINGS = "select " + COL_SETTINGS_VALUE_TEXT + " from " + TABLE_SETTINGS
                                                 + " where " + COL_SETTINGS_KEY + "=?";

const std::string SQL_CHECK_IF_PARAM_EXISTS = "select " + COL_SETTINGS_KEY + " from " + TABLE_SETTINGS
                                                + " where " + COL_SETTINGS_KEY + "=?";

// ------ (end) Database SETTINGS ------

}

namespace tizen_browser {
namespace storage {

SettingsStorage::SettingsStorage()
    : m_dbSettingsInitialised(false)
    , m_isInitialized(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    init();
}

SettingsStorage::~SettingsStorage()
{
}

void SettingsStorage::init(bool testmode)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_isInitialized) {
        return;
    }

    std::string resourceDbDir;
    std::string dbSettings;

    if (!testmode) {
        resourceDbDir = boost::any_cast < std::string > (tizen_browser::config::Config::getInstance().get("resourcedb/dir"));
        dbSettings = boost::any_cast < std::string > (tizen_browser::config::Config::getInstance().get("DB_SETTINGS"));
    } else {
        resourceDbDir = boost::any_cast < std::string > (tizen_browser::config::Config::getInstance().get("resourcedb/dir"));
        dbSettings = "settings_test.db";
    }

    DB_SETTINGS = resourceDbDir + dbSettings;

    BROWSER_LOGD("[%s:%d] DB_SETTINGS=%s", __PRETTY_FUNCTION__, __LINE__, DB_SETTINGS.c_str());

    try {
        initDatabaseSettings(DB_SETTINGS);
    } catch (storage::StorageExceptionInitialization & e) {
        BROWSER_LOGE("[%s:%d] Cannot initialize database %s!", __PRETTY_FUNCTION__, __LINE__, DB_SETTINGS.c_str());
    }

    m_isInitialized = true;
}

/**
 * @throws StorageExceptionInitialization on error
 */
void SettingsStorage::initDatabaseSettings(const std::string & db_str)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!m_dbSettingsInitialised) {
        try {
            dbtools::checkAndCreateTable(db_str, TABLE_SETTINGS, DDL_CREATE_TABLE_SETTINGS);
        } catch (storage::StorageException & e) {
            throw storage::StorageExceptionInitialization(e.getMessage(),
                                                          e.getErrorCode());
        }

        m_dbSettingsInitialised = true;
    }

    M_ASSERT(m_dbSettingsInitialised);
}

void SettingsStorage::initWebEngineSettingsFromDB()
{
    for (auto s : basic_webengine::PARAMS_NAMES) {
        basic_webengine::WebEngineSettings param = s.first;
        if (isParamPresent(param))
            setWebEngineSettingsParam(param, getParamVal(param));
    }
}

void SettingsStorage::setParam(basic_webengine::WebEngineSettings param, bool value) const
{
    const std::string& paramName = basic_webengine::PARAMS_NAMES.at(param);
    setSettingsInt(paramName, static_cast<int>(value));
}

bool SettingsStorage::isParamPresent(basic_webengine::WebEngineSettings param) const
{
    const std::string& paramName = basic_webengine::PARAMS_NAMES.at(param);
    auto con = storage::DriverManager::getDatabase(DB_SETTINGS);

    storage::SQLQuery select(con->prepare(SQL_CHECK_IF_PARAM_EXISTS));
    select.bindText(1, paramName);
    select.exec();
    return select.hasNext();
}

bool SettingsStorage::getParamVal(basic_webengine::WebEngineSettings param) const
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    const std::string& paramName = basic_webengine::PARAMS_NAMES.at(param);
    return static_cast<bool>(getSettingsInt(paramName, 0));
}

/**
 * @throws StorageException on error
 */
int SettingsStorage::getSettingsInt(const std::string & key, const int defaultValue) const
{
    auto con = storage::DriverManager::getDatabase(DB_SETTINGS);

    storage::SQLQuery select(con->prepare(SQL_FIND_VALUE_INT_SETTINGS));
    select.bindText(1, key);
    select.exec();

    if (select.hasNext()) {
        return select.getInt(0);
    }

    return defaultValue;
}

/**
 * @throws StorageException on error
 */
double SettingsStorage::getSettingsDouble(const std::string & key, const double defaultValue) const
{
    auto con = storage::DriverManager::getDatabase(DB_SETTINGS);

    storage::SQLQuery select(con->prepare(SQL_FIND_VALUE_DOUBLE_SETTINGS));
    select.bindText(1, key);
    select.exec();

    if (select.hasNext()) {
        return select.getDouble(0);
    }

    return defaultValue;
}

/**
 * @throws StorageException on error
 */
const std::string SettingsStorage::getSettingsText(const std::string & key, const std::string & defaultValue) const
{
    auto con = storage::DriverManager::getDatabase(DB_SETTINGS);

    storage::SQLQuery select(con->prepare(SQL_FIND_VALUE_TEXT_SETTINGS));
    select.bindText(1, key);
    select.exec();

    if (select.hasNext()) {
        return select.getString(0);
    }

    return defaultValue;
}

/**
 * @throws StorageException on error
 */
void SettingsStorage::setSettingsValue(const std::string & key, storage::FieldPtr field) const
{
    auto con = storage::DriverManager::getDatabase(DB_SETTINGS);

    storage::SQLQuery insert;

    switch (field->getType()) {
        case SQLITE_INTEGER:
            insert = con->prepare(INSERT_TABLE_SETTINGS_INT_VALUE);
            insert.bindInt(2, field->getInt());
            break;
        case SQLITE_FLOAT:
            insert = con->prepare(INSERT_TABLE_SETTINGS_DOUBLE_VALUE);
            insert.bindDouble(2, field->getDouble());
            break;
        case SQLITE3_TEXT:
            insert = con->prepare(INSERT_TABLE_SETTINGS_TEXT_VALUE);
            insert.bindText(2, field->getString());
            break;
        default:
            BROWSER_LOGE("[%s:%d] Unknown filed type!", __PRETTY_FUNCTION__, __LINE__);
            M_ASSERT(0);
            return;
    }

    insert.bindText(1, key);
    insert.exec();
}

/**
 * @throws StorageException on error
 */
void SettingsStorage::setSettingsInt(const std::string & key, int value) const
{
    storage::FieldPtr field = std::make_shared<storage::Field>(value);
    setSettingsValue(key, field);
}

/**
 * @throws StorageException on error
 */
void SettingsStorage::setSettingsDouble(const std::string & key, double value) const
{
    storage::FieldPtr field = std::make_shared<storage::Field>(value);
    setSettingsValue(key, field);
}

/**
 * @throws StorageException on error
 */
void SettingsStorage::setSettingsString(const std::string & key, std::string value) const
{
    storage::FieldPtr field = std::make_shared<storage::Field>(value);
    setSettingsValue(key, field);
}

}
}
