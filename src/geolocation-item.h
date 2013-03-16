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

#ifndef GEOLOCATION_ITEM_H
#define GEOLOCATION_ITEM_H

#include <Evas.h>
#include "browser-object.h"

class geolocation_item : public browser_object {
public:
	geolocation_item(const char *uri, bool accept = true, const char *title = "", const char *date = "");
	~geolocation_item(void);

	/* If the title & uri is same, they are equal. */
	friend bool operator==(geolocation_item item1, geolocation_item item2) {
		return ((!item1.get_uri() && !item2.get_uri()) || !strcmp(item1.get_uri(), item2.get_uri()));
	}

	const char *get_title(void) { return m_title; }
	const char *get_uri(void) { return m_uri; }
	const char *get_date(void) { return m_date; }
	Eina_Bool get_allow(void) { return m_accept; }
	void set_title(const char *title);
	void set_uri(const char *uri);
	void set_date(const char *date);

private:
	const char *m_uri;
	const char *m_title;
	const char *m_date;
	Eina_Bool m_accept;
	int m_counter;
};

#endif /* GEOLOCATION_ITEM_H */
