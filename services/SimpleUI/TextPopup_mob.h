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

#ifndef __TEXT_POPUP_H__
#define __TEXT_POPUP_H__ 1

#include <Evas.h>
#include <Elementary.h>
#include <string>
#include <list>
#include <memory>

#include "AbstractPopup.h"
#include "PopupButtons.h"
#include "WebConfirmation.h"

namespace tizen_browser
{

namespace base_ui
{

class TextPopup : public interfaces::AbstractPopup
{
public:
    static TextPopup* createPopup(Evas_Object* parent);
    static TextPopup* createPopup(Evas_Object* parent, const std::string& title, const std::string& message);

    void show();
    void dismiss();
    void onBackPressed();

    void setTitle(const std::string& title);
    void setMessage(const std::string& message);
    void setLeftButton(const PopupButtons& button);
    void setRightButton(const PopupButtons& button);
    void createLayout();
    boost::signals2::signal<void (PopupButtons)> buttonClicked;

    ~TextPopup();

private:
    TextPopup(Evas_Object* parent);
    TextPopup(Evas_Object* parent, const std::string& title, const std::string& message);

    Evas_Object* m_parent;
    Evas_Object* m_layout;
    Evas_Object* m_left_button;
    Evas_Object* m_right_button;
    PopupButtons m_left_button_type;
    PopupButtons m_right_button_type;
    std::string m_title;
    std::string m_message;
    static void _left_response_cb(void* data, Evas_Object* obj, void* event_info);
    static void _right_response_cb(void* data, Evas_Object* obj, void* event_info);
    std::string edjFilePath;
};

}

}

#endif //__TEXT_POPUP_H__
