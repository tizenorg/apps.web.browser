/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#include "browser-history-db.h"
#include "browser-predictive-history.h"
#include "browser-view.h"

Browser_Predictive_History::Browser_Predictive_History(Evas_Object *navi_bar, Browser_History_DB *history_db,
						Browser_View *browser_view)
:
	m_navi_bar(navi_bar)
	,m_history_db(history_db)
	,m_browser_view(browser_view)
	,m_genlist(NULL)
	,m_main_layout(NULL)
{
	BROWSER_LOGD("[%s]", __func__);

	memset(m_param, 0x00, sizeof(genlist_callback_param) * BROWSER_PREDICTIVE_HISTORY_COUNT);
}

Browser_Predictive_History::~Browser_Predictive_History(void)
{
	BROWSER_LOGD("[%s]", __func__);
	edje_object_signal_emit(elm_layout_edje_get(m_browser_view->m_main_layout),
						"hide,predictive_history,signal", "");
	if (m_main_layout)
		evas_object_del(m_main_layout);
}

char *Browser_Predictive_History::__genlist_predictive_history_get(void *data,
						Evas_Object *obj, const char *part)
{
	if (!part || !data)
		return NULL;

	std::string url = *((std::string *)data);
	if(!strncmp(part, "elm.text", strlen("elm.text")))
		return strdup(url.c_str());

	return NULL;
}

Evas_Object *Browser_Predictive_History::__genlist_history_icon_get_cb(void *data,
						Evas_Object *obj, const char *part)
{
	if (part && strlen(part) > 0) {
		if (!strncmp(part, "elm.icon", strlen("elm.icon"))) {
			Evas_Object *globe_icon = elm_icon_add(obj);
			if (!elm_icon_file_set(globe_icon, BROWSER_IMAGE_DIR"/globe.png", NULL)) {
				BROWSER_LOGE("elm_icon_file_set is failed.\n");
				return NULL;
			}

			evas_object_size_hint_aspect_set(globe_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return globe_icon;
		}
	}

	return NULL;
}

void Browser_Predictive_History::__genlist_scrolled_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Predictive_History *predictive_history = (Browser_Predictive_History *)data;

	predictive_history->m_browser_view->set_edit_mode(BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF);

	elm_object_focus_allow_set(predictive_history->m_genlist, EINA_TRUE);
	elm_object_focus_set(predictive_history->m_genlist, EINA_TRUE);
}

Evas_Object *Browser_Predictive_History::create_predictive_history_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_main_layout = elm_layout_add(m_navi_bar);
	if (!m_main_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	if (!elm_layout_file_set(m_main_layout, BROWSER_EDJE_DIR"/browser-predictive-history.edj",
				"browser/predictive-history")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return NULL;
	}

	evas_object_show(m_main_layout);

	m_genlist = elm_genlist_add(m_navi_bar);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	memset(&m_item_class, 0x00, sizeof(m_item_class));
	m_item_class.item_style = "browser/1text.1icon.2";
	m_item_class.func.text_get = __genlist_predictive_history_get;
	m_item_class.func.content_get = __genlist_history_icon_get_cb;
	m_item_class.func.state_get = NULL;
	m_item_class.func.del = NULL;

	elm_object_part_content_set(m_main_layout, "elm.swallow.predictive_history_genlist", m_genlist);

	evas_object_smart_callback_add(m_genlist, "drag,stop", __genlist_scrolled_cb, this);
	evas_object_show(m_genlist);

	/* Do not allow focus to genlist because if the genlist has focus,
	 * the keypad is hided automatically when user touch the genlist. */
	elm_object_focus_allow_set(m_genlist, EINA_FALSE);

	return m_main_layout;
}

Eina_Bool Browser_Predictive_History::__load_uri_idler_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!data)
		return ECORE_CALLBACK_CANCEL;
	Browser_Predictive_History *predictive_history = (Browser_Predictive_History *)data;
	predictive_history->m_browser_view->load_url(predictive_history->m_uri_to_load.c_str());

	/* Workaround, give focus to option header cancel button to hide imf. */
	elm_object_focus_set(m_data_manager->get_browser_view()->m_option_header_cancel_button, EINA_TRUE);

	return ECORE_CALLBACK_CANCEL;
}


void Browser_Predictive_History::__predictive_history_item_clicked_cb(void *data, Evas_Object *obj,
											void *event_info)
{
	if (!data)
		return;
	genlist_callback_param *param = (genlist_callback_param *)data;
	Browser_Predictive_History *predictive_history = param->predictive_history;
	int index = param->index;
	BROWSER_LOGD("selected url=[%s]", predictive_history->m_history_list[index].c_str());

	/* Remove the <match>, </match> tag. */
	std::string source_string = predictive_history->m_history_list[index];
	string::size_type pos = string::npos;
	if((pos = source_string.find("<match>")) != string::npos)
		source_string.replace(pos, strlen("<match>"), std::string(""));

	if((pos = source_string.find("</match>")) != string::npos)
		source_string.replace(pos, strlen("</match>"), std::string(""));

	BROWSER_LOGD("source_string=[%s]", source_string.c_str());

	Evas_Object *edit_field_entry = br_elm_editfield_entry_get(predictive_history->m_browser_view->_get_activated_url_entry());
	/* Becaue of predictive hisotry. */
	evas_object_smart_callback_del(edit_field_entry, "changed", Browser_View::__url_entry_changed_cb);

 	predictive_history->m_uri_to_load = source_string;

	ecore_idler_add(__load_uri_idler_cb, predictive_history);
}

void Browser_Predictive_History::url_changed(const char *url)
{
	m_history_list.clear();
	m_history_db->get_history_list_by_partial_url(url, BROWSER_PREDICTIVE_HISTORY_COUNT, m_history_list);
	elm_genlist_clear(m_genlist);
	memset(m_param, 0x00, sizeof(genlist_callback_param) * BROWSER_PREDICTIVE_HISTORY_COUNT);

	for (int i = 0 ; i < m_history_list.size() ; i++) {
		/* Remove 'http://' prefix. */
		if (!strncmp(m_history_list[i].c_str(), BROWSER_HTTP_SCHEME, strlen(BROWSER_HTTP_SCHEME)))
			m_history_list[i] = m_history_list[i].substr(strlen(BROWSER_HTTP_SCHEME));

		/* mark matched words with match tag.
		  * The <match> tag is defined in browser-predictive-history.edc */
		int pos = m_history_list[i].find(url);
		m_history_list[i] = m_history_list[i].substr(0, pos) + std::string("<match>") + std::string(url)
					+ std::string("</match>") + m_history_list[i].substr(pos + strlen(url));

		m_param[i].predictive_history = this;
		m_param[i].index = i;
		elm_genlist_item_append(m_genlist, &m_item_class, &m_history_list[i], NULL, ELM_GENLIST_ITEM_NONE,
						__predictive_history_item_clicked_cb, &m_param[i]);
	}

	if (url && strlen(url))
		edje_object_signal_emit(elm_layout_edje_get(m_browser_view->m_main_layout),
							"show,predictive_history,signal", "");
	else
		edje_object_signal_emit(elm_layout_edje_get(m_browser_view->m_main_layout),
							"hide,predictive_history,signal", "");
}

