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
#include <boost/format.hpp>
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
    : b_mm_layout(nullptr)
    , m_itemClass(nullptr)
    , m_gengrid(nullptr)
    , m_parent(nullptr)
    , m_bookmark_item_class(nullptr)
    , m_gengridSetup(false)
#if PROFILE_MOBILE
    , m_details_item_class(nullptr)
    , m_folder_new_item_class(nullptr)
    , m_folder_all_item_class(nullptr)
    , m_folder_mobile_item_class(nullptr)
    , m_folder_custom_item_class(nullptr)
    , b_details_layout(nullptr)
    , m_details_gengrid(nullptr)
#endif
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

#if PROFILE_MOBILE
    if(m_details_item_class)
        elm_gengrid_item_class_free(m_details_item_class);

    if(m_folder_new_item_class)
        elm_gengrid_item_class_free(m_folder_new_item_class);

    if(m_folder_all_item_class)
        elm_gengrid_item_class_free(m_folder_all_item_class);

    if(m_folder_mobile_item_class)
        elm_gengrid_item_class_free(m_folder_mobile_item_class);

    if(m_folder_custom_item_class)
        elm_gengrid_item_class_free(m_folder_custom_item_class);
#endif
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

#if PROFILE_MOBILE
    m_details_item_class = elm_gengrid_item_class_new();
    m_details_item_class->item_style = "grid_details_item";
    m_details_item_class->func.text_get = _grid_title_text_get;
    m_details_item_class->func.content_get =  _grid_bookmark_content_get;
    m_details_item_class->func.state_get = nullptr;
    m_details_item_class->func.del = nullptr;

    m_folder_new_item_class = elm_gengrid_item_class_new();
    m_folder_new_item_class->item_style = "grid_new_folder_item";
    m_folder_new_item_class->func.text_get = _grid_folder_title_text_get;
    m_folder_new_item_class->func.content_get =  nullptr;
    m_folder_new_item_class->func.state_get = nullptr;
    m_folder_new_item_class->func.del = nullptr;

    m_folder_all_item_class = elm_gengrid_item_class_new();
    m_folder_all_item_class->item_style = "grid_all_item";
    m_folder_all_item_class->func.text_get = _grid_folder_title_text_get;
    m_folder_all_item_class->func.content_get =  nullptr;
    m_folder_all_item_class->func.state_get = nullptr;
    m_folder_all_item_class->func.del = nullptr;

    m_folder_mobile_item_class = elm_gengrid_item_class_new();
    m_folder_mobile_item_class->item_style = "grid_mobile_folder_item";
    m_folder_mobile_item_class->func.text_get = _grid_folder_title_text_get;
    m_folder_mobile_item_class->func.content_get =  nullptr;
    m_folder_mobile_item_class->func.state_get = nullptr;
    m_folder_mobile_item_class->func.del = nullptr;

    m_folder_custom_item_class = elm_gengrid_item_class_new();
    m_folder_custom_item_class->item_style = "grid_custom_folder_item";
    m_folder_custom_item_class->func.text_get = _grid_folder_title_text_get;
    m_folder_custom_item_class->func.content_get =  nullptr;
    m_folder_custom_item_class->func.state_get = nullptr;
    m_folder_custom_item_class->func.del = nullptr;
#endif
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
    m_focusManager.startFocusManager(m_gengrid);
}

void BookmarkManagerUI::hideUI()
{
    evas_object_hide(b_mm_layout);
    elm_gengrid_clear(m_gengrid);
    m_map_bookmark.clear();
    m_focusManager.stopFocusManager();
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
    createFocusVector();
    return b_mm_layout;
}

//TODO: Make parend the argument and return created object to make code more modular.
//      (After fixing window managment)
void BookmarkManagerUI::createGenGrid()
{
    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
    m_gengrid = elm_gengrid_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid", m_gengrid);
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
#if PROFILE_MOBILE
    elm_scroller_bounce_set(m_gengrid, EINA_FALSE, EINA_TRUE);
    elm_object_scroll_lock_x_set(m_gengrid, EINA_TRUE);
    elm_gengrid_item_size_set(m_gengrid, (319+30) * efl_scale, (361+30) * efl_scale);
#else
    elm_object_style_set(m_gengrid, "back_ground");
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_horizontal_set(m_gengrid, EINA_TRUE);
    elm_gengrid_item_size_set(m_gengrid, 404 * efl_scale, 320 * efl_scale);
#endif
}

void BookmarkManagerUI::showTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(b_mm_layout);
    elm_theme_extension_add(nullptr, edjFilePath.c_str());

    m_topContent = elm_layout_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "top_content", m_topContent);
    evas_object_size_hint_weight_set(m_topContent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_topContent, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_topContent);

    elm_layout_file_set(m_topContent, edjFilePath.c_str(), "topContent");

    Evas_Object* close_button = elm_button_add(m_topContent);
    elm_object_style_set(close_button, "hidden_button");
    evas_object_smart_callback_add(close_button, "clicked", close_clicked_cb, this);
    elm_object_part_content_set(m_topContent, "close_click", close_button);

    evas_object_show(close_button);
    elm_object_focus_custom_chain_append(m_topContent, close_button, nullptr);
    elm_object_focus_set(close_button, EINA_TRUE);
    elm_object_tree_focus_allow_set(b_mm_layout, EINA_TRUE);
    elm_object_focus_allow_set(close_button, EINA_TRUE);
}

void BookmarkManagerUI::close_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkManagerUI* id = static_cast<BookmarkManagerUI*>(data);
        id->closeBookmarkManagerClicked();
    }
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

#if PROFILE_MOBILE
void BookmarkManagerUI::addBookmarkFolder(std::shared_ptr<tizen_browser::services::BookmarkItem> hi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = hi;
    itemData->bookmarkManagerUI.reset(this);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_bookmark_custom_folder_item_class, itemData, _bookmarkCustomFolderClicked, itemData);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
    setEmptyGengrid(false);
}

void BookmarkManagerUI::addNewFolderItem()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_bookmark_new_folder_item_class, NULL, _bookmarkNewFolderItemClicked, NULL);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
    setEmptyGengrid(false);
}

void BookmarkManagerUI::addAllItem()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_bookmark_all_item_class, NULL, _bookmarkAllItemClicked, NULL);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
    setEmptyGengrid(false);
}

void BookmarkManagerUI::addMobileItem()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_bookmark_mobile_folder_item_class, NULL, _bookmarkMobileFolderItemClicked, NULL);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
    setEmptyGengrid(false);
}

void BookmarkManagerUI::addBookmarkFolders(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    addNewFolderItem();
    addAllItem();
    addMobileItem();

    for (auto it = items.begin(); it != items.end(); ++it) {
        BROWSER_LOGD("%d %s %s",(*it)->is_folder(),(*it)->getTittle().c_str(), (*it)->getAddress().c_str());
        if ((*it)->is_folder()) {
            addBookmarkFolder(*it);
        }
    }
    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid",getGenGrid());
    evas_object_show(getGenGrid());
}

char* BookmarkManagerUI::_grid_folder_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr))
    {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        BookmarkItemData *itemData = static_cast<BookmarkItemData*>(data);
        const char *part_name1 = "page_title";
        static const int part_name1_len = strlen(part_name1);
        if (!strncmp(part_name1, part, part_name1_len))
        {
            return strdup(itemData->item->getTittle().c_str());
        }
    }
    return strdup("");
}

#endif

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

void BookmarkManagerUI::_bookmarkItemClicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    if (data != nullptr)
    {
        BookmarkItemData * itemData = static_cast<BookmarkItemData*>(data);
        BROWSER_LOGD("Bookmark URL: %s" , itemData->item->getAddress().c_str());
        itemData->bookmarkManagerUI->bookmarkItemClicked(itemData->item);
    }
}

#if PROFILE_MOBILE
void BookmarkManagerUI::_bookmarkCustomFolderClicked(void * , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //TODO : Custom Folder
}

void BookmarkManagerUI::_bookmarkNewFolderItemClicked(void * , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //TODO : New Folder
}

void BookmarkManagerUI::_bookmarkAllItemClicked(void * , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //TODO : All
}

void BookmarkManagerUI::_bookmarkMobileFolderItemClicked(void * , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //TODO : Mobile(root) Folder
}
#endif

void BookmarkManagerUI::createFocusVector()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_focusManager.addItem(elm_object_part_content_get(m_topContent, "close_click"));
    m_focusManager.addItem(m_gengrid);
    m_focusManager.setIterator();
}

#if PROFILE_MOBILE
void BookmarkManagerUI::back_details_clicked_cb(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkManagerUI* id = static_cast<BookmarkManagerUI*>(data);
        id->hideFolderDetailsUI();
    }
}

void BookmarkManagerUI::more_details_clicked_cb(void*, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //TODO: more menu in folder details
}

void BookmarkManagerUI::createFolderDetailsGenGrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    double efl_scale = elm_config_scale_get() / elm_app_base_scale_get();
    m_details_gengrid = elm_gengrid_add(b_details_layout);
    elm_object_part_content_set(b_details_layout, "elm.swallow.grid", m_details_gengrid);
    elm_gengrid_align_set(m_details_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_details_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_details_gengrid, EINA_FALSE);
    elm_scroller_bounce_set(m_details_gengrid, EINA_FALSE, EINA_TRUE);
    elm_object_scroll_lock_x_set(m_details_gengrid, EINA_TRUE);
    elm_gengrid_item_size_set(m_details_gengrid, (319+30) * efl_scale, (361+30) * efl_scale);
}

void BookmarkManagerUI::createFolderDetailsTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_details_topContent = elm_layout_add(b_details_layout);
    elm_object_part_content_set(b_details_layout, "top_content", m_details_topContent);
    evas_object_size_hint_weight_set(m_details_topContent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_details_topContent, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_details_topContent);

    elm_layout_file_set(m_details_topContent, edjFilePath.c_str(), "topDetailContent");

    Evas_Object* close_button = elm_button_add(m_details_topContent);
    elm_object_style_set(close_button, "hidden_button");
    evas_object_smart_callback_add(close_button, "clicked", back_details_clicked_cb, this);
    elm_object_part_content_set(m_details_topContent, "back_click", close_button);
    evas_object_show(close_button);

    Evas_Object* more_button = elm_button_add(m_details_topContent);
    elm_object_style_set(more_button, "hidden_button");
    evas_object_smart_callback_add(more_button, "clicked", more_details_clicked_cb, this);
    elm_object_part_content_set(m_details_topContent, "more_click", more_button);
    evas_object_show(more_button);

}

Evas_Object* BookmarkManagerUI::createFolderDetailsLayout(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(nullptr, edjFilePath.c_str());
    b_details_layout = elm_layout_add(parent);
    elm_layout_file_set(b_details_layout, edjFilePath.c_str(), "bookmarkmanager-layout");
    evas_object_size_hint_weight_set(b_details_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(b_details_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    createFolderDetailsGenGrid();
    createFolderDetailsTopContent();

    return b_details_layout;
}

void BookmarkManagerUI::showDetailsUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_show(b_details_layout);
}

void BookmarkManagerUI::hideFolderDetailsUI()
{
    evas_object_hide(b_details_layout);
    elm_gengrid_clear(m_details_gengrid);
}

Evas_Object* BookmarkManagerUI::getDetailsContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    b_details_layout = createFolderDetailsLayout(b_mm_layout);
    return b_mm_layout;
}

void BookmarkManagerUI::setDetailsEmptyGengrid(bool setEmpty)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(setEmpty) {
        elm_object_part_content_set(m_details_gengrid, "elm.swallow.empty", createNoHistoryLabel());
    } else {
        elm_object_part_content_set(m_details_gengrid, "elm.swallow.empty", nullptr);
    }
}

void BookmarkManagerUI::addBookmarkFolder(std::shared_ptr<tizen_browser::services::BookmarkItem> hi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = hi;
    itemData->bookmarkManagerUI.reset(this);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_custom_item_class, itemData, _bookmarkCustomFolderClicked, itemData);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
    setEmptyGengrid(false);
}

void BookmarkManagerUI::addNewFolderItem()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_new_item_class, NULL, _bookmarkNewFolderItemClicked, NULL);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
    setEmptyGengrid(false);
}

void BookmarkManagerUI::addAllItem()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = NULL;
    itemData->bookmarkManagerUI.reset(this);    
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_all_item_class, itemData, _bookmarkAllItemClicked, itemData);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
    setEmptyGengrid(false);
}

void BookmarkManagerUI::addMobileItem()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = NULL;
    itemData->bookmarkManagerUI.reset(this);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_mobile_item_class, itemData, _bookmarkMobileFolderItemClicked, itemData);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
    setEmptyGengrid(false);
}

void BookmarkManagerUI::addFolders(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    addNewFolderItem();
    addAllItem();
    addMobileItem();

    for (auto it = items.begin(); it != items.end(); ++it) {
        BROWSER_LOGD("%d %s %s",(*it)->is_folder(),(*it)->getTittle().c_str(), (*it)->getAddress().c_str());
        if ((*it)->is_folder()) {
            addBookmarkFolder(*it);
        }
    }
    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid",getGenGrid());
    evas_object_show(getGenGrid());
}

void BookmarkManagerUI::addDetailsItem(std::shared_ptr<tizen_browser::services::BookmarkItem> hi)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkItemData *itemData = new BookmarkItemData();
    itemData->item = hi;
    itemData->bookmarkManagerUI.reset(this);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_details_gengrid, m_details_item_class, itemData, _bookmarkItemClicked, itemData);
    m_map_bookmark.insert(std::pair<std::string,Elm_Object_Item*>(hi->getAddress(),BookmarkView));
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
    setDetailsEmptyGengrid(false);
}

void BookmarkManagerUI::addDetails(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items, std::string folder_name, int count)
{
     BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
     elm_object_part_text_set(m_details_topContent,"title_text",(boost::format("%s(%d)") % folder_name.c_str() % count).str().c_str());

     for (auto it = items.begin(); it != items.end(); ++it)
     {
         BROWSER_LOGD("%d %s %s",(*it)->is_folder(),(*it)->getTittle().c_str(), (*it)->getAddress().c_str());
         if (!(*it)->is_folder()) {
             addDetailsItem(*it);
         }
     }
     elm_object_part_content_set(b_details_layout, "elm.swallow.grid", m_details_gengrid);
     evas_object_show(m_details_gengrid);
}

char* BookmarkManagerUI::_grid_title_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr))
    {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        BookmarkItemData *itemData = static_cast<BookmarkItemData*>(data);
        const char *part_name1 = "page_title";
        static const int part_name1_len = strlen(part_name1);
        if (!strncmp(part_name1, part, part_name1_len))
        {
            return strdup(itemData->item->getTittle().c_str());
        }
    }
    return strdup("");
}

char* BookmarkManagerUI::_grid_folder_title_text_get(void *data, Evas_Object *, const char *part)
{
    //TODO: need to get count from vector
    if ((data != nullptr) && (part != nullptr))
    {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        BookmarkItemData *itemData = static_cast<BookmarkItemData*>(data);
        const char *part_name1 = "page_title";
        static const int part_name1_len = strlen(part_name1);
        if (!strncmp(part_name1, part, part_name1_len))
        {
            return strdup(itemData->item->getTittle().c_str());
        }
    }
    return strdup("");
}

void BookmarkManagerUI::_bookmarkCustomFolderClicked(void * data , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkItemData * itemData = static_cast<BookmarkItemData*>(data);
        BROWSER_LOGD("Folder Name: %s" , itemData->item->getTittle().c_str());
        itemData->bookmarkManagerUI->customFolderClicked(itemData->item);
    }
}

void BookmarkManagerUI::_bookmarkNewFolderItemClicked(void * , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //TODO: New Folder    
}

void BookmarkManagerUI::_bookmarkAllItemClicked(void * data , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkItemData * itemData = static_cast<BookmarkItemData*>(data);
        itemData->bookmarkManagerUI->allFolderClicked();
    }
}

void BookmarkManagerUI::_bookmarkMobileFolderItemClicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkItemData * itemData = static_cast<BookmarkItemData*>(data);
        itemData->bookmarkManagerUI->mobileFolderClicked();
    }
}

#endif

}
}
