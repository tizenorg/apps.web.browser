//#ifndef __CONTENT_POPUP_H__
//#define __CONTENT_POPUP_H__ 1

#include <Evas.h>
#include <Elementary.h>
#include <string>
#include <list>
#include <memory>

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
    static void _view_certificate_clicked_cb(void* data, Evas_Object* obj, void* event_info);

private:
    Evas_Object* m_mainLayout;
    Evas_Object* m_parent;
};

}
}
