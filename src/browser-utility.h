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

#ifndef BROWSER_UTILITY_H
#define BROWSER_UTILITY_H

#include "browser-config.h"

#ifdef __cplusplus
extern "C" {
#endif

Evas_Object *br_elm_url_editfield_add(Evas_Object *parent);

Evas_Object *br_elm_editfield_add(Evas_Object *parent, Eina_Bool title = EINA_FALSE);

void br_elm_editfield_label_set(Evas_Object *obj, const char *label);

Evas_Object *br_elm_editfield_entry_get(Evas_Object *obj);

void br_elm_editfield_guide_text_set(Evas_Object *obj, const char *text);

void br_elm_editfield_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);

void br_elm_editfield_eraser_set(Evas_Object *obj, Eina_Bool visible);

Evas_Object *br_elm_searchbar_add(Evas_Object *parent);
void br_elm_searchbar_text_set(Evas_Object *obj, const char *text);
char *br_elm_searchbar_text_get(Evas_Object *obj);
Evas_Object *br_elm_searchbar_entry_get(Evas_Object *obj);


#ifdef __cplusplus
}
#endif

#endif /* SCISSORBOX_H*/

