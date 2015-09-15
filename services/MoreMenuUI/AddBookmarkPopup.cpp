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

#include "Tools/EflTools.h"
#include "AddBookmarkPopup.h"
#include "BrowserLogger.h"

namespace tizen_browser{
namespace base_ui{

typedef struct _BookmarkFolderItemData
{
    int index;
    std::shared_ptr<tizen_browser::services::BookmarkItem> item;
    std::shared_ptr<tizen_browser::base_ui::AddBookmarkPopup> addBookmarkPopup;
} BookmarkFolderItemData;


AddBookmarkPopup::AddBookmarkPopup(Evas_Object* main_layout):
    m_mainLayout(main_layout),
    m_popup(nullptr),
    m_gengrid(nullptr),
    m_itemClass(nullptr)
{
}

AddBookmarkPopup::~AddBookmarkPopup ()
{}

void AddBookmarkPopup::show()
{
    BROWSER_LOGD("[%s],", __func__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("MoreMenuUI/AddBookmarkPopup.edj");
    elm_theme_extension_add(0, m_edjFilePath.c_str());
    m_popup = elm_layout_add(m_mainLayout);
    if (m_popup != nullptr)
    {
        elm_object_signal_emit(m_mainLayout, "elm,state,show", "elm");
        elm_layout_file_set(m_popup, m_edjFilePath.c_str(), "bookmark_popup");
        BROWSER_LOGI("PATH: %s", m_edjFilePath.c_str());

        m_gengrid = elm_gengrid_add(m_popup);

        if (m_gengrid != nullptr)
        {
            evas_object_smart_callback_add(m_gengrid, "item,focused", focusItem, nullptr);
            evas_object_smart_callback_add(m_gengrid, "item,unfocused", unFocusItem, nullptr);
        //  evas_object_smart_callback_add(m_gengrid, "activated", _itemSelected, this);

              if (!m_itemClass) {
                    m_itemClass = elm_gengrid_item_class_new();
                    m_itemClass->item_style = "folder_grid_item";
                    m_itemClass->func.text_get = _grid_text_get;
                    m_itemClass->func.content_get =  _grid_content_get;
                    m_itemClass->func.state_get = nullptr;
                    m_itemClass->func.del = nullptr;
                }

            elm_gengrid_align_set(m_gengrid, 0, 0);
            elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
            elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
            elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
            elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
            elm_scroller_page_size_set(m_gengrid, 0, 327);
            evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
            double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
            elm_gengrid_item_size_set(m_gengrid, 226 * efl_scale, 226 * efl_scale);

            elm_object_part_content_set(m_popup, "elm.swallow.gengrid", m_gengrid);
            elm_object_part_content_set(m_mainLayout, "popup", m_popup);
            evas_object_show(m_gengrid);
        }
    }
}

void AddBookmarkPopup::hide()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    elm_object_signal_emit(m_mainLayout, "elm,state,hide", "elm");
    evas_object_hide(elm_layout_content_get(m_popup, "elm.swallow.gengrid"));
    evas_object_hide(elm_layout_content_get(m_mainLayout, "popup"));
    evas_object_hide(m_gengrid);
    evas_object_hide(m_popup);
    clearItems();
}

Evas_Object * AddBookmarkPopup::getContent()
{
    return m_gengrid;
}

void AddBookmarkPopup::setEmptyGengrid(bool setEmpty)
{
    if(setEmpty) {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", createNoHistoryLabel());
    } else {
        elm_object_part_content_set(m_gengrid, "elm.swallow.empty", nullptr);
    }
}

Evas_Object* AddBookmarkPopup::createNoHistoryLabel()
{
    Evas_Object *label = elm_label_add(m_popup);
    if (label != nullptr)
    {
        elm_object_text_set(label, "No favorite websites.");
        evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    }
    return label;
}

void AddBookmarkPopup::addBookmarkFolderItem(std::shared_ptr<tizen_browser::services::BookmarkItem> hi , int ind)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    BookmarkFolderItemData *itemData = new BookmarkFolderItemData();
    if (itemData != nullptr)
    {
        itemData->item = hi;
        itemData->index = ind;
        itemData->addBookmarkPopup = std::shared_ptr<tizen_browser::base_ui::AddBookmarkPopup>(this);
        Elm_Object_Item* BookmarkFolderView = elm_gengrid_item_append(m_gengrid, m_itemClass, itemData, nullptr, this);
        if (BookmarkFolderView)
        {
            if(ind == 1)
            {
                 BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
                 m_map_bookmark_folder_views.insert(std::pair<std::string,Elm_Object_Item*>("new_folder_button",BookmarkFolderView));
            }
            else
                m_map_bookmark_folder_views.insert(std::pair<std::string,Elm_Object_Item*>(hi->getAddress(),BookmarkFolderView));

            elm_gengrid_item_selected_set(BookmarkFolderView, EINA_FALSE);
            setEmptyGengrid(false);
        }
    }
}

void AddBookmarkPopup::addBookmarkFolderItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
    BROWSER_LOGD("%s:%d %s Bookmark Folder Items: %d", __FILE__, __LINE__, __func__, items.size());
    int i = 1;
    addBookmarkFolderItem(nullptr, i++);
    for (auto it = items.begin(); it != items.end(); ++it, ++i) {
        addBookmarkFolderItem(*it, i);
    }
    elm_object_part_content_set(m_popup, "elm.swallow.gengrid",getContent());
    evas_object_show(getContent());
}

char* AddBookmarkPopup::_grid_text_get(void *data, Evas_Object *, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    if ((data != nullptr) && (part != nullptr))
    {
        static const char* part_name = "page_title";
        static const int part_len = strlen(part_name);

        BookmarkFolderItemData *itemData = static_cast<BookmarkFolderItemData*>(data);
        if (!strncmp(part, part_name, part_len)) {
              if(itemData->item)
                return strdup(itemData->item->getTittle().c_str());
        }
    }
    return strdup("");
}

const char* AddBookmarkPopup::getImageFileNameForType(int index, bool focused)
{
    static const char *file_name = "";
    if (index == 1) {
        file_name = focused ? "btn_bar_new_foc.png" : "btn_bar_new_nor.png";
    }
    return file_name;
}

Evas_Object * AddBookmarkPopup::_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    if ((data != nullptr) && (obj != nullptr) && (part != nullptr))
    {
        BookmarkFolderItemData *itemData = static_cast<BookmarkFolderItemData*>(data);

        static const char* part1_name = "elm.thumbnail";
        static const char* part2_name = "elm.thumbButton";
        static const int part1_len = strlen(part1_name);
        static const int part2_len = strlen(part2_name);

        if (!strncmp(part, part1_name, part1_len) && (itemData->index == 1)) {
            BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
            Evas_Object* thumb_nail = elm_icon_add(obj);
            const char* file_name = getImageFileNameForType(itemData->index, false);
            BROWSER_LOGD("%s:%d %s file=%s", __FILE__, __LINE__, __func__, file_name);
            if (thumb_nail != nullptr)
            {
                elm_image_file_set(thumb_nail, itemData->addBookmarkPopup->m_edjFilePath.c_str(), file_name);
            }
            return thumb_nail;
        }
        if (!strncmp(part, part2_name, part2_len)) {
            Evas_Object *thumbButton = elm_button_add(obj);
            if (thumbButton != nullptr)
            {
                elm_object_style_set(thumbButton, part2_name);
                if(itemData->index == 1)
                {
                    evas_object_smart_callback_add(thumbButton, "clicked", tizen_browser::base_ui::AddBookmarkPopup::_newFolderButton, data);
                    evas_object_event_callback_add(thumbButton, EVAS_CALLBACK_MOUSE_IN, __cb_mouse_in, data);
                    evas_object_event_callback_add(thumbButton, EVAS_CALLBACK_MOUSE_OUT, __cb_mouse_out, data);
                }
                else
                    evas_object_smart_callback_add(thumbButton, "clicked", tizen_browser::base_ui::AddBookmarkPopup::_thumbSelected, data);
            }
            return thumbButton;
        }

    }
    return nullptr;
}

void AddBookmarkPopup::__cb_mouse_in(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    if ((data != nullptr) && (obj != nullptr))
    {
        elm_object_focus_set(obj, EINA_TRUE);
        BookmarkFolderItemData *itemData = static_cast<BookmarkFolderItemData*>(data);

        const char* file_name = getImageFileNameForType(itemData->index, true);
        Elm_Object_Item * selected = itemData->addBookmarkPopup->m_map_bookmark_folder_views["new_folder_button"];
        if (selected != nullptr)
        {
            Evas_Object *thumb_nail = elm_object_item_part_content_get(selected, "elm.thumbnail");
            if (thumb_nail != nullptr)
            {
                elm_image_file_set(thumb_nail, itemData->addBookmarkPopup->m_edjFilePath.c_str(), file_name);
            }
        }
    }
}

void AddBookmarkPopup::__cb_mouse_out(void * data, Evas *, Evas_Object *obj, void* )
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj){
        BookmarkFolderItemData *itemData = static_cast<BookmarkFolderItemData*>(data);
        elm_object_focus_set(obj, EINA_FALSE);
        const char* file_name = getImageFileNameForType(itemData->index, false);
        Elm_Object_Item * selected = itemData->addBookmarkPopup->m_map_bookmark_folder_views["new_folder_button"];
        Evas_Object *thumb_nail = elm_object_item_part_content_get(selected, "elm.thumbnail");
        elm_image_file_set(thumb_nail, itemData->addBookmarkPopup->m_edjFilePath.c_str(), file_name);
    }
}

//void AddBookmarkPopup::_itemSelected(void* , Evas_Object* , void*)
//{
//    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
//    if (data && event_info) {
//        Elm_Object_Item * selected = static_cast<Elm_Object_Item *>(event_info);
//        HistoryItemData * itemData = static_cast<HistoryItemData *>(elm_object_item_data_get(selected));
//        AddBookmarkPopup * self = static_cast<AddBookmarkPopup *>(data);
//
//        self->bookmarkClicked(itemData->item);
//    }
//}


void AddBookmarkPopup::_newFolderButton(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data) {
        BookmarkFolderItemData * itemData = static_cast<BookmarkFolderItemData *>(data);
        itemData->addBookmarkPopup->addNewFolderClicked(std::string());
    }
}

void AddBookmarkPopup::_thumbSelected(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data) {
        BookmarkFolderItemData * itemData = static_cast<BookmarkFolderItemData *>(data);
        itemData->addBookmarkPopup->folderSelected(itemData->item->getId());
    }
}

void AddBookmarkPopup::clearItems()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    elm_gengrid_clear(m_gengrid);
    m_map_bookmark_folder_views.clear();
    elm_theme_extension_del(nullptr, m_edjFilePath.c_str());
    elm_theme_full_flush();
    elm_cache_all_flush();
}


void AddBookmarkPopup::updateGengrid()
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    elm_gengrid_clear(m_gengrid);
    m_map_bookmark_folder_views.clear();
}

void AddBookmarkPopup::focusItem(void*, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (event_info) {
        Elm_Object_Item *item = static_cast<Elm_Object_Item*>(event_info);
        elm_object_item_signal_emit(item, "mouse,in", "over2");

        // selected manually
        elm_gengrid_item_selected_set(item, EINA_TRUE);
    }
}

void AddBookmarkPopup::unFocusItem(void*, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (event_info) {
        Elm_Object_Item *item = static_cast<Elm_Object_Item*>(event_info);
        elm_object_item_signal_emit(item, "mouse,out", "over2");

        // unselected manually
        elm_gengrid_item_selected_set(item, EINA_FALSE);
    }
}

}
}
