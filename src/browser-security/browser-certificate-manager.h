/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#ifndef BROWSER_CERTIFICATE_MANAGER_H
#define BROWSER_CERTIFICATE_MANAGER_H

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include <string>

#include "browser-common-view.h"
#include "browser-config.h"

using namespace std;

class Browser_Certificate_Manager : public Browser_Common_View {
public:
	Browser_Certificate_Manager(void);
	~Browser_Certificate_Manager(void);

	Eina_Bool init(void);
	void reset_certificate(void);

	typedef enum {
		UNKNOWN_CA	  = (1 << 0),
		BAD_IDENTITY  = (1 << 1),
		NOT_ACTIVATED = (1 << 2),
		EXPIRED	  = (1 << 3),
		REVOKED	  = (1 << 4),
		INSECURE	  = (1 << 5),
		GENERIC_ERROR = (1 << 6),
		VALIDATE_ALL  = 0x007f
	} certificate_error_code;
private:
	void _destroy_certificate_list(void);
	Eina_Bool _create_crt_file(const char *dir_path, const char *des_file_path);
	Eina_Bool _create_certificate_list(Eina_List *certificate_list);
	gnutls_datum_t *_create_certificate(char *certificate_source);
	void _print_certificate(const gnutls_datum_t *certificate);

	static Eina_Bool __certificate_confirm_cb(Eina_Bool is_trused, const char *uri,
							char *certificate_source, int error);

	Eina_List *m_certificate_list;
	gnutls_datum_t* m_certificate;
};

#endif /* BROWSER_CERTIFICATE_MANAGER_H */

