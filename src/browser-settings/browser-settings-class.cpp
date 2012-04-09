/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#include "browser-settings-class.h"
#include "browser-settings-main-view.h"

Browser_Settings_Class::Browser_Settings_Class(void)
:
	m_settings_main_view(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_Class::~Browser_Settings_Class(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_settings_main_view)
		delete m_settings_main_view;
}

Eina_Bool Browser_Settings_Class::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_settings_main_view = new(nothrow) Browser_Settings_Main_View;
	if (!m_settings_main_view) {
		BROWSER_LOGE("new Browser_Settings_Main_View failed");
		return EINA_FALSE;
	}

	if (!m_settings_main_view->init()) {
		BROWSER_LOGE("m_settings_main_view->init() failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

