#include "AutoFillForm.h"
#include "AutoProfileDeleteView.h"
#include "BrowserLogger.h"

//#include <efl_extension.h>
#include <regex.h>
#include "browser-string.h"

auto_profile_delete_view::auto_profile_delete_view(void)
:
	m_del_layout(NULL),
	m_del_genlist(NULL),
	m_button_done(NULL),
	m_auto_fill_form_check(NULL),
#if defined(DELETE_CONFIRM_POPUP_ENABLE)
	m_delete_popup(NULL),
#endif
	//m_naviframe_item(NULL),
	m_select_all_flag(EINA_FALSE),
	m_count_checked_item(0),
	m_item_callback_data(NULL),
	m_select_all_callback_data(NULL)
{
	BROWSER_LOGD("");
}

auto_profile_delete_view::~auto_profile_delete_view(void)
{
	BROWSER_LOGD("");
	m_select_all_flag = EINA_FALSE;
#if defined(DELETE_CONFIRM_POPUP_ENABLE)
	if (m_delete_popup)
		evas_object_smart_callback_del(m_delete_popup, "language,changed", __auto_profile_delete_popup_lang_changed_cb);
	m_delete_popup = NULL;
#endif
	_remove_each_item_callback_data();
}

void auto_profile_delete_view::__checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;
	Eina_Bool all_selected = EINA_TRUE;
	Elm_Object_Item *it = elm_genlist_first_item_get(apdv->m_del_genlist);
	Elm_Object_Item *it_select_all = it;
	it = elm_genlist_item_next_get(it);//To avoid the first "select All" option.
	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data) {
			if (cb_data->is_checked == EINA_FALSE) {
				all_selected = EINA_FALSE;
				break;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it_select_all);
	if (cb_data) {
		cb_data->is_checked = all_selected;
		elm_genlist_item_update(it_select_all);
		apdv->m_select_all_flag = all_selected;
	}
	it = elm_genlist_first_item_get(apdv->m_del_genlist);
	it = elm_genlist_item_next_get(it);//To avoid the first "select All" option.
	apdv->m_count_checked_item = 0;
	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data) {
			if (cb_data->is_checked == EINA_TRUE)
				apdv->m_count_checked_item++;
		}
		it = elm_genlist_item_next_get(it);
	}
	apdv->_set_selected_title();
	/* Set delete button status */
	it = elm_genlist_first_item_get(apdv->m_del_genlist);
	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data) {
			if (cb_data->is_checked == EINA_TRUE) {
				elm_object_disabled_set(apdv->m_button_done, EINA_FALSE);
				return;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	elm_object_disabled_set(apdv->m_button_done, EINA_TRUE);
}

void auto_profile_delete_view::show(void )
{
	BROWSER_LOGD("");

	m_del_layout = _create_auto_form_delete_layout(m_parent);

	/*if (!m_naviframe) {
		BROWSER_LOGE("create delete layout failed");
		return ;
	}
	m_naviframe_item = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, m_del_layout, NULL);*/

	Evas_Object *btn_cancel = elm_button_add(m_del_layout);
	if (!btn_cancel) return;
	elm_object_style_set(btn_cancel, "naviframe/title_cancel");
	evas_object_smart_callback_add(btn_cancel, "clicked", __cancel_button_click_cb, this);
	elm_object_item_part_content_set(m_del_layout, "title_left_btn", btn_cancel);

	Evas_Object *btn_save = elm_button_add(m_del_layout);
	if (!btn_save) return;
	elm_object_style_set(btn_save, "naviframe/title_done");
	evas_object_smart_callback_add(btn_save, "clicked", __delete_item_cb, this);
	elm_object_item_part_content_set(m_del_layout, "title_right_btn", btn_save);
	m_button_done = btn_save;
	elm_object_disabled_set(m_button_done, EINA_TRUE);

	//elm_naviframe_item_pop_cb_set(m_naviframe_item, __naviframe_pop_cb, this);
	//evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_finished_cb);

	if (m_del_genlist)
		evas_object_smart_callback_add(m_del_genlist, "language,changed", __auto_profile_delete_title_lang_changed_cb, this);
	m_count_checked_item = 0;
	_set_selected_title();
}

Evas_Object *auto_profile_delete_view::_create_auto_form_delete_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	if (m_auto_fill_form_item_callback_data_list.size())
		_remove_each_item_callback_data();

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	m_select_all_callback_data = (genlist_callback_data *)malloc(sizeof(genlist_callback_data));
	if (!m_select_all_callback_data){
			if (item_ic)
				elm_genlist_item_class_free(item_ic);
			return NULL;
	}
	m_select_all_callback_data->user_data = this;
	m_select_all_callback_data->type = DELETE_PROFILE_SELECT_ALL;
	m_select_all_callback_data->is_checked = EINA_FALSE;

	m_item_callback_data = (genlist_callback_data *)malloc(sizeof(genlist_callback_data));
	if (!m_item_callback_data){
			if (item_ic)
				elm_genlist_item_class_free(item_ic);
			return NULL;
	}
	m_item_callback_data->user_data = this;
	m_item_callback_data->type = DELETE_PROFILE_ITEM;
	m_item_callback_data->is_checked = EINA_FALSE;

	Elm_Genlist_Item_Class *selectall_item_ic = elm_genlist_item_class_new();
	if (!selectall_item_ic) {
		BROWSER_LOGE("elm_genlist_item_class_new failed");
		if (item_ic)
			elm_genlist_item_class_free(item_ic);

		return NULL;
	}
	selectall_item_ic->item_style = "select_all";
	selectall_item_ic->func.text_get = __text_get_cb;
	selectall_item_ic->func.content_get = __content_get_cb;
	selectall_item_ic->func.state_get = NULL;
	selectall_item_ic->func.del = NULL;

	if (!item_ic) {
		BROWSER_LOGE("elm_genlist_item_class_new failed");
		if (selectall_item_ic)
			elm_genlist_item_class_free(selectall_item_ic);

		return NULL;
	}
	item_ic->item_style = "1line";
	item_ic->func.text_get = __text_get_cb;
	item_ic->func.content_get = __content_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

	unsigned int item_count = auto_fill_form_manager::get_auto_fill_form_manager()->get_auto_fill_form_item_count();
	BROWSER_LOGD("%d ",item_count);

	if (item_count >0) {
		genlist_callback_data *each_item_callback_data = (genlist_callback_data *)malloc(sizeof(genlist_callback_data));
		if (!each_item_callback_data){
			if (selectall_item_ic) elm_genlist_item_class_free(selectall_item_ic);
			if (item_ic) elm_genlist_item_class_free(item_ic);
			return NULL;
		}
		memset(each_item_callback_data, 0x00, sizeof(genlist_callback_data));
		each_item_callback_data->type = DELETE_PROFILE_SELECT_ALL;
		each_item_callback_data->user_data = this;
		each_item_callback_data->it = elm_genlist_item_append(genlist, selectall_item_ic,
									each_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_selected_cb, m_select_all_callback_data);
		m_auto_fill_form_item_callback_data_list.push_back(each_item_callback_data);
	}
	for (unsigned int i = 0; i < item_count; i++) {
		genlist_callback_data *each_item_callback_data = (genlist_callback_data *)malloc(sizeof(genlist_callback_data));
		if (!each_item_callback_data)
			continue;

		memset(each_item_callback_data, 0x00, sizeof(genlist_callback_data));
		each_item_callback_data->type = DELETE_PROFILE_ITEM;
		each_item_callback_data->menu_index = i;
		each_item_callback_data->user_data = this;
		each_item_callback_data->it = elm_genlist_item_append(genlist, item_ic,
									each_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_selected_cb, m_item_callback_data);
		m_auto_fill_form_item_callback_data_list.push_back(each_item_callback_data);
	}

	if (selectall_item_ic) elm_genlist_item_class_free(selectall_item_ic);
	if (item_ic) elm_genlist_item_class_free(item_ic);
	m_del_genlist = genlist;
	evas_object_show(m_del_genlist);
	return m_del_genlist;
}

void auto_profile_delete_view::__genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);
	Evas_Object *checkbox = elm_object_item_part_content_get(item, "elm.icon.right");
	Eina_Bool state = elm_check_state_get(checkbox);
	elm_check_state_set(checkbox, !state);
	genlist_callback_data *gcd = (genlist_callback_data *)data;
	auto_profile_delete_view *apdv = (auto_profile_delete_view *)(gcd->user_data);
	if (gcd->type == DELETE_PROFILE_SELECT_ALL)
		__select_all_icon_clicked_cb((void *)apdv, NULL, NULL);
	else
		__checkbox_changed_cb((void *)apdv, NULL, NULL);
}

void auto_profile_delete_view::__delete_item_cb(void *data,Evas_Object *obj,void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;

	if (apdv->m_select_all_flag)
		apdv->_items_delete_all();
	else
		apdv->_item_delete();
}
void auto_profile_delete_view::_items_delete_all(void)
{
	BROWSER_LOGD("");

	auto_fill_form_manager::get_auto_fill_form_manager()->delete_all_auto_fill_form_items();
	_back_delete_view();
	auto_fill_form_manager::get_auto_fill_form_manager()->get_list_view()->refresh_view();
}

void auto_profile_delete_view::_item_delete(void)
{

	BROWSER_LOGD("");

	genlist_callback_data *cb_data = NULL;
	Elm_Object_Item *it = elm_genlist_first_item_get(m_del_genlist);
	it = elm_genlist_item_next_get(it); //To avoid the first "select All" option.
	Evas_Object *checkbox;
	int del_count =0;

	while (it) {
		checkbox = elm_object_item_part_content_get(it, "elm.icon.right");
		cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data && cb_data->is_checked ) {
			auto_fill_form_item *item = auto_fill_form_manager::get_auto_fill_form_manager()->get_item_list()[cb_data->menu_index - del_count];
			auto_fill_form_manager::get_auto_fill_form_manager()->delete_auto_fill_form_item(item);
			del_count++;
		}
		it = elm_genlist_item_next_get(it);
	}
	BROWSER_LOGD("Total items deleted %d",del_count);
	_back_delete_view();
	auto_fill_form_manager::get_auto_fill_form_manager()->get_list_view()->refresh_view();
}

void auto_profile_delete_view::__auto_profile_delete_title_lang_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;
	apdv->_set_selected_title();
}

#if defined(DELETE_CONFIRM_POPUP_ENABLE)
void auto_profile_delete_view::__popup_delete_item_cb(void *data, Evas_Object *obj, void *event_info)
{

	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;
	if (apdv->m_delete_popup)
		evas_object_smart_callback_del(apdv->m_delete_popup, "language,changed", __auto_profile_delete_popup_lang_changed_cb);
	apdv->m_delete_popup = NULL;
	genlist_callback_data *cb_data = NULL;
	Elm_Object_Item *it = elm_genlist_first_item_get(apdv->m_del_genlist);
	Evas_Object *checkbox;
	int del_count =0;

	while (it) {
		checkbox = elm_object_item_part_content_get(it, "elm.icon.right");
		cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data && elm_check_state_get(checkbox)) {
			auto_fill_form_item *item = auto_fill_form_manager::get_auto_fill_form_manager()->get_item_list()[cb_data->menu_index - del_count];
			auto_fill_form_manager::get_auto_fill_form_manager()->delete_auto_fill_form_item(item);
			del_count++;
		}
		it = elm_genlist_item_next_get(it);
	}
	BROWSER_LOGD("Total items deleted %d",del_count);
	apdv->_back_delete_view();
	auto_fill_form_manager::get_auto_fill_form_manager()->get_list_view()->refresh_view();
}

void auto_profile_delete_view::_set_popup_text()
{
	BROWSER_LOGD("");
	if ((m_count_checked_item > 1) && m_delete_popup) {
		char label_count[256] = {'\0', };
		char *text = NULL;
		int len ;
		snprintf(label_count, sizeof(label_count), "%d", m_count_checked_item);
		len = strlen(label_count) + strlen(BR_STRING_SETTINGS_PD_PROFILES_DELETED) - 1;
		text = (char *)malloc(len * sizeof(char));
		RET_MSG_IF(!text, "text is NULL");
		snprintf(text, len, BR_STRING_SETTINGS_PD_PROFILES_DELETED, m_count_checked_item);
		elm_object_text_set(m_delete_popup, text);

		free(text);
	}
}
#endif

void auto_profile_delete_view::_set_selected_title()
{
	BROWSER_LOGD("");
	m_sub_title.clear();
	char label_count[1024] = {'\0', };
	char *text = NULL;
	int len ;

	snprintf(label_count, sizeof(label_count), "%d", (m_count_checked_item));
	len = strlen(label_count) + strlen(BR_STRING_SELECTED) + 1;
	text = (char *)malloc(len * sizeof(char));

	if (!text)
		return ;

	memset(text, 0x00, len);
	snprintf(text, len-1, BR_STRING_SELECTED, m_count_checked_item);
	m_sub_title.append(text);
	//elm_object_item_part_text_set(m_naviframe_item, "elm.text.title", m_sub_title.c_str());
	free(text);
}

void auto_profile_delete_view::_unset_selected_title()
{
	BROWSER_LOGD(" ");
	//elm_object_item_part_text_set(m_naviframe_item, "elm.text.title", BR_STRING_DELETE);
}

void auto_profile_delete_view::__select_all_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;
	Eina_Bool state = apdv->m_select_all_flag = !apdv->m_select_all_flag;

	Elm_Object_Item *it = elm_genlist_first_item_get(apdv->m_del_genlist);
	genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
	if (cb_data && cb_data->is_checked != state) {
		cb_data->is_checked = state;
		elm_genlist_item_update(it);
	}

	apdv->m_count_checked_item = 0;
	it = elm_genlist_item_next_get(it); //To avoid select all
	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data && cb_data->is_checked != state) {
			cb_data->is_checked = state;
			elm_genlist_item_update(it);
		}
		if (cb_data && cb_data->is_checked)
			apdv->m_count_checked_item++;
		it = elm_genlist_item_next_get(it);
	}
	apdv->_set_selected_title();
	if (apdv->m_select_all_flag)
		elm_object_disabled_set(apdv->m_button_done, EINA_FALSE);
	else
		elm_object_disabled_set(apdv->m_button_done, EINA_TRUE);

}

char *auto_profile_delete_view::__text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_profile_delete_view *view = (auto_profile_delete_view *)callback_data->user_data;
	menu_type type = callback_data->type;

	if (type == DELETE_PROFILE_SELECT_ALL) {
		if (!strcmp(part, "elm.text.main"))
			return strdup(BR_STRING_SELECT_ALL);
	}
	if (type == DELETE_PROFILE_ITEM) {
		if (!strcmp(part, "elm.text.main.left")) {
			const char *item_full_name = view->_get_each_item_full_name((unsigned int)callback_data->menu_index);
			if (item_full_name)
				return strdup(item_full_name);
			else
				return NULL;
		}
	}
	return NULL;
}

const char *auto_profile_delete_view::_get_each_item_full_name(unsigned int index)
{
	BROWSER_LOGD("");
	if (auto_fill_form_manager::get_auto_fill_form_manager()->get_auto_fill_form_item_count() == 0)
		return NULL;
	return (auto_fill_form_manager::get_auto_fill_form_manager()->get_item_list())[index]->get_name();
}

Evas_Object *auto_profile_delete_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	Evas_Object *checkbox = NULL;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;

	if (!strcmp(part, "elm.icon")) {
		checkbox = elm_check_add(obj);
		if (!checkbox) {
			BROWSER_LOGE("Failed to add check");
			return NULL;
		}
		elm_check_state_pointer_set(checkbox, &(callback_data->is_checked));
		if (callback_data->type == DELETE_PROFILE_SELECT_ALL)
			evas_object_smart_callback_add(checkbox, "changed", __select_all_icon_clicked_cb, callback_data->user_data);
		evas_object_propagate_events_set(checkbox, EINA_FALSE);
		return checkbox;
	}
	if (!strcmp(part, "elm.icon.right")) {
		checkbox = elm_check_add(obj);
		if (!checkbox) {
			BROWSER_LOGE("Failed to add check");
			return NULL;
		}
		elm_check_state_pointer_set(checkbox, &(callback_data->is_checked));
		evas_object_smart_callback_add(checkbox, "changed", __checkbox_changed_cb, callback_data->user_data);
		evas_object_propagate_events_set(checkbox, EINA_FALSE);
	}
	return checkbox;
}

void auto_profile_delete_view::_remove_each_item_callback_data(void)
{
	BROWSER_LOGD("");

	free(m_select_all_callback_data);
	m_select_all_callback_data = NULL;
	free(m_item_callback_data);
	m_item_callback_data = NULL;

	for (unsigned int i = 0; i < m_auto_fill_form_item_callback_data_list.size(); i++) {
		free(m_auto_fill_form_item_callback_data_list[i]);
		m_auto_fill_form_item_callback_data_list[i] = NULL;
	}
}

Eina_Bool auto_profile_delete_view::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");
	auto_profile_delete_view *view = (auto_profile_delete_view *)data;
	view->m_select_all_flag = EINA_FALSE;

	return EINA_TRUE;
}

void auto_profile_delete_view::_back_delete_view(void)
{
	BROWSER_LOGD("");

	/*if (m_naviframe_item == elm_naviframe_top_item_get(m_naviframe)) {
		elm_naviframe_item_pop(m_naviframe);
		m_select_all_layout = NULL;
	}*/
}

void auto_profile_delete_view::__cancel_button_click_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	__naviframe_pop_finished_cb(data, obj, event_info);

}

void auto_profile_delete_view::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	/*auto_profile_delete_view *view = (auto_profile_delete_view *)data;

	if (view->m_naviframe_item == elm_naviframe_top_item_get(m_naviframe)) {
		if (view->m_del_genlist)
			evas_object_smart_callback_del(view->m_del_genlist, "language,changed", view->__auto_profile_delete_title_lang_changed_cb);
		elm_naviframe_item_pop(m_naviframe);
	}*/
	auto_fill_form_manager::get_auto_fill_form_manager()->get_list_view()->refresh_view();
}
