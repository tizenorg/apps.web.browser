#ifndef ADDNEWFOLDERPOPUP_H
#define ADDNEWFOLDERPOPUP_H

#include <Elementary.h>
#include <boost/signals2.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "BookmarkItem.h"



namespace tizen_browser{
namespace base_ui{

class AddNewFolderPopup{

public:
    AddNewFolderPopup(Evas_Object *main_layout);
    AddNewFolderPopup(Evas_Object *main_layout
                    , Evas_Object *content
                    , const std::string& message
                    , const std::string& title
                    , const std::string& okButtonText
                    , const std::string& cancelButtonText);

    /*
     * These setters should be called before showing popup.
     */
    void setContent(Evas_Object *content);
    void setMessage(const std::string &message);
    void setTitle(const std::string &title);
    void setOkButtonText(const std::string &okButtonText);
    void setCancelButtonText(const std::string &cancelButtonText);

    void show();
    void hide();
    boost::signals2::signal<void (Evas_Object *)> on_ok;
    boost::signals2::signal<void (Evas_Object *)> on_cancel;


private:
    static void popup_ok_cb(void *data, Evas_Object *, void*);
    static void popup_cancel_cb(void *data, Evas_Object *, void*);
    Evas_Object *m_popup;
    Evas_Object *m_cancel_button;
    Evas_Object *m_editfield_entry;
    Evas_Object *m_ok_button;
    Evas_Object *m_content;
    std::string m_message;
    std::string m_title;
    std::string m_okButtonText;
    std::string m_cancelButtonText;

protected:
    Evas_Object *m_mainLayout;
};

}
}
#endif
