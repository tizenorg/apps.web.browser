/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "NetworkErrorHandler.h"

#if PLATFORM(TIZEN)
#include <network/net_connection.h>
#endif

namespace tizen_browser
{
namespace basic_ui
{

NetworkErrorHandler::NetworkErrorHandler()
{
#if PLATFORM(TIZEN)
    connection_create(&connection);
    connection_set_type_changed_cb(connection, onConnectionStateChanged, this);
#endif
}

NetworkErrorHandler::~NetworkErrorHandler()
{
#if PLATFORM(TIZEN)
    connection_unset_type_changed_cb(connection);
    connection_destroy(connection);
#endif
}

#if PLATFORM(TIZEN)
void NetworkErrorHandler::onConnectionStateChanged(connection_type_e type, void* user_data)
{
    NetworkErrorHandler *self = static_cast<NetworkErrorHandler*>(user_data);
    if(type == CONNECTION_TYPE_DISCONNECTED){
        self->networkError();
    }else{
        self->networkConnected();
    }
}
#endif

} /* end of namespace basic_ui */
} /* end of namespace tizen_browser */
