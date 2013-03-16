/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "text-encoding-type-view.h"

#include <string>

#include "browser-dlog.h"
#include "browser-string.h"

text_encoding_type_view::text_encoding_type_view(void)
:
	m_selected_type(m_preference->get_text_encoding_type_index())
	,m_genlist(NULL)
	,m_radio_main(NULL)
	,m_item_ic(NULL)
	,m_genlist_callback_data_list(NULL)
{
	BROWSER_LOGD("");
}

text_encoding_type_view::~text_encoding_type_view(void)
{
	BROWSER_LOGD("");

	evas_object_smart_callback_del(m_naviframe, "title,clicked", __title_clicked_cb);

	if (m_item_ic)
		elm_genlist_item_class_free(m_item_ic);

	if (m_genlist_callback_data_list) {
		Eina_List* list;
		void* data;

		EINA_LIST_FOREACH(m_genlist_callback_data_list, list, data) {
			if (data)
				free ((genlist_callback_data *)data);
		}
		eina_list_free(m_genlist_callback_data_list);
	}
}

void text_encoding_type_view::show(void)
{
	BROWSER_LOGD("");

	Evas_Object *layout = _create_main_layout(m_naviframe);
	Elm_Object_Item *navi_it = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, layout, NULL);

	Evas_Object *label = elm_label_add(m_naviframe);
	if (!label) {
		BROWSER_LOGE("elm_label_add failed.");
		return;
	}
	elm_object_style_set(label, "naviframe_title");
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_wrap_width_set(label, EINA_TRUE);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_object_text_set(label, BR_STRING_ENCODING_TYPE);
	evas_object_show(label);
	elm_object_item_part_content_set(navi_it, "elm.swallow.title", label);

	evas_object_smart_callback_add(m_naviframe, "title,clicked", __title_clicked_cb, this);
}

char *text_encoding_type_view::get_menu_label(text_encoding_type type)
{
	BROWSER_LOGD("");
	const char *menu_str;
	switch (type) {
	case TEXT_ENCODING_TYPE_ISO_8859_1:
		menu_str = BR_STRING_ENCODING_TYPE_LATIN_1_ISO_8859_1;
		break;
	case TEXT_ENCODING_TYPE_UTF_8:
		menu_str = BR_STRING_ENCODING_TYPE_UNICODE_UTF_8;
		break;
	case TEXT_ENCODING_TYPE_GBK:
		menu_str = BR_STRING_ENCODING_TYPE_CHINESE_GBK;
		break;
	case TEXT_ENCODING_TYPE_BIG5:
		menu_str = BR_STRING_ENCODING_TYPE_CHINESE_BIG5;
		break;
	case TEXT_ENCODING_TYPE_ISO_2022_JP:
		menu_str = BR_STRING_ENCODING_TYPE_JAPANESE_ISO_2022_JP;
		break;
	case TEXT_ENCODING_TYPE_SHIFT_JIS:
		menu_str = BR_STRING_ENCODING_TYPE_JAPANESE_SHIFT_JIS;
		break;
	case TEXT_ENCODING_TYPE_EUC_JP:
		menu_str = BR_STRING_ENCODING_TYPE_JAPANESE_EUC_JP;
		break;
	case TEXT_ENCODING_TYPE_EUC_KR:
		menu_str = BR_STRING_ENCODING_TYPE_KOREAN_EUC_KR;
		break;
	case TEXT_ENCODING_TYPE_AUTOMATIC:
	case TEXT_ENCODING_TYPE_NUM:
	default :
		menu_str = BR_STRING_ENCODING_TYPE_AUTOMATIC;
		break;
	}
	return strdup(menu_str);
}


Evas_Object *text_encoding_type_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	m_genlist = _create_genlist(parent);
	if (!m_item_ic) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	return m_genlist;
}

Evas_Object *text_encoding_type_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	m_radio_main = elm_radio_add(genlist);
	if (!m_radio_main) {
		BROWSER_LOGE("elm_radio_add failed");
		return NULL;
	}
	elm_radio_state_value_set(m_radio_main, 0);
	elm_radio_value_set(m_radio_main, 0);

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	memset(item_ic, 0x00, sizeof(Elm_Genlist_Item_Class));

	item_ic->item_style = "1text.1icon.2";
	item_ic->func.text_get = __genlist_label_get_cb;
	item_ic->func.content_get = __genlist_icon_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;
	m_item_ic = item_ic;

	Eina_Bool has_problem = EINA_FALSE;
	for (int i = 0; i < TEXT_ENCODING_TYPE_NUM; i++) {
		genlist_callback_data *data = (genlist_callback_data *)malloc(sizeof(genlist_callback_data));
		if (!data) {
			has_problem = EINA_TRUE;
			break;
		}

		memset(data, 0x00, sizeof(genlist_callback_data));
		data->type = (text_encoding_type)i;
		data->user_data = this;
		data->it =  elm_genlist_item_append(genlist, m_item_ic, data, NULL, ELM_GENLIST_ITEM_NONE, __item_selected_cb, data);
		m_genlist_callback_data_list = eina_list_append(m_genlist_callback_data_list, data);
	}

	if (has_problem == EINA_TRUE && m_genlist_callback_data_list) {
		Eina_List* list;
		void* data;

		EINA_LIST_FOREACH(m_genlist_callback_data_list, list, data){
			if (data)
				free ((genlist_callback_data *)data);
		}
		eina_list_free(m_genlist_callback_data_list);
		return NULL;
	}
	return genlist;
}

char *text_encoding_type_view::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	text_encoding_type_view *view = (text_encoding_type_view *)callback_data->user_data;

	BROWSER_LOGD("callback_data->type[%d]", callback_data->type);
	if (!strcmp(part, "elm.text")) {
		return view->get_menu_label(callback_data->type);
	}

	return NULL;
}

Evas_Object *text_encoding_type_view::__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	text_encoding_type_view *view = (text_encoding_type_view *)callback_data->user_data;

	BROWSER_LOGD("callback_data->type[%d]", callback_data->type);

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
		Evas_Object *radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, callback_data->type);
		elm_radio_group_add(radio, view->m_radio_main);
		if (callback_data->type == view->m_selected_type)
			elm_radio_value_set(radio, view->m_selected_type);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);

		return radio;
	}

	return NULL;
}

void text_encoding_type_view::__item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	text_encoding_type_view *view = (text_encoding_type_view *)callback_data->user_data;

	Elm_Object_Item *item = (Elm_Object_Item *) event_info;

	if (item) {
		int index = (int) elm_object_item_data_get(item);
		elm_genlist_item_update(item);
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}
	view->m_selected_type = callback_data->type;
	m_preference->set_text_encoding_type(callback_data->type);
}

void text_encoding_type_view::__title_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(event_info);

	Elm_Object_Item *naviframe_item = (Elm_Object_Item *)event_info;
	Evas_Object *label = elm_object_item_part_content_get(naviframe_item, "elm.swallow.title");
	EINA_SAFETY_ON_NULL_RETURN(label);

	elm_label_slide_go(label);
}

