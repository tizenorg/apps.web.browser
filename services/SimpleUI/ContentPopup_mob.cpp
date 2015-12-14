#include "ContentPopup_mob.h"
#include "ServiceManager.h"
#include "AbstractMainWindow.h"

namespace tizen_browser
{

namespace base_ui
{
ContentPopup* ContentPopup::createPopup(Evas_Object* parent)
{
    ContentPopup *raw_popup = new ContentPopup(parent);
    return raw_popup;
}

ContentPopup* ContentPopup::createPopup(Evas_Object* parent,
                                  const std::string& title,
                                  Evas_Object* content)
{
    ContentPopup *raw_popup = new ContentPopup(parent, title, content);
    return raw_popup;
}

ContentPopup::~ContentPopup()
{
    buttonClicked.disconnect_all_slots();
    evas_object_del(m_left_button);
    evas_object_del(m_middle_button);
    evas_object_del(m_right_button);
    evas_object_del(m_layout);
}

ContentPopup::ContentPopup(Evas_Object* parent)
    : m_parent(parent)
    , m_button_count(2)
{
    edjFilePath = EDJE_DIR;
    edjFilePath.append("SimpleUI/ContentPopup.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
}

ContentPopup::ContentPopup(Evas_Object* parent,
                     const std::string& title,
                     Evas_Object* content)
    : m_parent(parent)
    , m_title(title)
    , m_content(content)
    , m_button_count(2)
{
    edjFilePath = EDJE_DIR;
    edjFilePath.append("SimpleUI/ContentPopup.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
}

void ContentPopup::show()
{
    createLayout();
    popupShown(this);
}

void ContentPopup::dismiss(){
    popupDismissed(this);
}

void ContentPopup::onBackPressed(){
    dismiss();
}

void ContentPopup::_middle_response_cb(void* data,
                                   Evas_Object* /*obj*/,
                                   void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ContentPopup *self = static_cast<ContentPopup*>(data);
    self->buttonClicked(self->m_middle_button_type);
}

void ContentPopup::_left_response_cb(void* data,
                                  Evas_Object* /*obj*/,
                                  void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ContentPopup *self = static_cast<ContentPopup*>(data);
    self->buttonClicked(self->m_left_button_type);
}

void ContentPopup::_right_response_cb(void* data,
                                   Evas_Object* /*obj*/,
                                   void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ContentPopup *self = static_cast<ContentPopup*>(data);
    self->buttonClicked(self->m_right_button_type);
}

void ContentPopup::setTitle(const std::string& title)
{
    this->m_title = title;
}

void ContentPopup::updateTitle(const std::string& title)
{
    this->m_title = title;
    if (m_layout)
        elm_layout_text_set(m_layout, "popup_title", m_title.c_str());
}

void ContentPopup::setContent(Evas_Object* content)
{
    this->m_content = content;
}

void ContentPopup::setButtonCount(unsigned count)
{
    this->m_button_count = count;
}

void ContentPopup::setLeftButton(const PopupButtons& button)
{
    this->m_left_button_type = button;
}

void ContentPopup::setMiddleButton(const PopupButtons& button)
{
    this->m_middle_button_type = button;
}

void ContentPopup::setRightButton(const PopupButtons& button)
{
    this->m_right_button_type = button;
}

void ContentPopup::enlarge()
{
    elm_object_signal_emit(m_layout, "enlarge,popup", "");
    elm_layout_content_set(m_layout, "button_bar", createButtonBarLayout(m_layout, m_button_count));
}

void ContentPopup::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    m_layout = elm_layout_add(m_parent);
    elm_object_tree_focus_allow_set(m_layout, EINA_FALSE);
    elm_layout_file_set(m_layout, edjFilePath.c_str(), "content_popup_layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_layout_text_set(m_layout, "popup_title", m_title.c_str());
    elm_layout_content_set(m_layout, "popup_content", m_content);

    m_button_bar_layout = createButtonBarLayout(m_layout, m_button_count);
    elm_layout_content_set(m_layout, "button_bar", m_button_bar_layout);

    evas_object_show(m_layout);
}

Evas_Object* ContentPopup::createButtonBarLayout(Evas_Object* parent, unsigned buttonCount)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    char buf[15];
    sprintf(buf, "%d_button_bar", buttonCount);

    Evas_Object* layout = elm_layout_add(parent);
    elm_object_tree_focus_allow_set(layout, EINA_FALSE);
    elm_layout_file_set(layout, edjFilePath.c_str(), buf);

    switch(buttonCount) {
        case 3:
        case 2:
        {
           elm_layout_text_set(layout,
                   "right_button_text",
                   buttonsTranslations[m_right_button_type].c_str());
           elm_layout_text_set(layout,
                   "left_button_text",
                   buttonsTranslations[m_left_button_type].c_str());

           m_right_button = elm_button_add(layout);
           elm_object_style_set(m_right_button, "invisible_button");
           evas_object_smart_callback_add(m_right_button, "clicked", _right_response_cb, this);
           elm_layout_content_set(layout, "right_button_click", m_right_button);

           m_left_button = elm_button_add(layout);
           elm_object_style_set(m_left_button, "invisible_button");
           evas_object_smart_callback_add(m_left_button, "clicked", _left_response_cb, this);
           elm_layout_content_set(layout, "left_button_click", m_left_button);
           if (buttonCount == 2)
               break;
        }
        case 1:
        {
           elm_layout_text_set(layout,
                   "middle_button_text",
                   buttonsTranslations[m_middle_button_type].c_str());

           m_middle_button = elm_button_add(layout);
           elm_object_style_set(m_middle_button, "invisible_button");
           evas_object_smart_callback_add(m_middle_button, "clicked", _middle_response_cb, this);
           elm_layout_content_set(layout, "middle_button_click", m_middle_button);
           break;
        }
    }
    return layout;
}

}
}
