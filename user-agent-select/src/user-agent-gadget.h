/*
 *  ug-browser-user-agent-efl
 *
 * Copyright (c) 2012 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *              Sangpyo Kim <sangpyo7.kim@samsung.com>
 *              Junghwan Kang <junghwan.kang@samsung.com>
 *              Inbum Chang <ibchang@samsung.com>
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

#ifndef USER_AGENT_GADGET_H
#define USER_AGENT_GADGET_H

#include <Elementary.h>

#define PKG_NAME "ug-browser-user-agent-efl"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct ug_data {
	Evas_Object *bg;
	Evas_Object *base;
	ui_gadget_h ug;

	// PUT WHATEVER YOU NEED
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* USER_AGENT_GADGET_H */
