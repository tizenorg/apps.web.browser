/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contact: Junghwan Kang <junghwan.kang@samsung.com>
 *
 */
#ifndef AUTOFILLFORM_H_
#define AUTOFILLFORM_H_

#include <string>
#include <vector>

#include <Elementary.h>
#include <Evas.h>
//#include <efl_assist.h>
//#include <efl_extension.h>

#include <ewk_chromium.h>

#include "common.h"
#include "AutoProfileDeleteView.h"
//#include "ListViewItemClickedPopup.h"
#include "AutoFillFormItem.h"
#include "AutoFillFormListView.h"
#include "AutoFillFormComposeView.h"

#define AUTO_FILL_FORM_ENTRY_MAX_COUNT 1024
#define PHONE_FIELD_VALID_ENTRIES "0123456789*#()/N,.;+ "

//class auto_fill_form_compose_view;
//class auto_fill_form_list_view;
//class auto_fill_form_item;

class auto_fill_form_manager {
public:
	auto_fill_form_manager(void);
	~auto_fill_form_manager(void);

	Eina_Bool save_auto_fill_form_item(auto_fill_form_item_data *item_data);
	Eina_Bool delete_auto_fill_form_item(auto_fill_form_item *item);
	Eina_Bool delete_all_auto_fill_form_items(void);
	unsigned int get_auto_fill_form_item_count(void);
	auto_fill_form_item *create_new_auto_fill_form_item(Ewk_Autofill_Profile *profile = NULL);
	auto_fill_form_list_view *show_list_view(void);
	auto_fill_form_list_view *get_list_view(void) { return m_list_view; }
	auto_fill_form_compose_view *show_composer(auto_fill_form_item *item = NULL);
	auto_fill_form_compose_view *get_compose_view(void) { return m_composer; }
	Eina_Bool delete_list_view(void);
	Eina_Bool delete_composer(void);
	std::vector<auto_fill_form_item *> get_item_list(void) { return m_auto_fill_form_item_list; }
	std::vector<auto_fill_form_item *> load_entire_item_list(void);
	Eina_Bool add_item_to_list(auto_fill_form_item *item);
	void refresh_items_view();

	/* test */
	void see_all_data(void);
        static auto_fill_form_manager* affm;
        static auto_fill_form_manager* get_auto_fill_form_manager() { return affm; }
        static void set_auto_fill_form_manager(auto_fill_form_manager* manager) { affm = manager; }
private:
	static void profiles_updated_cb(void* data);
	std::vector<auto_fill_form_item *> m_auto_fill_form_item_list;
	auto_fill_form_list_view *m_list_view;
	auto_fill_form_compose_view *m_composer;

};

#endif
