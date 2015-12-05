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

#ifndef AUTOFILLFORMLISTVIEW_H_
#define AUTOFILLFORMLISTVIEW_H_

class auto_fill_form_compose_view;
class auto_fill_form_list_view;
class auto_profile_delete_view;
class auto_fill_form_item;
class auto_fill_form_manager;

class auto_fill_form_list_view {
public:
	auto_fill_form_list_view(auto_fill_form_manager *affm);
	~auto_fill_form_list_view(void);

	Eina_Bool show(Evas_Object *parent);
	void hide();
	void refresh_view(void);
private:
	typedef struct _genlist_callback_data {
		unsigned int menu_index;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_genlist(Evas_Object *parent);
	Eina_Bool _create_genlist_style_items(void);
	const char *_get_each_item_full_name(unsigned int index);
	Eina_Bool _append_genlist(Evas_Object *genlist);
	void _remove_each_item_callback_data(void);
	auto_profile_delete_view *_get_delete_view(void);
	Eina_Bool _is_in_list_view(void);
	void _show_more_context_popup(void);
	static char *__text_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_profile_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_auto_fill_form_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_edit_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static void __change_autofillform_check_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_delete_view_pop_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_profile_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_item_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_menu_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __resize_more_ctxpopup(void *data);
	static void __context_popup_back_cb(void *data, Evas_Object *obj, void 	*event_info);
	static void __back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info);

	auto_fill_form_manager *m_manager;
	Evas_Object *m_genlist;
	Evas_Object *m_main_layout;
	Elm_Genlist_Item_Class *m_item_class;
	std::string m_edjFilePath;
};

#endif /* AUTOFILLFORMLISTVIEW_H_ */
