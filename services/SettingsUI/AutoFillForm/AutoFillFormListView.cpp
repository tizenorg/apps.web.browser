#include "AutoFillFormManager.h"
#include "AutoFillFormListView.h"
#include "AutoFillFormItem.h"
#include "AutoProfileDeleteView.h"
#include "BrowserLogger.h"

#include <regex.h>
#include "browser-string.h"

auto_fill_form_list_view::auto_fill_form_list_view(auto_fill_form_manager *affm)
:
	m_manager(affm),
	m_main_layout(NULL),
	m_genlist(NULL),
	m_auto_fill_form_check(NULL),
	m_plus_button(NULL),
	m_ctx_popup_more_menu(NULL)
	//m_select_popup(NULL)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
        BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
        m_edjFilePath = EDJE_DIR;
        m_edjFilePath.append("SettingsUI/AutoFillMobileUI.edj");

}

auto_fill_form_list_view::~auto_fill_form_list_view(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	//delete_list_item_selected_popup();

        if (m_genlist) {
            elm_genlist_clear(m_genlist);
            evas_object_del(m_genlist);
        }
        if (m_main_layout) {
            evas_object_hide(m_main_layout);
            evas_object_del(m_main_layout);
        }
/*	if (m_ctx_popup_more_menu) {
		evas_object_smart_callback_del(elm_object_top_widget_get(m_ctx_popup_more_menu), "rotation,changed", __rotate_ctxpopup_cb);
		evas_object_del(m_ctx_popup_more_menu);
		m_ctx_popup_more_menu = NULL;
	}*/
}

/*auto_profile_delete_view *auto_fill_form_list_view::_get_delete_view(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	if (!m_delete_view)
		m_delete_view = new auto_profile_delete_view(m_manager);

	return m_delete_view;
}*/

/*void auto_fill_form_list_view::__change_autofillform_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_object_item_signal_emit(it, "play,touch_sound,signal", "");

	Eina_Bool auto_fill_form_enalbed;
	genlist_callback_data *cb = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)cb->user_data;

	elm_genlist_item_selected_set(cb->it, EINA_FALSE);
	auto_fill_form_enalbed = m_preference->get_auto_fill_forms_enabled();
	elm_check_state_set(view->m_auto_fill_form_check, !auto_fill_form_enalbed);
	__check_changed_cb(view, obj, event_info);
}

Eina_Bool auto_fill_form_list_view::_is_in_list_view(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	return EINA_FALSE;
}*/

void auto_fill_form_list_view::__more_menu_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view *)data;
	afflv->_show_more_context_popup();
}

void auto_fill_form_list_view::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
}

void auto_fill_form_list_view::__context_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	//eext_ctxpopup_back_cb(data, obj, event_info);
}

#if 0
Eina_Bool auto_fill_form_list_view::__resize_more_ctxpopup(void *data)
{
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view *)data;
	if (!afflv->m_ctx_popup_more_menu)
		return ECORE_CALLBACK_CANCEL;

	// Move ctxpopup to proper position.
	int x = 0;
	int y = 0;
	platform_service ps;
	ps.get_more_ctxpopup_position(&x, &y);
	evas_object_move(afflv->m_ctx_popup_more_menu, x, y);

	return ECORE_CALLBACK_CANCEL;
}

void auto_fill_form_list_view::__rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	__resize_more_ctxpopup(data);
}

void auto_fill_form_list_view::__ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
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
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	if (!_is_in_list_view())
		return;
	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
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
	evas_object_smart_callback_add(elm_object_top_widget_get(more_popup), "rotation,changed", __rotate_ctxpopup_cb, this);

	brui_ctxpopup_item_append(more_popup, BR_STRING_ADD,
			__add_item_cb, NULL, NULL, this);
	if (m_manager->get_auto_fill_form_item_count() >= 1)
		brui_ctxpopup_item_append(more_popup, BR_STRING_DELETE, __delete_item_cb, NULL, NULL, this);

	__resize_more_ctxpopup(this);

	evas_object_show(more_popup);
}
#endif

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
	/*Eina_Bool auto_fill_form_enalbed = true;//m_preference->get_auto_fill_forms_enabled();
	elm_check_state_set(check_box, auto_fill_form_enalbed);
	m_auto_fill_form_check = check_box;*/
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

/*list_view_item_clicked_popup *auto_fill_form_list_view::get_list_item_selected_popup(auto_fill_form_item *item)
{
	delete_list_item_selected_popup();

	if (item)
		m_select_popup = new list_view_item_clicked_popup(item);

	return m_select_popup;
}

void auto_fill_form_list_view::delete_list_item_selected_popup(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
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
        /*elm_genlist_homogeneous_set(genlist, EINA_FALSE);
        elm_genlist_multi_select_set(genlist, EINA_FALSE);
        elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
        elm_genlist_mode_set(genlist, ELM_LIST_LIMIT);
        elm_genlist_decorate_mode_set(genlist, EINA_TRUE);
        evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);*/

        m_item_class = elm_genlist_item_class_new();
        if (!m_item_class) {
                BROWSER_LOGE("elm_genlist_item_class_new for description_item_class failed");
                return EINA_FALSE;
        }
        m_item_class->item_style = "afflv_item";
        m_item_class->func.content_get = NULL; //__content_get_cb;

        m_item_class->func.text_get = __text_get_cb;
        m_item_class->func.state_get = NULL;
        m_item_class->func.del = NULL;

        _append_genlist(genlist);

	//evas_object_smart_callback_add(genlist, "realized", __genlist_item_realized_cb, this);
	//evas_object_smart_callback_add(genlist, "language,changed", common_view::__genlist_lang_changed, NULL);

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

	//evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

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
	//elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

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
            //return strdup("Profile 1");
	}
        return NULL;

}

/*Evas_Object *auto_fill_form_list_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
	Evas_Object *checkbox = NULL;
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
	return checkbox;
        return NULL;
}

void auto_fill_form_list_view::__check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_list_view *view = (auto_fill_form_list_view *)data;

	Eina_Bool state = elm_check_state_get(view->m_auto_fill_form_check);
	//m_preference->set_auto_fill_forms_enabled(state);

	if (m_browser->get_settings()->is_shown())
		m_browser->get_settings()->realize_genlist();
}*/

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

#if 0
void auto_fill_form_list_view::__popup_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;

	//m_browser->get_browser_view()->destroy_popup(obj);
	m_manager->show_composer(item);
}

void auto_fill_form_list_view::__popup_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;

	//m_browser->get_browser_view()->destroy_popup(obj);
	m_manager->delete_auto_fill_form_item(item);
}

void auto_fill_form_list_view::__genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
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

	elm_object_item_signal_emit(selected_item, "elm,state,normal", "");
}

Elm_Object_Item *brui_ctxpopup_item_append(Evas_Object *obj,
			const char *label,
			Evas_Smart_Cb func,
			const char *icon_file,
			const char *icon_group,
			const void *data)
{
	RETV_MSG_IF(!obj, NULL, "obj is NULL");
	Evas_Object *icon = NULL;

	return elm_ctxpopup_item_append(obj, label, icon, func, data);
}
#endif
