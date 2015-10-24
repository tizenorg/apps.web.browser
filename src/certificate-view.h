/*
 *  browser
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Karthick R <karthick.r@samsung.com>
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

#ifndef CERTIFICATE_VIEW_H
#define CERTIFICATE_VIEW_H

#include <Evas.h>
#include <Elementary.h>
#include <vector>

#include "browser-object.h"
#include "common-view.h"
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <string>

typedef enum _certificate_field{
	ISSUED_TO_HEADER = 0,
	ISSUED_TO_CN,
	ISSUED_TO_ORG,
	ISSUED_TO_ORG_UNIT,
	ISSUED_TO_SERIAL_NO,
	ISSUED_BY_HEADER,
	ISSUED_BY_CN,
	ISSUED_BY_ORG,
	ISSUED_BY_ORG_UNIT,
	VALIDITY_HEADER,
	VALIDITY_ISSUED_ON,
	VALIDITY_EXPIRES_ON,
	FINGERPRINTS_HEADER,
	FINGERPRINTS_SHA_256_FP,
	FINGERPRINTS_SHA_1_FP,
	FIELD_END
} certificate_field;

typedef struct _genlist_callback_data {
	certificate_field type;
	const char *title;
	const char *value;
} genlist_callback_data;

class certificate_view : public common_view {
public:
	certificate_view(X509 *cert, Evas_Smart_Cb cb_func, void *cb_data);
	~certificate_view(void);
	void show(void);
	void show_certificate_info_popup(void);
private:
	void _parse_certificate();
	void _populate_certificate_field_data(char *data, certificate_field field);
	void _generate_genlist_data(certificate_field type, const char *title, const char *value);
	Evas_Object *_create_genlist(Evas_Object *parent);
	static char *__genlist_status_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static char * __genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Eina_Bool __naviframe_pop_cb(void *data, Elm_Object_Item *it);
	static void __certifciate_popup_cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	std::vector<genlist_callback_data *> m_genlist_callback_data_list;
	Evas_Object *m_genlist;
	Evas_Object *m_certificate_info_popup;
	Elm_Theme *m_theme;
	Elm_Object_Item *m_naviframe_item;
	X509 *m_certificate;
	Evas_Smart_Cb m_cb_func;
	void *m_cb_data;
};
#endif /* CERTIFICATE_VIEW_H */
