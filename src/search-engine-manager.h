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
 * Contact: Kwangyong Choi <ky0.choi@samsung.com>
 *
 */

#ifndef SEARCH_ENGINE_MANAGER_H
#define SEARCH_ENGINE_MANAGER_H

#define LANGSET_LENGTH 5

#include <string>

#include "browser-object.h"
#include "browser.h"

#define	SEARCH_ENGINE_NOT_SELECTED 0
#define	SEARCH_ENGINE_GOOGLE 1
#define	SEARCH_ENGINE_YAHOO 2
#define	SEARCH_ENGINE_BING 3
#define	SEARCH_ENGINE_YANDEX 4

class search_engine_manager : public browser_object {
public:
	search_engine_manager(void);
	~search_engine_manager(void);

	std::string query_string_get(const int engine, const char *keyword);

private:
	std::string _query_string_google_get(const char *keyword);
#if defined(ENABLE_YAHOO_SEARCH)
	std::string _query_string_yahoo_get(const char *keyword);
#endif
#if defined(ENABLE_BING_SEARCH)
	std::string _query_string_bing_get(const char *keyword);
#endif

	std::string _make_keyword_string(const char *keyword);

	char m_langset[LANGSET_LENGTH + 1]; // Including '\0'
};

#endif // SEARCH_ENGINE_MANAGER_H
