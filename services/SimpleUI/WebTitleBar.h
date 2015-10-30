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

#ifndef WEBTITLEBAR_H
#define WEBTITLEBAR_H

#include <Evas.h>

namespace tizen_browser
{
namespace base_ui
{

class WebTitleBar
{
public:
    WebTitleBar(Evas_Object * parent, const std::string &edjFile, const std::string &groupName);
    ~WebTitleBar();

    /**
     * @brief Return layout of the Web Title bar
     */
    Evas_Object * getContent() { return m_layout; };

    /**
     * @brief Show Web Title bar
     *
     * @param url title of webpage
     */
    void show(const std::string & title);

    /**
     * @brief Set favicon on Web tittle bar
     *
     * @param favicon Favicon image
     */
    void setFavIcon(std::shared_ptr<tizen_browser::tools::BrowserImage> favicon);

    /**
     * @brief remove favicon properly, and update view
     *
     */
    void removeFavIcon();

    /**
     * @brief Hide Web tittle bar
     */
    void hide();

private:
    void setTitle(const std::string & title);
    static Eina_Bool show_cb(void * data);
    static Eina_Bool hide_cb(void * data);
    Evas_Object * m_layout;
    Evas_Object * m_icon;
    Ecore_Timer * m_timer;

};

} /* end of base_ui */
} /* end of tizen_browser */

#endif // WEBTITLEBAR_H
