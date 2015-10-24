/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 */

#ifndef BROWSER_DLOG_H
#define BROWSER_DLOG_H

#include <dlog.h>

#define BROWSER_LOGD(fmt, args...) LOGD(fmt, ##args)
#define BROWSER_LOGI(fmt, args...) LOGI(fmt, ##args)
#define BROWSER_LOGW(fmt, args...) LOGW(fmt, ##args)
#define BROWSER_LOGE(fmt, args...) LOGE(fmt, ##args)
#define BROWSER_LOGE_IF(cond, fmt, args...) LOGE_IF(cond, fmt, ##args)
#define BROWSER_SECURE_LOGD(fmt, args...) SECURE_LOGD(fmt, ##args)
#define BROWSER_SECURE_LOGE(fmt, args...) SECURE_LOGE(fmt, ##args)

#define RETV_MSG_IF(expr, val, fmt, arg...) do { \
			if (expr) { \
				BROWSER_LOGE(fmt, ##arg); \
				return (val); \
			} \
} while (0)

#define RETV_SECURE_MSG_IF(expr, val, fmt, arg...) do { \
			if (expr) { \
				BROWSER_SECURE_LOGE(fmt, ##arg); \
				return (val); \
			} \
} while (0)

#define RETV_IF(expr, val) do { \
			if (expr) { \
				return (val); \
			} \
} while (0)

#define RETV_ABORT_MSG_IF(expr, val, fmt, arg...) do { \
			if (expr) { \
				BROWSER_LOGE(fmt, ##arg); \
				abort();\
				return (val); \
			} \
} while (0)

#define RET_MSG_IF(expr, fmt, arg...) do { \
		if (expr) { \
			BROWSER_LOGE(fmt, ##arg); \
			return; \
		} \
} while (0)

#define RET_SECURE_MSG_IF(expr, fmt, arg...) do { \
		if (expr) { \
			BROWSER_SECURE_LOGD(fmt, ##arg); \
			return; \
		} \
} while (0)

#define RET_IF(expr, fmt, arg...) do { \
		if (expr) { \
			return; \
		} \
} while (0)

#define RET_ABORT_MSG_IF(expr, fmt, arg...) do { \
			if (expr) { \
				BROWSER_LOGE(fmt, ##arg); \
				abort();\
				return; \
			} \
} while (0)

#define TRACE_BEGIN do {\
	{\
		BROWSER_LOGD("\n\033[0;35mENTER FUNCTION: %s. \033[0m\t%s:%d\n", \
		__FUNCTION__, (char *)(strrchr(__FILE__, '/')+1), __LINE__);\
	} \
} while (0)

#define TRACE_END do {\
	{\
		BROWSER_LOGD("\n\033[0;35mEXIT FUNCTION: %s. \033[0m\t%s:%d\n", \
		__FUNCTION__, (char *)(strrchr(__FILE__, '/')+1), __LINE__);\
	} \
} while (0)

#define PARAM_CHECK(expr)			RET_MSG_IF(!(expr),"INVALID PARAM RETURN")
#define PARAM_CHECK_FALSE(expr)	RETV_MSG_IF(!(expr),FALSE,"INVALID PARM RETURN FALSE")
#define PARAM_CHECK_VAL(expr, val)	RETV_MSG_IF(!(expr),val,"INVALID PARM RETURN NULL")
#define PARAM_CHECK_NULL(expr) 	RETV_MSG_IF(!(expr),NULL,"INVALID PARM RETURN NULL")

#define RET_PARAM_INVALID(expr) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d]invalid parameter!", (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__,\
			__LINE__);\
		return;\
	}\
} while (0)

#define EWK_VIEW_SD_GET(ewkView, pointer) \
    Ewk_View_Smart_Data* pointer = static_cast<Ewk_View_Smart_Data*>(evas_object_smart_data_get(ewkView))

#endif /* BROWSER_DLOG_H */

