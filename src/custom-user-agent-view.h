/*
 *  browser
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Sangpyo Kim <sangpyo7.kim@samsung.com>
 *              Hyerim Bae <hyerim.bae@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef CUSTOM_USER_AGENT_H
#define CUSTOM_USER_AGENT_H

#include "browser-object.h"
#include "common-view.h"

class custom_user_agent_view : public common_view {
public:
	custom_user_agent_view(void);
	~custom_user_agent_view(void);

	Eina_Bool show(void);
	Eina_Bool popup_edit_view_show(void);
	void destroy_custom_user_agent_edit_popup_show(void);

private:
	typedef struct _genlist_callback_data {
		void *user_data;
		Elm_Genlist_Item_Class *item;
		Elm_Object_Item *it;
	} genlist_callback_data;
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_genlist(Evas_Object *parent);
	Evas_Object *_create_edit_field(Evas_Object *parent);

/* callback functions */
	static char *__text_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_Info);
	static void __save_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __back_button_clicked_cb(void *data, Elm_Object_Item *it);
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __eraser_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_focused_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info);

	static void __popup_edit_save_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_edit_entry_changed_cb(void *data, Evas_Object *obj, void *event_info);

	genlist_callback_data m_editfield_data;
	Evas_Object *m_editfield_entry;
	Evas_Object *m_popup;
	Evas_Object *m_save_button;
	Evas_Object *m_cancel_button;
	Elm_Entry_Filter_Limit_Size m_entry_limit_size;
};

#endif /* CUSTOM_USER_AGENT_H */

