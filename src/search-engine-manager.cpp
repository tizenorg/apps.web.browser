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

#include "search-engine-manager.h"

#include "browser-dlog.h"
#include "platform-service.h"

#define MAX_KEYWORD_LENGTH 1024

// Google search URL.
static const char *google_query_url = "http://www.google.com/search?ie=UTF-8&q=";

#if defined(ENABLE_YAHOO_SEARCH)
// Yahoo search URLs.
static const char *yahoo_query_url[][2] = {
	{"en-US", "http://search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"en-GB", "http://uk.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"ja-JP", "http://search.yahoo.co.jp/search?ei=UTF-8&fr=crmas&p="},
	{"pt-BR", "http://br.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"pt-BR", "http://br.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	//{"de-CH", "http://ch.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="}, // Not used on Tizen.
	{"zh-CN", "http://search.cn.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"da-DK", "http://dk.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"de-DE", "http://de.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"zh-TW", "http://tw.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"es-ES", "http://es.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	//{"en-NZ", "http://nz.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="}, // Not used on Tizen.
	{"nl-NL", "http://nl.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"nb-NO", "http://no.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"fr-FR", "http://fr.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"ko-KR", "http://kr.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"zh-HK", "http://hk.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"zh-SG", "http://sg.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"sv-SE", "http://se.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	{"it-IT", "http://it.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="},
	//{"de-AT", "http://at.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="}, // Not used on Tizen.
	//{"en-AU", "http://au.search.yahoo.com/search?ei=UTF-8&fr=crmas&p="}, // Not used on Tizen.
	{NULL, NULL}
};
#endif

// Bing search URL.
#if defined(ENABLE_BING_SEARCH)
static const char *bing_query_url = "http://www.bing.com/search?q=";
static const char *bing_query_url_postfix = "&PC=SMSMT&FORM=MBDPSB";
#endif

search_engine_manager::search_engine_manager(void)
{
	platform_service ps;

	char *lang = ps.get_system_language_set();
	char *region = ps.get_system_region_set();

	memset(m_langset, 0x0, LANGSET_LENGTH + 1);

	if (lang != NULL) {
		memcpy(m_langset, lang, 2);
		free(lang);
	} else
		memcpy(m_langset, "en", 2);

	if (region != NULL) {
		m_langset[2] = '-';
		memcpy(m_langset + 3, region, 2);
		free(region);
	}

	BROWSER_SECURE_LOGD("langset: %s", m_langset);
}

search_engine_manager::~search_engine_manager(void)
{
	BROWSER_LOGD("");
}

std::string search_engine_manager::query_string_get(int engine, const char *keyword)
{
	BROWSER_SECURE_LOGD("engine: %d, keyword: %s", engine, keyword);

	switch (engine) {
	case SEARCH_ENGINE_GOOGLE:
		return _query_string_google_get(keyword);
#if defined(ENABLE_YAHOO_SEARCH)
	case SEARCH_ENGINE_YAHOO:
		return _query_string_yahoo_get(keyword);
#endif
#if defined(ENABLE_BING_SEARCH)
	case SEARCH_ENGINE_BING:
		return _query_string_bing_get(keyword);
#endif
	default:
		return _query_string_google_get(keyword);
	}
}

std::string search_engine_manager::_query_string_google_get(const char *keyword)
{
	BROWSER_LOGD("");

	return std::string(google_query_url) + _make_keyword_string(keyword);
}

#if defined(ENABLE_YAHOO_SEARCH)
std::string search_engine_manager::_query_string_yahoo_get(const char *keyword)
{
	BROWSER_LOGD("");

	int i = 0;
	while (yahoo_query_url[i][0] != NULL) {
		if (strncmp(m_langset, yahoo_query_url[i][0], LANGSET_LENGTH) == 0) {
			BROWSER_LOGD("match");
			return std::string(yahoo_query_url[i][1]) + _make_keyword_string(keyword);
		}
		i++;
	}

	BROWSER_LOGD("no match");

	return std::string(yahoo_query_url[0][1]) + _make_keyword_string(keyword);
}
#endif

#if defined(ENABLE_BING_SEARCH)
std::string search_engine_manager::_query_string_bing_get(const char *keyword)
{
	BROWSER_LOGD("");

	return std::string(bing_query_url) + _make_keyword_string(keyword) + bing_query_url_postfix;
}
#endif

std::string search_engine_manager::_make_keyword_string(const char *keyword)
{
	if (!keyword)
		return std::string("");

	std::string keyword_str = std::string(keyword).substr(0, MAX_KEYWORD_LENGTH);
	if (keyword_str.size() > 0) {
		std::string emp("&");
		unsigned found = 0;
		while (found != std::string::npos) {
			found = keyword_str.find(emp);
			if (found != std::string::npos)
				keyword_str.replace(found, emp.length(), "%26");
		}
	}

	return keyword_str;
}
