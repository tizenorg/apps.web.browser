#include "SimplePopup.h"
#include "ServiceManager.h"
#include "AbstractMainWindow.h"
namespace tizen_browser
{

namespace base_ui
{
    SimplePopup* SimplePopup::createPopup()
    {
        SimplePopup *raw_popup = new SimplePopup();
        return raw_popup;
    }

    SimplePopup* SimplePopup::createPopup(const std::string &title, const std::string &message)
    {
        SimplePopup *raw_popup = new SimplePopup(title, message);
        return raw_popup;
    }

    SimplePopup::~SimplePopup()
    {
        evas_object_del(popup);
    }

    SimplePopup::SimplePopup() : content(nullptr) { }

    SimplePopup::SimplePopup(const std::string &title, const std::string &message)
	: content(nullptr)
	, title(title)
	, message(message)
    { }
    void SimplePopup::show()
    {
       std::shared_ptr<tizen_browser::base_ui::AbstractMainWindow<Evas_Object>> mainUi =
        std::dynamic_pointer_cast
        <
            tizen_browser::base_ui::AbstractMainWindow<Evas_Object>,
            tizen_browser::core::AbstractService
        >
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.simpleui"));

        popup = elm_popup_add(mainUi->getMainWindow().get());
        evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

        if(content != nullptr)
            elm_object_content_set(popup, content);
        else
            elm_object_text_set(popup, message.c_str());

        elm_popup_content_text_wrap_type_set(popup, ELM_WRAP_WORD);
#if MERGE_ME
        elm_popup_content_text_wrap_type_set(popup, ELM_WRAP_CHAR);
#endif
        elm_object_part_text_set(popup, "title,text", title.c_str());

        int buttonsCounter = 1;
        for(std::list<PopupButtons>::iterator it = buttons.begin(); it != buttons.end(); ++it)
        {
            Evas_Object *btn1 = elm_button_add(popup);
            elm_object_text_set(btn1, buttonsTranslations[*it].c_str());
            std::string buttonName = "button";
            buttonName.append(std::to_string(buttonsCounter));
            elm_object_part_content_set(popup, buttonName.c_str(), btn1);
            addedButtons[btn1] = *it;
            evas_object_smart_callback_add(btn1, "clicked", _response_cb, this);
            ++buttonsCounter;
        }

        evas_object_show(popup);
    }

    void SimplePopup::_response_cb(void *data, Evas_Object *obj, void */*event_info*/)
    {
        SimplePopup *self = static_cast<SimplePopup*>(data);
        self->buttonClicked(self->addedButtons[obj], self->popupData);
        delete self;
    }

    void SimplePopup::setTitle(const std::string &title)
    {
        this->title = title;
    }

    void SimplePopup::setMessage(const std::string &message)
    {
        this->message = message;
    }

    void SimplePopup::setContent(Evas_Object* content)
    {
        this->content = content;
    }

    void SimplePopup::setData(std::shared_ptr< PopupData > popupData)
    {
        this->popupData = popupData;
    }

    void SimplePopup::addButton(PopupButtons buttonId)
    {
        buttons.push_back(buttonId);
    }
}

}
