#include "AutoFillForm.h"
#include "AutoFillFormListView.h"
#include "BrowserLogger.h"

//#include <efl_extension.h>
#include <regex.h>
#include "browser-string.h"

auto_fill_form_list_view::auto_fill_form_list_view(auto_fill_form_manager *affm)
:
	m_manager(affm),
	m_main_layout(NULL),
	m_genlist(NULL),
	m_auto_fill_form_check(NULL),
	m_plus_button(NULL),
	m_ctx_popup_more_menu(NULL),
	//m_navi_it(NULL),
	//m_multiline_1text_item_class(NULL),
	//m_dialogue_1text_item_class(NULL),
	//m_dialogue_group_title_item_class(NULL),
	//m_dialogue_bottom_item_class(NULL),
	m_delete_view(NULL)
	//m_select_popup(NULL)
{
	BROWSER_LOGD("");
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        m_edjFilePath = EDJE_DIR;
        m_edjFilePath.append("SettingsUI/SettingsMobileUI.edj");

}

auto_fill_form_list_view::~auto_fill_form_list_view(void)
{
	BROWSER_LOGD("");

	//delete_list_item_selected_popup();
	//_destroy_all_genlist_style_items();
	//_remove_each_item_callback_data();
	if (m_delete_view)
		delete m_delete_view;
	m_delete_view = NULL;

	if (m_ctx_popup_more_menu) {
#if !defined(SPLIT_WINDOW)
		evas_object_smart_callback_del(elm_object_top_widget_get(m_ctx_popup_more_menu), "rotation,changed", __rotate_ctxpopup_cb);
#endif
		evas_object_del(m_ctx_popup_more_menu);
		m_ctx_popup_more_menu = NULL;
	}

	//evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_finished_cb);
}

auto_profile_delete_view *auto_fill_form_list_view::_get_delete_view(void)
{
	BROWSER_LOGD("");
	if (!m_delete_view)
		m_delete_view = new auto_profile_delete_view();

	return m_delete_view;
}

void auto_fill_form_list_view::__change_autofillform_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_object_item_signal_emit(it, "play,touch_sound,signal", "");

	Eina_Bool auto_fill_form_enalbed;
	genlist_callback_data *cb = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)cb->user_data;

	elm_genlist_item_selected_set(cb->it, EINA_FALSE);
	auto_fill_form_enalbed = true;/*m_preference->get_auto_fill_forms_enabled();*/
	elm_check_state_set(view->m_auto_fill_form_check, !auto_fill_form_enalbed);
	__check_changed_cb(view, obj, event_info);
}

Eina_Bool auto_fill_form_list_view::_is_in_list_view(void)
{
	BROWSER_LOGD("");
	/*if (m_navi_it == elm_naviframe_top_item_get(m_naviframe))
		return EINA_TRUE;
	else*/
		return EINA_FALSE;
}

void auto_fill_form_list_view::__delete_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view*)data;
	afflv->__ctxpopup_dismissed_cb(data, obj, event_info);
	afflv->_get_delete_view()->show();
}

void auto_fill_form_list_view::__add_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view*)data;
	afflv->__ctxpopup_dismissed_cb(data, obj, event_info);
	auto_fill_form_manager::get_auto_fill_form_manager()->show_composer();
}

void auto_fill_form_list_view::__more_menu_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view *)data;
	afflv->_show_more_context_popup();
}

void auto_fill_form_list_view::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	/*auto_fill_form_list_view *afflv = (auto_fill_form_list_view *)data;
	if (afflv->m_navi_it != elm_naviframe_top_item_get(m_naviframe))
		return;

	elm_naviframe_item_pop(m_naviframe);*/
}

void auto_fill_form_list_view::__context_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	//eext_ctxpopup_back_cb(data, obj, event_info);
}

#if !defined(SPLIT_WINDOW)
Eina_Bool auto_fill_form_list_view::__resize_more_ctxpopup(void *data)
{
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view *)data;
	if (!afflv->m_ctx_popup_more_menu)
		return ECORE_CALLBACK_CANCEL;

	// Move ctxpopup to proper position.
	int x = 0;
	int y = 0;
	//platform_service ps;
	//ps.get_more_ctxpopup_position(&x, &y);
	evas_object_move(afflv->m_ctx_popup_more_menu, x, y);

	return ECORE_CALLBACK_CANCEL;
}

void auto_fill_form_list_view::__rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	__resize_more_ctxpopup(data);
}
#endif

void auto_fill_form_list_view::__ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	auto_fill_form_list_view *cp = (auto_fill_form_list_view *)data;

#if !defined(SPLIT_WINDOW)
	evas_object_smart_callback_del(elm_object_top_widget_get(cp->m_ctx_popup_more_menu), "rotation,changed", __rotate_ctxpopup_cb);
#endif
	if (obj)
		evas_object_del(obj);
	cp->m_ctx_popup_more_menu = NULL;
}

void auto_fill_form_list_view::_show_more_context_popup(void)
{
	BROWSER_LOGD("");

	/*if (!_is_in_list_view())
		return;
#if defined(SPLIT_WINDOW)
	Evas_Object *more_popup = ea_menu_popup_add(m_window);
#else
	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
#endif
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}

	elm_object_style_set(more_popup, "more/default");
	elm_ctxpopup_direction_priority_set(more_popup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", __ctxpopup_dismissed_cb, this);
	elm_ctxpopup_auto_hide_disabled_set(more_popup, EINA_TRUE);
	m_ctx_popup_more_menu = more_popup;
#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_BACK, __context_popup_back_cb, this);
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_MORE, __context_popup_back_cb, this);
#endif
#if !defined(SPLIT_WINDOW)
	evas_object_smart_callback_add(elm_object_top_widget_get(more_popup), "rotation,changed", __rotate_ctxpopup_cb, this);
#endif

	brui_ctxpopup_item_append(more_popup, BR_STRING_ADD,
			__add_item_cb, NULL, NULL, this);
	if (auto_fill_form_manager::get_auto_fill_form_manager()->get_auto_fill_form_item_count() >= 1)
		brui_ctxpopup_item_append(more_popup, BR_STRING_DELETE, __delete_item_cb, NULL, NULL, this);

#if defined(SPLIT_WINDOW)
	ea_menu_popup_move(more_popup);
#else
	__resize_more_ctxpopup(this);
#endif

	evas_object_show(more_popup);*/
}

void auto_fill_form_list_view::init(Evas_Object* parent)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
}

Eina_Bool auto_fill_form_list_view::show(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	m_main_layout = _create_main_layout(m_parent);
	if (!m_main_layout) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	//m_navi_it = elm_naviframe_item_push(m_naviframe, "IDS_BR_MBODY_AUTO_FILL_FORMS", NULL, NULL, m_main_layout, NULL);//BR_STRING_SETTINGS_AUTO_FILL_FORMS
	//elm_object_item_domain_text_translatable_set(m_navi_it, BROWSER_DOMAIN_NAME, EINA_TRUE);
	/*Evas_Object *check_box = elm_check_add(m_main_layout);
	RETV_MSG_IF(!check_box, EINA_FALSE, "Failed to add checkbox");

	elm_object_style_set(check_box, "naviframe/title_on&off");
	evas_object_propagate_events_set(check_box, EINA_FALSE);
	evas_object_smart_callback_add(check_box, "changed", __check_changed_cb, this);
	elm_object_item_part_content_set(m_main_layout, "title_right_btn", check_box);
	elm_access_info_set(check_box, ELM_ACCESS_INFO, BR_STRING_SETTINGS_AUTO_FILL_FORMS);

#if defined(HW_MORE_BACK_KEY)
		// Add HW key callback.
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_MORE, __more_menu_cb, this);
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_BACK, __back_cb, this);
#endif
	Eina_Bool auto_fill_form_enalbed = true;//m_preference->get_auto_fill_forms_enabled();
	elm_check_state_set(check_box, auto_fill_form_enalbed);
	m_auto_fill_form_check = check_box;

	Evas_Object *btn = elm_button_add(m_main_layout);
	elm_object_style_set(btn, "bottom");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(btn, "clicked", __add_profile_button_cb, this);
	elm_object_item_part_content_set(m_main_layout, "toolbar", btn);
	elm_object_domain_translatable_part_text_set(btn, NULL, BROWSER_DOMAIN_NAME, "IDS_BR_MBODY_ADD_PROFILE");

	//elm_naviframe_item_pop_cb_set(m_navi_it, __naviframe_pop_cb, this);
	//evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_finished_cb, this);*/
        evas_object_show(m_main_layout);

	return EINA_TRUE;
}

Evas_Object *auto_fill_form_list_view::refresh_view(void)
{
	BROWSER_LOGD("");
	/*elm_genlist_clear(m_genlist);

	if (_append_genlist(m_genlist) == EINA_FALSE) {
		_destroy_all_genlist_style_items();
		_remove_each_item_callback_data();
		return NULL;
	}*/

	return m_genlist;
}

/*list_view_item_clicked_popup *auto_fill_form_list_view::get_list_item_selected_popup(auto_fill_form_item *item)
{
	delete_list_item_selected_popup();

	if (item)
		m_select_popup = new list_view_item_clicked_popup(item);

	return m_select_popup;
}

void auto_fill_form_list_view::delete_list_item_selected_popup(void)
{
	BROWSER_LOGD("");
	if (m_select_popup)
		delete m_select_popup;
	m_select_popup = NULL;

	if (m_ctx_popup_more_menu)
		evas_object_del(m_ctx_popup_more_menu);
	m_ctx_popup_more_menu = NULL;
}*/

Evas_Object *auto_fill_form_list_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RETV_MSG_IF(!parent, NULL, "parent is NULL");    

        Evas_Object *layout = elm_layout_add(parent);
        if (!layout) {
                BROWSER_LOGD("elm_layout_add failed");
                return NULL;
        }
        elm_layout_file_set(layout, m_edjFilePath.c_str(), "afflv_layout");
        evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_genlist = _create_genlist(layout);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
        elm_object_part_content_set(layout, "afflv_genlist", m_genlist);
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	return layout;
}

Evas_Object *auto_fill_form_list_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
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
        m_item_class->func.content_get = NULL;//__content_get_cb;

        m_item_class->func.text_get = __text_get_cb;
        m_item_class->func.state_get = NULL;
        m_item_class->func.del = NULL;

	/*if (_create_genlist_style_items() == EINA_FALSE) {
		BROWSER_LOGE("_create_genlist_style_items failed");
		return NULL;
	}

	if (_append_genlist(genlist) == EINA_FALSE) {
		_destroy_all_genlist_style_items();
		_remove_each_item_callback_data();
		return NULL;
	}*/
	//evas_object_smart_callback_add(genlist, "realized", __genlist_item_realized_cb, this);
	//evas_object_smart_callback_add(genlist, "language,changed", common_view::__genlist_lang_changed, NULL);

	return genlist;
}

/*Eina_Bool auto_fill_form_list_view::_create_genlist_style_items(void)
{
	BROWSER_LOGD("");
	Elm_Genlist_Item_Class *description_item_class = elm_genlist_item_class_new();
	if (!description_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for description_item_class failed");
		return EINA_FALSE;
	}
	description_item_class->item_style = "multiline/1text";
	description_item_class->func.content_get = __content_get_cb;

	description_item_class->func.text_get = __text_get_cb;
	description_item_class->func.state_get = NULL;
	description_item_class->func.del = NULL;
	m_multiline_1text_item_class = description_item_class;

	Elm_Genlist_Item_Class *title_item_class = elm_genlist_item_class_new();
	if (!title_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for title_item_class failed");
		return EINA_FALSE;
	}
	title_item_class->item_style = "groupindex";

	title_item_class->func.text_get = __text_get_cb;
	title_item_class->func.content_get = NULL;
	title_item_class->func.state_get = NULL;
	title_item_class->func.del = NULL;
	m_dialogue_group_title_item_class = title_item_class;

	Elm_Genlist_Item_Class *each_form_data_menu_item_class = elm_genlist_item_class_new();
	if (!each_form_data_menu_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for each_form_data_menu_item_class failed");
		return EINA_FALSE;
	}

	each_form_data_menu_item_class->item_style = "1line";
	each_form_data_menu_item_class->func.content_get = NULL;

	each_form_data_menu_item_class->decorate_all_item_style = "edit_default";
	each_form_data_menu_item_class->func.text_get = __text_get_cb;
	each_form_data_menu_item_class->func.state_get = NULL;
	each_form_data_menu_item_class->func.del = NULL;
	m_dialogue_1text_item_class = each_form_data_menu_item_class;

	Elm_Genlist_Item_Class *dialogue_bottom_item_class = elm_genlist_item_class_new();
	if (!dialogue_bottom_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for dialogue_bottom_item_class failed");
		return EINA_FALSE;
	}
	dialogue_bottom_item_class->item_style = "separator";
	dialogue_bottom_item_class->func.text_get = NULL;
	dialogue_bottom_item_class->func.content_get = NULL;
	dialogue_bottom_item_class->func.state_get = NULL;
	dialogue_bottom_item_class->func.del = NULL;
	m_dialogue_bottom_item_class = dialogue_bottom_item_class;

	return EINA_TRUE;
}

Eina_Bool auto_fill_form_list_view::_destroy_all_genlist_style_items(void)
{
	BROWSER_LOGD("");

	if (m_multiline_1text_item_class)
		elm_genlist_item_class_free(m_multiline_1text_item_class);
	m_multiline_1text_item_class = NULL;

	if (m_dialogue_group_title_item_class)
		elm_genlist_item_class_free(m_dialogue_group_title_item_class);
	m_dialogue_group_title_item_class = NULL;

	if (m_dialogue_1text_item_class)
		elm_genlist_item_class_free(m_dialogue_1text_item_class);
	m_dialogue_1text_item_class = NULL;

	if (m_dialogue_bottom_item_class)
		elm_genlist_item_class_free(m_dialogue_bottom_item_class);
	m_dialogue_bottom_item_class = NULL;

	return EINA_TRUE;
}*/

const char *auto_fill_form_list_view::_get_each_item_full_name(unsigned int index)
{
	if (m_manager->get_auto_fill_form_item_count() == 0)
		return NULL;
	return (m_manager->get_item_list())[index]->get_name();
}

Eina_Bool auto_fill_form_list_view::_append_genlist(Evas_Object *genlist)
{
	BROWSER_LOGD("");

	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	//elm_genlist_realization_mode_set(genlist, EINA_TRUE);

	/*m_description_top_callback_data.type = aufo_fill_form_description;
	m_description_top_callback_data.user_data = this;
	m_description_top_callback_data.it = elm_genlist_item_append(genlist, m_dialogue_bottom_item_class,
						&m_description_top_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(m_description_top_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);


	m_description_callback_data.it = elm_genlist_item_append(genlist, m_multiline_1text_item_class,
						&m_description_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(m_description_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);*/

	unsigned int item_count = m_manager->get_auto_fill_form_item_count();
	if (item_count > 0) {
		/*m_bottom_pad_item_callback_data.type = aufo_fill_form_bottom;
		m_bottom_pad_item_callback_data.user_data = this;
		m_bottom_pad_item_callback_data.it = elm_genlist_item_append(genlist, m_dialogue_bottom_item_class,
							&m_bottom_pad_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							NULL, &m_bottom_pad_item_callback_data);
		elm_genlist_item_select_mode_set(m_bottom_pad_item_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);*/

		//m_auto_fill_forms_profile_title_item_callback_data.type = auto_fill_profile_title;
		//m_auto_fill_forms_profile_title_item_callback_data.user_data = this;
                for (unsigned int i = 0; i < item_count; i++) {
                   item_callback_data.menu_index = i;
                   item_callback_data.user_data = this;
		   item_callback_data.it = elm_genlist_item_append(genlist, m_item_class,
							&item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, &item_callback_data);
                }
		//elm_genlist_item_select_mode_set(m_auto_fill_forms_profile_title_item_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		//if (m_auto_fill_form_item_callback_data_list.size())
		//	_remove_each_item_callback_data();

		/*for (unsigned int i = 0; i < item_count; i++) {
			profile_item_genlist_callback_data *each_item_callback_data = (profile_item_genlist_callback_data *)malloc(sizeof(profile_item_genlist_callback_data));
			if (!each_item_callback_data)
				continue;

			memset(each_item_callback_data, 0x00, sizeof(profile_item_genlist_callback_data));
			each_item_callback_data->cd.type = auto_fill_profile_each_item;
			each_item_callback_data->cd.menu_index = i;
			each_item_callback_data->cd.user_data = this;

			each_item_callback_data->cd.it = elm_genlist_item_append(genlist, m_dialogue_1text_item_class,
								each_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
								__genlist_item_clicked_cb, each_item_callback_data);

			m_auto_fill_form_item_callback_data_list.push_back(each_item_callback_data);
		}

		m_bottom_pad_item_callback_data.type = aufo_fill_form_bottom;
		m_bottom_pad_item_callback_data.user_data = this;
		m_bottom_pad_item_callback_data.it = elm_genlist_item_append(genlist, m_dialogue_bottom_item_class,
							&m_bottom_pad_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							NULL, &m_bottom_pad_item_callback_data);

		elm_genlist_item_select_mode_set(m_bottom_pad_item_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);*/
	}

	//elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	return EINA_TRUE;
}

void auto_fill_form_list_view::_remove_each_item_callback_data(void)
{
	BROWSER_LOGD("");

	/*for (unsigned int i = 0; i < m_auto_fill_form_item_callback_data_list.size(); i++) {
		free(m_auto_fill_form_item_callback_data_list[i]);
		m_auto_fill_form_item_callback_data_list[i] = NULL;
	}*/
}

char *auto_fill_form_list_view::__text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
        genlist_callback_data *callback_data = (genlist_callback_data *)data;
        auto_fill_form_list_view *view = (auto_fill_form_list_view *)(callback_data->user_data);

        if (!strcmp(part, "title_text")) {
            const char *item_full_name = view->_get_each_item_full_name((unsigned int)callback_data->menu_index);
            if (item_full_name)
               return strdup(item_full_name);
            else
               return NULL;
	}
        return NULL;

	/*genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)callback_data->user_data;
	auto_fill_form_list_view::menu_type type = callback_data->type;

	if (type == aufo_fill_form_description_top) {
		if (!strcmp(part, "elm.text.main.left"))
			return strdup("");
	} else if (type == aufo_fill_form_description) {

		if (!strcmp(part, "elm.text.main"))
			return strdup(BR_STRING_SETTINGS_AUTO_FILL_FORMS);
		else if (!strcmp(part, "elm.text.multiline"))
			return strdup(BR_STRING_AUTO_FILL_DESC);
	}
	else if (type == auto_fill_profile_title) {
		if (!strcmp(part, "elm.text.main")) {
			if (view->m_manager->get_auto_fill_form_item_count() == 0)
				return NULL;
			else if (view->m_manager->get_auto_fill_form_item_count() == 1)
				return strdup(BR_STRING_PROFILE);
			else
				return strdup(BR_STRING_PROFILES);
		}
	} else if (type == auto_fill_profile_each_item) {
		if (!strcmp(part, "elm.text.main.left")) {
			const char *item_full_name = view->_get_each_item_full_name((unsigned int)callback_data->menu_index);
			if (item_full_name)
				return strdup(item_full_name);
			else
				return NULL;
		}
	} else
		return NULL;*/

	return NULL;
}

Evas_Object *auto_fill_form_list_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
	/*Evas_Object *checkbox = NULL;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)callback_data->user_data;
	if (!strcmp(part, "elm.icon")) {
		checkbox = elm_check_add(obj);
		if (!checkbox) {
				BROWSER_LOGE("Failed to add clear_history_check");
				return NULL;
		}
		elm_object_style_set(checkbox, "on&off");
		evas_object_propagate_events_set(checkbox, EINA_FALSE);
		evas_object_smart_callback_add(checkbox, "changed", __check_changed_cb, callback_data->user_data);
		Eina_Bool auto_fill_form_enalbed = true;//m_preference->get_auto_fill_forms_enabled();
		elm_check_state_set(checkbox, auto_fill_form_enalbed);
		view->m_auto_fill_form_check = checkbox;
	}
	return checkbox;*/
        return NULL;
}

void auto_fill_form_list_view::__check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_list_view *view = (auto_fill_form_list_view *)data;

	Eina_Bool state = elm_check_state_get(view->m_auto_fill_form_check);
	//m_preference->set_auto_fill_forms_enabled(state);

	/*if (m_browser->get_settings()->is_shown())
		m_browser->get_settings()->realize_genlist();*/
}

void auto_fill_form_list_view::__add_profile_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	/* create new profile */
	auto_fill_form_list_view *list_view = (auto_fill_form_list_view *)data;
	list_view->m_manager->show_composer();
}

void auto_fill_form_list_view::__delete_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

//	auto_fill_form_list_view *view = (auto_fill_form_list_view *)data;
}

void auto_fill_form_list_view::__back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	//elm_naviframe_item_pop(m_naviframe);
}

void auto_fill_form_list_view::__genlist_auto_fill_form_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)(callback_data->user_data);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);

	Eina_Bool state = elm_check_state_get(view->m_auto_fill_form_check);
	elm_check_state_set(view->m_auto_fill_form_check, !state);
	//m_preference->set_auto_fill_forms_enabled(!state);

	/*if (m_browser->get_settings()->is_shown())
		m_browser->get_settings()->realize_genlist();*/
}

void auto_fill_form_list_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)(callback_data->user_data);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);
	auto_fill_form_manager *manager = auto_fill_form_manager::get_auto_fill_form_manager();
	RET_MSG_IF(!manager, "auto_fill_form_manager is NULL");

	manager->show_composer((view->m_manager->get_item_list())[callback_data->menu_index]);
}

void auto_fill_form_list_view::__popup_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;

	//m_browser->get_browser_view()->destroy_popup(obj);
	auto_fill_form_manager::get_auto_fill_form_manager()->show_composer(item);
}

void auto_fill_form_list_view::__popup_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;

	//m_browser->get_browser_view()->destroy_popup(obj);
	auto_fill_form_manager::get_auto_fill_form_manager()->delete_auto_fill_form_item(item);
}

Eina_Bool auto_fill_form_list_view::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");

//	auto_fill_form_list_view *view = (auto_fill_form_list_view *)data;

	return EINA_TRUE;
}

void auto_fill_form_list_view::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	/*auto_fill_form_list_view *view = (auto_fill_form_list_view *)data;

	if (view->m_navi_it == elm_naviframe_top_item_get(m_naviframe))
		auto_fill_form_manager::get_auto_fill_form_manager()->delete_composer();*/
}

void auto_fill_form_list_view::__genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	/*BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!event_info, "event_info is NULL");

	auto_fill_form_list_view *psView = (auto_fill_form_list_view *)(data);
	Elm_Object_Item *selected_item = (Elm_Object_Item *)(event_info);
	profile_item_genlist_callback_data *callback_data = (profile_item_genlist_callback_data *)elm_object_item_data_get(selected_item);
	menu_type type = callback_data->cd.type;
	BROWSER_LOGD("type[%d]", type);

	profile_item_genlist_callback_data *cb_data = (profile_item_genlist_callback_data *)elm_object_item_data_get(selected_item);
	RET_MSG_IF(!cb_data, "cb_data is NULL");
	unsigned int iItemCount = psView->m_manager->get_auto_fill_form_item_count();

	if (elm_genlist_item_select_mode_get(selected_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		Elm_Object_Item *next_it = NULL;
		Elm_Object_Item *prev_it = NULL;
		next_it = elm_genlist_item_next_get(selected_item);
		prev_it = elm_genlist_item_prev_get(selected_item);

		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && (!next_it && (iItemCount > 1))) {
			elm_object_item_signal_emit(selected_item, "elm,state,top", "");
			return;
		}
		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && (next_it && elm_genlist_item_select_mode_get(next_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
			elm_object_item_signal_emit(selected_item, "elm,state,top", "");
			return;
		}
		if (next_it && prev_it) {
			if ((elm_genlist_item_select_mode_get(next_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
			    && (elm_genlist_item_select_mode_get(prev_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
				elm_object_item_signal_emit(selected_item, "elm,state,center", "");
				return;
			}
		}
		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && (next_it && elm_genlist_item_select_mode_get(next_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
			elm_object_item_signal_emit(selected_item, "elm,state,bottom", "");
			return;
		}
		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && !next_it) {
			elm_object_item_signal_emit(selected_item, "elm,state,bottom", "");
			return;
		}
	}

	elm_object_item_signal_emit(selected_item, "elm,state,normal", "");*/
}



/*Elm_Object_Item *brui_ctxpopup_item_append(Evas_Object *obj,
			const char *label,
			Evas_Smart_Cb func,
			const char *icon_file,
			const char *icon_group,
			const void *data)
{
	RETV_MSG_IF(!obj, NULL, "obj is NULL");
	Evas_Object *icon = NULL;

	return elm_ctxpopup_item_append(obj, label, icon, func, data);
}*/
