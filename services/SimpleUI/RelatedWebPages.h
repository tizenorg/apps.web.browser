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

#ifndef RELATEDWEBPAGES_H
#define RELATEDWEBPAGES_H

#include <Elementary.h>
#include "BookmarkItem.h"

#include <boost/signals2/signal.hpp>

namespace tizen_browser{
namespace base_ui{

class RelatedWebPages
{
public:
    RelatedWebPages();
    void init(Evas_Object* p);
    Evas_Object* getContent();
    void addRelatedPage(std::shared_ptr<tizen_browser::services::BookmarkItem> bi);
    void deleteAllItems();

    boost::signals2::signal<void (std::shared_ptr<tizen_browser::services::BookmarkItem>)> bookmarkClicked;

private:
    Evas_Object *m_parent, *m_gengrid;
    Elm_Gen_Item_Class *m_item_class;
    bool m_gengridSetup;

    static Evas_Object* _grid_content_get(void *data, Evas_Object *obj, const char *part);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);
    static void focusItem(void* data, Evas_Object* obj, void* event_info);
    static void unFocusItem(void* data, Evas_Object* obj, void* event_info);
};

}

}

#endif // RELATEDWEBPAGES_H
