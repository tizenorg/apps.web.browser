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

#ifndef BROWSER_H
#define BROWSER_H

#include <Elementary.h>
#include <Evas.h>

#include "browser-object.h"

class add_tag_view;
class bookmark;
class bookmark_view;
class bookmark_add_view;
class bookmark_select_folder_view;
class bookmark_create_folder_view;
class bookmark_create_folder_save_view;
class bookmark_edit_view;
class history;
class geolocation_manager;
class history_view;
class browser_view;
class download_manager;
#if defined(MDM)
class mdm_manager;
#endif
class multiwindow_view;
class network_manager;
class scrap_view;
class setting_view;
class user_agent_manager;
#if defined(INSTALL_WEB_APP)
class webapp_install_manager;
#endif
class webview_list;
#if defined(WEBCLIP)
class scissorbox_view;
#endif

class browser : public browser_object {
public:
	browser(Evas_Object *main_window);
	~browser(void);

	void pause(void);
	void resume(void);
	void rotate(int degree);
	void launch(const char *uri = NULL);
	void show_popup_browser(Eina_Bool popup);
	void set_full_screen_enable(Eina_Bool enable);
	void set_app_in_app_enable(Eina_Bool enable);
	Eina_Bool get_app_in_app_enable(void);

	browser_view *get_browser_view(void) { return m_browser_view; }
	multiwindow_view *get_multiwindow_view(Eina_Bool init_bookmark = EINA_FALSE);
	network_manager *get_network_manager(void);
	user_agent_manager *get_user_agent_manager(void);
	download_manager *get_download_manager(void);
	webview_list *get_webview_list(void) { return m_webview_list; }
	bookmark *get_bookmark(void);
	bookmark_view *get_bookmark_view(void);
	bookmark_add_view *get_bookmark_add_view(void) { return m_bookmark_add_view;}
	bookmark_add_view *create_bookmark_add_view(const char  *title = NULL, const char *uri = NULL, int folder_id_to_save = root_folder_id, Eina_Bool edit_mode = EINA_FALSE);
	bookmark_select_folder_view *create_bookmark_select_folder_view(Evas_Smart_Cb cb_func, void *cb_data, Eina_Bool enable_create_folder);
	bookmark_select_folder_view *get_bookmark_select_folder_view() { return m_bookmark_select_folder_view;}
	bookmark_create_folder_view *get_bookmark_create_folder_view(Evas_Smart_Cb cb_func = NULL, void *cb_data = NULL);
	bookmark_create_folder_save_view *get_bookmark_create_folder_save_view(Evas_Smart_Cb cb_func = NULL, void *cb_data = NULL, int folder_id = root_folder_id);
	bookmark_edit_view *create_bookmark_edit_view(bool mode);
	bookmark_edit_view *get_bookmark_edit_view() { return m_bookmark_edit_view; }
	geolocation_manager *get_geolocation_manager(void);
#if defined(INSTALL_WEB_APP)
	webapp_install_manager *get_webapp_install_manager(void);
#endif
	history *get_history(void);
	history_view *get_history_view(void);
	setting_view *get_setting_view(void);
	scrap_view *get_scrap_view(void);
#if defined(BROWSER_TAG)
	add_tag_view *get_add_tag_view(Evas_Smart_Cb done_cb = NULL, void *done_cb_data = NULL, const char *tag = NULL);
#endif
	void delete_setting_view(void);
	void delete_multiwindow_view(void);
	void delete_history_view_view(void);
	void delete_bookmark_add_view(void);
	void delete_bookmark_select_folder_view(void);
	void delete_bookmark_create_folder_view(void);
	void delete_bookmark_create_folder_save_view(void);
	void delete_bookmark_edit_view(void);
	void delete_scrap_view(void);
#if defined(BROWSER_TAG)
	void delete_add_tag_view(void);
#endif
#if defined(MDM)
	mdm_manager *get_mdm_manager(void) { return m_mdm_manager; }
#endif
#if defined(WEBCLIP)
	scissorbox_view *get_scissorbox_view(void);
	void delete_scissorbox_view(void);
#endif
private:
	browser_view *m_browser_view;
#if defined(MDM)
	mdm_manager *m_mdm_manager;
#endif
	webview_list *m_webview_list;
	network_manager *m_network_manager;
	user_agent_manager *m_user_agent_manager;
	download_manager *m_download_manager;
	bookmark *m_bookmark;
	bookmark_view *m_bookmark_view;
	bookmark_add_view *m_bookmark_add_view;
	history *m_history;
	geolocation_manager *m_geolocation_manager;
#if defined(INSTALL_WEB_APP)
	webapp_install_manager *m_webapp_install_manager;
#endif
	history_view *m_history_view;
	multiwindow_view *m_multiwindow_view;
	setting_view *m_setting_view;
	bookmark_select_folder_view *m_bookmark_select_folder_view;
	bookmark_create_folder_view *m_bookmark_create_folder_view;
	bookmark_create_folder_save_view *m_bookmark_create_folder_save_view;
	bookmark_edit_view *m_bookmark_edit_view;
	scrap_view *m_scrap_view;
#if defined(BROWSER_TAG)
	add_tag_view *m_add_tag_view;
#endif
#if defined(WEBCLIP)
	scissorbox_view *m_scissorbox_view;
#endif
};

#endif /* BROWSER_H */

