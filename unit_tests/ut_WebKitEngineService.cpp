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

#include <boost/test/unit_test.hpp>

#include "ServiceManager.h"
#include "AbstractWebEngine.h"

BOOST_AUTO_TEST_SUITE(WebKitEngineService)

BOOST_AUTO_TEST_CASE(UriSetGet)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    webkitEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webkitEngineService->addTab("www.test.com");

    BOOST_CHECK(!(webkitEngineService->getLayout()));

    webkitEngineService->setURI("www.test2.com");

    // URIs are different because of WebKit didn't load webpage (lack of initialization)
    BOOST_TEST_MESSAGE("Print getURI():" << webkitEngineService->getURI());

    BOOST_TEST_MESSAGE("Print getTitle():" << webkitEngineService->getTitle());

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab());
}

BOOST_AUTO_TEST_CASE(NavigationTest)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    webkitEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webkitEngineService->addTab("www.test2.com");

    webkitEngineService->stopLoading();

    webkitEngineService->reload();

    webkitEngineService->setURI("www.nextpage.com");

    BOOST_TEST_MESSAGE("Is back enabled: " << webkitEngineService->isBackEnabled());

    webkitEngineService->back();

    BOOST_TEST_MESSAGE("Is forward enabled: " << webkitEngineService->isForwardEnabled());

    webkitEngineService->forward();

    BOOST_TEST_MESSAGE("Is loading: " << webkitEngineService->isLoading());

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab());
}

BOOST_AUTO_TEST_CASE(ClearPrivateData)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    tizen_browser::basic_webengine::TabId parentTabId = webkitEngineService->addTab("www.clearingdata.com");

    webkitEngineService->clearPrivateData();

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab());
}

BOOST_AUTO_TEST_CASE(TabsCreationDeletion)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    BOOST_TEST_MESSAGE("Tabs count: " << webkitEngineService->tabsCount());

    tizen_browser::basic_webengine::TabId first = webkitEngineService->addTab("www.first.com");

    BOOST_CHECK(webkitEngineService->tabsCount() == 1);

    tizen_browser::basic_webengine::TabId second = webkitEngineService->addTab("www.second.com");

    BOOST_CHECK(webkitEngineService->tabsCount() == 2);

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab(first));

    BOOST_CHECK(webkitEngineService->tabsCount() == 1);

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab(second));

    BOOST_CHECK(!(webkitEngineService->tabsCount()));
}

BOOST_AUTO_TEST_CASE(TabsSwitching)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    BOOST_TEST_MESSAGE("Tabs count: " << webkitEngineService->tabsCount());

    BOOST_TEST_MESSAGE("Initial current tab: " << webkitEngineService->currentTabId().toString());

    tizen_browser::basic_webengine::TabId first = webkitEngineService->addTab("www.first.com");
    BOOST_TEST_MESSAGE("First tab: " << first.toString());

    webkitEngineService->switchToTab(first);

    BOOST_CHECK(webkitEngineService->currentTabId() == first);

    BOOST_CHECK(webkitEngineService->tabsCount() == 1);

    tizen_browser::basic_webengine::TabId second = webkitEngineService->addTab("www.second.com");
    BOOST_TEST_MESSAGE("Second tab: " << second.toString());

    BOOST_CHECK(webkitEngineService->currentTabId() == first);

    webkitEngineService->switchToTab(second);

    BOOST_CHECK(webkitEngineService->currentTabId() == second);

    BOOST_CHECK(webkitEngineService->tabsCount() == 2);

    auto listTabs = webkitEngineService->listTabs();

    BOOST_CHECK(find(listTabs.begin(), listTabs.end(), first) != listTabs.end());

    BOOST_CHECK(find(listTabs.begin(), listTabs.end(), second) != listTabs.end());

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab(second));

    BOOST_CHECK(webkitEngineService->currentTabId() == first);

    BOOST_CHECK(webkitEngineService->tabsCount() == 1);

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab(first));

    BOOST_CHECK(!(webkitEngineService->tabsCount()));
}

BOOST_AUTO_TEST_CASE(Snapshots)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    webkitEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webkitEngineService->addTab("www.test.com");

    webkitEngineService->getSnapshotData(100, 100);

    webkitEngineService->getSnapshotData(parentTabId, 100, 100);

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab());
}

BOOST_AUTO_TEST_CASE(PrivateModeOnOff)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    webkitEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webkitEngineService->addTab("www.test.com");

    BOOST_CHECK(!(webkitEngineService->isPrivateMode()));

    webkitEngineService->setPrivateMode(true);

    BOOST_CHECK(webkitEngineService->isPrivateMode());

    webkitEngineService->setPrivateMode(false);

    BOOST_CHECK(!(webkitEngineService->isPrivateMode()));

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab());
}

BOOST_AUTO_TEST_CASE(LoadErrorDefaultValue)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    webkitEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webkitEngineService->addTab("www.test.com");

    BOOST_CHECK(!(webkitEngineService->isLoadError()));

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab());
}

BOOST_AUTO_TEST_CASE(Focus)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    webkitEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webkitEngineService->addTab("www.test.com");

    BOOST_TEST_MESSAGE("Has focus at start:" << webkitEngineService->hasFocus());

    webkitEngineService->setFocus();

    BOOST_TEST_MESSAGE("Has focus after setting focus:" << webkitEngineService->hasFocus());

    webkitEngineService->clearFocus();

    BOOST_TEST_MESSAGE("Has focus after clearing focus:" << webkitEngineService->hasFocus());

    webkitEngineService->setFocus();

    BOOST_TEST_MESSAGE("Has focus after setting focus:" << webkitEngineService->hasFocus());

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab());
}

BOOST_AUTO_TEST_CASE(ZoomAndAutofit)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    webkitEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webkitEngineService->addTab("www.test.com");

    BOOST_TEST_MESSAGE("Zoom factor at start:" << webkitEngineService->getZoomFactor());

    webkitEngineService->setZoomFactor(200);

    BOOST_TEST_MESSAGE("Zoom factor after setting 200:" << webkitEngineService->getZoomFactor());

    webkitEngineService->setZoomFactor(100);

    BOOST_TEST_MESSAGE("Zoom factor after setting 100:" << webkitEngineService->getZoomFactor());

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab());
}

BOOST_AUTO_TEST_CASE(Favicon)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    webkitEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webkitEngineService->addTab("www.test.com");

    webkitEngineService->getFavicon();

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab());
}


BOOST_AUTO_TEST_CASE(SearchOnWebsite)
{
    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webkitEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webkitengineservice"));

    BOOST_CHECK(webkitEngineService);

    webkitEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webkitEngineService->addTab("www.test.com");

    webkitEngineService->searchOnWebsite("test", 0);

    webkitEngineService->searchOnWebsite("test2", 0);

    webkitEngineService->searchOnWebsite("", 0);

    BOOST_TEST_MESSAGE("Print closeTab():" << webkitEngineService->closeTab());
}

BOOST_AUTO_TEST_SUITE_END()
