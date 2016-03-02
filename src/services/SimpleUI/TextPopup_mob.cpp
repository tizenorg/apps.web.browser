#include "services/SimpleUI/TextPopup_mob.h"
#include "core/ServiceManager/ServiceManager.h"
#include "core/BasicUI/AbstractMainWindow.h"

namespace tizen_browser
{

namespace base_ui
{
TextPopup* TextPopup::createPopup(Evas_Object* parent)
{
    TextPopup *raw_popup = new TextPopup(parent);
    return raw_popup;
}

TextPopup* TextPopup::createPopup(Evas_Object* parent,
                                  const std::string& title,
                                  const std::string& message)
{
    TextPopup *raw_popup = new TextPopup(parent, title, message);
    return raw_popup;
}

TextPopup::~TextPopup()
{
    buttonClicked.disconnect_all_slots();
    evas_object_del(m_layout);
}

TextPopup::TextPopup(Evas_Object* parent)
    : m_parent(parent)
    , m_layout(nullptr)
    , m_buttons_box(nullptr)
    , m_button_left(nullptr)
    , m_button_right(nullptr)
{
    m_edjFilePath = "edje/SimpleUI/TextPopup.edj";
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

TextPopup::TextPopup(Evas_Object* parent,
                     const std::string& title,
                     const std::string& message)
    : m_parent(parent)
    , m_layout(nullptr)
    , m_buttons_box(nullptr)
    , m_button_left(nullptr)
    , m_button_right(nullptr)
    , m_title(title)
    , m_message(message)
{
    m_edjFilePath = "edje/SimpleUI/TextPopup.edj";
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

void TextPopup::show()
{
    createLayout();
    popupShown(this);
}

void TextPopup::dismiss(){
    popupDismissed(this);
}

void TextPopup::onBackPressed(){
    dismiss();
}

void TextPopup::_left_response_cb(void* data,
                                  Evas_Object* /*obj*/,
                                  void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TextPopup *self = static_cast<TextPopup*>(data);
    self->buttonClicked(self->m_left_button_type);
    self->dismiss();
}

void TextPopup::_right_response_cb(void* data,
                                   Evas_Object* /*obj*/,
                                   void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TextPopup *self = static_cast<TextPopup*>(data);
    self->buttonClicked(self->m_right_button_type);
    self->dismiss();
}

void TextPopup::setTitle(const std::string& title)
{
    this->m_title = title;
}

void TextPopup::setMessage(const std::string& message)
{
    this->m_message = message;
}

void TextPopup::setLeftButton(const PopupButtons& button)
{
    this->m_left_button_type = button;
}

void TextPopup::setRightButton(const PopupButtons& button)
{
    this->m_right_button_type = button;
}

void TextPopup::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    m_layout = elm_layout_add(m_parent);
    elm_object_tree_focus_allow_set(m_layout, EINA_FALSE);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "text_popup_layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(m_layout, "title_text", m_title.c_str());
    elm_object_translatable_text_set(m_layout, m_message.c_str());

    m_buttons_box = elm_box_add(m_layout);
    elm_box_horizontal_set(m_buttons_box, EINA_TRUE);
    evas_object_size_hint_weight_set(m_buttons_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_buttons_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_button_left = elm_button_add(m_buttons_box);
    elm_object_style_set(m_button_left, "text-popup-button");
    evas_object_size_hint_weight_set(m_button_left, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_button_left, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(m_button_left, "elm.text", buttonsTranslations[m_left_button_type].c_str());
    elm_box_pack_end(m_buttons_box, m_button_left);
    evas_object_smart_callback_add(m_button_left, "clicked", _left_response_cb, (void*)this);

    evas_object_show(m_button_left);

    m_button_right = elm_button_add(m_buttons_box);
    elm_object_style_set(m_button_right, "text-popup-button");
    evas_object_size_hint_weight_set(m_button_right, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_button_right, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(m_button_right, "elm.text", buttonsTranslations[m_right_button_type].c_str());
    elm_box_pack_end(m_buttons_box, m_button_right);
    evas_object_smart_callback_add(m_button_right, "clicked", _right_response_cb, (void*)this);

    evas_object_show(m_button_right);
    elm_object_signal_emit(m_button_right, "visible", "ui");

    evas_object_show(m_buttons_box);
    elm_object_part_content_set(m_layout, "buttons_swallow", m_buttons_box);

    evas_object_show(m_layout);
    elm_object_part_content_set(m_parent, "popup_content", m_layout);
}
}

}
