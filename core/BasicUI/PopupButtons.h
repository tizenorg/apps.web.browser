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

#ifndef __POPUP_BUTTONS_H__
#define __POPUP_BUTTONS_H__ 1

#include <map>

namespace tizen_browser
{
namespace base_ui
{

    enum PopupButtons
    {
        OK      = 1 << 1
       ,CANCEL  = 1 << 2
       ,YES     = 1 << 3
       ,NO      = 1 << 4
       ,CLOSE   = 1 << 5
       ,CONNECT = 1 << 6
       ,CONTINUE= 1 << 7
       ,CLOSE_TAB = 1 << 8
    };

    static std::map<PopupButtons, std::string> createTranslations()
    {
        std::map<PopupButtons, std::string> m;
        m[OK] = "Ok";
        m[CANCEL] = "Cancel";
        m[YES] = "Yes";
        m[NO] = "No";
        m[CLOSE] = "Close";
        m[CONNECT] = "Connect";
        m[CONTINUE] = "Continue";
        m[CLOSE_TAB] = "Close tab";

        return m;
    }

    static std::map<PopupButtons, std::string> buttonsTranslations = createTranslations();

}

}

#endif //__POPUP_BUTTONS_H__
