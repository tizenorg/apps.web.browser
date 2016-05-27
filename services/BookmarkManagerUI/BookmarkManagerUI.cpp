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
#include "app_i18n.h"
#include "Tools/BookmarkItem.h"

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(BookmarkManagerUI, "org.tizen.browser.bookmarkmanagerui")

struct ItemData
{
    tizen_browser::base_ui::BookmarkManagerUI * m_bookmarkManager;
    tizen_browser::services::BookmarkItem * h_item;
    Elm_Object_Item * e_item;
};

BookmarkManagerUI::BookmarkManagerUI()
    : m_parent(nullptr)
    , b_mm_layout(nullptr)
    , m_topContent(nullptr)
    , m_cancel_button(nullptr)
    , m_delete_button(nullptr)
    , m_navigatorToolbar(nullptr)
    , m_genlist(nullptr)
    , m_empty_layout(nullptr)
    , m_select_all(nullptr)
    , m_gengrid(nullptr)
    #if !PROFILE_MOBILE
    , m_bottom_content(nullptr)
    #else
    , m_folder_new_item_class(nullptr)
    #endif
    , m_folder_all_item_class(nullptr)
    , m_folder_mobile_item_class(nullptr)
    , m_folder_custom_item_class(nullptr)
    , m_bookmark_item_class(nullptr)
    , m_state(BookmarkManagerState::Default)
    , m_reordered(false)
    , m_delete_count(0)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("BookmarkManagerUI/BookmarkManagerUI.edj");
    createGengridItemClasses();
    createGenlistItemClasses();
}

BookmarkManagerUI::~BookmarkManagerUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if(m_folder_all_item_class)
        elm_gengrid_item_class_free(m_folder_all_item_class);

    if(m_folder_custom_item_class)
        elm_gengrid_item_class_free(m_folder_custom_item_class);

#if PROFILE_MOBILE
    if(m_folder_new_item_class)
        elm_gengrid_item_class_free(m_folder_new_item_class);
#endif
    if(m_folder_mobile_item_class)
        elm_gengrid_item_class_free(m_folder_mobile_item_class);

    if (m_bookmark_item_class)
        elm_genlist_item_class_free(m_bookmark_item_class);
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
#if PROFILE_MOBILE
    orientationChanged();
#endif
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
    changeState(BookmarkManagerState::Default);

    return b_mm_layout;
}

void BookmarkManagerUI::createGengridItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_folder_all_item_class = elm_gengrid_item_class_new();
    m_folder_all_item_class->item_style = "grid_all_item";
    m_folder_all_item_class->func.text_get = _grid_all_folder_title_text_get;
    m_folder_all_item_class->func.content_get =  nullptr;
    m_folder_all_item_class->func.state_get = nullptr;
    m_folder_all_item_class->func.del = _grid_content_delete;

    m_folder_custom_item_class = elm_gengrid_item_class_new();
    m_folder_custom_item_class->item_style = "grid_custom_folder_item";
    m_folder_custom_item_class->func.text_get = _grid_folder_title_text_get;
    m_folder_custom_item_class->func.content_get =  nullptr;
    m_folder_custom_item_class->func.state_get = nullptr;
    m_folder_custom_item_class->func.del = _grid_content_delete;

#if PROFILE_MOBILE
    m_folder_new_item_class = elm_gengrid_item_class_new();
    m_folder_new_item_class->item_style = "grid_new_folder_item";
    m_folder_new_item_class->func.text_get = nullptr;
    m_folder_new_item_class->func.content_get =  nullptr;
    m_folder_new_item_class->func.state_get = nullptr;
    m_folder_new_item_class->func.del = _grid_content_delete;
#endif

    m_folder_mobile_item_class = elm_gengrid_item_class_new();
    m_folder_mobile_item_class->item_style = "grid_mobile_folder_item";
    m_folder_mobile_item_class->func.text_get = _grid_folder_title_text_get;
    m_folder_mobile_item_class->func.content_get =  nullptr;
    m_folder_mobile_item_class->func.state_get = nullptr;
    m_folder_mobile_item_class->func.del = _grid_content_delete;
}

void BookmarkManagerUI::createGenlistItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_bookmark_item_class = elm_genlist_item_class_new();
    m_bookmark_item_class->item_style = "type1";
    m_bookmark_item_class->func.text_get = _genlist_bookmark_text_get;
    m_bookmark_item_class->func.content_get =  _genlist_bookmark_content_get;
    m_bookmark_item_class->func.state_get = nullptr;
    m_bookmark_item_class->func.del = nullptr;
    //m_bookmark_item_class->decorate_all_item_style = "edit_default";
}

void BookmarkManagerUI::_grid_content_delete(void *data, Evas_Object */*obj*/)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    auto itemData = static_cast<FolderData*>(data);
    if (itemData)
        delete itemData;
}

char* BookmarkManagerUI::_genlist_bookmark_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr)) {
        BookmarkData *bookmarkData = static_cast<BookmarkData*>(data);
        if (!strcmp(part, "elm.text"))
            return strdup((boost::format("%s") % bookmarkData->bookmarkItem->getTitle()).str().c_str());
        if (!bookmarkData->bookmarkItem->is_folder())
            if (!strcmp(part, "elm.text.sub"))
                return strdup((boost::format("%s") % bookmarkData->bookmarkItem->getAddress()).str().c_str());
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

Evas_Object *BookmarkManagerUI::_genlist_bookmark_content_get(void *data, Evas_Object *obj, const char *part)
{
    if ((data != nullptr) && (part != nullptr)) {
        BookmarkData *bookmarkData = static_cast<BookmarkData*>(data);
        if (!strcmp(part, "elm.swallow.icon")) {
            Evas_Object* layout = elm_layout_add(obj);
            if (bookmarkData->bookmarkItem->is_folder())
                elm_layout_file_set(layout, bookmarkData->bookmarkManagerUI->m_edjFilePath.c_str(), "folder_image");
            else if (bookmarkData->bookmarkItem->has_favicon()) {
                elm_layout_file_set(layout, bookmarkData->bookmarkManagerUI->m_edjFilePath.c_str(), "content_image");
                std::shared_ptr<tools::BrowserImage> image = bookmarkData->bookmarkItem->getFavicon();
                Evas_Object* favicon = image->getEvasImage(obj);
                elm_object_part_content_set(layout, "content", favicon);
            } else
                elm_layout_file_set(layout, bookmarkData->bookmarkManagerUI->m_edjFilePath.c_str(), "favicon_image");
            return layout;
        } else if (!strcmp(part, "elm.swallow.end")) {
            switch (bookmarkData->bookmarkManagerUI->m_state) {
            case BookmarkManagerState::Delete: {
                Evas_Object* checkbox = elm_check_add(obj);
                evas_object_propagate_events_set(checkbox, EINA_FALSE);
                elm_check_state_set(checkbox, bookmarkData->bookmarkManagerUI->m_map_delete[bookmarkData->bookmarkItem->getId()]
                        ? EINA_TRUE : EINA_FALSE);
                evas_object_smart_callback_add(checkbox, "changed", _check_state_changed, bookmarkData);
                evas_object_show(checkbox);
                return checkbox;
                }
                break;
            case BookmarkManagerState::Reorder: {
                Evas_Object *reorder_button = elm_button_add(obj);
                elm_object_style_set(reorder_button, "icon_reorder");
                return reorder_button;
                }
                break;
            default:
                break;
            }
        }
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return nullptr;
}

char* BookmarkManagerUI::_grid_all_folder_title_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr)) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        FolderData *folderData = static_cast<FolderData*>(data);
        const char *part_name1 = "page_title";
        const int part_name1_len = strlen(part_name1);
        if (!strncmp(part_name1, part, part_name1_len))
            return strdup((boost::format("%s  (%d)") % _("IDS_BR_BODY_ALL") % folderData->count).str().c_str());
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return strdup("");
}

char* BookmarkManagerUI::_grid_folder_title_text_get(void *data, Evas_Object *, const char *part)
{
    if ((data != nullptr) && (part != nullptr)) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        FolderData *folderData = static_cast<FolderData*>(data);
        const char *part_name1 = "page_title";
        const int part_name1_len = strlen(part_name1);
        if (!strncmp(part_name1, part, part_name1_len))
            return strdup((boost::format("%s  (%d)") % folderData->name.c_str() % folderData->count).str().c_str());
    } else
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    return strdup("");
}

Evas_Object* BookmarkManagerUI::createBookmarksLayout(Evas_Object* parent)
{
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    b_mm_layout = elm_layout_add(parent);
    elm_layout_file_set(b_mm_layout, m_edjFilePath.c_str(), "bookmarkmanager-layout");
    evas_object_size_hint_weight_set(b_mm_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(b_mm_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(b_mm_layout);

    //createGengrid();
    createTopContent();
    createNavigatorToolbar();

    //TODO: Add translation

    createGenlist();
    createEmptyLayout();
    createFocusVector();
#if !PROFILE_MOBILE
    createBottomContent();
#endif
    return b_mm_layout;
}

void BookmarkManagerUI::createNavigatorToolbar()
{
    m_navigatorToolbar = elm_toolbar_add(b_mm_layout);

    elm_object_style_set(m_navigatorToolbar, "default");
    elm_object_style_set(m_navigatorToolbar, "navigationbar");
    elm_toolbar_shrink_mode_set(m_navigatorToolbar, ELM_TOOLBAR_SHRINK_SCROLL);
    elm_toolbar_transverse_expanded_set(m_navigatorToolbar, EINA_TRUE);
    elm_toolbar_align_set(m_navigatorToolbar, 0.0);
    elm_toolbar_homogeneous_set(m_navigatorToolbar, EINA_FALSE);
    elm_toolbar_select_mode_set(m_navigatorToolbar, ELM_OBJECT_SELECT_MODE_DEFAULT);
    elm_object_part_content_set(b_mm_layout, "toolbar", m_navigatorToolbar);
    evas_object_show(m_navigatorToolbar);
    elm_object_signal_emit(b_mm_layout, "show_toolbar", "ui");
}

void BookmarkManagerUI::createGenlist()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_box = elm_box_add(b_mm_layout);
    elm_object_focus_set(m_box, EINA_FALSE);
    evas_object_size_hint_weight_set(m_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_box_clear(m_box);

    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid", m_box);
    evas_object_show(m_box);

    m_genlist = elm_genlist_add(m_box);

    elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_genlist_homogeneous_set(m_genlist, EINA_TRUE);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);
    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genlist, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genlist, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(m_genlist, "moved", _genlist_bookmark_moved, this);

    elm_box_pack_end(m_box, m_genlist);
    evas_object_show(m_genlist);

    m_select_all = elm_layout_add(m_genlist);
    elm_object_focus_allow_set(m_select_all, EINA_TRUE);
    elm_layout_theme_set(m_select_all, "genlist", "item", "type1/default");
    evas_object_size_hint_weight_set(m_select_all, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
    evas_object_size_hint_align_set(m_select_all, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Evas_Object *checkbox = elm_check_add(m_select_all);
    elm_object_part_content_set(m_select_all, "elm.swallow.end", checkbox);
    evas_object_propagate_events_set(checkbox, EINA_FALSE);

    elm_object_part_text_set(m_select_all, "elm.text", _("IDS_BR_OPT_SELECT_ALL"));

    evas_object_event_callback_add(m_select_all, EVAS_CALLBACK_MOUSE_DOWN, _select_all_down, this);
    evas_object_smart_callback_add(checkbox, "changed", _select_all_state_changed, this);

    evas_object_show(m_select_all);
}

void BookmarkManagerUI::createEmptyLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_empty_layout = elm_layout_add(b_mm_layout);
    elm_layout_theme_set(m_empty_layout, "layout", "nocontents", "default");

    evas_object_size_hint_weight_set(m_empty_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_empty_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(m_empty_layout, "elm.text", _("IDS_BR_BODY_NO_BOOKMARKS"));
    elm_object_translatable_part_text_set(m_empty_layout, "elm.help.text", "After you add bookmarks, they will be shown here.");

    elm_layout_signal_emit(m_empty_layout, "text,disabled", "");
    elm_layout_signal_emit(m_empty_layout, "align.center", "elm");

    elm_object_part_content_set(b_mm_layout, "empty_layout_swallow", m_empty_layout);
}

//TODO: Make parend the argument and return created object to make code more modular.
//      (After fixing window managment)
void BookmarkManagerUI::createGengrid()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_gengrid = elm_gengrid_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid", m_gengrid);
    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
#if PROFILE_MOBILE
    elm_scroller_bounce_set(m_gengrid, EINA_FALSE, EINA_TRUE);
    elm_object_scroll_lock_x_set(m_gengrid, EINA_TRUE);
#else
    elm_object_style_set(m_gengrid, "back_ground");
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_horizontal_set(m_gengrid, EINA_TRUE);
    elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH), ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
#endif
}

void BookmarkManagerUI::createTopContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(b_mm_layout);
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

    m_topContent = elm_layout_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "top_content", m_topContent);
    evas_object_size_hint_weight_set(m_topContent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_topContent, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_topContent);
    elm_layout_theme_set(m_topContent, "naviframe", "item/basic", "default");

    m_cancel_button = elm_button_add(m_topContent);
    elm_object_part_content_set(m_topContent, "title_left_btn", m_cancel_button);
    elm_object_style_set(m_cancel_button, "naviframe/title_left");
    elm_object_text_set(m_cancel_button, _("IDS_TPLATFORM_ACBUTTON_CANCEL_ABB"));
    evas_object_smart_callback_add(m_cancel_button, "clicked", _cancel_clicked, this);
    evas_object_size_hint_weight_set(m_cancel_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_cancel_button, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_delete_button = elm_button_add(m_topContent);
    elm_object_part_content_set(m_topContent, "title_right_btn", m_delete_button);
    elm_object_style_set(m_delete_button, "naviframe/title_right");
    elm_object_text_set(m_delete_button, _("IDS_TPLATFORM_ACBUTTON_DELETE_ABB"));
    evas_object_smart_callback_add(m_delete_button, "clicked", _delete_clicked, this);
    evas_object_size_hint_weight_set(m_delete_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_delete_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

#if !PROFILE_MOBILE
void BookmarkManagerUI::createBottomContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(b_mm_layout);

    m_bottom_content = elm_layout_add(b_mm_layout);
    elm_object_part_content_set(b_mm_layout, "bottom_content", m_bottom_content);
    evas_object_size_hint_weight_set(m_bottom_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_bottom_content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_bottom_content);

    elm_layout_file_set(m_bottom_content, m_edjFilePath.c_str(), "bottom-content");
}
#endif

void BookmarkManagerUI::_close_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkManagerUI* id = static_cast<BookmarkManagerUI*>(data);
        id->closeBookmarkManagerClicked();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_cancel_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        bookmarkManagerUI->updateGenlistItems();
        bookmarkManagerUI->changeState(BookmarkManagerState::Default);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_delete_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        for (auto it = bookmarkManagerUI->m_map_delete.begin(); it != bookmarkManagerUI->m_map_delete.end(); ++it)
            if (it->second) {
                BookmarkData *bookmarkData = static_cast<BookmarkData*>(elm_object_item_data_get(bookmarkManagerUI->m_map_bookmark[it->first]));
                bookmarkManagerUI->bookmarkItemDeleted(bookmarkData->bookmarkItem);
                elm_object_item_del(bookmarkManagerUI->m_map_bookmark[it->first]);
                bookmarkManagerUI->m_map_bookmark.erase(it->first);
            }
        bookmarkManagerUI->changeState(BookmarkManagerState::Default);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_check_state_changed(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr)
    {
        BookmarkData* bookmarkData = static_cast<BookmarkData*>(data);
        bookmarkData->bookmarkManagerUI->m_delete_count -= bookmarkData->bookmarkManagerUI->
                m_map_delete[bookmarkData->bookmarkItem->getId()] ? 1 : -1;
        bookmarkData->bookmarkManagerUI->m_map_delete[bookmarkData->bookmarkItem->getId()] =
                !bookmarkData->bookmarkManagerUI->m_map_delete[bookmarkData->bookmarkItem->getId()];
        bookmarkData->bookmarkManagerUI->updateDeleteTopContent();
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_genlist_bookmark_moved(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        bookmarkManagerUI->m_reordered = true;
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_select_all_down(void *data, Evas *, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        Evas_Object *checkbox = elm_object_part_content_get(bookmarkManagerUI->m_select_all, "elm.swallow.end");
        elm_check_state_set(checkbox, !elm_check_state_get(checkbox));
        _select_all_state_changed(bookmarkManagerUI, checkbox, nullptr);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::_select_all_state_changed(void *data, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        bool state = elm_check_state_get(obj) == EINA_TRUE;
        Elm_Object_Item *it = elm_genlist_first_item_get(bookmarkManagerUI->m_genlist);
        while (it) {
            services::SharedBookmarkItem bookmarkItem = (static_cast<BookmarkData*>(elm_object_item_data_get(it)))->bookmarkItem;
            if (state != bookmarkManagerUI->m_map_delete[bookmarkItem->getId()]) {
                bookmarkManagerUI->m_delete_count -= bookmarkManagerUI-> m_map_delete[bookmarkItem->getId()] ? 1 : -1;
                bookmarkManagerUI->m_map_delete[bookmarkItem->getId()] = state;
                elm_genlist_item_update(bookmarkManagerUI-> m_map_bookmark[bookmarkItem->getId()]);
            }
            it = elm_genlist_item_next_get(it);
        }

        bookmarkManagerUI->updateDeleteTopContent();
        elm_genlist_realized_items_update(bookmarkManagerUI->m_genlist);
    } else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void BookmarkManagerUI::createFocusVector()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    m_focusManager.addItem(elm_object_part_content_get(m_topContent, "close_click"));
    m_focusManager.addItem(m_gengrid);
    m_focusManager.setIterator();
}

void BookmarkManagerUI::addBookmarkItems(std::shared_ptr<services::BookmarkItem> parent, std::vector<std::shared_ptr<services::BookmarkItem> > items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (parent && (m_folder_path.empty() || m_folder_path.back() != parent)) {
        if (parent->getParent() == -1) {
            int count = elm_toolbar_items_count(m_navigatorToolbar);
            for (int i = 0; i < count; ++i)
                elm_object_item_del(elm_toolbar_last_item_get(m_navigatorToolbar));
            m_folder_path.clear();
        }
        m_folder_path.push_back(parent);
        elm_toolbar_item_append(m_navigatorToolbar, NULL, parent->getTitle().c_str(), _navigatorFolderClicked, this);
    }
    elm_genlist_clear(m_genlist);
    m_map_bookmark.clear();
    for (auto it = items.begin(); it != items.end(); ++it) {
        BookmarkData* data = new BookmarkData();
        data->bookmarkManagerUI = this;
        data->bookmarkItem = *it;
        addBookmarkItem(data);
    }
    updateNoBookmarkText();
}

void BookmarkManagerUI::addBookmarkItemCurrentFolder(services::SharedBookmarkItem item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BookmarkData* data = new BookmarkData();
    data->bookmarkManagerUI = this;
    data->bookmarkItem = item;
    Elm_Object_Item* bookmarkView = elm_genlist_item_append(m_genlist, m_bookmark_item_class,
            data, nullptr, ELM_GENLIST_ITEM_NONE, _bookmarkItemClicked, data);
    elm_gengrid_item_selected_set(bookmarkView, EINA_FALSE);
    updateNoBookmarkText();
}

void BookmarkManagerUI::addBookmarkItem(BookmarkData* item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* bookmarkView = elm_genlist_item_append(m_genlist, m_bookmark_item_class,
            item, nullptr, ELM_GENLIST_ITEM_NONE, _bookmarkItemClicked, item);
    m_map_bookmark.insert(std::pair<unsigned int, Elm_Object_Item*>(item->bookmarkItem->getId(), bookmarkView));
    elm_genlist_item_selected_set(bookmarkView, EINA_FALSE);
}

void BookmarkManagerUI::addCustomFolder(FolderData* item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* bookmarkView;

    bookmarkView = elm_genlist_item_append(m_genlist, m_bookmark_item_class, item, nullptr, ELM_GENLIST_ITEM_NONE, _bookmarkItemClicked, item);
//    if (item->folder_id == m_all_folder_id)
//        BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_all_item_class,
//                                                            item, _bookmarkAllFolderClicked, item);
//    else if (item->folder_id == m_special_folder_id)
//        BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_mobile_item_class,
//                                                            item, _bookmarkMobileFolderClicked, item);
//    else
//        BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_custom_item_class,
//                                                            item, _bookmarkCustomFolderClicked, item);
    elm_gengrid_item_selected_set(bookmarkView, EINA_FALSE);
}

void BookmarkManagerUI::setFoldersId(unsigned int all, unsigned int special)
{
    m_all_folder_id = all;
    m_special_folder_id = special;
}

#if PROFILE_MOBILE
void BookmarkManagerUI::addNewFolder()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item* BookmarkView = elm_gengrid_item_append(m_gengrid, m_folder_new_item_class,
                                                            NULL, _bookmarkNewFolderClicked, this);
    elm_gengrid_item_selected_set(BookmarkView, EINA_FALSE);
}

void BookmarkManagerUI::orientationChanged()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    boost::optional<bool> portrait = isLandscape();

    if (portrait) {
        if (*portrait) {
            elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH_LANDSCAPE),
                                      ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT_LANDSCAPE));
        } else {
            elm_gengrid_item_size_set(m_gengrid, ELM_SCALE_SIZE(GENGRID_ITEM_WIDTH),
                                      ELM_SCALE_SIZE(GENGRID_ITEM_HEIGHT));
        }
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}
#endif
void BookmarkManagerUI::onBackPressed()
{
    switch (m_state) {
        case BookmarkManagerState::Default:
            closeBookmarkManagerClicked();
            break;
        default:
            changeState(BookmarkManagerState::Default);
            break;
    }
}

void BookmarkManagerUI::showContextMenu()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    boost::optional<Evas_Object*> window = getWindow();
    if (window) {
        if (m_state == BookmarkManagerState::Default) {
            createContextMenu(*window);

            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_SK_DELETE"), nullptr, _cm_delete_clicked, this);
            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_OPT_SHARE"), nullptr, _cm_share_clicked, this);
            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_OPT_REORDER_ABB"), nullptr, _cm_reorder_clicked, this);
            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_BUTTON_EDIT"), nullptr, _cm_edit_clicked, this);
            elm_ctxpopup_item_append(m_ctxpopup, _("IDS_BR_SK3_CREATE_FOLDER"), nullptr, _cm_create_folder_clicked, this);

            alignContextMenu(*window);
        }
    } else
        BROWSER_LOGE("[%s:%d] Signal not found", __PRETTY_FUNCTION__, __LINE__);
}

void BookmarkManagerUI::_cm_delete_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
        bookmarkManagerUI->changeState(BookmarkManagerState::Delete);
    }
}

void BookmarkManagerUI::_cm_share_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
    }
}

void BookmarkManagerUI::_cm_reorder_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
        bookmarkManagerUI->changeState(BookmarkManagerState::Reorder);
    }
}

void BookmarkManagerUI::_cm_edit_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
        bookmarkManagerUI->changeState(BookmarkManagerState::Edit);
    }
}

void BookmarkManagerUI::_cm_create_folder_clicked(void* data, Evas_Object*, void* )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        _cm_dismissed(nullptr, bookmarkManagerUI->m_ctxpopup, nullptr);
        bookmarkManagerUI->newFolderItemClicked(bookmarkManagerUI->m_folder_path.back()->getId());
    }
}

void BookmarkManagerUI::addCustomFolders(services::SharedBookmarkFolderList folders)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (auto it = folders.begin(); it != folders.end(); ++it) {
        FolderData *folderData = new FolderData();
        folderData->name = (*it)->getName();
        folderData->folder_id = (*it)->getId();
        folderData->count = (*it)->getCount();
        folderData->bookmarkManagerUI = this;
        addCustomFolder(folderData);
    }
    elm_object_part_content_set(b_mm_layout, "elm.swallow.grid", m_genlist);
    evas_object_show(m_genlist);
#if !PROFILE_MOBILE
    elm_object_part_text_set(m_bottom_content, "text", (boost::format("%d %s") % elm_gengrid_items_count(m_gengrid) % "folders").str().c_str());
#endif
}

void BookmarkManagerUI::addCustomFolders(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > items)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    for (auto it = items.begin(); it != items.end(); ++it) {
        if ((*it)->is_folder()) {
            FolderData *folderData = new FolderData();
            int count = 0;

            for (auto it2 = items.begin(); it2 != items.end(); ++it2) {
                BROWSER_LOGD("%d %d %d",(*it2)->is_folder(),(*it2)->getParent(),(*it)->getId());
                if (!(*it2)->is_folder() && (*it2)->getParent()==(*it)->getId()) {
                    count++;
                }
            }
            folderData->name = (*it)->getTitle();
            folderData->count = count;
            folderData->folder_id = (*it)->getId();
            folderData->bookmarkManagerUI = this;
            addCustomFolder(folderData);
        }
    }
}

void BookmarkManagerUI::_navigatorFolderClicked(void* data, Evas_Object*, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkManagerUI *bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
        Elm_Object_Item *clicked = static_cast<Elm_Object_Item*>(event_info);
        Elm_Object_Item *it = elm_toolbar_last_item_get(bookmarkManagerUI->m_navigatorToolbar);
        if (clicked == it)
            return;
        while (clicked != it) {
            Elm_Object_Item *it_prev = elm_toolbar_item_prev_get(it);
            elm_object_item_del(it);
            bookmarkManagerUI->m_folder_path.pop_back();
            it = it_prev;
        }
        bookmarkManagerUI->bookmarkItemClicked(bookmarkManagerUI->m_folder_path.back());
    }
}

void BookmarkManagerUI::_bookmarkItemClicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        BookmarkData *bookmarkData = static_cast<BookmarkData*>(data);
        switch (bookmarkData->bookmarkManagerUI->m_state) {
        case BookmarkManagerState::Default:
            bookmarkData->bookmarkManagerUI->bookmarkItemClicked(bookmarkData->bookmarkItem);
            break;
        case BookmarkManagerState::Edit:
            bookmarkData->bookmarkManagerUI->bookmarkItemEdit(bookmarkData->bookmarkItem);
            bookmarkData->bookmarkManagerUI->changeState(BookmarkManagerState::Default);
            break;
        case BookmarkManagerState::Delete:
            bookmarkData->bookmarkManagerUI->m_delete_count -= bookmarkData->bookmarkManagerUI->
                    m_map_delete[bookmarkData->bookmarkItem->getId()] ? 1 : -1;
            bookmarkData->bookmarkManagerUI->m_map_delete[bookmarkData->bookmarkItem->getId()] =
                    !bookmarkData->bookmarkManagerUI->m_map_delete[bookmarkData->bookmarkItem->getId()];
            bookmarkData->bookmarkManagerUI->updateDeleteTopContent();
            elm_genlist_item_update(bookmarkData->bookmarkManagerUI->
                    m_map_bookmark[bookmarkData->bookmarkItem->getId()]);
            elm_genlist_realized_items_update(bookmarkData->bookmarkManagerUI->m_genlist);
            break;
        default:
            break;
        }
    }
}

void BookmarkManagerUI::_bookmarkCustomFolderClicked(void * data , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        FolderData * itemData = (FolderData*)(data);
        BROWSER_LOGD("Folder Name: %s" , itemData->name.c_str());
        itemData->bookmarkManagerUI->customFolderClicked(itemData->folder_id);
    }
}
#if PROFILE_MOBILE
void BookmarkManagerUI::_bookmarkNewFolderClicked(void * /*data*/, Evas_Object *, void *)
{
//    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
//    if (data != nullptr)
//    {
//        BookmarkManagerUI* bookmarkManagerUI = static_cast<BookmarkManagerUI*>(data);
//        bookmarkManagerUI->newFolderItemClicked();
//    }
}
#endif
void BookmarkManagerUI::_bookmarkAllFolderClicked(void * data , Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        FolderData * itemData = (FolderData*)(data);
        itemData->bookmarkManagerUI->allFolderClicked();
    }
}

void BookmarkManagerUI::_bookmarkMobileFolderClicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        FolderData * itemData = (FolderData*)(data);
        itemData->bookmarkManagerUI->specialFolderClicked();
    }
}

void BookmarkManagerUI::changeState(BookmarkManagerState state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_state = state;
    switch (state) {
    case BookmarkManagerState::Edit:
        m_reordered = false;
        elm_object_signal_emit(m_topContent, "elm,state,title_left_btn,hide", "elm");
        elm_object_signal_emit(m_topContent, "elm,state,title_right_btn,hide", "elm");
        elm_object_part_text_set(m_topContent, "elm.text.title", _("IDS_BR_HEADER_SELECT_BOOKMARK"));
        elm_object_signal_emit(b_mm_layout, "hide_toolbar", "ui");
        break;
    case BookmarkManagerState::Delete:
        m_delete_count = 0;
        m_map_delete.clear();
        for (auto it = m_map_bookmark.begin(); it != m_map_bookmark.end(); ++it)
            m_map_delete.insert(std::pair<unsigned int, bool>(it->first, false));

        elm_genlist_realized_items_update(m_genlist);
        elm_object_signal_emit(m_topContent, "elm,state,title_left_btn,show", "elm");
        elm_object_signal_emit(m_topContent, "elm,state,title_right_btn,show", "elm");
        updateDeleteTopContent();
        elm_object_signal_emit(b_mm_layout, "hide_toolbar", "ui");
        elm_check_state_set(elm_object_part_content_get(m_select_all, "elm.swallow.end"), EINA_FALSE);
        elm_box_pack_start(m_box, m_select_all);
        evas_object_show(m_select_all);
        break;
    case BookmarkManagerState::Reorder:
        elm_object_signal_emit(m_topContent, "elm,state,title_left_btn,hide", "elm");
        elm_object_signal_emit(m_topContent, "elm,state,title_right_btn,hide", "elm");
        elm_object_part_text_set(m_topContent, "elm.text.title", _("IDS_BR_OPT_REORDER_ABB"));
        elm_object_signal_emit(b_mm_layout, "hide_toolbar", "ui");
        elm_genlist_reorder_mode_set(m_genlist, EINA_TRUE);
        elm_genlist_decorate_mode_set(m_genlist, EINA_FALSE);
        break;
    case BookmarkManagerState::Default:
    default:
        updateNoBookmarkText();
        reoderBookmarkItems();
        elm_object_signal_emit(m_topContent, "elm,state,title_left_btn,hide", "elm");
        elm_object_signal_emit(m_topContent, "elm,state,title_right_btn,hide", "elm");
        elm_object_signal_emit(b_mm_layout, "show_toolbar", "ui");
        elm_object_part_text_set(m_topContent, "elm.text.title", _("IDS_BR_BODY_BOOKMARKS"));
        elm_genlist_reorder_mode_set(m_genlist, EINA_FALSE);
        elm_genlist_decorate_mode_set(m_genlist, EINA_TRUE);
        elm_box_unpack(m_box, m_select_all);
        evas_object_hide(m_select_all);
        break;
    }
    elm_genlist_realized_items_update(m_genlist);
}

void BookmarkManagerUI::updateGenlistItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (auto it = m_map_delete.begin(); it != m_map_delete.end(); ++it)
        if (it->second) {
            it->second = false;
            elm_genlist_item_update(m_map_bookmark[it->first]);
        }
}

void BookmarkManagerUI::reoderBookmarkItems()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_reordered) {
        m_reordered = false;
        Elm_Object_Item *it = elm_genlist_first_item_get(m_genlist);
        BookmarkData *bookmarkData;
        int order = 1;
        while (it) {
            bookmarkData = static_cast<BookmarkData*>(elm_object_item_data_get(it));
            if (order !=  bookmarkData->bookmarkItem->getOrder()) {
                bookmarkData->bookmarkItem->setOrder(order);
                bookmarkItemOrderEdited(bookmarkData->bookmarkItem);
            }
            it = elm_genlist_item_next_get(it);
            order++;
        }
    }
}

void BookmarkManagerUI::updateNoBookmarkText()
{
    if (m_map_bookmark.size()) {
        evas_object_hide(m_empty_layout);
        elm_object_signal_emit(b_mm_layout, "hide_empty_layout", "ui");
    } else {
        evas_object_show(m_empty_layout);
        elm_object_signal_emit(b_mm_layout, "show_empty_layout", "ui");
    }
}

void BookmarkManagerUI::updateDeleteTopContent()
{
    if (m_delete_count) {
        elm_object_part_text_set(m_topContent, "elm.text.title",
                (boost::format(_("IDS_BR_HEADER_PD_SELECTED_ABB")) % m_delete_count).str().c_str());
        elm_object_signal_emit(m_delete_button, "elm,state,enabled", "elm");
    } else {
        elm_object_part_text_set(m_topContent, "elm.text.title", "Select Items");
        //TODO: Add translation
        elm_object_signal_emit(m_delete_button, "elm,state,disabled", "elm");
    }
}

}
}
