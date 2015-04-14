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

#ifndef __CONFIG_H__
#define __CONFIG_H__ 1

#include <string>
#include <map>

#include <boost/any.hpp>
//#include <boost/property_tree/json_parser.hpp>
//#include <boost/property_tree/ptree.hpp>

namespace tizen_browser
{
namespace config
{

/**
 * @brief Abstract config interface.
 */
class BasicConfig
{
public:
    virtual ~BasicConfig() {}
    virtual void load(const std::string & filename) = 0;
    virtual void store(const std::string & filename) = 0;
};


/**
 * @brief Default config placeholder.
 */
class DefaultConfig : BasicConfig
{
public:

    /**
     * @brief This method loads config from file, validates if file has
	 * necessary values and sections.
     *
     * @param filename Name of config file.
     */
    void load(const std::string & filename); //final;
    /**
     * @brief This method stores config to file.
     *
     * @param filename Name of config file.
     */
    void store(const std::string & filename); //final;
    /**
     * @brief This method gets value from config stored under key.
     *
     * @param key Key of item we want to get
     *
     * @return Value from config.
     */
    boost::any get(const std::string & key);
    /**
     * @brief This method sets passed value under passed key.
     *
     * @param key Key of item to set value.
     * @param value Value to be set.
     */
    void set(const std::string & key, const boost::any & value);
private:
    std::map<std::string, boost::any> m_data;
};


} /* end of namespace config */
} /* end of namespace tizen_browser */

#endif /* __CONFIG_H__ */
