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

/*
 * BookmarkFlowUI.cpp
 *
 *  Created on: Nov 10, 2015
 *      Author: m.kawonczyk@samsung.com
 */

#ifndef BOOKMARKFLOWUI_H
#define BOOKMARKFLOWUI_H

#include <Evas.h>
#include <boost/signals2/signal.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "services/HistoryService/HistoryItem.h"
#include "BookmarkItem.h"
#include "FocusManager.h"

#define BOOKMARK_FLOW_SERVICE "org.tizen.browser.bookmarkflowui"
#define M_UNUSED(x) (void)(x)

namespace tizen_browser{
namespace base_ui{

class BROWSER_EXPORT BookmarkFlowUI
        : public tizen_browser::interfaces::AbstractUIComponent
        , public tizen_browser::core::AbstractService
{
public:
    BookmarkFlowUI();
    ~BookmarkFlowUI();
    //AbstractUIComponent interface methods
    void init(Evas_Object *parent);
    void showUI();
    void hideUI();
    Evas_Object *getContent();

    void setState(bool state);
    void setTitle(const std::string& title);
    void setURL(const std::string& title);
    virtual std::string getName();
    void hide();

private:
    Evas_Object* createBookmarkFlowLayout(Evas_Object* parent);
    void createTitleArea();
    void createContentsArea();

private:
    static void _save_clicked(void * data, Evas_Object *, void *);
    static void _cancel_clicked(void * data, Evas_Object *, void *);
    static void _entry_focused(void * data, Evas_Object *, void *);
    static void _entry_unfocused(void * data, Evas_Object *, void *);
    static void _inputCancel_clicked(void * data, Evas_Object *, void *);
    static void _folder_clicked(void * data, Evas_Object *, void *);
    static void _remove_clicked(void * data, Evas_Object *, void *);

    Evas_Object *m_removeButton;
    Evas_Object *m_entry;
    Evas_Object *m_saveButton;
    Evas_Object *m_cancelButton;
    Evas_Object *m_inputCancelButton;
    Evas_Object *m_folderButton;
    Evas_Object *m_titleArea;
    Evas_Object *m_contentsArea;
    Evas_Object *m_bf_layout;
    Evas_Object *m_parent;
    std::string m_edjFilePath;
    bool m_state;
};

}
}

#endif // BOOKMARKFLOWUI_H
