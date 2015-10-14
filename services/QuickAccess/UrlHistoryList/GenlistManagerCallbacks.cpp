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

#include "BrowserLogger.h"
#include "GenlistManagerCallbacks.h"
#include "GenlistManager.h"

namespace tizen_browser
{
namespace services
{

GenlistManagerCallbacks::GenlistManagerCallbacks()
{
}

GenlistManagerCallbacks::~GenlistManagerCallbacks()
{
}

void GenlistManagerCallbacks::cb_genlistAnimStop(void *data, Evas_Object *obj,
		void *event_info)
{
}

void GenlistManagerCallbacks::cb_genlistEdgeTop(void *data, Evas_Object *obj,
		void *event_info)
{
	BROWSER_LOGD("@@ %s", __FUNCTION__);
	auto manager = static_cast<GenlistManager*>(data);
	manager->setLastEdgeTop(false);
	// spaces added for 'slide in' effect are not longer needed
	manager->removeSpaces();
}

void GenlistManagerCallbacks::cb_genlistEdgeBottom(void *data, Evas_Object *obj,
		void *event_info)
{
	auto manager = static_cast<GenlistManager*>(data);
	manager->setLastEdgeTop(true);
	if (manager->isWidgetHidden())
	{
		manager->clearWidget();
		evas_object_hide(manager->getWidget());
	}
}

void GenlistManagerCallbacks::cb_genlistActivated(void *data, Evas_Object *obj,
		void *event_info)
{
}
void GenlistManagerCallbacks::cb_genlistPressed(void *data, Evas_Object *obj,
		void *event_info)
{
}
void GenlistManagerCallbacks::cb_genlistSelected(void *data, Evas_Object *obj,
		void *event_info)
{
}
void GenlistManagerCallbacks::cb_genlistUnselected(void *data, Evas_Object *obj,
		void *event_info)
{
}

void GenlistManagerCallbacks::cb_genlistFocused(void *data, Evas_Object *obj,
		void *event_info)
{
}

void GenlistManagerCallbacks::cb_genlistUnfocused(void *data, Evas_Object *obj,
		void *event_info)
{
}

void GenlistManagerCallbacks::cb_genlistMouseIn(void *data, Evas *e,
		Evas_Object *obj, void *event_info)
{
	auto manager = static_cast<GenlistManager*>(data);
	manager->onMouseFocusChange(true);
}
void GenlistManagerCallbacks::cb_genlistMouseOut(void *data, Evas *e,
		Evas_Object *obj, void *event_info)
{
	auto manager = static_cast<GenlistManager*>(data);
	manager->onMouseFocusChange(false);
}

void GenlistManagerCallbacks::cb_itemFocused(void *data, Evas_Object *obj,
		void *event_info)
{
}
void GenlistManagerCallbacks::cb_itemUnfocused(void *data, Evas_Object *obj,
		void *event_info)
{
}
void GenlistManagerCallbacks::cb_itemMouseIn(void *data, Elm_Object_Item *it,
        const char *emission, const char *source)
{
}
void GenlistManagerCallbacks::cb_itemMouseOut(void *data, Evas *e,
		Evas_Object *obj, void *event_info)
{
}

} /* namespace services */
} /* namespace tizen_browser */
