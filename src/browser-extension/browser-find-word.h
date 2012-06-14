/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
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

