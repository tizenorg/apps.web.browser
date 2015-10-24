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

#include <Elementary.h>

class progress_bar {
public:
	progress_bar(Evas_Object *parent);
	~progress_bar(void);

	Evas_Object *get_layout(void) { return m_main_layout; }
	void show(void);
	void hide(void);
	void update_progress(double rate);

private:
	Evas_Object *_create_main_layout(Evas_Object *parent);

	Evas_Object *m_main_layout;
	Eina_Bool m_is_visible;
	int m_level;
};
