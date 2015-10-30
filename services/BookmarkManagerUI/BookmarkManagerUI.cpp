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

#include "AddNewFolderPopup.h"
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
} BookmarkItemData, BookmarkFolderItemData;

BookmarkManagerUI::BookmarkManagerUI()
    : m_genList(nullptr)
    , popup_content(nullptr)
    , b_mm_layout(nullptr)
    , m_itemClass(nullptr)
    , m_gengrid(nullptr)
    , m_parent(nullptr)
    , m_item_class(nullptr)
    , m_detail_item_class(nullptr)
    , m_gengridSetup(false)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    edjFilePath = EDJE_DIR;
    edjFilePath.append("BookmarkManagerUI/BookmarkManagerUI.edj");
}

BookmarkManagerUI::~BookmarkManagerUI()
{
}

void BookmarkManagerUI::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
    m_folder.clear();
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    b_mm_layout = elm_layout_add(parent);
    elm_layout_file_set(b_mm_layout, edjFilePath.c_str(), "bookmarkmanager-layout");
    evas_object_size_hint_weight_set(b_mm_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(b_mm_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(b_mm_layout);

    m_gengrid = elm_gengrid_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid", m_gengrid);

    elm_object_style_set(m_gengrid, "back_ground");
    evas_object_smart_callback_add(m_gengrid, "item,focused", focusItem, nullptr);
    evas_object_smart_callback_add(m_gengrid, "item,unfocused", unFocusItem, nullptr);
    evas_object_smart_callback_add(m_gengrid, "activated", _itemSelected, this);

      if (!m_item_class) {
            m_item_class = elm_gengrid_item_class_new();
            m_item_class->item_style = "grid_bm_item";
            m_item_class->func.text_get = _grid_text_get;
            m_item_class->func.content_get =  _grid_content_get;
            m_item_class->func.state_get = nullptr;
            m_item_class->func.del = nullptr;
        }

      if (!m_detail_item_class) {
            m_detail_item_class = elm_gengrid_item_class_new();
            m_detail_item_class->item_style = "grid_ds_item";
            m_detail_item_class->func.text_get = _grid_bookmark_text_get;
            m_detail_item_class->func.content_get =  _grid_bookmark_content_get;
            m_detail_item_class->func.state_get = nullptr;
            m_detail_item_class->func.del = nullptr;
        }

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
        const char *part_name1 = "new_folder_click";
        const char *part_name2 = "close_click";
        static const int part_name1_len = strlen(part_name1);
        static const int part_name2_len = strlen(part_name2);
        ItemData * id = static_cast<ItemData *>(data);
        if(!strncmp(part_name1, part, part_name1_len))
        {
            Evas_Object *new_folder_click = elm_button_add(obj);
            elm_object_style_set(new_folder_click, "hidden_button");
            evas_object_smart_callback_add(new_folder_click, "clicked", BookmarkManagerUI::new_folder_clicked_cb, id);
            return new_folder_click;
        }
        if(!strncmp(part_name2, part, part_name2_len))
        {
            Evas_Object *close_click = elm_button_add(obj);
            elm_object_style_set(close_click, "hidden_button");
            evas_object_smart_callback_add(close_click, "clicked", BookmarkManagerUI::close_clicked_cb, id);
            return close_click;
        }
    }
    return nullptr;
}

void BookmarkManagerUI::item_clicked_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void BookmarkManagerUI::close_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        ItemData * id = static_cast<ItemData *>(data);
        id->m_bookmarkManager->closeBookmarkManagerClicked(std::string());
        id->m_bookmarkManager->clearItems();
    }
}

void BookmarkManagerUI::new_folder_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        ItemData * id = static_cast<ItemData *>(data);
        id->m_bookmarkManager->newFolderPopup();
    }
}

void BookmarkManagerUI::newFolderPopup()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_popup.reset(new AddNewFolderPopup(m_parent, nullptr,"Add New Folder?","New Folder","Ok","Cancel"));
    m_popup->on_ok.disconnect_all_slots();
    m_popup->on_ok.connect(boost::bind(&BookmarkManagerUI::NewFolderCreate, this, _1));
    m_popup->on_cancel.disconnect_all_slots();
    m_popup->on_cancel.connect(boost::bind(&BookmarkManagerUI::CancelClicked, this, _1));
    m_popup->show();
}

void BookmarkManagerUI::NewFolderCreate(Evas_Object * popup_content)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (popup_content != nullptr)
    {
        m_folderName = elm_entry_entry_get(popup_content);
        saveFolderClicked(m_folderName.c_str(), 0,0);
        m_popup->hide();
    }
}

void BookmarkManagerUI::CancelClicked(Evas_Object * popup)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (popup != nullptr)
    {
        m_popup->hide();
    }
}

char* BookmarkManagerUI::listItemTextGet(void* data, Evas_Object*, const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if ((data != nullptr) && (part != nullptr))
    {
        const char *part_name = "folder_text";
        static const int part_name_len = strlen(part_name);
        ItemData * id = static_cast<ItemData *>(data);
        if(!strncmp(part_name, part, part_name_len))
        {
            if(!id->m_bookmarkManager->m_folder.empty())
            {
                std::string s = std::string("Bookmark > ") + id->m_bookmarkManager->m_folder;
                return strdup(s.c_str());
            }
        }
    }
    return strdup("Bookmark");
}

void BookmarkManagerUI::hide()
{
    evas_object_hide(elm_layout_content_get(b_mm_layout, "elm.swallow.grid"));
    evas_object_hide(elm_layout_content_get(b_mm_layout, "elm.swallow.genlist"));
    evas_object_hide(b_mm_layout);
}

Evas_Object * BookmarkManagerUI::getGenList()
{
    return m_genList;
}

Evas_Object * BookmarkManagerUI::getContent()
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

void BookmarkManagerUI::addBookmarkFolderItem(std::shared_ptr<tizen_browser::services::BookmarkItem> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    BookmarkFolderItemData *itemData = new BookmarkFolderItemData();
    itemData->item = hi;
    itemData->bookmarkManagerUI.reset(this);
    Elm_Object_Item* BookmarkFolderView = elm_gengrid_item_append(m_gengrid, m_item_class, itemData, nullptr, this);
    m_map_bookmark_folder_views.insert(std::pair<std::string,Elm_Object_Item*>(hi->getAddress(),BookmarkFolderView));
    elm_gengrid_item_selected_set(BookmarkFolderView, EINA_FALSE);
    setEmptyGengrid(false);
}

void BookmarkManagerUI::addBookmarkFolderItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
     BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
     for (auto it = items.begin(); it != items.end(); ++it)
     {
         addBookmarkFolderItem(*it);
     }
     elm_object_part_content_set(b_mm_layout, "elm.swallow.grid",getContent());
     evas_object_show(getContent());
}


void BookmarkManagerUI::addBookmarkItem(std::shared_ptr<tizen_browser::services::BookmarkItem> hi)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = hi;
    itemData->bookmarkManagerUI.reset(this);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_detail_item_class, itemData, nullptr, this);
    m_map_bookmark_folder_views.insert(std::pair<std::string,Elm_Object_Item*>(hi->getAddress(),BookmarkView));
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
     elm_object_part_content_set(b_mm_layout, "elm.swallow.grid",getContent());
     evas_object_show(getContent());
}

char* BookmarkManagerUI::_grid_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr))
    {
        BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
        BookmarkFolderItemData *itemData = static_cast<BookmarkFolderItemData*>(data);
        const char *part_name1 = "page_title";
        const char *part_name2 = "page_url";
        static const int part_name1_len = strlen(part_name1);
        static const int part_name2_len = strlen(part_name2);

        if (!strncmp(part_name1, part, part_name1_len) && !itemData->item->getTittle().empty())
        {
            return strdup(itemData->item->getTittle().c_str());
        }
        if (!strncmp(part_name2, part, part_name2_len) && !itemData->item->getAddress().empty())
        {
            return strdup(itemData->item->getAddress().c_str());
        }
    }
    return strdup("");
}

Evas_Object * BookmarkManagerUI::_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    if ((data != nullptr) && (obj != nullptr) && (part != nullptr))
    {
        BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
        BookmarkFolderItemData *itemData = static_cast<BookmarkFolderItemData*>(data);
        const char *part_name1 = "elm.thumbnail";
        const char *part_name2 = "elm.thumbButton";
        static const int part_name1_len = strlen(part_name1);
        static const int part_name2_len = strlen(part_name2);
        if (!strncmp(part_name1, part, part_name1_len))
        {
            Evas_Object * thumb = nullptr;
            std::shared_ptr<tizen_browser::tools::BrowserImage> image = itemData->item->getThumbnail();
            if (image)
            {
                thumb = tizen_browser::tools::EflTools::getEvasImage(image, itemData->bookmarkManagerUI->m_parent);
            }
            return thumb;
        }
        else if (!strncmp(part_name2, part, part_name2_len))
        {
            Evas_Object *thumbButton = elm_button_add(obj);
            if (thumbButton != nullptr)
            {
                elm_object_style_set(thumbButton, "thumbButton");
                evas_object_smart_callback_add(thumbButton, "clicked", tizen_browser::base_ui::BookmarkManagerUI::_thumbSelected, data);
            }
            return thumbButton;
        }
    }
    return nullptr;
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
                evas_object_smart_callback_add(thumbButton, "clicked", tizen_browser::base_ui::BookmarkManagerUI::_bookmark_thumbSelected, data);
            }
            return thumbButton;
        }
    }
    return nullptr;
}

void BookmarkManagerUI::_itemSelected(void * data, Evas_Object *, void * event_info)
{
    (void)data;
    (void)event_info;
#if 0
    if ((data != nullptr) && (event_info != nullptr))
    {
        Elm_Object_Item * selected = static_cast<Elm_Object_Item *>(event_info);
        BookmarkManagerUI * self = static_cast<BookmarkManagerUI *>(data);
        HistoryItemData * itemData = static_cast<HistoryItemData *>(elm_object_item_data_get(selected));
        if (itemData != nullptr)
        {
            self->bookmarkClicked(itemData->item);
        }
    }
#endif
}

void BookmarkManagerUI::_thumbSelected(void * data, Evas_Object *, void *)
{
    if (data != nullptr)
    {
        BookmarkFolderItemData * itemData = static_cast<BookmarkFolderItemData *>(data);
        BROWSER_LOGD("Folder ID: %d" , itemData->item->getId());
        itemData->bookmarkManagerUI->set_folder(itemData->item->getTittle().c_str());
        itemData->bookmarkManagerUI->bookmarkFolderClicked(itemData->item->getId());
    }
}


void BookmarkManagerUI::_bookmark_thumbSelected(void * data, Evas_Object *, void *)
{
    (void)data;
#if 0
    if (data != nullptr)
    {
        HistoryItemData * itemData = static_cast<HistoryItemData *>(data);
        itemData->bookmarkManagerUI->bookmarkClicked(itemData->item);
    }
#endif
}

void BookmarkManagerUI::clearItems()
{
    hide();
    BROWSER_LOGD("Deleting all items from gengrid");
    elm_gengrid_clear(m_gengrid);
    elm_genlist_clear(m_genList);
    m_map_bookmark_folder_views.clear();
    elm_theme_extension_del(nullptr, edjFilePath.c_str());
    elm_theme_full_flush();
    elm_cache_all_flush();
}


void BookmarkManagerUI::updateGengrid()
{
    elm_genlist_clear(m_genList);
    elm_gengrid_clear(m_gengrid);
    m_map_bookmark_folder_views.clear();
}

void BookmarkManagerUI::focusItem(void*, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (event_info != nullptr)
    {
        Elm_Object_Item *item = static_cast<Elm_Object_Item*>(event_info);
        elm_object_item_signal_emit( item, "mouse,in", "over2");

        // selected manually
        elm_gengrid_item_selected_set(item, EINA_TRUE);
    }
}

void BookmarkManagerUI::unFocusItem(void*, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (event_info != nullptr)
    {
        Elm_Object_Item *item = static_cast<Elm_Object_Item*>(event_info);
        elm_object_item_signal_emit( item, "mouse,out", "over2");

        // unselected manually
        elm_gengrid_item_selected_set(item, EINA_FALSE);
    }
}

}
}
