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


#ifndef BROWSER_CONTEXT_MENU_H
#define BROWSER_CONTEXT_MENU_H

#include "browser-config.h"
#include "browser-common-view.h"
#include "browser-utility.h"

class Browser_View;

class Browser_Context_Menu : public Browser_Utility, public Browser_Common_View {
public:
	Browser_Context_Menu(Browser_View *browser_view);
	~Browser_Context_Menu(void);

	Eina_Bool init(void) {}
	void init(Evas_Object *ewk_view);
	void deinit(void);

	enum {
		CustomContextMenuItemBaseApplicationTag = EWK_CONTEXT_MENU_ITEM_BASE_APPLICATION_TAG,
		CustomContextMenuItemImageSendViaMessageTag,
		CustomContextMenuItemImageSendViaEmailTag,
		CustomContextMenuItemSelectedTextShareTag
	};
private:
	static void __get_context_menu_from_proposed_context_menu_cb(void *data, Evas_Object *obj, void *event_info);
	static void __custom_context_menu_item_selected_cb(void *data, Evas_Object *obj, void *event_info);

	Browser_View *m_browser_view;
	Evas_Object *m_ewk_view;
};
#endif /* BROWSER_CONTEXT_MENU_H */

