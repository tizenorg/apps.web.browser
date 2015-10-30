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

#include "HistoryException.h"

namespace tizen_browser
{
namespace services
{

HistoryException::HistoryException(const std::string & message)
{
    this->m_message = message;
}

HistoryException::~HistoryException() throw ()
{
}

const char * HistoryException::getMessage() const throw ()
{
    return m_message.c_str();
}

}
}
