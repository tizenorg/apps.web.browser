/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
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


#include "webview-list.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-view.h"
#include "uri-bar.h"
#include "webview.h"

using namespace std;

webview_list::webview_list(void)
{
	BROWSER_LOGD("");
}

webview_list::~webview_list(void)
{
	BROWSER_LOGD("");

	for (int i = 0 ; i < m_webview_list.size() ; i++)
		delete m_webview_list[i];

	m_webview_list.clear();
}

webview *webview_list::create_webview(Eina_Bool user_created)
{
	BROWSER_LOGD("");
	webview *wv = new webview(user_created);
	m_webview_list.push_back(wv);

	m_browser->get_browser_view()->get_uri_bar()->set_multi_window_button_count(m_webview_list.size());

	return wv;
}

webview *webview_list::delete_webview(webview *wv)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(wv, NULL);

	int webview_count = m_webview_list.size();
	if (webview_count <= 1)
		return NULL;

	webview *return_wv = NULL;

	for (int i = 0 ; i < webview_count ; i++) {
		if (m_webview_list[i] == wv) {

			// If deleted webview is current webview, set the current webview of browser-view null.
			if (m_browser->get_browser_view()->get_current_webview() == m_webview_list[i])
				m_browser->get_browser_view()->set_current_webview(NULL);

			delete m_webview_list[i];

			if (i)
				return_wv = m_webview_list[i - 1];
			else
				return_wv = m_webview_list[i + 1];

			m_webview_list.erase(m_webview_list.begin() + i);

			m_browser->get_browser_view()->get_uri_bar()->set_multi_window_button_count(m_webview_list.size());

			break;
		}
	}

	return return_wv;
}

void webview_list::clean_up_webviews(void)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(m_browser->get_browser_view());

	for (int i = 0 ; i < m_webview_list.size() ; i++) {
		if (m_webview_list[i] != m_browser->get_browser_view()->get_current_webview())
			m_webview_list[i]->delete_ewk_view();
	}

	m_webview_context->clear_memory_cache();
	m_webview_context->notify_low_memory();
}

unsigned int webview_list::get_count(void)
{
	unsigned int count = (unsigned int)m_webview_list.size();
	BROWSER_LOGD("webviews count = [%d]", count);

	return count;
}

webview *webview_list::get_webview(int index)
{
	BROWSER_LOGD("");
	if (index < 0 || index >= get_count())
		return NULL;

	return m_webview_list[index];
}

