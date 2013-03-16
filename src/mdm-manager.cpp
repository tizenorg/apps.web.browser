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

#include <Elementary.h>
#include <bundle.h>
#include <syspopup_caller.h>

#include "mdm-manager.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "preference.h"
#include "webview.h"
#include "webview-list.h"

mdm_manager::mdm_manager(void)
:
	m_policy_handle(0)
{
	BROWSER_LOGD("");
	_register_policy();
}

mdm_manager::~mdm_manager(void)
{
	BROWSER_LOGD("");
	_deregister_policy();
}

static void _exit_browser(void)
{
	BROWSER_LOGD("");
	bundle *b = bundle_create();
	bundle_add(b, "_SYSPOPUP_CONTENT_", BR_STRING_MSG_MDM_POLICY);
	syspopup_launch("mdm-syspopup", b);
	bundle_free(b);

	elm_exit();
}

void mdm_manager::__policy_receiver_cb(int status, void *data)
{
	BROWSER_LOGD("");
	if (status & MDM_POLICY_BROWSER_AUTOFILL_SETTING) {
		if (status & MDM_POLICY_BROWSER_NOTI_RESTRICTED)
			m_preference->set_mdm_auto_fill_status(MDM_RESTRICTED);
		else
			m_preference->set_mdm_auto_fill_status(MDM_ALLOWED);
	} else if (status & MDM_POLICY_BROWSER_COOKIES_SETTING) {
		if (status & MDM_POLICY_BROWSER_NOTI_RESTRICTED) {
			m_preference->set_mdm_cookies_status(MDM_RESTRICTED);
			m_webview_context->accept_cookie_enabled_set(EINA_FALSE);
		} else {
			m_preference->set_mdm_cookies_status(MDM_ALLOWED);
			m_webview_context->accept_cookie_enabled_set(m_preference->get_accept_cookies_enabled());
		}
	} else if (status & MDM_POLICY_BROWSER_FORCE_FRAUD_WARNING_SETTING) {
		if (status & MDM_POLICY_BROWSER_NOTI_RESTRICTED)
			m_preference->set_mdm_fraud_warning_status(MDM_RESTRICTED);
		else
			m_preference->set_mdm_fraud_warning_status(MDM_ALLOWED);
	} else if (status & MDM_POLICY_BROWSER_JAVASCRIPT_SETTING) {
		if (status & MDM_POLICY_BROWSER_NOTI_RESTRICTED) {
			m_preference->set_mdm_javascript_status(MDM_RESTRICTED);

			int count = m_browser->get_webview_list()->get_count();
			for (int i = 0 ; i < count ; i++)
				m_browser->get_webview_list()->get_webview(i)->javascript_enabled_set(EINA_FALSE);
		} else {
			m_preference->set_mdm_javascript_status(MDM_ALLOWED);

			int count = m_browser->get_webview_list()->get_count();
			for (int i = 0 ; i < count ; i++)
				m_browser->get_webview_list()->get_webview(i)->javascript_enabled_set(m_preference->get_javascript_enabled());
		}
	} else if (status & MDM_POLICY_BROWSER_POPUP_SETTING) {
		if (status & MDM_POLICY_BROWSER_NOTI_RESTRICTED) {
			m_preference->set_mdm_popup_status(MDM_RESTRICTED);

			int count = m_browser->get_webview_list()->get_count();
			for (int i = 0 ; i < count ; i++)
				m_browser->get_webview_list()->get_webview(i)->scripts_window_open_enabled_set(EINA_FALSE);
		} else {
			m_preference->set_mdm_popup_status(MDM_ALLOWED);

			int count = m_browser->get_webview_list()->get_count();
			for (int i = 0 ; i < count ; i++)
				m_browser->get_webview_list()->get_webview(i)->scripts_window_open_enabled_set(!m_preference->get_block_popup_enabled());
		}
	} else if (status == MDM_RESTRICTED) {
		_exit_browser();
	}
}

Eina_Bool mdm_manager::_register_policy(void)
{
	BROWSER_LOGD("");
	mdm_result_t result = mdm_get_service();
	if (result != MDM_RESULT_SUCCESS) {
		BROWSER_LOGE("mdm_get_service() is failed [0x%0x]", result);
		return EINA_FALSE;
	}
	m_policy_handle = mdm_register_policy_receiver(MDM_POLICY_ON_BROWSER, this, __policy_receiver_cb);
	if (m_policy_handle == MDM_RESULT_FAIL) {
		BROWSER_LOGE("mdm_register_policy_receiver() is failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool mdm_manager::_deregister_policy(void)
{
	mdm_result_t result = mdm_deregister_policy_receiver(m_policy_handle);
	if (result != MDM_RESULT_SUCCESS)
		BROWSER_LOGE("mdm_deregister_policy_receiver failed");

	result = mdm_release_service();
	if (result != MDM_RESULT_SUCCESS) {
		BROWSER_LOGE("mdm_release_service failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void mdm_manager::check_mdm_policy(void)
{
	mdm_status_t result = mdm_get_allow_browser();
	BROWSER_LOGD("result = %d", result);
	if (result == MDM_RESTRICTED)
		_exit_browser();
}
