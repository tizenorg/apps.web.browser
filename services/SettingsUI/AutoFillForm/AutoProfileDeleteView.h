/*
 * AutoFillFormDeleteView.h
 *
 *  Created on: Nov 16, 2015
 *      Author: youngj
 */

#ifndef AUTOFILLFORMDELETEVIEW_H_
#define AUTOFILLFORMDELETEVIEW_H_

#include "common.h"
#include "AutoFillForm.h"

class auto_profile_delete_view {//: public common_view {
public:
	auto_profile_delete_view(void);
	~auto_profile_delete_view(void);
	void show(void);
private:
	typedef enum _menu_type
	{
		DELETE_PROFILE_SELECT_ALL = 0,
		DELETE_PROFILE_ITEM
	} menu_type;
	typedef struct _genlist_callback_data{
		menu_type type;
		unsigned int menu_index;
		void *user_data;
		Eina_Bool is_checked;
		Evas_Object *checkbox;
		Elm_Object_Item *it;
	} genlist_callback_data;
	typedef struct _profile_item_genlist_callback_data {
		genlist_callback_data cd;
		Evas_Object *checkbox;
	} profile_item_genlist_callback_data;

#if defined(DELETE_CONFIRM_POPUP_ENABLE)
	void _set_popup_text();
#endif
	void _set_selected_title(void);
	void _unset_selected_title(void);
	void _back_delete_view(void);
	const char *_get_each_item_full_name(unsigned int index);
	Evas_Object *_create_auto_form_delete_layout(Evas_Object *parent);
	void _remove_each_item_callback_data(void);
	Evas_Object *_create_box(Evas_Object *parent);
#if !defined(DELETE_CONFIRM_POPUP_ENABLE)
	void _item_delete(void);
	void _items_delete_all(void);
#endif
	static char *__text_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static Eina_Bool __naviframe_pop_cb(void *data, Elm_Object_Item *it);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_item_cb(void *data, Evas_Object *obj, void *event_info);
	static void __checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_click_cb(void *data, Evas_Object *obj, void *event_info);
#if defined(DELETE_CONFIRM_POPUP_ENABLE)
	static void __popup_delete_all_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_destroy_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_delete_item_cb(void *data, Evas_Object *obj, void *event_info);
	static void __auto_profile_delete_popup_lang_changed_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static void __auto_profile_delete_title_lang_changed_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_parent;
	Evas_Object *m_del_layout;
	Evas_Object *m_del_genlist;
	Evas_Object *m_button_done;
	Evas_Object *m_auto_fill_form_check;
	Evas_Object *m_box;
	Evas_Object *m_select_all_layout;
#if defined(DELETE_CONFIRM_POPUP_ENABLE)
	Evas_Object *m_delete_popup;
#endif
	//Elm_Object_Item *m_naviframe_item;
	Eina_Bool m_select_all_flag;
	unsigned int m_count_checked_item;
	genlist_callback_data *m_item_callback_data;
	genlist_callback_data *m_select_all_callback_data;
	std::string m_sub_title;
	std::vector<genlist_callback_data *> m_auto_fill_form_item_callback_data_list;
};




#endif /* AUTOFILLFORMDELETEVIEW_H_ */
