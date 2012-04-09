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

#ifndef BROWSER_FIND_WORD_H
#define BROWSER_FIND_WORD_H

#include "browser-config.h"
#include "browser-view.h"

#include <string>

using namespace std;

class Browser_Find_Word {
public:
	Browser_Find_Word(Browser_View *browser_view);
	~Browser_Find_Word(void);

	typedef enum _Find_Word_Direction {
		BROWSER_FIND_WORD_FORWARD,
		BROWSER_FIND_WORD_BACKWARD
	} Find_Word_Direction;

	void find_word(const char *word, Find_Word_Direction direction);
	Evas_Object *get_layout(void) { return m_option_header_find_word_layout; }
	void init(Evas_Object *webview);
	void deinit(void);
private:
	Eina_Bool _create_layout(void);

	static void __find_word_entry_imf_event_cb(void *data, Ecore_IMF_Context *ctx, int value);
	static void __find_word_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_prev_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_next_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_word_erase_button_clicked_cb(void *data, Evas_Object *obj,
								const char *emission, const char *source);
	void _update_find_word_index_text(const char *index_text);

	int m_find_word_index;
	int m_find_word_max_count;
	Evas_Object *m_webview;
	Browser_View *m_browser_view;

	Evas_Object *m_find_word_entry_layout;
	Evas_Object *m_find_word_edit_field;
	Evas_Object *m_find_word_cancel_button;
	Evas_Object *m_find_word_prev_button;
	Evas_Object *m_find_word_next_button;
	Evas_Object *m_option_header_find_word_layout;

	std::string m_prev_searched_word;
};

#endif /* BROWSER_FIND_WORD_H */

