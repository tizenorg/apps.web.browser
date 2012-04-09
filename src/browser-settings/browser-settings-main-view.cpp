/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#include "browser-view.h"
#include "browser-data-manager.h"
#include "browser-settings-clear-data-view.h"
#include "browser-settings-edit-homepage-view.h"
#include "browser-settings-main-view.h"
#include "browser-settings-plugin-view.h"

#include "browser-settings-accelerated-composition.h"
#include "browser-settings-user-agent-view.h"

Browser_Settings_Main_View::Browser_Settings_Main_View(void)
:
	m_genlist(NULL)
	,m_back_button(NULL)
	,m_homepage_radio_group(NULL)
	,m_edit_homepage_view(NULL)
	,m_default_view_level_radio_group(NULL)
	,m_auto_save_id_pass_radio_group(NULL)
	,m_run_javascript_check(NULL)
	,m_display_images_check(NULL)
	,m_accept_cookies_check(NULL)
	,m_clear_data_view(NULL)
	,m_default_storage_radio_group(NULL)
	,m_default_storage_mmc_radio_button(NULL)
	,m_plugin_view(NULL)
	,m_block_popup_check(NULL)
	,m_user_agent_view(NULL)
	,m_reset_confirm_popup(NULL)
	,m_navi_it(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_Main_View::~Browser_Settings_Main_View(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (vconf_ignore_key_changed(VCONFKEY_SYSMAN_MMC_STATUS, __mmc_key_changed_cb) < 0)
		BROWSER_LOGE("[%s]vconf_ignore_key_changed failed", VCONFKEY_SYSMAN_MMC_STATUS);

	if (m_edit_homepage_view) {
		delete m_edit_homepage_view;
		m_edit_homepage_view = NULL;
	}
	if (m_clear_data_view) {
		delete m_clear_data_view;
		m_clear_data_view = NULL;
	}
	if (m_plugin_view) {
		delete m_plugin_view;
		m_plugin_view = NULL;
	}
	if (m_user_agent_view) {
		delete m_user_agent_view;
		m_user_agent_view = NULL;
	}
	evas_object_smart_callback_del(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb);
}

Eina_Bool Browser_Settings_Main_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	return _create_main_layout();
}

void Browser_Settings_Main_View::__back_button_clicked_cb(void *data, Evas_Object* obj,
									void* event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (elm_naviframe_bottom_item_get(m_navi_bar) != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);

	m_data_manager->get_browser_view()->return_to_browser_view();
}

void Browser_Settings_Main_View::__naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	if (main_view->m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;

	if (main_view->m_edit_homepage_view) {
		delete main_view->m_edit_homepage_view;
		main_view->m_edit_homepage_view = NULL;

		/* If back from edit user homepage, update the url string. */
		char* homepage = vconf_get_str(HOMEPAGE_KEY);
		if (homepage) {
			if (!strncmp(homepage, USER_HOMEPAGE,
						strlen(USER_HOMEPAGE)))
				elm_radio_value_set(main_view->m_homepage_radio_group, 2);

			free(homepage);

			elm_genlist_realized_items_update(main_view->m_genlist);
		}
	}

	if (main_view->m_clear_data_view) {
		delete main_view->m_clear_data_view;
		main_view->m_clear_data_view = NULL;
	}
	if (main_view->m_plugin_view) {
		delete main_view->m_plugin_view;
		main_view->m_plugin_view = NULL;
	}
	if (main_view->m_user_agent_view) {
		delete main_view->m_user_agent_view;
		main_view->m_user_agent_view = NULL;
	}
}

Eina_Bool Browser_Settings_Main_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_genlist = _create_content_genlist();
	if (!m_genlist) {
		BROWSER_LOGE("_create_content_genlist failed");
		return EINA_FALSE;
	}

	evas_object_show(m_genlist);

	m_back_button = elm_button_add(m_genlist);
	if (!m_back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_back_button, "browser/bookmark_controlbar_back");
	evas_object_show(m_back_button);
	evas_object_smart_callback_add(m_back_button, "clicked", __back_button_clicked_cb, this);

	m_homepage_radio_group = elm_radio_add(m_genlist);
	if (!m_homepage_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		return EINA_FALSE;
	}
	elm_radio_state_value_set(m_homepage_radio_group, -1);

	m_default_view_level_radio_group = elm_radio_add(m_genlist);
	if (!m_default_view_level_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		return EINA_FALSE;
	}
	elm_radio_state_value_set(m_default_view_level_radio_group, -1);

	m_auto_save_id_pass_radio_group = elm_radio_add(m_genlist);
	if (!m_auto_save_id_pass_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		return EINA_FALSE;
	}
	elm_radio_state_value_set(m_auto_save_id_pass_radio_group, -1);

	m_default_storage_radio_group = elm_radio_add(m_genlist);
	if (!m_default_storage_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		return EINA_FALSE;
	}
	elm_radio_state_value_set(m_default_storage_radio_group, -1);

	m_navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_INTERNET,
							m_back_button, NULL, m_genlist, "browser_titlebar");

	evas_object_smart_callback_add(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb, this);

	return EINA_TRUE;
}

Evas_Object *Browser_Settings_Main_View::__genlist_icon_get(void *data, Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	if (type == BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES
	    || type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE
	    || type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			Evas_Object *radio_button = elm_radio_add(obj);
			if (radio_button) {
				if (type == BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES)
					elm_radio_state_value_set(radio_button, 0);
				else if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE)
					elm_radio_state_value_set(radio_button, 1);
				else if (type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE)
					elm_radio_state_value_set(radio_button, 2);

				elm_radio_group_add(radio_button, main_view->m_homepage_radio_group);
				evas_object_propagate_events_set(radio_button, EINA_FALSE);

				evas_object_smart_callback_add(radio_button, "changed",
							__homepage_sub_item_clicked_cb, data);

				char* homepage = vconf_get_str(HOMEPAGE_KEY);
				if (!homepage) {
					vconf_set_str(HOMEPAGE_KEY, MOST_VISITED_SITES);
					homepage = strdup(MOST_VISITED_SITES);
				}

				if (!homepage) {
					BROWSER_LOGE("strdup failed");
					return NULL;
				}

				if (!strncmp(homepage, MOST_VISITED_SITES,
							strlen(MOST_VISITED_SITES)))
					elm_radio_value_set(main_view->m_homepage_radio_group, 0);
				else if (!strncmp(homepage, RECENTLY_VISITED_SITE,
							strlen(RECENTLY_VISITED_SITE)))
					elm_radio_value_set(main_view->m_homepage_radio_group, 1);
				else if (!strncmp(homepage, USER_HOMEPAGE,
							strlen(USER_HOMEPAGE)))
					elm_radio_value_set(main_view->m_homepage_radio_group, 2);
				else
					elm_radio_value_set(main_view->m_homepage_radio_group, 3);

				free(homepage);
			}

			return radio_button;
		}
	} else if (type == BR_DISPLAY_SUBMENU_FIT_TO_WIDTH
	     || type == BR_DISPLAY_SUBMENU_READABLE) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			Evas_Object *radio_button = elm_radio_add(obj);
			if (radio_button) {
				if (type == BR_DISPLAY_SUBMENU_FIT_TO_WIDTH)
					elm_radio_state_value_set(radio_button, 0);
				else
					elm_radio_state_value_set(radio_button, 1);

				elm_radio_group_add(radio_button, main_view->m_default_view_level_radio_group);

				char *view_level = vconf_get_str(DEFAULT_VIEW_LEVEL_KEY);
				if (!view_level) {
					vconf_set_str(DEFAULT_VIEW_LEVEL_KEY, READABLE);
					view_level = strdup(BR_STRING_READABLE);
				}
				if (!view_level) {
					BROWSER_LOGE("strdup failed");
					return NULL;
				}

				if (!strncmp(view_level, FIT_TO_WIDTH, strlen(FIT_TO_WIDTH)))
					elm_radio_value_set(main_view->m_default_view_level_radio_group, 0);
				else
					elm_radio_value_set(main_view->m_default_view_level_radio_group, 1);

				free(view_level);
			}

			return radio_button;
		}
	} else if (type == BR_CONTENT_MENU_RUN_JAVASCRIPT) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			main_view->m_run_javascript_check = elm_check_add(obj);
			if (main_view->m_run_javascript_check) {
				elm_object_style_set(main_view->m_run_javascript_check, "on&off");
				evas_object_smart_callback_add(main_view->m_run_javascript_check, "changed",
						__run_javascript_check_changed_cb, main_view->m_run_javascript_check);

				int run_javascript = 1;
				if (vconf_get_bool(RUN_JAVASCRIPT_KEY, &run_javascript) < 0)
					BROWSER_LOGE("Can not get [%s] value.\n", RUN_JAVASCRIPT_KEY);

				elm_check_state_set(main_view->m_run_javascript_check, run_javascript);
				evas_object_propagate_events_set(main_view->m_run_javascript_check, EINA_FALSE);
			}
			return main_view->m_run_javascript_check;
		}
	} else if (type == BR_CONTENT_MENU_DISPLAY_IMAGES) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			main_view->m_display_images_check = elm_check_add(obj);
			if (main_view->m_display_images_check) {
				elm_object_style_set(main_view->m_display_images_check, "on&off");
				evas_object_smart_callback_add(main_view->m_display_images_check, "changed",
						__display_images_check_changed_cb, main_view->m_display_images_check);

				int display_images = 1;
				if (vconf_get_bool(DISPLAY_IMAGES_KEY, &display_images) < 0)
					BROWSER_LOGE("Can not get [%s] value.\n", DISPLAY_IMAGES_KEY);

				elm_check_state_set(main_view->m_display_images_check, display_images);
				evas_object_propagate_events_set(main_view->m_display_images_check, EINA_FALSE);
			}
			return main_view->m_display_images_check;
		}
	} else if (type == BR_CONTENT_MENU_BLOCK_POPUP) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			main_view->m_block_popup_check = elm_check_add(obj);
			if (main_view->m_block_popup_check) {
				elm_object_style_set(main_view->m_block_popup_check, "on&off");
				evas_object_smart_callback_add(main_view->m_block_popup_check, "changed",
						__block_popup_check_changed_cb, main_view->m_block_popup_check);

				int block_popup = 1;
				if (vconf_get_bool(BLOCK_POPUP_KEY, &block_popup) < 0)
					BROWSER_LOGE("Can not get [%s] value.\n", BLOCK_POPUP_KEY);

				elm_check_state_set(main_view->m_block_popup_check, block_popup);
				evas_object_propagate_events_set(main_view->m_block_popup_check, EINA_FALSE);
			}
			return main_view->m_block_popup_check;
		}
	} else if (type == BR_PRIVACY_MENU_ACCEPT_COOKIES) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			main_view->m_accept_cookies_check = elm_check_add(obj);
			if (main_view->m_accept_cookies_check) {
				elm_object_style_set(main_view->m_accept_cookies_check, "on&off");
				evas_object_smart_callback_add(main_view->m_accept_cookies_check, "changed",
						__accept_cookies_check_changed_cb, main_view->m_accept_cookies_check);

				int accept_cookies = 1;
				if (vconf_get_bool(ACCEPT_COOKIES_KEY, &accept_cookies) < 0)
					BROWSER_LOGE("Can not get [%s] value.\n", ACCEPT_COOKIES_KEY);

				elm_check_state_set(main_view->m_accept_cookies_check, accept_cookies);
				evas_object_propagate_events_set(main_view->m_accept_cookies_check, EINA_FALSE);
			}
			return main_view->m_accept_cookies_check;
		}
	} else if (type == BR_PRIVACY_SUBMENU_ALWAYS_ASK
	    || type == BR_PRIVACY_SUBMENU_ALWAYS_ON
	    || type == BR_PRIVACY_SUBMENU_ALWAYS_OFF) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			Evas_Object *radio_button = elm_radio_add(obj);
			if (radio_button) {
				if (type == BR_PRIVACY_SUBMENU_ALWAYS_ASK)
					elm_radio_state_value_set(radio_button, 0);
				else if (type == BR_PRIVACY_SUBMENU_ALWAYS_ON)
					elm_radio_state_value_set(radio_button, 1);
				else if (type == BR_PRIVACY_SUBMENU_ALWAYS_OFF)
					elm_radio_state_value_set(radio_button, 2);

				elm_radio_group_add(radio_button, main_view->m_auto_save_id_pass_radio_group);

				char* auto_save = vconf_get_str(AUTO_SAVE_ID_PASSWORD_KEY);
				if (!auto_save) {
					vconf_set_str(AUTO_SAVE_ID_PASSWORD_KEY, ALWAYS_ASK);
					auto_save = strdup(ALWAYS_ASK);
				}

				if (!auto_save) {
					BROWSER_LOGE("strdup failed");
					return NULL;
				}

				if (!strncmp(auto_save, ALWAYS_ASK, strlen(ALWAYS_ASK)))
					elm_radio_value_set(main_view->m_auto_save_id_pass_radio_group, 0);
				else if (!strncmp(auto_save, ALWAYS_ON, strlen(ALWAYS_ON)))
					elm_radio_value_set(main_view->m_auto_save_id_pass_radio_group, 1);
				else
					elm_radio_value_set(main_view->m_homepage_radio_group, 2);

				free(auto_save);
			}

			return radio_button;
		}
	} else if (type == BR_STORAGE_SUBMENU_PHONE
		|| type == BR_STORAGE_SUBMENU_MEMORY_CARD) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			Evas_Object *radio_button = elm_radio_add(obj);
			if (radio_button) {
				if (type == BR_STORAGE_SUBMENU_PHONE)
					elm_radio_state_value_set(radio_button, 0);
				else {
					main_view->m_default_storage_mmc_radio_button = radio_button;
					elm_radio_state_value_set(radio_button, 1);
				}

				elm_radio_group_add(radio_button, main_view->m_default_storage_radio_group);

				int current_storage = SETTING_DEF_MEMORY_PHONE;
				if(vconf_get_int(VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT, &current_storage) < 0)
				{
					vconf_set_int(VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT, SETTING_DEF_MEMORY_PHONE);
					current_storage = SETTING_DEF_MEMORY_PHONE;
				}
				int mmc = VCONFKEY_SYSMAN_MMC_MOUNTED;
				if (vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &mmc) < 0) {
					BROWSER_LOGE("[%s]vconf_get_int failed", VCONFKEY_SYSMAN_MMC_STATUS);
					mmc = SETTING_DEF_MEMORY_MMC;
				}
				if (mmc == VCONFKEY_SYSMAN_MMC_MOUNTED)
					elm_object_disabled_set(main_view->m_default_storage_mmc_radio_button, EINA_FALSE);
				else
					elm_object_disabled_set(main_view->m_default_storage_mmc_radio_button, EINA_TRUE);

				if (current_storage == SETTING_DEF_MEMORY_PHONE)
					elm_radio_value_set(main_view->m_default_storage_radio_group, 0);
				else if (current_storage == SETTING_DEF_MEMORY_MMC) {
					if (mmc == VCONFKEY_SYSMAN_MMC_MOUNTED)
						elm_radio_value_set(main_view->m_default_storage_radio_group, 1);
					else
						elm_radio_value_set(main_view->m_default_storage_radio_group, 0);
				}
			}
			return radio_button;
		}
	}

	return NULL;
}

void Browser_Settings_Main_View::__run_javascript_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	const char *key = RUN_JAVASCRIPT_KEY;
	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	if (vconf_set_bool(key, state) != 0)
		SLOGE("Key: %s, FAILED", key);
}

void Browser_Settings_Main_View::__display_images_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	const char *key = DISPLAY_IMAGES_KEY;
	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	if (vconf_set_bool(key, state) != 0)
		SLOGE("Key: %s, FAILED", key);
}

void Browser_Settings_Main_View::__block_popup_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	const char *key = BLOCK_POPUP_KEY;
	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	if (vconf_set_bool(key, state) != 0)
		SLOGE("Key: %s, FAILED", key);
}

void Browser_Settings_Main_View::__accept_cookies_check_changed_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	const char *key = ACCEPT_COOKIES_KEY;
	Eina_Bool state = elm_check_state_get((Evas_Object*)data);
	if (vconf_set_bool(key, state) != 0)
		SLOGE("Key: %s, FAILED", key);
}

char *Browser_Settings_Main_View::__genlist_label_get(void *data, Evas_Object *obj, const char *part)
{
	if (!data)
		return NULL;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	if (type == BR_HOMEPAGE_TITLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_HOMEPAGE);
	} else if (type == BR_HOMEPAGE_MENU) {
		if (!strncmp(part, "elm.text.1", strlen("elm.text.1")))
			return strdup(BR_STRING_HOMEPAGE);
		else if (!strncmp(part, "elm.text.2", strlen("elm.text.2"))) {
			char *homepage = vconf_get_str(HOMEPAGE_KEY);
			BROWSER_LOGD("homepage = %s", homepage);
			if (homepage) {
				if (!strncmp(homepage, MOST_VISITED_SITES, strlen(MOST_VISITED_SITES))) {
					free(homepage);
					return strdup(BR_STRING_EMPTY_PAGE);
				} else if (!strncmp(homepage, RECENTLY_VISITED_SITE,
							strlen(RECENTLY_VISITED_SITE))) {
					free(homepage);
					return strdup(BR_STRING_RECENTLY_VISITED_SITE);
				} else {
					free(homepage);
					return strdup(BR_STRING_USER_HOMEPAGE);
				}
			}
			return NULL;
		}
	} else if (type == BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_EMPTY_PAGE);
	} else if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_RECENTLY_VISITED_SITE);
	} else if (type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return vconf_get_str(USER_HOMEPAGE_KEY);
	} else if (type == BR_DISPLAY_TITLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_DISPLAY);
	} else if (type == BR_DISPLAY_MENU_DEFAULT_VIEW_LEVEL) {
		if (!strncmp(part, "elm.text.1", strlen("elm.text.1")))
			return strdup(BR_STRING_DEFAULT_VIEW_LEVEL);
		else if (!strncmp(part, "elm.text.2", strlen("elm.text.2"))) {
			char *view_level = vconf_get_str(DEFAULT_VIEW_LEVEL_KEY);
			if (view_level) {
				if (!strncmp(view_level, FIT_TO_WIDTH, strlen(FIT_TO_WIDTH))) {
					free(view_level);
					return strdup(BR_STRING_FIT_TO_WIDTH);
				} else {
					free(view_level);
					return strdup(BR_STRING_READABLE);
				}
			}
			return NULL;
		}
	} else if (type == BR_DISPLAY_SUBMENU_FIT_TO_WIDTH) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_FIT_TO_WIDTH);
	} else if (type == BR_DISPLAY_SUBMENU_READABLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_READABLE);
	} else if (type == BR_CONTENT_TITLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_CONTENT);
	} else if (type == BR_CONTENT_MENU_RUN_JAVASCRIPT) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_RUN_JAVASCRIPT);
	} else if (type == BR_CONTENT_MENU_DISPLAY_IMAGES) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_DISPLAY_IMAGES);
	} else if (type == BR_CONTENT_MENU_BLOCK_POPUP) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_BLOCK_POPUP);
	} else if (type == BR_PRIVACY_TITLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_PRIVACY);
	} else if (type == BR_PRIVACY_MENU_ACCEPT_COOKIES) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_ACCEPT_COOKIES);
	} else if (type == BR_PRIVACY_MENU_AUTOSAVE_ID_PASSWORD) {
		if (!strncmp(part, "elm.text.1", strlen("elm.text.1")))
			return strdup(BR_STRING_AUTO_SAVE_ID_PASSWORD);
		else if (!strncmp(part, "elm.text.2", strlen("elm.text.2"))) {
			char *auto_save = vconf_get_str(AUTO_SAVE_ID_PASSWORD_KEY);
			BROWSER_LOGD("auto_save = %s", auto_save);
			if (auto_save) {
				if (!strncmp(auto_save, ALWAYS_ASK, strlen(ALWAYS_ASK))) {
					free(auto_save);
					return strdup(BR_STRING_ALWAYS_ASK);
				} else if (!strncmp(auto_save, ALWAYS_ON, strlen(ALWAYS_ON))) {
					free(auto_save);
					return strdup(BR_STRING_ON);
				} else {
					free(auto_save);
					return strdup(BR_STRING_OFF);
				}
			}
			return NULL;
		}
	} else if (type == BR_PRIVACY_SUBMENU_ALWAYS_ASK) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_ALWAYS_ASK);
	} else if (type == BR_PRIVACY_SUBMENU_ALWAYS_ON) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_ON);
	} else if (type == BR_PRIVACY_SUBMENU_ALWAYS_OFF) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_OFF);
	} else if (type == BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_CLEAR_PRIVATE_DATA);
	} else if (type == BR_DEBUG_TITLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_DEBUG);
	} else if (type == BR_MENU_USER_AGENT) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_USER_AGENT);
	} else if (type == BR_STORAGE_TITLE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_STORAGE);
	} else if (type == BR_STORAGE_MENU_DEFAULT_STORAGE) {
		if (!strncmp(part, "elm.text.1", strlen("elm.text.1")))
			return strdup(BR_STRING_DEFAULT_STORAGE);
		else if (!strncmp(part, "elm.text.2", strlen("elm.text.2"))) {
			int current_storage = SETTING_DEF_MEMORY_PHONE;
			if(vconf_get_int(VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT, &current_storage) < 0) {
				BROWSER_LOGE("[%s]vconf_get_int failed", VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT);
				return strdup(BR_STRING_PHONE);
			}
			if (current_storage == SETTING_DEF_MEMORY_PHONE)
				return strdup(BR_STRING_PHONE);
			else {
				int mmc = VCONFKEY_SYSMAN_MMC_MOUNTED;
				if (vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &mmc) < 0) {
					BROWSER_LOGE("[%s]vconf_get_int failed", VCONFKEY_SYSMAN_MMC_STATUS);
					return strdup(BR_STRING_MEMORY_CARD);
				}
				if (mmc == VCONFKEY_SYSMAN_MMC_MOUNTED)
					return strdup(BR_STRING_MEMORY_CARD);
				else
					return strdup(BR_STRING_PHONE);
			}
		}
	} else if (type == BR_STORAGE_SUBMENU_PHONE) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_PHONE);
	} else if (type == BR_STORAGE_SUBMENU_MEMORY_CARD) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_MEMORY_CARD);
	} else if (type == BR_MENU_PLUGINS) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_PLUGINS);
	} else if (type == BR_MENU_ACCELERATED_COMPOSITION) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup("Accelerated composition");
	} else if (type == BR_MENU_RESET_TO_DEFAULT) {
		if (!strncmp(part, "elm.text", strlen("elm.text")))
			return strdup(BR_STRING_RESET_TO_DEFAULT);
	}

	return NULL;
}

void Browser_Settings_Main_View::__homepage_sub_item_clicked_cb(void *data, Evas_Object *obj,
											void *event_info)
{
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	int radio_value = 0;
	if (type == BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES)
		radio_value = 0;
	else if (type == BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE)
		radio_value = 1;
	else if (type == BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE)
		radio_value = 2;

	elm_radio_value_set(main_view->m_homepage_radio_group, radio_value);

	if (radio_value == 0)
		vconf_set_str(HOMEPAGE_KEY, MOST_VISITED_SITES);
	else if (radio_value == 1)
		vconf_set_str(HOMEPAGE_KEY, RECENTLY_VISITED_SITE);
	else if (radio_value == 2) {
		main_view->m_edit_homepage_view = new(nothrow) Browser_Settings_Edit_Homepage_View(main_view);
		if (!main_view->m_edit_homepage_view) {
			BROWSER_LOGE("new Browser_Settings_Edit_Homepage_View failed");
			return;
		}
		if (!main_view->m_edit_homepage_view->init()) {
			BROWSER_LOGE("m_edit_homepage_view failed");
			delete main_view->m_edit_homepage_view;
			main_view->m_edit_homepage_view = NULL;
		}
	}
	elm_genlist_item_update(main_view->m_homepage_item_callback_data.it);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void Browser_Settings_Main_View::__default_view_level_sub_item_clicked_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	int radio_value = 0;
	if (type == BR_DISPLAY_SUBMENU_FIT_TO_WIDTH)
		radio_value = 0;
	else if (type == BR_DISPLAY_SUBMENU_READABLE)
		radio_value = 1;

	if (elm_radio_value_get(main_view->m_default_view_level_radio_group) != radio_value) {
		elm_radio_value_set(main_view->m_default_view_level_radio_group, radio_value);
		if (radio_value == 0)
			vconf_set_str(DEFAULT_VIEW_LEVEL_KEY, FIT_TO_WIDTH);
		else
			vconf_set_str(DEFAULT_VIEW_LEVEL_KEY, READABLE);

		elm_genlist_item_update(main_view->m_defailt_view_level_item_callback_data.it);
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void Browser_Settings_Main_View::__auto_save_id_pass_sub_item_clicked_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	int radio_value = 0;
	if (type == BR_PRIVACY_SUBMENU_ALWAYS_ASK)
		radio_value = 0;
	else if (type == BR_PRIVACY_SUBMENU_ALWAYS_ON)
		radio_value = 1;
	else if (type == BR_PRIVACY_SUBMENU_ALWAYS_OFF)
		radio_value = 2;

	if (elm_radio_value_get(main_view->m_auto_save_id_pass_radio_group) != radio_value) {
		elm_radio_value_set(main_view->m_auto_save_id_pass_radio_group, radio_value);
		if (radio_value == 0)
			vconf_set_str(AUTO_SAVE_ID_PASSWORD_KEY, ALWAYS_ASK);
		else if (radio_value == 1)
			vconf_set_str(AUTO_SAVE_ID_PASSWORD_KEY, ALWAYS_ON);
		else if (radio_value == 2)
			vconf_set_str(AUTO_SAVE_ID_PASSWORD_KEY, ALWAYS_OFF);

		elm_genlist_item_update(main_view->m_auto_save_item_callback_data.it);
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void Browser_Settings_Main_View::__default_storage_sub_item_clicked_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	int radio_value = 0;
	if (type == BR_STORAGE_SUBMENU_PHONE)
		radio_value = 0;
	else
		radio_value = 1;

	if (elm_radio_value_get(main_view->m_default_storage_radio_group) != radio_value) {
		elm_radio_value_set(main_view->m_default_storage_radio_group, radio_value);
		if (radio_value == 0) {
			if (vconf_set_int(VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT, SETTING_DEF_MEMORY_PHONE) < 0)
				BROWSER_LOGE("[%s]vconf_set_int failed", VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT);
		}
		else if (radio_value == 1
			  && elm_object_disabled_get(main_view->m_default_storage_mmc_radio_button) == EINA_FALSE)
			if (vconf_set_int(VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT, SETTING_DEF_MEMORY_MMC) < 0)
				BROWSER_LOGE("[%s]vconf_set_int failed", VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT);

		elm_genlist_item_update(main_view->m_default_storage_item_callback_data.it);
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void Browser_Settings_Main_View::__expandable_icon_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);
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
			/* If homepage menu. */
			main_view->m_most_visited_item_callback_data.type = BR_HOMEPAGE_SUBMENU_MOST_VISITED_SITES;
			main_view->m_most_visited_item_callback_data.user_data = main_view;
			main_view->m_most_visited_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_most_visited_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_most_visited_item_callback_data));

			main_view->m_recently_visited_item_callback_data.type = BR_HOMEPAGE_SUBMENU_RECENTLY_VISITED_SITE;
			main_view->m_recently_visited_item_callback_data.user_data = main_view;
			main_view->m_recently_visited_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_recently_visited_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_recently_visited_item_callback_data));

			main_view->m_user_homepage_item_callback_data.type = BR_HOMEPAGE_SUBMENU_USER_HOMEPAGE;
			main_view->m_user_homepage_item_callback_data.user_data = main_view;
			main_view->m_user_homepage_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_user_homepage_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __homepage_sub_item_clicked_cb,
										&(main_view->m_user_homepage_item_callback_data));
		} else if (it == main_view->m_defailt_view_level_item_callback_data.it) {
			main_view->m_fit_to_width_item_callback_data.type = BR_DISPLAY_SUBMENU_FIT_TO_WIDTH;
			main_view->m_fit_to_width_item_callback_data.user_data = main_view;
			main_view->m_fit_to_width_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_fit_to_width_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __default_view_level_sub_item_clicked_cb,
										&(main_view->m_fit_to_width_item_callback_data));

			main_view->m_readable_item_callback_data.type = BR_DISPLAY_SUBMENU_READABLE;
			main_view->m_readable_item_callback_data.user_data = main_view;
			main_view->m_readable_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_readable_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __default_view_level_sub_item_clicked_cb,
										&(main_view->m_readable_item_callback_data));
		} else if (it == main_view->m_auto_save_item_callback_data.it) {
			main_view->m_auto_save_always_ask_item_callback_data.type = BR_PRIVACY_SUBMENU_ALWAYS_ASK;
			main_view->m_auto_save_always_ask_item_callback_data.user_data = main_view;
			main_view->m_auto_save_always_ask_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_auto_save_always_ask_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __auto_save_id_pass_sub_item_clicked_cb,
										&(main_view->m_auto_save_always_ask_item_callback_data));

			main_view->m_auto_save_always_on_item_callback_data.type = BR_PRIVACY_SUBMENU_ALWAYS_ON;
			main_view->m_auto_save_always_on_item_callback_data.user_data = main_view;
			main_view->m_auto_save_always_on_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_auto_save_always_on_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __auto_save_id_pass_sub_item_clicked_cb,
										&(main_view->m_auto_save_always_on_item_callback_data));

			main_view->m_auto_save_always_off_item_callback_data.type = BR_PRIVACY_SUBMENU_ALWAYS_OFF;
			main_view->m_auto_save_always_off_item_callback_data.user_data = main_view;
			main_view->m_auto_save_always_off_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_auto_save_always_off_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __auto_save_id_pass_sub_item_clicked_cb,
										&(main_view->m_auto_save_always_off_item_callback_data));
		} else if (it == main_view->m_default_storage_item_callback_data.it) {
			main_view->m_default_storage_phone_item_callback_data.type = BR_STORAGE_SUBMENU_PHONE;
			main_view->m_default_storage_phone_item_callback_data.user_data = main_view;
			main_view->m_default_storage_phone_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_default_storage_phone_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __default_storage_sub_item_clicked_cb,
										&(main_view->m_default_storage_phone_item_callback_data));
			main_view->m_default_storage_mmc_item_callback_data.type = BR_STORAGE_SUBMENU_MEMORY_CARD;
			main_view->m_default_storage_mmc_item_callback_data.user_data = main_view;
			main_view->m_default_storage_mmc_item_callback_data.it = elm_genlist_item_append(genlist, &(main_view->m_radio_text_item_class),
										&(main_view->m_default_storage_mmc_item_callback_data), it,
										ELM_GENLIST_ITEM_NONE, __default_storage_sub_item_clicked_cb,
										&(main_view->m_default_storage_mmc_item_callback_data));

			int mmc = VCONFKEY_SYSMAN_MMC_MOUNTED;
			if (vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &mmc) < 0) {
				BROWSER_LOGE("[%s]vconf_get_int failed", VCONFKEY_SYSMAN_MMC_STATUS);
				mmc = SETTING_DEF_MEMORY_MMC;
			}
			if (mmc == VCONFKEY_SYSMAN_MMC_MOUNTED)
				elm_object_item_disabled_set(main_view->m_default_storage_mmc_item_callback_data.it, EINA_FALSE);
			else
				elm_object_item_disabled_set(main_view->m_default_storage_mmc_item_callback_data.it, EINA_TRUE);
		}
	}

	elm_genlist_item_selected_set(it, EINA_FALSE);
}

void Browser_Settings_Main_View::__on_off_check_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	Eina_Bool state = EINA_TRUE;
	if (type == BR_CONTENT_MENU_RUN_JAVASCRIPT) {
		state = elm_check_state_get(main_view->m_run_javascript_check);
		elm_check_state_set(main_view->m_run_javascript_check, !state);
		__run_javascript_check_changed_cb(main_view->m_run_javascript_check, NULL, NULL);
	} else if (type == BR_CONTENT_MENU_DISPLAY_IMAGES) {
		state = elm_check_state_get(main_view->m_display_images_check);
		elm_check_state_set(main_view->m_display_images_check, !state);
		__display_images_check_changed_cb(main_view->m_display_images_check, NULL, NULL);
	} else if (type == BR_CONTENT_MENU_BLOCK_POPUP) {
		state = elm_check_state_get(main_view->m_block_popup_check);
		elm_check_state_set(main_view->m_block_popup_check, !state);
		__block_popup_check_changed_cb(main_view->m_block_popup_check, NULL, NULL);
	} else if (type == BR_PRIVACY_MENU_ACCEPT_COOKIES) {
		state = elm_check_state_get(main_view->m_accept_cookies_check);
		elm_check_state_set(main_view->m_accept_cookies_check, !state);
		__accept_cookies_check_changed_cb(main_view->m_accept_cookies_check, NULL, NULL);
	}
	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

Eina_Bool Browser_Settings_Main_View::_call_user_agent(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_user_agent_view)
		delete m_user_agent_view;
	m_user_agent_view = new(nothrow) Browser_Settings_User_Agent_View(this);
	if (!m_user_agent_view) {
		BROWSER_LOGE("new Browser_Settings_User_Agent_View failed");
		return EINA_FALSE;
	}
	if (!m_user_agent_view->init()) {
		BROWSER_LOGE("m_user_agent_view->init failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Browser_Settings_Main_View::__genlist_item_clicked_cb(void *data, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Browser_Settings_Main_View::menu_type type = callback_data->type;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)(callback_data->user_data);

	if (type == BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA) {
		main_view->m_clear_data_view = new(nothrow) Browser_Settings_Clear_Data_View(main_view);
		if (!main_view->m_clear_data_view) {
			BROWSER_LOGE("new Browser_Settings_Clear_Data_View failed");
			return;
		}
		if (!main_view->m_clear_data_view->init()) {
			BROWSER_LOGE("m_clear_data_view->init failed");
			delete main_view->m_clear_data_view;
			main_view->m_clear_data_view = NULL;
			return;
		}
	} else if (type == BR_MENU_USER_AGENT) {
		if (!main_view->_call_user_agent())
			BROWSER_LOGE("_call_user_agent failed");
	}
	else if (type == BR_MENU_PLUGINS) {
		main_view->m_plugin_view = new(nothrow) Browser_Settings_Plugin_View(main_view);
		if (!main_view->m_plugin_view) {
			BROWSER_LOGE("new Browser_Settings_Plugin_View failed");
			return;
		}
		if (!main_view->m_plugin_view->init()) {
			BROWSER_LOGE("m_plugin_view->init failed");
			delete main_view->m_plugin_view;
			main_view->m_plugin_view = NULL;
			return;
		}
	}
	else if (type == BR_MENU_ACCELERATED_COMPOSITION) {
		main_view->m_accelerated_composition_view = new(nothrow) Browser_Settings_Accelerated_Composition(main_view);
		if (!main_view->m_accelerated_composition_view) {
			BROWSER_LOGE("new Browser_Settings_Accelerated_Composition failed");
			return;
		}
		if (!main_view->m_accelerated_composition_view->init()) {
			BROWSER_LOGE("m_accelerated_composition_view->init failed");
			delete main_view->m_accelerated_composition_view;
			main_view->m_accelerated_composition_view = NULL;
			return;
		}
	} else if (type == BR_MENU_RESET_TO_DEFAULT) {
		main_view->_show_reset_confirm_popup();
	}

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
}

void Browser_Settings_Main_View::_reset_settings(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (vconf_set_str(HOMEPAGE_KEY, MOST_VISITED_SITES) < 0)
		BROWSER_LOGE("vconf_set_str(HOMEPAGE_KEY, MOST_VISITED_SITES) failed");
	if (vconf_set_str(USER_HOMEPAGE_KEY, BROWSER_DEFAULT_USER_HOMEPAGE) < 0)
		BROWSER_LOGE("vconf_set_str(USER_HOMEPAGE_KEY, BROWSER_DEFAULT_USER_HOMEPAGE) failed");
	if (vconf_set_str(DEFAULT_VIEW_LEVEL_KEY, READABLE) < 0)
		BROWSER_LOGE("vconf_set_str(DEFAULT_VIEW_LEVEL_KEY, READABLE) failed");
	if (vconf_set_bool(RUN_JAVASCRIPT_KEY, 1) < 0)
		BROWSER_LOGE("vconf_set_bool(RUN_JAVASCRIPT_KEY, 1) failed");
	if (vconf_set_bool(DISPLAY_IMAGES_KEY, 1) < 0)
		BROWSER_LOGE("vconf_set_bool(DISPLAY_IMAGES_KEY, 1) failed");
	if (vconf_set_bool(BLOCK_POPUP_KEY, 1) < 0)
		BROWSER_LOGE("vconf_set_bool(BLOCK_POPUP_KEY, 1) failed");
	if (vconf_set_bool(ACCEPT_COOKIES_KEY, 1) < 0)
		BROWSER_LOGE("vconf_set_bool(ACCEPT_COOKIES_KEY, 1) failed");
	if (vconf_set_str(AUTO_SAVE_ID_PASSWORD_KEY, ALWAYS_ASK) < 0)
		BROWSER_LOGE("vconf_set_str(AUTO_SAVE_ID_PASSWORD_KEY, ALWAYS_ASK) failed");
	if (vconf_set_int(VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT, SETTING_DEF_MEMORY_PHONE) < 0)
		BROWSER_LOGE("vconf_set_int(VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT, SETTING_DEF_MEMORY_PHONE) failed");
	if (vconf_set_bool(RUN_PLUGINS_KEY, 1) < 0)
		BROWSER_LOGE("vconf_set_bool(RUN_PLUGINS_KEY, 1) failed");
	if (vconf_set_bool(RUN_FLASH_KEY, 0) < 0)
		BROWSER_LOGE("vconf_set_bool(RUN_FLASH_KEY, 0) failed");
	if (vconf_set_bool(PAUSE_FLASH_KEY, 1) < 0)
		BROWSER_LOGE("vconf_set_bool(PAUSE_FLASH_KEY, 1) failed");

	if (vconf_set_str(USERAGENT_KEY, DEFAULT_USER_AGENT_STRING) < 0)
		BROWSER_LOGE("vconf_set_str(USERAGENT_KEY, DEFAULT_USER_AGENT_STRING) failed");

	elm_genlist_realized_items_update(m_genlist);
}

void Browser_Settings_Main_View::__reset_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_reset_confirm_popup) {
		evas_object_del(main_view->m_reset_confirm_popup);
		main_view->m_reset_confirm_popup = NULL;
	}

	main_view->_reset_settings();
}

void Browser_Settings_Main_View::__cancel_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	if (main_view->m_reset_confirm_popup) {
		evas_object_del(main_view->m_reset_confirm_popup);
		main_view->m_reset_confirm_popup = NULL;
	}
}

Eina_Bool Browser_Settings_Main_View::_show_reset_confirm_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_reset_confirm_popup)
		evas_object_del(m_reset_confirm_popup);

	m_reset_confirm_popup = elm_popup_add(m_genlist);
	if (!m_reset_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_reset_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	std::string confirm_msg = std::string(BR_STRING_RESET_TO_DEFAULT) + std::string("?");
	elm_object_text_set(m_reset_confirm_popup, confirm_msg.c_str());

	Evas_Object *ok_button = elm_button_add(m_reset_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(m_reset_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __reset_confirm_response_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_reset_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(m_reset_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_confirm_response_cb, this);

	evas_object_show(m_reset_confirm_popup);

	return EINA_TRUE;
}

void Browser_Settings_Main_View::__mmc_key_changed_cb(keynode_t *keynode, void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Settings_Main_View *main_view = (Browser_Settings_Main_View *)data;
	int mmc = VCONFKEY_SYSMAN_MMC_MOUNTED;
	if (!vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &mmc) < 0) {
		BROWSER_LOGE("[%s] vconf_get_int failed");
		mmc = VCONFKEY_SYSMAN_MMC_MOUNTED;
	}
	if (mmc == VCONFKEY_SYSMAN_MMC_MOUNTED) {
		elm_object_disabled_set(main_view->m_default_storage_mmc_radio_button, EINA_FALSE);
		elm_object_item_disabled_set(main_view->m_default_storage_mmc_item_callback_data.it, EINA_FALSE);
	} else {
		elm_radio_value_set(main_view->m_default_storage_radio_group, 0);
		elm_object_disabled_set(main_view->m_default_storage_mmc_radio_button, EINA_TRUE);
		elm_object_item_disabled_set(main_view->m_default_storage_mmc_item_callback_data.it, EINA_TRUE);
	}

	if (mmc != VCONFKEY_SYSMAN_MMC_MOUNTED) {
		if (vconf_set_int(VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT, SETTING_DEF_MEMORY_PHONE) < 0)
			BROWSER_LOGE("[%s]vconf_set_int failed", VCONFKEY_SETAPPL_DEFAULT_MEM_WAP_INT);
		elm_genlist_item_update(main_view->m_default_storage_item_callback_data.it);
	}
}

Evas_Object *Browser_Settings_Main_View::_create_content_genlist(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *genlist = NULL;

	genlist = elm_genlist_add(m_navi_bar);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	m_radio_text_item_class.item_style = "dialogue/1text.1icon/expandable2";
	m_radio_text_item_class.func.text_get = __genlist_label_get;
	m_radio_text_item_class.func.content_get = __genlist_icon_get;
	m_radio_text_item_class.func.state_get = NULL;
	m_radio_text_item_class.func.del = NULL;

	m_category_title_item_class.item_style = "dialogue/title";
	m_category_title_item_class.func.text_get = __genlist_label_get;
	m_category_title_item_class.func.content_get = NULL;
	m_category_title_item_class.func.del = NULL;
	m_category_title_item_class.func.state_get = NULL;

	/* Homepage */
	m_homepage_title_callback_data.type = BR_HOMEPAGE_TITLE;
	m_homepage_title_callback_data.user_data = this;
	m_homepage_title_callback_data.it = elm_genlist_item_append(genlist, &m_category_title_item_class,
						&m_homepage_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_homepage_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_2_text_item_class.item_style = "dialogue/2text.3/expandable";
	m_2_text_item_class.func.text_get = __genlist_label_get;
	m_2_text_item_class.func.content_get = NULL;
	m_2_text_item_class.func.del = NULL;
	m_2_text_item_class.func.state_get = NULL;

	m_homepage_item_callback_data.type = BR_HOMEPAGE_MENU;
	m_homepage_item_callback_data.user_data = this;
	m_homepage_item_callback_data.it = elm_genlist_item_append(genlist, &m_2_text_item_class,
							&m_homepage_item_callback_data, NULL, ELM_GENLIST_ITEM_TREE,
							__expandable_icon_clicked_cb, &m_homepage_item_callback_data);

	/* Display - Default view level */
	m_display_title_callback_data.type = BR_DISPLAY_TITLE;
	m_display_title_callback_data.user_data = this;
	m_display_title_callback_data.it = elm_genlist_item_append(genlist, &m_category_title_item_class,
						&m_display_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_display_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_defailt_view_level_item_callback_data.type = BR_DISPLAY_MENU_DEFAULT_VIEW_LEVEL;
	m_defailt_view_level_item_callback_data.user_data = this;
	m_defailt_view_level_item_callback_data.it = elm_genlist_item_append(genlist, &m_2_text_item_class,
						&m_defailt_view_level_item_callback_data, NULL, ELM_GENLIST_ITEM_TREE,
						__expandable_icon_clicked_cb, &m_defailt_view_level_item_callback_data);

	/* Content - Run JavaScript / Display Images */
	m_content_title_callback_data.type = BR_CONTENT_TITLE;
	m_content_title_callback_data.user_data = this;
	m_content_title_callback_data.it = elm_genlist_item_append(genlist, &m_category_title_item_class,
						&m_content_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_content_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_1_text_1_icon_item_class.item_style = "dialogue/1text.1icon";
	m_1_text_1_icon_item_class.func.text_get = __genlist_label_get;
	m_1_text_1_icon_item_class.func.content_get = __genlist_icon_get;
	m_1_text_1_icon_item_class.func.state_get = NULL;
	m_1_text_1_icon_item_class.func.del = NULL;

	m_run_javascript_item_callback_data.type = BR_CONTENT_MENU_RUN_JAVASCRIPT;
	m_run_javascript_item_callback_data.user_data = this;
	m_run_javascript_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
						&m_run_javascript_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_run_javascript_item_callback_data);

	m_display_images_item_callback_data.type = BR_CONTENT_MENU_DISPLAY_IMAGES;
	m_display_images_item_callback_data.user_data = this;
	m_display_images_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
						&m_display_images_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_display_images_item_callback_data);

	m_block_popup_item_callback_data.type = BR_CONTENT_MENU_BLOCK_POPUP;
	m_block_popup_item_callback_data.user_data = this;
	m_block_popup_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
						&m_block_popup_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_block_popup_item_callback_data);

	/* Privacy - Accept cookies / Auto save ID, password / Clear private data */
	m_privacy_title_callback_data.type = BR_PRIVACY_TITLE;
	m_privacy_title_callback_data.user_data = this;
	m_privacy_title_callback_data.it = elm_genlist_item_append(genlist, &m_category_title_item_class,
						&m_privacy_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_privacy_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_accept_cookies_item_callback_data.type = BR_PRIVACY_MENU_ACCEPT_COOKIES;
	m_accept_cookies_item_callback_data.user_data = this;
	m_accept_cookies_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_1_icon_item_class,
						&m_accept_cookies_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
						__on_off_check_clicked_cb, &m_accept_cookies_item_callback_data);

	m_auto_save_item_callback_data.type = BR_PRIVACY_MENU_AUTOSAVE_ID_PASSWORD;
	m_auto_save_item_callback_data.user_data = this;
	m_auto_save_item_callback_data.it = elm_genlist_item_append(genlist, &m_2_text_item_class,
							&m_auto_save_item_callback_data, NULL, ELM_GENLIST_ITEM_TREE,
							__expandable_icon_clicked_cb, &m_auto_save_item_callback_data);

	m_1_text_item_class.item_style = "dialogue/1text";
	m_1_text_item_class.func.text_get = __genlist_label_get;
	m_1_text_item_class.func.content_get = NULL;
	m_1_text_item_class.func.state_get = NULL;
	m_1_text_item_class.func.del = NULL;

	m_clear_private_data_item_callback_data.type = BR_PRIVACY_MENU_CLEAR_PRIVATE_DATA;
	m_clear_private_data_item_callback_data.user_data = this;
	m_clear_private_data_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_item_class,
							&m_clear_private_data_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_clear_private_data_item_callback_data);

	/* Storage */
	m_storage_title_callback_data.type = BR_STORAGE_TITLE;
	m_storage_title_callback_data.user_data = this;
	m_storage_title_callback_data.it = elm_genlist_item_append(genlist, &m_category_title_item_class,
						&m_storage_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_storage_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_default_storage_item_callback_data.type = BR_STORAGE_MENU_DEFAULT_STORAGE;
	m_default_storage_item_callback_data.user_data = this;
	m_default_storage_item_callback_data.it = elm_genlist_item_append(genlist, &m_2_text_item_class,
							&m_default_storage_item_callback_data, NULL, ELM_GENLIST_ITEM_TREE,
							__expandable_icon_clicked_cb, &m_default_storage_item_callback_data);

	if (vconf_notify_key_changed(VCONFKEY_SYSMAN_MMC_STATUS, __mmc_key_changed_cb, this) < 0)
		BROWSER_LOGE("[%s]vconf_notify_key_changed failed", VCONFKEY_SYSMAN_MMC_STATUS);

	/* Others */
	m_seperator_item_class.item_style = "dialogue/seperator";
	m_seperator_item_class.func.text_get = NULL;
	m_seperator_item_class.func.content_get = NULL;
	m_seperator_item_class.func.state_get = NULL;
	m_seperator_item_class.func.del = NULL;
	Elm_Object_Item *it = elm_genlist_item_append(genlist, &m_seperator_item_class,
						NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_plugins_item_callback_data.type = BR_MENU_PLUGINS;
	m_plugins_item_callback_data.user_data = this;
	m_plugins_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_item_class,
							&m_plugins_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_plugins_item_callback_data);

	m_reset_item_callback_data.type = BR_MENU_RESET_TO_DEFAULT;
	m_reset_item_callback_data.user_data = this;
	m_reset_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_item_class,
							&m_reset_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_reset_item_callback_data);

	/* Debug */
	m_debug_title_callback_data.type = BR_DEBUG_TITLE;
	m_debug_title_callback_data.user_data = this;
	m_debug_title_callback_data.it = elm_genlist_item_append(genlist, &m_category_title_item_class,
						&m_debug_title_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_select_mode_set(m_debug_title_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	m_user_agent_item_callback_data.type = BR_MENU_USER_AGENT;
	m_user_agent_item_callback_data.user_data = this;
	m_user_agent_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_item_class,
							&m_user_agent_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_user_agent_item_callback_data);
	m_accelerated_composition_item_callback_data.type = BR_MENU_ACCELERATED_COMPOSITION;
	m_accelerated_composition_item_callback_data.user_data = this;
	m_accelerated_composition_item_callback_data.it = elm_genlist_item_append(genlist, &m_1_text_item_class,
							&m_accelerated_composition_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							__genlist_item_clicked_cb, &m_accelerated_composition_item_callback_data);
	return genlist;
}

