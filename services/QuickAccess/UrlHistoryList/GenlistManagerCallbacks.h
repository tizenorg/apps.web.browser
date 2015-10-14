/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#ifndef GENLISTMANAGERCALLBACKS_H_
#define GENLISTMANAGERCALLBACKS_H_

#include <Elementary.h>
#include <Evas.h>

class GenlistManager;

namespace tizen_browser
{
namespace services
{

class GenlistManagerCallbacks
{
public:
	GenlistManagerCallbacks();
	virtual ~GenlistManagerCallbacks();

	static void cb_genlistAnimStop(void *data, Evas_Object *obj,
			void *event_info);
	static void cb_genlistEdgeTop(void *data, Evas_Object *obj,
			void *event_info);
	static void cb_genlistEdgeBottom(void *data, Evas_Object *obj,
			void *event_info);

	static void cb_genlistActivated(void *data, Evas_Object *obj,
			void *event_info);
	static void cb_genlistPressed(void *data, Evas_Object *obj,
			void *event_info);
	static void cb_genlistSelected(void *data, Evas_Object *obj,
			void *event_info);
	static void cb_genlistUnselected(void *data, Evas_Object *obj,
			void *event_info);

	static void cb_genlistFocused(void *data, Evas_Object *obj,
			void *event_info);
	static void cb_genlistUnfocused(void *data, Evas_Object *obj,
			void *event_info);
	static void cb_genlistMouseIn(void *data, Evas *e, Evas_Object *obj,
			void *event_info);
	static void cb_genlistMouseOut(void *data, Evas *e, Evas_Object *obj,
			void *event_info);

	static void cb_itemFocused(void *data, Evas_Object *obj, void *event_info);
	static void cb_itemUnfocused(void *data, Evas_Object *obj,
			void *event_info);
	static void cb_itemMouseIn(void *data, Elm_Object_Item *it,
            const char *emission, const char *source);
	static void cb_itemMouseOut(void *data, Evas *e, Evas_Object *obj,
			void *event_info);
};

} /* namespace services */
} /* namespace tizen_browser */

#endif /* GENLISTMANAGERCALLBACKS_H_ */
