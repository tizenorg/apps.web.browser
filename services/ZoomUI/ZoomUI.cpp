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
#include <boost/concept_check.hpp>
#include <vector>
#include <AbstractMainWindow.h>

#include "ZoomUI.h"
#include "BrowserLogger.h"
#include "ServiceManager.h"

#define DX 50
#define iDX -50

namespace tizen_browser{
namespace base_ui{

EXPORT_SERVICE(ZoomUI, "org.tizen.browser.zoomui")

ZoomUI::ZoomUI()
    : m_layout(nullptr)
    , m_nav_layout(nullptr)
    , m_zoom_slider(nullptr)
    , m_current_translation_x(0)
    , m_current_translation_y(0)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("ZoomUI/ZoomUI.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

ZoomUI::~ZoomUI() {}

void ZoomUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* ZoomUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if(!m_layout)
        createLayout(m_parent);
    return m_layout;
}

void ZoomUI::showUI()
{
    evas_object_show(m_layout);
    evas_object_show(m_zoom_menu);
    evas_object_show(m_zoom_slider);
    m_keyDownHandler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _key_down_cb, this);
    m_keyUpHandler = ecore_event_handler_add(ECORE_EVENT_KEY_UP, _key_up_cb, this);
    int zoomFactor = *(getZoom());
    elm_slider_value_set(m_zoom_slider, calculateSliderValue(zoomFactor));
}

void ZoomUI::hideUI()
{
    evas_object_hide(m_zoom_slider);
    evas_object_hide(m_nav_layout);
    evas_object_hide(m_zoom_menu);
    evas_object_hide(m_layout);
    ecore_event_handler_del(m_keyDownHandler);
    ecore_event_handler_del(m_keyUpHandler);
}

void ZoomUI::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    init(parent);
    if (!m_layout)
        createLayout(parent);
    showUI();
    elm_object_focus_set(m_zoom_slider, EINA_TRUE);
}

void ZoomUI::showNavigation()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_nav_layout);
    int zoomFactor = *(getZoom());
    if (zoomFactor > ZOOM_DEFAULT)
        evas_object_show(m_nav_layout);
}

void ZoomUI::createLayout(Evas_Object *parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_layout = elm_layout_add(parent);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "zoom-layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    createZoomSlider();
    createNavigationButtons();
}

void ZoomUI::createZoomSlider()
{
    m_zoom_menu = elm_layout_add(m_layout);
    elm_layout_file_set(m_zoom_menu, m_edjFilePath.c_str(), "zoom-menu");
    evas_object_size_hint_weight_set(m_zoom_menu, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_zoom_menu, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_zoom_slider = elm_slider_add(m_zoom_menu);
    evas_object_size_hint_weight_set(m_zoom_slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_zoom_slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(m_zoom_slider, "changed", _zoom_slider_changed, this);
    elm_object_style_set(m_zoom_slider, "default");
    elm_slider_horizontal_set(m_zoom_slider, EINA_TRUE);
    elm_slider_min_max_set(m_zoom_slider, 1, 6);
    elm_slider_step_set(m_zoom_slider, 0.2);
    int zoomFactor = *(getZoom());
    elm_slider_value_set(m_zoom_slider, calculateSliderValue(zoomFactor));
    elm_slider_indicator_show_set(m_zoom_slider, EINA_FALSE);

    elm_object_part_content_set(m_zoom_menu, "slider_swallow", m_zoom_slider);
    evas_object_event_callback_add(m_zoom_slider, EVAS_CALLBACK_KEY_DOWN, _zoom_value_confirmed, this);
}

void ZoomUI::createNavigationButtons()
{
    m_nav_layout = elm_layout_add(m_layout);
    elm_layout_file_set(m_nav_layout, m_edjFilePath.c_str(), "nav-layout");
    evas_object_size_hint_weight_set(m_nav_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_nav_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_leftArrow = elm_icon_add(m_nav_layout);
    setImageFile(m_leftArrow, LEFT, false);
    evas_object_smart_callback_add(m_leftArrow, "clicked", _left_button_clicked, this);
    evas_object_event_callback_add(m_leftArrow, EVAS_CALLBACK_MOUSE_IN, _cb_focus_in_left_button, this);
    evas_object_event_callback_add(m_leftArrow, EVAS_CALLBACK_MOUSE_OUT, _cb_focus_out_left_button, this);
    elm_object_part_content_set(m_nav_layout, "left_button", m_leftArrow);

    m_rightArrow = elm_icon_add(m_nav_layout);
    setImageFile(m_rightArrow, RIGHT, false);
    evas_object_smart_callback_add(m_rightArrow, "clicked", _right_button_clicked, this);
    evas_object_event_callback_add(m_rightArrow, EVAS_CALLBACK_MOUSE_IN, _cb_focus_in_right_button, this);
    evas_object_event_callback_add(m_rightArrow, EVAS_CALLBACK_MOUSE_OUT, _cb_focus_out_right_button, this);
    elm_object_part_content_set(m_nav_layout, "right_button", m_rightArrow);

    m_downArrow = elm_icon_add(m_nav_layout);
    setImageFile(m_downArrow, DOWN, false);
    evas_object_smart_callback_add(m_downArrow, "clicked", _down_button_clicked, this);
    evas_object_event_callback_add(m_downArrow, EVAS_CALLBACK_MOUSE_IN, _cb_focus_in_down_button, this);
    evas_object_event_callback_add(m_downArrow, EVAS_CALLBACK_MOUSE_OUT, _cb_focus_out_down_button, this);
    elm_object_part_content_set(m_nav_layout, "down_button", m_downArrow);

    m_upArrow = elm_icon_add(m_nav_layout);
    setImageFile(m_upArrow, UP, false);
    evas_object_smart_callback_add(m_upArrow, "clicked", _up_button_clicked, this);
    evas_object_event_callback_add(m_upArrow, EVAS_CALLBACK_MOUSE_IN, _cb_focus_in_up_button, this);
    evas_object_event_callback_add(m_upArrow, EVAS_CALLBACK_MOUSE_OUT, _cb_focus_out_up_button, this);
    elm_object_part_content_set(m_nav_layout, "up_button", m_upArrow);
}

void ZoomUI::clearItems()
{
    evas_object_del(m_layout);
    setZoom(ZOOM_DEFAULT);
}

void ZoomUI::setImageFile(Evas_Object* obj, int direction, bool focused)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    switch (direction) {
        case LEFT:  elm_image_file_set(obj, m_edjFilePath.c_str(), focused ? "ic_zoom_indicator_left_foc.png" : "ic_zoom_indicator_left_nor.png");
                    break;
        case RIGHT: elm_image_file_set(obj, m_edjFilePath.c_str(), focused ? "ic_zoom_indicator_right_foc.png" : "ic_zoom_indicator_right_nor.png");
                    break;
        case DOWN:  elm_image_file_set(obj, m_edjFilePath.c_str(), focused ? "ic_zoom_indicator_down_foc.png" : "ic_zoom_indicator_down_nor.png");
                    break;
        case UP:    elm_image_file_set(obj, m_edjFilePath.c_str(), focused ? "ic_zoom_indicator_up_foc.png" : "ic_zoom_indicator_up_nor.png");
                    break;
        default:    BROWSER_LOGD("[%s:%d] Warning: Unhandled button", __PRETTY_FUNCTION__, __LINE__);
                    break;
    }
}

void ZoomUI::_zoom_slider_changed(void *data, Evas_Object *obj, void *event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if(data && obj) {
        int val = elm_slider_value_get(obj);
        int zoomLevel = ZOOM_DEFAULT;    
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);

        switch (val) {
            case 1: zoomLevel = ZOOM_50;
                    break;
            case 2: zoomLevel = ZOOM_75;
                    break;
            case 3: zoomLevel = ZOOM_DEFAULT;
                    break;
            case 4: zoomLevel = ZOOM_150;
                    break;
            case 5: zoomLevel = ZOOM_200;
                    break;
            case 6: zoomLevel = ZOOM_300;
                    break;
            default:BROWSER_LOGD("[%s:%d] Warning: Unhandled zoom level", __PRETTY_FUNCTION__, __LINE__);
                    break;
        }
        zoomUI->setZoom(zoomLevel);
    }
}

void ZoomUI::_zoom_value_confirmed(void* data, Evas*, Evas_Object* obj, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Evas_Event_Key_Down* ev = static_cast<Evas_Event_Key_Down*>(event_info);

    if (!data || !ev || !ev->keyname)
        return;

    ZoomUI* self = static_cast<ZoomUI*>(data);
    if (std::string(ev->keyname) == "Return") {
        int val = (int)elm_slider_value_get(self->m_zoom_slider);
        BROWSER_LOGD("[%s:%d] val: %d", __PRETTY_FUNCTION__, __LINE__, val);
        if (val > 3) {
            BROWSER_LOGD("[%s:%d] value is greater than 3", __PRETTY_FUNCTION__, __LINE__);
            evas_object_show(self->m_nav_layout);
        } else {
            BROWSER_LOGD("[%s:%d] value is smaller or equal to 3", __PRETTY_FUNCTION__, __LINE__);
            evas_object_hide(self->m_nav_layout);
        }
        evas_object_hide(self->m_zoom_menu);
    }
}

Eina_Bool ZoomUI::_key_down_cb(void* data, int type, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Ecore_Event_Key* ev = static_cast<Ecore_Event_Key*>(event_info);

    if (!data || !ev || !ev->keyname)
        return EINA_FALSE;

    ZoomUI* self = static_cast<ZoomUI*>(data);
    if (std::string(ev->keyname) == "Left") {
        _left_button_clicked(data, self->m_leftArrow, event_info);
        _cb_focus_in_left_button(data, nullptr, self->m_leftArrow, event_info);
    } else if (std::string(ev->keyname) == "Right") {
        _right_button_clicked(data, self->m_rightArrow, event_info);
        _cb_focus_in_right_button(data, nullptr, self->m_rightArrow, event_info);
    } else if (std::string(ev->keyname) == "Up") {
        _up_button_clicked(data, self->m_upArrow, event_info);
        _cb_focus_in_up_button(data, nullptr, self->m_upArrow, event_info);
    } else if (std::string(ev->keyname) == "Down") {
        _down_button_clicked(data, self->m_downArrow, event_info);
        _cb_focus_in_down_button(data, nullptr, self->m_downArrow, event_info);
    } else
        return EINA_FALSE;

    return EINA_TRUE;
}

Eina_Bool ZoomUI::_key_up_cb(void* data, int type, void* event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    Ecore_Event_Key* ev = static_cast<Ecore_Event_Key*>(event_info);

    if (!data || !ev || !ev->keyname)
        return EINA_FALSE;

    ZoomUI* self = static_cast<ZoomUI*>(data);
    if (std::string(ev->keyname) == "Left") {
        _cb_focus_out_left_button(data, nullptr, self->m_leftArrow, event_info);
    } else if (std::string(ev->keyname) == "Right") {
        _cb_focus_out_right_button(data, nullptr, self->m_rightArrow, event_info);
    } else if (std::string(ev->keyname) == "Up") {
        _cb_focus_out_up_button(data, nullptr, self->m_upArrow, event_info);
    } else if (std::string(ev->keyname) == "Down") {
        _cb_focus_out_down_button(data, nullptr, self->m_downArrow, event_info);
    } else
        return EINA_FALSE;

    return EINA_TRUE;
}

void ZoomUI::_left_button_clicked(void * data, Evas_Object * obj, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->scrollView(iDX, 0);
    }
}

void ZoomUI::_right_button_clicked(void * data, Evas_Object * obj, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->scrollView(DX, 0);
    }
}

void ZoomUI::_up_button_clicked(void * data, Evas_Object * obj, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->scrollView(0, iDX);
    }
}

void ZoomUI::_down_button_clicked(void * data, Evas_Object * obj, void * event_info)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->scrollView(0, DX);
    }
}

void ZoomUI::_cb_focus_in_left_button(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        elm_object_focus_set(obj, EINA_TRUE);
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->setImageFile(obj, LEFT, true);
    }
}

void ZoomUI::_cb_focus_out_left_button(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        elm_object_focus_set(obj, EINA_FALSE);
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->setImageFile(obj, LEFT, false);
    }
}

void ZoomUI::_cb_focus_in_right_button(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        elm_object_focus_set(obj, EINA_TRUE);
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->setImageFile(obj, RIGHT, true);
    }
}

void ZoomUI::_cb_focus_out_right_button(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        elm_object_focus_set(obj, EINA_FALSE);
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->setImageFile(obj, RIGHT, false);
    }
}

void ZoomUI::_cb_focus_in_up_button(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        elm_object_focus_set(obj, EINA_TRUE);
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->setImageFile(obj, UP, true);
    }
}

void ZoomUI::_cb_focus_out_up_button(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        elm_object_focus_set(obj, EINA_FALSE);
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->setImageFile(obj, UP, false);
    }
}

void ZoomUI::_cb_focus_in_down_button(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        elm_object_focus_set(obj, EINA_TRUE);
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->setImageFile(obj, DOWN, true);
    }
}

void ZoomUI::_cb_focus_out_down_button(void * data, Evas *, Evas_Object *obj, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && obj) {
        elm_object_focus_set(obj, EINA_FALSE);
        ZoomUI *zoomUI = static_cast<ZoomUI*>(data);
        zoomUI->setImageFile(obj, DOWN, false);
    }
}

int ZoomUI::calculateSliderValue(int zoom)
{
    BROWSER_LOGD("[%s:%d] zoom factor: %d", __PRETTY_FUNCTION__, __LINE__, zoom);
    if (zoom >= ZOOM_300)
        return 6;
    else if (zoom >= ZOOM_200)
        return 5;
    else if (zoom >= ZOOM_150)
        return 4;
    else if (zoom >= ZOOM_100)
        return 3;
    else if (zoom >= ZOOM_75)
        return 2;
    else if (zoom < ZOOM_75)
        return 1;
}


}
}
