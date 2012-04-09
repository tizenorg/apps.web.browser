/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#ifndef BROWSER_SETTINGS_ACCELERATED_COMPOSITION_H
#define BROWSER_SETTINGS_ACCELERATED_COMPOSITION_H

#include "browser-common-view.h"
#include "browser-config.h"
#include "browser-settings-main-view.h"

class Browser_Settings_Accelerated_Composition : public Browser_Common_View {
public:
	Browser_Settings_Accelerated_Composition(Browser_Settings_Main_View *main_view);
	~Browser_Settings_Accelerated_Composition(void);

	Eina_Bool init(void);

	typedef enum _menu_type {
		BR_ACCELERATED_COMPOSITION,
		BR_EXTERNAL_VIDEO_PLAYER,
		BR_UNKOWN
	} menu_type;
private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Eina_Bool _create_main_layout(void);

	/* genlist callback functions */
	static Evas_Object *__genlist_icon_get(void *data, Evas_Object *obj, const char *part);
	static char *__genlist_label_get(void *data, Evas_Object *obj, const char *part);

	/* Elementary event callback functions */
	static void __item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __accelerated_composition_check_button_changed_cb(void *data,
							Evas_Object *obj, void *event_info);
	static void __external_video_player_check_button_changed_cb(void *data,
							Evas_Object *obj, void *event_info);

	Evas_Object *m_genlist;
	Evas_Object *m_back_button;
	Evas_Object *m_accelerated_composition_check_button;
	Evas_Object *m_external_video_player_check_button;

	Elm_Genlist_Item_Class m_seperator_item_class;
	Elm_Genlist_Item_Class m_1_text_1_icon_item_class;

	genlist_callback_data m_accelerated_composition_callback_data;
	genlist_callback_data m_accelerated_external_video_player_callback_data;
	Browser_Settings_Main_View *m_main_view;
};

#endif /* BROWSER_SETTINGS_ACCELERATED_COMPOSITION_H */

