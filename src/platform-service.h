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

#ifndef PLATFORM_SERVICE_H
#define PLATFORM_SERVICE_H

#include "browser-object.h"

#include <Evas.h>

class platform_service : public browser_object {
public:
	platform_service(void);
	~platform_service(void);

	Evas_Object *editfield_add(Evas_Object *parent, Eina_Bool title = EINA_FALSE);
	void editfield_label_set(Evas_Object *obj, const char *label);

	/* @ param : obj - The edit field layout which gets by editfield_add */
	Evas_Object *editfield_entry_get(Evas_Object *obj);
	void editfield_guide_text_set(Evas_Object *obj, const char *text);
	void editfield_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);
	void editfield_eraser_set(Evas_Object *obj, Eina_Bool visible);

	void evas_image_size_get(Evas_Object *image, int *w, int *h, int *stride = NULL);
	Evas_Object *copy_evas_image(Evas_Object *origin_image);
	char *get_system_language_set(void);
	char *get_system_region_set(void);
	char *get_system_encoding_set(void);

private:

};
#endif /* PLATFORM_SERVICE_H */

