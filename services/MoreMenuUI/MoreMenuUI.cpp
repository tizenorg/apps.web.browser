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

#include "MoreMenuUI.h"
#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "Tools/EflTools.h"
#include "../Tools/BrowserImage.h"

#define efl_scale       (elm_config_scale_get() / elm_app_base_scale_get())

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(MoreMenuUI, "org.tizen.browser.moremenuui")

struct ItemData{
        tizen_browser::base_ui::MoreMenuUI * m_moreMenu;
        tizen_browser::services::HistoryItem * h_item;
        Elm_Object_Item * e_item;
    };

typedef struct _MoreItemData
{
	ItemType item;
	std::shared_ptr<tizen_browser::base_ui::MoreMenuUI> moreMenuUI;
} MoreMenuItemData;

MoreMenuUI::MoreMenuUI()
    : m_gengrid(NULL)
    , m_parent(NULL)
    , m_item_class(NULL)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    edjFilePath = EDJE_DIR;
    edjFilePath.append("MoreMenuUI/MoreMenu.edj");
}

MoreMenuUI::~MoreMenuUI()
{
}

void MoreMenuUI::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
    elm_theme_extension_add(NULL, edjFilePath.c_str());
    m_mm_layout = elm_layout_add(parent);
    elm_layout_file_set(m_mm_layout, edjFilePath.c_str(), "moremenu-layout");
    evas_object_size_hint_weight_set(m_mm_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_mm_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_mm_layout);

    m_gengrid = elm_gengrid_add(m_mm_layout);
    elm_object_part_content_set(m_mm_layout, "elm.swallow.grid", m_gengrid);

    /*evas_object_smart_callback_add(m_gengrid, "item,focused", focusItem, NULL);
    evas_object_smart_callback_add(m_gengrid, "item,unfocused", unFocusItem, NULL);
    evas_object_smart_callback_add(m_gengrid, "activated", _itemSelected, this);*/

      if (!m_item_class) {
            m_item_class = elm_gengrid_item_class_new();
            m_item_class->item_style = "menu_item";
            m_item_class->func.text_get = _grid_text_get;
            m_item_class->func.content_get =  _grid_content_get;
            m_item_class->func.state_get = NULL;
            m_item_class->func.del = NULL;
        }

    elm_gengrid_align_set(m_gengrid, 0, 0);
    elm_gengrid_select_mode_set(m_gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_multi_select_set(m_gengrid, EINA_FALSE);
    elm_gengrid_horizontal_set(m_gengrid, EINA_FALSE);
    elm_scroller_policy_set(m_gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_size_set(m_gengrid, 0, 327);
    evas_object_size_hint_weight_set(m_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_gengrid_item_size_set(m_gengrid, 364 * efl_scale, 320 * efl_scale);

    addItems();
}

void MoreMenuUI::showCurrentTab(const std::shared_ptr<tizen_browser::services::HistoryItem> item)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_theme_extension_add(NULL, edjFilePath.c_str());
    m_genList = elm_genlist_add(m_mm_layout);
    elm_object_part_content_set(m_mm_layout, "elm.swallow.genlist", m_genList);
    elm_genlist_homogeneous_set(m_genList, EINA_FALSE);
    elm_genlist_multi_select_set(m_genList, EINA_FALSE);
    elm_genlist_select_mode_set(m_genList, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genList, ELM_LIST_LIMIT);
    elm_genlist_decorate_mode_set(m_genList, EINA_TRUE);
    evas_object_size_hint_weight_set(m_genList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    /*evas_object_smart_callback_add(m_genList, "item,focused", focusItem, this);
    evas_object_smart_callback_add(m_genList, "item,unfocused", unFocusItem, NULL);*/

    m_itemClass = elm_genlist_item_class_new();
    m_itemClass->item_style = "current_tab";
    m_itemClass->func.text_get = &listItemTextGet;
    m_itemClass->func.content_get = &listItemContentGet;
    m_itemClass->func.state_get = 0;
    m_itemClass->func.del = 0;

    ItemData * id = new ItemData;
    id->m_moreMenu = this;
    id->h_item = item ? item.get() : NULL;
    Elm_Object_Item* elmItem = elm_genlist_item_append(m_genList,            //genlist
                                                      m_itemClass,          //item Class
                                                      id,
                                                      NULL,                    //parent item
                                                      ELM_GENLIST_ITEM_NONE,  //item type
                                                      NULL,
                                                      NULL                  //data passed to above function
                                                     );
    id->e_item = elmItem;
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

Evas_Object* MoreMenuUI::listItemContentGet(void* data, Evas_Object* obj, const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData * id = static_cast<ItemData *>(data);
    if(!strcmp(part, "favicon") && id->h_item && id->h_item->getFavIcon())
    {
	 // Currently favicon is not getting fetched from the engine, so we are showing Google's favicon by default.
	 Evas_Object* thumb_nail = elm_icon_add(obj);
         const char* file_name = "favicon.png";
         elm_image_file_set(thumb_nail, id->m_moreMenu->edjFilePath.c_str(), file_name);
         return thumb_nail;
         //return tizen_browser::tools::EflTools::getEvasImage(id->h_item->getFavIcon(), obj);
    }
    else if(!strcmp(part, "star_click"))
    {
        Evas_Object *star_click = elm_button_add(obj);
        elm_object_style_set(star_click, "hidden_button");
        evas_object_smart_callback_add(star_click, "clicked", MoreMenuUI::star_clicked_cb, id);
        return star_click;
    }
    else if(!strcmp(part, "close_click"))
    {
        Evas_Object *close_click = elm_button_add(obj);
        elm_object_style_set(close_click, "hidden_button");
        evas_object_smart_callback_add(close_click, "clicked", MoreMenuUI::close_clicked_cb, id);
        return close_click;
    }
    return NULL;
}

void MoreMenuUI::item_clicked_cb(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void MoreMenuUI::star_clicked_cb(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData * id = static_cast<ItemData *>(data);
    id->m_moreMenu->AddBookmarkPopupCalled();
}

void MoreMenuUI::AddBookmarkPopupCalled()
{   //m_map_bookmark_folder_list.clear();
    popup = std::shared_ptr<tizen_browser::base_ui::AddBookmarkPopup> (new AddBookmarkPopup(m_mm_layout));
    popup->show();
    popup->addBookmarkFolderItems(m_map_bookmark_folder_list);
    popup->folderSelected.disconnect_all_slots();
    popup->folderSelected.connect(boost::bind(&MoreMenuUI::addToBookmarks, this, _1));
    popup->addNewFolderClicked.disconnect_all_slots();
    popup->addNewFolderClicked.connect(boost::bind(&MoreMenuUI::newFolderPopup, this,_1));
}

void MoreMenuUI::newFolderPopup(std::string)
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   m_popup = std::shared_ptr <tizen_browser::base_ui::NewFolderPopup > (new NewFolderPopup(m_mm_layout, NULL,"Add New Folder for adding to Bookmark","New Folder","Add to bookmark","Cancel"));
   m_popup->on_ok.disconnect_all_slots();
   m_popup->on_ok.connect(boost::bind(&MoreMenuUI::NewFolderCreate, this, _1));
   m_popup->on_cancel.disconnect_all_slots();
   m_popup->on_cancel.connect(boost::bind(&MoreMenuUI::CancelClicked, this, _1));
   popup->hide();
   m_popup->show();
}

void MoreMenuUI::NewFolderCreate(Evas_Object * popup_content)
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   m_folderName = elm_entry_entry_get(popup_content);
   BookmarkFolderCreated(m_folderName.c_str(), 0,0);
   m_popup->hide();
}

void MoreMenuUI::CancelClicked(Evas_Object * popup)
{
   BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
   m_popup->hide();
}

void MoreMenuUI::addToBookmarks(int folder_id)
{
     AddBookmarkInput(folder_id);
     BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
     popup->hide();
     popup.reset();
}
void MoreMenuUI::close_clicked_cb(void* data, Evas_Object* /* obj */, void* /* event_info */)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData * id = static_cast<ItemData *>(data);
    id->m_moreMenu->closeMoreMenuClicked(std::string());
    id->m_moreMenu->clearItems();
}

char* MoreMenuUI::listItemTextGet(void* data, Evas_Object* /* obj */, const char* part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    ItemData * id = static_cast<ItemData *>(data);
    if(!strcmp(part, "webpage_title"))
    {
	if(!id->h_item)
	    return strdup("New Tab");
        if(!id->h_item->getTitle().empty()){
            return strdup(id->h_item->getTitle().c_str());
        }
    }
    else if(!strcmp(part, "webpage_url"))
    {
	if(!id->h_item)
            return strdup("");
        if(!id->h_item->getUrl().empty()){
            return strdup(id->h_item->getUrl().c_str());
        }
    }
    return strdup("");
}

void MoreMenuUI::hide()
{
	evas_object_hide(elm_layout_content_get(m_mm_layout, "elm.swallow.grid"));
	evas_object_hide(elm_layout_content_get(m_mm_layout, "elm.swallow.genlist"));
	evas_object_hide(m_mm_layout);
}

void MoreMenuUI::addItems()
{
	 BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
	 for (size_t i = 0; i < 10; i++) {
	     MoreMenuItemData *itemData = new MoreMenuItemData();
	     itemData->item = static_cast<ItemType>(i);;
	     itemData->moreMenuUI = std::shared_ptr<tizen_browser::base_ui::MoreMenuUI>(this);
	     Elm_Object_Item* bookmarkView = elm_gengrid_item_append(m_gengrid, m_item_class, itemData, NULL, this);
	     m_map_menu_views.insert(std::pair<ItemType, Elm_Object_Item*>(itemData->item, bookmarkView));
	     elm_gengrid_item_selected_set(bookmarkView, EINA_FALSE);
	 }
	 BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
}

char* MoreMenuUI::_grid_text_get(void *data, Evas_Object *obj, const char *part)
{
	MoreMenuItemData *itemData = reinterpret_cast<MoreMenuItemData*>(data);

        BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
	if (!strcmp(part, "menu_label")) {
                const char* item_name = NULL;
                switch(itemData->item) {
                case READER_MODE:
                        item_name = "Reader mode"; break;
                case BOOKMARK_MANAGER:
                        item_name = "Bookmark manager"; break;
                case HISTORY:
                        item_name = "History"; break;
                case SCREEN_ZOOM:
                        item_name = "Screen zoom"; break;
                case START_MINIBROWSER:
                        item_name = "Start minibrowser"; break;
                case FOCUS_MODE:
                        item_name = "Focus mode"; break;
                case VIEW_MOBILE_WEB:
                        item_name = "View mobile web"; break;
                case SHARE:
                        item_name = "Share"; break;
                case SETTINGS:
                        item_name = "Settings"; break;
                case EXIT_BROWSER:
                        item_name = "Exit browser"; break;
                default:
                        item_name = "";
                }
		return strdup(item_name);
	}
	return NULL;
}

static const char* getImageFileNameForType(ItemType type, bool focused)
{
	const char* file_name = NULL;
         switch(type) {
                case READER_MODE:
                        file_name = focused ? "ic_more_readermode_foc.png" : "ic_more_readermode_nor.png"; break;
                case BOOKMARK_MANAGER:
                        file_name = focused ? "ic_more_bookmark_foc.png" : "ic_more_bookmark_nor.png"; break;
                case HISTORY:
                        file_name = focused ? "ic_more_history_foc.png" : "ic_more_history_nor.png"; break;
                case SCREEN_ZOOM:
                        file_name = focused ? "ic_more_zoom_foc.png" : "ic_more_zoom_nor.png"; break;
                case START_MINIBROWSER:
                        file_name = focused ? "ic_more_minibrowser_foc.png" : "ic_more_minibrowser_nor.png"; break;
                case FOCUS_MODE:
                        file_name = focused ? "ic_more_focusmode_foc.png" : "ic_more_focusmode_nor.png"; break;
                case VIEW_MOBILE_WEB:
                        file_name = focused ? "ic_more_mobileview_foc.png" : "ic_more_mobileview_nor.png"; break;
                case SHARE:
                        file_name = focused ? "ic_more_share_foc.png" : "ic_more_share_nor.png"; break;
                case SETTINGS:
                        file_name = focused ? "ic_more_setting_foc.png" : "ic_more_setting_nor.png"; break;
                case EXIT_BROWSER:
                        file_name = focused ? "ic_more_exit_foc.png" : "ic_more_exit_nor.png"; break;
                default:
                        file_name = "";
         }
	return file_name;
}

Evas_Object * MoreMenuUI::_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("%s:%d %s part=%s", __FILE__, __LINE__, __func__, part);
    MoreMenuItemData *itemData = reinterpret_cast<MoreMenuItemData*>(data);

    if (!strcmp(part, "thumbnail_item")) {
	 Evas_Object* thumb_nail = elm_icon_add(obj);
	 const char* file_name = getImageFileNameForType(itemData->item, false);
	 elm_image_file_set(thumb_nail, itemData->moreMenuUI->edjFilePath.c_str(), file_name);
	 return thumb_nail;
    }
    else if (!strcmp(part, "thumbbutton_item")) {
		Evas_Object *thumbButton = elm_button_add(obj);
		elm_object_style_set(thumbButton, "clickButton");
		evas_object_smart_callback_add(thumbButton, "clicked", tizen_browser::base_ui::MoreMenuUI::_thumbSelected, data);
		evas_object_event_callback_add(thumbButton, EVAS_CALLBACK_MOUSE_IN, __cb_mouse_in, data);
		evas_object_event_callback_add(thumbButton, EVAS_CALLBACK_MOUSE_OUT, __cb_mouse_out, data);
		return thumbButton;
    }
    return NULL;
}

void MoreMenuUI::__cb_mouse_in(void * data, Evas *e, Evas_Object *obj, void *event_info)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    elm_object_focus_set(obj, EINA_TRUE);
    MoreMenuItemData *itemData = reinterpret_cast<MoreMenuItemData*>(data);

    const char* file_name = getImageFileNameForType(itemData->item, true);
    Elm_Object_Item * selected = itemData->moreMenuUI->m_map_menu_views[itemData->item];
    Evas_Object *thumb_nail = elm_object_item_part_content_get(selected, "thumbnail_item");
    elm_image_file_set(thumb_nail, itemData->moreMenuUI->edjFilePath.c_str(), file_name);
}

void MoreMenuUI::__cb_mouse_out(void * data, Evas *e, Evas_Object *obj, void *event_info)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    MoreMenuItemData *itemData = reinterpret_cast<MoreMenuItemData*>(data);
    elm_object_focus_set(obj, EINA_FALSE);
    const char* file_name = getImageFileNameForType(itemData->item, false);
    Elm_Object_Item * selected = itemData->moreMenuUI->m_map_menu_views[itemData->item];
    Evas_Object *thumb_nail = elm_object_item_part_content_get(selected, "thumbnail_item");
    elm_image_file_set(thumb_nail, itemData->moreMenuUI->edjFilePath.c_str(), file_name);
}

void MoreMenuUI::_itemSelected(void * data, Evas_Object * /* obj */, void * event_info)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
}

void MoreMenuUI::_thumbSelected(void * data, Evas_Object * /* obj */, void * /* event_info */)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    MoreMenuItemData *itemData = reinterpret_cast<MoreMenuItemData*>(data);
    switch(itemData->item) {
                case READER_MODE:
                        break;
                case BOOKMARK_MANAGER:
                        break;
                case HISTORY:
                        itemData->moreMenuUI->historyUIClicked(std::string()); break;
                case SCREEN_ZOOM:
			break;
                case START_MINIBROWSER:
                        break;
                case FOCUS_MODE:
                        break;
                case VIEW_MOBILE_WEB:
                        break;
                case SHARE:
                        break;
                case SETTINGS:
                        itemData->moreMenuUI->settingsClicked(std::string()); break;
                case EXIT_BROWSER:
                        break;
    }
}

void MoreMenuUI::getBookmarkFolderList(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> > folderList)
{
    m_map_bookmark_folder_list = folderList;
}

void MoreMenuUI::clearItems()
{
    hide();
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_gengrid_clear(m_gengrid);
    elm_genlist_clear(m_genList);
    m_map_menu_views.clear();
    m_map_bookmark_folder_list.clear();
    elm_theme_extension_del(NULL, edjFilePath.c_str());
    elm_theme_full_flush();
    elm_cache_all_flush();
}

void MoreMenuUI::focusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,in", "over2");

    // selected manually
    elm_gengrid_item_selected_set(item, EINA_TRUE);
}

void MoreMenuUI::unFocusItem(void* /*data*/, Evas_Object* /*obj*/, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Elm_Object_Item *item = reinterpret_cast<Elm_Object_Item*>(event_info);
    elm_object_item_signal_emit( item, "mouse,out", "over2");

    // unselected manually
    elm_gengrid_item_selected_set(item, EINA_FALSE);
}

}
}
