#include "AutoFillFormManager.h"
#include "ListViewItemClickedPopup.h"
#include "BrowserLogger.h"

//#include <efl_extension.h>
#include <regex.h>
//#include "browser-string.h"

list_view_item_clicked_popup::list_view_item_clicked_popup(auto_fill_form_item *item)
:
	m_popup(NULL),
	m_item(item),
	m_popup_last_item(NULL)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

}

list_view_item_clicked_popup::~list_view_item_clicked_popup(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	if (m_popup)
		evas_object_del(m_popup);
	m_popup = NULL;
}

Eina_Bool list_view_item_clicked_popup::show(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	Evas_Object *popup = brui_popup_add(m_window);
	RETV_MSG_IF(!popup, EINA_FALSE, "popup is NULL");
	m_popup = popup;
	evas_object_smart_callback_add(popup, "block,clicked", __cancel_button_cb, this);

#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __cancel_button_cb, this);
#endif

	elm_object_style_set(popup,"default");
	elm_object_part_text_set(popup, "title,text", m_item->get_name());
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *genlist = _create_genlist(popup);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}

	Evas_Object *box = elm_box_add(popup);
	if (!box) {
		BROWSER_LOGE("elm_box_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_min_set(box, 0, ELM_SCALE_SIZE(VIEWER_ATTACH_LIST_ITEM_HEIGHT * selected_item_num));
	evas_object_show(genlist);
	elm_box_pack_end(box, genlist);
	elm_object_content_set(popup, box);
	evas_object_show(popup);

	return EINA_TRUE;
}

void list_view_item_clicked_popup::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!(data), "data is NULL");
	RET_MSG_IF(!(event_info), "event_info is NULL");

	list_view_item_clicked_popup *popup_class = (list_view_item_clicked_popup *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	if (it == popup_class->m_popup_last_item)
		elm_object_item_signal_emit(it, "elm,state,bottomline,hide", "");
}

Evas_Object *list_view_item_clicked_popup::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}

	//evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, this);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	if (item_ic) {
		item_ic->item_style = "1text/popup";
		item_ic->func.text_get = __genlist_text_get;
		item_ic->func.content_get = NULL;
		item_ic->func.state_get = NULL;
		item_ic->func.del = NULL;

		elm_genlist_item_append(genlist, item_ic, (void *)selected_item_edit, NULL, ELM_GENLIST_ITEM_NONE, __popup_edit_cb, m_item);
		m_popup_last_item = elm_genlist_item_append(genlist, item_ic, (void *)selected_item_delete, NULL, ELM_GENLIST_ITEM_NONE, __popup_delete_cb, m_item);
		elm_genlist_item_class_free(item_ic);
	}

	return genlist;
}

char *list_view_item_clicked_popup::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	char *label = NULL;
	int type = (int)data;

	if (!strcmp(part, "elm.text")) {
		if (type == selected_item_edit)
			label = strdup(BR_STRING_EDIT);
		else if (type == selected_item_delete)
			label = strdup(BR_STRING_DELETE);
	}

	return label;
}

void list_view_item_clicked_popup::__popup_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;
	auto_fill_form_manager *manager = auto_fill_form_manager::get_auto_fill_form_manager();
	RET_MSG_IF(!manager, "auto_fill_form_manager is NULL");

	manager->get_list_view()->delete_list_item_selected_popup();
	manager->show_composer(item);
}

void list_view_item_clicked_popup::__popup_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;
	auto_fill_form_manager *manager = auto_fill_form_manager::get_auto_fill_form_manager();
	RET_MSG_IF(!manager, "auto_fill_form_manager is NULL");

	manager->get_list_view()->delete_list_item_selected_popup();
	m_browser->get_browser_view()->show_msg_popup("IDS_BR_HEADER_DELETE_PROFILE",
										"IDS_BR_POP_DELETE_Q",
										NULL,
										"IDS_BR_SK_CANCEL",
										NULL,
										"IDS_BR_SK_DELETE",
										__delete_ok_confirm_cb,
										item);
}

void list_view_item_clicked_popup::__delete_ok_confirm_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;
	auto_fill_form_manager::get_auto_fill_form_manager()->delete_auto_fill_form_item(item);
	m_browser->get_browser_view()->show_noti_popup(BR_STRING_DELETED);
}

void list_view_item_clicked_popup::__cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	list_view_item_clicked_popup *popup_class = (list_view_item_clicked_popup*)data;
	if (popup_class->m_popup) {
		evas_object_del(popup_class->m_popup);
		popup_class->m_popup = NULL;
	}
}

