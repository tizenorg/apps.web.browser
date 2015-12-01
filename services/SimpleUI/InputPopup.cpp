#include "InputPopup.h"

namespace tizen_browser {
namespace base_ui {

InputPopup::InputPopup() :
    m_parent(nullptr),
    m_layout(nullptr)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SimpleUI/InputPopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

InputPopup::~InputPopup()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_smart_callback_del(m_entry, "focused", _entry_focused);
    evas_object_smart_callback_del(m_entry, "unfocused", _entry_unfocused);
    evas_object_smart_callback_del(m_entry, "changed,user", _entry_changed);
    evas_object_smart_callback_del(m_inputCancel, "clicked", _inputCancel_clicked);
    evas_object_smart_callback_del(m_button_ok, "clicked", _okButton_clicked);
    evas_object_smart_callback_del(m_button_cancel, "clicked", _cancelButton_clicked);
    evas_object_del(m_inputCancel);
    evas_object_del(m_entry);
    evas_object_del(m_inputArea);
    evas_object_del(m_button_ok);
    evas_object_del(m_button_cancel);
    evas_object_del(m_buttonsBox);
    ecore_timer_del(m_timer);
    evas_object_del(m_layout);
    button_clicked.disconnect_all_slots();
    popupDismissed.disconnect_all_slots();
    popupShown.disconnect_all_slots();
}

InputPopup* InputPopup::createInputPopup(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = new InputPopup();
    inputPopup->m_parent = parent;
    return inputPopup;
}

InputPopup* InputPopup::createInputPopup(Evas_Object *parent,const std::string& title,const std::string& message,const std::string& input,
                                         const std::string& okButtonText, const std::string& cancelButtonText)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = new InputPopup();
    inputPopup->m_parent = parent;
    inputPopup->m_title = title;
    inputPopup->m_message = message;
    inputPopup->m_input = input;
    inputPopup->m_okButtonText = okButtonText;
    inputPopup->m_cancelButtonText = cancelButtonText;
    return inputPopup;
}

void InputPopup::setInput(const std::string& input)
{
    m_input = input;
}

void InputPopup::setTitle(const std::string& title)
{
    m_title = title;
}

void InputPopup::setMessage(const std::string& message)
{
    m_message = message;
}

void InputPopup::setOkButtonText(const std::string& okButtonText)
{
    m_okButtonText = okButtonText;
}

void InputPopup::setCancelButtonText(const std::string& cancelButtonText)
{
    m_cancelButtonText = cancelButtonText;
}

void InputPopup::show()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    createLayout();
    popupShown(this);
}

void InputPopup::dismiss()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_focus_allow_set(m_inputCancel, EINA_FALSE);
    elm_object_signal_emit(m_inputArea, "entry_unfocused", "ui");
    elm_object_signal_emit(m_entry, "unfocused", "ui");
    // TODO Workaround for too fast deleted callbacks. If there will be a better solution
    // timer should be removed.
    m_timer = ecore_timer_add(0.2, dismissSlower, this);
}

void InputPopup::onBackPressed()
{
    dismiss();
}

void InputPopup::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_layout = elm_layout_add(m_parent);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "input-popup-layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(m_layout, "title_text", m_title.c_str());

    m_inputArea = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout, "input_swallow", m_inputArea);

    evas_object_size_hint_weight_set(m_inputArea, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_inputArea, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_inputArea);

    elm_layout_file_set(m_inputArea, m_edjFilePath.c_str(), "input-area-layout");
    elm_object_part_text_set(m_inputArea, "input_message_text", m_message.c_str());

    m_entry = elm_entry_add(m_inputArea);
    elm_object_style_set(m_entry, "popup-input-entry");

    elm_entry_single_line_set(m_entry, EINA_TRUE);
    elm_entry_scrollable_set(m_entry, EINA_TRUE);
    elm_entry_input_panel_layout_set(m_entry, ELM_INPUT_PANEL_LAYOUT_URL);
    elm_object_part_content_set(m_inputArea, "input_text_swallow", m_entry);
    elm_object_part_text_set(m_entry, "elm.text", m_input.c_str());

    evas_object_smart_callback_add(m_entry, "focused", _entry_focused, (void*)this);
    evas_object_smart_callback_add(m_entry, "unfocused", _entry_unfocused, (void*)this);
    evas_object_smart_callback_add(m_entry, "changed,user", _entry_changed, (void*)this);

    m_inputCancel = elm_button_add(m_inputArea);
    elm_object_style_set(m_inputCancel, "invisible_button");
    evas_object_smart_callback_add(m_inputCancel, "clicked", _inputCancel_clicked, this);

    evas_object_show(m_inputCancel);
    elm_object_part_content_set(m_inputArea, "input_cancel_click", m_inputCancel);

    m_buttonsBox = elm_box_add(m_layout);
    elm_box_horizontal_set(m_buttonsBox, EINA_TRUE);
    evas_object_size_hint_weight_set(m_buttonsBox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_buttonsBox, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_button_cancel = elm_button_add(m_buttonsBox);
    elm_object_style_set(m_button_cancel, "input-popup-button");
    evas_object_size_hint_weight_set(m_button_cancel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_button_cancel, 0.5, 0.5);
    elm_object_part_text_set(m_button_cancel, "elm.text", m_cancelButtonText.c_str());
    elm_box_pack_end(m_buttonsBox, m_button_cancel);
    evas_object_smart_callback_add(m_button_cancel, "clicked", _cancelButton_clicked, (void*)this);

    evas_object_show(m_button_cancel);

    m_button_ok = elm_button_add(m_buttonsBox);
    elm_object_style_set(m_button_ok, "input-popup-button");
    evas_object_size_hint_weight_set(m_button_ok, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_button_ok, 0.5, 0.5);
    elm_object_part_text_set(m_button_ok, "elm.text", m_okButtonText.c_str());
    elm_box_pack_end(m_buttonsBox, m_button_ok);
    evas_object_smart_callback_add(m_button_ok, "clicked", _okButton_clicked, (void*)this);

    evas_object_show(m_button_ok);
    elm_object_signal_emit(m_button_ok, "visible", "ui");

    evas_object_show(m_buttonsBox);
    elm_object_part_content_set(m_layout, "buttons_swallow", m_buttonsBox);

    evas_object_show(m_layout);
}

void InputPopup::_entry_focused(void* data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        InputPopup*  inputPopup = static_cast<InputPopup*>(data);
        elm_object_focus_allow_set(inputPopup->m_inputCancel, EINA_TRUE);
        elm_object_signal_emit(inputPopup->m_inputArea, "entry_focused", "ui");
        elm_object_signal_emit(inputPopup->m_entry, "focused", "ui");
    }
}

void InputPopup::_entry_unfocused(void* data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        InputPopup*  inputPopup = static_cast<InputPopup*>(data);
        elm_object_focus_allow_set(inputPopup->m_inputCancel, EINA_FALSE);
        elm_object_signal_emit(inputPopup->m_inputArea, "entry_unfocused", "ui");
        elm_object_signal_emit(inputPopup->m_entry, "unfocused", "ui");
    }
}

void InputPopup::_entry_changed(void* data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        InputPopup*  inputPopup = static_cast<InputPopup*>(data);
        std::string text = elm_object_part_text_get(inputPopup->m_entry, "elm.text");
        if (text.empty()) {
            elm_object_disabled_set(inputPopup->m_button_ok, EINA_TRUE);
            elm_object_signal_emit(inputPopup->m_button_ok, "dissabled", "ui");
        } else {
            elm_object_disabled_set(inputPopup->m_button_ok, EINA_FALSE);
            elm_object_signal_emit(inputPopup->m_button_ok, "enabled", "ui");
        }
    }
}

void InputPopup::_inputCancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        InputPopup*  inputPopup = static_cast<InputPopup*>(data);
        elm_object_part_text_set(inputPopup->m_entry, "elm.text", "");
        elm_object_disabled_set(inputPopup->m_button_ok, EINA_TRUE);
        elm_object_signal_emit(inputPopup->m_button_ok, "dissabled", "ui");
    }
}

void InputPopup::_okButton_clicked(void *data, Evas_Object *, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = static_cast<InputPopup*>(data);
    inputPopup->button_clicked(elm_object_part_text_get(inputPopup->m_entry, "elm.text"));
    inputPopup->dismiss();
}

Eina_Bool InputPopup::dismissSlower(void* data) {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup* ip = static_cast<InputPopup*>(data);
    ip->popupDismissed(ip);
    return EINA_TRUE;
}

void InputPopup::_cancelButton_clicked(void* data, Evas_Object *, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = static_cast<InputPopup*>(data);
    inputPopup->dismiss();
}

}
}
