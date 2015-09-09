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

#ifndef SIMPLEURI_H
#define SIMPLEURI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>
#include "memory.h"

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "BasicUI/Action.h"
#include "BrowserImage.h"
#include "EflTools.h"


namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT SimpleURI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    enum IconType{
        IconTypeSearch
       ,IconTypeDoc
       ,IconTypeFav
    };
    SimpleURI();
    ~SimpleURI();
    Evas_Object *getContent(Evas_Object *main_layout);
    virtual std::string getName();

    Evas_Object * getContent() { return m_entry_layout;};

    void changeUri(const std::string);
    boost::signals2::signal<void (const std::string &)> uriChanged;

    void setFavIcon(std::shared_ptr<tizen_browser::tools::BrowserImage> favicon);
    void setCurrentFavIcon();
    void setSearchIcon();
    void setDocIcon();
    IconType getCurrentIconTyep();
    /**
     * \brief Adds Action to URI bar.
     *
     * All Actions will be displayed before URI entry.
     */
    void AddAction(sharedAction action);

    /**
     * \brief returns list of stored actions
     */
    std::list<sharedAction> actions() const;

    /**
     * \brief Sets Focus to URI entry.
     */
    void setFocus();

    /**
     * @brief Remove focus form URI
     */
    void clearFocus();

    /**
     * @brief check if URI is focused
     */
    bool hasFocus() const;

    void setDisabled(bool disabled);

private:
    static void activated(void *data, Evas_Object *obj, void *event_info);
    static void aborted(void *data, Evas_Object *obj, void *event_info);
    static void preeditChange(void *data, Evas_Object *obj, void *event_info);
    static void changedUser(void *data, Evas_Object *obj, void *event_info);
    static void focused(void *data, Evas_Object *obj, void *event_info);
    static void unfocused(void *data, Evas_Object *obj, void *event_info);
    static void fixed_entry_key_down_handler(void *data, Evas *e, Evas_Object *obj, void *event_info);
    static void _uriEntryBtnClicked(void *data, Evas_Object *obj, void *event_info);

    void editingCompleted();
    void editingCanceled();

    void selectWholeText();

    /**
     * \brief Rewrites URI to support search and prefixing http:// if needed
     */
    std::string rewriteURI(const std::string& url);

    static void _uriEntryClicked(void *data, Evas_Object *obj, void *event_info);

    static void __cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
    static void __cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
    static void focusedBtn(void *data, Evas_Object *obj, void *event_info);
    static void unfocusedBtn(void *data, Evas_Object *obj, void *event_info);

private:
    IconType m_currentIconType;
    std::list<sharedAction> m_actions;
    Evas_Object *m_entry;
    Evas_Object *m_favicon;
    Evas_Object *m_entry_layout;
    Evas_Object *m_entryBtn;
    bool m_entrySelectedAllFirst;
    std::string m_oryginalEntryText;
    bool m_searchTextEntered;
};


}
}

#endif // SIMPLEURI_H
