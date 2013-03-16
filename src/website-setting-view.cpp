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

#include "website-setting-view.h"

#include <Elementary.h>
#include <app.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "setting-view.h"
#include "webview.h"

website_setting_view::website_setting_view(void)
:
	m_item_ic(NULL)
	,m_genlist(NULL)
	,m_delete_all_button(NULL)
	,m_website_setting_manager(NULL)
	,m_naviframe_item(NULL)
{
	BROWSER_LOGD("");

	if (!m_website_setting_manager)
		m_website_setting_manager = get_website_setting_manager();
}

website_setting_view::~website_setting_view(void)
{
	BROWSER_LOGD("");

	if (m_item_ic)
		elm_genlist_item_class_free(m_item_ic);

	if (m_website_setting_manager)
		delete m_website_setting_manager;

	evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_cb);
	evas_object_smart_callback_del(m_naviframe, "title,clicked", __title_clicked_cb);
}

void website_setting_view::show(void)
{
	BROWSER_LOGD("");

	Evas_Object *layout = _create_main_layout(m_naviframe);
	m_naviframe_item = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, layout, NULL);

	m_delete_all_button = elm_button_add(m_naviframe);
	EINA_SAFETY_ON_NULL_RETURN(m_delete_all_button);

	elm_object_text_set(m_delete_all_button, BR_STRING_DELETE_ALL);
	evas_object_smart_callback_add(m_delete_all_button, "clicked", __delete_all_btn_cb, this);

	elm_object_style_set(m_delete_all_button, "naviframe/toolbar/default");
	elm_object_item_part_content_set(m_naviframe_item, "toolbar_button1", m_delete_all_button);
	evas_object_show(m_delete_all_button);

	if ((get_website_setting_manager()->get_website_setting_list()).size() > 0)
		delete_all_button_disabled_set(EINA_FALSE);
	else
		delete_all_button_disabled_set(EINA_TRUE);

	Evas_Object *label = elm_label_add(m_naviframe);
	EINA_SAFETY_ON_NULL_RETURN(label);

	elm_object_style_set(label, "naviframe_title");
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_wrap_width_set(label, EINA_TRUE);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_object_text_set(label, BR_STRING_WEBSITE_SETTINGS);
	evas_object_show(label);
	elm_object_item_part_content_set(m_naviframe_item, "elm.swallow.title", label);

	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_cb, this);
	evas_object_smart_callback_add(m_naviframe, "title,clicked", __title_clicked_cb, this);
}

void website_setting_view::refresh_view(void)
{
	BROWSER_LOGD("");

	m_website_setting_manager->delete_all_geolocation_data();
	m_website_setting_manager->delete_all_web_storate_data();

	elm_genlist_clear(m_genlist);

	std::vector<website_setting_item *> website_setting_list = get_website_setting_manager()->get_website_setting_list();

	for (int i = 0 ; i < website_setting_list.size() ; i++) {
		Elm_Object_Item *it = NULL;
		it = elm_genlist_item_append(m_genlist, m_item_ic,
									website_setting_list[i], NULL, ELM_GENLIST_ITEM_NONE,
									__item_selected_cb, website_setting_list[i]);
		website_setting_list[i]->set_user_data((void *)it);
	}

	if ((get_website_setting_manager()->get_website_setting_list()).size() > 0)
		delete_all_button_disabled_set(EINA_FALSE);
	else
		delete_all_button_disabled_set(EINA_TRUE);
}

void website_setting_view::add_website_setting_item_in_genlist(website_setting_item *item)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(item);

	Elm_Object_Item *it;
	it = elm_genlist_item_append(m_genlist, m_item_ic,
								item, NULL, ELM_GENLIST_ITEM_NONE,
								__item_selected_cb, item);
	item->set_user_data((void *)it);
}

website_setting_manager *website_setting_view::get_website_setting_manager(void)
{
	BROWSER_LOGD("");
	if (!m_website_setting_manager)
		m_website_setting_manager = new website_setting_manager();

	return m_website_setting_manager;
}

void website_setting_view::delete_all_button_disabled_set(Eina_Bool disabled)
{
	BROWSER_LOGD("disabled[%d]", disabled);

	if (disabled == EINA_TRUE)
		elm_object_disabled_set(m_delete_all_button, EINA_TRUE);
	else
		elm_object_disabled_set(m_delete_all_button, EINA_FALSE);
}

Evas_Object *website_setting_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	m_genlist = _create_genlist(parent);

	std::vector<website_setting_item *> website_setting_list = get_website_setting_manager()->get_website_setting_list();

	for (int i = 0 ; i < website_setting_list.size() ; i++) {
		Elm_Object_Item *it = NULL;
		it = elm_genlist_item_append(m_genlist, m_item_ic,
									website_setting_list[i], NULL, ELM_GENLIST_ITEM_NONE,
									__item_selected_cb, website_setting_list[i]);
		website_setting_list[i]->set_user_data((void *)it);
	}

	return m_genlist;
}

Evas_Object *website_setting_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	memset(item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));

	/*
	//Not supported yet
	item_ic->item_style = "2text.3icon.4";
	*/
	item_ic->item_style = "1text.3icon.2";
	item_ic->func.text_get = __genlist_label_get_cb;
	item_ic->func.content_get = __genlist_icon_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

	m_item_ic = item_ic;

	return genlist;
}

void website_setting_view::_delete_website_setting_manager(void)
{
	if (m_website_setting_manager)
		delete m_website_setting_manager;
	m_website_setting_manager = NULL;
}


char *website_setting_view::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

	website_setting_item *item = (website_setting_item *)data;

	if (part && strlen(part) > 0) {
		if (!strcmp(part,"elm.text")) {
			if (item->get_uri() && strlen(item->get_uri()))
				return strdup(item->get_uri());
		} else if (!strcmp(part,"elm.text.1")) {
			if (item->get_uri() && strlen(item->get_uri()))
				return strdup(item->get_uri());
		} else if (!strcmp(part,"elm.text.2")) {
			if (item->get_title() && strlen(item->get_title()))
				return strdup(item->get_title());
			else
				return strdup(BR_STRING_NO_TITLE);
		}
	}

	return NULL;
}

Evas_Object *website_setting_view::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

	website_setting_item *item = (website_setting_item *)data;
	if (!strcmp(part, "elm.icon.1")) {
		Evas_Object *favicon = NULL;
		favicon = m_webview_context->get_favicon(item->get_uri());
		if (favicon)
			return favicon;
		else {
			Evas_Object *default_icon = elm_icon_add(obj);
			if (!default_icon)
				return NULL;
			if (!elm_icon_file_set(default_icon, browser_img_dir"/faviconDefault.png", NULL)) {
				BROWSER_LOGE("elm_icon_file_set is failed.\n");
				return NULL;
			}
			evas_object_size_hint_aspect_set(default_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return default_icon;
		}
	} else if (!strcmp(part,"elm.icon.2")) {
		Evas_Object *location_icon = elm_icon_add(obj);
		if (!location_icon)
			return NULL;
		if (item->check_has_geolocation_data() == EINA_TRUE) {
			if (item->get_geolocation_allow() == true) {
				if (!elm_icon_file_set(location_icon, browser_img_dir"/I01_icon_location_allowed.png", NULL)) {
					BROWSER_LOGE("elm_icon_file_set is failed.\n");
					return NULL;
				}
				evas_object_size_hint_aspect_set(location_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
				return location_icon;
			} else {
				if (!elm_icon_file_set(location_icon, browser_img_dir"/I01_icon_location_denied.png", NULL)) {
					BROWSER_LOGE("elm_icon_file_set is failed.\n");
					return NULL;
				}
				evas_object_size_hint_aspect_set(location_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
				return location_icon;
			}
		}
	} else if (!strcmp(part,"elm.icon.3")) {
		Evas_Object *storage_icon = elm_icon_add(obj);
		if (!storage_icon)
			return NULL;
		if (item->check_has_web_storage_data() == EINA_TRUE) {
			if (!elm_icon_file_set(storage_icon, browser_img_dir"/I01_icon_web_storage.png", NULL)) {
				BROWSER_LOGE("elm_icon_file_set is failed.\n");
				return NULL;
			}
			evas_object_size_hint_aspect_set(storage_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return storage_icon;
		}
	}

	return NULL;
}

void website_setting_view::__delete_all_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	website_setting_view *wsv = (website_setting_view *)data;

	m_browser->get_browser_view()->show_msg_popup(NULL,
												BR_STRING_DELETE_ALL_WEBSITE_DATA_AND_LOCATION_PERMISSIONS,
												BR_STRING_YES,
												wsv->__delete_all_confirmed_cb,
												BR_STRING_NO,
												wsv->__delete_all_denied_cb,
												wsv);
}

void website_setting_view::__delete_all_confirmed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	website_setting_view *wsv = (website_setting_view *)data;
	wsv->get_website_setting_manager()->delete_all_website_settings();
	wsv->refresh_view();
}

void website_setting_view::__delete_all_denied_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
}

void website_setting_view::__item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	website_setting_item *item = (website_setting_item *)data;
	website_setting_view *wsv = (website_setting_view *)m_browser->get_setting_view()->get_website_setting_view();
	elm_genlist_item_selected_set((Elm_Object_Item *)item->get_user_data(), EINA_FALSE);
	wsv->m_selected_item = item;
	item->show();
}

void website_setting_view::__naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	website_setting_view *wsv = (website_setting_view *)data;
	if (wsv->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		return;

#if 0
	if (!wsv->m_selected_item)
		return;

	website_setting_item_view *current_item_view = wsv->m_selected_item->get_current_website_setting_item_view();

	if (current_item_view)
		delete current_item_view;
	wsv->m_selected_item = NULL;
#endif
}

void website_setting_view::__title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	Elm_Object_Item *naviframe_item = (Elm_Object_Item *)event_info;
	Evas_Object *label = elm_object_item_part_content_get(naviframe_item, "elm.swallow.title");
	EINA_SAFETY_ON_NULL_RETURN(label);

	elm_label_slide_go(label);
}