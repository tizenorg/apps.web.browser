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
#include "AutoProfileDeleteView.h"
#include "AutoFillFormListView.h"
#include "AutoFillFormItem.h"
#include "BrowserLogger.h"

#include <regex.h>

auto_profile_delete_view::auto_profile_delete_view(auto_fill_form_manager* manager)
:
	m_manager(manager),
	m_main_layout(NULL),
	m_genlist(NULL)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
        m_edjFilePath = EDJE_DIR;
        m_edjFilePath.append("SettingsUI/AutoFillMobileUI.edj");
}

auto_profile_delete_view::~auto_profile_delete_view(void)
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

void auto_profile_delete_view::show(Evas_Object* parent)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);


        elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
        m_main_layout = _create_main_layout(parent);
        if (!m_main_layout) {
                BROWSER_LOGE("_create_main_layout failed");
                return;
        }

        Evas_Object* back_button = elm_button_add(m_main_layout);
        if (!back_button) {
                BROWSER_LOGE("Failed to create back_button");
                return;
        }
        elm_object_style_set(back_button, "basic_button");
        evas_object_smart_callback_add(back_button, "clicked", __back_button_cb, this);
        elm_object_part_content_set(m_main_layout, "back_button", back_button);


        Evas_Object *del_sel_btn = elm_button_add(m_main_layout);
        elm_object_style_set(del_sel_btn, "basic_button");
        evas_object_smart_callback_add(del_sel_btn, "clicked", __delete_selected_button_cb, this);
        elm_object_part_content_set(m_main_layout, "del_sel_button", del_sel_btn);

        Evas_Object *del_btn = elm_button_add(m_main_layout);
        elm_object_style_set(del_btn, "basic_button");
        evas_object_smart_callback_add(del_btn, "clicked", __delete_all_button_cb, this);
        elm_object_part_content_set(m_main_layout, "del_all_button", del_btn);

        evas_object_show(m_main_layout);
}

Evas_Object *auto_profile_delete_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

        Evas_Object *layout = elm_layout_add(parent);
        if (!layout) {
                BROWSER_LOGD("elm_layout_add failed");
                return NULL;
        }
        elm_layout_file_set(layout, m_edjFilePath.c_str(), "affdv-layout");
        evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

        m_genlist = _create_genlist(layout);
        if (!m_genlist) {
                BROWSER_LOGE("elm_genlist_add failed");
                return NULL;
        }
        evas_object_show(m_genlist);
        elm_object_part_content_set(layout, "affdv_genlist", m_genlist);

        return layout;
}

Evas_Object *auto_profile_delete_view::_create_genlist(Evas_Object *parent)
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
        m_item_class->item_style = "affdv_item";
        m_item_class->func.content_get = __content_get_cb;

        m_item_class->func.text_get = __text_get_cb;
        m_item_class->func.state_get = NULL;
        m_item_class->func.del = NULL;

        _append_genlist(genlist);

        return genlist;
}

Eina_Bool auto_profile_delete_view::_append_genlist(Evas_Object *genlist)
{
        BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

        unsigned int item_count = m_manager->get_auto_fill_form_item_count();
        if (item_count > 0) {
                for (unsigned int i = 0; i < item_count; i++) {
                   genlist_callback_data* item_callback_data = new genlist_callback_data;
                   item_callback_data->menu_index = i;
                   item_callback_data->user_data = this;
                   item_callback_data->it = elm_genlist_item_append(genlist, m_item_class,
                                                        item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_selected_cb, item_callback_data);
                }
        }

        return EINA_TRUE;
}

void auto_profile_delete_view::__back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
        BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
        RET_MSG_IF(!data, "data is NULL");
        auto_profile_delete_view *view = (auto_profile_delete_view *)data;
        view->_back_delete_view();
}

void auto_profile_delete_view::__genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);
	Evas_Object *checkbox = elm_object_item_part_content_get(item, "checkbox");
	Eina_Bool state = elm_check_state_get(checkbox);
	elm_check_state_set(checkbox, !state);
}

void auto_profile_delete_view::__delete_selected_button_cb(void *data,Evas_Object *obj,void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");
	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;
	apdv->_item_delete_selected();
}

void auto_profile_delete_view::__delete_all_button_cb(void *data,Evas_Object *obj,void *event_info)
{
        BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
        RET_MSG_IF(!data, "data is NULL");
        auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;
        apdv->_items_delete_all();
}

void auto_profile_delete_view::_items_delete_all(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	m_manager->delete_all_auto_fill_form_items();
	_back_delete_view();
	m_manager->refresh_list_view();
}

void auto_profile_delete_view::_item_delete_selected(void)
{

	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	genlist_callback_data *cb_data = NULL;
	Elm_Object_Item *it = elm_genlist_first_item_get(m_genlist);
	Evas_Object *checkbox;
	int del_count =0;

	while (it) {
		checkbox = elm_object_item_part_content_get(it, "checkbox");
		cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
                if (elm_check_state_get(checkbox)) {
			auto_fill_form_item *item = m_manager->get_item_list()[cb_data->menu_index - del_count];
			m_manager->delete_auto_fill_form_item(item);
			del_count++;
		}
		it = elm_genlist_item_next_get(it);
	}
	BROWSER_LOGD("Total items deleted %d",del_count);
	_back_delete_view();
	m_manager->refresh_list_view();
}

char *auto_profile_delete_view::__text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_profile_delete_view *view = (auto_profile_delete_view *)callback_data->user_data;

        if (!strcmp(part, "item_title")) {
            const char *item_full_name = view->_get_each_item_full_name((unsigned int)callback_data->menu_index);
            if (item_full_name)
               return strdup(item_full_name);
        }
        return NULL;
}

const char *auto_profile_delete_view::_get_each_item_full_name(unsigned int index)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	if (m_manager->get_auto_fill_form_item_count() == 0)
		return NULL;
	return (m_manager->get_item_list())[index]->get_name();
}

Evas_Object *auto_profile_delete_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	Evas_Object *checkbox = NULL;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;

        if (!strcmp(part, "checkbox")) {
                checkbox = elm_check_add(obj);
                if (!checkbox) {
                        BROWSER_LOGE("Failed to add check");
                        return NULL;
                }
                elm_object_style_set(checkbox, "custom_check");
                Eina_Bool checked = false;
                elm_check_state_pointer_set(checkbox, &checked);
                //evas_object_smart_callback_add(checkbox, "changed", __checkbox_changed_cb, callback_data->user_data);
                evas_object_propagate_events_set(checkbox, EINA_FALSE);
                return checkbox;
        }

	return NULL;
}

void auto_profile_delete_view::_back_delete_view(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
        evas_object_hide(m_main_layout);
        //m_manager->delete_delete_view();
}
