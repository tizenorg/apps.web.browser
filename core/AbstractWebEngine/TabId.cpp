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

/*
 * TabId.cpp
 *
 *  Created on: Apr 11, 2014
 *      Author: p.rafalski
 */

#include "browser_config.h"
#include "TabId.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>

namespace tizen_browser {
namespace basic_webengine {

TabId TabId::NONE;

TabId::TabId()
    : m_id(boost::uuids::random_generator()())
{
}

TabId::TabId(const TabId & n)
{
    m_id = n.m_id;
}

TabId::~TabId()
{
}

std::string TabId::toString() const {
    std::stringstream ss;
    ss << m_id;
    return ss.str();
}

TabContent::TabContent(TabId id,const std::string& title, std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail){
    this->m_id = id;
    this->m_title = title;
    this->m_thumbnail = thumbnail;
}

TabId TabContent::getId() const {
    return m_id;
}

std::string TabContent::getTitle() const {
    return m_title;
}

std::shared_ptr<tizen_browser::tools::BrowserImage> TabContent::getThumbnail() const{
    return m_thumbnail;
}


} /* end of basic_webengine */
} /* end of tizen_browser */
