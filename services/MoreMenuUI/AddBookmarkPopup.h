#ifndef ADDBOOKMARKPOPUP_H
#define ADDBOOKMARKPOPUP_H

#include <Elementary.h>
#include <boost/signals2.hpp>

#include "AbstractUIComponent.h"
#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"
#include "BookmarkItem.h"

namespace tizen_browser{
namespace base_ui{

class AddBookmarkPopup{

public:
    AddBookmarkPopup(Evas_Object *main_layout);
    ~AddBookmarkPopup();

    Evas_Object *getContent();
    void addBookmarkFolderItem(std::shared_ptr<tizen_browser::services::BookmarkItem> , int);
    void addBookmarkFolderItems(std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem> >);
    void clearItems();
    void updateGengrid();
    Evas_Object* createNoHistoryLabel();
    void setEmptyGengrid(bool setEmpty);

    boost::signals2::signal<void (std::string)> blockClicked;
    boost::signals2::signal<void (int)> folderSelected;
    boost::signals2::signal<void (std::string)> addNewFolderClicked;
    /**
     * Theese setters should be called before showing popup.
     */
    void show();
    void hide();

private:
    static char* _grid_text_get(void *data, Evas_Object *, const char *part);
    static Evas_Object * _grid_content_get(void *data, Evas_Object *obj, const char *part);
    static void _itemSelected(void * data, Evas_Object * obj, void * event_info);
    static void _thumbSelected(void * data, Evas_Object * obj, void * event_info);
    static void _newFolderButton(void *data, Evas_Object *obj, void *event_info);
    static Evas_Object* listItemContentGet(void *data, Evas_Object *obj, const char *part);
    static char*        listItemTextGet(void *data, Evas_Object *obj, const char *part);

    static void item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
    static void close_clicked_cb(void *data, Evas_Object *obj, void *event_info);

    static const char* getImageFileNameForType(int index, bool focused);

    Evas_Object *m_mainLayout;
    Evas_Object *m_popup;
    Evas_Object *m_gengrid;

    Elm_Gengrid_Item_Class * m_itemClass;
    std::map<std::string,Elm_Object_Item*> m_map_bookmark_folder_views;
    std::string bm_edjFilePath;
    bool m_gengridSetup;

    static void __cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
    static void __cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);

    static void focusItem(void* data, Evas_Object* obj, void* event_info);
    static void unFocusItem(void* data, Evas_Object* obj, void* event_info);
};
}
}
#endif
