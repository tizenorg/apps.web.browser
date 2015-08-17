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
    void show(Evas_Object *main_layout);
    virtual std::string getName();
    void showActionBar();
    void showSettingsGenlist();
    void clearItems();
    void hide();

    boost::signals2::signal<void (const std::string & )> resetBrowserClicked;
    boost::signals2::signal<void (const std::string & )> resetMostVisitedClicked;
    boost::signals2::signal<void (const std::string & )> deleteSelectedDataClicked;
    boost::signals2::signal<void (const std::string & )> closeSettingsUIClicked;

private:
    static Evas_Object* listActionBarContentGet(void *data, Evas_Object *obj, const char *part);
    static Evas_Object* listSettingsGenlistContentGet(void *data, Evas_Object *obj, const char *part);

    static char* _grid_text_get(void *data, Evas_Object *obj, const char *part);
    static Evas_Object * _tab_grid_content_get(void *data, Evas_Object *obj, const char *part);
    static void _itemSelected(void * data, Evas_Object * obj, void * event_info);
    static void close_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void __check_changed_cb(void *data, Evas_Object *obj, void *event_info);

    static void _del_selected_data_clicked_cb(void * data, Evas_Object * obj, void * event_info);
    static void _reset_mv_clicked_cb(void * data, Evas_Object * obj, void * event_info);
    static void _reset_browser_clicked_cb(void * data, Evas_Object * obj, void * event_info);
    static void _closetabs_clicked(void * data, Evas_Object * obj, void * event_info);
    static void _onotherdevices_clicked(void * data, Evas_Object * obj, void * event_info);

private:
    Evas_Object *m_settings_layout;
    Evas_Object *m_genListActionBar;
    Evas_Object *m_genList;
    Elm_Genlist_Item_Class *m_itemClassActionBar;
    Evas_Object *m_parent;
    
    Elm_Gengrid_Item_Class * m_item_class;
    std::string m_edjFilePath;
};

}
}

#endif // BOOKMARKSUI_H
