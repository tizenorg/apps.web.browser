
/*
 *  browser
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Tiwari Deepak <t.deepak@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef CERTIFICATE_MANAGER_H
#define CERTIFICATE_MANAGER_H

#include "browser-object.h"
#include "certificate-view.h"
#include "webview.h"

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <string>
#include <map>

using namespace std;
class webview;

enum HOST_TYPE {
	SECURE_HOST = 1,
	UNSECURE_HOST_ALLOWED,
	UNSECURE_HOST_UNKNOWN,
	UNSECURE_HOST_ASK,
	HOST_ABSENT = -1
};

class certificate_manager : public common_view {
public:
	enum CERT_TYPE {
		NONE,
		VALID,
		INVALID
	};

	certificate_manager(webview *wv);
	~certificate_manager(void);
	virtual void on_pause(void);
	void show_certificate_status_popup(Eina_Bool ask = EINA_TRUE);
	void destroy_certificate_status_popup(void);
	void set_certificate_data(const char *cert_data) { m_certificate_data.assign(cert_data); }
	void set_cert_type(CERT_TYPE type) { m_cert_type = type; }
	CERT_TYPE get_cert_type(void) const { return m_cert_type; }
	Eina_Bool __is_valid_certificate() const { return m_cert_type == VALID; }
	static Eina_Bool _save_certificate_info(const char *pem, const char *host, HOST_TYPE allow);
	static Eina_Bool _delete_all(void);
	static HOST_TYPE __is_exist_cert_for_host(const char *host);
	static void load_host_cert_info();
	static Eina_Bool _update_certificate_info(const char *pem, const char *host,HOST_TYPE allow);
private:
	Eina_Bool _create_certificate(const char *cert_data);
	std::string _get_certificate_status();
	static void __view_certificate_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
	static void __certificate_popup_lang_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_destroy_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_certificate_popup_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
	static void __cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __certi_allow_cb(void *data, Evas_Object *obj, void *event_info);
	static void __certi_deny_cb(void *data, Evas_Object *obj, void *event_info);
	static void __request_certi_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ok_button_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool inititialize_untrusted_certificate_data(void *data);

	static int m_instance_count;
	webview *m_webview;
	std::string m_certificate_data;
	X509 *m_certificate;
	Evas_Object *m_certificate_popup;
	Evas_Object *m_certificate_popup_layout;
	Ewk_Certificate_Policy_Decision *m_certi_policy;
	CERT_TYPE m_cert_type;
	static map<string,HOST_TYPE> m_host_cert_info;
	Ecore_Idler *m_untrusted_cert_data_idler;
	static const int STATUS_FONT_SIZE;
};

#endif /* CERTIFICATE_MANAGER_H */
