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
    evas_object_show(m_nav_layout);
    evas_object_show(m_zoom_slider);
}

void ZoomUI::hideUI()
{
    evas_object_hide(m_zoom_slider);
    evas_object_hide(m_nav_layout);
    evas_object_hide(m_layout);
}

void ZoomUI::show(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    init(parent);
    createLayout(parent);
    showUI();
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
    m_zoom_slider = elm_slider_add(m_layout);
    evas_object_size_hint_weight_set(m_zoom_slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_zoom_slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(m_zoom_slider, "changed", _zoom_slider_changed, this);
    elm_object_style_set(m_zoom_slider, "default");
    elm_slider_horizontal_set(m_zoom_slider, EINA_TRUE);
    elm_slider_min_max_set(m_zoom_slider, 1, 6);
    elm_slider_step_set(m_zoom_slider, 1);
    elm_slider_value_set(m_zoom_slider, 3);
    elm_slider_indicator_format_set(m_zoom_slider, "%1.0f");

    elm_object_part_content_set(m_layout, "slider_swallow", m_zoom_slider);
}

void ZoomUI::createNavigationButtons()
{
    m_nav_layout = elm_layout_add(m_layout);
    elm_layout_file_set(m_nav_layout, m_edjFilePath.c_str(), "nav-layout");
    evas_object_size_hint_weight_set(m_nav_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_nav_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_content_set(m_layout, "nav_buttons", m_nav_layout);

    Evas_Object* icon = elm_icon_add(m_nav_layout);
    setImageFile(icon, LEFT, false);
    evas_object_smart_callback_add(icon, "clicked", _left_button_clicked, this);
    evas_object_event_callback_add(icon, EVAS_CALLBACK_MOUSE_IN, _cb_focus_in_left_button, this);
    evas_object_event_callback_add(icon, EVAS_CALLBACK_MOUSE_OUT, _cb_focus_out_left_button, this);
    elm_object_part_content_set(m_nav_layout, "left_button", icon);
    
    icon = elm_icon_add(m_nav_layout);
    setImageFile(icon, RIGHT, false);
    evas_object_smart_callback_add(icon, "clicked", _right_button_clicked, this);
    evas_object_event_callback_add(icon, EVAS_CALLBACK_MOUSE_IN, _cb_focus_in_right_button, this);
    evas_object_event_callback_add(icon, EVAS_CALLBACK_MOUSE_OUT, _cb_focus_out_right_button, this);
    elm_object_part_content_set(m_nav_layout, "right_button", icon);

    icon = elm_icon_add(m_nav_layout);
    setImageFile(icon, DOWN, false);
    evas_object_smart_callback_add(icon, "clicked", _down_button_clicked, this);
    evas_object_event_callback_add(icon, EVAS_CALLBACK_MOUSE_IN, _cb_focus_in_down_button, this);
    evas_object_event_callback_add(icon, EVAS_CALLBACK_MOUSE_OUT, _cb_focus_out_down_button, this);
    elm_object_part_content_set(m_nav_layout, "down_button", icon);

    icon = elm_icon_add(m_nav_layout);
    setImageFile(icon, UP, false);
    evas_object_smart_callback_add(icon, "clicked", _up_button_clicked, this);
    evas_object_event_callback_add(icon, EVAS_CALLBACK_MOUSE_IN, _cb_focus_in_up_button, this);
    evas_object_event_callback_add(icon, EVAS_CALLBACK_MOUSE_OUT, _cb_focus_out_up_button, this);
    elm_object_part_content_set(m_nav_layout, "up_button", icon);
}

void ZoomUI::clearItems()
{
    evas_object_del(m_layout);
    setZoom(ZOOM_DEFAULT);
}

void ZoomUI::setImageFile(Evas_Object* obj, int direction, bool focused)
{
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


}
}
