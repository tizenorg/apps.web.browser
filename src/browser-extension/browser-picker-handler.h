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

#ifndef BROWSER_PICKER_HANDLER_H
#define BROWSER_PICKER_HANDLER_H

#include "browser-common-view.h"
#include "browser-config.h"

class Browser_View;
class Browser_Picker_Handler : public Browser_Common_View {
public:
	Browser_Picker_Handler(Browser_View *browser_view);
	~Browser_Picker_Handler(void);

	Eina_Bool init(void) {}

	void init(Evas_Object *webview);
	void deinit(void);

	void destroy_picker_layout(void);

	typedef enum _input_type {
		INPUT_TYPE_DATE,
		INPUT_TYPE_DATE_TIME,
		INPUT_TYPE_DATE_TIME_LOCAL,
		INPUT_TYPE_UNKNOWN
	} input_type;
private:
	typedef struct _selected_info {
		int clicked_x;
		int clicked_y;
		int option_number;
		int current_option_index;
		char **option_list;
		Eina_Rectangle rect;
		bool is_prev;
		bool is_next;
	} selected_info;

	void _destroy_options(void);
	Eina_Bool _show_picker(void);
	Eina_Bool _show_calendar_picker(const char *date, input_type type);
	Eina_Bool _move_to_next_node(Eina_Bool is_next_node);

	static void __one_single_tap_cb(void *data, Evas_Object *obj, void *event_info);
	static void __input_method_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __win_resize_cb(void* data, Evas* evas, Evas_Object* obj, void* ev);

	static void __picker_layout_cb(struct ui_gadget *ug, enum ug_mode mode, void *priv);
	static void __picker_result_cb(struct ui_gadget *ug, bundle *result, void *priv);
	static void __picker_destroy_cb(struct ui_gadget *ug, void *priv);
	static void __calendar_picker_date_result_cb(struct ui_gadget *ug, bundle *result, void *priv);

	Browser_View *m_browser_view;
	Evas_Object *m_webview;
	Evas_Object *m_picker_layout;
	selected_info m_selected_info;

	struct ui_gadget *m_picker_ug;
};

#endif /* BROWSER_PICKER_HANDLER_H */

