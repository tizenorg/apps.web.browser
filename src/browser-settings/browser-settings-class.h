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

#ifndef BROWSER_SETTINGS_CLASS_H
#define BROWSER_SETTINGS_CLASS_H

#include "browser-config.h"

class Browser_Settings_Main_View;

class Browser_Settings_Class {
public:
	Browser_Settings_Class(void);
	~Browser_Settings_Class(void);

	Eina_Bool init(void);
private:
	Browser_Settings_Main_View *m_settings_main_view;
};

#endif /* BROWSER_SETTINGS_CLASS_H */

