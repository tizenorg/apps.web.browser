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
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 */

#ifndef PLATFORM_SERVICE_H
#define PLATFORM_SERVICE_H

#include "browser-object.h"

#include <Evas.h>

#ifdef __cplusplus
extern "C" {
#endif

Evas_Object *brui_popup_add(Evas_Object *parent);
void brui_object_del(Evas_Object *obj);
Elm_Object_Item *brui_ctxpopup_item_append(Evas_Object *obj,
			const char *label,
			Evas_Smart_Cb func,
			const char *icon_file,
			const char *icon_group,
			const void *data);

#ifdef __cplusplus
}
#endif

Eina_Bool brui_string_cmp(const char *str1, const char *str2);
Eina_Bool brui_is_regular_express(const char *uri);

#define SAFE_FREE_OBJ(x) do {\
					if ((x) != NULL) {\
						brui_object_del(x); \
						x = NULL;\
					} \
			     } while (0)

class platform_service : public browser_object {
public:
	platform_service(void);
	~platform_service(void);

	Eina_Bool remove_file(const char *path);

	/* Image related smart method */
	void evas_image_size_get(Evas_Object *image, int *w, int *h, int *stride = NULL);
	int get_png_file_image_size(const char *path, int *w, int *h);
	Evas_Object *copy_evas_image(Evas_Object *origin_image);
	Eina_Bool copy_evas_image(Evas_Object *origin_image, Evas_Object *target_image);
	Eina_Bool is_png(const char *file_path);

	/* Locale related smart method */
	char *get_system_language_set(void);
	char *get_system_region_set(void);
	char *get_system_encoding_set(void);
	void get_touch_point_by_degree(int *x, int *y);
	unsigned long long get_file_size(const char *full_path);

	/* String should be freed in caller side */
	char *get_file_size_str(const char *full_path);

	void get_more_ctxpopup_position(int *x, int *y);
	Eina_Bool check_hw_usb_keyboard_alive(void);
	void print_pdf(const char *path);
	void resize_webview_snapshot(Evas_Object *snapshot);
	Eina_Bool capture_screen_to_png(Evas_Object *window, const char *target_path, int requested_w = 0, int requested_h = 0);
	Evas_Object *get_captured_screen_from_png(Evas_Object *window, const char *target_path);
	Evas_Object *capture_screen_to_evas_obj(Evas_Object *window, int requested_w = 0, int requested_h = 0);
	void clip_obj_size_to_clipboard(Evas_Object *obj, int* w, int *h);

private:

};
#endif /* PLATFORM_SERVICE_H */

