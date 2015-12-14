//#ifndef __CONTENT_POPUP_H__
//#define __CONTENT_POPUP_H__ 1

#include <Evas.h>
#include <Elementary.h>
#include <string>
#include <list>
#include <memory>
#include "ContentPopup_mob.h"

namespace tizen_browser
{

namespace base_ui
{

class CertificateContents
{
public:
    CertificateContents();
    ~CertificateContents();

    void init(Evas_Object* parent);
    Evas_Object* getContent();
    Evas_Object* createCertificateLayout();
    void populateCertificateContents();
    static void _view_certificate_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static char* __text_get_cb(void* /*data*/, Evas_Object* /*obj*/, const char *part);
    void setOwnerPopup(ContentPopup* popup) { m_popup = popup; }

private:
    Evas_Object* m_mainLayout;
    Evas_Object* m_parent;
    Evas_Object* m_viewCertButton;
    ContentPopup* m_popup;
    std::string m_edjFilePath;
};

}
}
