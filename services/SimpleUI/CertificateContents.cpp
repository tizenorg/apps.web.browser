#include "CertificateContents.h"
#include "BrowserLogger.h"

namespace tizen_browser
{

namespace base_ui
{

const std::string SECURITY_CERTIFICATE_WARNING = "There are problems with the security certificate for this site.";
const std::string SECURITY_CERTIFICATE_CONTENTS = "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. ";

CertificateContents::CertificateContents()
    : m_mainLayout(NULL)
    , m_parent(NULL)
    , m_viewCertButton(NULL)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SimpleUI/CertificatePopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
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

    Evas_Object* layout = elm_layout_add(m_parent);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "popup_content");

    Evas_Object *main_content = elm_layout_add(layout);
    elm_layout_file_set(main_content, m_edjFilePath.c_str(), "main_content");
    elm_layout_text_set(main_content, "main_text", SECURITY_CERTIFICATE_WARNING.c_str());

    m_viewCertButton = elm_button_add(main_content);
    elm_object_style_set(m_viewCertButton, "basic_button");
    evas_object_smart_callback_add(m_viewCertButton, "clicked", _view_certificate_clicked_cb, this);
    elm_object_part_content_set(main_content, "view_cert_button", m_viewCertButton);
    elm_layout_content_set(layout, "content_swallow", main_content);

    return layout;
}

void CertificateContents::_view_certificate_clicked_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    CertificateContents* cc = static_cast<CertificateContents*>(data);
    evas_object_hide(elm_object_part_content_get(cc->m_mainLayout, "content_swallow"));
    elm_object_part_content_unset(cc->m_mainLayout, "content_swallow");
    cc->m_popup->updateTitle("Certificate contents");
    cc->m_popup->enlarge();
    cc->populateCertificateContents();
}

void CertificateContents::populateCertificateContents()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object *scroller = elm_scroller_add(m_mainLayout);
    elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
    evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    Evas_Object *genlist = elm_genlist_add(scroller);
    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Elm_Genlist_Item_Class *item_class = elm_genlist_item_class_new();
    if (!item_class) {
            BROWSER_LOGE("elm_genlist_item_class_new for item_class failed");
            return;
    }
    item_class->item_style = "cert_content";
    item_class->func.text_get = __text_get_cb;
    item_class->func.content_get = NULL;
    item_class->func.state_get = NULL;
    item_class->func.del = NULL;
    elm_genlist_item_append(genlist, item_class, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    elm_object_content_set(scroller, genlist);
    elm_layout_content_set(m_mainLayout, "content_swallow", scroller);
}

char *CertificateContents::__text_get_cb(void* /*data*/, Evas_Object* /*obj*/, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("part[%s]", part);
    return strdup(SECURITY_CERTIFICATE_CONTENTS.c_str());
}

}
}
