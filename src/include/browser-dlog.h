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

#ifndef BROWSER_DLOG_H
#define BROWSER_DLOG_H

#include <dlog.h>

#define BROWSER_LOGD(fmt, args...) LOGD("[%s: %s: %d] "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)
#define BROWSER_LOGI(fmt, args...) LOGI("[%s: %s: %d] "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)
#define BROWSER_LOGW(fmt, args...) LOGW("[%s: %s: %d] "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)
#define BROWSER_LOGE(fmt, args...) LOGE("[%s: %s: %d] "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)
#define BROWSER_LOGE_IF(cond, fmt, args...) LOGE_IF(cond, "[%s: %s: %d] "fmt, (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__, ##args)

#if defined(BROWSER_MEMALLOC_ABORT_ON)
#define new new(nothrow)
#define RET_NEWALLOC_FAILED(expr) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d] [Error] NULL returned!- blue screen is launched!",\
			(rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__),  __FUNCTION__,  __LINE__);\
		abort();\
		return;\
	}\
} while (0)
#else
#define RET_NEWALLOC_FAILED(expr) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d] [Error] NULL returned!- blue screen is launched!",\
			(rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__);\
		return;\
	}\
} while (0)
#endif

#if defined(BROWSER_MEMALLOC_ABORT_ON)
#define RETV_NEWALLOC_FAILED(expr,val) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d] [Error] NULL returned!- blue screen is launched!",\
			(rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__),  __FUNCTION__,  __LINE__);\
		abort();\
		return (val);\
	}\
} while (0)
#else
#define RETV_NEWALLOC_FAILED(expr,val) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d] [Error] NULL returned!- blue screen is launched!",\
			(rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__, __LINE__);\
		return (val);\
	}\
} while (0)
#endif

#define RET_PARAM_INVALID(expr) do {\
	if ((expr) == NULL) {\
		BROWSER_LOGE("[%s: %s: %d]invalid parameter!", (rindex(__FILE__, '/') ? rindex(__FILE__, '/') + 1 : __FILE__), __FUNCTION__,\
			__LINE__);\
		return;\
	}\
} while (0)

#endif /* BROWSER_DLOG_H */

