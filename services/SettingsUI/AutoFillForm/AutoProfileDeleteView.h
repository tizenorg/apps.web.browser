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

#ifndef AUTOFILLFORMDELETEVIEW_H_
#define AUTOFILLFORMDELETEVIEW_H_

class auto_fill_form_manager;
class auto_profile_delete_view {
public:
	auto_profile_delete_view(auto_fill_form_manager* manager);
	~auto_profile_delete_view(void);
	void show(Evas_Object *parent);
	void hide();
private:
	typedef struct _genlist_callback_data{
		unsigned int menu_index;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Evas_Object *_create_main_layout(Evas_Object *parent);
        Evas_Object *_create_genlist(Evas_Object *parent);
        Eina_Bool _append_genlist(Evas_Object *genlist);

	void _set_selected_title(void);
	void _unset_selected_title(void);
	const char *_get_each_item_full_name(unsigned int index);
	void _item_delete_selected(void);
	void _items_delete_all(void);
	static char *__text_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_selected_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_all_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info);

        auto_fill_form_manager* m_manager;
	Evas_Object *m_main_layout;
	Evas_Object *m_genlist;
        Elm_Genlist_Item_Class *m_item_class;

        std::string m_edjFilePath;
};

#endif /* AUTOFILLFORMDELETEVIEW_H_ */
