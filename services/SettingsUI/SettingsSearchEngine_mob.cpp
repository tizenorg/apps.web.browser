
#include "SettingsSearchEngine_mob.h"

namespace tizen_browser{
namespace base_ui{

SettingsSearchEngine::SettingsSearchEngine(Evas_Object* parent){
    init(parent);
    updateButtonMap();
};

SettingsSearchEngine::~SettingsSearchEngine(){
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void SettingsSearchEngine::updateButtonMap() {
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData test;
    test.buttonText = "TEST";
    m_buttonsMap[0] = test;
}

bool SettingsSearchEngine::populateList(Evas_Object* genlist){
    elm_object_translatable_part_text_set(m_actionBar, "settings_title", "Default search engine");
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[0], _test_page_cb);
    appendGenlist(genlist, m_setting_item_class, &m_buttonsMap[0], _test_page_cb);
    return true;
}

void SettingsSearchEngine::_test_page_cb(void* /*data*/, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}
}
}
