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

#include <Elementary.h>
#include <memory>
#include <boost/format.hpp>
#include "WebPageUI.h"
#include "BrowserLogger.h"
#include "ServiceManager.h"
#include "BrowserAssert.h"
#include "UrlHistoryList/UrlHistoryList.h"
#include "WebPageUIStatesManager.h"

namespace tizen_browser {
namespace base_ui {

EXPORT_SERVICE(WebPageUI, "org.tizen.browser.webpageui")

WebPageUI::WebPageUI()
    : m_parent(nullptr)
    , m_mainLayout(nullptr)
    , m_errorLayout(nullptr)
    , m_privateLayout(nullptr)
    , m_progressBar(nullptr)
    , m_bookmarkManagerButton(nullptr)
    , m_URIEntry(new URIEntry())
    , m_statesMgr(std::make_shared<WebPageUIStatesManager>(WPUState::MAIN_WEB_PAGE))
    , m_urlHistoryList(std::make_shared<UrlHistoryList>(getStatesMgr()))
    , m_webviewLocked(false)
    , m_WebPageUIvisible(false)
#if PROFILE_MOBILE
    , m_geastureLayer(nullptr)
    , m_uriBarHidden(false)
#endif
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

WebPageUI::~WebPageUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void WebPageUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* WebPageUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_mainLayout) {
        createLayout();
    }
    return m_mainLayout;
}

UrlHistoryPtr WebPageUI::getUrlHistoryList()
{
    return m_urlHistoryList;
}

void WebPageUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_mainLayout);
    evas_object_show(m_mainLayout);

    evas_object_show(elm_object_part_content_get(m_mainLayout, "web_view"));
    evas_object_show(m_URIEntry->getContent());
    evas_object_show(elm_object_part_content_get(m_mainLayout, "uri_bar_buttons_left"));
    evas_object_show(elm_object_part_content_get(m_mainLayout, "uri_bar_buttons_right"));

    if(m_statesMgr->equals(WPUState::QUICK_ACCESS))
        showQuickAccess();
    else
        m_URIEntry->showPageTitle();

    m_WebPageUIvisible = true;

    elm_object_event_callback_add(m_leftButtonBar->getContent(), _cb_down_pressed_on_urlbar, this);
    elm_object_event_callback_add(m_rightButtonBar->getContent(), _cb_down_pressed_on_urlbar, this);
    elm_object_event_callback_add(m_URIEntry->getContent(), _cb_down_pressed_on_urlbar, this);
#if PROFILE_MOBILE
    elm_gesture_layer_cb_add(m_geastureLayer, ELM_GESTURE_N_LINES, ELM_GESTURE_STATE_MOVE, _gesture_move, this);
    elm_object_signal_callback_add(m_mainLayout,  "animation_finished", "ui", _geasture_finished, this);
#endif
}


void WebPageUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_mainLayout);
    elm_object_focus_custom_chain_unset(m_mainLayout);
    evas_object_hide(m_mainLayout);

    if(m_statesMgr->equals(WPUState::QUICK_ACCESS))
        hideQuickAccess();

    evas_object_hide(elm_object_part_content_get(m_mainLayout, "web_view"));
    m_URIEntry->editingCanceled();
    evas_object_hide(m_URIEntry->getContent());
    evas_object_hide(elm_object_part_content_get(m_mainLayout, "uri_bar_buttons_left"));
    evas_object_hide(elm_object_part_content_get(m_mainLayout, "uri_bar_buttons_right"));

    m_WebPageUIvisible = false;

    elm_object_event_callback_del(m_leftButtonBar->getContent(), _cb_down_pressed_on_urlbar, this);
    elm_object_event_callback_del(m_rightButtonBar->getContent(), _cb_down_pressed_on_urlbar, this);
    elm_object_event_callback_del(m_URIEntry->getContent(), _cb_down_pressed_on_urlbar, this);
#if PROFILE_MOBILE
    elm_gesture_layer_cb_del(m_geastureLayer, ELM_GESTURE_N_LINES, ELM_GESTURE_STATE_MOVE, _gesture_move, this);
    elm_object_signal_callback_del(m_mainLayout,  "animation_finished", "ui", _geasture_finished);
#endif
}

void WebPageUI::loadStarted()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    showProgressBar();
    elm_object_signal_emit(m_URIEntry->getContent(), "shiftright_uribg", "ui");
    elm_object_signal_emit(m_mainLayout, "shiftright_uri", "ui");
    m_leftButtonBar->setActionForButton("refresh_stop_button", m_stopLoading);
}

void WebPageUI::progressChanged(double progress)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (progress == 1.0) {
        hideProgressBar();
    } else {
        elm_progressbar_value_set(m_progressBar, progress);
    }
}

bool WebPageUI::stateEquals(WPUState state) const
{
    return m_statesMgr->equals(state);
}

bool WebPageUI::stateEquals(std::initializer_list<WPUState> states) const
{
    return m_statesMgr->equals(states);
}

void WebPageUI::loadFinished()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "hide_progressbar_bg", "ui");
    m_leftButtonBar->setActionForButton("refresh_stop_button", m_reload);
    m_URIEntry->showPageTitle();
}

void WebPageUI::loadStopped()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_leftButtonBar->setActionForButton("refresh_stop_button", m_reload);
    m_URIEntry->setPageTitleFromURI();
    m_URIEntry->showPageTitle();
}

void WebPageUI::setPageTitle(const std::string& title)
{
     BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
     m_URIEntry->setPageTitle(title);
}

void WebPageUI::toIncognito(bool incognito)
{
    BROWSER_LOGD("[%s:%d,%d] ", __PRETTY_FUNCTION__, __LINE__, incognito);
    if (incognito) {
        elm_object_signal_emit(m_mainLayout, "incognito,true", "ui");
        elm_object_signal_emit(m_URIEntry->getEntryWidget(), "uri_entry_incognito", "ui");
    }
    else {
        elm_object_signal_emit(m_mainLayout, "incognito,false", "ui");
        elm_object_signal_emit(m_URIEntry->getEntryWidget(), "uri_entry_normal", "ui");
    }
}

void WebPageUI::setMainContent(Evas_Object* content)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(content);
    hideWebView();
    elm_object_part_content_set(m_mainLayout, "web_view", content);
    evas_object_show(content);
}

void WebPageUI::switchViewToErrorPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_statesMgr->set(WPUState::MAIN_ERROR_PAGE);
    setMainContent(m_errorLayout);
    evas_object_show(m_leftButtonBar->getContent());
    elm_object_signal_emit(m_mainLayout, "shiftright_uri", "ui");
    elm_object_signal_emit(m_URIEntry->getContent(), "shiftright_uribg", "ui");
    setErrorButtons();
    refreshFocusChain();
}

void WebPageUI::switchViewToIncognitoPage()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_statesMgr->set(WPUState::MAIN_INCOGNITO_PAGE);
    setMainContent(m_privateLayout);
    evas_object_show(m_leftButtonBar->getContent());
    elm_object_signal_emit(m_mainLayout, "shiftright_uri", "ui");
    elm_object_signal_emit(m_URIEntry->getContent(), "shiftright_uribg", "ui");
    setPrivateButtons();
    refreshFocusChain();
    m_URIEntry->changeUri("");
    m_URIEntry->setPageTitle("Start Incognito Browsing");
    m_URIEntry->setFocus();
}

void WebPageUI::switchViewToWebPage(Evas_Object* content, const std::string uri, const std::string title)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(m_statesMgr->equals(WPUState::QUICK_ACCESS))
    {
        hideQuickAccess();
        m_statesMgr->set(WPUState::MAIN_WEB_PAGE);
    }
    setMainContent(content);
    updateURIBar(uri);
    m_URIEntry->setPageTitle(title);
    m_URIEntry->showPageTitle();
    refreshFocusChain();
    evas_object_show(m_leftButtonBar->getContent());
    elm_object_signal_emit(m_mainLayout, "shiftright_uri", "ui");
    elm_object_signal_emit(m_URIEntry->getContent(), "shiftright_uribg", "ui");
    elm_object_signal_emit(m_urlHistoryList->getContent(), "switch_view_webpage", "ui");
    elm_object_focus_custom_chain_append(m_mainLayout, content, NULL);
}

void WebPageUI::switchViewToQuickAccess(Evas_Object* content)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_statesMgr->set(WPUState::QUICK_ACCESS);
    setMainContent(content);
    evas_object_hide(m_leftButtonBar->getContent());
    elm_object_signal_emit(m_mainLayout, "shiftback_uri", "ui");
    elm_object_signal_emit(m_URIEntry->getContent(), "shiftback_uribg", "ui");
    elm_object_signal_emit(m_urlHistoryList->getContent(), "switch_view_quickaccess", "ui");
    hideProgressBar();
    refreshFocusChain();
    m_URIEntry->changeUri("");
    m_URIEntry->setFocus();
    showQuickAccess();
}

void WebPageUI::faviconClicked(void* data, Evas_Object*, const char*, const char*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    WebPageUI* self = reinterpret_cast<WebPageUI*>(data);
    if(!self->stateEquals({ WPUState::QUICK_ACCESS, WPUState::MAIN_ERROR_PAGE })) {
        self->getURIEntry().clearFocus();
    }
}

Eina_Bool WebPageUI::_cb_down_pressed_on_urlbar(void *data, Evas_Object */*obj*/, Evas_Object */*src*/, Evas_Callback_Type type, void *event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    WebPageUI* self = reinterpret_cast<WebPageUI*>(data);
    if(type == EVAS_CALLBACK_KEY_DOWN) {
        Ecore_Event_Key *ev = static_cast<Ecore_Event_Key *>(event_info);
        const std::string keyName = ev->keyname;
        if(!keyName.compare("Down")){
            self->lockWebview();
        }
    }
    return EINA_FALSE;
}

void WebPageUI::setTabsNumber(int tabs)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (tabs == 0) {
        elm_object_part_text_set(m_rightButtonBar->getContent(), "tabs_number", "");
    } else {
        elm_object_part_text_set(m_rightButtonBar->getContent(), "tabs_number", (boost::format("%1%") % tabs).str().c_str());
    }
}

void WebPageUI::lockWebview()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(isWebPageUIvisible()) {
        if(m_statesMgr->equals(WPUState::MAIN_WEB_PAGE)) {
            elm_object_focus_custom_chain_unset(m_mainLayout);
            elm_object_focus_custom_chain_append(m_mainLayout, elm_object_part_content_get(m_mainLayout, "web_view"), NULL);
            m_webviewLocked = true;
        }
    }
}

void WebPageUI::lockUrlHistoryList()
{
    elm_object_focus_custom_chain_unset(m_mainLayout);
    elm_object_focus_custom_chain_append(m_mainLayout,
            getUrlHistoryList()->getContent(), NULL);
    getUrlHistoryList()->listWidgetFocusChangeTimerStart();
    elm_object_focus_set(getUrlHistoryList()->getContent(), EINA_TRUE);
}

void WebPageUI::unlockUrlHistoryList()
{
    refreshFocusChain();
    elm_object_focus_set(m_URIEntry->getEntryWidget(), EINA_TRUE);
    getUrlHistoryList()->onListWidgetFocusChange(false);
}

void WebPageUI::onRedKeyPressed()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(isWebPageUIvisible()) {
        if(m_statesMgr->equals(WPUState::MAIN_WEB_PAGE)) {
            if(m_webviewLocked) {
                refreshFocusChain();
                m_URIEntry->setFocus();
                m_webviewLocked = false;
            }
        }
    }
}

void WebPageUI::onYellowKeyPressed()
{
    if (!isWebPageUIvisible())
        return;

    Eina_Bool listVisible = evas_object_visible_get(
            getUrlHistoryList()->getGenlist());
    if(!listVisible) return;

    bool listFocused = getUrlHistoryList()->widgetFocused();
    if (listFocused) {
        unlockUrlHistoryList();
    } else {
        lockUrlHistoryList();
    }
}

void WebPageUI::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    // create web layout
    m_mainLayout = elm_layout_add(m_parent);
    evas_object_size_hint_weight_set(m_mainLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_layout_file_set(m_mainLayout, edjePath("WebPageUI/WebPageUI.edj").c_str(), "main_layout");

    createErrorLayout();
    createPrivateLayout();
    createActions();

    // left buttons
    m_leftButtonBar = std::unique_ptr<ButtonBar>(new ButtonBar(m_mainLayout, "WebPageUI/LeftButtonBar.edj", "left_button_bar"));
    m_leftButtonBar->addAction(m_back, "prev_button");
    m_leftButtonBar->addAction(m_forward, "next_button");
    m_leftButtonBar->addAction(m_reload, "refresh_stop_button");

    //register action that will be used later by buttons"
    m_leftButtonBar->registerEnabledChangedCallback(m_stopLoading, "refresh_stop_button");

    // right buttons
    m_rightButtonBar = std::unique_ptr<ButtonBar>(new ButtonBar(m_mainLayout, "WebPageUI/RightButtonBar.edj", "right_button_bar"));
    m_rightButtonBar->addAction(m_tab, "tab_button");
    m_rightButtonBar->addAction(m_showMoreMenu, "setting_button");

    // progress bar
    m_progressBar = elm_progressbar_add(m_mainLayout);
    elm_object_style_set(m_progressBar, "play_buffer");

    //URL bar (Evas Object is shipped by URIEntry object)
    m_URIEntry->init(m_mainLayout);
    elm_object_part_content_set(m_mainLayout, "uri_entry", m_URIEntry->getContent());
    elm_object_part_content_set(m_mainLayout, "uri_bar_buttons_left", m_leftButtonBar->getContent());
    elm_object_part_content_set(m_mainLayout, "uri_bar_buttons_right", m_rightButtonBar->getContent());

    elm_layout_signal_callback_add(m_URIEntry->getContent(), "slide_websearch", "elm", faviconClicked, this);

    elm_theme_extension_add(nullptr, edjePath("WebPageUI/UrlHistoryList.edj").c_str());
    m_urlHistoryList->setMembers(m_mainLayout, m_URIEntry->getEntryWidget());

    connectActions();

#if PROFILE_MOBILE
    m_geastureLayer = elm_gesture_layer_add(m_mainLayout);
    elm_gesture_layer_attach(m_geastureLayer, m_mainLayout);
#endif
}

void WebPageUI::createErrorLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_errorLayout =  elm_layout_add(m_mainLayout);
    evas_object_size_hint_weight_set(m_errorLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_errorLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_layout_file_set(m_errorLayout, edjePath("WebPageUI/ErrorMessage.edj").c_str(), "error_message");
}

void WebPageUI::createPrivateLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_privateLayout =  elm_layout_add(m_mainLayout);
    evas_object_size_hint_weight_set(m_privateLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_privateLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_layout_file_set(m_privateLayout, edjePath("WebPageUI/PrivateMode.edj").c_str(), "inco_message");

    m_bookmarkManagerButton = elm_button_add(m_privateLayout);
    elm_object_style_set(m_bookmarkManagerButton, "invisible_button");
    evas_object_smart_callback_add(m_bookmarkManagerButton, "clicked", _bookmark_manager_clicked, this);
    evas_object_show(m_bookmarkManagerButton);

    elm_object_part_content_set(m_privateLayout, "bookmarkmanager_click", m_bookmarkManagerButton);
}

void WebPageUI::_bookmark_manager_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("%s:%d %s", __FILE__, __LINE__, __func__);
    WebPageUI*  webpageUI = static_cast<WebPageUI*>(data);
    webpageUI->bookmarkManagerClicked();
}

void WebPageUI::createActions()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_back = sharedAction(new Action("Back"));
    m_back->setToolTip("Previous");
    m_back->setIcon("browser/toolbar_prev");

    m_forward = sharedAction(new Action("Next"));
    m_forward->setToolTip("Next");
    m_forward->setIcon("browser/toolbar_next");

    m_stopLoading = sharedAction(new Action("Stop"));
    m_stopLoading->setToolTip("Stop");
    m_stopLoading->setIcon("browser/toolbar_stop");

    m_reload = sharedAction(new Action("Reload"));
    m_reload->setToolTip("Reload");
    m_reload->setIcon("browser/toolbar_reload");
    m_tab = sharedAction(new Action("Tabs"));
    m_tab->setToolTip("Tab Manager");
    m_tab->setIcon("browser/toolbar_tab");

    m_showMoreMenu = sharedAction(new Action("More_Menu"));
    m_showMoreMenu->setToolTip("More Menu");
    m_showMoreMenu->setIcon("browser/toolbar_setting");
}

void WebPageUI::connectActions()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    //left bar
    m_back->triggered.connect(boost::bind(&WebPageUI::backPageConnect, this));
    m_forward->triggered.connect(boost::bind(&WebPageUI::forwardPageConnect, this));
    m_stopLoading->triggered.connect(boost::bind(&WebPageUI::stopLoadingPageConnect, this));
    m_reload->triggered.connect(boost::bind(&WebPageUI::reloadPageConnect, this));

    //right bar
    m_tab->triggered.connect(boost::bind(&WebPageUI::showTabUIConnect, this));
    m_showMoreMenu->triggered.connect(boost::bind(&WebPageUI::showMoreMenuConnect, this));
}

void WebPageUI::showProgressBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "show_progressbar_bg", "ui");
    elm_object_part_content_set(m_mainLayout, "progress_bar", m_progressBar);
}

void WebPageUI::hideProgressBar()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    elm_object_signal_emit(m_mainLayout, "hide_progressbar_bg", "ui");
    elm_progressbar_value_set(m_progressBar, 0.0);
    elm_object_part_content_unset(m_mainLayout, "progress_bar");
    evas_object_hide(m_progressBar);
}

void WebPageUI::hideWebView()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    evas_object_hide(elm_object_part_content_get(m_mainLayout, "web_view"));
    elm_object_part_content_unset(m_mainLayout, "web_view");
}

void WebPageUI::setErrorButtons()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_leftButtonBar->setActionForButton("refresh_stop_button", m_reload);
    m_stopLoading->setEnabled(false);
    m_reload->setEnabled(true);
    m_forward->setEnabled(false);
    evas_object_hide(m_progressBar);
}

void WebPageUI::setPrivateButtons()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_stopLoading->setEnabled(false);
    m_reload->setEnabled(false);
    m_forward->setEnabled(false);
    evas_object_hide(m_progressBar);
}

void WebPageUI::updateURIBar(const std::string& uri)
{
    BROWSER_LOGD("[%s:%d] URI:%s", __PRETTY_FUNCTION__, __LINE__, uri.c_str());
    m_URIEntry->changeUri(uri);
    m_leftButtonBar->setActionForButton("refresh_stop_button", m_reload);
    m_URIEntry->setURI(uri);

    m_stopLoading->setEnabled(true);
    m_reload->setEnabled(true);
    hideProgressBar();
}

std::string WebPageUI::edjePath(const std::string& file)
{
    return std::string(EDJE_DIR) + file;
}

void WebPageUI::showTabUIConnect()
{
    hideUI();
    showTabUI();
}
void WebPageUI::showMoreMenuConnect()
{
#if !PROFILE_MOBILE
    hideUI();
#endif
    showMoreMenu();
}

void WebPageUI::refreshFocusChain()
{
    // set custom focus chain
    elm_object_focus_custom_chain_unset(m_mainLayout);
    elm_object_focus_custom_chain_append(m_mainLayout, m_rightButtonBar->getContent(), NULL);
    if(!m_statesMgr->equals(WPUState::QUICK_ACCESS)) {
        elm_object_focus_custom_chain_append(m_mainLayout, m_leftButtonBar->getContent(), NULL);
        elm_object_focus_custom_chain_append(m_mainLayout, m_bookmarkManagerButton, NULL);
    } else {
        m_reload->setEnabled(false);
    }
    elm_object_focus_custom_chain_append(m_mainLayout, m_URIEntry->getContent(), NULL);
}

#if PROFILE_MOBILE
Evas_Event_Flags WebPageUI::_gesture_move(void* data , void* event_info)
{
    auto info = static_cast<Elm_Gesture_Line_Info*>(event_info);
    if (info->momentum.n == WebPageUI::SINGLE_FINGER) {
        if (info->momentum.my < -WebPageUI::SWIPE_MOMENTUM_TRESHOLD) {
            auto self = static_cast<WebPageUI*>(data);
            self->geastureUp();
        } else if (info->momentum.my > WebPageUI::SWIPE_MOMENTUM_TRESHOLD) {
            auto self = static_cast<WebPageUI*>(data);
            self->geastureDown();
        }
    }

    return EVAS_EVENT_FLAG_NONE;
}

void WebPageUI::geastureUp()
{
    if (!m_uriBarHidden) {
        m_uriBarHidden = true;
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        elm_object_signal_emit(m_mainLayout, "enlarge_webview", "ui");
        if (m_statesMgr->equals(WPUState::MAIN_WEB_PAGE)) {
            setWebViewTouchEvents(false);
        }
    }
}

void WebPageUI::geastureDown()
{
    if (m_uriBarHidden) {
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        if (m_statesMgr->equals(WPUState::MAIN_WEB_PAGE)) {
            setWebViewTouchEvents(false);
        }
        elm_object_signal_emit(m_mainLayout, "show_uri_bar", "ui");
        m_uriBarHidden = false;
    }
}

void WebPageUI::_geasture_finished(void* data, Evas_Object* /*obj*/, const char* /*emission*/, const char* /*source*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = reinterpret_cast<WebPageUI*>(data);
    if (self->m_statesMgr->equals(WPUState::MAIN_WEB_PAGE)) {
        self->setWebViewTouchEvents(true);
    }
}
#endif

}   // namespace tizen_browser
}   // namespace base_ui
