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

#include "webview.h"

#include <cairo.h>

#include <Ecore_X.h>
#include <Elementary.h>
#include <bundle.h>
#include <fcntl.h>
#include <string>
#include <web_tab.h>
#include <efl_extension.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "certificate-manager.h"
#include "cloud-sync-manager.h"
#include "download-manager.h"
#include "find-on-page.h"
#include "history.h"
#include "html5-feature-manager.h"
#include "main-view.h"
#include "network-manager.h"
#include "platform-service.h"
#include "preference.h"
#include "progress-bar.h"
#include "search-engine-manager.h"
#include "url-bar.h"
#include "user-agent-manager.h"
#include "webview-list.h"
#include "webview_info.h"

#define common_edj_path browser_edj_dir"/browser-common.edj"
#define PATH_WEBSITE_ICON_PNG browser_data_dir"/website_icon.png"
#define PATH_WEBVIEW_DEFAULT_SNAPSHOT browser_img_dir"/I01_webview_default_snapshot_bg.png"
#define APPLICATION_NAME_FOR_USER_AGENT "SamsungBrowser/1.0"
#ifdef WEBKIT_EFL
#define USER_AGENT_STRING_FOR_WORKAROUND "Mozilla/5.0 (Linux; Tizen 2.4; SAMSUNG SM-Z130H) AppleWebKit/537.48 (KHTML, like Gecko) Version/2.4 SamsungBrowser/1.0 Mobile Safari/537.48"
#else
#define USER_AGENT_STRING_FOR_WORKAROUND "Mozilla/5.0 (Linux; Tizen 2.4; SAMSUNG SM-Z130H) AppleWebKit/537.3 (KHTML, like Gecko) Chrome/40.0.2214.94 SamsungBrowser/1.0 Mobile Safari/537.3"
#endif
static void _exit_browser(void)
{
	BROWSER_LOGD("");
	elm_exit();
}

#define FIND_WORD_MAX_COUNT	10000

webview_context * webview_context::m_instance = NULL;

webview_context::webview_context(void)
	:m_context(NULL)
{
	BROWSER_LOGD("");
	ewk_init();
	m_context = ewk_context_default_get();
}

webview_context::~webview_context(void)
{
	BROWSER_LOGD("");

	ewk_shutdown();
}

void webview_context::network_session_changed(void)
{
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (wv && wv->is_loading())
		ewk_view_url_set(wv->get_ewk_view(), wv->get_uri());
}

Evas_Object *webview_context::get_favicon(const char *uri)
{
	RETV_MSG_IF(!uri, NULL, "uri is NULL");

	return ewk_context_icon_database_icon_object_add(
		m_context, uri, evas_object_evas_get(m_window));
}

void webview_context::clear_cookies(void)
{
	Ewk_Cookie_Manager *cookie_manager = ewk_context_cookie_manager_get(m_context);
	ewk_cookie_manager_cookies_clear(cookie_manager);
}

void webview_context::clear_form_data(void)
{
	ewk_context_form_candidate_data_delete_all(m_context);
}

void webview_context::clear_saved_ID_and_PW(void)
{
	ewk_context_form_password_data_delete_all(m_context);
}

void webview_context::clear_private_data(void)
{
	clear_memory_cache();

	ewk_context_web_indexed_database_delete_all(m_context);
	ewk_context_application_cache_delete_all(m_context);
	ewk_context_web_storage_delete_all(m_context);
}

void webview_context::vibration_callbacks_set(
	Ewk_Vibration_Client_Vibrate_Cb vibrate,
	Ewk_Vibration_Client_Vibration_Cancel_Cb cancel, void *data)
{
	ewk_context_vibration_client_callbacks_set(m_context, vibrate, cancel,
		data);
}

void webview_context::clear_memory_cache(void)
{
	ewk_context_cache_clear(m_context);
}

typedef struct _webview_callback
{
	const char *event;
	Evas_Smart_Cb func;
	void *func_data;
} webview_callback;

#ifdef ENABLE_INCOGNITO_WINDOW
webview::webview(Eina_Bool user_created, Eina_Bool is_incognito)
#else
webview::webview(Eina_Bool user_created)
#endif
	: m_ewk_view(NULL)
	, m_event_callbacks(NULL)
	, m_already_activated(EINA_FALSE)
	, m_is_user_created(user_created)
	, m_html5_manager(NULL)
	, m_certi_manager(NULL)
	, m_snapshot(NULL)
	, m_thumbnail(NULL)
	, m_loading_error_confirm_popup(NULL)
	, m_uri(NULL)
	, m_title(NULL)
	, m_has_focus(EINA_FALSE)
	, m_sync_id(-1)
#ifdef ENABLE_INCOGNITO_WINDOW
	, m_incognito(is_incognito)
#else
	, m_incognito(EINA_FALSE)
#endif
	, m_parent_webview(NULL)
	, m_load_finish_timer(NULL)
	, m_init_url(NULL)
	, m_request_uri(NULL)
	, m_url_for_bookmark_update(NULL)
	, m_is_data_scheme(EINA_FALSE)
	, m_is_file_scheme(EINA_FALSE)
	, m_is_backward(EINA_FALSE)
	, m_progress(0.0)
	, m_is_download_url(EINA_FALSE)
	, m_navigation_count(0)
	, m_is_error_page(EINA_FALSE)
	, m_webicon(NULL)
	, m_is_suspended(EINA_FALSE)
	, m_sync_webview_idler(NULL)
	, m_load_started_idler(NULL)
	, m_load_committed_idler(NULL)
	, m_load_finished_idler(NULL)
	, m_is_url_bar_shown(EINA_TRUE)
	, m_current_orientation(-1)
	, m_requested_webview(EINA_FALSE)
{
	BROWSER_LOGD("");
	_create_ewk_view();
}

webview::webview(const char *title, const char *uri, Eina_Bool incognito, int sync_id, Eina_Bool create_ewk_view)
	: m_ewk_view(NULL)
	, m_event_callbacks(NULL)
	, m_already_activated(EINA_FALSE)
	, m_is_user_created(EINA_TRUE)
	, m_html5_manager(NULL)
	, m_certi_manager(NULL)
	, m_snapshot(NULL)
	, m_thumbnail(NULL)
	, m_loading_error_confirm_popup(NULL)
	, m_uri(NULL)
	, m_title(NULL)
	, m_has_focus(EINA_FALSE)
	, m_sync_id(sync_id)
	, m_incognito(incognito)
	, m_parent_webview(NULL)
	, m_load_finish_timer(NULL)
	, m_init_url(NULL)
	, m_request_uri(NULL)
	, m_url_for_bookmark_update(NULL)
	, m_is_data_scheme(EINA_FALSE)
	, m_is_file_scheme(EINA_FALSE)
	, m_is_backward(EINA_FALSE)
	, m_progress(0.0)
	, m_is_download_url(EINA_FALSE)
	, m_is_error_page(EINA_FALSE)
	, m_webicon(NULL)
	, m_is_suspended(EINA_FALSE)
	, m_sync_webview_idler(NULL)
	, m_load_started_idler(NULL)
	, m_load_committed_idler(NULL)
	, m_load_finished_idler(NULL)
	, m_is_url_bar_shown(EINA_TRUE)
	, m_new_window_count(0)
	, m_current_orientation(-1)
	, m_requested_webview(EINA_FALSE)
{
	BROWSER_LOGD("");

	if (create_ewk_view)
		_create_ewk_view();

	m_title = eina_stringshare_add(title);
	m_uri = eina_stringshare_add(uri);
	BROWSER_SECURE_LOGD("m_title=[%s], m_uri=[%s]", m_title, m_uri);

	_set_init_url(uri);
}

webview::~webview(void)
{
	BROWSER_LOGD("");

	delete_all_idler();

	while (m_event_callbacks) {
		webview_callback *cb = (webview_callback *)eina_list_data_get(m_event_callbacks);
		m_event_callbacks = eina_list_remove(m_event_callbacks, cb);
		if (cb->event)
			eina_stringshare_del(cb->event);
		free(cb);
	}

	eina_stringshare_del(m_uri);
	eina_stringshare_del(m_title);

	eina_stringshare_del(m_init_url);

	if (m_request_uri)
		free(m_request_uri);

	SAFE_FREE_OBJ(m_snapshot);
	SAFE_FREE_OBJ(m_thumbnail);
	SAFE_FREE_OBJ(m_loading_error_confirm_popup);
	SAFE_FREE_OBJ(m_webicon);

	if (m_ewk_view) {
		_unregister_alive_callback();
		_delete_webview_managers();
		evas_object_del(m_ewk_view);
		m_ewk_view = NULL;
	}
}

void webview::delete_all_idler()
{
	if (m_sync_webview_idler) {
		ecore_idler_del(m_sync_webview_idler);
		m_sync_webview_idler = NULL;
	}

	if (m_load_started_idler) {
		ecore_idler_del(m_load_started_idler);
		m_load_started_idler = NULL;
	}

	if (m_load_committed_idler) {
		ecore_idler_del(m_load_committed_idler);
		m_load_committed_idler = NULL;
	}

	if (m_load_finished_idler) {
		ecore_timer_del(m_load_finished_idler);
		m_load_finished_idler = NULL;
	}

	if (m_load_finish_timer) {
		ecore_timer_del(m_load_finish_timer);
		m_load_finish_timer = NULL;
	}
}

void webview::_set_init_url(const char *url)
{
	BROWSER_SECURE_LOGD("url=[%s]", url);

	if (!url) {
		eina_stringshare_del(m_init_url);
		m_init_url = NULL;
	} else {
		if (!m_init_url)
			m_init_url = eina_stringshare_add(url);
		else
			eina_stringshare_replace(&m_init_url, url);
	}
	BROWSER_SECURE_LOGD("m_init_url=[%s]", m_init_url);
}

void webview::_set_url_for_bookmark_update(const char *url)
{
	BROWSER_SECURE_LOGD("url=[%s]", url);

	if (!url) {
		eina_stringshare_del(m_url_for_bookmark_update);
		m_url_for_bookmark_update = NULL;
	} else {
		if (!m_url_for_bookmark_update)
			m_url_for_bookmark_update = eina_stringshare_add(url);
		else
			eina_stringshare_replace(&m_url_for_bookmark_update, url);
	}
	BROWSER_SECURE_LOGD("m_url_for_bookmark_update=[%s]", m_url_for_bookmark_update);
}

void webview::delete_ewk_view(void)
{
	BROWSER_SECURE_LOGD("m_ewk_view=[%p], m_uri=[%s], get_uri()=[%s]", m_ewk_view, m_uri, get_uri());
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	if (get_uri())
		eina_stringshare_replace(&m_uri, get_uri());

	if (get_title())
		eina_stringshare_replace(&m_title, get_title());

	_unregister_alive_callback();
	_delete_webview_managers();
	evas_object_del(m_ewk_view);
	m_ewk_view = NULL;
}

void webview::orientation_send(int degree)
{
	BROWSER_LOGD("degree[%d]", degree);
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	if (degree == m_current_orientation)
		return;

	m_current_orientation = degree;

	if (degree == 270)
		ewk_view_orientation_send(m_ewk_view, -90);
	else
		ewk_view_orientation_send(m_ewk_view, degree);
}

static void __header_free_cb(void *data)
{
	free(data);
}

void webview::load_uri(const char *uri)
{
	RET_MSG_IF(!uri, "uri is NULL");

	m_browser->get_browser_view()->get_url_bar()->show_secure_icon(EINA_FALSE);

	if (!m_ewk_view)
		_create_ewk_view();

	std::string scheme_uri = std::string(uri);

	if (!is_supported_scheme(uri))
		scheme_uri = std::string(http_scheme) + scheme_uri;

	BROWSER_SECURE_LOGD("uri = [%s]", scheme_uri.c_str());

	if (m_browser->is_set_referer_header() == EINA_TRUE) {
		// Set referer header for external request.
		BROWSER_LOGD("referer header set");
		Eina_Hash *headers = eina_hash_string_superfast_new(__header_free_cb);
		eina_hash_add(headers, "Referer", strdup(m_browser->get_referer_header()));

		ewk_view_url_request_set(m_ewk_view, scheme_uri.c_str(), EWK_HTTP_METHOD_GET, headers, NULL);

		eina_hash_free(headers);
		m_browser->set_referer_header(NULL);
	} else
		ewk_view_url_set(m_ewk_view, scheme_uri.c_str());
	_set_init_url(scheme_uri.c_str());
	_set_url_for_bookmark_update(uri);
	m_browser->get_browser_view()->get_url_bar()->set_text(scheme_uri.c_str());
}

void webview::load_low_mem_uri(const char *reload_uri)
{
	BROWSER_LOGD("");

	m_browser->get_browser_view()->get_url_bar()->show_secure_icon(EINA_FALSE);

	if (!m_ewk_view)
		_create_ewk_view();

	_set_init_url(reload_uri);
}

void webview::load_stop(void)
{
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	// Hide progress bar.
	m_browser->get_browser_view()->get_url_bar()->set_loading_status(EINA_FALSE);

	ewk_view_stop(m_ewk_view);
}

void webview::reload(void)
{
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	ewk_view_reload(m_ewk_view);
}

Eina_Bool webview::is_empty_page_check(void)
{
	const char *uri = get_uri();
	if(!uri) {
		BROWSER_LOGE("uri is empty");
		return EINA_FALSE;
	}

	if (!strncmp("about:blank", uri, strlen("about:blank")))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void webview::reset_ewk_view(void)
{
	BROWSER_LOGD("");

	m_ewk_view = NULL;
	_delete_webview_managers();
}

const char *webview::get_uri(void)
{
	if (!m_ewk_view)
		return m_uri;

	return ewk_view_url_get(m_ewk_view);
}

const char *webview::get_title(void)
{
	if (!m_ewk_view)
		return m_title;

	return ewk_view_title_get(m_ewk_view);
}

void webview::update_uri(void)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	const char *uri = ewk_view_url_get(m_ewk_view);
	if (uri)
		eina_stringshare_replace(&m_uri, uri);
}

void webview::update_title(void)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	const char *title = ewk_view_title_get(m_ewk_view);
	if (title && strlen(title) > 0) {
		eina_stringshare_replace(&m_title, title);
		if (this == m_browser->get_browser_view()->get_current_webview()) {
			m_browser->get_browser_view()->get_url_bar()->set_text(title);
		}
	}
}

double webview::get_progress(void)
{
	RETV_MSG_IF(!m_ewk_view, 0.0, "m_ewk_view is NULL");

	//return ewk_view_load_progress_get(m_ewk_view);
	return m_progress;
}

Evas_Object *webview::get_favicon(void)
{
	RETV_MSG_IF(!m_ewk_view, NULL, "m_ewk_view is NULL");

	return ewk_context_icon_database_icon_object_add(
		ewk_view_context_get(m_ewk_view), get_uri(),
		evas_object_evas_get(m_ewk_view));
}

void webview::__download_website_icon_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	char *url = (char *)data;
	platform_service ps;
	SoupBuffer *body = soup_message_body_flatten(msg->response_body);

	int fd;
	int write_len = 0;

	if (!body->data || body->length <= 0) {
		soup_buffer_free(body);
		free(url);
		return;
	}

	ps.remove_file(PATH_WEBSITE_ICON_PNG);

	if ((fd = open(PATH_WEBSITE_ICON_PNG, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		soup_buffer_free(body);
		free(url);
		return;
	}
	write_len = write(fd, body->data, body->length);
	close(fd);
	soup_buffer_free(body);

	if (write_len != (int)body->length) {
		BROWSER_LOGE("icon download failed");
		ps.remove_file(PATH_WEBSITE_ICON_PNG);
		free(url);
		return;
	} else {
		if (ps.is_png(PATH_WEBSITE_ICON_PNG) == EINA_FALSE) {
			BROWSER_LOGE("downloaded icon is not a png file");
			ps.remove_file(PATH_WEBSITE_ICON_PNG);
			free(url);
			return;
		}
	}

	Evas_Object *webicon = evas_object_image_filled_add(evas_object_evas_get(m_window));

	if(!webicon) {
		BROWSER_LOGD("webicon is null");
		free(url);
		return;
	}

	evas_object_image_file_set(webicon, PATH_WEBSITE_ICON_PNG, NULL);
	m_browser->get_history()->set_history_webicon(url, webicon);

	int bookmark_id = -1;
	m_browser->get_bookmark()->get_id(url, &bookmark_id);
	if (bookmark_id >= 0)
		m_browser->get_bookmark()->set_webicon(bookmark_id, webicon);
	bookmark_id = -1;
	webview *current_webview = m_browser->get_browser_view()->get_current_webview();
	if (current_webview) {
		m_browser->get_bookmark()->get_id(current_webview->m_url_for_bookmark_update, &bookmark_id);
		if (bookmark_id >= 0)
			m_browser->get_bookmark()->set_webicon(bookmark_id, webicon);
	}

	evas_object_del(webicon);
	free(url);
}

Eina_Bool webview::_download_website_icon(const char *icon_data)
{
	BROWSER_SECURE_LOGD("icon_data[%s] is uri type, take a step to download icon with the uri", icon_data);
	SoupSession *soup_session = NULL;
	SoupMessage *soup_msg = NULL;
	SoupMessageHeaders *headers = NULL;

	soup_session = soup_session_async_new();
	g_object_set(soup_session, SOUP_SESSION_TIMEOUT, 15, (char *)NULL);

	const char *default_proxy_uri = "0.0.0.0";
	BROWSER_SECURE_LOGD("default_proxy_uri = [%s]", default_proxy_uri);
	if (default_proxy_uri) {
		std::string proxy_uri = std::string("http://") + std::string(default_proxy_uri);
		SoupURI *soup_uri = soup_uri_new(proxy_uri.c_str());
		g_object_set(soup_session, SOUP_SESSION_PROXY_URI, soup_uri, (char *)NULL);
		if (soup_uri)
			soup_uri_free(soup_uri);
	}
	BROWSER_SECURE_LOGD("request uri[%s]", icon_data);
	soup_msg = soup_message_new("GET", icon_data);
	if (!soup_msg)
		return EINA_FALSE;

	headers = soup_msg->request_headers;
	soup_message_headers_append(headers, "Accept", "image/png");
	soup_message_headers_append(headers, "User-Agent", user_agent_get());

	soup_session_queue_message(soup_session, soup_msg, __download_website_icon_finished_cb, (void *)strdup(get_uri()));

	return EINA_TRUE;
}

Eina_Bool webview::is_loading(void)
{
	RETV_MSG_IF(!m_ewk_view, EINA_FALSE, "m_ewk_view is NULL");

	if (get_progress() == 1.0f)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

void webview::backward(void)
{
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	m_is_backward = EINA_TRUE;
	ewk_view_back(m_ewk_view);
}

void webview::forward(void)
{
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	ewk_view_forward(m_ewk_view);
}

Eina_Bool webview::backward_possible(void)
{
	RETV_MSG_IF(!m_ewk_view, EINA_FALSE, "m_ewk_view is NULL");

	return ewk_view_back_possible(m_ewk_view);
}

Eina_Bool webview::forward_possible(void)
{
	RETV_MSG_IF(!m_ewk_view, EINA_FALSE, "m_ewk_view is NULL");

	return ewk_view_forward_possible(m_ewk_view);
}

void webview::suspend(void)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	ewk_view_suspend(m_ewk_view);

	m_is_suspended = EINA_TRUE;

	m_has_focus = evas_object_focus_get(m_ewk_view);
	BROWSER_LOGD("************ m_has_focus=%d", m_has_focus);
}

void webview::resume(void)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	ewk_view_resume(m_ewk_view);

	m_is_suspended = EINA_FALSE;
	m_new_window_count = 0;
}

void webview::set_page_visibility_state(Eina_Bool visible)
{
	BROWSER_LOGD("%d", visible);
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	ewk_view_visibility_set(m_ewk_view, visible);
}

Eina_Bool webview::__hide_progress_cb(void *data)
{
	BROWSER_LOGD("");

	m_browser->get_browser_view()->close_processing_popup();

	return ECORE_CALLBACK_CANCEL;
}

void webview::attach_event(const char *event, Evas_Smart_Cb func, const void *data)
{
	BROWSER_LOGD("");

	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	RET_MSG_IF(!event, "event is NULL");
	RET_MSG_IF(!func, "func is NULL");

	webview_callback *cb = (webview_callback *)malloc(sizeof(webview_callback));
	if (cb) {
		memset(cb, 0x00, sizeof(webview_callback));

		cb->event = eina_stringshare_add(event);
		cb->func = func;
		cb->func_data = (void *)data;
		m_event_callbacks = eina_list_append(m_event_callbacks, cb);
	}
}

void webview::detach_event(const char *event, Evas_Smart_Cb func)
{
	BROWSER_LOGD("");

	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	RET_MSG_IF(!event, "event is NULL");
	RET_MSG_IF(!func, "func is NULL");

	Eina_List *l;

	void *data;
	EINA_LIST_FOREACH(m_event_callbacks, l, data) {
		webview_callback *cb = (webview_callback *)data;
		if (cb->event && (!strcmp(cb->event, event)) && (cb->func == func)) {
			m_event_callbacks = eina_list_remove(m_event_callbacks, cb);
				eina_stringshare_del(cb->event);
			free(cb);

			break;
		}
	}
}

static Eina_Bool __webview_focus_cb(void *data)
{
	BROWSER_LOGD("");
	evas_object_focus_set((Evas_Object *)data, EINA_TRUE);
	return ECORE_CALLBACK_CANCEL;
}

void webview::activate(void)
{
	BROWSER_LOGD("");

	Eina_Bool is_deleted_ewk_view = EINA_FALSE;
	if (!m_ewk_view) {
		_create_ewk_view();
		is_deleted_ewk_view = EINA_TRUE;
	}

	BROWSER_LOGD("******** has_focus()=%d", has_focus());
	if (has_focus())
		ecore_timer_add(0.2, __webview_focus_cb, m_ewk_view);

	if (m_already_activated) {
		BROWSER_LOGD("already activated");
		return;
	}

	evas_object_smart_callback_add(m_ewk_view, "load,started", __load_started_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "load,committed", __load_committed_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "load,error", __load_error_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "url,changed", __url_changed_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "title,changed", __title_changed_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "load,progress", __load_progress_cb, this);
	//evas_object_smart_callback_add(m_ewk_view, "load,finished", __load_finished_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "process,crashed", __process_crashed_cb, this);

	//evas_object_smart_callback_add(m_ewk_view, "create,window", __create_window_cb, this);
	//evas_object_smart_callback_add(m_ewk_view, "close,window", __close_window_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "icon,received", __icon_received_cb, this);

	//evas_object_smart_callback_add(m_ewk_view, "policy,navigation,decide", __policy_navigation_decide_cb, this);
	//evas_object_smart_callback_add(m_ewk_view, "policy,response,decide", __policy_response_decide_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "form,submit", __form_submit_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "back,forward,list,changed", __back_forward_list_changed_cb, this);

	// the "rotate,prepared" should be received even during background.
	evas_object_smart_callback_del(m_ewk_view, "rotate,prepared", __rotate_prepared_cb);
	evas_object_smart_callback_add(m_ewk_view, "rotate,prepared", __rotate_prepared_cb, this);
	evas_object_event_callback_add(m_ewk_view, EVAS_CALLBACK_MOUSE_DOWN, __mouse_down_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "certificate,pem,set", __set_certificate_data_cb, this);

	evas_object_smart_callback_add(m_ewk_view, "urlbar,scroll", __urlbar_scroll_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "flick,canceled", __flick_canceled_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "fullscreen,enterfullscreen", __fullscreen_enter_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "fullscreen,exitfullscreen", __fullscreen_exit_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "usermedia,permission,request", __usermedia_permission_request_cb, this);

	ewk_context_did_start_download_callback_set(ewk_context_default_get(), __download_request_cb, this);

	enable_customize_contextmenu(EINA_TRUE);

	Eina_List *l;
	void *data;
	EINA_LIST_FOREACH(m_event_callbacks, l, data) {
		webview_callback *cb = (webview_callback *)data;
		//BROWSER_LOGD("cb->event=[%s]", cb->event);
		evas_object_smart_callback_add(m_ewk_view, cb->event, cb->func, cb->func_data);
	}

	resume();

	m_already_activated = EINA_TRUE;

	BROWSER_SECURE_LOGD("********** is_deleted_ewk_view=[%d], m_uri=[%s]", is_deleted_ewk_view, m_uri);
	if (is_deleted_ewk_view)
		load_uri(m_uri);
}

void webview::deactivate(void)
{
	BROWSER_LOGD("");

	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	if (!m_already_activated) {
		BROWSER_LOGD("already deactivated");
		return;
	}

	if (m_browser->get_full_screen_enable()) {
		exit_fullscreen_mode();
		m_browser->set_full_screen_enable(EINA_FALSE);
	}

	suspend();
	m_certi_manager->destroy_certificate_status_popup();

	evas_object_smart_callback_del(m_ewk_view, "load,started", __load_started_cb);
	evas_object_smart_callback_del(m_ewk_view, "load,committed", __load_committed_cb);
	evas_object_smart_callback_del(m_ewk_view, "load,error", __load_error_cb);
	evas_object_smart_callback_del(m_ewk_view, "url,changed", __url_changed_cb);
	evas_object_smart_callback_del(m_ewk_view, "title,changed", __title_changed_cb);
	evas_object_smart_callback_del(m_ewk_view, "load,progress", __load_progress_cb);
	//evas_object_smart_callback_del(m_ewk_view, "load,finished", __load_finished_cb);

	evas_object_smart_callback_del(m_ewk_view, "process,crashed", __process_crashed_cb);

	//evas_object_smart_callback_del(m_ewk_view, "create,window", __create_window_cb);
	//evas_object_smart_callback_del(m_ewk_view, "close,window", __close_window_cb);

	evas_object_smart_callback_del(m_ewk_view, "icon,received", __icon_received_cb);

	//evas_object_smart_callback_del(m_ewk_view, "policy,navigation,decide", __policy_navigation_decide_cb);
	//evas_object_smart_callback_del(m_ewk_view, "policy,response,decide", __policy_response_decide_cb);

	evas_object_smart_callback_del(m_ewk_view, "form,submit", __form_submit_cb);
	evas_object_smart_callback_del(m_ewk_view, "back,forward,list,changed", __back_forward_list_changed_cb);
	evas_object_event_callback_del(m_ewk_view, EVAS_CALLBACK_MOUSE_DOWN, __mouse_down_cb);

	evas_object_smart_callback_del(m_ewk_view, "certificate,pem,set", __set_certificate_data_cb);

	evas_object_smart_callback_del(m_ewk_view, "urlbar,scroll", __urlbar_scroll_cb);
	evas_object_smart_callback_del(m_ewk_view, "flick,canceled", __flick_canceled_cb);
	evas_object_smart_callback_del(m_ewk_view, "fullscreen,enterfullscreen", __fullscreen_enter_cb);
	evas_object_smart_callback_del(m_ewk_view, "fullscreen,exitfullscreen", __fullscreen_exit_cb);
	evas_object_smart_callback_del(m_ewk_view, "usermedia,permission,request", __usermedia_permission_request_cb);

	enable_customize_contextmenu(EINA_FALSE);

	Eina_List *l;
	void *data;
	EINA_LIST_FOREACH(m_event_callbacks, l, data) {
		webview_callback *cb = (webview_callback *)data;
		if (cb) {
			//BROWSER_LOGD("cb->event=[%s]", cb->event);
			evas_object_smart_callback_del(m_ewk_view, cb->event, cb->func);
		}
	}


	m_already_activated = EINA_FALSE;

//	m_has_focus = evas_object_focus_get(m_ewk_view);
//	BROWSER_LOGD("************ m_has_focus=%d", m_has_focus);
}

void webview::enable_customize_contextmenu(Eina_Bool enable)
{
	if (enable) {
		evas_object_smart_callback_add(m_ewk_view, "contextmenu,customize", __contextmenu_customize_cb, this);
		evas_object_smart_callback_add(m_ewk_view, "contextmenu,selected", __contextmenu_selected_cb, this);
	} else {
		evas_object_smart_callback_del(m_ewk_view, "contextmenu,customize", __contextmenu_customize_cb);
		evas_object_smart_callback_del(m_ewk_view, "contextmenu,selected", __contextmenu_selected_cb);
	}
}

void webview::auto_fitting_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_auto_fitting_set(settings, enable);
}

void webview::javascript_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_javascript_enabled_set(settings, enable);
}

void webview::auto_load_images_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);
	ewk_settings_loads_images_automatically_set(settings, enable);
}

void webview::auto_fill_forms_set(Eina_Bool enabled)
{
	BROWSER_LOGD("enabled = [%d]", enabled);

	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	Ewk_Settings *setting = ewk_view_settings_get(m_ewk_view);

	ewk_settings_form_profile_data_enabled_set(setting, enabled);
}

void webview::save_ID_and_PW_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);

	ewk_settings_autofill_password_form_enabled_set(settings, enable);
}

void webview::remember_form_data_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	Ewk_Settings *settings = ewk_view_settings_get(m_ewk_view);

	ewk_settings_form_candidate_data_enabled_set(settings, enable);
}

void webview::user_agent_set(const char *user_agent)
{
	BROWSER_SECURE_LOGD("user agent = [%s]", user_agent);

	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	ewk_view_user_agent_set(m_ewk_view, user_agent);
}

const char *webview::user_agent_get(void)
{
	RETV_MSG_IF(!m_ewk_view, NULL, "m_ewk_view is NULL");
	return ewk_view_user_agent_get(m_ewk_view);
}

void webview::get_geometry(int *x, int *y, int *w, int *h)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	evas_object_geometry_get(m_ewk_view, x, y, w, h);
}

void webview::motion_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	evas_object_smart_callback_call(m_ewk_view, "motion,enable", (void*)&enable);
}

void webview::scroll_position_get(int *x, int *y)
{
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	ewk_view_scroll_pos_get(m_ewk_view, x, y);
}

void webview::scroll_position_set(int x, int y)
{
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	ewk_view_scroll_set(m_ewk_view, x, y);
}

double webview::scale_get(void)
{
	RETV_MSG_IF(!m_ewk_view, 0.0, "m_ewk_view is NULL");

	return ewk_view_scale_get(m_ewk_view);
}

void webview::content_size_get(int *w, int *h)
{
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	ewk_view_contents_size_get(m_ewk_view, w, h);
}

void webview::give_focus(Eina_Bool focus)
{
	if (m_ewk_view)
		evas_object_focus_set(m_ewk_view, focus);
}

void webview::update_settings(void)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	auto_fitting_enabled_set(m_preference->get_open_pages_in_overview_enabled());
	set_url_bar_shown(get_url_bar_shown());

	javascript_enabled_set(m_preference->get_javascript_enabled());
	auto_load_images_enabled_set(m_preference->get_display_images_enabled());

	auto_fill_forms_set(m_preference->get_auto_fill_forms_enabled());
	save_ID_and_PW_enabled_set(m_preference->get_remember_passwords_enabled());
	remember_form_data_enabled_set(m_preference->get_remember_form_data_enabled());
}

void webview::_init_settings(void)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	Ewk_Context *ewk_context = ewk_view_context_get(m_ewk_view);
	if (ewk_context != NULL) {
		ewk_context_cache_model_set(ewk_context, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);
		accept_cookie_enabled_set(get_ewk_view(), m_preference->get_accept_cookies_enabled());
	}

	_register_alive_callback();
	_create_webview_managers();

	update_settings();

	uses_keypad_without_user_action_set(EINA_FALSE);
	ewk_view_application_name_for_user_agent_set(m_ewk_view, APPLICATION_NAME_FOR_USER_AGENT);

	const char *user_agent = NULL;
	user_agent = m_browser->get_user_agent_manager()->get_user_agent();
	BROWSER_SECURE_LOGD("[developer mode] user agent = [%s]", user_agent);
}

void webview::_create_webview_managers(void)
{
	BROWSER_LOGD("");

	m_html5_manager = new html5_feature_manager(this);
	m_certi_manager = new certificate_manager(this);
}

void webview::_delete_webview_managers(void)
{
	BROWSER_LOGD("");

	delete m_html5_manager;
	m_html5_manager = NULL;

	delete m_certi_manager;
	m_certi_manager = NULL;
}

void webview::_register_alive_callback(void)
{
	BROWSER_LOGD("");
	// Create window cb can be called many times. So, create window cb should not be unregistered when it deactivated.
	evas_object_smart_callback_add(m_ewk_view, "create,window", __create_window_cb, this);

	// Close window cb can be called from another window through JS. close window cb should not be unregistered when it is deactivated.
	evas_object_smart_callback_add(m_ewk_view, "close,window", __close_window_cb, this);

	// Should be checked during suspended in case of A tag with non-blank target.
	evas_object_smart_callback_add(m_ewk_view, "policy,navigation,decide", __policy_navigation_decide_cb, this);

	// It should not be unregistered when switching tabs, because the page/tab may be waiting for response from network.
	evas_object_smart_callback_add(m_ewk_view, "policy,response,decide", __policy_response_decide_cb, this);

	// It should not be unregistered when switching tabs, because history needs to be updated on load finished
	evas_object_smart_callback_add(m_ewk_view, "load,finished", __load_finished_cb, this);
	evas_object_smart_callback_add(m_ewk_view, "load,nonemptylayout,finished", __load_nonempty_layout_finished_cb,this);
}

void webview::_unregister_alive_callback(void)
{
	evas_object_smart_callback_del(m_ewk_view, "create,window", __create_window_cb);
	evas_object_smart_callback_del(m_ewk_view, "policy,navigation,decide", __policy_navigation_decide_cb);
	evas_object_smart_callback_del(m_ewk_view, "policy,response,decide", __policy_response_decide_cb);
	evas_object_smart_callback_del(m_ewk_view, "load,finished", __load_finished_cb);
	evas_object_smart_callback_del(m_ewk_view, "load,nonemptylayout,finished", __load_nonempty_layout_finished_cb);
}

void webview::private_browsing_enabled_set(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	m_incognito = enable;

	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	Ewk_Settings *setting = ewk_view_settings_get(m_ewk_view);
	ewk_settings_private_browsing_enabled_set(setting, enable);
}

Eina_Bool webview::private_browsing_enabled_get(void)
{
//	RETV_MSG_IF(!m_ewk_view, EINA_FALSE, "m_ewk_view is NULL");

	return m_incognito;

//	Ewk_Settings *setting = ewk_view_settings_get(m_ewk_view);
//	return ewk_settings_private_browsing_enabled_get(setting);
}


void webview::uses_keypad_without_user_action_set(Eina_Bool enabled)
{
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	Ewk_Settings *setting = ewk_view_settings_get(m_ewk_view);
	ewk_settings_uses_keypad_without_user_action_set(setting, enabled);
}

void webview::find_word(const char *word, Eina_Bool forward, Evas_Smart_Cb found_cb, void *data)
{
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");
	RET_MSG_IF(!word, "word is NULL");

	detach_event("text,found", found_cb);
	attach_event("text,found", found_cb, data);

	// If the some events was attached during working time,
	// the webview should be added smart event.
	evas_object_smart_callback_del(m_ewk_view, "text,found", found_cb);
	evas_object_smart_callback_add(m_ewk_view, "text,found", found_cb, data);

	Ewk_Find_Options find_option = (Ewk_Find_Options)(EWK_FIND_OPTIONS_CASE_INSENSITIVE | EWK_FIND_OPTIONS_WRAP_AROUND
			| EWK_FIND_OPTIONS_SHOW_FIND_INDICATOR | EWK_FIND_OPTIONS_SHOW_HIGHLIGHT);

	if (!forward)
		find_option = (Ewk_Find_Options)(find_option | EWK_FIND_OPTIONS_BACKWARDS);

	ewk_view_text_find(m_ewk_view, word, find_option, FIND_WORD_MAX_COUNT);
}

void webview::exit_fullscreen_mode(void)
{
	RET_MSG_IF(!m_ewk_view, "m_ewk_view is NULL");

	BROWSER_LOGD("");

	ewk_view_fullscreen_exit(m_ewk_view);
}

int webview::get_modified(void)
{
	int date = 0;

	bp_tab_adaptor_get_date_modified(m_sync_id, &date);

	return date;
}

void webview::set_request_uri(const char *uri)
{
	BROWSER_SECURE_LOGD("m_request_uri: %s, uri: %s", m_request_uri, uri);
	if (m_request_uri) {
		free(m_request_uri);
		m_request_uri = NULL;
	}

	if (uri)
		m_request_uri = strdup(uri);
}

char *webview::get_request_uri(void)
{
	BROWSER_SECURE_LOGD("m_request_uri: %s", m_request_uri);
	return m_request_uri;
}

void webview::__rotate_prepared_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *window = m_browser->get_browser_view()->get_window();

	if (m_browser->get_full_screen_enable() && elm_win_wm_rotation_manual_rotation_done_get(window))
		elm_win_wm_rotation_manual_rotation_done(window);
}

void webview::__load_started_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	webview *wv = (webview *)data;
	if(m_browser->get_developer_mode() == EINA_TRUE){
		const char *user_agent = NULL;
		user_agent = m_browser->get_user_agent_manager()->get_user_agent();
		wv->user_agent_set(user_agent);
	}
	BROWSER_LOGI("Page load started, URI: %s", wv->get_uri());

	/* const char *uri = wv->get_init_url();
	if (uri && strlen(uri) >= strlen(file_scheme) && strncmp(uri, file_scheme, strlen(file_scheme))) {
		// Check network availability if it's not file scheme.
		if (m_browser->get_network_manager()->get_network_connected() == EINA_FALSE) {
			m_browser->get_network_manager()->check_network_use_policy();
		}
	} */

	wv->_init_bookmark_thumbnail();
	wv->m_progress = 0.0;

	// Return fast to make WebKit keep going.
	if (wv->m_load_started_idler)
		ecore_idler_del(wv->m_load_started_idler);
	wv->m_load_started_idler = ecore_idler_add(__load_started_timer_cb, data);
}

Eina_Bool webview::__load_started_timer_cb(void *data)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	wv->m_load_started_idler = NULL;
	wv->get_certificate_manager()->set_cert_type(certificate_manager::NONE);
	browser_view *bv = m_browser->get_browser_view();

	// Show progress bar if webview is current.
	if (wv == bv->get_current_webview())
		bv->get_url_bar()->set_loading_status(EINA_TRUE);

	// Save tab information if it's new tab.
	if (wv->m_sync_id == -1) {
		webview_info *wvi = new webview_info(wv, EINA_TRUE); // Can not use snapshot info this time.
		m_browser->get_cloud_sync_manager()->sync_webview(wvi);
	}

	// Hide find on page.
	if (bv->is_show_find_on_page())
		bv->show_url_bar();

	// Check active webview.
	unsigned int count = m_browser->get_webview_list()->get_active_count();
	if (count > BROWSER_WINDOW_MAX_ACTIVE_SIZE)
		m_browser->get_webview_list()->deactivate_old_webview();

	return ECORE_CALLBACK_CANCEL;
}

void webview::__load_committed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	// Return fast to make WebKit keep going.
	wv->m_load_committed_idler = ecore_idler_add(__load_committed_timer_cb, data);
}

Eina_Bool webview::__load_committed_timer_cb(void *data)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	wv->m_load_committed_idler = NULL;
	const char *uri = wv->get_uri();

	// Need to set init uri after backward.
	if (wv->m_is_backward) {
		BROWSER_LOGD("Backward");
		wv->_set_init_url(uri);
		// Url for bookmark update has to be properly updated when new url is loaded
		wv->_set_url_for_bookmark_update(uri);
		wv->m_is_backward = EINA_FALSE;
	}

	return ECORE_CALLBACK_CANCEL;
}

void webview::__load_error_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Ewk_Error *error = (Ewk_Error *)event_info;
	int error_code = ewk_error_code_get(error);

	webview *wv = (webview *)data;
	wv->m_is_error_page = EINA_TRUE;

	if (error_code == EWK_ERROR_CODE_BAD_URL) {
		SAFE_FREE_OBJ(wv->m_loading_error_confirm_popup);
		wv->m_browser->get_browser_view()->show_msg_popup("IDS_BR_HEADER_DATA_CONNECTIVITY_PROBLEM", "IDS_BR_BODY_UNABLE_TO_DISPLAY_THE_WEBPAGE_NO_RESPONSE_FROM_THE_SERVER");
	}

	if (wv == m_browser->get_browser_view()->get_current_webview())
		m_browser->get_browser_view()->get_url_bar()->set_loading_status(EINA_FALSE);
}

void webview::__load_nonempty_layout_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	// Give focus on ewk view.
	if (!m_browser->is_tts_enabled() && !m_browser->get_browser_view()->is_show_url_input_bar())
		m_browser->get_browser_view()->set_focus_on_content();
}

void webview::__url_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	wv->update_uri();

	if (m_browser->is_tts_enabled()) {
		m_browser->get_browser_view()->get_url_bar()->set_text(wv->get_uri(), EINA_FALSE);
		m_browser->get_browser_view()->get_url_bar()->set_highlight_on_text();
	}
	wv->update_settings();
}

void webview::__title_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	wv->update_title();
}

void webview::__load_progress_cb(void *data, Evas_Object *obj, void *event_info)
{
	RET_MSG_IF(!data, "data is NULL");

	webview *wv = (webview *)data;
	wv->m_progress = *(double *)event_info;

	// Return fast to make WebKit keep going.
	ecore_idler_add(__load_progress_timer_cb, data);
}

Eina_Bool webview::__load_progress_timer_cb(void *data)
{
	webview *wv = (webview *)data;

	if (wv == m_browser->get_browser_view()->get_current_webview())
		m_browser->get_browser_view()->get_progress_bar()->update_progress(wv->m_progress);

	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool webview::__finish_timer_idler_cb(void *data)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL, "data is NULL");

	webview *wv = (webview *)data;
	wv->m_load_finish_timer = NULL;

	browser_view *bv = m_browser->get_browser_view();

	if (wv == bv->get_current_webview() && (bv->get_view_mode() == browser_view_mode_normal)) {
		// Update url bar text.
		if (wv->m_status_code == HTTP_NO_CONTENT) {		// if there is no content then reset URL to prevent from address spoofing
			bv->get_url_bar()->set_text(EMPTY_URL, EINA_FALSE);
			ewk_view_url_set(wv->get_ewk_view(), EMPTY_URL);
			wv->update_uri();
		} else {
			if (wv->get_title() != NULL && strlen(wv->get_title()) > 0)
				bv->get_url_bar()->set_text(wv->get_title());
			else
				bv->get_url_bar()->set_text(wv->get_uri());
			wv->update_uri();
			wv->update_title();
		}
	}
	BROWSER_LOGD("wv[%p], cur_Wv[%p]", wv, bv->get_current_webview());

	// Save tab info into tab DB.
	wv->m_sync_webview_idler = ecore_idler_add(__sync_webview_cb, wv);

	Evas_Object *favicon = NULL;
	if (wv != bv->get_current_webview()) {
		const char *current_uri = wv->get_uri();
		const char *page_title = wv->get_title();
		if (!wv->private_browsing_enabled_get() && !wv->is_data_scheme()) {
			favicon = wv->get_favicon();
			if(page_title && strlen(page_title)) {
				m_browser->get_history()->save_history(page_title, current_uri, wv->m_thumbnail, favicon, NULL);
				if (m_browser->get_history()->get_memory_full())
					bv->show_noti_popup(BR_STRING_DISK_FULL);
			} else {
				m_browser->get_history()->save_history(wv->get_uri(), current_uri, wv->m_thumbnail, favicon, NULL);
				if (m_browser->get_history()->get_memory_full())
					bv->show_noti_popup(BR_STRING_DISK_FULL);

			}
		}

		/* Update favicon to Bookmark DB if there are same URLs */
		int bookmark_id = -1;
		m_browser->get_bookmark()->get_id(current_uri, &bookmark_id);
		if (bookmark_id >= 0) {
			wv->_set_bookmark_favicon(data, bookmark_id, EINA_FALSE);
		}
		/* Also Update favicon to Bookmark DB if there are same redirected URLs */
		if (wv->m_url_for_bookmark_update && strcmp(current_uri, wv->m_url_for_bookmark_update)) {
			bookmark_id = -1;
			m_browser->get_bookmark()->get_id(wv->m_url_for_bookmark_update, &bookmark_id);
			if (bookmark_id >= 0) {
				m_browser->get_bookmark()->increase_access_count(bookmark_id);
				wv->_set_bookmark_favicon(data, bookmark_id, EINA_TRUE);
			}
		}
		SAFE_FREE_OBJ(favicon);
		//Delete the snapshot after the thumbnail is captured, as its nowhere needed
		SAFE_FREE_OBJ(wv->m_snapshot);

		return ECORE_CALLBACK_CANCEL;
	}

	// If private mode, do not save the history.
	int visit_count = 0;
	const char *current_uri = wv->get_uri();
#ifdef ENABLE_INCOGNITO_WINDOW
	if (!wv->is_incognito() && !wv->is_data_scheme()) {
#else
	if (!wv->private_browsing_enabled_get() && !wv->is_data_scheme()) {
#endif
		Evas_Object *favicon = wv->get_favicon();
		if(wv->get_title() && strlen(wv->get_title())) {
			m_browser->get_history()->save_history(wv->get_title(), current_uri, wv->m_thumbnail, favicon, &visit_count);
			if (m_browser->get_history()->get_memory_full())
				bv->show_noti_popup(BR_STRING_DISK_FULL);
		} else {
			m_browser->get_history()->save_history(wv->get_uri(), current_uri, wv->m_thumbnail, favicon, &visit_count);
			if (m_browser->get_history()->get_memory_full())
				bv->show_noti_popup(BR_STRING_DISK_FULL);
		}

		BROWSER_LOGD("visit_count: [%d]", visit_count);
		SAFE_FREE_OBJ(favicon);
	}

	/* Update favicon to Bookmark DB if there are same URLs */
	int bookmark_id = -1;
	m_browser->get_bookmark()->get_id(current_uri, &bookmark_id);
	if (bookmark_id >= 0) {
		wv->_set_bookmark_favicon(data, bookmark_id, EINA_FALSE);
	}
	/* Also Update favicon to Bookmark DB if there are same redirected URLs */
	bookmark_id = -1;
	m_browser->get_bookmark()->get_id(wv->m_url_for_bookmark_update, &bookmark_id);
	if (bookmark_id >= 0) {
		m_browser->get_bookmark()->increase_access_count(bookmark_id);
		wv->_set_bookmark_favicon(data, bookmark_id, EINA_TRUE);
	}

	// Check active webview.
	unsigned int count = m_browser->get_webview_list()->get_active_count();
	BROWSER_LOGD("active webview count: %d", count);
	if (count > BROWSER_WINDOW_MAX_ACTIVE_SIZE)
		m_browser->get_webview_list()->deactivate_old_webview();

	return ECORE_CALLBACK_CANCEL;
}

void webview::__download_request_cb(const char *download_uri, void *data)
{
	BROWSER_SECURE_LOGD("download_uri = [%s]", download_uri);
	RET_MSG_IF(!download_uri, "download_uri is NULL");

	if (!strncmp(download_uri, data_scheme, strlen(data_scheme))){
		m_browser->get_browser_view()->show_noti_popup(BR_STRING_DOWNLOADING_ING);
		if (m_browser->get_download_manager()->handle_data_scheme(download_uri) == EINA_TRUE)
			m_browser->get_browser_view()->show_noti_popup(BR_STRING_SAVED);
		else
			m_browser->get_browser_view()->show_noti_popup(BR_STRING_FAILED);
	} else if (strncmp(download_uri, http_scheme, strlen(http_scheme)) && strncmp(download_uri, https_scheme, strlen(https_scheme))) {
		BROWSER_LOGE("Only http or https URLs can be downloaded");
		m_browser->get_browser_view()->show_noti_popup(BR_STRING_HTTP_URL_CAN_BE_DOWNLOADED);
		return;
	} else {
		m_browser->get_download_manager()->launch_download_app(download_uri);
	}
}

void webview::__popup_close(webview *wv)
{
	BROWSER_LOGD("");

	if(wv->m_permission_info.popup) {
		evas_object_focus_set(wv->m_window, EINA_TRUE);
		evas_object_hide(wv->m_permission_info.popup);
		wv->m_browser->get_browser_view()->destroy_msg_popup();
		wv->m_permission_info.popup = NULL;
	}
}

void webview::__usermedia_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	webview *wv = static_cast<webview *>(data);
	ewk_user_media_permission_request_set(wv->m_permission_info.permission_request, EINA_FALSE);
	__popup_close(wv);
}

void webview::__usermedia_permission_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	webview *wv = static_cast<webview *>(data);
	if (wv->m_permission_info.popup)
		__popup_close(wv);

	Ewk_User_Media_Permission_Request* permission_request =
		static_cast<Ewk_User_Media_Permission_Request *>(event_info);

	const char *host_uri = wv->get_uri();
	host_uri = ( host_uri == NULL || strlen(host_uri) == 0 ? "NULL" : host_uri);

	unsigned short prefix_len = strlen(BR_STRING_REQUEST_WEBPAGE_PREFIX);
	unsigned short base_uri_len = strlen(host_uri);
	unsigned short msg_len = strlen(BR_STRING_USER_MEDIA_PERMISSION_POPUP_DESC);

	char *msg = new char[prefix_len + base_uri_len + msg_len + 1];
	snprintf(msg, (prefix_len + msg_len + base_uri_len),
			BR_STRING_USER_MEDIA_PERMISSION_POPUP_DESC,
			BR_STRING_REQUEST_WEBPAGE_PREFIX,
			host_uri + strlen(http_scheme));

	wv->m_permission_info.popup = m_browser->get_browser_view()->show_msg_popup(
			BR_STRING_USER_MEDIA_PERMISSION_POPUP_TITLE,
			msg, NULL,
			BR_STRING_OK, __usermedia_ok_cb,
			BR_STRING_CANCEL, __usermedia_cancel_cb,
			wv,
			NULL, NULL);

	ewk_user_media_permission_request_suspend(permission_request);
	wv->m_permission_info.permission_request = permission_request;
	delete [] msg;
}

void webview::__usermedia_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	webview *wv = static_cast<webview *>(data);
	ewk_user_media_permission_request_set(wv->m_permission_info.permission_request, EINA_TRUE);
	__popup_close(wv);
}

Eina_Bool webview::__sync_webview_cb(void *data)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	wv->m_sync_webview_idler = NULL;
	if (!m_browser->get_webview_list()->is_webview_exist(wv))
		return ECORE_CALLBACK_CANCEL;

	browser_view *bv = m_browser->get_browser_view();

	webview_info *wvi = NULL;
	if (wv != bv->get_current_webview())
		wvi = new webview_info(wv);
	else
		wvi = new webview_info(wv, EINA_TRUE); //init without snapshot

	m_browser->get_cloud_sync_manager()->sync_webview(wvi);

	return ECORE_CALLBACK_CANCEL;
}

void webview::_init_bookmark_thumbnail(void)
{
	BROWSER_LOGD("");
	SAFE_FREE_OBJ(m_thumbnail);
}

void webview::_set_bookmark_favicon(void *data, int bookmark_id, Eina_Bool is_update_URI)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	Evas_Object *favicon = NULL;

	if (is_update_URI == EINA_TRUE) {
		favicon = m_browser->get_history()->get_history_favicon(wv->get_uri());
	} else {
		favicon = wv->get_favicon();
	}
	m_browser->get_bookmark()->set_favicon(bookmark_id, favicon);

	SAFE_FREE_OBJ(favicon);
}

void webview::_set_bookmark_thumbnail(void *data, int bookmark_id, Eina_Bool is_update_URI)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	Evas_Object *thumbnail = NULL;

	if (is_update_URI == EINA_TRUE) {
		thumbnail = m_browser->get_history()->get_snapshot(wv->get_uri());
	} else {
		const char *evas_obj_type = evas_object_type_get(wv->m_thumbnail);
		if (evas_obj_type && !strcmp(evas_obj_type, "image"))
			thumbnail = wv->m_thumbnail;
		//Rare error condition - if snapshot is not available try to get the snapshot from history
		else
			thumbnail = m_browser->get_history()->get_snapshot(wv->get_uri());
	}
	m_browser->get_bookmark()->set_thumbnail(bookmark_id, thumbnail);

	if (thumbnail && wv->m_thumbnail != thumbnail) {
		SAFE_FREE_OBJ(thumbnail);
	}
}

void webview::_set_bookmark_webicon(void *data, int bookmark_id, Eina_Bool is_update_URI)
{
	BROWSER_LOGD("");

	RET_MSG_IF(!data, "data is NULL");
	webview *wv = (webview *)data;
	Evas_Object *webicon = NULL;
	if (is_update_URI == EINA_TRUE)
		webicon = m_browser->get_history()->get_history_webicon(wv->get_uri());
	else
		evas_object_image_file_set(webicon, PATH_WEBSITE_ICON_PNG, NULL);

	if(webicon) {
		m_browser->get_bookmark()->set_webicon(bookmark_id, webicon);
		evas_object_del(webicon);
	}
}

void webview::_show_context_menu_text_only(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("");

	unsigned short count = ewk_context_menu_item_count(menu);
	RET_MSG_IF(count == 0, "There is no context menu for text only");

	const char *selected_text = ewk_view_text_selection_text_get(m_ewk_view);
	Eina_Bool text_selected = EINA_FALSE;
	if (selected_text && strlen(selected_text) > 0)
		text_selected = EINA_TRUE;

	Ewk_Context_Menu_Item_Tag *original_ctx_item = NULL;
	original_ctx_item = (Ewk_Context_Menu_Item_Tag *)malloc(sizeof(Ewk_Context_Menu_Item_Tag) * count);
	RET_MSG_IF(original_ctx_item == NULL, "Failed to allocate memory for ctx menu reordering");

	memset(original_ctx_item, 0x00, sizeof(Ewk_Context_Menu_Item_Tag) * count);

	for (unsigned short i = 0 ; i < count ; i++) {
		Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
		Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

		original_ctx_item[i] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	/* Select all */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL, BR_STRING_CTXMENU_SELECT_ALL, true);
			break;
		}
	}

	/* Copy */
	if (text_selected == EINA_TRUE) {
		for (unsigned short i = 0 ; i < count ; i++) {
			if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY) {
				ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY, BR_STRING_CTXMENU_COPY, true);
				break;
			}
		}
	}

	/* Share */
	if (text_selected == EINA_TRUE)
		ewk_context_menu_item_append(menu, CUSTOM_CONTEXT_MENU_ITEM_SHARE, BR_STRING_SHARE, browser_img_dir"/I01_ctx_popup_icon_share.png", true);

	if (text_selected == EINA_TRUE)
	ewk_context_menu_item_append(menu, CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_PAGE, BR_STRING_CTXMENU_FIND, browser_img_dir"/I01_ctx_popup_icon_find_on_page.png", true);

	/* Search */
	if (text_selected == EINA_TRUE) {
		for (unsigned short i = 0 ; i < count ; i++) {
			if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB) {
				ewk_context_menu_item_append(menu, CUSTOM_CONTEXT_MENU_ITEM_SEARCH, BR_STRING_CTXMENU_WEB_SEARCH, browser_img_dir"/I01_ctx_popup_icon_web_search.png", true);
				break;
			}
		}
	}

	/* Translator */
	if (text_selected == EINA_TRUE) {
		for (unsigned short i = 0 ; i < count ; i++) {
			if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_TRANSLATE) {
				ewk_context_menu_item_append(menu, EWK_CONTEXT_MENU_ITEM_TAG_TRANSLATE, BR_STRING_CTXMENU_TRANSLATE, browser_img_dir"/I01_ctx_popup_icon_translate.png", true);
				break;
			}
		}
	}

	free(original_ctx_item);
}

void webview::_show_context_menu_text_link(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("");

	unsigned short count = ewk_context_menu_item_count(menu);
	RET_MSG_IF(count == 0, "There is no context menu for text only");

	Ewk_Context_Menu_Item_Tag *original_ctx_item = NULL;
	original_ctx_item = (Ewk_Context_Menu_Item_Tag *)malloc(sizeof(Ewk_Context_Menu_Item_Tag) * count);
	RET_MSG_IF(original_ctx_item == NULL, "Failed to allocate memory for ctx menu reordering");

	memset(original_ctx_item, 0x00, sizeof(Ewk_Context_Menu_Item_Tag) * count);

	for (unsigned short i = 0 ; i < count ; i++) {
		Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
		Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

		original_ctx_item[i] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	/* Open in new window */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW, true);
			break;
		}
	}

	/* Save link */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK, BR_STRING_CTXMENU_SAVE_LINK, true);
			break;
		}
	}

	/* Copy link */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, BR_STRING_CTXMENU_COPY_LINK, true);
			break;
		}
	}

	/* Selection mode */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE, BR_STRING_CTXMENU_SELECT_TEXT, true);
			break;
		}
	}

	free(original_ctx_item);
}

void webview::_show_context_menu_image_only(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("");

	unsigned short count = ewk_context_menu_item_count(menu);
	RET_MSG_IF(count == 0, "There is no context menu for image only");

	Ewk_Context_Menu_Item_Tag *original_ctx_item = NULL;
	original_ctx_item = (Ewk_Context_Menu_Item_Tag *)malloc(sizeof(Ewk_Context_Menu_Item_Tag) * count);
	RET_MSG_IF(original_ctx_item == NULL, "Failed to allocate memory for ctx menu reordering");

	memset(original_ctx_item, 0x00, sizeof(Ewk_Context_Menu_Item_Tag) * count);

	for (unsigned short i = 0 ; i < count ; i++) {
		Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
		Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

		original_ctx_item[i] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	/* Save image */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK, BR_STRING_CTXMENU_SAVE_IMAGE, true);
			break;
		}
	}

	/* copy image */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD, BR_STRING_CTXMENU_COPY_IMAGE, true);
			break;
		}
	}

	/* View image */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW, BR_STRING_CTXMENU_VIEW_IMAGE, true);
			break;
		}
	}

	free(original_ctx_item);
}

void webview::_show_context_menu_image_link(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("");

	unsigned short count = ewk_context_menu_item_count(menu);
	RET_MSG_IF(count == 0, "There is no context menu for image link");

	Ewk_Context_Menu_Item_Tag *original_ctx_item = NULL;
	original_ctx_item = (Ewk_Context_Menu_Item_Tag *)malloc(sizeof(Ewk_Context_Menu_Item_Tag) * count);
	RET_MSG_IF(original_ctx_item == NULL, "Failed to allocate memory for ctx menu reordering");

	memset(original_ctx_item, 0x00, sizeof(Ewk_Context_Menu_Item_Tag) * count);

	for (unsigned short i = 0 ; i < count ; i++) {
		Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
		Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

		original_ctx_item[i] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	/* Open in new window */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW, true);
			break;
		}
	}

	/* Save link */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK, BR_STRING_CTXMENU_SAVE_LINK, true);
			break;
		}
	}

	/* Copy link */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, BR_STRING_CTXMENU_COPY_LINK, true);
			break;
		}
	}

	/* Save image */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK, BR_STRING_CTXMENU_SAVE_IMAGE, true);
			break;
		}
	}

	/* copy image */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD, BR_STRING_CTXMENU_COPY_IMAGE, true);
			break;
		}
	}

	/* View image */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW, BR_STRING_CTXMENU_VIEW_IMAGE, true);
			break;
		}
	}

	free(original_ctx_item);
}

void webview::_show_context_menu_email_address(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("");

	unsigned short count = ewk_context_menu_item_count(menu);
	RET_MSG_IF(count == 0, "There is no context menu for email address");

	Ewk_Context_Menu_Item_Tag *original_ctx_item = NULL;
	original_ctx_item = (Ewk_Context_Menu_Item_Tag *)malloc(sizeof(Ewk_Context_Menu_Item_Tag) * count);
	RET_MSG_IF(original_ctx_item == NULL, "Failed to allocate memory for ctx menu reordering");

	memset(original_ctx_item, 0x00, sizeof(Ewk_Context_Menu_Item_Tag) * count);

	for (unsigned short i = 0 ; i < count ; i++) {
		Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
		Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

		original_ctx_item[i] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	/* Send email */
	ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_SEND_EMAIL, BR_STRING_CTXMENU_SEND_EMAIL, true);

	/* Add to contact */
	ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_SEND_ADD_TO_CONTACT, BR_STRING_CTXMENU_ADD_TO_CONTACT, true);

	/* Copy */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_DATA) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_DATA, BR_STRING_CTXMENU_COPY, true);
			break;
		}
	}

	/* Selection mode */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE, BR_STRING_CTXMENU_SELECT_TEXT, true);
			break;
		}
	}

	free(original_ctx_item);
}

void webview::_show_context_menu_call_number(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("");

	unsigned short count = ewk_context_menu_item_count(menu);
	RET_MSG_IF(count == 0, "There is no context menu for email address");

	Ewk_Context_Menu_Item_Tag *original_ctx_item = NULL;
	original_ctx_item = (Ewk_Context_Menu_Item_Tag *)malloc(sizeof(Ewk_Context_Menu_Item_Tag) * count);
	RET_MSG_IF(original_ctx_item == NULL, "Failed to allocate memory for ctx menu reordering");

	memset(original_ctx_item, 0x00, sizeof(Ewk_Context_Menu_Item_Tag) * count);

	for (unsigned short i = 0 ; i < count ; i++) {
		Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
		Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

		original_ctx_item[i] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	/* Call */
	ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_CALL, BR_STRING_CTXMENU_CALL, true);

	/* Send message */
	ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_SEND_MESSAGE, BR_STRING_CTXMENU_SEND_MESSAGE, true);

	/* Add to contact */
	ewk_context_menu_item_append_as_action(menu, CUSTOM_CONTEXT_MENU_ITEM_SEND_ADD_TO_CONTACT, BR_STRING_CTXMENU_ADD_TO_CONTACT, true);

	/* Copy */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_DATA) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_DATA, BR_STRING_CTXMENU_COPY, true);
			break;
		}
	}

	/* Selection mode */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE, BR_STRING_CTXMENU_SELECT_TEXT, true);
			break;
		}
	}

	free(original_ctx_item);
}

void webview::_show_context_menu_text_image_link(Ewk_Context_Menu *menu)
{
	BROWSER_LOGD("");

	unsigned short count = ewk_context_menu_item_count(menu);
	RET_MSG_IF(count == 0, "There is no context menu for image link");

	Ewk_Context_Menu_Item_Tag *original_ctx_item = NULL;
	original_ctx_item = (Ewk_Context_Menu_Item_Tag *)malloc(sizeof(Ewk_Context_Menu_Item_Tag) * count);
	RET_MSG_IF(original_ctx_item == NULL, "Failed to allocate memory for ctx menu reordering");

	memset(original_ctx_item, 0x00, sizeof(Ewk_Context_Menu_Item_Tag) * count);

	for (unsigned short i = 0 ; i < count ; i++) {
		Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, 0);
		Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

		original_ctx_item[i] = tag;
		ewk_context_menu_item_remove(menu, item);
	}

	/* Open in new window */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW, true);
			break;
		}
	}

	/* Save link */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK, BR_STRING_CTXMENU_SAVE_LINK, true);
			break;
		}
	}

	/* Copy link */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, BR_STRING_CTXMENU_COPY_LINK, true);
			break;
		}
	}

	/* Save image */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK, BR_STRING_CTXMENU_SAVE_IMAGE, true);
			break;
		}
	}

	/* copy image */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD, BR_STRING_CTXMENU_COPY_IMAGE, true);
			break;
		}
	}

	/* View image */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW, BR_STRING_CTXMENU_VIEW_IMAGE, true);
			break;
		}
	}

	/* Selection mode */
	for (unsigned short i = 0 ; i < count ; i++) {
		if (original_ctx_item[i] == EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE) {
			ewk_context_menu_item_append_as_action(menu, EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE, BR_STRING_CTXMENU_SELECT_TEXT, true);
			break;
		}
	}

	free(original_ctx_item);
}

void webview::__load_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is null");

	webview *wv = (webview *)data;
	BROWSER_LOGI("Page load finished, URI: %s", wv->get_uri());
	wv->m_progress = 1.0;

	// Load started timer cb should be called first.
	wv->m_load_finished_idler = ecore_timer_add(0.01, __load_finished_timer_cb, data);

	if (wv->m_load_finish_timer)
		ecore_timer_del(wv->m_load_finish_timer);
	wv->m_load_finish_timer = ecore_timer_add(0.2, __finish_timer_idler_cb, wv);
}

Eina_Bool webview::__load_finished_timer_cb(void *data)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL, "data is null");

	browser_view *bv = m_browser->get_browser_view();
	webview *wv = (webview *)data;
	RETV_MSG_IF(!wv, ECORE_CALLBACK_CANCEL, "wv is null");
	RETV_MSG_IF(!bv, ECORE_CALLBACK_CANCEL, "bv is null");

	wv->m_load_finished_idler = NULL;

	if (wv == bv->get_current_webview())
		bv->get_url_bar()->set_loading_status(EINA_FALSE);

	if ((!bv->is_show_url_input_bar()) /*&& (!m_browser->get_network_manager()->is_wifi_connect_popup_shown()*/) {
		elm_access_highlight_set(elm_access_object_get(wv->m_ewk_view));
		elm_access_say(BR_STRING_WEBPAGE_LOADED_T);
	}

	//Get the host type if cert is stored so that we can decide on the icon to show
	HOST_TYPE type = certificate_manager::__is_exist_cert_for_host(wv->get_uri());
	BROWSER_LOGD("Host type = %d", type);
	if (type == SECURE_HOST)
		wv->get_certificate_manager()->set_cert_type(certificate_manager::VALID);
	else if (type == UNSECURE_HOST_ALLOWED || type == UNSECURE_HOST_UNKNOWN)
		wv->get_certificate_manager()->set_cert_type(certificate_manager::INVALID);

	return ECORE_CALLBACK_CANCEL;
}

static void _kill_browser_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	elm_exit();
}

void webview::__process_crashed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_browser_view()->show_msg_popup(NULL, BR_STRING_WEBPROCESS_CRASH, NULL, "IDS_BR_SK_OK", _kill_browser_cb);
}

void webview::__create_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_browser_view()->hide_all_views_return_main_view();

	if (m_browser->get_webview_list()->get_count() >= BROWSER_WINDOW_MAX_SIZE) {
		m_browser->get_browser_view()->show_max_window_limit_reached_popup();
		*((Evas_Object **)event_info) = NULL;
		return;
	}

	RET_MSG_IF(!data, "data is null");
	webview *wv = (webview *)data;

	wv->m_new_window_count++;

	if (wv->m_new_window_count > BROWSER_WINDOW_MAX_NEW_WINDOW)
		return;

	webview *new_wv = NULL;
	new_wv = m_browser->get_webview_list()->create_renewed_webview();
	new_wv->set_parent_webview(wv);

	*((Evas_Object **)event_info) = new_wv->get_ewk_view();

	m_browser->get_browser_view()->set_current_webview(new_wv);

	/* parent window can received signals such as multiple windows requested to create from webkit. */
	if (wv != NULL)
		wv->resume();

}

Eina_Bool webview::__close_window_idler_cb(void *data)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;

	if (m_browser->get_webview_list()->get_count() <= 1) {
		char *default_homepage = m_preference->get_homepage_uri();
		if (default_homepage && (wv->m_is_download_url == EINA_FALSE))
			m_browser->get_browser_view()->get_current_webview()->load_uri(default_homepage);
		else {
			wv->m_is_download_url = EINA_FALSE;
			// Do not change current url if there is only one webview.
			// m_browser->get_browser_view()->get_current_webview()->load_uri(blank_page);
		}

		if (default_homepage)
			free(default_homepage);

		return ECORE_CALLBACK_CANCEL;
	}

	//If tab manager exists, let tab manager take care of deleting the window
	if (m_browser->is_tab_manager_view_exist())
		m_browser->get_tab_manager_view()->delete_tab(wv);
	else {
		webview *replace_wv = m_browser->get_webview_list()->delete_webview(wv);
		if (replace_wv)
			m_browser->get_browser_view()->set_current_webview(replace_wv);
	}

	return ECORE_CALLBACK_CANCEL;
}

/* The window should be deleted by idler. The webkit recommands to delete by idler. */
void webview::__close_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	ecore_idler_add(__close_window_idler_cb, data);
}

void webview::__icon_received_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	if (!m_browser->get_webview_list()->is_webview_exist(wv))
		return;

	webview_info *wvi = new webview_info(wv, EINA_TRUE); // Snapshot is not used below cb funcion.
	ecore_timer_add(0.3, __icon_received_timer_cb, wvi);
}

Eina_Bool webview::__icon_received_timer_cb(void *data)
{
	BROWSER_LOGD("");
	webview_info *wvi = (webview_info *)data;
	RETV_MSG_IF(!wvi, ECORE_CALLBACK_CANCEL, "wvi is null");

	Evas_Object *favicon = wvi->get_favicon();

	/* Save favicon to history DB - if the icon was not in icon DB*/
	if (!wvi->is_private_mode_enabled())
		m_browser->get_history()->set_history_favicon(wvi->get_uri(), favicon);

	/* Update favicon to Bookmark DB if there are same URLs */
	int bookmark_id = -1;
	m_browser->get_bookmark()->get_id(wvi->get_uri(), &bookmark_id);
	if (bookmark_id >= 0)
		m_browser->get_bookmark()->set_favicon(bookmark_id, favicon);
	/* Also Update favicon to Bookmark DB if there are same redirected URLs */
	bookmark_id = -1;
	m_browser->get_bookmark()->get_id(wvi->get_url_for_bookmark_update(), &bookmark_id);
	if (bookmark_id >= 0)
		m_browser->get_bookmark()->set_favicon(bookmark_id, favicon);

	// Update favicon in url-bar
	m_browser->get_browser_view()->get_url_bar()->update_fav_icon();

	delete wvi;
	return ECORE_CALLBACK_CANCEL;
}

void webview::__urlbar_scroll_cb(void *data, Evas_Object *obj, void *event_info)
{
	int delta = *((int *)event_info);
	BROWSER_LOGD("delta: %d", delta);

	main_view *mv = m_browser->get_main_view();
	mv->set_scroll_position_delta(delta);
}

void webview::__flick_canceled_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	main_view *mv = m_browser->get_main_view();
	mv->set_is_flicked(EINA_FALSE);
}

Eina_Bool webview::__delete_webview_idler_cb(void *data)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;

	webview *replace_wv = m_browser->get_webview_list()->delete_webview(wv);
	if (replace_wv)
		m_browser->get_browser_view()->set_current_webview(replace_wv);

	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __update_title_cb(void *data)
{
	BROWSER_LOGD("");
	url_bar *ub = (url_bar*)data;
	ub->update_star_icon();
	ub->update_secure_icon();
	ub->update_fav_icon();
	return ECORE_CALLBACK_CANCEL;
}

void webview::__policy_navigation_decide_cb(void *data, Evas_Object *obj, void *event_info)
{
	webview *wv = (webview *)data;
	Ewk_Policy_Decision *policy_decision = (Ewk_Policy_Decision *)event_info;
	const char *uri = ewk_policy_decision_url_get(policy_decision);
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RET_MSG_IF(!uri, "URI is NULL");

	wv->m_navigation_count++;

	browser_view *bv = m_browser->get_browser_view();
	Eina_Bool is_scheme_handled = bv->handle_scheme(uri);

	if (is_scheme_handled) {
		BROWSER_LOGD("scheme handled");
		ewk_policy_decision_ignore(policy_decision);

		if (!wv->backward_possible() && !wv->is_user_created()) {
			ecore_idler_add(__delete_webview_idler_cb, wv);
			wv = NULL;
		}
		return;
	}

	ewk_policy_decision_use(policy_decision);

	Ewk_Policy_Navigation_Type policy_type = ewk_policy_decision_navigation_type_get(policy_decision);

	// Load can be initiated by another window in case of A tag with non-blank target.
	if (policy_type == EWK_POLICY_NAVIGATION_TYPE_LINK_CLICKED) {
		// Set init url if the url is changed by link clicked.
		wv->_set_init_url(uri);
		wv->set_request_uri(uri);

		//If any link is clicked by user update the url for bookmark updation with the latest URL.
		wv->_set_url_for_bookmark_update(uri);

		webview *current_webview = bv->get_current_webview();
		if (current_webview != wv && wv->get_navigation_count() == 1) // In case of new window.
			bv->set_current_webview(wv);
	}

	// Check data scheme.
	if (strlen(uri) > strlen("data:") && !strncmp(uri, "data:", 5)) {
		BROWSER_LOGD("data scheme");
		wv->m_is_data_scheme = EINA_TRUE;
	} else
		wv->m_is_data_scheme = EINA_FALSE;

	// Check file scheme.
	if (strlen(uri) > strlen("file:") && !strncmp(uri, "file:", 5)) {
		BROWSER_LOGD("file scheme");
		wv->m_is_file_scheme = EINA_TRUE;
	} else
		wv->m_is_file_scheme = EINA_FALSE;
}

void webview::__policy_response_decide_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	Ewk_Policy_Decision *policy_decision = (Ewk_Policy_Decision *)event_info;
	Ewk_Policy_Decision_Type policy_type = ewk_policy_decision_type_get(policy_decision);
	wv->m_status_code = ewk_policy_decision_response_status_code_get(policy_decision);
	const char *uri = ewk_policy_decision_url_get(policy_decision);
	const char *cookie = ewk_policy_decision_cookie_get(policy_decision);
	const char *content_type = ewk_policy_decision_response_mime_get(policy_decision);
	const Eina_Hash *headers = ewk_policy_decision_response_headers_get(policy_decision);
	wv->m_is_error_page = EINA_FALSE;

	switch (policy_type) {
	case EWK_POLICY_DECISION_USE:
		BROWSER_LOGD("policy_use");
		ewk_policy_decision_use(policy_decision);
		break;

	case EWK_POLICY_DECISION_DOWNLOAD: {
		app_control_h app_control = NULL;
		if (app_control_create(&app_control) < 0) {
			BROWSER_LOGE("Fail to app_control_create");
			return;
		}

		if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) < 0) {
			BROWSER_LOGE("Fail to app_control_set_operation");
			app_control_destroy(app_control);
			return;
		}

		BROWSER_SECURE_LOGD("uri: %s", uri);
		if (app_control_set_uri(app_control, uri) < 0) {
			BROWSER_LOGE("Fail to app_control_set_uri");
			app_control_destroy(app_control);
			return;
		}

		BROWSER_SECURE_LOGD("content_type: %s", content_type);
		if (app_control_set_mime(app_control, content_type) < 0) {
			BROWSER_LOGE("Fail to app_control_set_mime");
			app_control_destroy(app_control);
			return;
		}

		const char *content_dispotision = (const char *)eina_hash_find(headers, "Content-Disposition");
		BROWSER_LOGD("Content-disposition: %s", content_dispotision);
		if (content_dispotision && (strstr(content_dispotision, "attachment") != NULL)){
			m_browser->get_download_manager()->handle_download_request(uri, content_type);
			app_control_destroy(app_control);
			ewk_policy_decision_ignore(policy_decision);
			break;
		}

		if (!strcmp(content_type, "application/sdp")) {
			BROWSER_SECURE_LOGD("sdp : download uri: %s", uri);
			m_browser->get_browser_view()->launch_sdp_svc(uri);
			app_control_destroy(app_control);
			ewk_policy_decision_ignore(policy_decision);
			break;
		}

		if (app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_APP_NOT_FOUND) {
			BROWSER_SECURE_LOGD("app_control_send_launch_request returns APP_CONTROL_ERROR_APP_NOT_FOUND");
			m_browser->get_download_manager()->handle_download_request(uri, content_type);
		}
		app_control_destroy(app_control);
		ewk_policy_decision_ignore(policy_decision);
		break;
	}
	case EWK_POLICY_DECISION_IGNORE:
	default:
		BROWSER_LOGD("policy_ignore");
		ewk_policy_decision_ignore(policy_decision);
		break;
	}

	if (wv->m_navigation_count == 1 && policy_type == EWK_POLICY_DECISION_DOWNLOAD) {
		wv->m_is_download_url = EINA_TRUE;
		ecore_idler_add(__close_window_idler_cb, data);
	}
}

void webview::__form_submit_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
}

void webview::__back_forward_list_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	webview *wv = (webview *)data;

	m_browser->get_main_view()->get_main_toolbar()->enable_forward_button(wv->forward_possible());
	m_browser->get_main_view()->get_main_toolbar()->enable_backward_button(wv->backward_possible());
}

void webview::__set_certificate_data_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	RET_MSG_IF(!event_info, "event_info is NULL");
	RET_MSG_IF(!data, "data is NULL");

	webview *wv = (webview *)data;
	const char *cert_data = (const char *)event_info;
	wv->m_certi_manager->set_certificate_data(cert_data);

	/*If the cert. doesn't exist for a host, add it else update it instead of adding a new row*/
	if (certificate_manager::__is_exist_cert_for_host(wv->get_uri()) == HOST_ABSENT)
		certificate_manager::_save_certificate_info(cert_data, wv->get_uri(), SECURE_HOST);
	else
		certificate_manager::_update_certificate_info(cert_data, wv->get_uri(), SECURE_HOST);
}

void webview::__fullscreen_enter_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if(m_browser != NULL )
		m_browser->set_full_screen_enable(EINA_TRUE);
}

void webview::__fullscreen_exit_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if(m_browser != NULL )
		m_browser->set_full_screen_enable(EINA_FALSE);
}

void webview::show_certificate_status_popup(Eina_Bool ask)
{
	BROWSER_LOGD("enter->");

	m_certi_manager->show_certificate_status_popup(ask);
}

context_menu_type webview::_get_menu_type(Ewk_Context_Menu *menu)
{
	RETV_MSG_IF(!menu, UNKNOWN_MENU, "menu is NULL");

	unsigned short count = ewk_context_menu_item_count(menu);
	Eina_Bool text = EINA_FALSE;
	Eina_Bool link = EINA_FALSE;
	Eina_Bool image = EINA_FALSE;
	Eina_Bool email_address = EINA_FALSE;
	Eina_Bool call_number = EINA_FALSE;
	Eina_Bool selection_mode = EINA_FALSE;
	for (unsigned short i = 0 ; i < count ; i++) {
		Ewk_Context_Menu_Item *item = ewk_context_menu_nth_item_get(menu, i);
		Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
		const char *link_url = ewk_context_menu_item_link_url_get(item);
		BROWSER_LOGD("tag=%d", tag);

		if (link_url && !strncmp(mailto_scheme, link_url, strlen(mailto_scheme)))
			email_address = EINA_TRUE;
		if (link_url && !strncmp(tel_scheme, link_url, strlen(tel_scheme)))
			call_number = EINA_TRUE;
		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE)
			selection_mode = EINA_TRUE;
		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_CLIPBOARD)
			return INPUT_FIELD;
		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB)
			text = EINA_TRUE;
		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW || tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD)
			link = EINA_TRUE;
		if (tag == EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD)
			image = EINA_TRUE;
	}

	if (email_address && selection_mode)
		return EMAIL_SCHEME;
	if (call_number && selection_mode)
		return TEL_SCHEME;
	if (text && !link)
		return TEXT_ONLY;
	if (link && !image)
		return TEXT_LINK;
	if (image && !link)
		return IMAGE_ONLY;
	 if(selection_mode && image && link)
		return TEXT_IMAGE_LINK;
	if (image && link)
		return IMAGE_LINK;

	return UNKNOWN_MENU;
}

void webview::_customize_context_menu(Ewk_Context_Menu *menu)
{
	RET_MSG_IF(!menu, "menu is NULL");

	context_menu_type menu_type = _get_menu_type(menu);
	BROWSER_LOGD("menu_type=%d", menu_type);

	if (menu_type == UNKNOWN_MENU || menu_type == INPUT_FIELD)
		return;

	if (menu_type != TEXT_ONLY)
		give_focus(EINA_FALSE);

	switch (menu_type) {
		case TEXT_ONLY:
			_show_context_menu_text_only(menu);
		break;

		case TEXT_LINK:
			_show_context_menu_text_link(menu);
		break;

		case IMAGE_ONLY:
			_show_context_menu_image_only(menu);
		break;

		case IMAGE_LINK:
			_show_context_menu_image_link(menu);
		break;

		case EMAIL_SCHEME:
			_show_context_menu_email_address(menu);
		break;

		case TEL_SCHEME:
			_show_context_menu_call_number(menu);
		break;

		case TEXT_IMAGE_LINK:
			_show_context_menu_text_image_link(menu);
		break;

		default:
			return;
		break;
	}
}

void webview::__contextmenu_customize_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	Ewk_Context_Menu *menu = static_cast<Ewk_Context_Menu*>(event_info);
	webview *wv = (webview *)data;

	wv->_customize_context_menu(menu);
}

void webview::__contextmenu_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = (webview *)data;
	Ewk_Context_Menu_Item* item = static_cast<Ewk_Context_Menu_Item*>(event_info);
	Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
	const char *selected_text = ewk_view_text_selection_text_get(wv->m_ewk_view);
	const char *link_url = ewk_context_menu_item_link_url_get(item);
	const char *image_url = ewk_context_menu_item_image_url_get(item);
	const char *item_text = NULL;

	if (selected_text && strlen(selected_text))
		item_text = selected_text;
	else if (link_url && strlen(link_url))
		item_text = link_url;
	else if (image_url && strlen(image_url))
		item_text = image_url;

	if (tag == CUSTOM_CONTEXT_MENU_ITEM_FIND_ON_PAGE) {
		m_browser->get_browser_view()->show_find_on_page(selected_text, wv, EINA_FALSE);
	} else if (tag == CUSTOM_CONTEXT_MENU_ITEM_SHARE) {
		m_browser->get_browser_view()->share(item_text, NULL);
	} else if (tag == CUSTOM_CONTEXT_MENU_ITEM_SEARCH) {
		if (!(selected_text && strlen(selected_text)))
			return;

		m_browser->get_browser_view()->hide_all_views_return_main_view();

		if (m_browser->get_webview_list()->get_count() >= BROWSER_WINDOW_MAX_SIZE)
			m_browser->get_browser_view()->show_max_window_limit_reached_popup();
		else {
			int search_engine_type = m_preference->get_search_engine_type();

			// Set default search engine based on csc feature.
			if (search_engine_type == SEARCH_ENGINE_NOT_SELECTED) {
				BROWSER_LOGD("Default search engine google");
				search_engine_type = SEARCH_ENGINE_GOOGLE;
				m_preference->set_search_engine_type(search_engine_type);
			}

			search_engine_manager *search_manager = new search_engine_manager();
			std::string query_uri = search_manager->query_string_get(search_engine_type, selected_text);

			webview *new_wv = m_browser->get_webview_list()->create_renewed_webview(EINA_FALSE);
			//Parent webview should be the webview where the query text copied
			new_wv->set_parent_webview(wv);
			m_browser->get_browser_view()->set_current_webview(new_wv);
			new_wv->load_uri(query_uri.c_str());

			delete search_manager;
		}
	} else if (tag == CUSTOM_CONTEXT_MENU_ITEM_SEND_EMAIL) {
		m_browser->get_browser_view()->handle_scheme(link_url);
	} else if (tag == CUSTOM_CONTEXT_MENU_ITEM_CALL) {
		m_browser->get_browser_view()->handle_scheme(link_url);
	} else if (tag == CUSTOM_CONTEXT_MENU_ITEM_SEND_MESSAGE) {
		if (link_url && !strncmp(tel_scheme, link_url, strlen(tel_scheme))) {
			std::string::size_type pos = std::string::npos;
			std::string source = std::string(link_url);
			while ((pos = source.find(tel_scheme)) != std::string::npos)
				source.replace(pos, strlen(tel_scheme), sms_scheme);
			m_browser->get_browser_view()->handle_scheme(source.c_str());
		}
	} else if (tag == CUSTOM_CONTEXT_MENU_ITEM_SEND_ADD_TO_CONTACT) {
		if (link_url && !strncmp(tel_scheme, link_url, strlen(tel_scheme))) {
			m_browser->get_browser_view()->launch_contact(link_url + strlen(tel_scheme));
		} else if (link_url && !strncmp(mailto_scheme, link_url, strlen(mailto_scheme))) {
			size_t source_end_pos = 0;
			std::string source = std::string(link_url);

			if (source.find("?") != std::string::npos) {
				source_end_pos = source.find("?");
				source = source.substr(0, source_end_pos);
			}
			m_browser->get_browser_view()->launch_contact(NULL, source.c_str() + strlen(mailto_scheme));
		}
	}
}

void webview::__mouse_down_cb(void *data, Evas* evas, Evas_Object *obj, void *event_info)
{
	webview *wv = (webview *)data;

	if (m_browser->get_browser_view()->is_show_find_on_page())
		wv->give_focus();
}

void webview::_create_ewk_view()
{
	BROWSER_LOGD("");
	RET_MSG_IF(!m_window, "m_window is NULL");

#ifdef ENABLE_INCOGNITO_WINDOW
	if (is_incognito())
		m_ewk_view = ewk_view_add_in_incognito_mode(evas_object_evas_get(m_window));
	else
		m_ewk_view = ewk_view_add(evas_object_evas_get(m_window));
#else
	m_ewk_view = ewk_view_add(evas_object_evas_get(m_window));
#endif
	if (!m_ewk_view)
		_exit_browser();

	evas_object_color_set(m_ewk_view, 255, 255, 255, 255);
	evas_object_size_hint_weight_set(m_ewk_view, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_ewk_view, EVAS_HINT_FILL, EVAS_HINT_FILL);

	_init_settings();
}

void webview::set_url_bar_shown(Eina_Bool shown)
{
	BROWSER_LOGD("%d", shown);

	m_is_url_bar_shown = shown;

	if (shown)
		evas_object_smart_callback_call(m_ewk_view, "browser,urlbar,show", NULL);
	else
		evas_object_smart_callback_call(m_ewk_view, "browser,urlbar,hide", NULL);
}

void webview::accept_cookie_enabled_set(Evas_Object *ewk_view, Eina_Bool enable)
{
	RET_MSG_IF(!ewk_view, "ewk_view is NULL");

	BROWSER_LOGD("enable = [%d]", enable);

	Ewk_Cookie_Manager *cookie_manager = ewk_context_cookie_manager_get(ewk_view_context_get(ewk_view));
	if (enable)
		ewk_cookie_manager_accept_policy_set(cookie_manager, EWK_COOKIE_ACCEPT_POLICY_ALWAYS);
	else
		ewk_cookie_manager_accept_policy_set(cookie_manager, EWK_COOKIE_ACCEPT_POLICY_NEVER);
}

void webview::clear_cookies(Evas_Object *ewk_view)
{
	RET_MSG_IF(!ewk_view, "ewk_view is NULL");

	Ewk_Cookie_Manager *cookie_manager = ewk_context_cookie_manager_get(ewk_view_context_get(ewk_view));
	ewk_cookie_manager_cookies_clear(cookie_manager);
}
