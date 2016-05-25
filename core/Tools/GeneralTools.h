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
#include <cstring>
#include "app_control.h"

#ifndef __GENERALTOOLS_H__
#define __GENERALTOOLS_H__

namespace tizen_browser
{
namespace tools
{
    static const int SUFIX_CHAR_DEL = 1;
    static const char * PROTCOL_BEGIN = "://";
    static const char END_SLASH = '/';
    // which protocol, when only domain is available?
    static const std::string PROTOCOL_DEFAULT("http://");

    // declaration using 'unused' attribute because in some modules not all functions are used
    static std::string fromChar(const char* c) __attribute__ ((unused));
    static std::string clearURL(const std::string & url) __attribute__ ((unused));
    static std::string extractDomain(const std::string & url) __attribute__ ((unused));
    static void ui_app_suspend() __attribute__ ((unused));

    static std::string fromChar(const char* c) { return c ? std::string(c) : std::string(); }

    static std::string clearURL(const std::string & url) {
        size_t beg = url.find(PROTCOL_BEGIN);
        beg += strlen(PROTCOL_BEGIN);
        return url.substr(beg, url.size() - beg - SUFIX_CHAR_DEL);
    }

    static std::string extractDomain(const std::string & url) {
        size_t beg = url.find(PROTCOL_BEGIN);
        beg += strlen(PROTCOL_BEGIN);
        size_t end = url.find(END_SLASH, beg);
        return url.substr(beg, end - beg);
    }

    static void ui_app_pause() {
        //From app_control.h on APP_CONTROL_OPERATION_MAIN:
        //"Definition for the app_control operation: An explicit launch for a homescreen application."
        app_control_h service;
        app_control_create(&service);
        app_control_set_operation(service, APP_CONTROL_OPERATION_MAIN);
        app_control_set_app_id(service, "org.tizen.homescreen-efl");
        app_control_send_launch_request(service, NULL, NULL);
        app_control_destroy(service);
    }
}
}

#endif
