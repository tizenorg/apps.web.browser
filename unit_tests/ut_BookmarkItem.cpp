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

#include <string>

#include <boost/test/unit_test.hpp>
#include <boost/any.hpp>

#include "BrowserLogger.h"
#include "BookmarkItem.h"
#include "BrowserImage.h"

BOOST_AUTO_TEST_SUITE(bookmark_item)

BOOST_AUTO_TEST_CASE(bookm_item_set_get)
{
    BROWSER_LOGI("[UT] BookmarkItem - bookm_item_set_get - START --> ");

    std::string retstr = "";
    std::unique_ptr<tizen_browser::services::BookmarkItem>
	  bitem(new tizen_browser::services::BookmarkItem());

    //Check empty address and title
    bitem->setAddress("");
    retstr= bitem->getAddress();
    BOOST_CHECK_EQUAL("",retstr);
    bitem->setTittle("");
    retstr= bitem->getTittle();
    BOOST_CHECK_EQUAL("",retstr);

    //Check non empty address and title
    bitem->setAddress("www.address.com");
    retstr= bitem->getAddress();
    BOOST_CHECK_EQUAL("www.address.com",retstr);
    bitem->setTittle("Page Title");
    retstr= bitem->getTittle();
    BOOST_CHECK_EQUAL("Page Title",retstr);

    //Check set/get id
    bitem->setId(0);
    BOOST_CHECK_EQUAL(0, bitem->getId());
    bitem->setId(9999);
    BOOST_CHECK_EQUAL(9999, bitem->getId());

    BROWSER_LOGI("[UT] --> END - BookmarkItem - bookm_item_set_get");
}

BOOST_AUTO_TEST_CASE(bookm_item_favicon_thumb)
{
    BROWSER_LOGI("[UT] BookmarkItem - bookm_item_favicon_thumb - START --> ");

    std::unique_ptr<tizen_browser::services::BookmarkItem>
	  bitem(new tizen_browser::services::BookmarkItem());
    std::shared_ptr<tizen_browser::tools::BrowserImage> bimg
	  = std::make_shared<tizen_browser::tools::BrowserImage>();

    bimg->width = 10;
    bimg->height = 10;
    bimg->dataSize = 500;

    bitem->setFavicon(bimg);
    BOOST_CHECK_EQUAL(10, bitem->getFavicon()->width);
    BOOST_CHECK_EQUAL(10, bitem->getFavicon()->height);
    BOOST_CHECK_EQUAL(500, bitem->getFavicon()->dataSize);

    bitem->setThumbnail(bimg);
    BOOST_CHECK_EQUAL(10, bitem->getThumbnail()->width);
    BOOST_CHECK_EQUAL(10, bitem->getThumbnail()->height);
    BOOST_CHECK_EQUAL(500, bitem->getThumbnail()->dataSize);

    BROWSER_LOGI("[UT] --> END - BookmarkItem - bookm_item_favicon_thumb");
}

BOOST_AUTO_TEST_SUITE_END()

