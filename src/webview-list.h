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

#ifndef WEBVIEW_LIST_H
#define WEBVIEW_LIST_H

#include <Evas.h>
#include <vector>

#include "browser-object.h"

#define BROWSER_WINDOW_MAX_SIZE	50

class webview;
class webview_list : public browser_object {
public:
	webview_list(void);
	~webview_list(void);

	webview *create_webview(Eina_Bool user_created = EINA_FALSE);
	/* @ return - The previous webview of deleted window will be returned to be able to replace it. */
	webview *delete_webview(webview *wv);
	void clean_up_webviews(void);
	unsigned int get_count(void);
	webview *get_webview(int index);
private:
	std::vector<webview *> m_webview_list;
};

#endif /* WEBVIEW_LIST_H */

