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

#ifndef AUTOFILLFORM_H_
#define AUTOFILLFORM_H_

#include <string>
#include <vector>

#include <Elementary.h>
#include <Evas.h>

#include <ewk_chromium.h>

#define RETV_MSG_IF(expr, val, fmt, arg...) do { \
                        if (expr) { \
                                BROWSER_LOGE(fmt, ##arg); \
                                return (val); \
                        } \
} while (0)

#define RET_MSG_IF(expr, fmt, arg...) do { \
                if (expr) { \
                        BROWSER_LOGE(fmt, ##arg); \
                        return; \
                } \
} while (0)

#define AUTO_FILL_FORM_ENTRY_MAX_COUNT 1024
#define PHONE_FIELD_VALID_ENTRIES "0123456789*#()/N,.;+ "

class AutoFillFormComposeView;
class AutoFillFormListView;
class AutoProfileDeleteView;
class AutoFillFormItem;

struct _AutoFillFormItemData;
typedef _AutoFillFormItemData AutoFillFormItemData;

class AutoFillFormManager
{
public:
    AutoFillFormManager(void);
    ~AutoFillFormManager(void);

    Eina_Bool saveAutoFillFormItem(AutoFillFormItemData *item_data);
    Eina_Bool deleteAutoFillFormItem(AutoFillFormItem *item);
    Eina_Bool deleteAllAutoFillFormItemss(void);
    unsigned int getAutoFillFormItemCount(void);
    AutoFillFormItem *createNewAutoFillFormItem(Ewk_Autofill_Profile *profile = NULL);
    AutoFillFormListView *showListView(void);
    AutoProfileDeleteView *showDeleteView(void);
    AutoFillFormListView *get_list_view(void) { return m_listView; }
    AutoFillFormComposeView *showComposer(AutoFillFormItem *item = NULL);
    AutoFillFormComposeView *getComposeView(void) { return m_composer; }
    Eina_Bool deleteListView(void);
    Eina_Bool deleteDeleteView(void);
    Eina_Bool deleteComposer(void);
    std::vector<AutoFillFormItem *> getItemList(void) { return m_AutoFillFormItemList; }
    std::vector<AutoFillFormItem *> loadEntireItemList(void);
    Eina_Bool addItemToList(AutoFillFormItem *item);
    void refreshListView();
    void init(Evas_Object* parent) { m_parent = parent; }
    void destroy() { delete this; }

    /* test */
    void seeAllData(void);
private:
    Evas_Object* m_parent;
    static void profiles_updated_cb(void* data);
    std::vector<AutoFillFormItem *> m_AutoFillFormItemList;
    AutoFillFormListView *m_listView;
    AutoFillFormComposeView *m_composer;
    AutoProfileDeleteView *m_deleteView;
};

#endif
