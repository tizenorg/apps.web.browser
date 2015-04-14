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

#include "TestServiceOne.h"
#include "service_macros.h"
#include "boost/test/unit_test.hpp"


TestServiceOne::TestServiceOne()
    : m_isRunning(false)
{

}

TestServiceOne::~TestServiceOne()
{
    BOOST_TEST_MESSAGE(__PRETTY_FUNCTION__);
}

bool TestServiceOne::isRunning()
{
    BOOST_TEST_MESSAGE(std::string(__PRETTY_FUNCTION__) + "->" + (m_isRunning ? "true": "false"));
    return m_isRunning;
}

void TestServiceOne::run()
{
    BOOST_TEST_MESSAGE(__PRETTY_FUNCTION__);
    m_isRunning=true;
}

void TestServiceOne::stop()
{
    BOOST_TEST_MESSAGE(__PRETTY_FUNCTION__);
    m_isRunning=false;
}
