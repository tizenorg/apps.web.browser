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

#include "platform-service.h"

#include <Elementary.h>
#include <vconf.h>
#include <vconf-internal-setting-keys.h>

#include "browser-dlog.h"

static void __changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	void *erase = evas_object_data_get(layout, "eraser");
	BROWSER_LOGD("erase = %d", erase);
	if (!erase)
		return;

	if (elm_object_focus_get(layout)) {
		if (elm_entry_is_empty(obj))
			elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
		else {
			elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");
		}
	}
	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
}

static void __focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	void *erase = evas_object_data_get(layout, "eraser");
	BROWSER_LOGD("erase = %d", erase);

	if (!elm_entry_is_empty(obj) && erase)
		elm_object_signal_emit(layout, "elm,state,eraser,show", "elm");

	elm_object_signal_emit(layout, "elm,state,guidetext,hide", "elm");
}

static void __unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *)data;

	if (elm_entry_is_empty(obj))
		elm_object_signal_emit(layout, "elm,state,guidetext,show", "elm");

	elm_object_signal_emit(layout, "elm,state,eraser,hide", "elm");
}

static void __eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *entry = (Evas_Object *)data;
	elm_entry_entry_set(entry, "");
}

platform_service::platform_service(void)
{
	BROWSER_LOGD("");
}

platform_service::~platform_service(void)
{
	BROWSER_LOGD("");
}

Evas_Object *platform_service::editfield_add(Evas_Object *parent, Eina_Bool title)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *editfield_layout = elm_layout_add(parent);
	if (title)
		elm_layout_theme_set(editfield_layout, "layout", "editfield", "title");
	else
		elm_layout_theme_set(editfield_layout, "layout", "editfield", "default");

	Evas_Object *entry = elm_entry_add(editfield_layout);
	elm_object_part_content_set(editfield_layout, "elm.swallow.content", entry);

	elm_object_signal_emit(editfield_layout, "elm,state,guidetext,hide", "elm");
	evas_object_data_set(editfield_layout, "eraser", (void *)EINA_TRUE);

	evas_object_smart_callback_add(entry, "changed", __changed_cb, editfield_layout);
	evas_object_smart_callback_add(entry, "preedit,changed", __changed_cb, editfield_layout);
	evas_object_smart_callback_add(entry, "focused", __focused_cb, editfield_layout);
	evas_object_smart_callback_add(entry, "unfocused", __unfocused_cb, editfield_layout);

	elm_object_signal_callback_add(editfield_layout, "elm,eraser,clicked", "elm",
						__eraser_clicked_cb, entry);

	return editfield_layout;
}

void platform_service::editfield_label_set(Evas_Object *obj, const char *label)
{
	EINA_SAFETY_ON_NULL_RETURN(obj);

	elm_object_part_text_set(obj, "elm.text", label);
}

Evas_Object *platform_service::editfield_entry_get(Evas_Object *obj)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

	return elm_object_part_content_get(obj, "elm.swallow.content");
}

void platform_service::editfield_guide_text_set(Evas_Object *obj, const char *text)
{
	EINA_SAFETY_ON_NULL_RETURN(obj);

	elm_object_part_text_set(obj, "elm.guidetext", text);
}

void platform_service::editfield_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line)
{
	EINA_SAFETY_ON_NULL_RETURN(obj);

	Evas_Object *entry = editfield_entry_get(obj);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
}

void platform_service::editfield_eraser_set(Evas_Object *obj, Eina_Bool visible)
{
	EINA_SAFETY_ON_NULL_RETURN(obj);

	evas_object_data_set(obj, "eraser", (void *)visible);

	if (visible)
		elm_object_signal_emit(obj, "elm,state,eraser,show", "elm");
	else
		elm_object_signal_emit(obj, "elm,state,eraser,hide", "elm");
}

void platform_service::evas_image_size_get(Evas_Object *image, int *w, int *h, int *stride)
{
	EINA_SAFETY_ON_NULL_RETURN(image);
	EINA_SAFETY_ON_NULL_RETURN(w);
	EINA_SAFETY_ON_NULL_RETURN(h);

	Evas_Object *evas_image = evas_object_image_add(evas_object_evas_get(image));
	const char *image_type = evas_object_type_get(evas_image);
	const char *type = evas_object_type_get(image);
	BROWSER_LOGD("snapshot type=[%s], image type=[%s]", type, image_type);
	if (type && image_type && !strcmp(type, image_type)) {
		evas_object_image_size_get(image, w, h);

		if (stride) {
			*stride = evas_object_image_stride_get(image);
			BROWSER_LOGD("snapshot w=[%d], h=[%d], stride=[%d]", *w, *h, *stride);
		} else
			BROWSER_LOGD("snapshot w=[%d], h=[%d]", *w, *h);
	} else {
		*w = 0;
		*h = 0;
		if (stride)
			*stride = 0;
	}
	evas_object_del(evas_image);
}

Evas_Object *platform_service::copy_evas_image(Evas_Object *origin_image)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(origin_image, NULL);

	int w, h, stride;
	evas_image_size_get(origin_image, &w, &h, &stride);
	
	if (!w || !h)
		return NULL;
	
	void *origin_image_data = evas_object_image_data_get(origin_image, EINA_TRUE);
	
	Evas_Object *image = evas_object_image_filled_add(evas_object_evas_get(m_window));
	evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
	evas_object_image_size_set(image, w, h);
	evas_object_image_fill_set(image, 0, 0, w, h);
	evas_object_image_filled_set(image, EINA_TRUE);
	evas_object_image_alpha_set(image,EINA_TRUE);

	void *target_image_data = evas_object_image_data_get(image, EINA_TRUE);
	memcpy(target_image_data, origin_image_data, stride * h);
	evas_object_image_data_set(image, target_image_data);

	return image;
}

char *platform_service::get_system_language_set(void)
{
	char system_lang[3] = {0, };
	char *langset = vconf_get_str(VCONFKEY_LANGSET);

	if (!langset)
		strncpy(system_lang, "en", 2);
	else {
		/* get lang */
		strncpy(system_lang, langset, 2);
		free(langset);
		return strdup(system_lang);
	}

	return NULL;
}

char *platform_service::get_system_region_set(void)
{
	char system_region[3] = {0, };
	char *langset = vconf_get_str(VCONFKEY_LANGSET);

	if (!langset)
		return NULL;

	/* get region */
	char *raw_data = strchr(langset, '_');
	/* 5 is reserved length for "system_lang" + '_' + "system_region" */
	if (raw_data && strlen(langset) >= 5) {
		strncpy(system_region, raw_data + 1, 2);
		free(langset);
		return strdup(system_region);
	}
	free(langset);
	return NULL;
}

char *platform_service::get_system_encoding_set(void)
{
	char *langset = vconf_get_str(VCONFKEY_LANGSET);

	if (!langset)
		return NULL;

	/* get_encoding */
	unsigned short entire_strlen = strlen(langset);
	unsigned short encoding_strlen = 0;
	char *raw_data = strchr(langset, '.');
	/* 7 is reserved length for "system_lang" + '_' + "system_region" + '.' + at least 1 char. */
	if (raw_data && strlen(langset) >= 7) {
		encoding_strlen = entire_strlen - 6; /* 6 is reserved length for "system_lang" + '_' + "system_region" + '.' */
		free(langset);
		return strdup(raw_data + 1);
	}
	free(langset);
	return NULL;
}

