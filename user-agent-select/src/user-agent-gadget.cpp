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

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#include <Elementary.h>

#include <app_control.h>
#include <dlog.h>
#include <stdlib.h>
#include <ui-gadget-module.h>

#include "user-agent-gadget.h"
#include "user-agent-view.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static Evas_Object *_create_bg(Evas_Object *parent, struct ug_data *ugd)
{
    Evas_Object *bg = elm_bg_add(parent);
    if (!bg) {
	SLOGE("_create_bg failed");
	return NULL;
    }
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
//    elm_win_resize_object_add(parent, bg);
    evas_object_show(bg);

    return bg;
}

static Evas_Object *_create_fullview(Evas_Object *parent, struct ug_data *ugd)
{
	Evas_Object *base = elm_layout_add(parent);
	if (!base) {
		SLOGE("elm_layout_add failed");
		return NULL;
	}

	elm_layout_theme_set(base, "layout", "application", "default");
	evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(base, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(base);

	return base;

}

static Evas_Object *_create_content(Evas_Object *parent, struct ug_data *ugd)
{
	if (ugd) {
		User_Agent_View *user_agent_view = new(nothrow) User_Agent_View(parent, ugd->ug);
		if (!user_agent_view) {
			SLOGE("new(nothrow) UAView failed");
			return NULL;
		}
		if (!user_agent_view->init()) {
			SLOGE("user_agent_view->init failed");
			delete user_agent_view;
			return NULL;
		}
		return user_agent_view->get_layout();
	}

	return NULL;
}

static void *on_create(ui_gadget_h ug, enum ug_mode mode, app_control_h data, void *priv)
{
	SLOGE("create");

	Evas_Object *parent;
	Evas_Object *content;
	struct ug_data *ugd;

	if (!ug || !priv) {
		SLOGE("ug or priv data is null");
		return NULL;
	}

	bindtextdomain(PKG_NAME, "/usr/ug/res/locale");

	ugd = (struct ug_data *)priv;
	ugd->ug = ug;

	parent = (Evas_Object *)ug_get_window();
	if (!parent) {
		SLOGE("ug_get_window is null");
		return NULL;
	}

	ugd->bg = _create_bg(parent, ugd);
	if (!ugd->bg) {
		SLOGE("_create_bg failed");
	}

	if (mode == UG_MODE_FULLVIEW) {
		ugd->base = _create_fullview(parent, ugd);
		elm_object_part_content_set(ugd->base, "elm.swallow.bg", ugd->bg);
	} else {
		SLOGE("[ug-browser-user-agnet-efl] Not supported frameview mode!");
		ugd->base = NULL;
	}

	if (ugd->base) {
		content = _create_content(parent, ugd);
		elm_object_part_content_set(ugd->base, "elm.swallow.content", content);
	}

	return ugd->base;
}

static void on_start(ui_gadget_h ug, app_control_h data, void *priv)
{
}

static void on_pause(ui_gadget_h ug, app_control_h data, void *priv)
{
	SLOGE("on_pause");
}

static void on_resume(ui_gadget_h ug, app_control_h data, void *priv)
{
	SLOGE("on_resume");
}

static void on_destroy(ui_gadget_h ug, app_control_h data, void *priv)
{
	struct ug_data *ugd;

	if (!ug || !priv)
		return;

	ugd = (struct ug_data *)priv;
	evas_object_del(ugd->base);
	evas_object_del(ugd->bg);

	ugd->base = NULL;
}

static void on_message(ui_gadget_h ug, app_control_h msg, app_control_h data, void *priv)
{
}

static void on_event(ui_gadget_h ug, enum ug_event event, app_control_h data, void *priv)
{
	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		break;
	case UG_EVENT_LOW_BATTERY:
		break;
	case UG_EVENT_LANG_CHANGE:
		break;
	case UG_EVENT_ROTATE_PORTRAIT:
		break;
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
		break;
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		break;
	default:
		break;
	}
}

static void on_key_event(ui_gadget_h ug, enum ug_key_event event, app_control_h data, void *priv)
{
	if (!ug)
		return;
	SLOGD("\nkey : %d", event);

	switch (event) {
		case UG_KEY_EVENT_END:
			break;
		default:
			break;
	}
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	struct ug_data *ugd;

	if (!ops)
		return -1;

	ugd = (struct ug_data *)calloc(1, sizeof(struct ug_data));
	if (!ugd)
		return -1;

	ops->create = on_create;
	ops->start = on_start;
	ops->pause = on_pause;
	ops->resume = on_resume;
	ops->destroy = on_destroy;
	ops->message = on_message;
	ops->event = on_event;
	ops->key_event = on_key_event;
	ops->priv = ugd;
	ops->opt = UG_OPT_INDICATOR_ENABLE;

	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	struct ug_data *ugd;

	if (!ops)
		return;

	ugd = (struct ug_data *)ops->priv;
	if (ugd)
		free(ugd);
}
#ifdef __cplusplus
}

#endif /* __cplusplus */
