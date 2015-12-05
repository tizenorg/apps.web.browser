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

#include "AutoFillFormManager.h"
#include "AutoFillFormListView.h"
#include "AutoFillFormItem.h"
#include "AutoProfileDeleteView.h"
#include "BrowserLogger.h"

#include <regex.h>

auto_fill_form_list_view::auto_fill_form_list_view(auto_fill_form_manager *affm)
:
	m_manager(affm),
	m_main_layout(NULL),
	m_genlist(NULL)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        m_edjFilePath = EDJE_DIR;
        m_edjFilePath.append("SettingsUI/AutoFillMobileUI.edj");
}

auto_fill_form_list_view::~auto_fill_form_list_view(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

        if (m_genlist) {
            elm_genlist_clear(m_genlist);
            evas_object_del(m_genlist);
        }
        if (m_main_layout) {
            evas_object_hide(m_main_layout);
            evas_object_del(m_main_layout);
        }
}

void auto_fill_form_list_view::init(Evas_Object* parent)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	m_parent = parent;
}

Eina_Bool auto_fill_form_list_view::show(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

        elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
        m_parent = m_manager->getParent();
        m_main_layout = _create_main_layout(m_parent);
	if (!m_main_layout) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

#if defined(HW_MORE_BACK_KEY)
		// Add HW key callback.
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_MORE, __more_menu_cb, this);
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_BACK, __back_cb, this);
#endif
        Evas_Object* back_button = elm_button_add(m_main_layout);
        if (!back_button) {
                BROWSER_LOGE("Failed to create back_button");
                return EINA_FALSE;
        }
        elm_object_style_set(back_button, "basic_button");
        evas_object_smart_callback_add(back_button, "clicked", __back_button_cb, this);
        elm_object_part_content_set(m_main_layout, "back_button", back_button);


	Evas_Object *add_btn = elm_button_add(m_main_layout);
	elm_object_style_set(add_btn, "basic_button");
	evas_object_smart_callback_add(add_btn, "clicked", __add_profile_button_cb, this);
	elm_object_part_content_set(m_main_layout, "add_profile_button", add_btn);

        Evas_Object *del_btn = elm_button_add(m_main_layout);
        elm_object_style_set(del_btn, "basic_button");
        evas_object_smart_callback_add(del_btn, "clicked", __delete_profile_button_cb, this);
        elm_object_part_content_set(m_main_layout, "delete_profile_button", del_btn);

        evas_object_show(m_main_layout);

	return EINA_TRUE;
}

void auto_fill_form_list_view::refresh_view(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	elm_genlist_clear(m_genlist);
	_append_genlist(m_genlist);
}

Evas_Object *auto_fill_form_list_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

        Evas_Object *layout = elm_layout_add(parent);
        if (!layout) {
                BROWSER_LOGD("elm_layout_add failed");
                return NULL;
        }
        elm_layout_file_set(layout, m_edjFilePath.c_str(), "afflv-layout");
        evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_genlist = _create_genlist(layout);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	evas_object_show(m_genlist);
        elm_object_part_content_set(layout, "afflv_genlist", m_genlist);

	return layout;
}

Evas_Object *auto_fill_form_list_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

        m_item_class = elm_genlist_item_class_new();
        if (!m_item_class) {
                BROWSER_LOGE("elm_genlist_item_class_new for description_item_class failed");
                return EINA_FALSE;
        }
        m_item_class->item_style = "afflv_item";
        m_item_class->func.content_get = NULL;

        m_item_class->func.text_get = __text_get_cb;
        m_item_class->func.state_get = NULL;
        m_item_class->func.del = NULL;

        _append_genlist(genlist);

	return genlist;
}

const char *auto_fill_form_list_view::_get_each_item_full_name(unsigned int index)
{
	if (m_manager->get_auto_fill_form_item_count() == 0)
		return NULL;
	return (m_manager->get_item_list())[index]->get_name();
}

Eina_Bool auto_fill_form_list_view::_append_genlist(Evas_Object *genlist)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	unsigned int item_count = m_manager->get_auto_fill_form_item_count();
	if (item_count > 0) {
                for (unsigned int i = 0; i < item_count; i++) {
                   genlist_callback_data* item_callback_data = new genlist_callback_data;
                   item_callback_data->menu_index = i;
                   item_callback_data->user_data = this;
		   item_callback_data->it = elm_genlist_item_append(genlist, m_item_class,
							item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, item_callback_data);
                }
	}

	return EINA_TRUE;
}

char *auto_fill_form_list_view::__text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
        genlist_callback_data *callback_data = (genlist_callback_data *)data;
	BROWSER_LOGD("-----------[Menu index %d]---------", callback_data->menu_index);
        auto_fill_form_list_view *view = (auto_fill_form_list_view *)(callback_data->user_data);

        if (!strcmp(part, "item_title")) {
            const char *item_full_name = view->_get_each_item_full_name((unsigned int)callback_data->menu_index);
            if (item_full_name)
               return strdup(item_full_name);
	}
        return NULL;

}

void auto_fill_form_list_view::__add_profile_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	/* create new profile */
	auto_fill_form_list_view *list_view = (auto_fill_form_list_view *)data;
	list_view->m_manager->show_composer();
}

void auto_fill_form_list_view::__delete_profile_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view*)data;
	afflv->m_manager->show_delete_view();
}

void auto_fill_form_list_view::__back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");
	auto_fill_form_list_view *list_view = (auto_fill_form_list_view *)data;
	list_view->m_manager->delete_list_view();
}

void auto_fill_form_list_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)(callback_data->user_data);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
	view->m_manager->show_composer((view->m_manager->get_item_list())[callback_data->menu_index]);
}
