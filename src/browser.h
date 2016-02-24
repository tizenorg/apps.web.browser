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

#ifndef BROWSER_H
#define BROWSER_H

#include <Elementary.h>
#include <Evas.h>
#include <vector>

#include "browser-object.h"
#include "certificate-view.h"
#include "bookmark-add-view.h"
#include "certificate-view.h"
#include <ui-gadget.h>

#include "tab-manager-view-lite.h"
class bookmark;
class bookmark_view;
class bookmark_add_view;
class bookmark_select_folder_view;
class bookmark_create_folder_view;
class bookmark_edit_view;
class bookmark_listener;
class browser_view;
class cloud_sync_manager;
class download_manager;
class history;
class history_item;
class history_listener;
class history_view;
class main_view;
class network_manager;
class settings;
class tab_sync_view;
class user_agent_manager;
class webview_container;
class webview_list;
#ifdef ENABLE_WEB_NOTIFICATION_API
class web_notification_manager;
#endif
class browser : public browser_object {
public:
	browser(Evas_Object *main_window);
	~browser(void);

	void unregister_common_view(common_view *view);
	void register_common_view(common_view *view);
	void register_history_listener(history_listener *hl);
	void unregister_history_listener(history_listener *hl);
	void register_bookmark_listener(bookmark_listener *bl);
	void unregister_bookmark_listener(bookmark_listener *bl);
	void notify_history_added(int id, const char *title, const char *uri, Evas_Object *snapshot, Evas_Object *favicon, int visit_count);
	void notify_history_deleted(const char *uri);
	void notify_history_cleared(Eina_Bool is_cancelled);
	void notify_bookmark_added(const char *uri, int bookmark_id, int parent_id);
	void notify_bookmark_removed(const char *uri, int id, int parent_id);
	void notify_bookmark_updated(const char *uri, const char *title, int bookmark_id, int parent_id);
	void reset(void);
	void deregister(void);
	void pause(void);
	void resume(void);
	void rotate(int degree);
	void launch(const char *uri = NULL);
	void launch_homepage(void);
	void language_changed(void);
	void set_full_screen_enable(Eina_Bool enable);
	Eina_Bool get_full_screen_enable(void) const { return m_is_full_screen; }
	main_view *get_main_view(void);
	browser_view *get_browser_view(void);
	certificate_view *get_certificate_view(X509 *m_certificate = NULL, Evas_Smart_Cb cb_func = NULL, void *cb_data = NULL);
	Eina_Bool is_certificate_view_exist(void) const { return (m_certificate_view != NULL); }
	Eina_Bool is_history_view_exist(void) const { return (m_history_view != NULL); }
	Eina_Bool is_bookmark_view_exist(void) const { return (m_bookmark_view != NULL); }
	Eina_Bool is_tab_manager_view_exist(void) const { return (m_tab_manager_view != NULL); }

	network_manager *get_network_manager(void);
	user_agent_manager *get_user_agent_manager(void);
	download_manager *get_download_manager(void);
	webview_list *get_webview_list(void);
	bookmark *get_bookmark(void);
	bookmark_view *get_bookmark_view(void);
	bookmark_add_view *get_bookmark_add_view(void) const { return m_bookmark_add_view;}
	bookmark_add_view *create_bookmark_add_view(const char *title, const char *uri, int folder_id_to_save, Eina_Bool edit_mode);
	bookmark_select_folder_view *create_bookmark_select_folder_view(Evas_Smart_Cb cb_func, void *cb_data);
	bookmark_select_folder_view *get_bookmark_select_folder_view() const { return m_bookmark_select_folder_view;}
	bookmark_create_folder_view *get_bookmark_create_folder_view(Evas_Smart_Cb cb_func, void *cb_data, int parent_id);
	bookmark_edit_view *create_bookmark_edit_folder_reorder_view(Evas_Smart_Cb cb_func, void *cb_data, int folder_id, const char *path_string);
	bookmark_edit_view *create_bookmark_edit_folder_move_view(Evas_Smart_Cb cb_func, void *cb_data, int folder_id, const char *path_string);
	bookmark_edit_view *create_bookmark_edit_folder_delete_view(Evas_Smart_Cb cb_func, void *cb_data, int folder_id, const char *path_string);
	bookmark_edit_view *get_bookmark_edit_view() const { return m_bookmark_edit_view; }
	cloud_sync_manager *get_cloud_sync_manager(void);
	history *get_history(void);
	history_view *get_history_view(void);
	settings *get_settings(void);
	Eina_Bool is_settings_exist(void) const { return (m_settings != NULL); }
#ifdef ENABLE_WEB_NOTIFICATION_API
	web_notification_manager *get_web_noti_manager(void);
#endif
	tab_manager_view_lite *get_tab_manager_view(void);
	void delete_settings(void);
#ifdef ENABLE_WEB_NOTIFICATION_API
	void delete_web_noti_manager(void);
#endif
	void delete_history_view_view(void);
	void delete_bookmark_view(void);
	void delete_bookmark_add_view(void);
	void delete_bookmark_select_folder_view(void);
	void delete_bookmark_create_folder_view(void);
	void delete_bookmark_edit_view(void);
	void delete_tab_manager_view(void);
	void delete_certificate_view();
	void launch_for_searching(const char *keyword);
	void set_developer_mode(Eina_Bool mode) { m_developer_mode = mode; }
	Eina_Bool get_developer_mode(void) { return m_developer_mode; }
	Eina_Bool get_app_paused(void) const { return m_app_paused; }
	void set_app_paused(Eina_Bool paused) { m_app_paused = paused; }
	Eina_Bool get_app_paused_by_display_off(void) const { return m_app_paused_by_display_off; }
	void set_app_paused_by_display_off(Eina_Bool paused) { m_app_paused_by_display_off = paused; }
	void clear_all_popups(void);
	Eina_Bool get_add_bookmark_view_exist(void) const { return (m_bookmark_add_view != NULL); }
	void navi_frame_tree_focus_allow_set(Eina_Bool allow);
	Eina_Bool is_tts_enabled(void);
	Eina_Bool is_keyboard_active(void);
	void set_referer_header(char *header);
	char *get_referer_header(void) const { return m_referer_header; }
	Eina_Bool is_set_referer_header(void) const { return (m_referer_header != NULL); }

private:
	void _hide_browser_popups(void);

	main_view *m_main_view;
	browser_view *m_browser_view;
	webview_list *m_webview_list;
	network_manager *m_network_manager;
	user_agent_manager *m_user_agent_manager;
	download_manager *m_download_manager;
	bookmark *m_bookmark;
	bookmark_view *m_bookmark_view;
	bookmark_add_view *m_bookmark_add_view;
	history *m_history;
	history_view *m_history_view;
	Eina_Bool m_developer_mode;
	certificate_view *m_certificate_view;
	settings *m_settings;
#ifdef ENABLE_WEB_NOTIFICATION_API
	web_notification_manager *m_web_notification_manager;
#endif
	bookmark_select_folder_view *m_bookmark_select_folder_view;
	bookmark_create_folder_view *m_bookmark_create_folder_view;
	bookmark_edit_view *m_bookmark_edit_view;
	cloud_sync_manager *m_cloud_sync_manager;
	tab_manager_view_lite *m_tab_manager_view;
	Eina_Bool m_is_full_screen;
	Eina_Bool m_app_paused;
	Eina_Bool m_app_paused_by_display_off;
	std::vector<common_view *> m_common_view_list;
	std::vector<history_listener *> m_history_listener_list;
	std::vector<bookmark_listener * > m_bookmark_listener_list;
	char *m_referer_header;
};

#endif /* BROWSER_H */
