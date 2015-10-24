/*
 * Copyright 2013  Samsung Electronics Co., Ltd
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
 * Contact: Mahesh Domakuntla <mahesh.d@samsung.com>
 *
 */

#ifndef HISTORY_LISTENER_H
#define HISTORY_LISTENER_H

#include "history-item.h"

class history_listener{
public:
	virtual void __history_added(history_item *item) {};
	virtual void __history_deleted(const char *uri) {};
	virtual void __history_cleared(Eina_Bool is_cancelled) {};
};

#endif  /* HISTORY_LISTENER_H */

