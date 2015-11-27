/*
 * common.h
 *
 *  Created on: Nov 17, 2015
 *      Author: youngj
 */

#ifndef COMMON_H_
#define COMMON_H_

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

#define TRIM_SPACE " \t\n\v"
#define settings_edj_path "SettingsUI/settings.edj" //TODO yjkim
#define VIEWER_ATTACH_LIST_ITEM_HEIGHT 116
#define BROWSER_PACKAGE_NAME	"browser"
#define BROWSER_DOMAIN_NAME BROWSER_PACKAGE_NAME

typedef enum _profile_compose_mode
{
	profile_create = 0,
	profile_edit
} profile_compose_mode;

typedef enum _profile_edit_errorcode
{
	profile_edit_failed = 0,
	profile_already_exist,
	update_error_none
} profile_edit_errorcode;

typedef enum _profile_save_errorcode
{
	profile_create_failed = 0,
	duplicate_profile,
	save_error_none
} profile_save_errorcode;

typedef struct _auto_fill_form_item_data {
	unsigned profile_id;
	const char *name;
	const char *company;
	const char *primary_address;
	const char *secondary_address;
	const char *city_town;
	const char *state_province_region;
	const char *post_code;
	const char *country;
	const char *phone_number;
	const char *email_address;
	bool activation;
	profile_compose_mode compose_mode;
} auto_fill_form_item_data;

inline std::string _trim(std::string& s, const std::string& drop = TRIM_SPACE)
{
	std::string r = s.erase(s.find_last_not_of(drop) + 1);
	return r.erase(0, r.find_first_not_of(drop));
}

#endif /* COMMON_H_ */
