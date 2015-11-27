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

/*
 * BookmarkFolder.cpp
 *
 *  Created on: Nov 26, 2015
 *      Author: m.kawonczyk@samsung.com
 */

#include "browser_config.h"
#include "BookmarkFolder.h"

#include <string>
#include <Evas.h>

namespace tizen_browser{
namespace services{

BookmarkFolder::BookmarkFolder()
    : m_id(0),
      m_name(),
      m_count(0)
{
}

BookmarkFolder::BookmarkFolder(unsigned int id, const std::string& name, unsigned int count)
    : m_id(id),
      m_name(name),
      m_count(count)
{
}

BookmarkFolder::~BookmarkFolder()
{
}

}
}
