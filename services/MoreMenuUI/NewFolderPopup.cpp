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

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <vector>

#include "Tools/EflTools.h"
#include "NewFolderPopup.h"
#include "BrowserLogger.h"

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

NewFolderPopup::NewFolderPopup(Evas_Object* main_layout) :
    m_popup(NULL),
    m_content(NULL),
    m_mainLayout(main_layout)
{

}

NewFolderPopup::NewFolderPopup(Evas_Object *main_layout, Evas_Object *content, const char *message, char* title, char* okButtonText, char* cancelButtonText) :
    m_popup(NULL),
    m_mainLayout(main_layout),
    m_content(content),
    m_message(message),
    m_title(title),
    m_okButtonText(okButtonText),
    m_cancelButtonText(cancelButtonText)
{

}

void NewFolderPopup::setTitle(const std::string& title)
{
    m_title = title;
}

void NewFolderPopup::setMessage(const std::string& message)
{
    m_message = message;
}

void NewFolderPopup::setContent(Evas_Object* content)
{
    m_content = content;
}

void NewFolderPopup::setOkButtonText(const std::string& okButtonText)
{
    m_okButtonText = okButtonText;
}

void NewFolderPopup::setCancelButtonText(const std::string& cancelButtonText)
{
    m_cancelButtonText = cancelButtonText;
}

void NewFolderPopup::show()
{
    BROWSER_LOGD("[%si %d],", __func__, __LINE__);
    std::string edjePath = std::string(EDJE_DIR);
    edjePath.append("MoreMenuUI/NewFolderPopup.edj");
    elm_theme_extension_add(0, edjePath.c_str());
    m_popup = elm_layout_add(m_mainLayout);
    elm_layout_file_set(m_popup, edjePath.c_str(), "new_folder_popup");
    elm_object_part_text_set(m_popup, "title_text", m_title.c_str());

    Evas_Object *buttonsBox = elm_box_add(m_popup);
    elm_box_horizontal_set(buttonsBox, EINA_TRUE);

     /* for margins */
    Evas_Object *layout = elm_layout_add(m_popup);
    elm_layout_file_set(layout, edjePath.c_str() , "popup_input_text");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_content_set(m_popup, layout);
    elm_object_part_text_set(m_popup, "title,text", m_title.c_str());

    m_editfield_entry = elm_entry_add(layout);
    elm_object_style_set(m_editfield_entry, "uri_entry_popup");
    evas_object_size_hint_weight_set(m_editfield_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_editfield_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_entry_editable_set(m_editfield_entry, EINA_TRUE);
    elm_entry_cnp_mode_set(m_editfield_entry, ELM_CNP_MODE_PLAINTEXT);
    elm_entry_scrollable_set(m_editfield_entry, EINA_TRUE);
    elm_entry_autocapital_type_set(m_editfield_entry, ELM_AUTOCAPITAL_TYPE_NONE);
    elm_entry_prediction_allow_set(m_editfield_entry, EINA_FALSE);
    elm_entry_single_line_set(m_editfield_entry, EINA_TRUE);

    elm_entry_input_panel_layout_set(m_editfield_entry, ELM_INPUT_PANEL_LAYOUT_URL);
    elm_entry_input_panel_return_key_type_set(m_editfield_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
    elm_object_part_content_set(layout, "elm.swallow.content" , m_editfield_entry);
    elm_entry_cursor_end_set(m_editfield_entry);

    if (!m_okButtonText.empty())
    {
        BROWSER_LOGD("Button1, %s", edjePath.c_str());
        Evas_Object *btn1 = elm_button_add(buttonsBox);
        evas_object_size_hint_weight_set(btn1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(btn1, 0.5, 0.5);
        elm_object_style_set(btn1, "bookmark_button");
        elm_object_part_text_set(btn1, "elm.text", m_okButtonText.c_str());
        elm_box_pack_end(buttonsBox, btn1);
        evas_object_smart_callback_add(btn1, "clicked", popup_ok_cb, (void*)this);
        evas_object_show(btn1);
    }

    if (!m_cancelButtonText.empty())
    {
        BROWSER_LOGD("Button2");
        Evas_Object *btn2 = elm_button_add(buttonsBox);
        evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(btn2, 0.5, 0.5);
        elm_object_style_set(btn2, "bookmark_button");
        elm_object_part_text_set(btn2, "elm.text", m_cancelButtonText.c_str());
        elm_box_pack_end(buttonsBox, btn2);
        evas_object_smart_callback_add(btn2, "clicked", popup_cancel_cb, (void*)this);
        evas_object_show(btn2);
    }

    if(!m_message.empty())
        elm_object_part_text_set(m_popup, "elm.text", m_message.c_str());

    evas_object_show(m_editfield_entry);
    evas_object_show(buttonsBox);
    evas_object_show(layout);
    elm_object_part_content_set(m_popup, "buttons", buttonsBox);
    elm_object_part_content_set(m_popup, "content", layout);
    elm_object_part_content_set(m_mainLayout, "popup", m_popup);

    elm_object_signal_emit(m_mainLayout, "elm,state,show", "elm");
}

void NewFolderPopup::hide()
{
   evas_object_hide(m_popup);
   elm_object_signal_emit(m_mainLayout, "elm,state,hide", "elm");
}

void NewFolderPopup::popup_ok_cb(void *data, Evas_Object *btn, void*)
{
    BROWSER_LOGD("[%s],", __func__);
    NewFolderPopup *ownPopup = static_cast<NewFolderPopup*>(data);
    ownPopup->on_ok(ownPopup->m_editfield_entry);
}
void NewFolderPopup::popup_cancel_cb(void *data, Evas_Object *btn, void*)
{
    BROWSER_LOGD("[%s],", __func__);
    NewFolderPopup *ownPopup = static_cast<NewFolderPopup*>(data);
    ownPopup->on_cancel(ownPopup->m_editfield_entry);
}

}
}
