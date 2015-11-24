/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SETTINGSUI_H
#define SETTINGSUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT SettingsUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    SettingsUI();
    ~SettingsUI();
    void init(Evas_Object* parent);
    Evas_Object* getContent();
    void showUI();
    void hideUI();
    void onBackKey();
    bool isSubpage();
    virtual std::string getName();
    Evas_Object* createActionBar(Evas_Object* settings_layout);
    Evas_Object* createBackActionBar(Evas_Object* settings_layout);
    Evas_Object* createSettingsMobilePage(Evas_Object* settings_layout);
    Evas_Object* createDelDataMobilePage(Evas_Object* settings_layout);
    Evas_Object* createRemoveMostVisitedMobilePage(Evas_Object* settings_layout);
    Evas_Object* createRemoveBrowserDataMobilePage(Evas_Object* settings_layout);

    boost::signals2::signal<void ()> resetBrowserClicked;
    boost::signals2::signal<void ()> resetMostVisitedClicked;
    boost::signals2::signal<void (std::string&)> deleteSelectedDataClicked;
    boost::signals2::signal<void ()> closeSettingsUIClicked;

private:
    Evas_Object* createSettingsUILayout(Evas_Object* parent);
    void resetItemsLayoutContent();
//    void onBackKey();
//    bool isSubpage();
    static void close_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void back_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void __checkbox_label_click_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

    static void _del_selected_data_clicked_cb(void * data, Evas_Object * obj, void * event_info);
    static void _del_selected_data_menu_clicked_cb(void * data, Evas_Object * obj, void * event_info);

    static void _reset_mv_clicked_cb(void * data, Evas_Object * obj, void * event_info);
    static void _reset_mv_menu_clicked_cb(void * data, Evas_Object * obj, void * event_info);

    static void _reset_browser_clicked_cb(void * data, Evas_Object * obj, void * event_info);
    static void _reset_browser_menu_clicked_cb(void * data, Evas_Object * obj, void * event_info);

    Evas_Object *m_settings_layout;
    Evas_Object *m_actionBar;
    Evas_Object* m_subpage_layout;
    Evas_Object *m_items_layout;
    Evas_Object *m_parent;

    std::string m_edjFilePath;
};

}
}

#endif // BOOKMARKSUI_H
