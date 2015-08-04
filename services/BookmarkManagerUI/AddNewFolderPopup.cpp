

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <vector>

#include "Tools/EflTools.h"
#include "BrowserLogger.h"

#include "AddNewFolderPopup.h"
#include "BrowserLogger.h"
#include "ServiceManager.h"
#include "AbstractMainWindow.h"


namespace tizen_browser{
namespace base_ui{

AddNewFolderPopup::AddNewFolderPopup(Evas_Object* main_layout) :
    m_popup(nullptr),
    m_cancel_button(nullptr),
    m_editfield_entry(nullptr),
    m_ok_button(nullptr),
    m_mainLayout(main_layout)
{
}

AddNewFolderPopup::AddNewFolderPopup(Evas_Object *main_layout, Evas_Object *content, const std::string& message,
                                     const std::string& title, const std::string& okButtonText,
                                     const std::string& cancelButtonText) :
    m_popup(nullptr),
    m_cancel_button(nullptr),
    m_editfield_entry(nullptr),
    m_ok_button(nullptr),
    m_content(content),
    m_message(message),
    m_title(title),
    m_okButtonText(okButtonText),
    m_cancelButtonText(cancelButtonText),
    m_mainLayout(main_layout)
{

}

void AddNewFolderPopup::setTitle(const std::string& title)
{
    m_title = title;
}

void AddNewFolderPopup::setMessage(const std::string& message)
{
    m_message = message;
}

void AddNewFolderPopup::setContent(Evas_Object* content)
{
    m_content = content;
}

void AddNewFolderPopup::setOkButtonText(const std::string& okButtonText)
{
    m_okButtonText = okButtonText;
}

void AddNewFolderPopup::setCancelButtonText(const std::string& cancelButtonText)
{
    m_cancelButtonText = cancelButtonText;
}

void AddNewFolderPopup::show()
{
    std::string edjePath = std::string(EDJE_DIR);
    edjePath.append("BookmarkManagerUI/AddNewFolderPopup.edj");
    elm_theme_extension_add(0, edjePath.c_str());
    m_popup = elm_popup_add(m_mainLayout);
    evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

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

    m_cancel_button = elm_button_add(m_popup);
    elm_object_style_set(m_cancel_button, "popup");
    elm_object_part_content_set(m_popup, "button1",m_cancel_button);
    elm_object_part_text_set(m_cancel_button, "elm.text", m_cancelButtonText.c_str());
    evas_object_smart_callback_add(m_cancel_button, "clicked", popup_cancel_cb, this);

    m_ok_button = elm_button_add(m_popup);
    elm_object_style_set(m_ok_button, "popup");
    elm_object_part_content_set(m_popup, "button2", m_ok_button);
    elm_object_part_text_set(m_ok_button, "elm.text", m_okButtonText.c_str());
    evas_object_smart_callback_add(m_ok_button, "clicked", popup_ok_cb, this);

    evas_object_show(m_popup);
}

void AddNewFolderPopup::hide()
{
    evas_object_hide(m_popup);
    elm_object_signal_emit(m_mainLayout, "elm,state,hide", "elm");
}

void AddNewFolderPopup::popup_ok_cb(void *data, Evas_Object *, void*)
{
    BROWSER_LOGD("[%s],", __func__);
    if (data != nullptr)
    {
        AddNewFolderPopup *ownPopup = static_cast<AddNewFolderPopup*>(data);
        ownPopup->on_ok(ownPopup->m_editfield_entry);
    }
}
void AddNewFolderPopup::popup_cancel_cb(void *data, Evas_Object *, void*)
{
    BROWSER_LOGD("[%s],", __func__);
    if (data != nullptr)
    {
        AddNewFolderPopup *ownPopup = static_cast<AddNewFolderPopup*>(data);
        ownPopup->on_cancel(ownPopup->m_editfield_entry);
    }
}

}
}
