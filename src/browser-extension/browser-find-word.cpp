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
 *
 */


#include "browser-find-word.h"
#include "browser-window.h"

#define FIND_WORD_MAX_COUNT	999

Browser_Find_Word::Browser_Find_Word(Browser_View *browser_view)
//:	m_page(NULL)
:	m_find_word_index(0)
	,m_find_word_max_count(0)
	,m_browser_view(browser_view)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Find_Word::~Browser_Find_Word(void)
{
	BROWSER_LOGD("[%s]", __func__);
	evas_object_smart_callback_del(m_browser_view->m_focused_window->m_ewk_view, "text,found",  Browser_Find_Word::__did_find_string_cb);
}

void Browser_Find_Word::__did_find_string_cb(void *data, Evas_Object *obj, void *event_info)
{
	unsigned int match_count = *(unsigned int *)event_info;

	BROWSER_LOGD("match_count = %u", match_count);

	Browser_Find_Word *find_word = (Browser_Find_Word *)data;
	find_word->m_find_word_max_count = match_count;

	if (match_count == 0) {
		find_word->m_find_word_index = 0;
		find_word->m_find_word_max_count = 0;

		find_word->m_browser_view->_update_find_word_index_text("0/0", find_word->m_find_word_index, find_word->m_find_word_max_count);
		return;
	}

	if (match_count == 1) {
		find_word->m_find_word_index = 1;
		find_word->m_find_word_max_count = 1;

		find_word->m_browser_view->_update_find_word_index_text("1/1", find_word->m_find_word_index, find_word->m_find_word_max_count);
		return;
	}

	if (find_word->m_find_word_max_count < 0 || find_word->m_find_word_max_count > FIND_WORD_MAX_COUNT) {
		BROWSER_LOGD("Matched word counts are over Max. which browser supported. Set as Maximum");
		find_word->m_find_word_max_count = FIND_WORD_MAX_COUNT;
	}

	GString *index_string = g_string_new(NULL);
	if (!index_string) {
		BROWSER_LOGE("g_string_new failed");
		return;
	}
	g_string_printf(index_string, "%d/%d", find_word->m_find_word_index, find_word->m_find_word_max_count);
	find_word->m_browser_view->_update_find_word_index_text(index_string->str, find_word->m_find_word_index, find_word->m_find_word_max_count);
	g_string_free(index_string, true);
}

int Browser_Find_Word::find_word(const char *word, Find_Word_Direction direction)
{
	evas_object_smart_callback_add(m_browser_view->m_focused_window->m_ewk_view, "text,found",  Browser_Find_Word::__did_find_string_cb, this);

	BROWSER_LOGD("word to find=[%s]", word);

	Ewk_Find_Options find_option = (Ewk_Find_Options)(EWK_FIND_OPTIONS_CASE_INSENSITIVE | EWK_FIND_OPTIONS_WRAP_AROUND
			| EWK_FIND_OPTIONS_SHOW_FIND_INDICATOR | EWK_FIND_OPTIONS_SHOW_HIGHLIGHT);

	if (direction == BROWSER_FIND_WORD_FORWARD) {
		m_find_word_index++;

		ewk_view_text_find(m_browser_view->m_focused_window->m_ewk_view, word, find_option, FIND_WORD_MAX_COUNT);
	} else if (direction == BROWSER_FIND_WORD_BACKWARD) {
		m_find_word_index--;
		ewk_view_text_find(m_browser_view->m_focused_window->m_ewk_view, word, find_option, FIND_WORD_MAX_COUNT);
	}
	if (m_find_word_index > FIND_WORD_MAX_COUNT)
		m_find_word_index = FIND_WORD_MAX_COUNT;

	if (m_find_word_index < 0)
		m_find_word_index = 0;

	return m_find_word_index;
}

void Browser_Find_Word::init_index()
{
	m_find_word_index = 0;
}

int Browser_Find_Word::get_match_max_value()
{
	return m_find_word_max_count;
}
