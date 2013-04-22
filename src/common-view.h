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

#ifndef COMMON_VIEW_H
#define COMMON_VIEW_H

#include <Evas.h>
#include <account.h>

#include "browser-object.h"

typedef struct _popup_callback popup_callback;

class common_view : public browser_object {
public:
	common_view(void);
	~common_view(void);

	/* @ param:uri - uri which has specific scheme such as 'rtsp://', 'tel:' etc. */
	Eina_Bool handle_scheme(const char *uri);

	void show_content_popup(const char *title, Evas_Object *content_obj, const char *button1_text = NULL, Evas_Smart_Cb button1_func = NULL, const char *button2_text = NULL, Evas_Smart_Cb button2_func = NULL, void *data = NULL, Eina_Bool share = EINA_FALSE);
	Evas_Object *show_msg_popup(const char *title, const char *msg, const char *button1_text = NULL, Evas_Smart_Cb button1_func = NULL, const char *button2_text = NULL, Evas_Smart_Cb button2_func = NULL, void *data = NULL, const char *button3_text = NULL, Evas_Smart_Cb button3_func = NULL);
	void show_delete_popup(const char *title, const char *msg, const char *button1_text = NULL, Evas_Smart_Cb button1_func = NULL, const char *button2_text = NULL, Evas_Smart_Cb button2_func = NULL, void *data = NULL);
	void show_msg_popup(const char *msg, int timeout = 3);
	void show_noti_popup(const char *msg, int timeout = 3);
	void destroy_popup(Evas_Object *sub_obj);
	Eina_Bool launch_tizenstore(const char *uri);
	Eina_Bool launch_streaming_player(const char *uri, const char *cookie = NULL);
	Eina_Bool launch_email(const char *uri, Eina_Bool file_attach = EINA_FALSE);
	Eina_Bool launch_message(const char *uri, const char *receiver = NULL, Eina_Bool file_attach = EINA_FALSE);
	Eina_Bool launch_bluetooth(const char *file_path);
	Eina_Bool launch_contact(const char *phone_number);
	Eina_Bool launch_Snote(const char *uri);	
	Eina_Bool launch_phone_call(const char *number, Eina_Bool is_vt_call = EINA_FALSE);
	Evas_Object *get_naviframe(void) { return m_naviframe; }
protected:
	static Evas_Object *m_bg;
	static Evas_Object *m_naviframe;
	static Evas_Object *m_conformant;
private:
	void _show_tel_vtel_popup(const char *number);
	Eina_Bool _handle_intent_scheme(const char *uri);
	Eina_Bool _handle_tizen_service_scheme(const char *uri);
	Eina_Bool _handle_mailto_scheme(const char *uri);
	Eina_Bool _handle_unknown_scheme(const char *uri);
	Eina_Bool _make_html_for_BT_send(const char *url, const char *file_path);

	static void __button1_cb(void *data, Evas_Object *obj, void *event_info);
	static void __button2_cb(void *data, Evas_Object *obj, void *event_info);
	static void __button3_cb(void *data, Evas_Object *obj, void *event_info);
	static void __call_response_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __call_response_ok_cb(void *data, Evas_Object *obj, void *event_info);
	static void __share_email_cb(void *data, Evas_Object *obj, void *event_info);
	static void __share_message_cb(void *data, Evas_Object *obj, void *event_info);
	static void __share_sns_cb(void *data, Evas_Object *obj, void *event_info);
	static void __sns_cancel_cb(void *data, Evas_Object *obj, void *event_info);

	static bool _get_account_cb(account_h account, void *data);

	const char *m_share_uri;
	Eina_List *m_sns_list;
};

#endif /* COMMON_VIEW_H */

