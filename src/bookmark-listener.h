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
 * Contact: Naveena N <naveena.nag@samsung.com>
 *
 */

#ifndef BOOKMARK_LISTENER_H
#define BOOKMARK_LISTENER_H

class bookmark_listener{
public:
	virtual void __bookmark_added(const char *uri,  int bookmark_id, int parent_id) {};
	virtual void __bookmark_removed(const char *uri, int bookmark_id, int parent_id) {};
	virtual void __bookmark_updated(const char *uri, const char *title, int bookmark_id, int parent_id) {};
};

#endif  /* BOOKMARK_LISTENER_H */
