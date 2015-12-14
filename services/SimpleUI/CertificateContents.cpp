#include "CertificateContents.h"
#include "BrowserLogger.h"

namespace tizen_browser
{

namespace base_ui
{

CertificateContents::CertificateContents()
    : m_mainLayout(NULL)
    , m_parent(NULL)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

CertificateContents::~CertificateContents()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void CertificateContents::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
}

Evas_Object* CertificateContents::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_mainLayout)
        m_mainLayout = createCertificateLayout();

    return m_mainLayout;
}

Evas_Object* CertificateContents::createCertificateLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string edjFilePath = EDJE_DIR;
    edjFilePath.append("SimpleUI/CertificatePopup.edj");
    elm_theme_extension_add(nullptr, edjFilePath.c_str());

    Evas_Object *popup_content = elm_layout_add(m_parent);
    elm_layout_file_set(popup_content, edjFilePath.c_str(), "certificate_popup");

    elm_layout_text_set(popup_content, "main_text", "There are problems with the security certificate for this site.");

    Evas_Object* button = elm_button_add(popup_content);
    elm_object_style_set(button, "basic_button");
    evas_object_smart_callback_add(button, "clicked", _view_certificate_clicked_cb, this);
    elm_object_part_content_set(popup_content, "view_cert_button", button);

    return popup_content;
}

void CertificateContents::_view_certificate_clicked_cb(void* /*data*/, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

}
}
