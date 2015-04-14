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

#include "HomeGenerator.h"
#include <boost/format.hpp>
#include <boost/any.hpp>
#include <sstream>
#include <fstream>
#include "BrowserLogger.h"
namespace tizen_browser
{
namespace services
{

EXPORT_SERVICE(HomeGenerator, "org.tizen.browser.HomeGenerator")

HomeGenerator::HomeGenerator()
    :maxBooksInRow(4)
{
    config.load("whatever");
    getBookmarks();
    /* <base href..> have to be finished with '/' if ther will be no '/' at the end of the link
     * no resources will be loaded
     */
    m_pageTemplate="<html>\n"
                   "<head>\n"
                   "    <title> Home </title>\n"
                   "    <base href=\"file://%1%\" />\n"
                   "    <link rel=\"Stylesheet\" type=\"text/css\" href=\"HomePage.css\" />\n"
                   "    <meta http-equiv=\"Content-Type\" conent=\"text/html\"; charset=utf-8\"/>\n"
                   "    <meta http-equiv=\"Pragma\" content=\"no-cache\" />"
                   "<script type=\"text/javascript\">window.lineElementsNo=%2%;</script>\n"
                   "<script type=\"text/javascript\" src=\"jquery-1.11.0.min.js\"></script>\n"
                   "<script type=\"text/javascript\" src=\"mousetrap.min.js\"></script>\n"
                   "<script type=\"text/javascript\" src=\"navi.js\"></script>\n"
                   "</head>\n"
                   "<body>\n"
                   "<div class=\"main\">\n"
                   "%3%"
                   "</div>\n"
                   "</body>\n"
                   "</html>\n";

    m_divTemplate="    <div class=bookmark>\n"
                  "        <a href=\"%1%\">\n"
                  "            <img class=\"thumb\" src=\"%2%\"/>\n"
                  "        </a>\n"
                  "        <div class=\"description\">\n"
                  "            <img class=\"ico\" src=\"%3%\"/>\n"
                  "            <h1>%4%</h1>\n"
                  "        </div>\n"
                  "    </div>\n";
    m_outPage.clear();
}

HomeGenerator::~HomeGenerator()
{

}

void HomeGenerator::preparePage()
{
    BROWSER_LOGD("preparing HomePage");
    getBookmarks();
    auto end = m_bookmarks.end();
    std::stringstream mainDiv;
    int i=1;
    for(auto bookmark=m_bookmarks.begin(); bookmark != end; bookmark++, i++){
        mainDiv << boost::format(m_divTemplate) % bookmark->url % bookmark->thumb_path % bookmark->favicon_path % bookmark->title;
        mainDiv << std::endl;
    }
    m_outPage = boost::str(boost::format(m_pageTemplate) % boost::any_cast<std::string>(config.get("resource/dir")) % maxBooksInRow % mainDiv.str());
}

std::string HomeGenerator::getHomePageUrl()
{
    std::string homePageUrl("/tmp/HomePage.html");
    if(m_outPage.length()==0){
        preparePage();
        std::fstream fileOut;
        fileOut.open(getHomePageUrl(), std::fstream::out | std::fstream::trunc);
        fileOut << m_outPage;
        fileOut.close();
        m_outPage.clear();
    }
    return homePageUrl;
}

std::string HomeGenerator::getHomePage()
{
    if(m_outPage.length()==0){
        preparePage();
    }
    return m_outPage;
}

void HomeGenerator::getBookmarks()
{
    m_bookmarks.clear();

    BookmarkItem b1, b2, b3, b4, b5, b6, b7, b8;
    b1.url          = "http://www.koreatimes.com/";
    b1.thumb_path   = "06.png";
    b1.favicon_path = "06.ico";
    b1.title        = "The Korea Times";
    //b1.title        = "한국 일보";

    b2.url          = "https://www.google.com/";
    b2.thumb_path   = "02.png";
    b2.favicon_path = "02.ico";
    b2.title        = "Google";

    b3.url          = "http://www.thetimes.co.uk/";
    b3.thumb_path   = "07.png";
    b3.favicon_path = "07.ico";
    b3.title        = "The Times";

    b4.url = "http://www.baidu.com/";
    b4.thumb_path = "baidu.png";
    b4.favicon_path = "baidu.ico";
    b4.title = "Baidu";

    b5.url = "http://www.facebook.com/";
    b5.thumb_path   = "facebook.png";
    b5.favicon_path = "facebook.ico";
    b5.title = "Facebook";

    b6.url = "http://www.daum.net/";
    b6.thumb_path   = "daum.png";
    b6.favicon_path = "daum.ico";
    b6.title = "Daum";

    b7.url = "http://www.ms.com/";
    b7.thumb_path   = "ms.png";
    b7.favicon_path = "ms.ico";
    b7.title = "Morgan Stanley";

    b8.url = "http://www.nate.com/";
    b8.thumb_path   = "nate.png";
    b8.favicon_path = "nate.ico";
    b8.title = "Nate";



//     for (int i=0; i< 10 ; i++){
#if 0
        //probably requires move constructor
        m_bookmarks.emplace_back<BookmarkItem>(b1);
        m_bookmarks.emplace_back<BookmarkItem>(b2);
        m_bookmarks.emplace_back<BookmarkItem>(b3);
        m_bookmarks.emplace_back<BookmarkItem>(b4);
#else
        m_bookmarks.push_back(b1);
        m_bookmarks.push_back(b2);
        m_bookmarks.push_back(b3);
        m_bookmarks.push_back(b4);

        m_bookmarks.push_back(b5);
        m_bookmarks.push_back(b6);
        m_bookmarks.push_back(b7);
        m_bookmarks.push_back(b8);
#endif
//     }


}


} /* end of namespace services*/
} /* end of namespace tizen_browser */
