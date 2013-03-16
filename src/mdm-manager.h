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

#ifndef MDM_MANAGER_MANAGER_H
#define MDM_MANAGER_MANAGER_H

#include <mdm.h>

#include "browser-object.h"

class mdm_manager : public browser_object {
public:
	mdm_manager(void);
	~mdm_manager(void);

	void check_mdm_policy(void);
private:
	Eina_Bool _register_policy(void);
	Eina_Bool _deregister_policy(void);

	static void __policy_receiver_cb(int status, void *data);

	policy_receiver_handle m_policy_handle;
};

#endif /* MDM_MANAGER_MANAGER_H */

