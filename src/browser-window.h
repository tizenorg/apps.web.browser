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

#ifndef BROWSER_WINODW_H
#define BROWSER_WINODW_H

#include "browser-config.h"
#include <cairo.h>

class Browser_Window {
public:
	Browser_Window(void);
	~Browser_Window(void);

	/* Caution : m_ewk_view can be null even though the Browser_Window is not null.
	  * Because the m_ewk_view is destroyed and assigned to null after Browser_Class::clean_up_windows.
	  * So The null check is necessary at every usage. */
	Evas_Object *m_ewk_view;

	Evas_Object *m_portrait_snapshot_image;
	Evas_Object *m_landscape_snapshot_image;

	/* m_parent is a Browser_Window which invoke itself by javascript etc. */
	Browser_Window *m_parent;
	Evas_Object *m_favicon;
	Evas_Object *m_option_header_favicon;
	Eina_Bool m_created_by_user;
	/* The url & title are only valid when the window is deleted
	  * because of unused case. (etc. low memory) */
	std::string m_url;
	std::string m_title;
};
#endif /* BROWSER_WINODW_H */

