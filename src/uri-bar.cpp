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

#include "uri-bar.h"

#include <Elementary.h>

#include "add-tag-view.h"
#include "bookmark-add-view.h"
#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "history.h"
#include "history-view.h"
#include "multiwindow-view.h"
#include "preference.h"
#include "scrap.h"
#include "scrap-view.h"
#include "setting-view.h"
#include "user-agent-manager.h"
#if defined(INSTALL_WEB_APP)
#include "webapp-install-manager.h"
#endif
#include "webview.h"
#include "webview-list.h"
#if defined(WEBCLIP)
#include "scissorbox-view.h"
#endif

#define uri_bar_edj_path browser_edj_dir"/uri-bar.edj"

#define PROGRESSBAR_H	(98 * efl_scale)

#define PRINT_PDF_W	210
#define PRINT_PDF_H	297

#define pdf_file_path	browser_data_dir"/print.pdf"
#define print_pkg_name	"org.tizen.mobileprint"
#define print_service_name	"http://tizen.org/appcontrol/operation/PRINT"
#define print_files_type	"service_print_files_type"

uri_bar::uri_bar(Evas_Object *parent)
:
	m_backward_button(NULL)
	,m_forward_button(NULL)
	,m_multi_window_button(NULL)
	,m_progress_bar(NULL)
	,m_progress_bar_bg(NULL)
	,m_uri_entry_layout(NULL)
	,m_is_private_mode(EINA_FALSE)
	,m_back_longpressed_timer(NULL)
	,m_scrap_tag(NULL)
{
	BROWSER_LOGD("");
	m_main_layout = _create_main_layout(parent);

	elm_theme_extension_add(NULL, uri_bar_edj_path);
}

uri_bar::~uri_bar(void)
{
	BROWSER_LOGD("");
	if (m_scrap_tag)
		free(m_scrap_tag);

	if (m_progress_bar)
		evas_object_del(m_progress_bar);
	if (m_progress_bar_bg)
		evas_object_del(m_progress_bar_bg);
	if (m_back_longpressed_timer) {
		ecore_timer_del(m_back_longpressed_timer);
		m_back_longpressed_timer = NULL;
	}

	elm_theme_extension_del(NULL, uri_bar_edj_path);
}

const char *uri_bar::get_uri(void)
{
	return edje_object_part_text_get(elm_layout_edje_get(m_uri_entry_layout), "uri_text");
}

void uri_bar::set_uri(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN(uri);

	char *display_uri = NULL;
	int offset = 0;
	if (strstr(uri, http_scheme))
		offset = strlen(http_scheme);

	display_uri = elm_entry_utf8_to_markup(uri + offset);
	BROWSER_LOGD("display_uri = [%s]", display_uri);

	if (display_uri) {
		edje_object_part_text_set(elm_layout_edje_get(m_uri_entry_layout), "uri_text", (const char *)display_uri);
		if (m_browser->get_app_in_app_enable())
			m_browser->get_browser_view()->set_app_in_app_title((const char *)display_uri);

		free(display_uri);
	}

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	set_private_mode(wv->private_browsing_enabled_get());
}

void uri_bar::show_app_in_app(Eina_Bool app_in_app)
{
	BROWSER_LOGD("app_in_app=[%d]", app_in_app);
	if (app_in_app)
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "app_in_app,signal", "");
	else
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "normal,signal", "");
}

void uri_bar::_set_entry_icon(Eina_Bool private_mode)
{
	BROWSER_LOGD("private_mode = [%d]", private_mode);
	if (m_browser->get_browser_view()->get_current_webview() == NULL)
		return;

	const char *current_uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
	if (current_uri && strlen(current_uri) > strlen(https_scheme) && !strncmp(https_scheme, current_uri, strlen(https_scheme))) {
		if (private_mode)
			edje_object_signal_emit(elm_layout_edje_get(m_uri_entry_layout), "show,private_secure_on,signal", "");
		else
			edje_object_signal_emit(elm_layout_edje_get(m_uri_entry_layout), "show,secure_icon,signal", "");
	} else {
		if (private_mode)
			edje_object_signal_emit(elm_layout_edje_get(m_uri_entry_layout), "show,private_icon,signal", "");
		else
			edje_object_signal_emit(elm_layout_edje_get(m_uri_entry_layout), "hide,icon,signal", "");
	}
}

void uri_bar::_show_loading_status(Eina_Bool loading)
{
	BROWSER_LOGD("loading = [%d]", loading);

	if (loading) {
		edje_object_signal_emit(elm_layout_edje_get(m_uri_entry_layout), "show,stop_icon,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "loading,signal", "");
		// Workaround, the load,start progress bar is not visible, so display it using edc.
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,dummy_progressbar,signal", "");
	} else {
		edje_object_signal_emit(elm_layout_edje_get(m_uri_entry_layout), "show,reload_icon,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "loading_finished,signal", "");
		// Workaround, the load,start progress bar is not visible, so display it using edc.
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,dummy_progressbar,signal", "");
	}
}

void uri_bar::update_progress_bar(double rate)
{
	if (rate) {
		if (rate == 1.0) {
			if (m_progress_bar) {
				evas_object_del(m_progress_bar);
				m_progress_bar = NULL;
			}
			// When loading is finished, show title instead of uri according to the UI guide.
			const char *title = m_browser->get_browser_view()->get_current_webview()->get_title();
			webview *wv = m_browser->get_browser_view()->get_current_webview();

			if (title && strlen(title))
				set_uri(title);
			else
				set_uri(wv->get_uri());

			if (wv->forward_possible()) {
				disable_forward_button(EINA_FALSE);
				m_browser->get_browser_view()->disable_mini_forward_button(EINA_FALSE);
			} else {
				disable_forward_button(EINA_TRUE);
				m_browser->get_browser_view()->disable_mini_forward_button(EINA_TRUE);
			}

			if (wv->backward_possible())
				m_browser->get_browser_view()->disable_mini_backward_button(EINA_FALSE);
			else
				m_browser->get_browser_view()->disable_mini_backward_button(EINA_TRUE);

			_show_loading_status(EINA_FALSE);

			return;
		}

		int progress_bar_height = (int)PROGRESSBAR_H;
		int x, y, w, h;
		edje_object_part_geometry_get(elm_layout_edje_get(m_main_layout), "entry_circle_bg", &x, &y, &w, &h);

		if (!m_progress_bar) {
			m_progress_bar = evas_object_rectangle_add(evas_object_evas_get(m_window));
			evas_object_color_set(m_progress_bar, 91, 166, 255, 255);

			elm_object_part_content_set(m_main_layout, "elm.swallow.progressbar", m_progress_bar);
		}

		evas_object_resize(m_progress_bar, (int)(w * rate), progress_bar_height);

		if (rate == 0.05f)
			_show_loading_status(EINA_TRUE);
	}
}

void uri_bar::set_multi_window_button_count(int window_count)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(m_multi_window_button);

	char num[5] = {0, };
	sprintf(num, "%d", window_count);
	elm_object_text_set(m_multi_window_button, num);
}

void uri_bar::disable_forward_button(Eina_Bool disable)
{
	EINA_SAFETY_ON_NULL_RETURN(m_forward_button);
	elm_object_disabled_set(m_forward_button, disable);
}

void uri_bar::disable_backward_button(Eina_Bool disable)
{
	EINA_SAFETY_ON_NULL_RETURN(m_backward_button);
	elm_object_disabled_set(m_backward_button, disable);
}

void uri_bar::set_private_mode(Eina_Bool enable, Eina_Bool update_history)
{
	m_is_private_mode = enable;
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	wv->private_browsing_enabled_set(m_is_private_mode);
	_set_entry_icon(m_is_private_mode);

	if (update_history) {
		if (m_is_private_mode)
			m_browser->get_history()->delete_history(wv->get_uri());
		else {
			int visit_count = 0;
			m_browser->get_history()->save_history(wv->get_title(), wv->get_uri(), wv->get_snapshot(), &visit_count);
		}
	}
}

void uri_bar::disable_buttons(Eina_Bool disable)
{
	BROWSER_LOGD("disable=[%d]");
	Evas_Object *button = elm_object_part_content_get(m_main_layout, "elm.swallow.menu_button");
	elm_object_disabled_set(button, disable);
	BROWSER_LOGD("menu=%d", button);

	button = elm_object_part_content_get(m_main_layout, "elm.swallow.multi_window_button");
	elm_object_disabled_set(button, disable);
	BROWSER_LOGD("multi window=%d", button);
}

void uri_bar::__uri_entry_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	uri_bar *ub = (uri_bar *)data;
	ub->disable_buttons(EINA_TRUE);

	m_browser->get_browser_view()->show_uri_input_bar(EINA_TRUE);
}

void uri_bar::__reload_stop_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (wv->is_loading())
		wv->load_stop();
	else
		wv->reload();
}

void uri_bar::__context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Evas_Object *menu_button = (Evas_Object *)evas_object_data_get(obj, "memu_button");
	if (menu_button)
		evas_object_smart_callback_add(menu_button, "clicked", __menu_clicked_cb, data);

	Evas_Object *back_button = (Evas_Object *)evas_object_data_get(obj, "back_button");
	if (back_button)
		evas_object_smart_callback_add(back_button, "clicked", __backward_clicked_cb, NULL);

	evas_object_del(obj);
}

#if defined(INSTALL_WEB_APP)
void uri_bar::__install_web_app_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_webapp_install_manager()->request_make_webapp(m_browser->get_browser_view()->get_current_webview()->get_ewk_view());

	__context_popup_dismissed_cb(data, obj, event_info);
}
#endif

void uri_bar::__setting_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	m_browser->get_setting_view()->show();

	__context_popup_dismissed_cb(data, obj, event_info);
}

void uri_bar::__bookmark_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	m_browser->get_multiwindow_view(EINA_TRUE)->show();

	__context_popup_dismissed_cb(data, obj, event_info);
}

void uri_bar::__bookmark_add_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	const char *title = m_browser->get_browser_view()->get_current_webview()->get_title();
	const char *uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
	m_browser->create_bookmark_add_view(title, uri)->show();

	__context_popup_dismissed_cb(data, obj, event_info);
}

void uri_bar::__find_on_page_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	m_browser->get_browser_view()->show_find_on_page(EINA_TRUE);

	__context_popup_dismissed_cb(data, obj, event_info);
}

void uri_bar::__history_cb(void *data, Evas_Object *obj, void *event_info)
{
	m_browser->get_history_view()->show();

	__context_popup_dismissed_cb(data, obj, event_info);
}

void uri_bar::__reader_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->get_browser_view()->show_reader(EINA_TRUE);
}

void uri_bar::show_reader_icon(Eina_Bool show)
{
	BROWSER_LOGD("show=%d", show);
	if (show) {
		const char* state = edje_object_part_state_get(elm_layout_edje_get(m_uri_entry_layout), "elm.swallow.reader_icon", NULL);
		if (state && !strcmp(state, "reader_on"))
			// Already visible
			return;

		Evas_Object *reader_button = elm_button_add(m_uri_entry_layout);
		if (!reader_button) {
			BROWSER_LOGE("elm_button_add failed");
			return;
		}
		elm_object_style_set(reader_button, "browser/reader");
		evas_object_smart_callback_add(reader_button, "clicked", __reader_cb, this);
		elm_object_part_content_set(m_uri_entry_layout, "elm.swallow.reader_icon", reader_button);

		edje_object_signal_emit(elm_layout_edje_get(m_uri_entry_layout), "show,reader_icon,signal", "");
	} else {
		Evas_Object *reader_button = elm_object_part_content_get(m_uri_entry_layout, "elm.swallow.reader_icon");
		if (reader_button) {
			elm_object_part_content_unset(m_uri_entry_layout, "elm.swallow.reader_icon");
			evas_object_del(reader_button);
		}

		edje_object_signal_emit(elm_layout_edje_get(m_uri_entry_layout), "hide,reader_icon,signal", "");

		_set_entry_icon(m_is_private_mode);
	}
}

void uri_bar::__print_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	m_browser->get_browser_view()->get_current_webview()->save_as_pdf(PRINT_PDF_W, PRINT_PDF_H, pdf_file_path);

	service_h service;

	service_create(&service);
	service_set_operation(service, print_service_name);
	service_set_package(service, print_pkg_name);

	service_set_uri(service, pdf_file_path);
	service_add_extra_data(service, print_files_type, "WEB");
	service_send_launch_request(service, NULL, NULL);
	service_destroy(service);

	__context_popup_dismissed_cb(data, obj, event_info);
}

void uri_bar::__private_on_off_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	uri_bar *ub = (uri_bar *)data;
	ub->m_is_private_mode = !(ub->m_is_private_mode);
	ub->set_private_mode(ub->m_is_private_mode, EINA_TRUE);

	__context_popup_dismissed_cb(data, obj, event_info);
}

void uri_bar::__desktop_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool desktop_view = !m_preference->get_desktop_view_enabled();
	m_preference->set_desktop_view_enabled(desktop_view);

	const char *user_agent = NULL;
	if (desktop_view)
		user_agent = m_browser->get_user_agent_manager()->get_desktop_user_agent();
	else
		user_agent = m_browser->get_user_agent_manager()->get_user_agent();

	int count = m_browser->get_webview_list()->get_count();
	for (int i = 0 ; i < count ; i++)
		m_browser->get_webview_list()->get_webview(i)->user_agent_set(user_agent);

	m_browser->get_browser_view()->get_current_webview()->reload();

	__context_popup_dismissed_cb(data, obj, event_info);
}

#if defined(ADD_TO_HOME)
void uri_bar::__add_to_home_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	uri_bar *cp = (uri_bar *)data;
	//cp->m_browser->get_browser_view()->show_add_to_home_popup();
#if defined(WEBCLIP)
	cp->m_browser->get_scissorbox_view()->show();
#endif
	__context_popup_dismissed_cb(data, obj, event_info);
}
#endif

void uri_bar::__mht_contents_get_cb(Evas_Object *ewk_view, const char *data, void *user_data)
{
	EINA_SAFETY_ON_NULL_RETURN(data);
	uri_bar *ub = (uri_bar *)user_data;
	scrap scrap_instance;

	char *file_path = scrap_instance.save(m_browser->get_browser_view()->get_current_webview()->get_title(), m_browser->get_browser_view()->get_current_webview()->get_uri(), data, ub->m_scrap_tag);
	if (file_path)
		free(file_path);

	if (ub->m_scrap_tag) {
		free(ub->m_scrap_tag);
		ub->m_scrap_tag = NULL;
	}

	m_browser->get_browser_view()->show_noti_popup(BR_STRING_SAVED);
}


void uri_bar::__add_scrap_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	uri_bar *ub = (uri_bar *)data;
	std::vector<char *> tag_list = *((std::vector<char *> *)event_info);

	for (int i = 1 ; i < tag_list.size() ; i++) {
		BROWSER_LOGD("[%d]=[%s]", i, tag_list[i]);
	}

	std::string tag_str;
	if (tag_list.size() > 0)
		tag_str = tag_list[0];

	for (int i = 1 ; i < tag_list.size() ; i++) {
		tag_str = tag_str + std::string(",") + std::string(tag_list[i]);
		BROWSER_LOGD("[%d]=[%s]", i, tag_list[i]);
	}

	if (ub->m_scrap_tag) {
		free(ub->m_scrap_tag);
		ub->m_scrap_tag = NULL;
	}

	if (tag_list.size() > 0)
		ub->m_scrap_tag = strdup(tag_str.c_str());

	m_browser->get_browser_view()->get_current_webview()->mht_contents_get(__mht_contents_get_cb, ub);

	for (int i = 1 ; i < tag_list.size() ; i++)
		delete tag_list[i];

	m_browser->get_browser_view()->show_noti_popup(BR_STRING_SAVED);
}

void uri_bar::__scrap_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	uri_bar *ub = (uri_bar *)data;
#if defined(BROWSER_TAG)
	m_browser->get_add_tag_view(__add_scrap_done_cb, ub, NULL)->show();
#else
	m_browser->get_browser_view()->get_current_webview()->mht_contents_get(__mht_contents_get_cb, ub);
#endif

	__context_popup_dismissed_cb(data, obj, event_info);
}

void uri_bar::__scrapbook_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	m_browser->get_scrap_view()->show();

	__context_popup_dismissed_cb(data, obj, event_info);
}

void uri_bar::_show_more_context_popup(Evas_Object *parent)
{
	BROWSER_LOGD("");
	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	elm_object_style_set(more_popup, "more/default");
	evas_object_data_set(more_popup, "memu_button", parent);
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", __context_popup_dismissed_cb, this);

	Evas_Object *icon = elm_icon_add(more_popup);
	if (!icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_bookmark.png", NULL);
//	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

	elm_ctxpopup_item_append(more_popup, BR_STRING_BOOKMARKS, icon, __bookmark_cb, this);

	icon = elm_icon_add(more_popup);
	if (!icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_add_bookmark.png", NULL);
	elm_ctxpopup_item_append(more_popup, BR_STRING_ADD_BOOKMARK, icon, __bookmark_add_cb, this);

	Elm_Object_Item *it = NULL;
	const char *current_uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
#if defined(ADD_TO_HOME)
	icon = elm_icon_add(more_popup);
	if (!icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_add.png", NULL);

	it = elm_ctxpopup_item_append(more_popup, BR_STRING_ADD_LIVEBOX, icon, __add_to_home_cb, this);
	if (!current_uri || strlen(current_uri) == 0)
		elm_object_item_disabled_set(it, EINA_TRUE);
#endif
#if defined(INSTALL_WEB_APP)
	icon = elm_icon_add(more_popup);
	if (!icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_add.png", NULL);

	elm_ctxpopup_item_append(more_popup, BR_STRING_INSTALL_WEB_APP, icon, __install_web_app_cb, this);
#endif

	icon = elm_icon_add(more_popup);
	if (!icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_find_on_page.png", NULL);
	Elm_Object_Item *find_on_page_item = elm_ctxpopup_item_append(more_popup, BR_STRING_FIND_ON_PAGE, icon, __find_on_page_cb, this);

	if (!current_uri || strlen(current_uri) == 0)
		elm_object_item_disabled_set(find_on_page_item, EINA_TRUE);

	icon = elm_icon_add(more_popup);
	if (!icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}

	if (!m_preference->get_desktop_view_enabled()) {
		elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_desktop_view.png", NULL);
		elm_ctxpopup_item_append(more_popup, BR_STRING_DESKTOP_VIEW, icon, __desktop_view_cb, this);
	} else {
		elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_mobile_view.png", NULL);
		elm_ctxpopup_item_append(more_popup, BR_STRING_MOBILE_VIEW, icon, __desktop_view_cb, this);
	}

	icon = elm_icon_add(more_popup);
	if (!icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}

	if (m_is_private_mode) {
		elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_private.png", NULL);
		elm_ctxpopup_item_append(more_popup, BR_STRING_PRIVATE_OFF, icon, __private_on_off_cb, this);
	} else {
		elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_private.png", NULL);
		elm_ctxpopup_item_append(more_popup, BR_STRING_PRIVATE_ON, icon, __private_on_off_cb, this);
	}

	icon = elm_icon_add(more_popup);
	if (!icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_history.png", NULL);
	elm_ctxpopup_item_append(more_popup, BR_STRING_HISTORY, icon, __history_cb, this);

	icon = elm_icon_add(more_popup);
	if (!icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_print.png", NULL);
	elm_ctxpopup_item_append(more_popup, BR_STRING_PRINT, icon, __print_cb, this);

	icon = elm_icon_add(more_popup);
	if (!icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}
	elm_image_file_set(icon, browser_img_dir"/I01_more_popup_icon_setting.png", NULL);
	elm_ctxpopup_item_append(more_popup, BR_STRING_SETTINGS, icon, __setting_cb, this);

	int x, y, w, h;
	evas_object_geometry_get(parent, &x, &y, &w, &h);
	evas_object_move(more_popup, x + (w / 2), y + (h /2));
	evas_object_show(more_popup);
}

void uri_bar::__minimize_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	__context_popup_dismissed_cb(data, obj, event_info);

	m_browser->set_app_in_app_enable(EINA_TRUE);
}

void uri_bar::__menu_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	uri_bar *ub = (uri_bar *)data;

	evas_object_smart_callback_del(obj, "clicked", __menu_clicked_cb);

	ub->_show_more_context_popup(obj);
}

void uri_bar::__multi_window_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	if (m_browser->is_multiwindow_view_running())
		return;

	m_browser->get_multiwindow_view()->show();
}

void uri_bar::__backward_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = m_browser->get_browser_view()->get_current_webview();

	if (wv->backward_possible())
		wv->backward();
	else if (wv->is_user_created() || (m_browser->get_webview_list()->get_count() == 1))
		elm_win_lower(m_window);
	else {
		webview *replace_wv = m_browser->get_webview_list()->delete_webview(wv);
		if (replace_wv)
			m_browser->get_browser_view()->set_current_webview(replace_wv);
	}
}

void uri_bar::_show_longpress_back_popup(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN(parent);

	if (m_browser->get_webview_list()->get_count() == 1)
		return;

	Evas_Object *back_popup = elm_ctxpopup_add(m_window);
	if (!back_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	evas_object_data_set(back_popup, "back_button", parent);
	evas_object_size_hint_weight_set(back_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(back_popup, "dismissed", __context_popup_dismissed_cb, this);

	Evas_Object *back_icon = elm_icon_add(back_popup);
	if (!back_icon) {
		BROWSER_LOGE("elm_icon_add failed");
		return;
	}
	elm_image_file_set(back_icon, browser_img_dir"/00_icon_Back.png", NULL);
	evas_object_size_hint_aspect_set(back_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

	elm_ctxpopup_item_append(back_popup, BR_STRING_CLOSE_WINDOW, back_icon, __close_window_cb, this);

	int x, y, w, h;
	evas_object_geometry_get(parent, &x, &y, &w, &h);
	evas_object_move(back_popup, x + (w / 2), y);
	evas_object_show(back_popup);

	evas_object_smart_callback_del(m_backward_button, "clicked", __backward_clicked_cb);
}

void uri_bar::__close_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	webview *current_wv = m_browser->get_browser_view()->get_current_webview();
	webview *replace_wv = m_browser->get_webview_list()->delete_webview(current_wv);
	m_browser->get_browser_view()->set_current_webview(replace_wv);

	__context_popup_dismissed_cb(data, obj, event_info);
}

Eina_Bool uri_bar::__back_longpressed_timer_cb(void *data)
{
	uri_bar *ub = (uri_bar *)data;
	ub->m_back_longpressed_timer = NULL;

	ub->_show_longpress_back_popup(ub->m_backward_button);
	return ECORE_CALLBACK_CANCEL;
}

void uri_bar::__backward_pressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	uri_bar *ub = (uri_bar *)data;
	if (ub->m_back_longpressed_timer) {
		ecore_timer_del(ub->m_back_longpressed_timer);
		ub->m_back_longpressed_timer = NULL;
	}

	ub->m_back_longpressed_timer = ecore_timer_add(elm_config_longpress_timeout_get(), __back_longpressed_timer_cb, ub);
}

void uri_bar::__backward_unpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	uri_bar *ub = (uri_bar *)data;
	if (ub->m_back_longpressed_timer) {
		ecore_timer_del(ub->m_back_longpressed_timer);
		ub->m_back_longpressed_timer = NULL;
	}
}

void uri_bar::__forward_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	wv->forward();
}

Evas_Object *uri_bar::_create_main_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}

	elm_layout_file_set(layout, uri_bar_edj_path, "uri-bar-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *menu = elm_button_add(layout);
	if (!menu) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(menu, "browser/toolbar_menu");
	evas_object_smart_callback_add(menu, "clicked", __menu_clicked_cb, this);
	elm_object_part_content_set(layout, "elm.swallow.menu_button", menu);

	m_multi_window_button = elm_button_add(layout);
	if (!m_multi_window_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_multi_window_button, "browser/toolbar_multi_window");
	evas_object_smart_callback_add(m_multi_window_button, "clicked", __multi_window_clicked_cb, NULL);
	elm_object_part_content_set(layout, "elm.swallow.multi_window_button", m_multi_window_button);

	m_forward_button = elm_button_add(layout);
	if (!m_forward_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(m_forward_button, "browser/toolbar_forward");
	evas_object_smart_callback_add(m_forward_button, "clicked", __forward_clicked_cb, NULL);
	elm_object_part_content_set(layout, "elm.swallow.forward_button", m_forward_button);

	m_backward_button = elm_button_add(layout);
	if (!m_backward_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}

	elm_object_style_set(m_backward_button, "browser/toolbar_backward");

	evas_object_smart_callback_add(m_backward_button, "clicked", __backward_clicked_cb, NULL);
	evas_object_smart_callback_add(m_backward_button, "pressed", __backward_pressed_cb, this);
	evas_object_smart_callback_add(m_backward_button, "unpressed", __backward_unpressed_cb, this);
	elm_object_part_content_set(layout, "elm.swallow.backward_button", m_backward_button);

	m_uri_entry_layout = _create_entry_layout(layout);
	elm_object_part_content_set(layout, "elm.swallow.entry_layout", m_uri_entry_layout);

	return layout;
}

Evas_Object *uri_bar::_create_entry_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *entry_layout = elm_layout_add(parent);
	if (!entry_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(entry_layout, uri_bar_edj_path, "entry-layout");
	evas_object_size_hint_weight_set(entry_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	edje_object_signal_callback_add(elm_layout_edje_get(entry_layout), "mouse,clicked,1", "uri_text,touch_area", __uri_entry_clicked_cb, this);
	edje_object_signal_callback_add(elm_layout_edje_get(entry_layout), "mouse,clicked,1", "reload_icon,touch_area", __reload_stop_clicked_cb, this);

	return entry_layout;
}

