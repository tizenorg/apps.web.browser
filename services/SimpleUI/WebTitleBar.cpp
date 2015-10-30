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

#include <Elementary.h>
#include "BrowserLogger.h"
#include "EflTools.h"

#include "WebTitleBar.h"

// show bar by this time (seconds) - due to IA Guidline
#define DELAY_TIME 4.0

namespace tizen_browser
{
namespace base_ui
{

WebTitleBar::WebTitleBar(Evas_Object *parent, const std::string &edjFile, const std::string &groupName) :
	m_timer(NULL)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string edjFilePath = EDJE_DIR;
    edjFilePath.append(edjFile);
    elm_theme_extension_add(NULL, edjFilePath.c_str());
    m_layout = elm_layout_add(parent);
    Eina_Bool layoutSetResult = elm_layout_file_set(m_layout, edjFilePath.c_str(), groupName.c_str());
    if(!layoutSetResult)
        BROWSER_LOGE("[%s:%d]: Layout file not found: ", __func__, __LINE__, edjFilePath.c_str());
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

WebTitleBar::~WebTitleBar()
{
	if(m_timer)
		ecore_timer_del(m_timer);
}

void WebTitleBar::setFavIcon(std::shared_ptr<tizen_browser::tools::BrowserImage> favicon)
{
    BROWSER_LOGD("[%s:%d] faviconType:%d ", __PRETTY_FUNCTION__, __LINE__, favicon->imageType);
    if(favicon->imageType != tools::BrowserImage::ImageTypeNoImage){
        m_icon = tizen_browser::tools::EflTools::getEvasImage(favicon, m_layout);
        if(m_icon){
            evas_object_image_fill_set(m_icon, 0, 0, 37, 37);
            evas_object_resize(m_icon, 37, 37);
            elm_object_part_content_set(m_layout, "favicon", m_icon);
            evas_object_show(m_icon);
        } else {
            removeFavIcon();
        }
    } else {
        removeFavIcon();
    }
}

void WebTitleBar::removeFavIcon()
{
    Evas_Object * favicon = elm_object_part_content_unset(m_layout, "favicon");
    evas_object_hide(favicon);
    if (favicon) {
        evas_object_del(favicon);
    }
}


void WebTitleBar::show(const std::string & title)
{
	setTitle(title);

	if(m_timer)
		ecore_timer_del(m_timer);

	m_timer = ecore_timer_add(0.7, show_cb, this);
}

Eina_Bool WebTitleBar::show_cb(void * data)
{
	WebTitleBar * self = static_cast<WebTitleBar *> (data);
	if(self->m_timer)
		ecore_timer_del(self->m_timer);
	self->m_timer = ecore_timer_add(DELAY_TIME, hide_cb, self);
	elm_object_signal_emit(self->m_layout, "show_webtitle_bar", "web");
	return ECORE_CALLBACK_CANCEL;
}

void WebTitleBar::setTitle(const std::string & title)
{
	BROWSER_LOGD("Changing title on WebTitle bar to: %s", title.c_str());
	elm_object_part_text_set(m_layout, "text", title.c_str());
}

void WebTitleBar::hide()
{
	hide_cb(this);
}

Eina_Bool WebTitleBar::hide_cb(void * data)
{
	WebTitleBar * self = static_cast<WebTitleBar *> (data);
	elm_object_signal_emit(self->m_layout, "hide_webtitle_bar", "web");
	if(self->m_timer)
		ecore_timer_del(self->m_timer);
	self->m_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

} /* end of base_ui */
} /* end of tizen_browser */
