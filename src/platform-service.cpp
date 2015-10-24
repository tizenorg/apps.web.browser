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

#include "platform-service.h"

#include <Ecore_X.h>
#include <Elementary.h>
#include <app_control.h>
#include <app.h>
#include <cairo.h>
#include <fcntl.h>
#include <image_util.h>
#include <image_util_internal.h>
#include <regex.h>
#include <stdlib.h>
#include <string>
#include <sys/shm.h>
#include <time.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xatom.h>
#include <vconf.h>
#include <vconf-internal-setting-keys.h>

#include "browser-dlog.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

#define unit_size 1024
#define MENU_POS_X (30 * efl_scale)
#define PROP_X_EXT_KEYBOARD_INPUT_DETECTED "HW Keyboard Input Started"
#define print_service_name	"http://tizen.org/appcontrol/operation/print"
#define print_files_type	"service_print_files_type"
#define SERVICE_PRINT_CONTENT_COUNT	"ContentCount"
#define SERVICE_PRINT_CONTENT_TYPE	"ContentType"
#define SERVICE_PRINT_CONTENT	"ContentPath"
#define SERVICE_PRINT_TITLE	"Title"

#define URLEXPR "((https?|ftp|gopher|telnet|file|notes|ms-help):((//)|(\\\\))[\\w\\d:#@%/;$()~_?+-=\\.&]+)"
#define TRIM_SPACE " \t\n\v"


static char *brui_capture_window(Window id, Visual *visual, int width, int height, int depth, int *size)
{
	BROWSER_LOGD("");
	XShmSegmentInfo si;
	XImage *xim;
	int img_size;
	char *captured_img = NULL;

	si.shmid = shmget(IPC_PRIVATE, width * height * ((depth >> 3) + 1), IPC_CREAT | 0666);
	RETV_MSG_IF(si.shmid < 0, EINA_FALSE, "Failed to shmget");

	si.readOnly = False;
	si.shmaddr = (char *)shmat(si.shmid, NULL, 0);

	if (si.shmaddr == (char *)-1) {
		shmdt(si.shmaddr);
		shmctl(si.shmid, IPC_RMID, 0);
		return EINA_FALSE;
	}

	xim = XShmCreateImage((Display *)ecore_x_display_get(), visual, depth, ZPixmap, NULL, &si, width, height);

	if (xim == 0) {
		shmdt(si.shmaddr);
		shmctl(si.shmid, IPC_RMID, 0);

		return NULL;
	}

	img_size = xim->bytes_per_line * xim->height;
	xim->data = si.shmaddr;

	XSync((Display *)ecore_x_display_get(), False);
	XShmAttach((Display *)ecore_x_display_get(), &si);
	XShmGetImage((Display *)ecore_x_display_get(), id, xim, 0, 0, 0xFFFFFFFF);
	XSync((Display *)ecore_x_display_get(), False);

	captured_img = (char *)calloc(1, img_size);
	if (captured_img) {
		memcpy(captured_img, xim->data, img_size);
	} else {
		BROWSER_LOGE("calloc failed");
	}

	XShmDetach((Display *)ecore_x_display_get(), &si);
	XDestroyImage(xim);

	shmdt(si.shmaddr);
	shmctl(si.shmid, IPC_RMID, 0);

	*size = img_size;

	return captured_img;
}

static int brui_resize_captured_window(const char* pDataIn, char* pDataOut, int inWidth, int inHeight, int outWidth, int outHeight)
{
	BROWSER_LOGD("");
	int scaleX = 0;
	int scaleY = 0;
	int i = 0;
	int j = 0;
	int iRow = 0;
	int iIndex = 0;
	char* pOutput = pDataOut;
	char* pOut = pDataOut;
	const char* pIn = NULL;
	int *pColLUT = (int *)malloc(sizeof(int) * outWidth);
	if(!pColLUT){
		return 0;
	}
	/* Calculate X Scale factor */
	scaleX = inWidth * 256 / outWidth;
	/* Calculate Y Scale factor, aspect ratio is not maintained */
	scaleY = inHeight * 256 / outHeight;
	for (j = 0; j < outWidth; j++)
	{
	/* Get input index based on column scale factor */
	/* To get more optimization, this is calculated once and
	* is placed in a LUT and used for indexing
	*/
	pColLUT [j] = ((j * scaleX) >> 8) * 4;
	}
	pOut = pOutput;
	for (i = 0; i < outHeight; i++)
	{
		/* Get input routWidth index based on routWidth scale factor */
		iRow = (i * scaleY >> 8) * inWidth * 4;
		/* Loop could be unrolled for more optimization */
		for (j = 0; j < (outWidth); j++)
		{
			/* Get input index based on column scale factor */
			iIndex = iRow + pColLUT [j];
			pIn = pDataIn + iIndex;
			*pOut++ = *pIn++;
			*pOut++ = *pIn++;
			*pOut++ = *pIn++;
			*pOut++ = *pIn++;
		}
	}

	free(pColLUT);
	return 0;
}

#define _WND_REQUEST_ANGLE_IDX 0
#define _WND_CURR_ANGLE_IDX    1
int brui_get_window_angle(Ecore_X_Window win_id)
{
	int after = -1;
	int before = -1;

	do {
		int ret, count;
		int angle[2] = {-1, -1};
		unsigned char* prop_data = NULL;

		ret = ecore_x_window_prop_property_get(win_id,
				ECORE_X_ATOM_E_ILLUME_ROTATE_WINDOW_ANGLE,
				ECORE_X_ATOM_CARDINAL,
				32,
				&prop_data,
				&count);
		if (ret <= 0) {
			if (prop_data) free(prop_data);
			break;
		}

		if (prop_data) {
			memcpy(&angle, prop_data, sizeof (int) *count);
			free(prop_data);
		}

		after = angle[_WND_REQUEST_ANGLE_IDX];
		before = angle[_WND_CURR_ANGLE_IDX];
		BROWSER_SECURE_LOGD("after[%d], before[%d]", after, before);

	} while (0);

	if (after == -1)
		after = 0;

	return after;
}

static void brui_rotate_img(Evas_Object *image_object, int angle, int cx, int cy)
{
	Evas_Map *em;
	RET_MSG_IF(image_object == NULL, "image_object is NULL");

	em = evas_map_new(4);
	RET_MSG_IF(em == NULL, "em is NULL");

	evas_map_util_points_populate_from_object(em, image_object);
	evas_map_util_rotate(em, (double) angle, cx, cy);

	evas_object_map_set(image_object, em);
	evas_object_map_enable_set(image_object, EINA_TRUE);

	evas_map_free(em);
}

static Eina_Bool brui_flush_data_to_file(Evas *e, char *data, const char *filename, int w, int h)
{
	Evas_Object *output;

	output = evas_object_image_add(e);
	if (!output) {
		BROWSER_LOGE("Failed to create an image object\n");
		return EINA_FALSE;
	}

	evas_object_image_data_set(output, NULL);
	evas_object_image_colorspace_set(output, EVAS_COLORSPACE_ARGB8888);
	evas_object_image_alpha_set(output, EINA_TRUE);
	evas_object_image_size_set(output, w, h);
	evas_object_image_smooth_scale_set(output, EINA_TRUE);
	evas_object_image_data_set(output, data);
	evas_object_image_data_update_add(output, 0, 0, w, h);

	if (evas_object_image_save(output, filename, NULL, "quality=100 compress=1") == EINA_FALSE) {
		evas_object_del(output);
		BROWSER_LOGE("Faild to save a captured image (%s)\n", filename);
		return EINA_FALSE;
	}

	evas_object_del(output);

	if (access(filename, F_OK) != 0) {
		BROWSER_LOGE("File %s is not found\n", filename);
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

static Evas *brui_virtual_canvas_create(int w, int h)
{
	Ecore_Evas *internal_ee;
	Evas *internal_e;

	// Create virtual canvas
	internal_ee = ecore_evas_buffer_new(w, h);
	if (!internal_ee) {
		BROWSER_LOGE("Failed to create a new canvas buffer\n");
		return NULL;
	}

	ecore_evas_alpha_set(internal_ee, EINA_TRUE);
	ecore_evas_manual_render_set(internal_ee, EINA_TRUE);

	// Get the "Evas" object from a virtual canvas
	internal_e = ecore_evas_get(internal_ee);
	if (!internal_e) {
		ecore_evas_free(internal_ee);
		BROWSER_LOGE("Faield to get Evas object\n");
		return NULL;
	}

	return internal_e;
}

static Eina_Bool brui_virtual_canvas_destroy(Evas *e)
{
	Ecore_Evas *ee = NULL;

	ee = ecore_evas_ecore_evas_get(e);

	if (!ee) {
		BROWSER_LOGE("Failed to ecore evas object\n");
		return EINA_FALSE;
	}

	ecore_evas_free(ee);

	return EINA_TRUE;
}

static Eina_Bool brui_virtual_canvas_flush_to_file(Evas *e, const char *filename, int w, int h)
{
	void *data;
	Ecore_Evas *internal_ee;

	internal_ee = ecore_evas_ecore_evas_get(e);
	if (!internal_ee) {
		BROWSER_LOGE("Failed to get ecore evas\n");
		return EINA_FALSE;
	}

	ecore_evas_manual_render(internal_ee);

	// Get a pointer of a buffer of the virtual canvas
	data = (void *) ecore_evas_buffer_pixels_get(internal_ee);
	if (!data) {
		BROWSER_LOGE("Failed to get pixel data\n");
		return EINA_FALSE;
	}

	return brui_flush_data_to_file(e, (char *)data, filename, w, h);
}

static Eina_Bool brui_make_capture_file(const char *target_path, int width, int height, char *img, int angle)
{
	RETV_MSG_IF(!(target_path && strlen(target_path) > 0), EINA_FALSE, "Invalid target path");

	Evas *e = NULL;
	Evas_Object *image_object = NULL;
	int canvas_width = 0, canvas_height = 0;
	int cx = 0, cy = 0;
	int mx = 0;

	if (90 == angle || 270 == angle) {
		canvas_width = height;
		canvas_height = width;
	} else {
		canvas_width = width;
		canvas_height = height;
	}

	e = brui_virtual_canvas_create(canvas_width, canvas_height);
	if (e == NULL)
		goto error;

	image_object = evas_object_image_add(e);
	if (image_object == NULL)
		goto error;

	evas_object_image_size_set(image_object, width, height);
	evas_object_image_data_set(image_object, img);
	evas_object_image_data_update_add(image_object, 0, 0, width, height);
	evas_object_resize(image_object, width, height);
	evas_object_image_filled_set(image_object, EINA_TRUE);
	switch (angle) {
		case 90:
			cx = canvas_width - width / 2;
			cy = canvas_height / 2;
			mx = canvas_width - width;
			break;
		case 180:
			cx = width / 2;
			cy = height / 2;
			break;
		case 270:
			cx = width / 2;
			cy = canvas_height / 2;
			break;
		default:
			break;
	}
	evas_object_move(image_object, mx, 0);
	brui_rotate_img(image_object, angle, cx, cy);
	evas_object_show(image_object);

	if (access(default_snapshot_local_path, F_OK) != 0) {
		if (mkdir(default_snapshot_local_path, 0666) != 0)
			goto error;
	}
	if (brui_virtual_canvas_flush_to_file(e, target_path, canvas_width, canvas_height) == EINA_FALSE)
		goto error;

	evas_object_del(image_object);
	brui_virtual_canvas_destroy(e);

	return EINA_TRUE;

error:
	if (!e)
		brui_virtual_canvas_destroy(e);

	if (!image_object)
		evas_object_del(image_object);

	return EINA_FALSE;
}

/* The Evas_Object would be removed when it used */
static Evas_Object *brui_make_capture_evas_image(int width, int height, char *img, int angle)
{
	Evas *e = NULL;
	Evas_Object *image_object = NULL;
	int canvas_width = 0, canvas_height = 0;
	int cx = 0, cy = 0;
	int mx = 0;

	if (90 == angle || 270 == angle) {
		canvas_width = height;
		canvas_height = width;
	} else {
		canvas_width = width;
		canvas_height = height;
	}

	e = brui_virtual_canvas_create(canvas_width, canvas_height);
	if (e == NULL)
		goto error;

	image_object = evas_object_image_add(e);
	if (image_object == NULL)
		goto error;

	evas_object_image_size_set(image_object, width, height);
	evas_object_image_data_set(image_object, img);
	evas_object_image_data_update_add(image_object, 0, 0, width, height);
	evas_object_resize(image_object, width, height);
	evas_object_image_filled_set(image_object, EINA_TRUE);
	switch (angle) {
		case 90:
			cx = canvas_width - width / 2;
			cy = canvas_height / 2;
			mx = canvas_width - width;
			break;
		case 180:
			cx = width / 2;
			cy = height / 2;
			break;
		case 270:
			cx = width / 2;
			cy = canvas_height / 2;
			break;
		default:
			break;
	}
	evas_object_move(image_object, mx, 0);
	brui_rotate_img(image_object, angle, cx, cy);
	evas_object_show(image_object);

	return image_object;

error:
	if (!e)
		brui_virtual_canvas_destroy(e);

	if (!image_object)
		evas_object_del(image_object);

	return NULL;
}

Evas_Object *brui_popup_add(Evas_Object *parent)
{
	RETV_MSG_IF(!parent, NULL, "parent is NULL");
	Evas_Object *popup = NULL;
	popup = elm_popup_add(parent);
	elm_popup_align_set(popup, -1.0, 1.0);
	return popup;
}

void brui_object_del(Evas_Object *obj)
{
	RET_MSG_IF(!obj, "obj is NULL");

	const char *evas_obj_type = evas_object_type_get(obj);
	if (evas_obj_type && !strcmp(evas_obj_type, "image"))
		evas_object_image_data_set(obj, NULL);

	evas_object_del(obj);
}

Elm_Object_Item *brui_ctxpopup_item_append(Evas_Object *obj,
			const char *label,
			Evas_Smart_Cb func,
			const char *icon_file,
			const char *icon_group,
			const void *data)
{
	RETV_MSG_IF(!obj, NULL, "obj is NULL");
	Evas_Object *icon = NULL;

	return elm_ctxpopup_item_append(obj, label, icon, func, data);
}

Eina_Bool brui_string_cmp(const char *str1, const char *str2)
{
	RETV_MSG_IF(!str1, EINA_TRUE, "str1 is NULL");
	RETV_MSG_IF(!str2, EINA_TRUE, "str2 is NULL");

	unsigned short string1_len = strlen(str1);
	if (string1_len == 0)
		return EINA_TRUE;

	unsigned short string2_len = strlen(str2);
	if (string2_len == 0)
		return EINA_TRUE;

	if (string1_len != string2_len)
		return EINA_TRUE;

	return (strncmp(str1, str2, string1_len) != 0 ? EINA_TRUE : EINA_FALSE);
}

Eina_Bool brui_is_regular_express(const char *uri)
{
	BROWSER_SECURE_LOGD("uri=[%s]", uri);

	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");
	regex_t regex;
	if (regcomp(&regex, URLEXPR, REG_EXTENDED | REG_ICASE) != 0) {
		BROWSER_LOGD("regcomp failed");
		return EINA_FALSE;
	}

	if (regexec(&regex, uri, 0, NULL, REG_NOTEOL) == 0) {
		BROWSER_LOGD("url expression");
		regfree(&regex);
		return EINA_TRUE;
	}

	regfree(&regex);
	if (!strcmp(uri, blank_page))
		return EINA_TRUE;

	int len = strlen(uri);
	if (*uri != '.' && *(uri + len - 1) != '.' && strstr(uri, ".")) {
		BROWSER_LOGD("url tmp expression");
		return EINA_TRUE;
	}

	return EINA_FALSE;
}

platform_service::platform_service(void)
{
	//BROWSER_LOGD("");
}

platform_service::~platform_service(void)
{
	//BROWSER_LOGD("");
}

Eina_Bool platform_service::remove_file(const char *path)
{
	BROWSER_SECURE_LOGD("removing..[%s]", path);

	if (path && strlen(path)) {
		if (unlink(path) == -1) {
			BROWSER_LOGE("Failed to remove file in the path");
			return EINA_FALSE;
		}
	} else {
		BROWSER_LOGE("Failed to remove file Path is invalid");
		return EINA_FALSE;
	}
	return EINA_TRUE;
}

void platform_service::evas_image_size_get(Evas_Object *image, int *w, int *h, int *stride)
{
	RET_MSG_IF(!image, "image is NULL");
	RET_MSG_IF(!w, "w is NULL");
	RET_MSG_IF(!h, "h is NULL");

	const char *evas_obj_type = evas_object_type_get(image);
	if (evas_obj_type && !strcmp(evas_obj_type, "image")) {
		evas_object_image_size_get(image, w, h);

		if (stride) {
			*stride = evas_object_image_stride_get(image);
			//BROWSER_LOGD("snapshot w=[%d], h=[%d], stride=[%d]", *w, *h, *stride);
		} else
			BROWSER_LOGD("snapshot w=[%d], h=[%d]", *w, *h);
	} else {
		*w = 0;
		*h = 0;
		if (stride)
			*stride = 0;
	}
}

int platform_service::get_png_file_image_size(const char *path, int *w, int *h)
{
	BROWSER_LOGD("");
	if (!path || (strlen(path) == 0))
		return -1;

	cairo_surface_t *img_surface = cairo_image_surface_create_from_png(path);

	if (img_surface) {
		*w = cairo_image_surface_get_width(img_surface);
		*h = cairo_image_surface_get_height(img_surface);
	} else {
		*w = 0;
		*h = 0;
		return -1;
	}

	cairo_surface_destroy(img_surface);
	return 1;
}

Evas_Object *platform_service::copy_evas_image(Evas_Object *origin_image)
{
	RETV_MSG_IF(!origin_image, NULL, "origin_image is NULL");

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

Eina_Bool platform_service::copy_evas_image(Evas_Object *origin_image, Evas_Object *target_image)
{
	RETV_MSG_IF(!origin_image, EINA_FALSE, "origin_image is NULL");

	int w, h, stride;
	evas_image_size_get(origin_image, &w, &h, &stride);

	if (!w || !h)
		return EINA_FALSE;

	void *origin_image_data = evas_object_image_data_get(origin_image, EINA_TRUE);

	evas_object_image_colorspace_set(target_image, EVAS_COLORSPACE_ARGB8888);
	evas_object_image_size_set(target_image, w, h);
	evas_object_image_fill_set(target_image, 0, 0, w, h);
	evas_object_image_filled_set(target_image, EINA_TRUE);
	evas_object_image_alpha_set(target_image,EINA_TRUE);

	void *target_image_data = evas_object_image_data_get(target_image, EINA_TRUE);
	memcpy(target_image_data, origin_image_data, stride * h);
	evas_object_image_data_set(target_image, target_image_data);

	return EINA_TRUE;
}

Eina_Bool platform_service::is_png(const char *file_path)
{
	BROWSER_LOGD("");
	if (!file_path || (strlen(file_path) == 0))
		return EINA_FALSE;

	FILE *file_read = NULL;
	char file_header[8];
	/* Read raw data from default config_sample.xml */
	file_read = fopen(file_path, "r");

	if (!file_read) {
		BROWSER_LOGE("failed to open input file");
		return EINA_FALSE;
	}

	size_t result = fread(file_header, sizeof(char) * 8, 1, file_read);
	BROWSER_LOGE("result: %d", result);

	if (result == 1) {
		if (file_header[0] == 0x89 && file_header[1] == 0x50 && file_header[2] == 0x4E
			&& file_header[3] == 0x47 && file_header[4] == 0x0D && file_header[5] == 0x0A
			&& file_header[6] == 0x1A && file_header[7] == 0x0A) {
			BROWSER_LOGD("This file is PNG image file");
			fclose(file_read);
			return EINA_TRUE;
		}
	}

	BROWSER_LOGD("This file is NOT PNG image file");
	fclose(file_read);
	return EINA_FALSE;
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
	char *raw_data = strchr(langset, '.');
	/* 7 is reserved length for "system_lang" + '_' + "system_region" + '.' + at least 1 char. */
	if (raw_data && strlen(langset) >= 7) {
		free(langset);
		return strdup(raw_data + 1);
	}
	free(langset);
	return NULL;
}

void platform_service::get_touch_point_by_degree(int *x, int *y)
{
	// Get the touch point based on screen.
	int touch_x, touch_y;
	ecore_x_pointer_last_xy_get(&touch_x, &touch_y);

	int window_w, window_h;
	evas_object_geometry_get(m_window, NULL, NULL, &window_w, &window_h);

	int degree = elm_win_rotation_get(m_window);
	if (degree == 0) {
		*x = touch_x;
		*y = touch_y;
		return;
	}

	if (degree == 90) {
		*x = window_w - touch_y;
		*y = touch_x;
		return;
	}

	if (degree == 180) {
		*x = window_w - touch_x;
		*y = window_h - touch_y;
		return;
	}

	if (degree == 270) {
		*x = touch_y;
		*y = window_h - touch_x;
		return;
	}
}

unsigned long long platform_service::get_file_size(const char *full_path)
{
	FILE *fp = NULL;
	unsigned long long size = 0;

	fp = fopen(full_path, "r");
	if (fp) {
		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);
		fclose(fp);
	}
	BROWSER_LOGD("size [%ld]", size);

	return size;
}

char *platform_service::get_file_size_str(const char *full_path)
{
	RETV_SECURE_MSG_IF(!(full_path && strlen(full_path)), NULL, "full_path is invalid");

	FILE *fp = NULL;
	unsigned long long size = 0;
	double size_double = 0.0f;
	char size_str[10 + 1] = {0, };
	std::string size_text;

	fp = fopen(full_path, "r");
	if (!fp)
		return NULL;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fclose(fp);

	if (size >= unit_size) {
		size_double = (double)size / (double)unit_size;
		if (size_double >= unit_size) {
			size_double = (double)size_double / (double)unit_size;
			if (size_double >= unit_size) {
				size_double = (double)size_double / (double)unit_size;
				snprintf(size_str, 10, "%.2f", size_double);
				size_text = std::string(size_str) + std::string("GB");
			} else {
				snprintf(size_str, 10, "%.2f", size_double);
				size_text = std::string(size_str) + std::string("MB");
			}
		} else {
			snprintf(size_str, 10, "%.2f", size_double);
			size_text = std::string(size_str) + std::string("KB");
		}
	} else {
		snprintf(size_str, 10, "%u", (int)size);
		size_text = std::string(size_str) + std::string("B");
	}

	return strdup(size_text.c_str());
}

void platform_service::get_more_ctxpopup_position(int *x, int *y)
{
	int w = 0;
	int h = 0;

	elm_win_screen_size_get(m_window, NULL, NULL, &w, &h);
	int pos = elm_win_rotation_get(m_window);
	switch (pos) {
		case 0:
		case 180:
			*x = w/2 ;
			*y = h;
			break;
		case 90:
		case 270:
			*x = h/2 ;
			*y = w;
			break;
	}
}

/* Work arround with IME Status. */
/* In docomo version, 'cause of word prediction in entry,
	virtualkeypad,state,off is called when prediction is hide with HW keyboard
	To avoid hiding uri input bar, return here without calling show_uri_input_bar(EINA_FALSE) */
Eina_Bool platform_service::check_hw_usb_keyboard_alive(void)
{
	unsigned int val = 0;
	int res = 0;
	res = ecore_x_window_prop_card32_get(ecore_x_window_root_first_get(), ecore_x_atom_get(PROP_X_EXT_KEYBOARD_INPUT_DETECTED), &val, 1);
	if (res < 0)
		BROWSER_LOGE("Failed to get usb/hw keyboard status. Keep goting");
	else {
		if (val == 1) /* 1 means usb-hw keypad is alive */
			return EINA_TRUE;
	}

	return EINA_FALSE;
}

void platform_service::print_pdf(const char *path)
{
	BROWSER_LOGD("");

	app_control_h app_control;

	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return;
	}

	if (app_control_set_operation(app_control, print_service_name) < 0) {
		BROWSER_LOGE("Fail to launch app_control_set_operation()");
		app_control_destroy(app_control);
		return;
	}

	if (app_control_add_extra_data(app_control, SERVICE_PRINT_TITLE, "From Browser") < 0) {
		BROWSER_LOGE("Fail to launch app_control_set_operation()");
		app_control_destroy(app_control);
		return;
	}

	const char **files = (const char **)malloc(sizeof(char *));
	if(!files) {
		BROWSER_LOGE("malloc failed");
		app_control_destroy(app_control);
		return;
	}
	files[0] = path;
	if (app_control_add_extra_data_array(app_control, SERVICE_PRINT_CONTENT, (const char **)files, 1) < 0) {
		BROWSER_LOGE("Fail to launch app_control_add_extra_data_array()");
		app_control_destroy(app_control);
		free(files);
		return;
	}

	if (app_control_add_extra_data(app_control, SERVICE_PRINT_CONTENT_COUNT, "1") < 0) {
		BROWSER_LOGE("Fail to launch app_control_add_extra_data()");
		app_control_destroy(app_control);
		free(files);
		return;
	}

	if (app_control_add_extra_data(app_control, SERVICE_PRINT_CONTENT_TYPE, "DOC") < 0) {
		BROWSER_LOGE("Fail to launch app_control_add_extra_data()");
		app_control_destroy(app_control);
		free(files);
		return;
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		free(files);
		return;
	}
	app_control_destroy(app_control);

	free(files);
}

void platform_service::resize_webview_snapshot(Evas_Object *snapshot)
{
	if (!snapshot)
		return;

	int snapshot_w = 0;
	int snapshot_h = 0;
	evas_object_image_size_get(snapshot, &snapshot_w, &snapshot_h);
	BROWSER_LOGD("snapshot: %p, %d, %d", snapshot, snapshot_w, snapshot_h);

	int window_w = 0;
	int window_h = 0;
	evas_object_geometry_get(m_window, NULL, NULL, &window_w, &window_h);

	int webview_w = 0;
	int webview_h = 0;
	webview_w = window_w;
	webview_h = window_h - 98; // 98: uri bar height
	BROWSER_LOGD("webview: %d, %d", webview_w, webview_h);

	int min_w = snapshot_w < webview_w ? snapshot_w : webview_w;
	int min_h = snapshot_h < webview_h ? snapshot_h : webview_h;
	evas_object_size_hint_min_set(snapshot, ELM_SCALE_SIZE(min_w), ELM_SCALE_SIZE(min_h));
	evas_object_resize(snapshot, min_w, min_h);
}

Eina_Bool platform_service::capture_screen_to_png(Evas_Object *window, const char *target_path, int requested_w, int requested_h)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!window, EINA_FALSE, "window is NULL");
	RETV_MSG_IF(!(target_path && strlen(target_path) > 0), EINA_FALSE, "Invalid target path");

	Ecore_X_Window win_id = elm_win_xwindow_get(window);
	RETV_MSG_IF(win_id == 0, EINA_FALSE, "Failed to get window ID");

	/* get window attribute */
	int window_width = 0;
	int window_height = 0;
	int depth = 0;
	int size = 0;

	Visual *visual;
	XWindowAttributes attr;
	if (!XGetWindowAttributes((Display *)ecore_x_display_get(), win_id, &attr)) {
		BROWSER_LOGE("Failed to XGetWindowAttributes");
		return EINA_FALSE;
	}

	depth = attr.depth;
	window_width = attr.width;
	window_height = attr.height;
	visual = attr.visual;

	BROWSER_LOGD("depth[%d], width[%d], height[%d]", depth, window_width, window_height);

	char *img = NULL;
	img = brui_capture_window(win_id, visual, window_width, window_height, depth, &size);
	RETV_MSG_IF(img == NULL, EINA_FALSE, "Failed to brui_capture_window");

	int target_w = window_width;
	int target_h = window_height;

	if (requested_w < 1 || requested_h < 1) {
		/* Do not resize - invalid reisizing target */
	} else if (requested_w == window_width && requested_h == window_height) {
		/* Do not have to resize */
	} else {
		/* resize to requested */
		target_w = requested_w;
		target_h = requested_h;

		brui_resize_captured_window(img, img, window_width, window_height, target_w, target_h);
	}

	int angle = brui_get_window_angle(win_id);
	BROWSER_LOGD("angle[%d], target_w[%d], target_h[%d]", angle, target_w, target_h);

	if (brui_make_capture_file(target_path, target_w, target_h, img, angle) == EINA_FALSE) {
		BROWSER_LOGE("Failed to brui_make_capture_file");
		free(img);
		return EINA_FALSE;
	}

	free(img);

	return EINA_TRUE;
}

Evas_Object *platform_service::get_captured_screen_from_png(Evas_Object *window, const char *target_path)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!window, NULL, "window is NULL");
	RETV_MSG_IF(!(target_path && strlen(target_path) > 0), EINA_FALSE, "Invalid target path");

	int w = 0, h = 0;

	get_png_file_image_size(target_path, &w, &h);
	BROWSER_LOGD("w[%d], h[%d]", w, h);

	Evas_Object *screenshotImage = evas_object_image_add(evas_object_evas_get(window));
	cairo_surface_t *surface = cairo_image_surface_create_from_png(target_path);

	int surfaceWidth = cairo_image_surface_get_width(surface);
	int surfaceHeight = cairo_image_surface_get_height(surface);

	const unsigned char *pixels = (const unsigned char *)cairo_image_surface_get_data(surface);

	evas_object_image_size_set(screenshotImage, surfaceWidth, surfaceHeight);
	evas_object_image_colorspace_set(screenshotImage, EVAS_COLORSPACE_ARGB8888);

	evas_object_image_smooth_scale_set(screenshotImage, true);
	evas_object_size_hint_min_set(screenshotImage, ELM_SCALE_SIZE(surfaceWidth), ELM_SCALE_SIZE(surfaceHeight));
	evas_object_resize(screenshotImage, surfaceWidth, surfaceHeight);
	evas_object_image_fill_set(screenshotImage, 0, 0, surfaceWidth, surfaceHeight);
	evas_object_image_data_set(screenshotImage, (void *)pixels);

	screenshotImage = copy_evas_image(screenshotImage);
	cairo_surface_destroy(surface);

	return screenshotImage;

}


Evas_Object *platform_service::capture_screen_to_evas_obj(Evas_Object *window, int requested_w, int requested_h)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!window, NULL, "window is NULL");

	Ecore_X_Window win_id = elm_win_xwindow_get(window);
	RETV_MSG_IF(win_id == 0, EINA_FALSE, "Failed to get window ID");

	/* get window attribute */
	int window_width = 0;
	int window_height = 0;
	int window_depth = 0;
	int window_size = 0;

	Visual *visual;
	XWindowAttributes attr;
	if (!XGetWindowAttributes((Display *)ecore_x_display_get(), win_id, &attr)) {
		BROWSER_LOGE("Failed to XGetWindowAttributes");
		return EINA_FALSE;
	}

	window_depth = attr.depth;
	window_width = attr.width;
	window_height = attr.height;
	visual = attr.visual;

	BROWSER_LOGD("depth[%d], width[%d], height[%d]", window_depth, window_width, window_height);

	char *img = NULL;
	img = brui_capture_window(win_id, visual, window_width, window_height, window_depth, &window_size);
	RETV_MSG_IF(img == NULL, EINA_FALSE, "Failed to brui_capture_window");

	int target_w = window_width;
	int target_h = window_height;

	if (requested_w < 1 || requested_h < 1) {
		/* Do not resize - invalid reisizing target */
	} else if (requested_w == window_width && requested_h == window_height) {
		/* Do not have to resize */
	} else {
		/* resize to requested */
		target_w = requested_w;
		target_h = requested_h;
		brui_resize_captured_window(img, img, window_width, window_height, target_w, target_h);
	}

	int angle = brui_get_window_angle(win_id);
	BROWSER_LOGD("angle[%d], target_w[%d], target_h[%d]", angle, target_w, target_h);

	Evas_Object *target_image = NULL;
	if ((target_image = brui_make_capture_evas_image(target_w, target_h, img, angle)) == NULL) {
		BROWSER_LOGE("Failed to brui_make_capture_file");
		free(img);
		return NULL;
	}

	free(img);
	BROWSER_LOGD("target_image[%p]", target_image);

	return target_image;
}


void platform_service::clip_obj_size_to_clipboard(Evas_Object *obj, int* w, int *h)
{
	int clipboard_x = 0, clipboard_y= 0, clipboard_w = 0, clipboard_h = 0;
	int obj_x = 0, obj_y= 0;

	if (!w || !h)
		return;

	ecore_x_e_illume_clipboard_geometry_get(ecore_x_window_focus_get(),
		&clipboard_x, &clipboard_y, &clipboard_w, &clipboard_h);

	BROWSER_LOGD("Clipboard x, y, h, w: %d, %d, %d, %d",
		clipboard_x, clipboard_y, clipboard_w, clipboard_h);

	evas_object_geometry_get(obj, &obj_x, &obj_y, w, h);

	if (clipboard_w && clipboard_h) {
		if (obj_x + *w > clipboard_x)
			*w = clipboard_x - obj_x;

		if (obj_y + *h > clipboard_y)
			*h = clipboard_y - obj_y;
	}
}
