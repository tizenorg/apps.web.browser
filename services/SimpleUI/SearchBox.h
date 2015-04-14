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

#ifndef _SEARCHBOX_H
#define _SEARCHBOX_H

#include <boost/signals2/signal.hpp>
#include <Elementary.h>
#include <Evas.h>

namespace tizen_browser
{
namespace base_ui
{

enum Ewk_Find_Options {
    EWK_FIND_OPTIONS_NONE, /**< no search flags, this means a case sensitive, no wrap, forward only search. */
    EWK_FIND_OPTIONS_CASE_INSENSITIVE = 1 << 0, /**< case insensitive search. */
    EWK_FIND_OPTIONS_AT_WORD_STARTS = 1 << 1, /**< search text only at the beginning of the words. */
    EWK_FIND_OPTIONS_TREAT_MEDIAL_CAPITAL_AS_WORD_START = 1 << 2, /**< treat capital letters in the middle of words as word start. */
    EWK_FIND_OPTIONS_BACKWARDS = 1 << 3, /**< search backwards. */
    EWK_FIND_OPTIONS_WRAP_AROUND = 1 << 4, /**< if not present search will stop at the end of the document. */
    EWK_FIND_OPTIONS_SHOW_OVERLAY = 1 << 5, /**< show overlay */
    EWK_FIND_OPTIONS_SHOW_FIND_INDICATOR = 1 << 6, /**< show indicator */
    EWK_FIND_OPTIONS_SHOW_HIGHLIGHT = 1 << 7 /**< show highlight */
};

class SearchBox{

public:
    SearchBox(Evas_Object *parent);
    Evas_Object* getContent();
    void hide();
    void show();
    bool is_visible();
    boost::signals2::signal<void (std::string, int)> textChanged;

private:
    Evas_Object* m_webSearchEntry;
    Evas_Object* m_entry_layout;
    Evas_Object* m_searchPrev;
    Evas_Object* m_searchNext;
    Evas_Object* m_caseSensitive;

    bool search_caseSensitive;
    bool visible;

    static void searchNext(void *data, Evas_Object */*obj*/, void */*event_info*/);
    static void searchPrev(void *data, Evas_Object */*obj*/, void */*event_info*/);
    static void caseSensitiveChanged(void *data, Evas_Object */*obj*/, void */*event_info*/);
};

}

}

#endif //_SEARCHBOX_H
