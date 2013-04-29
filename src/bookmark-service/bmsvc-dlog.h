/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
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

#ifndef __BMSVC_DLOG_H_
#define __BMSVC_DLOG_H_

#include <stdio.h>
#include <string.h>

#define BMSVC_USE_LOG
#ifdef BMSVC_USE_LOG

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG		"BMSVC"
#include <dlog.h>

//LOGD("[%s][%d] "fmt "\n", strrchr(__FILE__, '/')+1, __LINE__, ##arg);
#define DBG_LOGD(fmt, arg...) do {\
		LOGD(fmt "\n", ##arg);\
} while (0)
//LOGE("[%s][%d] "fmt"\n", strrchr(__FILE__, '/')+1, __LINE__, ##arg);
#define DBG_LOGE(fmt, arg...) do {\
		LOGE(fmt "\n", ##arg);\
} while (0)
#define DBG_LOGE_ASSERT(fmt, args...) do {\
		LOGE("[ASSERT][%s][%d] "fmt"\n", __func__, __LINE__, ##args);\
} while (0)
#define DBG_LOGW(fmt, args...) do {\
		LOGW("[%s][%d] "fmt"\n", __func__, __LINE__, ##args);\
} while (0)
//LOGE("[%s][%d] "fmt"\n", __func__, __LINE__, ##args);
#define DBG_LOGI(fmt, args...) do {\
		LOGE(fmt"\n", ##args);\
} while (0)
//LOGE("[ABORT][%s][%d] "fmt"\n", __func__, __LINE__, ##args);
#define DBG_ABORT(fmt, args...) do {\
		LOGE("[ABORT]"fmt"\n", ##args);\
		abort();\
} while (0)
#else

#define DBG_LOGD(fmt, arg...) do {\
		printf("[BMSVC][D]%s:%d: " fmt "\n", __FILE__, __LINE__, ##arg);\
} while (0)
#define DBG_LOGE(fmt, arg...) do {\
		printf("[BMSVC][E][%s][%d] "fmt"\n", strrchr(__FILE__, '/')+1, __LINE__, ##arg);\
} while (0)
#define DBG_LOGE_ASSERT(fmt, args...) do {\
		printf("[BMSVC][A][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args);\
} while (0)
#define DBG_LOGW(fmt, args...) do {\
		printf("[BMSVC][W][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args);\
} while (0)
#define DBG_LOGI(fmt, args...) do {\
		printf("[BMSVC][I][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args);\
} while (0)
#define DBG_ABORT(fmt, args...) do {\
		printf("[BMSVC][AB][%40s:%4d] "fmt"\n", __func__, __LINE__, ##args);\
} while (0)
#endif

#define RETV_MSG_IF(expr, val, fmt, arg...) do { \
			if (expr) { \
				DBG_LOGE(fmt, ##arg); \
				return (val); \
			} \
} while (0)

#define RETV_IF(expr, val) do { \
			if (expr) { \
				return (val); \
			} \
} while (0)


#define RET_MSG_IF(expr, fmt, arg...) do { \
		if (expr) { \
			DBG_LOGE(fmt, ##arg); \
			return; \
		} \
} while (0)

#define RET_IF(expr, fmt, arg...) do { \
		if (expr) { \
			return; \
		} \
} while (0)

#define TRACE_BEGIN do {\
	{\
		DBG_LOGD("\n\033[0;35mENTER FUNCTION: %s. \033[0m\t%s:%d\n", \
		__FUNCTION__, (char *)(strrchr(__FILE__, '/')+1), __LINE__);\
	} \
} while (0) ;

#define TRACE_END do {\
	{\
		DBG_LOGD("\n\033[0;35mEXIT FUNCTION: %s. \033[0m\t%s:%d\n", \
		__FUNCTION__, (char *)(strrchr(__FILE__, '/')+1), __LINE__);\
	} \
} while (0) ;

#define PARAM_CHECK(expr)			RET_MSG_IF(!(expr),"INVALID PARAM RETURN")
#define PARAM_CHECK_FALSE(expr)	RETV_MSG_IF(!(expr),FALSE,"INVALID PARM RETURN FALSE")
#define PARAM_CHECK_VAL(expr, val)	RETV_MSG_IF(!(expr),val,"INVALID PARM RETURN NULL")
#define PARAM_CHECK_NULL(expr) 	RETV_MSG_IF(!(expr),NULL,"INVALID PARM RETURN NULL")

/* old log. temporary keeping*/
#define ENALBE_DEBUG_LOG
#ifdef ENALBE_DEBUG_LOG
#define __BOOKMARK_DEBUG_TRACE(format, ARG...) do {\
{ \
	DBG_LOGD(format, ##ARG);\
} \
} while (0) ;
#else
#define __BOOKMARK_DEBUG_TRACE(format, ARG...) do {\

} while (0) ;
#endif
#endif
