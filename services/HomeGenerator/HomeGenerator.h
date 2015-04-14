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

#ifndef HOMEGENERATOR_H
#define HOMEGENERATOR_H


#include <list>
#include <string>

#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

#include "Config.h"

namespace tizen_browser
{

namespace services {

class BROWSER_EXPORT HomeGenerator: public tizen_browser::core::AbstractService
{
public:

    /**
     * @brief Create HomeGenerator class
     *
     */
    HomeGenerator();
    ~HomeGenerator();
    /**
     * @brief Returns url to HomePage
     *
     * @return std::string URL to Home page
     */
    std::string getHomePageUrl();
    /**
     * @brief Returns source of the html page;
     *
     * @return std::string
     */
    std::string getHomePage();

    virtual std::string getName();

    struct BookmarkItem {
        std::string url;            ///< page url.
        std::string title;          ///< page title.
        std::string thumb_path;     ///< path to thumbnail.
        std::string favicon_path;   ///< path to favicon.
    };
    typedef std::list<BookmarkItem> BookmarksList;
private:
    /**
     * @brief Generates page with bookmarks
     *
     */
    void preparePage();
    /**
     * @brief Reads list of bookmarks.
     *
     * Retrieves list of bookmarks from the bookmark storage.
     * Currently there's no real storage support so we generate our own list.
     */
    void getBookmarks();

    BookmarksList m_bookmarks;  ///< list of bookmarks
    std::string m_outPage;      ///< generated page
    std::string m_pageTemplate; ///< template for page
    std::string m_divTemplate;  ///< template for div
    int maxBooksInRow;          ///< whe to brake the line
    config::DefaultConfig config;
};

} /* end of namespace services*/
} /* end of namespace tizen_browser */

#endif // HOMEGENERATOR_H
