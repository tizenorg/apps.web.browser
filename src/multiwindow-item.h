/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MULTIWINDOW_ITEM_H
#define MULTIWINDOW_ITEM_H

#include <Evas.h>
#include "browser-object.h"

class multiwindow_item : public browser_object {
public:
	multiwindow_item(Evas_Object *snapshot, const char *title, const char *uri, Evas_Smart_Cb clicked_cb, Evas_Smart_Cb delete_cb, void *data);
	~multiwindow_item(void);

	const char *get_title(void) { return m_title; }
	const char *get_uri(void) { return m_uri; }

	Evas_Object *get_layout(void) { return m_layout; }
	void show_delete_button(Eina_Bool show);
	void shrink_layout(Eina_Bool shrink);
	Eina_Bool is_shrink(void) { return m_is_shrink; }
	void show_tilting_effect(void);
	void show_spring_effect(void);
private:
	Evas_Object *_create_layout(Evas_Object *parent);
	static void __delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

	Evas_Smart_Cb m_clicked_cb;
	Evas_Smart_Cb m_delete_cb;
	void *m_cb_data;

	const char *m_title;
	const char *m_uri;
	Evas_Object *m_snapshot;
	Evas_Object *m_layout;
	Eina_Bool m_is_shrink;
};

#endif /* MULTIWINDOW_ITEM_H */

