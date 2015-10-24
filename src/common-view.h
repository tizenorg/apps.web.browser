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

#ifndef COMMON_VIEW_H
#define COMMON_VIEW_H

#include <Evas.h>
#include <string>

#include "browser-object.h"

typedef enum {
	SIZE_INDEX_SMALL = 0,
	SIZE_INDEX_NORMAL,
	SIZE_INDEX_LARGE,
	SIZE_INDEX_HUGE,
	SIZE_INDEX_GIANT
} font_size_index;

class common_view : public browser_object {
public:
	common_view(void);
	~common_view(void);

	/* @ param:uri - uri which has specific scheme such as 'rtsp://', 'tel:' etc. */
	Eina_Bool handle_scheme(const char *uri);
	Eina_Bool _is_file_scheme(const char *uri);

	Evas_Object *show_content_popup(Evas_Object *popup, const char *title_text_id, Evas_Object *content_obj, Evas_Smart_Cb popup_destroy_func = NULL, const char *button1_text_id = NULL, Evas_Smart_Cb button1_func = NULL, const char *button2_text_id = NULL, Evas_Smart_Cb button2_func = NULL, void *data = NULL, Eina_Bool share = EINA_FALSE);
	Evas_Object *show_msg_popup(const char *title_text_id, const char *msg_text_id, Evas_Smart_Cb popup_destroy_func = NULL, const char *button1_text_id = NULL, Evas_Smart_Cb button1_func = NULL, const char *button2_text_id = NULL, Evas_Smart_Cb button2_func = 	NULL, void *data = NULL, const char *button3_text_id = NULL, Evas_Smart_Cb button3_func = NULL);
	void show_delete_popup(const char *title_text_id, const char *msg_text_id, Evas_Smart_Cb popup_destroy_func, const char *button1_text_id = NULL, Evas_Smart_Cb button1_func = NULL, const char *button2_text_id = NULL, Evas_Smart_Cb button2_func = NULL, void *data = NULL);
	void show_msg_popup(const char *msg, int timeout = 3, Evas_Smart_Cb func = NULL, void *data = NULL, Eina_Bool has_focus = EINA_TRUE);
	void show_noti_popup(const char *msg);
	void destroy_popup(Evas_Object *sub_obj);
	void destroy_msg_popup(void);
	void destroy_delete_popup(void);
	void destroy_content_popup(void);
	Elm_Object_Item *add_item_to_popup(Evas_Object *popup, const char *label_id, Evas_Object *icon = NULL, Evas_Smart_Cb func = NULL, const void *data = NULL);
	Eina_Bool launch_tizenstore(const char *uri);
	Eina_Bool launch_streaming_player(const char *uri, const char *cookie = NULL);
	Eina_Bool launch_audio_player(const char *uri, const char *cookie = NULL);
	Eina_Bool launch_message(const char *uri, const char *receiver = NULL, Eina_Bool file_attach = EINA_FALSE);
	Eina_Bool launch_bluetooth(const char *file_path);
	Eina_Bool launch_contact(const char *phone_number, const char *email_address = NULL);
	Eina_Bool launch_dialer(const char *phone_number);
	Eina_Bool launch_nfc(const char *uri);
	Eina_Bool launch_Snote(const char *uri);
	Eina_Bool launch_phone_call(const char *number, Eina_Bool is_vt_call = EINA_FALSE);
	Eina_Bool launch_cms_svc(const char *uri);
	Eina_Bool launch_sdp_svc(const char *uri);
	void share(const char *uri, const char *title);
	Evas_Object *get_naviframe(void) { return m_naviframe; }
	Evas_Object *get_window(void) { return m_window; }
	Evas_Object *get_conformant(void) { return m_conformant; }
	Evas_Object *get_elm_bg(void);
	void hide_common_view_popups(void);
	int genlist_popup_calculate_height(Evas_Object *genlist);
	int genlist_popup_calculate_height_with_margin(Evas_Object *genlist, popup_shape shape);
	virtual void on_pause(void);
	virtual void on_rotate(int degree);
	void set_trans_text_to_object(Evas_Object *obj, const char *text_id, const char* part);
	void set_trans_text_to_object_item(Elm_Object_Item *it, const char *text_id, const char* part);
	char *get_text_from_ID(const char *ID);
	virtual void clear_popups(void);
	void show_notification(const char* data, Evas_Object *obj, unsigned int length);
	std::string get_font_size_tag(void);
/* mailto, tel scheme handling */
	void show_email_scheme_popup(const char *uri);
	void show_phone_number_scheme_popup(const char *uri);
	void show_processing_popup(void);
	void close_processing_popup(void);

protected:
	static Evas_Object *m_bg;
	static Evas_Object *m_naviframe;
	static Evas_Object *m_conformant;

	Evas_Object *_create_title_icon_btn(Evas_Object *parent, Evas_Smart_Cb func, const char *icon_path, const char *icon_group, void *data);
	Evas_Object *_create_title_text_btn(Evas_Object *parent, const char *text_id, Evas_Smart_Cb func, void *data);
	Evas_Object *_create_title_text_btn(Evas_Object *parent, const char *text_id, Evas_Smart_Cb func, void *data, Eina_Bool is_left);
	Evas_Object *_create_box(Evas_Object *parent);
	Evas_Object *_create_tabbar(Evas_Object *parent);
	Evas_Object *_create_main_layout(Evas_Object *parent, const char *file);
	Evas_Object *_create_no_content(Evas_Object *parent, const char *text, const char *text_help);
	static void __genlist_lang_changed(void *data, Evas_Object * obj, void *event_info);

private:
	void _show_tel_vtel_popup(const char *number);
	Eina_Bool _handle_intent_scheme(const char *uri);
	Eina_Bool _handle_tizen_service_scheme(const char *uri);
	Eina_Bool _handle_mailto_scheme(const char *uri);
	Eina_Bool _handle_unknown_scheme(const char *uri);
	static void __button1_cb(void *data, Evas_Object *obj, void *event_info);
	static void __button2_cb(void *data, Evas_Object *obj, void *event_info);
	static void __button3_cb(void *data, Evas_Object *obj, void *event_info);
	static void __msg_button1_cb(void *data, Evas_Object *obj, void *event_info);
	static void __msg_button2_cb(void *data, Evas_Object *obj, void *event_info);
	static void __msg_button3_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_button1_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_button2_cb(void *data, Evas_Object *obj, void *event_info);
	static void __content_button1_cb(void *data, Evas_Object *obj, void *event_info);
	static void __content_button2_cb(void *data, Evas_Object *obj, void *event_info);
	static void __call_response_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __call_response_ok_cb(void *data, Evas_Object *obj, void *event_info);
#if defined(HW_MORE_BACK_KEY)
	static void __popup_hw_back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_timeout_hw_back_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static void __popup_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
	static void __msg_popup_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
	static void __list_popup_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
	static void __content_popup_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
	static void __scheme_popup_email_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scheme_popup_message_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scheme_popup_call_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scheme_popup_contact_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scheme_popup_copy_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scheme_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_freed_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

	const char *m_share_uri;
	static int m_view_count;
	Evas_Object *m_popup;
	Evas_Object *m_msg_popup;
	Evas_Object *m_popup_confirm;
	Evas_Object *m_content_popup;
	Evas_Object *m_context_popup;
	Evas_Object *m_popup_list;
	Evas_Object *m_box_of_popup_content;
	Evas_Object *m_genlist_of_popup_content;
	Evas_Object *m_app_in_app_popup_window;
	static Evas_Object *m_toast_popup;
	Evas_Object *m_popup_processing;
};

#endif /* COMMON_VIEW_H */

