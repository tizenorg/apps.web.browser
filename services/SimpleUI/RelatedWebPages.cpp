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

#include "RelatedWebPages.h"
#include "BrowserLogger.h"
#include "BrowserAssert.h"
#include "Tools/EflTools.h"

namespace tizen_browser{
namespace base_ui{

typedef struct _BookmarkItemData
{
    std::shared_ptr<tizen_browser::services::BookmarkItem> item;
    std::shared_ptr<tizen_browser::base_ui::RelatedWebPages> relatedWebPages;
} BookmarkItemData;

RelatedWebPages::RelatedWebPages() : m_parent(NULL), m_gengrid(NULL), m_item_class(NULL), m_gengridSetup(false)
{

}

void RelatedWebPages::init(Evas_Object* p)
{
    m_parent = p;
    m_gengrid = elm_gengrid_add(m_parent);

    evas_object_smart_callback_add(m_gengrid, "item,focused", focusItem, NULL);
    evas_object_smart_callback_add(m_gengrid, "item,unfocused", unFocusItem, NULL);

    if (!m_item_class) {
        m_item_class = elm_genlist_item_class_new();
        m_item_class->item_style = "rel_page_item";
        m_item_class->func.text_get = NULL;
        m_item_class->func.content_get =  _grid_content_get;
        m_item_class->func.state_get = NULL;
        m_item_class->func.del = NULL;
    }
}

void RelatedWebPages::addRelatedPage(std::shared_ptr<tizen_browser::services::BookmarkItem> bi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = bi;
    itemData->relatedWebPages = std::shared_ptr<tizen_browser::base_ui::RelatedWebPages>(this);
    /*Elm_Object_Item* bookmarkView =*/ elm_gengrid_item_append(m_gengrid, m_item_class, itemData, NULL, this);
    //m_map_bookmark_views.insert(std::pair<std::string,Elm_Object_Item*>(bi->getAddress(),bookmarkView));

    //setEmptyGengrid(false);
}

Evas_Object * RelatedWebPages::_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    BookmarkItemData *itemData = reinterpret_cast<BookmarkItemData*>(data);

    if (!strcmp(part, "elm.thumbnail")) {
    if (itemData->item->getThumbnail()) {
            Evas_Object * thumb = tizen_browser::tools::EflTools::getEvasImage(itemData->item->getThumbnail(), itemData->relatedWebPages->m_parent);
            return thumb;
        }
        else {
            return NULL;
        }
    }
    else if (!strcmp(part, "elm.label")) {
        Evas_Object *label = elm_label_add(obj);
        elm_object_style_set(label, "rel_pages_label");
        elm_object_text_set(label, itemData->item->getTittle().c_str());
        return label;
    }
    else if (!strcmp(part, "elm.thumbButton")) {
        Evas_Object *thumbButton = elm_button_add(obj);
        elm_object_style_set(thumbButton, "thumbButton");
        evas_object_smart_callback_add(thumbButton, "clicked", RelatedWebPages::_thumbSelected, data);
        return thumbButton;
       }
    return NULL;
}

Evas_Object * RelatedWebPages::getContent()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    M_ASSERT(m_parent);

    /*if(!elm_gengrid_items_count(m_gengrid))
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", createNoBookmarksLabel());*/

    if(!m_gengridSetup) {
        std::string edjFilePath = EDJE_DIR;
        edjFilePath.append("SimpleUI/RelatedPages.edj");
        elm_theme_extension_add(NULL, edjFilePath.c_str());


        elm_object_style_set(m_gengrid, "rel_pages");

        elm_gengrid_align_set(m_gengrid, 0, 0);
        elm_gengrid_item_size_set(m_gengrid, 252, 222);
        elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
        elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
        elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
        elm_gengrid_highlight_mode_set(m_gengrid, EINA_TRUE);
        elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
        elm_scroller_page_size_set(m_gengrid, 0, 327);

        evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

        m_gengridSetup = true;
    }
    return m_gengrid;
}

void RelatedWebPages::deleteAllItems()
{
    BROWSER_LOGD("Deleting all items from gengrid");
    elm_gengrid_clear(m_gengrid);
}

void RelatedWebPages::_thumbSelected(void * data, Evas_Object * /*obj*/, void * /*event_info*/)
{
    BookmarkItemData * itemData = reinterpret_cast<BookmarkItemData *>(data);
    itemData->relatedWebPages->bookmarkClicked(itemData->item);
}

void RelatedWebPages::focusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,in", "over2");
}

void RelatedWebPages::unFocusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,out", "over2");
}

}

}
