/*
 * ListViewItemClickedPopup.h
 *
 *  Created on: Nov 16, 2015
 *      Author: youngj
 */

#ifndef LISTVIEWITEMCLICKEDPOPUP_H_
#define LISTVIEWITEMCLICKEDPOPUP_H_

#include "common.h"
#include "AutoFillForm.h"

class auto_fill_form_item;

class list_view_item_clicked_popup : public common_view {
public:
	list_view_item_clicked_popup(auto_fill_form_item *item);
	~list_view_item_clicked_popup(void);

	Eina_Bool show(void);
private:
	typedef enum _menu_type
	{
		selected_item_edit = 0,
		selected_item_delete,
		selected_item_num
	} menu_type;

	Evas_Object *_create_genlist(Evas_Object *parent);

	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static void __popup_edit_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_ok_confirm_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_popup;
	auto_fill_form_item *m_item;
	Elm_Object_Item *m_popup_last_item;

protected:
};


#endif /* LISTVIEWITEMCLICKEDPOPUP_H_ */
