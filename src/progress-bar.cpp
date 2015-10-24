/*
 * Copyright 2014  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contact: Kwangyong Choi <ky0.choi@samsung.com>
 *
 */

#include "progress-bar.h"

#include "browser-dlog.h"
#include "browser-object.h"

#define MAX_SIGNAL_NAME 32
#define PROGRESS_BAR_EDJ browser_edj_dir"/progress-bar.edj"
#define MAX_PROGRESS_LEVEL 20
#define PROGRESS_STEP 0.05

progress_bar::progress_bar(Evas_Object *parent)
	: m_main_layout(NULL)
	, m_is_visible(EINA_FALSE)
	, m_level(0)
{
	BROWSER_LOGD("");

	m_main_layout = _create_main_layout(parent);
}

progress_bar::~progress_bar(void)
{
	BROWSER_LOGD("");

	if (m_main_layout != NULL)
		evas_object_del(m_main_layout);
}

void progress_bar::show(void)
{
	BROWSER_LOGD("");

	if (m_is_visible == EINA_TRUE)
		return;

	m_is_visible = EINA_TRUE;
	m_level = 0;
	elm_object_signal_emit(m_main_layout, "show,progress,signal", "");
	elm_object_signal_emit(m_main_layout, "update,progress,0.00,signal", "");
}

void progress_bar::hide(void)
{
	BROWSER_LOGD("");

	if (m_is_visible == EINA_FALSE)
		return;

	m_is_visible = EINA_FALSE;
	elm_object_signal_emit(m_main_layout, "hide,progress,signal", "");
}

void progress_bar::update_progress(double rate)
{
	BROWSER_LOGD("%.2f", rate);

	if (m_is_visible == EINA_FALSE)
		return;

	int level = (int)(rate * MAX_PROGRESS_LEVEL);

	// Do not update progress if previous rate and current rate is same.
	if (level == m_level)
		return;

	// Do not display progress bar if rate is directly changed to 1 from 0.
	if (level == MAX_PROGRESS_LEVEL && m_level == 0) {
		hide();
		return;
	}

	m_level = level;

	char signal_name[MAX_SIGNAL_NAME] = { 0 };
	snprintf(signal_name, MAX_SIGNAL_NAME, "update,progress,%.02f,signal", ((double)level * PROGRESS_STEP));

	elm_object_signal_emit(m_main_layout, signal_name, "");
}

Evas_Object *progress_bar::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	Evas_Object *main_layout = elm_layout_add(parent);
	if (main_layout == NULL) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(main_layout, PROGRESS_BAR_EDJ, "progress-bar-layout");

	return main_layout;
}
