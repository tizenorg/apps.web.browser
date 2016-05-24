/*
 * SettingsHomePage_mob.h
 *
 *  Created on: May 24, 2016
 *      Author: knowac
 */

#ifndef SETTINGS_HOME_PAGE_MOB_H_
#define SETTINGS_HOME_PAGE_MOB_H_

#include "SettingsUI_mob.h"

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

enum SettingsHomePageOptions {
    DEFAULT,
    CURRENT,
    QUICK_ACCESS,
    MOST_VIS,
    OTHER
};

class SettingsHomePage
    : public SettingsUI
{
public:
    SettingsHomePage(){};
    SettingsHomePage(Evas_Object* parent);
    std::string getCurrentPage();
    virtual ~SettingsHomePage();
    virtual bool populateList(Evas_Object* genlist);
    virtual void updateButtonMap();
    virtual void connectSignals(){};
    virtual void disconnectSignals(){};
    static void _default_cb(void *data, Evas_Object*obj , void* event_info);
    static void _current_cb(void *data, Evas_Object*obj , void* event_info);
    static void _quick_cb(void *data, Evas_Object*obj , void* event_info);
    static void _most_visited_cb(void *data, Evas_Object*obj , void* event_info);
    static void _other_cb(void* data, Evas_Object* obj, void* event_info);
    static const std::string DEF_HOME_PAGE;
private:
    std::map<SettingsHomePageOptions, Elm_Object_Item*> m_itemsMap;
    boost::optional<std::string> m_current;
};

}
}

#endif /* SETTINGS_HOME_PAGE_MOB_H_ */
