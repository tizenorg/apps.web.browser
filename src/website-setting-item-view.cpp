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

#include "website-setting-item-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "common-view.h"
#include "setting-view.h"
#include "website-setting-view.h"

website_setting_item_view::website_setting_item_view(website_setting_item *item)
:
	m_item(item)
	,m_menu_type(WEBSITE_SETTING_ITEM_VIEW_MENU_UNKNOWN)
	,m_genlist(NULL)
	,m_geolocation_item_ic(NULL)
	,m_web_storage_item_ic(NULL)
	,m_geolocatin_item_it(NULL)
	,m_webstorage_item_it(NULL)
{
	EINA_SAFETY_ON_NULL_RETURN(item);
}

website_setting_item_view::~website_setting_item_view(void)
{
	BROWSER_LOGD("");

	if (m_geolocation_item_ic)
		elm_genlist_item_class_free(m_geolocation_item_ic);

	if (m_web_storage_item_ic)
		elm_genlist_item_class_free(m_web_storage_item_ic);

	evas_object_smart_callback_del(m_naviframe, "title,clicked", __title_clicked_cb);
}

void website_setting_item_view::show(void)
{
	BROWSER_LOGD("");

	Evas_Object *layout = _create_main_layout(m_naviframe);
	Elm_Object_Item *navi_it = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, layout, NULL);

	Evas_Object *label = elm_label_add(m_naviframe);
	if (!label) {
		BROWSER_LOGE("elm_label_add failed.");
		return;
	}
	elm_object_style_set(label, "naviframe_title");
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_wrap_width_set(label, EINA_TRUE);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_object_text_set(label, m_item->get_uri());
	evas_object_show(label);
	elm_object_item_part_content_set(navi_it, "elm.swallow.title", label);

	evas_object_smart_callback_add(m_naviframe, "title,clicked", __title_clicked_cb, this);
}

Evas_Object *website_setting_item_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	m_genlist = _create_genlist(parent);
	if (!m_genlist) {
		BROWSER_LOGE("_create_main_layout failed");
		return NULL;
	}

	if (m_item->check_has_geolocation_data() == EINA_TRUE) {
		m_geolocatin_item_it = elm_genlist_item_append(m_genlist,
		m_geolocation_item_ic, this, NULL, ELM_GENLIST_ITEM_NONE,
		__genlist_clear_geolocation_clicked_cb, this);
	}

	if (m_item->check_has_web_storage_data() == EINA_TRUE) {
		m_webstorage_item_it = elm_genlist_item_append(m_genlist,
		m_web_storage_item_ic, this, NULL, ELM_GENLIST_ITEM_NONE,
		__genlist_clear_web_storage_clicked_cb, this);
	}

	return m_genlist;
}

Evas_Object *website_setting_item_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	Elm_Genlist_Item_Class *geolocation_item_ic = elm_genlist_item_class_new();
	if (!geolocation_item_ic) {
		BROWSER_LOGE("failed to get geolocation_item_ic");
		return NULL;
	}
	memset(geolocation_item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));
	geolocation_item_ic->item_style = "1text.1icon";
	geolocation_item_ic->func.text_get = __genlist_geolocation_label_get_cb;
	geolocation_item_ic->func.content_get = __genlist_geolocation_icon_get_cb;
	geolocation_item_ic->func.state_get = NULL;
	geolocation_item_ic->func.del = NULL;
	m_geolocation_item_ic = geolocation_item_ic;

	Elm_Genlist_Item_Class *web_storage_item_ic = elm_genlist_item_class_new();
	if (!web_storage_item_ic) {
		BROWSER_LOGE("failed to get web_storage_item_ic");
		return NULL;
	}
	memset(web_storage_item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));
	web_storage_item_ic->item_style = "1text.1icon";
	web_storage_item_ic->func.text_get = __genlist_web_storate_label_get_cb;
	web_storage_item_ic->func.content_get = __genlist_web_storate_icon_get_cb;
	web_storage_item_ic->func.state_get = NULL;
	web_storage_item_ic->func.del = NULL;
	m_web_storage_item_ic = web_storage_item_ic;

	return genlist;
}

void website_setting_item_view::_refresh_item_view(void)
{
	BROWSER_LOGD("");
	elm_genlist_clear(m_genlist);

	if (m_item->check_has_geolocation_data() == EINA_TRUE) {
		m_geolocatin_item_it = elm_genlist_item_append(m_genlist,
													m_geolocation_item_ic, this, NULL, ELM_GENLIST_ITEM_NONE,
													__genlist_clear_geolocation_clicked_cb, this);
	}

	if 	(m_item->check_has_web_storage_data() == EINA_TRUE) {
		m_webstorage_item_it = elm_genlist_item_append(m_genlist,
													m_web_storage_item_ic, this, NULL, ELM_GENLIST_ITEM_NONE,
													__genlist_clear_web_storage_clicked_cb, this);
	}
}

char *website_setting_item_view::__genlist_geolocation_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

	website_setting_item_view *item_view = (website_setting_item_view *)data;
	website_setting_item *item = item_view->m_item;

	if (!strcmp(part,"elm.text")) {
		if (item->check_has_geolocation_data() == EINA_TRUE)
			return strdup(BR_STRING_CLEAR_LOCATION_ACCESS);
	}
	return NULL;
}

Evas_Object *website_setting_item_view::__genlist_geolocation_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

	website_setting_item_view *item_view = (website_setting_item_view *)data;
	website_setting_item *item = item_view->m_item;

	if (!strcmp(part,"elm.icon")) {
		if (item->check_has_geolocation_data() == EINA_TRUE) {
			Evas_Object *location_icon = elm_icon_add(obj);
			if (!location_icon)
				return NULL;
			if (item->get_geolocation_allow() == true) {
				if (!elm_icon_file_set(location_icon, browser_img_dir"/I01_icon_location_allowed_small.png", NULL)) {
					BROWSER_LOGE("elm_icon_file_set is failed.\n");
					return NULL;
				}
				evas_object_size_hint_aspect_set(location_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
				return location_icon;
			} else {
				if (!elm_icon_file_set(location_icon, browser_img_dir"/I01_icon_location_denied_small.png", NULL)) {
					BROWSER_LOGE("elm_icon_file_set is failed.\n");
					return NULL;
				}
				evas_object_size_hint_aspect_set(location_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
				return location_icon;
			}
		} else if (item->check_has_web_storage_data() == EINA_TRUE) {
			BROWSER_LOGD("web storage menu.\n");
			Evas_Object *storage_icon = elm_icon_add(obj);
			if (!storage_icon)
				return NULL;
			if (!elm_icon_file_set(storage_icon, browser_img_dir"/I01_icon_web_storage_small.png", NULL)) {
				BROWSER_LOGE("elm_icon_file_set is failed.\n");
				return NULL;
			}
			evas_object_size_hint_aspect_set(storage_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return storage_icon;
		} else {
			BROWSER_LOGD("wrong menu index.\n");
			return NULL;
		}
	}
	return NULL;
}

void website_setting_item_view::__genlist_clear_geolocation_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	website_setting_item_view *item_view = (website_setting_item_view *)data;
	website_setting_item *item = item_view->m_item;

	elm_genlist_item_selected_set(item_view->m_geolocatin_item_it, EINA_FALSE);

	if (item->delete_geolocation_item(item->get_uri()) == EINA_FALSE) {
		/* show error popup */
		return;
	}

	if (item->check_has_web_storage_data() == EINA_FALSE) {
		if (elm_naviframe_bottom_item_get(m_naviframe) != elm_naviframe_top_item_get(m_naviframe))
			elm_naviframe_item_pop(m_naviframe);

		/* refresh website setting view */
		m_browser->get_setting_view()->get_website_setting_view()->refresh_view();
	} else
		item_view->_refresh_item_view();
}

char *website_setting_item_view::__genlist_web_storate_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

	website_setting_item_view *item_view = (website_setting_item_view *)data;
	website_setting_item *item = item_view->m_item;

	if (!strcmp(part,"elm.text")) {
		if (item->check_has_web_storage_data() == EINA_TRUE)
			return strdup(BR_STRING_CLEAR_STORED_DATA);
	}
	return NULL;
}

Evas_Object *website_setting_item_view::__genlist_web_storate_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

	website_setting_item_view *item_view = (website_setting_item_view *)data;
	website_setting_item *item = item_view->m_item;

	if (part && strlen(part) > 0) {
		if (!strcmp(part,"elm.icon")) {
			if (item->check_has_web_storage_data() == EINA_TRUE) {
				BROWSER_LOGD("web storage menu.\n");
				Evas_Object *storage_icon = elm_icon_add(obj);
				if (!storage_icon)
					return NULL;
				if (!elm_icon_file_set(storage_icon, browser_img_dir"/I01_icon_web_storage_small.png", NULL)) {
					BROWSER_LOGE("elm_icon_file_set is failed.\n");
					return NULL;
				}
				evas_object_size_hint_aspect_set(storage_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
				return storage_icon;
			} else {
				BROWSER_LOGD("wrong menu index.\n");
				return NULL;
			}
		}
	}
	return NULL;
}

void website_setting_item_view::__genlist_clear_web_storage_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	website_setting_item_view *item_view = (website_setting_item_view *)data;
	website_setting_item *item = item_view->m_item;

	elm_genlist_item_selected_set(item_view->m_webstorage_item_it, EINA_FALSE);

	if (item->delete_web_storage_item(item->get_origin()) == EINA_FALSE) {
		/* show error popup */
		return;
	}

	if (item->check_has_web_storage_data() == EINA_FALSE) {
		if (elm_naviframe_bottom_item_get(m_naviframe) != elm_naviframe_top_item_get(m_naviframe))
			elm_naviframe_item_pop(m_naviframe);
			/* refresh website setting view */
			m_browser->get_setting_view()->get_website_setting_view()->refresh_view();
	} else {
		item_view->_refresh_item_view();
	}
}

void website_setting_item_view::__title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	Elm_Object_Item *naviframe_item = (Elm_Object_Item *)event_info;
	Evas_Object *label = elm_object_item_part_content_get(naviframe_item, "elm.swallow.title");
	EINA_SAFETY_ON_NULL_RETURN(label);

	elm_label_slide_go(label);
}

