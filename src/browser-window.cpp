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

#include "browser-window.h"

Browser_Window::Browser_Window(void)
:
	m_ewk_view(NULL)
	,m_portrait_snapshot_image(NULL)
	,m_landscape_snapshot_image(NULL)
	,m_parent(NULL)
	,m_favicon(NULL)
	,m_option_header_favicon(NULL)
	,m_created_by_user(EINA_FALSE)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Window::~Browser_Window(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_ewk_view)
		evas_object_del(m_ewk_view);
	if (m_portrait_snapshot_image)
		evas_object_del(m_portrait_snapshot_image);
	if (m_landscape_snapshot_image)
		evas_object_del(m_landscape_snapshot_image);
	if (m_favicon)
		evas_object_del(m_favicon);
	if (m_option_header_favicon)
		evas_object_del(m_option_header_favicon);
}

