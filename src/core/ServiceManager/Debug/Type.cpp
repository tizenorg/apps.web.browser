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

#include "browser_config.h"
#include "core/ServiceManager/Debug/Type.h"
#ifdef __GNUG__ // check if compiler is GCC

#include <memory>

#include <cstdlib>

#include <cxxabi.h>

#if __cplusplus < 201103L // check for c++11 features
//for c++03

struct handle {
    char * p;
    handle(char * ptr) : p(ptr) { }
    ~handle() {
        std::free(p);
    }
};

std::string demangle(const char * name)
{

    int status = -4; // some arbitrary value to eliminate the compiler warning

    handle result(abi::__cxa_demangle(name, NULL, NULL, &status));

    return (status == 0) ? result.p : name ;
}

#else // __cplusplus | compiler supporting c++11
//for c++11

std::string demangle(const char * name)
{
    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void( *)(void *)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status == 0) ? res.get() : name ;
}

#endif // __cplusplus

#else // __GNUG__

// does nothing if not g++
std::string demangle(const char * name)
{
    return name;
}

#endif // __GUNG__
