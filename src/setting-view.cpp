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

#include "setting-view.h"

#include <vconf.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "geolocation-manager.h"
#include "history.h"
#include "preference.h"
#include "webview.h"

setting_view::setting_view(void)
:
	m_category_title_item_class(NULL)
	,m_2_text_item_class(NULL)
	,m_2_text_2_item_class(NULL)
	,m_2_text_3_item_class(NULL)
	,m_1_text_1_icon_item_class(NULL)
	,m_1_text_item_class(NULL)
	,m_2_text_1_icon_item_class(NULL)
	,m_radio_text_item_class(NULL)
	,m_seperator_item_class(NULL)
	,m_seperator_with_bottom_line_item_class(NULL)
	,m_naviframe_item(NULL)
	,m_clear_private_data_view(NULL)
	,m_homepage_edit_view(NULL)
	,m_text_encoding_type_view(NULL)
	,m_website_setting_view(NULL)
	,m_user_agent_view(NULL)
#if defined(TEST_CODE)
	,m_custom_user_agent_view(NULL)
#endif
{
	BROWSER_LOGD("");
}

setting_view::~setting_view(void)
{
	BROWSER_LOGD("");

	if (m_category_title_item_class)
		elm_genlist_item_class_free(m_category_title_item_class);
	if (m_2_text_item_class)
		elm_genlist_item_class_free(m_2_text_item_class);
	if (m_2_text_2_item_class)
		elm_genlist_item_class_free(m_2_text_2_item_class);
	if (m_2_text_3_item_class)
		elm_genlist_item_class_free(m_2_text_3_item_class);
	if (m_1_text_1_icon_item_class)
		elm_genlist_item_class_free(m_1_text_1_icon_item_class);
	if (m_1_text_item_class)
		elm_genlist_item_class_free(m_1_text_item_class);
	if (m_2_text_1_icon_item_class)
		elm_genlist_item_class_free(m_2_text_1_icon_item_class);
	if (m_radio_text_item_class)
		elm_genlist_item_class_free(m_radio_text_item_class);
	if (m_seperator_item_class)
		elm_genlist_item_class_free(m_seperator_item_class);
	if (m_seperator_with_bottom_line_item_class)
		elm_genlist_item_class_free(m_seperator_with_bottom_line_item_class);

	if (m_clear_private_data_view)
		delete m_clear_private_data_view;

	if(m_homepage_edit_view)
		delete m_homepage_edit_view;

	if (m_text_encoding_type_view)
		delete m_text_encoding_type_view;

	if (m_website_setting_view)
		delete m_website_setting_view;

	if (m_user_agent_view)
		delete m_user_agent_view;
#if defined(TEST_CODE)
	if (m_custom_user_agent_view)
		delete m_custom_user_agent_view;
#endif

	evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_cb);
	evas_object_smart_callback_del(m_naviframe, "title,clicked", __title_clicked_cb);
}

void setting_view::show(void)
{
	m_genlist = _create_genlist(m_naviframe);

	m_homepage_radio_group = elm_radio_add(m_genlist);
	if (!m_homepage_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		return;
	}
	elm_radio_state_value_set(m_homepage_radio_group, -1);

	m_default_view_level_radio_group = elm_radio_add(m_genlist);
	if (!m_default_view_level_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		return;
	}
	elm_radio_state_value_set(m_default_view_level_radio_group, -1);

	m_auto_save_id_pass_radio_group = elm_radio_add(m_genlist);
	if (!m_auto_save_id_pass_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		return;
	}
	elm_radio_state_value_set(m_auto_save_id_pass_radio_group, -1);

	m_naviframe_item = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, m_genlist, NULL);

	Evas_Object *label = elm_label_add(m_naviframe);
	if (!label) {
		BROWSER_LOGE("elm_label_add failed.");
		return;
	}
	elm_object_style_set(label, "naviframe_title");
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_wrap_width_set(label, EINA_TRUE);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_object_text_set(label, BR_STRING_SETTINGS);
	evas_object_show(label);
	elm_object_item_part_content_set(m_naviframe_item, "elm.swallow.title", label);

	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_cb, this);
	evas_object_smart_callback_add(m_naviframe, "title,clicked", __title_clicked_cb, this);

	return;
}

clear_private_data_view *setting_view::get_clear_private_data_view(void)
{
	BROWSER_LOGD("");
	if (!m_clear_private_data_view)
		m_clear_private_data_view = new clear_private_data_view();

	return m_clear_private_data_view;
}

void setting_view::_delete_clear_private_data_view(void)
{
	if (m_clear_private_data_view)
		delete m_clear_private_data_view;
	m_clear_private_data_view = NULL;
}

homepage_edit_view *setting_view::get_homepage_edit_view(void)
{
	BROWSER_LOGD("");
	if (!m_homepage_edit_view)
		m_homepage_edit_view = new homepage_edit_view();

	return m_homepage_edit_view;
}

void setting_view::_delete_homepage_edit_view(void)
{
	if (m_homepage_edit_view)
		delete m_homepage_edit_view;
	m_homepage_edit_view = NULL;
}

text_encoding_type_view *setting_view::get_text_encoding_type_view(void)
{
	BROWSER_LOGD("");
	if (!m_text_encoding_type_view)
		m_text_encoding_type_view = new text_encoding_type_view();

	return m_text_encoding_type_view;
}

void setting_view::_delete_text_encoding_type_view(void)
{
	if (m_text_encoding_type_view)
		delete m_text_encoding_type_view;
	m_text_encoding_type_view = NULL;
}

website_setting_view *setting_view::get_website_setting_view(void)
{
	BROWSER_LOGD("");
	if (!m_website_setting_view)
		m_website_setting_view = new website_setting_view();

	return m_website_setting_view;
}

void setting_view::_delete_website_setting_view(void)
{
	if (m_website_setting_view)
		delete m_website_setting_view;
	m_website_setting_view = NULL;
}

user_agent_view *setting_view::get_user_agent_view(void)
{
	BROWSER_LOGD("");
	if (!m_user_agent_view)
		m_user_agent_view = new user_agent_view();

	return m_user_agent_view;
}

void setting_view::_delete_user_agent_view(void)
{
	if (m_user_agent_view)
		delete m_user_agent_view;
	m_user_agent_view = NULL;
}

#if defined(TEST_CODE)
custom_user_agent_view *setting_view::get_custom_user_agent_view(void)
{
	BROWSER_LOGD("");
	if (!m_custom_user_agent_view)
		m_custom_user_agent_view = new custom_user_agent_view();

	return m_custom_user_agent_view;
}

void setting_view::_delete_custom_user_agent_view(void)
{
	if (m_custom_user_agent_view)
		delete m_custom_user_agent_view;
	m_custom_user_agent_view = NULL;
}
#endif

void setting_view::_reset_to_default_setting(void)
{
	BROWSER_LOGD("");

	m_preference->reset();

	if (vconf_set_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT, "System user agent") < 0)
		BROWSER_LOGE("vconf_set_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT, [System user agent]) failed");

	elm_genlist_realized_items_update(m_genlist);
}

Eina_Bool setting_view::_call_user_agent(void)
{
	BROWSER_LOGD("");

	struct ug_cbs cbs = {0, };
	cbs.layout_cb = __ug_layout_cb;
	cbs.result_cb = NULL; //__ug_result_cb;
	cbs.destroy_cb = __ug_destroy_cb;
	cbs.priv = (void *)this;

	service_h data = NULL;
	service_create(&data);
	if (data == NULL) {
		BROWSER_LOGE("fail to service_create.");
		return EINA_FALSE;
	}
	if (!ug_create(NULL, "browser-user-agent-efl", UG_MODE_FULLVIEW, data, &cbs))
		BROWSER_LOGE("ug_create is failed.");

	if (service_destroy(data))
		BROWSER_LOGE("service_destroy is failed.");

	return EINA_TRUE;
}

char *setting_view::__text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);

	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	setting_view *view = (setting_view *)callback_data->user_data;
	setting_view::menu_type type = callback_data->type;

	if (type == BR_HOMEPAGE_TITLE) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_HOMEPAGE);
	} else if (type == BR_HOMEPAGE_MENU) {
		if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_HOMEPAGE);
		else if (!strcmp(part, "elm.text.2")) {
			homepage_type homepage = m_preference->get_homepage_type();

			BROWSER_LOGD("homepage = [%d]", homepage);
			if (homepage == HOMEPAGE_TYPE_RECENTLY_VISITED_SITE)
				return strdup(BR_STRING_LAST_VIEWED_PAGE);
			else if (homepage == HOMEPAGE_TYPE_EMPTY_PAGE)
				return strdup(BR_STRING_BLANK_PAGE);
			else if (homepage == HOMEPAGE_TYPE_USER_HOMEPAGE)
				return strdup(BR_STRING_USER_HOMEPAGE);
			else
				return NULL;
		}
	} else if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_LAST_VIEWED_PAGE);
	} else if (type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE) {
		if (!strcmp(part, "elm.text")) {
			char *homepage = m_preference->get_user_homagepage();
			if (!homepage) {
				BROWSER_LOGE("failed to get UserHomepage preference\n");
				return NULL;
			}
			return homepage;
		}
	} else if (type == BR_HOMEPAGE_SUBMENU_CURRENT_PAGE) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_CURRENT_PAGE);
	} else if (type == BR_HOMEPAGE_SUBMENU_EMPTY_PAGE) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_BLANK_PAGE);
	} else if (type == BR_CONTENT_MENU_DEFAULT_VIEW_LEVEL) {
		if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_DEFAULT_VIEW_LEVEL);
		else if (!strcmp(part, "elm.text.2")) {
			view_level_type view_level = m_preference->get_view_level_type();
			if (view_level == VIEW_LEVEL_TYPE_FIT_TO_WIDTH) {
				return strdup(BR_STRING_FIT_TO_WIDTH);
			} else {
				return strdup(BR_STRING_READABLE);
			}
			return NULL;
		}
	} else if (type == BR_CONTENT_SUBMENU_FIT_TO_WIDTH) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_FIT_TO_WIDTH);
	} else if (type == BR_CONTENT_SUBMENU_READABLE) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_READABLE);
	} else if (type == BR_CONTENT_TITLE) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_CONTENT_SETTINGS);
	} else if (type == BR_CONTENT_MENU_RUN_JAVASCRIPT) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_RUN_JAVASCRIPT);
	} else if (type == BR_CONTENT_MENU_DISPLAY_IMAGES) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_SHOW_IMAGES);
	} else if (type == BR_CONTENT_MENU_BLOCK_POPUP) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_BLOCK_POPUP);
	} else if (type == BR_CONTENT_MENU_ENCODING_TYPE) {
		if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_ENCODING_TYPE);
		else if (!strcmp(part, "elm.text.2")) {
			text_encoding_type type = m_preference->get_text_encoding_type_index();
			return view->get_text_encoding_type_view()->get_menu_label(type);
		}
	} else if (type == BR_PRIVACY_TITLE) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_PRIVACY_AND_SECURTY);
	} else if (type == BR_PRIVATE_MENU_CLEAR_CACHE) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_CLEAR_CACHE);
	} else if (type == BR_PRIVATE_MENU_CLEAR_HISTORY) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_CLEAR_HISTORY);
	} else if (type == BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_CLEAR_PERSONALISED_DATA);
	} else if (type == BR_PRIVATE_MENU_SHOW_SECURITY_WARNINGS) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_SHOW_SECURITY_WARNINGS);
	} else if (type == BR_PRIVACY_MENU_ACCEPT_COOKIES) {
		if (!strcmp(part, "elm.text.2"))
			return strdup(BR_STRING_COOKIES);
		else if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_ACCEPT_COOKIES);
	} else if (type == BR_PRIVATE_MENU_CLEAR_ALL_COOKIE_DATA) {
		if (!strcmp(part, "elm.text.2"))
			return strdup(BR_STRING_COOKIES);
		else if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_CLEAR_ALL_COOKIE_DATA);
	} else if (type == BR_PRIVACY_MENU_REMEMBER_FORM_DATA) {
		if (!strcmp(part, "elm.text.2"))
			return strdup(BR_STRING_FORMDATA);
		else if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_REMEMBER_FORMDATA);
	} else if (type == BR_PRIVACY_CLEAR_FORM_DATA) {
		if (!strcmp(part, "elm.text.2"))
			return strdup(BR_STRING_FORMDATA);
		else if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_CLEAR_FORMDATA);
	} else if (type == BR_PRIVACY_MENU_REMEMBER_PASSWORDS) {
		if (!strcmp(part, "elm.text.2"))
			return strdup(BR_STRING_PASSWORD);
		else if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_REMEMBER_PASSWORDS);
	} else if (type == BR_PRIVACY_CLEAR_PASSWORDS) {
		if (!strcmp(part, "elm.text.2"))
			return strdup(BR_STRING_PASSWORD);
		else if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_CLEAR_PASSWORDS);
	} else if (type == BR_PRIVACY_SUBMENU_ALWAYS_ASK) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_ALWAYS_ASK);
	} else if (type == BR_PRIVACY_SUBMENU_ALWAYS_ON) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_ON);
	} else if (type == BR_PRIVACY_SUBMENU_ALWAYS_OFF) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_OFF);
	} else if (type == BR_PRIVACY_MENU_ENABLE_LOCATION) {
		if (!strcmp(part, "elm.text.2"))
			return strdup(BR_STRING_LOCATION);
		else if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_ENABLE_LOCATION);
	} else if (type == BR_PRIVACY_MENU_CLEAR_LOCATION_ACCESS) {
		if (!strcmp(part, "elm.text.2"))
			return strdup(BR_STRING_LOCATION);
		else if (!strcmp(part, "elm.text.1"))
			return strdup(BR_STRING_CLEAR_LOCATION_ACCESS);
	} else if (type == BR_PRIVACY_WEBSITE_SETTING) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_WEBSITE_SETTINGS);
	} else if (type == BR_DEBUG_TITLE) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_DEVELOPER_MODE);
	} else if (type == BR_MENU_USER_AGENT) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_USER_AGENT);
	}
	else if (type == BR_MENU_ABOUT_BROWSER) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_ABOUT_BROWSER);
	}
#if defined(TEST_CODE)
	else if (type == BR_MENU_USER_CUSTOM_USER_AGENT) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_CUSTOM_USER_AGENT);
	}
#endif
	else if (type == BR_MENU_RESET_TO_DEFAULT) {
		if (!strcmp(part, "elm.text"))
			return strdup(BR_STRING_RESET_TO_DEFAULT);
	}

	return NULL;
}

Evas_Object *setting_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part=[%s]", part);

	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	setting_view::menu_type type = callback_data->type;
	setting_view *main_view = (setting_view *)(callback_data->user_data);

	BROWSER_LOGD("part=[%s], type=[%d]", part, type);
	if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE
	    || type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE
	    || type == BR_HOMEPAGE_SUBMENU_CURRENT_PAGE
	    || type == BR_HOMEPAGE_SUBMENU_EMPTY_PAGE) {
		if (!strcmp(part, "elm.icon")) {
			Evas_Object *radio_button = elm_radio_add(obj);
			if (radio_button) {
				if (type == BR_HOMEPAGE_SUBMENU_CURRENT_PAGE)
					elm_radio_state_value_set(radio_button, 0);
				else if (type == BR_HOMEPAGE_SUBMENU_EMPTY_PAGE)
					elm_radio_state_value_set(radio_button, 1);
				else if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE)
					elm_radio_state_value_set(radio_button, 2);
				else if (type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE)
					elm_radio_state_value_set(radio_button, 3);

				elm_radio_group_add(radio_button, main_view->m_homepage_radio_group);
				evas_object_propagate_events_set(radio_button, EINA_FALSE);
				evas_object_smart_callback_add(radio_button, "changed", __homepage_sub_item_clicked_cb, data);

				homepage_type homepage = m_preference->get_homepage_type();
				elm_radio_value_set(main_view->m_homepage_radio_group, homepage);
			}

			return radio_button;
		}
	} else if (type == BR_CONTENT_SUBMENU_FIT_TO_WIDTH
	     || type == BR_CONTENT_SUBMENU_READABLE) {
		if (!strcmp(part, "elm.icon")) {
			Evas_Object *radio_button = elm_radio_add(obj);
			if (radio_button) {
				if (type == BR_CONTENT_SUBMENU_FIT_TO_WIDTH)
					elm_radio_state_value_set(radio_button, 0);
				else
					elm_radio_state_value_set(radio_button, 1);

				elm_radio_group_add(radio_button, main_view->m_default_view_level_radio_group);

				view_level_type view_level = m_preference->get_view_level_type();

				if (view_level == VIEW_LEVEL_TYPE_FIT_TO_WIDTH)
					elm_radio_value_set(main_view->m_default_view_level_radio_group, 0);
				else
					elm_radio_value_set(main_view->m_default_view_level_radio_group, 1);
			}

			return radio_button;
		}
	} else if (type == BR_CONTENT_MENU_RUN_JAVASCRIPT) {
		if (!strcmp(part, "elm.icon")) {
			main_view->m_run_javascript_check = elm_check_add(obj);
			if (main_view->m_run_javascript_check) {
				elm_object_style_set(main_view->m_run_javascript_check, "on&off");
				evas_object_smart_callback_add(main_view->m_run_javascript_check, "changed",
						__run_javascript_check_changed_cb, main_view->m_run_javascript_check);

				Eina_Bool run_javascript = m_preference->get_javascript_enabled();
				elm_check_state_set(main_view->m_run_javascript_check, run_javascript);
				evas_object_propagate_events_set(main_view->m_run_javascript_check, EINA_FALSE);
			}
			return main_view->m_run_javascript_check;
		}
	} else if (type == BR_CONTENT_MENU_DISPLAY_IMAGES) {
		if (!strcmp(part, "elm.icon")) {
			main_view->m_display_images_check = elm_check_add(obj);
			if (main_view->m_display_images_check) {
				elm_object_style_set(main_view->m_display_images_check, "on&off");
				evas_object_smart_callback_add(main_view->m_display_images_check, "changed",
						__show_images_check_changed_cb, main_view->m_display_images_check);

				Eina_Bool display_images = m_preference->get_display_images_enabled();
				elm_check_state_set(main_view->m_display_images_check, display_images);
				evas_object_propagate_events_set(main_view->m_display_images_check, EINA_FALSE);
			}
			return main_view->m_display_images_check;
		}
	} else if (type == BR_CONTENT_MENU_BLOCK_POPUP) {
		if (!strcmp(part, "elm.icon")) {
			main_view->m_block_popup_check = elm_check_add(obj);
			if (main_view->m_block_popup_check) {
				elm_object_style_set(main_view->m_block_popup_check, "on&off");
				evas_object_smart_callback_add(main_view->m_block_popup_check, "changed",
						__block_popup_check_changed_cb, main_view->m_block_popup_check);

				Eina_Bool block_popup = m_preference->get_block_popup_enabled();
				elm_check_state_set(main_view->m_block_popup_check, block_popup);
				evas_object_propagate_events_set(main_view->m_block_popup_check, EINA_FALSE);
			}
			return main_view->m_block_popup_check;
		}
	} else if (type == BR_PRIVATE_MENU_SHOW_SECURITY_WARNINGS) {
		if (!strcmp(part, "elm.icon")) {
			main_view->m_show_security_warnings_check = elm_check_add(obj);
			if (main_view->m_show_security_warnings_check) {
				elm_object_style_set(main_view->m_show_security_warnings_check, "on&off");
				evas_object_smart_callback_add(main_view->m_show_security_warnings_check, "changed",
						__show_security_warnings_check_changed_cb, main_view->m_show_security_warnings_check);

				Eina_Bool show_security_warnings = m_preference->get_security_warnings_enabled();
				elm_check_state_set(main_view->m_show_security_warnings_check, show_security_warnings);
				evas_object_propagate_events_set(main_view->m_show_security_warnings_check, EINA_FALSE);
			}
			return main_view->m_show_security_warnings_check;
		}
	} else if (type == BR_PRIVACY_MENU_REMEMBER_FORM_DATA) {
		if (!strcmp(part, "elm.icon")) {
			main_view->m_auto_save_form_data_check = elm_check_add(obj);
			if (main_view->m_auto_save_form_data_check) {
				elm_object_style_set(main_view->m_auto_save_form_data_check, "on&off");
				evas_object_smart_callback_add(main_view->m_auto_save_form_data_check, "changed",
						__auto_save_form_data_check_changed_cb, main_view->m_auto_save_form_data_check);

				Eina_Bool auto_save_form = m_preference->get_auto_save_form_data_enabled();
				elm_check_state_set(main_view->m_auto_save_form_data_check, auto_save_form);
				evas_object_propagate_events_set(main_view->m_auto_save_form_data_check, EINA_FALSE);
			}
			return main_view->m_auto_save_form_data_check;
		}
	} else if (type == BR_PRIVACY_MENU_REMEMBER_PASSWORDS) {
		if (!strcmp(part, "elm.icon")) {
			main_view->m_auto_save_id_pass_check = elm_check_add(obj);
			if (main_view->m_auto_save_id_pass_check) {
				elm_object_style_set(main_view->m_auto_save_id_pass_check, "on&off");
				evas_object_smart_callback_add(main_view->m_auto_save_id_pass_check, "changed",
						__auto_save_id_pass_check_changed_cb, main_view->m_auto_save_id_pass_check);

				Eina_Bool auto_save = m_preference->get_auto_save_id_password_enabled();
				elm_check_state_set(main_view->m_auto_save_id_pass_check, auto_save);
				evas_object_propagate_events_set(main_view->m_auto_save_id_pass_check, EINA_FALSE);
			}
			return main_view->m_auto_save_id_pass_check;
		}
	} else if (type == BR_PRIVACY_MENU_ACCEPT_COOKIES) {
		if (!strcmp(part, "elm.icon")) {
			main_view->m_accept_cookies_check = elm_check_add(obj);
			if (main_view->m_accept_cookies_check) {
				elm_object_style_set(main_view->m_accept_cookies_check, "on&off");
				evas_object_smart_callback_add(main_view->m_accept_cookies_check, "changed",
						__accept_cookies_check_changed_cb, main_view->m_accept_cookies_check);

				Eina_Bool accept_cookies = m_preference->get_accept_cookies_enabled();
				elm_check_state_set(main_view->m_accept_cookies_check, accept_cookies);
				evas_object_propagate_events_set(main_view->m_accept_cookies_check, EINA_FALSE);
			}
			return main_view->m_accept_cookies_check;
		}
	} else if (type == BR_PRIVACY_MENU_ENABLE_LOCATION) {
		if (!strcmp(part, "elm.icon")) {
			main_view->m_enable_location_check = elm_check_add(obj);
			if (main_view->m_enable_location_check) {
				elm_object_style_set(main_view->m_enable_location_check, "on&off");
				evas_object_smart_callback_add(main_view->m_enable_location_check, "changed",
						__enable_location_check_changed_cb, main_view->m_enable_location_check);

				Eina_Bool enable_location = m_preference->get_location_enabled();
				elm_check_state_set(main_view->m_enable_location_check, enable_location);
				evas_object_propagate_events_set(main_view->m_enable_location_check, EINA_FALSE);
			}
			return main_view->m_enable_location_check;
		}
	}

	return NULL;
}

void setting_view::__naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	setting_view *sv = (setting_view *)data;
	if (sv->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		return;
	sv->_delete_homepage_edit_view();
	sv->_delete_website_setting_view();
	sv->_delete_clear_private_data_view();
	sv->_delete_text_encoding_type_view();
	sv->_delete_homepage_edit_view();
	sv->_delete_user_agent_view();
#if defined(TEST_CODE)
	sv->_delete_custom_user_agent_view();
#endif
}

Evas_Object *setting_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	elm_object_style_set(genlist, "dialogue");

	m_radio_text_item_class = elm_genlist_item_class_new();
	memset(m_radio_text_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));

	m_radio_text_item_class->item_style = "dialogue/1text.1icon/expandable2";
	m_radio_text_item_class->func.text_get = __text_get_cb;
	m_radio_text_item_class->func.content_get = __content_get_cb;
	m_radio_text_item_class->func.state_get = NULL;
	m_radio_text_item_class->func.del = NULL;

	m_category_title_item_class = elm_genlist_item_class_new();
	memset(m_category_title_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_category_title_item_class->item_style = "dialogue/title";
	m_category_title_item_class->func.text_get = __text_get_cb;
	m_category_title_item_class->func.content_get = NULL;
	m_category_title_item_class->func.state_get = NULL;
	m_category_title_item_class->func.del = NULL;

	/* Homepage */
	m_homepage_title_callback_data.type = BR_HOMEPAGE_TITLE;
	m_homepage_title_callback_data.user_data = this;
	m_homepage_title_callback_data.it = elm_genlist_item_append(genlist, m_category_title_item_class,
						&m_homepage_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_homepage_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_2_text_item_class = elm_genlist_item_class_new();
	memset(m_2_text_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_2_text_item_class->item_style = "dialogue/2text.3/expandable";
	m_2_text_item_class->func.text_get = __text_get_cb;
	m_2_text_item_class->func.content_get = NULL;
	m_2_text_item_class->func.state_get = NULL;
	m_2_text_item_class->func.del = NULL;

	m_homepage_item_callback_data.type = BR_HOMEPAGE_MENU;
	m_homepage_item_callback_data.user_data = this;
	m_homepage_item_callback_data.it = elm_genlist_item_append(genlist, m_2_text_item_class,
							&m_homepage_item_callback_data, NULL, ELM_GENLIST_ITEM_TREE,
							__expandable_icon_clicked_cb, &m_homepage_item_callback_data);

	/* Content - Run JavaScript / Display Images */
	m_content_title_callback_data.type = BR_CONTENT_TITLE;
	m_content_title_callback_data.user_data = this;
	m_content_title_callback_data.it = elm_genlist_item_append(genlist, m_category_title_item_class,
						&m_content_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_content_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_defailt_view_level_item_callback_data.type = BR_CONTENT_MENU_DEFAULT_VIEW_LEVEL;
	m_defailt_view_level_item_callback_data.user_data = this;
	m_defailt_view_level_item_callback_data.it = elm_genlist_item_append(genlist, m_2_text_item_class,
						&m_defailt_view_level_item_callback_data, NULL, ELM_GENLIST_ITEM_TREE,
						__expandable_icon_clicked_cb, &m_defailt_view_level_item_callback_data);

	m_1_text_1_icon_item_class = elm_genlist_item_class_new();
	memset(m_1_text_1_icon_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_1_text_1_icon_item_class->item_style = "dialogue/1text.1icon";
	m_1_text_1_icon_item_class->func.text_get = __text_get_cb;
	m_1_text_1_icon_item_class->func.content_get = __content_get_cb;
	m_1_text_1_icon_item_class->func.state_get = NULL;
	m_1_text_1_icon_item_class->func.del = NULL;

	m_run_javascript_item_callback_data.type = BR_CONTENT_MENU_RUN_JAVASCRIPT;
	m_run_javascript_item_callback_data.user_data = this;
	m_run_javascript_item_callback_data.it = elm_genlist_item_append(genlist, m_1_text_1_icon_item_class,
						&m_run_javascript_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_run_javascript_item_callback_data);
#if defined(BROWSER_MDM_PHASE_2) && defined(BROWSER_MDM_PHASE_1)
#if 0
	if (get_browser()->m_mdm_java_script_flag == MDM_RESTRICTED)
		elm_object_item_disabled_set(m_run_javascript_item_callback_data.it, EINA_TRUE);
	#endif
#endif

	m_display_images_item_callback_data.type = BR_CONTENT_MENU_DISPLAY_IMAGES;
	m_display_images_item_callback_data.user_data = this;
	m_display_images_item_callback_data.it = elm_genlist_item_append(genlist, m_1_text_1_icon_item_class,
						&m_display_images_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_display_images_item_callback_data);

	m_block_popup_item_callback_data.type = BR_CONTENT_MENU_BLOCK_POPUP;
	m_block_popup_item_callback_data.user_data = this;
	m_block_popup_item_callback_data.it = elm_genlist_item_append(genlist, m_1_text_1_icon_item_class,
						&m_block_popup_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_block_popup_item_callback_data);

	m_2_text_3_item_class = elm_genlist_item_class_new();
	memset(m_2_text_3_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_2_text_3_item_class->item_style = "dialogue/2text.3";
	m_2_text_3_item_class->func.text_get = __text_get_cb;
	m_2_text_3_item_class->func.content_get = __content_get_cb;
	m_2_text_3_item_class->func.state_get = NULL;
	m_2_text_3_item_class->func.del = NULL;

	m_encoding_type_callback_data.type = BR_CONTENT_MENU_ENCODING_TYPE;
	m_encoding_type_callback_data.user_data = this;
	m_encoding_type_callback_data.it = elm_genlist_item_append(genlist, m_2_text_3_item_class,
							&m_encoding_type_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_encoding_type_callback_data);
#if defined(BROWSER_MDM_PHASE_2) && defined(BROWSER_MDM_PHASE_1)
#if 0
	if (get_browser()->m_mdm_popup_flag == MDM_RESTRICTED)
		elm_object_item_disabled_set(m_block_popup_item_callback_data.it, EINA_TRUE);
#endif
#endif

	/* Privacy - Accept cookies / Auto save ID, password / Clear private data */
	m_privacy_title_callback_data.type = BR_PRIVACY_TITLE;
	m_privacy_title_callback_data.user_data = this;
	m_privacy_title_callback_data.it = elm_genlist_item_append(genlist, m_category_title_item_class,
						&m_privacy_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_privacy_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_clear_cache_item_callback_data.type = BR_PRIVATE_MENU_CLEAR_CACHE;
	m_clear_cache_item_callback_data.user_data = this;
	m_clear_cache_item_callback_data.it = elm_genlist_item_append(genlist, m_1_text_1_icon_item_class,
						&m_clear_cache_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_clicked_cb, &m_clear_cache_item_callback_data);

	m_clear_history_item_callback_data.type = BR_PRIVATE_MENU_CLEAR_HISTORY;
	m_clear_history_item_callback_data.user_data = this;
	m_clear_history_item_callback_data.it = elm_genlist_item_append(genlist, m_1_text_1_icon_item_class,
						&m_clear_history_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_clicked_cb, &m_clear_history_item_callback_data);

	m_clear_private_data.type = BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA;
	m_clear_private_data.user_data = this;
	m_clear_private_data.it = elm_genlist_item_append(genlist, m_1_text_1_icon_item_class,
						&m_clear_private_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_clicked_cb, &m_clear_private_data);

	m_show_security_warnings_item_callback_data.type = BR_PRIVATE_MENU_SHOW_SECURITY_WARNINGS;
	m_show_security_warnings_item_callback_data.user_data = this;
	m_show_security_warnings_item_callback_data.it = elm_genlist_item_append(genlist, m_1_text_1_icon_item_class,
						&m_show_security_warnings_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_show_security_warnings_item_callback_data);
#if defined(BROWSER_MDM_PHASE_2) && defined(BROWSER_MDM_PHASE_1)
#if 0
	if (get_browser()->m_mdm_fraud_warning_flag == MDM_RESTRICTED)
		elm_object_item_disabled_set(m_show_security_warnings_item_callback_data.it, EINA_TRUE);
#endif
#endif
	m_2_text_1_icon_item_class = elm_genlist_item_class_new();
	memset(m_2_text_1_icon_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_2_text_1_icon_item_class->item_style = "dialogue/2text.1icon.5";
	m_2_text_1_icon_item_class->func.text_get = __text_get_cb;
	m_2_text_1_icon_item_class->func.content_get = __content_get_cb;
	m_2_text_1_icon_item_class->func.state_get = NULL;
	m_2_text_1_icon_item_class->func.del = NULL;

	m_accept_cookies_item_callback_data.type = BR_PRIVACY_MENU_ACCEPT_COOKIES;
	m_accept_cookies_item_callback_data.user_data = this;
	m_accept_cookies_item_callback_data.it = elm_genlist_item_append(genlist, m_2_text_1_icon_item_class,
						&m_accept_cookies_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_accept_cookies_item_callback_data);

	m_2_text_2_item_class = elm_genlist_item_class_new();
	memset(m_2_text_2_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_2_text_2_item_class->item_style = "dialogue/2text.2";
	m_2_text_2_item_class->func.text_get = __text_get_cb;
	m_2_text_2_item_class->func.content_get = __content_get_cb;
	m_2_text_2_item_class->func.state_get = NULL;
	m_2_text_2_item_class->func.del = NULL;

	m_clear_all_cookies_data_item_callback_data.type = BR_PRIVATE_MENU_CLEAR_ALL_COOKIE_DATA;
	m_clear_all_cookies_data_item_callback_data.user_data = this;
	m_clear_all_cookies_data_item_callback_data.it = elm_genlist_item_append(genlist, m_2_text_2_item_class,
						&m_clear_all_cookies_data_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__genlist_item_clicked_cb, &m_clear_all_cookies_data_item_callback_data);
#if defined(BROWSER_MDM_PHASE_2) && defined(BROWSER_MDM_PHASE_1)
#if 0
	if (get_browser()->m_mdm_cookies_flag == MDM_RESTRICTED) {
		elm_object_item_disabled_set(m_accept_cookies_item_callback_data.it, EINA_TRUE);
		elm_object_item_disabled_set(m_clear_all_cookies_data_item_callback_data.it, EINA_TRUE);
	}
#endif
#endif
	m_1_text_item_class = elm_genlist_item_class_new();
	memset(m_1_text_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));
	m_1_text_item_class->item_style = "dialogue/1text";
	m_1_text_item_class->func.text_get = __text_get_cb;
	m_1_text_item_class->func.content_get = NULL;
	m_1_text_item_class->func.state_get = NULL;
	m_1_text_item_class->func.del = NULL;

	m_auto_save_form_data_callback_data.type = BR_PRIVACY_MENU_REMEMBER_FORM_DATA;
	m_auto_save_form_data_callback_data.user_data = this;
	m_auto_save_form_data_callback_data.it = elm_genlist_item_append(genlist, m_2_text_1_icon_item_class,
							&m_auto_save_form_data_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__on_off_check_clicked_cb, &m_auto_save_form_data_callback_data);

	m_clear_form_data_callback_data.type = BR_PRIVACY_CLEAR_FORM_DATA;
	m_clear_form_data_callback_data.user_data = this;
	m_clear_form_data_callback_data.it = elm_genlist_item_append(genlist, m_2_text_2_item_class,
							&m_clear_form_data_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_clear_form_data_callback_data);
#if defined(BROWSER_MDM_PHASE_2) && defined(BROWSER_MDM_PHASE_1)
#if 0
	if (get_browser()->m_mdm_autofill_flag == MDM_RESTRICTED) {
		elm_object_item_disabled_set(m_auto_save_form_data_callback_data.it, EINA_TRUE);
		elm_object_item_disabled_set(m_clear_form_data_callback_data.it, EINA_TRUE);
	}
#endif
#endif
	m_enable_location_callback_data.type = BR_PRIVACY_MENU_ENABLE_LOCATION;
	m_enable_location_callback_data.user_data = this;
	m_enable_location_callback_data.it = elm_genlist_item_append(genlist, m_2_text_1_icon_item_class,
						&m_enable_location_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_enable_location_callback_data);

	m_clear_location_access_callback_data.type = BR_PRIVACY_MENU_CLEAR_LOCATION_ACCESS;
	m_clear_location_access_callback_data.user_data = this;
	m_clear_location_access_callback_data.it = elm_genlist_item_append(genlist, m_2_text_2_item_class,
							&m_clear_location_access_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_clear_location_access_callback_data);

	Eina_Bool enable_location = m_preference->get_location_enabled();
	if (enable_location== EINA_FALSE)
		elm_object_item_disabled_set(m_clear_location_access_callback_data.it, EINA_TRUE);

	m_auto_save_item_callback_data.type = BR_PRIVACY_MENU_REMEMBER_PASSWORDS;
	m_auto_save_item_callback_data.user_data = this;
	m_auto_save_item_callback_data.it = elm_genlist_item_append(genlist, m_2_text_1_icon_item_class,
							&m_auto_save_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__on_off_check_clicked_cb, &m_auto_save_item_callback_data);

	m_clear_passwords_callback_data.type = BR_PRIVACY_CLEAR_PASSWORDS;
	m_clear_passwords_callback_data.user_data = this;
	m_clear_passwords_callback_data.it = elm_genlist_item_append(genlist, m_2_text_2_item_class,
							&m_clear_passwords_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_clear_passwords_callback_data);
#if defined(BROWSER_MDM_PHASE_2) && defined(BROWSER_MDM_PHASE_1)
#if 0
	if (get_browser()->m_mdm_autofill_flag== MDM_RESTRICTED) {
		elm_object_item_disabled_set(m_auto_save_item_callback_data.it, EINA_TRUE);
		elm_object_item_disabled_set(m_clear_passwords_callback_data.it, EINA_TRUE);
	}
#endif
#endif

	m_seperator_item_class = elm_genlist_item_class_new();
	memset(m_seperator_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));

	/* Others */
	m_seperator_item_class->item_style = "grouptitle.dialogue.seperator";
	m_seperator_item_class->func.text_get = NULL;
	m_seperator_item_class->func.content_get = NULL;
	m_seperator_item_class->func.state_get = NULL;
	m_seperator_item_class->func.del = NULL;
	Elm_Object_Item *it = elm_genlist_item_append(genlist, m_seperator_item_class,
						NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_website_setting_callback_data.type = BR_PRIVACY_WEBSITE_SETTING;
	m_website_setting_callback_data.user_data = this;
	m_website_setting_callback_data.it = elm_genlist_item_append(genlist, m_1_text_1_icon_item_class,
							&m_website_setting_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_website_setting_callback_data);

	m_reset_item_callback_data.type = BR_MENU_RESET_TO_DEFAULT;
	m_reset_item_callback_data.user_data = this;
	m_reset_item_callback_data.it = elm_genlist_item_append(genlist, m_1_text_item_class,
							&m_reset_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_reset_item_callback_data);

	/* Debug */
	m_debug_title_callback_data.type = BR_DEBUG_TITLE;
	m_debug_title_callback_data.user_data = this;
	m_debug_title_callback_data.it = elm_genlist_item_append(genlist, m_category_title_item_class,
						&m_debug_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_debug_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_user_agent_item_callback_data.type = BR_MENU_USER_AGENT;
	m_user_agent_item_callback_data.user_data = this;
	m_user_agent_item_callback_data.it = elm_genlist_item_append(genlist, m_1_text_item_class,
							&m_user_agent_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_user_agent_item_callback_data);
#if defined(TEST_CODE)
	m_custom_user_agent_item_callback_data.type = BR_MENU_USER_CUSTOM_USER_AGENT;
	m_custom_user_agent_item_callback_data.user_data = this;
	m_custom_user_agent_item_callback_data.it = elm_genlist_item_append(genlist, m_1_text_item_class,
							&m_custom_user_agent_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_custom_user_agent_item_callback_data);
#endif

	m_seperator_with_bottom_line_item_class = elm_genlist_item_class_new();
	memset(m_seperator_with_bottom_line_item_class, 0x00, sizeof(Elm_Genlist_Item_Class));

	m_seperator_with_bottom_line_item_class->item_style = "dialogue/seperator.2";
	m_seperator_with_bottom_line_item_class->func.text_get = NULL;
	m_seperator_with_bottom_line_item_class->func.content_get = NULL;
	m_seperator_with_bottom_line_item_class->func.state_get = NULL;
	m_seperator_with_bottom_line_item_class->func.del = NULL;

	elm_genlist_item_append(genlist, m_seperator_with_bottom_line_item_class, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_NONE);

	return genlist;
}



void setting_view::__expandable_icon_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	setting_view *main_view = (setting_view *)(callback_data->user_data);
	Elm_Object_Item *it = callback_data->it;

	Evas_Object *genlist = elm_object_item_widget_get(it);
	Eina_Bool is_expanded = elm_genlist_item_expanded_get(it);
	if (is_expanded) {
		elm_genlist_item_expanded_set(it, EINA_FALSE);
		elm_genlist_item_subitems_clear(it);

		main_view->m_user_homepage_item_callback_data.it = NULL;
	} else {
		elm_genlist_item_expanded_set(it, EINA_TRUE);
		if (it == main_view->m_homepage_item_callback_data.it) {
			homepage_type homepage = m_preference->get_homepage_type();
			BROWSER_LOGD("__expandable_icon_clicked_cb - homepage[%d]", homepage);
			/* If homepage menu. */
			main_view->m_current_page_item_callback_data.type = BR_HOMEPAGE_SUBMENU_CURRENT_PAGE;
			main_view->m_current_page_item_callback_data.user_data = main_view;
			main_view->m_current_page_item_callback_data.it = elm_genlist_item_append(genlist, (main_view->m_radio_text_item_class),
										&(main_view->m_current_page_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_current_page_item_callback_data));

			main_view->m_empty_page_item_callback_data.type = BR_HOMEPAGE_SUBMENU_EMPTY_PAGE;
			main_view->m_empty_page_item_callback_data.user_data = main_view;
			main_view->m_empty_page_item_callback_data.it = elm_genlist_item_append(genlist, main_view->m_radio_text_item_class,
										&(main_view->m_empty_page_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_empty_page_item_callback_data));

			main_view->m_recently_visited_item_callback_data.type = BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE;
			main_view->m_recently_visited_item_callback_data.user_data = main_view;
			main_view->m_recently_visited_item_callback_data.it = elm_genlist_item_append(genlist, main_view->m_radio_text_item_class,
										&(main_view->m_recently_visited_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_recently_visited_item_callback_data));

			main_view->m_user_homepage_item_callback_data.type = BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE;
			main_view->m_user_homepage_item_callback_data.user_data = main_view;
			main_view->m_user_homepage_item_callback_data.it = elm_genlist_item_append(genlist, main_view->m_radio_text_item_class,
										&(main_view->m_user_homepage_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_user_homepage_item_callback_data));
			const char *current_page_uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
			if (!(current_page_uri && strlen(current_page_uri)))
				elm_object_item_disabled_set(main_view->m_current_page_item_callback_data.it, EINA_TRUE);

		} else if (it == main_view->m_defailt_view_level_item_callback_data.it) {
			BROWSER_LOGD("__expandable_icon_clicked_cb - default view");
			main_view->m_fit_to_width_item_callback_data.type = BR_CONTENT_SUBMENU_FIT_TO_WIDTH;
			main_view->m_fit_to_width_item_callback_data.user_data = main_view;
			main_view->m_fit_to_width_item_callback_data.it = elm_genlist_item_append(genlist, main_view->m_radio_text_item_class,
										&(main_view->m_fit_to_width_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __default_view_level_sub_item_clicked_cb,
										&(main_view->m_fit_to_width_item_callback_data));

			main_view->m_readable_item_callback_data.type = BR_CONTENT_SUBMENU_READABLE;
			main_view->m_readable_item_callback_data.user_data = main_view;
			main_view->m_readable_item_callback_data.it = elm_genlist_item_append(genlist, main_view->m_radio_text_item_class,
										&(main_view->m_readable_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __default_view_level_sub_item_clicked_cb,
										&(main_view->m_readable_item_callback_data));
		}
	}
	elm_genlist_item_update(it);
	elm_genlist_item_selected_set(it, EINA_FALSE);
}

void setting_view::__homepage_sub_item_clicked_cb(void *data, Evas_Object *obj,
											void *event_info)
{
	BROWSER_LOGD("");

	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	setting_view::menu_type type = callback_data->type;
	setting_view *main_view = (setting_view *)(callback_data->user_data);

	int radio_value = 0;

	BROWSER_LOGD("type[%d]", type);

	if (type == BR_HOMEPAGE_SUBMENU_CURRENT_PAGE)
		radio_value = HOMEPAGE_TYPE_CURRENT_PAGE;
	else if (type == BR_HOMEPAGE_SUBMENU_EMPTY_PAGE)
		radio_value = HOMEPAGE_TYPE_EMPTY_PAGE;
	else if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE)
		radio_value = HOMEPAGE_TYPE_RECENTLY_VISITED_SITE;
	else if (type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE)
		radio_value = HOMEPAGE_TYPE_USER_HOMEPAGE;

	elm_radio_value_set(main_view->m_homepage_radio_group, radio_value);

	if (radio_value == HOMEPAGE_TYPE_CURRENT_PAGE) {
		elm_radio_value_set(main_view->m_homepage_radio_group, HOMEPAGE_TYPE_USER_HOMEPAGE);
		const char *homepage = m_browser->get_browser_view()->get_current_webview()->get_uri();
		m_preference->set_user_homagepage((const char *)homepage);
		m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);
		elm_genlist_item_update(main_view->m_user_homepage_item_callback_data.it);
	}
	else if (radio_value == HOMEPAGE_TYPE_EMPTY_PAGE)
		m_preference->set_homepage_type(HOMEPAGE_TYPE_EMPTY_PAGE);
	else if (radio_value == HOMEPAGE_TYPE_RECENTLY_VISITED_SITE)
		m_preference->set_homepage_type(HOMEPAGE_TYPE_RECENTLY_VISITED_SITE);
	else if (radio_value == HOMEPAGE_TYPE_USER_HOMEPAGE) {
		m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);
		main_view->get_homepage_edit_view()->show();
	}
	elm_genlist_item_update(main_view->m_homepage_item_callback_data.it);
	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void setting_view::__default_view_level_sub_item_clicked_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	setting_view::menu_type type = callback_data->type;
	setting_view *main_view = (setting_view *)(callback_data->user_data);
	BROWSER_LOGD("type[%d]", type);
	int radio_value = 0;
	if (type == BR_CONTENT_SUBMENU_FIT_TO_WIDTH)
		radio_value = 0;
	else if (type == BR_CONTENT_SUBMENU_READABLE)
		radio_value = 1;
	BROWSER_LOGD("elm_radio_value_get[%d]", elm_radio_value_get(main_view->m_default_view_level_radio_group));
	if (elm_radio_value_get(main_view->m_default_view_level_radio_group) != radio_value) {
		elm_radio_value_set(main_view->m_default_view_level_radio_group, radio_value);
		if (radio_value == 0)
			m_preference->set_view_level_type(VIEW_LEVEL_TYPE_FIT_TO_WIDTH);
		else
			m_preference->set_view_level_type(VIEW_LEVEL_TYPE_READABLE);

		elm_genlist_item_update(main_view->m_defailt_view_level_item_callback_data.it);
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void setting_view::__on_off_check_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	setting_view::menu_type type = callback_data->type;
	setting_view *main_view = (setting_view *)(callback_data->user_data);

	Eina_Bool state = EINA_TRUE;
	if (type == BR_CONTENT_MENU_RUN_JAVASCRIPT) {
		state = elm_check_state_get(main_view->m_run_javascript_check);
		elm_check_state_set(main_view->m_run_javascript_check, !state);
		__run_javascript_check_changed_cb(main_view->m_run_javascript_check, NULL, NULL);
	} else if (type == BR_CONTENT_MENU_DISPLAY_IMAGES) {
		state = elm_check_state_get(main_view->m_display_images_check);
		elm_check_state_set(main_view->m_display_images_check, !state);
		__show_images_check_changed_cb(main_view->m_display_images_check, NULL, NULL);
	} else if (type == BR_CONTENT_MENU_BLOCK_POPUP) {
		state = elm_check_state_get(main_view->m_block_popup_check);
		elm_check_state_set(main_view->m_block_popup_check, !state);
		__block_popup_check_changed_cb(main_view->m_block_popup_check, NULL, NULL);
	}
	else if (type == BR_PRIVATE_MENU_SHOW_SECURITY_WARNINGS) {
		state = elm_check_state_get(main_view->m_show_security_warnings_check);
		elm_check_state_set(main_view->m_show_security_warnings_check, !state);
		__show_security_warnings_check_changed_cb(main_view->m_show_security_warnings_check, NULL, NULL);
	} else if (type == BR_PRIVACY_MENU_ACCEPT_COOKIES) {
		state = elm_check_state_get(main_view->m_accept_cookies_check);
		elm_check_state_set(main_view->m_accept_cookies_check, !state);
		__accept_cookies_check_changed_cb(main_view->m_accept_cookies_check, NULL, NULL);
	} else if (type == BR_PRIVACY_MENU_REMEMBER_FORM_DATA) {
		state = elm_check_state_get(main_view->m_auto_save_form_data_check);
		elm_check_state_set(main_view->m_auto_save_form_data_check, !state);
		__auto_save_form_data_check_changed_cb(main_view->m_auto_save_form_data_check, NULL, NULL);
	} else if (type == BR_PRIVACY_MENU_ENABLE_LOCATION) {
		state = elm_check_state_get(main_view->m_enable_location_check);
		elm_check_state_set(main_view->m_enable_location_check, !state);
		__enable_location_check_changed_cb(main_view->m_enable_location_check, NULL, NULL);
	} else if (type == BR_PRIVACY_MENU_REMEMBER_PASSWORDS) {
		state = elm_check_state_get(main_view->m_auto_save_id_pass_check);
		elm_check_state_set(main_view->m_auto_save_id_pass_check, !state);
		__auto_save_id_pass_check_changed_cb(main_view->m_auto_save_id_pass_check, NULL, NULL);
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void setting_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	setting_view::menu_type type = callback_data->type;
	setting_view *main_view = (setting_view *)(callback_data->user_data);

	if (type == BR_PRIVATE_MENU_CLEAR_CACHE) {
		m_browser->get_browser_view()->show_msg_popup(NULL,
													BR_STRING_CLEAR_ALL_CACHE_DATA_Q,
													BR_STRING_YES,
													__clear_cache_confirm_response_cb,
													BR_STRING_NO,
													__cancel_response_cb,
													main_view);
	} else if (type == BR_PRIVATE_MENU_CLEAR_HISTORY) {
		m_browser->get_browser_view()->show_msg_popup(NULL,
													BR_STRING_CLEAR_ALL_HISTORY_DATA_Q,
													BR_STRING_YES,
													__clear_history_confirm_response_cb,
													BR_STRING_NO,
													__cancel_response_cb,
													main_view);
	} else if (type == BR_PRIVATE_MENU_CLEAR_ALL_COOKIE_DATA) {
		m_browser->get_browser_view()->show_msg_popup(NULL,
													BR_STRING_CLEAR_ALL_COOKIE_DATA_Q,
													BR_STRING_YES,
													__clear_all_cookie_data_confirm_response_cb,
													BR_STRING_NO,
													__cancel_response_cb,
													main_view);
	} else if (type == BR_PRIVACY_CLEAR_FORM_DATA) {
		m_browser->get_browser_view()->show_msg_popup(NULL,
													BR_STRING_CLEAR_ALL_FORMDATA_Q,
													BR_STRING_YES,
													__clear_form_data_confirm_response_cb,
													BR_STRING_NO,
													__cancel_response_cb,
													main_view);
	} else if (type == BR_PRIVACY_CLEAR_PASSWORDS) {
		m_browser->get_browser_view()->show_msg_popup(NULL,
													BR_STRING_CLEAR_ALL_SAVED_PASSWORDS_Q,
													BR_STRING_YES,
													__clear_passwords_confirm_response_cb,
													BR_STRING_NO,
													__cancel_response_cb,
													main_view);
	} else if (type == BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA) {
		if (main_view->get_clear_private_data_view()->show() != EINA_TRUE)
			BROWSER_LOGE("_call_user_agent failed");
	} else if (type == BR_PRIVACY_MENU_CLEAR_LOCATION_ACCESS) {
		m_browser->get_browser_view()->show_msg_popup(NULL,
													BR_STRING_MSG_DELETE_WEBSITE_LOCATION_ACCESS_INFORMATION_Q,
													BR_STRING_YES,
													__clear_location_confirm_response_cb,
													BR_STRING_NO,
													__cancel_response_cb,
													main_view);
	} else if (type == BR_MENU_RESET_TO_DEFAULT) {
		m_browser->get_browser_view()->show_msg_popup(NULL,
													BR_STRING_RESET_TO_DEFAULT_Q,
													BR_STRING_YES,
													__reset_confirm_response_cb,
													BR_STRING_NO,
													__cancel_response_cb,
													main_view);
	} else if (type == BR_PRIVACY_WEBSITE_SETTING) {
		main_view->get_website_setting_view()->show();
	} else if (type == BR_CONTENT_MENU_ENCODING_TYPE) {
		main_view->get_text_encoding_type_view()->show();
	} else if (type == BR_MENU_USER_AGENT) {
		main_view->get_user_agent_view()->show();
	}
#if defined(TEST_CODE)
	else if (type == BR_MENU_USER_CUSTOM_USER_AGENT) {
		main_view->get_custom_user_agent_view()->show();
	}
#endif
	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void setting_view::__title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	Elm_Object_Item *naviframe_item = (Elm_Object_Item *)event_info;
	Evas_Object *label = elm_object_item_part_content_get(naviframe_item, "elm.swallow.title");
	EINA_SAFETY_ON_NULL_RETURN(label);

	elm_label_slide_go(label);
}

void setting_view::__run_javascript_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	m_preference->set_javascript_enabled(state);
}

void setting_view::__show_images_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	m_preference->set_display_images_enabled(state);
}

void setting_view::__block_popup_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	m_preference->set_block_popup_enabled(state);
}

void setting_view::__show_security_warnings_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	m_preference->set_security_warnings_enabled(state);
}

void setting_view::__accept_cookies_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	m_preference->set_accept_cookies_enabled(state);
}

void setting_view::__auto_save_form_data_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	m_preference->set_auto_save_form_data_enabled(state);
}

void setting_view::__auto_save_id_pass_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	m_preference->set_auto_save_id_password_enabled(state);
}

void setting_view::__enable_location_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	m_preference->set_location_enabled(state);
}

void setting_view::__clear_cache_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	m_webview_context->clear_private_data();
}

void setting_view::__clear_history_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	setting_view *main_view = (setting_view *)data;
	m_browser->get_history()->delete_all();
}

void setting_view::__clear_all_cookie_data_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_webview_context->clear_cookies();
}

void setting_view::__clear_form_data_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_webview_context->clear_form_data();
}

void setting_view::__clear_passwords_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_webview_context->clear_saved_ID_and_PW();
}

void setting_view::__clear_location_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	setting_view *main_view = (setting_view *)data;
	m_browser->get_geolocation_manager()->delete_all();
}

void setting_view::__reset_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	setting_view *main_view = (setting_view *)data;
	main_view->_reset_to_default_setting();
}

void setting_view::__cancel_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	/* Do something if action is needed from cancel button */
}

void setting_view::__ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(priv);
	EINA_SAFETY_ON_NULL_RETURN(ug);

	setting_view *main_view = (setting_view *)priv;
	Evas_Object *base = (Evas_Object*)ug_get_layout(ug);
	if (!base)
		return;

	Evas_Object *win = (Evas_Object *)ug_get_window();

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(win, base);
		evas_object_show(base);
		break;
	default:
		break;
	}
}

void setting_view::__ug_destroy_cb(ui_gadget_h ug, void *priv)
{
	EINA_SAFETY_ON_NULL_RETURN(priv);
	EINA_SAFETY_ON_NULL_RETURN(ug);

	if (ug_destroy(ug))
		BROWSER_LOGD("ug_destroy is failed.\n");
}
