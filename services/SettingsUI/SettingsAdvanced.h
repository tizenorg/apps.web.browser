/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#ifndef SETTINGSADVANCED_MOB_H_
#define SETTINGSADVANCED_MOB_H_

#include "SettingsUI.h"

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <Evas.h>
#include "BrowserLogger.h"
#include "Tools/EflTools.h"

namespace tizen_browser{
namespace base_ui{

enum SettingsAdvancedOptions {
    ENABLE_JS,
    BLOCK_POPUPS,
    SAVE_CONTENT,
    MANAGE_WEB_DATA
};

class SettingsAdvanced
    : public SettingsUI
{
public:
    SettingsAdvanced(){};
    SettingsAdvanced(Evas_Object* parent);
    virtual ~SettingsAdvanced();
    virtual bool populateList(Evas_Object* genlist);
    virtual void updateButtonMap();
    Evas_Object* createOnOffCheckBox(Evas_Object* obj, ItemData*);
    Eina_Bool getOriginalState(int id);
    static void _enable_js_cb(void *data, Evas_Object*obj , void* event_info);
    static void _block_popups_cb(void *data, Evas_Object*obj , void* event_info);
    static void _save_content_cb(void *data, Evas_Object*obj , void* event_info);
    static void _manage_web_data_cb(void *data, Evas_Object*obj , void* event_info);
    static void grid_item_check_changed(void *data, Evas_Object *obj, void *event_info);
};

}
}

#endif /* SETTINGSADVANCED_MOB_H_ */
