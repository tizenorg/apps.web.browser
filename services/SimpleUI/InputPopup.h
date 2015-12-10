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
 * InputPopup.h
 *
 *  Created on: Nov 24, 2015
 *      Author: m.kawonczyk@samsung.com
 */

#ifndef __INPUT_POPUP_H__
#define __INPUT_POPUP_H__ 1

#include <Evas.h>
#include <Elementary.h>
#include <string>
#include <list>
#include <memory>
#include "AbstractPopup.h"
#include "ServiceManager.h"

namespace tizen_browser {
namespace base_ui {

class InputPopup : public interfaces::AbstractPopup
{

public:
    static InputPopup* createPopup(Evas_Object *parent);
    static InputPopup* createPopup(Evas_Object *parent,const std::string& title,const std::string& message,const std::string& input,
                                        const std::string& rightButtonText, const std::string& leftButtonText, bool accept_right_left);

    void show();
    void dismiss();
    void onBackPressed();

    void setContent(Evas_Object *content);
    void setInput(const std::string &input);
    void setTitle(const std::string &title);
    void setMessage(const std::string &message);
    void setOkButtonText(const std::string &okButtonText);
    void setCancelButtonText(const std::string &cancelButtonText);
    void setAcceptRightLeft(bool);
    void setFocusOnEntry();

    boost::signals2::signal<void (const std::string&)> button_clicked;
    boost::signals2::signal<void ()> setFocusOnMoreMenu;

private:
    InputPopup();
    ~InputPopup();
    void createLayout();

    std::string m_edjFilePath;
    static void _right_button_clicked(void *data, Evas_Object *btn, void*);
    static void _left_button_clicked(void *data, Evas_Object *btn, void*);
    static void _entry_changed(void * data, Evas_Object *, void*);
    static void _entry_unfocused(void * data, Evas_Object *, void*);
    static void _entry_focused(void * data, Evas_Object *, void*);
    static void _input_cancel_clicked(void * data, Evas_Object *, void *);
    static Eina_Bool dismissSlower(void* data);

    Evas_Object *m_parent;
    Evas_Object *m_layout;
    Evas_Object *m_buttons_box;
    Evas_Object *m_button_left;
    Evas_Object *m_button_right;
    Evas_Object *m_input_area;
    Evas_Object *m_input_cancel;
    Evas_Object *m_entry;
    std::string m_input;
    std::string m_title;
    std::string m_message;
    std::string m_ok_button_text;
    std::string m_cancel_button_text;
    bool m_accept_right_left;
    Ecore_Timer *m_timer; // Workaround for too fast removal of callbacks
};

}
}
#endif //__INPUT_POPUP_H__
