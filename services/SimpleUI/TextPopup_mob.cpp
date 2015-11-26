#include "TextPopup_mob.h"
#include "ServiceManager.h"
#include "AbstractMainWindow.h"

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
    evas_object_del(m_left_button);
    evas_object_del(m_right_button);
    evas_object_del(m_layout);
}

TextPopup::TextPopup(Evas_Object* parent) : m_parent(parent)
{
    edjFilePath = EDJE_DIR;
    edjFilePath.append("SimpleUI/TextPopup.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
}

TextPopup::TextPopup(Evas_Object* parent,
                     const std::string& title,
                     const std::string& message)
    : m_parent(parent)
    , m_title(title)
    , m_message(message)
{
    edjFilePath = EDJE_DIR;
    edjFilePath.append("SimpleUI/TextPopup.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
}

void TextPopup::show()
{
    createLayout();
}

void TextPopup::_left_response_cb(void* data,
                                  Evas_Object* /*obj*/,
                                  void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TextPopup *self = static_cast<TextPopup*>(data);
    self->buttonClicked(PopupButtons::CANCEL);
    delete self;
}

void TextPopup::_right_response_cb(void* data,
                                   Evas_Object* /*obj*/,
                                   void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    TextPopup *self = static_cast<TextPopup*>(data);
    self->buttonClicked(PopupButtons::OK);
    delete self;
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
    this->m_left_button_text = button;
}

void TextPopup::setRightButton(const PopupButtons& button)
{
    this->m_right_button_text = button;
}

void TextPopup::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    m_layout = elm_layout_add(m_parent);
    elm_object_tree_focus_allow_set(m_layout, EINA_FALSE);
    elm_layout_file_set(m_layout, edjFilePath.c_str(), "text_popup_layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_layout_text_set(m_layout, "popup_title", m_title.c_str());
    elm_layout_text_set(m_layout, "popup_text", m_message.c_str());
    elm_layout_text_set(m_layout,
                        "ok_button_text",
                        buttonsTranslations[m_right_button_text].c_str());
    elm_layout_text_set(m_layout,
                        "cancel_button_text",
                        buttonsTranslations[m_left_button_text].c_str());

    m_right_button = elm_button_add(m_layout);
    elm_object_style_set(m_right_button, "invisible_button");
    evas_object_smart_callback_add(m_right_button, "clicked", _right_response_cb, this);
    elm_layout_content_set(m_layout, "ok_button_click", m_right_button);

    m_left_button = elm_button_add(m_layout);
    elm_object_style_set(m_left_button, "invisible_button");
    evas_object_smart_callback_add(m_left_button, "clicked", _left_response_cb, this);
    elm_layout_content_set(m_layout, "cancel_button_click", m_left_button);

    evas_object_show(m_layout);
}
}

}
