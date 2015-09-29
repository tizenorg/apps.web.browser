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

#include <Elementary.h>
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "BookmarkManagerUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(BookmarkManagerUI, "org.tizen.browser.bookmarkmanagerui")

struct ItemData
{
    tizen_browser::base_ui::BookmarkManagerUI * m_bookmarkManager;
    tizen_browser::services::BookmarkItem * h_item;
    Elm_Object_Item * e_item;
};

typedef struct
{
    std::shared_ptr<tizen_browser::services::BookmarkItem> item;
    std::shared_ptr<tizen_browser::base_ui::BookmarkManagerUI> bookmarkManagerUI;
} BookmarkItemData;

BookmarkManagerUI::BookmarkManagerUI()
    : m_genList(nullptr)
    , b_mm_layout(nullptr)
    , m_itemClass(nullptr)
    , m_gengrid(nullptr)
    , m_parent(nullptr)
    , m_bookmark_item_class(nullptr)
    , m_gengridSetup(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    edjFilePath = EDJE_DIR;
    edjFilePath.append("BookmarkManagerUI/BookmarkManagerUI.edj");
    createGengridItemClasses();
}

BookmarkManagerUI::~BookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_bookmark_item_class)
        elm_gengrid_item_class_free(m_bookmark_item_class);
}

void BookmarkManagerUI::createGengridItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_bookmark_item_class = elm_gengrid_item_class_new();
    m_bookmark_item_class->item_style = "grid_ds_item";
    m_bookmark_item_class->func.text_get = _grid_bookmark_text_get;
    m_bookmark_item_class->func.content_get =  _grid_bookmark_content_get;
    m_bookmark_item_class->func.state_get = nullptr;
    m_bookmark_item_class->func.del = nullptr;
}

void BookmarkManagerUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

void BookmarkManagerUI::showUI()
{
   evas_object_show(b_mm_layout);
}

void BookmarkManagerUI::hideUI()
{
    evas_object_hide(b_mm_layout);
    elm_gengrid_clear(m_gengrid);
    m_map_bookmark.clear();
}

Evas_Object* BookmarkManagerUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!b_mm_layout)
      b_mm_layout = createBookmarksLayout(m_parent);
    return b_mm_layout;
}

Evas_Object* BookmarkManagerUI::createBookmarksLayout(Evas_Object* parent)
{
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    b_mm_layout = elm_layout_add(parent);
    elm_layout_file_set(b_mm_layout, edjFilePath.c_str(), "bookmarkmanager-layout");
    evas_object_size_hint_weight_set(b_mm_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(b_mm_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(b_mm_layout);

    createGenGrid();
    showTopContent();
    return b_mm_layout;
}

//TODO: Make parend the argument and return created object to make code more modular.
//      (After fixing window managment)
void BookmarkManagerUI::createGenGrid()
{
    m_gengrid = elm_gengrid_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid", m_gengrid);
    elm_object_style_set(m_gengrid, "back_ground");
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(m_gengrid, EINA_TRUE);
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
    elm_gengrid_item_size_set(m_gengrid, 404 * efl_scale, 320 * efl_scale);
}

void BookmarkManagerUI::showTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    m_genList = elm_genlist_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "elm.swallow.genlist", m_genList);
    elm_genlist_homogeneous_set(m_genList, EINA_FALSE);
    elm_genlist_multi_select_set(m_genList, EINA_FALSE);
    elm_genlist_select_mode_set(m_genList, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genList, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genList, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    m_itemClass = elm_genlist_item_class_new();
    m_itemClass->item_style = "topContent";
    m_itemClass->func.text_get = &listItemTextGet;
    m_itemClass->func.content_get = &listItemContentGet;
    m_itemClass->func.state_get = 0;
    m_itemClass->func.del = 0;

    ItemData * id = new ItemData;
    id->m_bookmarkManager = this;
    Elm_Object_Item* elmItem = elm_genlist_item_append(m_genList,            //genlist
                                                      m_itemClass,           //item Class
                                                      id,
                                                      nullptr,               //parent item
                                                      ELM_GENLIST_ITEM_NONE, //item type
                                                      nullptr,
                                                      nullptr                //data passed to above function
                                                     );

    id->e_item = elmItem;
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

Evas_Object* BookmarkManagerUI::listItemContentGet(void* data, Evas_Object* obj, const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if ((data != nullptr) && (obj != nullptr) && (part != nullptr))
    {
        const char *part_name = "close_click";
        static const int part_name_len = strlen(part_name);
        ItemData * id = static_cast<ItemData *>(data);

        if(!strncmp(part_name, part, part_name_len))
        {
            Evas_Object *close_click = elm_button_add(obj);
            elm_object_style_set(close_click, "hidden_button");
            evas_object_smart_callback_add(close_click, "clicked", BookmarkManagerUI::close_clicked_cb, id);
            return close_click;
        }
    }
    return nullptr;
}

void BookmarkManagerUI::close_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        ItemData * id = static_cast<ItemData *>(data);
        id->m_bookmarkManager->closeBookmarkManagerClicked();
    }
}

char* BookmarkManagerUI::listItemTextGet(void* data, Evas_Object*, const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    return strdup("Bookmark");
}

Evas_Object * BookmarkManagerUI::getGenList()
{
    return m_genList;
}

Evas_Object * BookmarkManagerUI::getGenGrid()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    return m_gengrid;
}

void BookmarkManagerUI::setEmptyGengrid(bool setEmpty)
{
    if(setEmpty) {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", createNoHistoryLabel());
    } else {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", nullptr);
    }
}

Evas_Object* BookmarkManagerUI::createNoHistoryLabel()
{
    Evas_Object *label = elm_label_add(m_parent);
    if (label != nullptr)
    {
        elm_object_text_set(label, "No favorite websites.");
        evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    }
    return label;
}

void BookmarkManagerUI::addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = hi;
    itemData->bookmarkManagerUI.reset(this);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_bookmark_item_class, itemData, _bookmarkItemClicked, itemData);
    m_map_bookmark.insert(std::pair<std::string,Elm_Object_Item*>(hi->getAddress(),BookmarkView));
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
    setEmptyGengrid(false);
}

void BookmarkManagerUI::addBookmarkItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
     BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
     for (auto it = items.begin(); it != items.end(); ++it)
     {
         addBookmarkItem(*it);
     }
     elm_object_part_content_set(b_mm_layout, "elm.swallow.grid",getGenGrid());
     evas_object_show(getGenGrid());
}

char* BookmarkManagerUI::_grid_bookmark_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr))
    {
        BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
        BookmarkItemData *itemData = static_cast<BookmarkItemData*>(data);
        const char *part_name1 = "page_title";
        const char *part_name2 = "page_url";
        static const int part_name1_len = strlen(part_name1);
        static const int part_name2_len = strlen(part_name2);
        if (!strncmp(part_name1, part, part_name1_len))
        {
            return strdup(itemData->item->getTittle().c_str());
        }
        else if (!strncmp(part_name2, part, part_name2_len))
        {
            return strdup(itemData->item->getAddress().c_str());
        }
    }
    return strdup("");
}

Evas_Object * BookmarkManagerUI::_grid_bookmark_content_get(void *data, Evas_Object *obj, const char *part)
{
    if ((data != nullptr) && (obj != nullptr) && (part != nullptr))
    {
        BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
        BookmarkItemData *itemData = static_cast<BookmarkItemData*>(data);
        const char *part_name1 = "elm.thumbnail";
        const char *part_name2 = "elm.thumbButton";
        static const int part_name1_len = strlen(part_name1);
        static const int part_name2_len = strlen(part_name2);
        if (!strncmp(part_name1, part, part_name1_len))
        {
            std::shared_ptr<tizen_browser::tools::BrowserImage> image = itemData->item->getThumbnail();
            if (image)
            {
                return tizen_browser::tools::EflTools::getEvasImage(image, itemData->bookmarkManagerUI->m_parent);
            }
            else
            {
                return nullptr;
            }
        }
        else if (!strncmp(part_name2, part, part_name2_len))
        {
            Evas_Object *thumbButton = elm_button_add(obj);
            if (thumbButton != nullptr)
            {
                elm_object_style_set(thumbButton, "thumbButton");
            }
            return thumbButton;
        }
    }
    return nullptr;
}

void BookmarkManagerUI::_bookmarkItemClicked(void * data, Evas_Object *, void * event_info)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data != nullptr)
    {
        BookmarkItemData * itemData = static_cast<BookmarkItemData*>(data);
        BROWSER_LOGD("Bookmark URL: %s" , itemData->item->getAddress().c_str());
        itemData->bookmarkManagerUI->bookmarkItemClicked(itemData->item);
    }
}

}
}
