/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#ifndef __ABSTRACT_POPUP_H__
#define __ABSTRACT_POPUP_H__ 1

#include <boost/signals2/signal.hpp>

namespace tizen_browser {
namespace interfaces {

/**
 * @brief This is common popup interface
 */
class AbstractPopup
{
public:

    /**
     * @brief static method to create popup instance
     * @return popup instance
     */
    static AbstractPopup* createPopup();

    /**
     * @brief virtual method to show popup
     * Note that it may require to notify some popup manager
     */
    virtual void show() = 0;

    /**
     * @brief show notification signal
     */
    boost::signals2::signal<void (interfaces::AbstractPopup*)> popupShown;

    /**
     * @brief virtual method to close popup
     * Note that it may require to notify some popup manager
     */
    virtual void dismiss() = 0;

    /**
     * @brief dismiss notification signal
     */
    boost::signals2::signal<void (interfaces::AbstractPopup*)> popupDismissed;

    /**
     * @brief virtual method to handle back key pressed
     * Note that in most cases it will just call dismiss()
     */
    virtual void onBackPressed() = 0;

    virtual ~AbstractPopup() {};
};

} //iterfaces
} //tizen_browser

#endif // __ABSTRACT_POPUP_H__
