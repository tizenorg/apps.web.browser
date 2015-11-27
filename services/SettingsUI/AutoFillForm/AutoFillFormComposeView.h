#ifndef AUTOFILLFORMCOMPOSEVIEW_H_
#define AUTOFILLFORMCOMPOSEVIEW_H_

#include "AutoFillFormItem.h"

class auto_fill_form_manager;
class auto_fill_form_compose_view {
public:
	auto_fill_form_compose_view(auto_fill_form_manager* manager, auto_fill_form_item *item = NULL);
	~auto_fill_form_compose_view(void);

	void show(void);
private:
	typedef enum _menu_type
	{
		profile_composer_title_full_name = 0,
		profile_composer_title_company_name,
		profile_composer_title_address_line_1,
		profile_composer_title_address_line_2,
		profile_composer_title_city_town,
		profile_composer_title_county_region,
		profile_composer_title_post_code,
		profile_composer_title_country,
		profile_composer_title_phone,
		profile_composer_title_email,

		profile_composer_menu_end
	} menu_type;

	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		Evas_Object *entry;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_scroller(Evas_Object * parent);
	Evas_Object *_create_box(Evas_Object * parent);
	void _create_list_items(Evas_Object * parent);
	Evas_Object *_create_genlist(Evas_Object *parent);
	Eina_Bool _create_genlist_style_items(void);
	Eina_Bool _destroy_all_genlist_style_items(void);
	Eina_Bool is_entry_has_only_space(const char *);
	Eina_Bool _apply_entry_data(void);
	void _show_error_popup(int error_code);

	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static char *__text_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __done_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_next_key_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __entry_clicked_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __entry_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __editfield_changed_cb(void *data, Evas_Object *obj, void *event_info);
	auto_fill_form_item *m_item_for_compose;
	auto_fill_form_manager *m_manager;

	Evas_Object *m_parent;
	Evas_Object *m_main_layout;
	Evas_Object *m_genlist;
	Evas_Object *m_scroller;
	Evas_Object *m_box;
	Evas_Object *m_done_button;
	Evas_Object *m_cancel_button;
	Evas_Object *m_entry_full_name;
	Evas_Object *m_entry_company_name;
	Evas_Object *m_entry_address_line_1;
	Evas_Object *m_entry_address_line_2;
	Evas_Object *m_entry_city_town;
	Evas_Object *m_entry_county;
	Evas_Object *m_entry_post_code;
	Evas_Object *m_entry_country;
	Evas_Object *m_entry_phone;
	Evas_Object *m_entry_email;

	profile_edit_errorcode m_edit_errorcode;
	profile_save_errorcode m_save_errorcode;
	Elm_Genlist_Item_Class *m_edit_field_item_class;
	Elm_Entry_Filter_Limit_Size m_entry_limit_size;

	genlist_callback_data m_full_name_item_callback_data;
	genlist_callback_data m_company_name_item_callback_data;
	genlist_callback_data m_address_line_1_item_callback_data;
	genlist_callback_data m_address_line_2_item_callback_data;
	genlist_callback_data m_city_town_item_callback_data;
	genlist_callback_data m_country_item_callback_data;
	genlist_callback_data m_post_code_item_callback_data;
	genlist_callback_data m_county_region_item_callback_data;
	genlist_callback_data m_phone_item_callback_data;
	genlist_callback_data m_email_item_callback_data;
	std::string m_edjFilePath;
};

#endif /* AUTOFILLFORMCOMPOSEVIEW_H_ */
