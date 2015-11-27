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
 * Contact: Ankit Prabhat <ankit.pr@samsung.com>
 *
 */

#ifndef BROWSER_STRING_H
#define BROWSER_STRING_H

#include <app.h>

/* browser string define for translation */

/* System string */
/*************************************************************************************************************/
#define BR_STRING_OK               _("IDS_BR_SK_OK")
#define BR_STRING_EDIT             _("IDS_BR_BUTTON_EDIT")
#define BR_STRING_CANCEL           _("IDS_BR_SK_CANCEL")
#define BR_STRING_SAVE             _("IDS_BR_SK_SAVE")
#define BR_STRING_DONE             _("IDS_BR_SK_DONE")
#define BR_STRING_LOGIN            _("IDS_BR_BODY_LOG_IN")
#define BR_STRING_CALL             _("IDS_BR_OPT_CALL")
#define BR_STRING_BACK             _("IDS_BR_SK_BACK")
#define BR_STRING_INTERNET         _("IDS_BR_BODY_INTERNET")
#define BR_STRING_STREAMING_PLAYER _("IDS_BR_BODY_STREAMING")
#define BR_STRING_NO_NAME          _("IDS_BR_BODY_NO_NAME")
#define BR_STRING_DELETE           _("IDS_BR_SK_DELETE")
#define BR_STRING_WARNING          dgettext("sys_string", "Warning")
#define BR_STRING_OPT_SELECTED     _("IDS_BR_OPT_SELECTED")
#define BR_STRING_ON               _("IDS_BR_BODY_ON")
#define BR_STRING_OFF              _("IDS_BR_BODY_OFF")
#define BR_STRING_OPT_DISABLED     _("IDS_BR_POP_DISABLED")
#define BR_STRING_OPT_ENABLED      dgettext("sys_string", "IDS_COM_POP_ENABLED")
#define BR_STRING_PROCESSING       _("IDS_BR_BODY_PROCESSING_ING")
/*************************************************************************************************************/

/* Browser common */
/*************************************************************************************************************/
#define BR_STRING_REORDER               _("IDS_BR_OPT_REORDER_ABB")
#define BR_STRING_ITEMS_SELECTED        _("IDS_BR_POP_PD_ITEMS_SELECTED")
#define BR_STRING_ONE_ITEM_SELECTED     _("IDS_BR_POP_1_ITEM_SELECTED")
#define BR_STRING_HISTORY               _("IDS_BR_TAB_HISTORY")
#define BR_STRING_MAX_CHARACTER_WARNING _("IDS_BR_BODY_THE_MAXIMUM_NUMBER_OF_CHARACTERS_HPD_HAS_BEEN_EXCEEDED")
#define BR_STRING_BODY_WEB_ADDRESS _("IDS_BR_BODY_WEB_ADDRESS")
#define BR_STRING_BODY_TITLE            _("IDS_BR_BODY_TITLE")
#define BR_STRING_CLEAR_ALL		_("IDS_BR_OPT_CLEAR_ALL")
#define BR_STRING_REFRESH		_("IDS_BR_OPT_REFRESH")
#define BR_STRING_URL                   _("IDS_BR_BODY_URL")
#define BR_STRING_DISK_FULL		_("IDS_BR_POP_NOT_ENOUGH_MEMORY_DELETE_SOME_ITEMS_AND_TRY_AGAIN")
#define BR_STRING_NO_WINDOWS		_("IDS_BR_BODY_NO_WINDOWS")
#define BR_STRING_URL_GUIDE_TEXT        _("IDS_BR_BODY_SEARCH_OR_ENTER_URL")
#define BR_STRING_SELECTED		_("IDS_BR_HEADER_PD_SELECTED_ABB")
#define BR_STRING_SHARE                 _("IDS_BR_OPT_SHARE")
#define BR_STRING_GOBACKWARD		_("IDS_BR_OPT_GOBACKWARD")
#define BR_STRING_GOFORWARD		_("IDS_BR_OPT_GOFORWARD")
#define BR_STRING_NO_SEARCH_RESULT      _("IDS_BR_BODY_NO_RESULTS_FOUND")
#define BR_STRING_NO_SAVED_PAGE		_("IDS_BR_BODY_NO_SAVED_PAGES")
#define BR_STRING_NEW_WINDOW            _("IDS_BR_OPT_NEW_WINDOW")
#ifdef ENABLE_INCOGNITO_WINDOW
#define BR_STRING_NEW_INCOGNITO_WINDOW  _("IDS_BR_OPT_NEW_INCOGNITO_WINDOW")
#endif
#define BR_STRING_ADD                   _("IDS_BR_OPT_ADD")
#define BR_STRING_NO_TITLE              _("IDS_BR_BODY_NO_TITLE")
#define BR_STRING_CLOSE_ALL             _("IDS_BR_OPT_CLOSE_ALL")
#define BR_STRING_ADDED                 _("IDS_BR_OPT_ADDED")
#define BR_STRING_SETTINGS              _("IDS_BR_BODY_SETTINGS")
#define BR_STRING_OPT_DISABLE           _("IDS_BR_OPT_DISABLED")
/*************************************************************************************************************/

/* Bookmark */
/*************************************************************************************************************/
#define BR_STRING_ENTER_BOOKMARK_NAME   _("IDS_BR_POP_ENTER_BOOKMARK_NAME")
#define BR_STRING_TARGET_FOLDER         _("IDS_BR_BODY_TARGET_FOLDER")
#define BR_STRING_NO_BOOKMARKS          _("IDS_BR_BODY_NO_BOOKMARKS")
#define BR_STRING_ADDED_TO_BOOKMARKS    _("IDS_BR_POP_ADDED_TO_BOOKMARKS")
#define BR_STRING_DOWNLOADING_ING       _("IDS_BR_POP_STARTING_DOWNLOAD_ING")
#define BR_STRING_ADD_BOOKMARK          _("IDS_BR_OPT_ADD_BOOKMARK")
#define BR_STRING_UNTAGGED              _("IDS_BR_BODY_UNTAGGED")
#define BR_STRING_1_POPUP_BLOCKED       _("IDS_BR_TPOP_1_POP_UP_BLOCKED")
#define BR_STRING_PD_POPUPS_BLOCKED     _("IDS_BR_BODY_PD_POP_UPS_BLOCKED_ABB")
#define BR_STRING_EDIT_BOOKMARK         _("IDS_BR_HEADER_EDIT_BOOKMARK")
#define BR_STRING_SAVED_TO_BOOKMARK     _("IDS_BR_POP_SAVED_TO_BOOKMARKS")
#define BR_STRING_BOOKMARK_DELETED      _("IDS_BR_TPOP_BOOKMARK_DELETED")
#define BR_STRING_CREATE_FOLDER         _("IDS_BR_SK3_CREATE_FOLDER")
#define BR_STRING_PD_ITEMS_DELETED      _("IDS_BR_BODY_PD_ITEMS_WILL_BE_DELETED")
#define BR_STRING_1_ITEM_DELETED        _("IDS_BR_BODY_1_ITEM_WILL_BE_DELETED")
/*************************************************************************************************************/

/*History*/
/*************************************************************************************************************/
#define BR_STRING_NO_HISTORY                     _("IDS_BR_BODY_NO_HISTORIES")
#define BR_STRING_HISTORY_TODAY                  _("IDS_BR_BODY_TODAY")
#define BR_STRING_HISTORY_YESTERDAY              _("IDS_BR_BODY_YESTERDAY")
#define BR_STRING_HISTORY_LAST_7_DAYS            _("IDS_BR_BODY_LAST_7_DAYS")
#define BR_STRING_HISTORY_LAST_MONTH             _("IDS_BR_BODY_LAST_MONTH")
#define BR_STRING_HISTORY_OLDER                  _("IDS_BR_BODY_OLDER")
#define BR_STRING_CLEAR                          _("IDS_BR_SK_CLEAR")
#define BR_STRING_HISTORY_NO_CONTENT_HELP_TEXT   _("IDS_BR_BODY_AFTER_YOU_VIEW_WEBPAGES_THEY_WILL_BE_SHOWN_HERE")
/*************************************************************************************************************/

/* Auto fill form */
/*************************************************************************************************************/
#define BR_STRING_PRIMARY_ADDRESS_GUIDE_TEXT    _("IDS_BR_BODY_STREET_ADDRESS_PO_BOX_C_O_ETC_ABB")
#define BR_STRING_SECONDARY_ADDRESS_GUIDE_TEXT  _("IDS_BR_BODY_FLAT_SUITE_UNIT_BUILDING_FLOOR_ETC_ABB")
#define BR_STRING_ENTER_NAME                    _("IDS_BR_BODY_ENTER_NAME")
#define BR_STRING_ENTER_YOUR_NAME               _("IDS_BR_BODY_ENTER_YOUR_NAME_ABB")
#define BR_STRING_ENTER_COMPANY_NAME            _("IDS_BR_BODY_ENTER_COMPANY_NAME_ABB")
#define BR_STRING_ENTER_TOWN_CITY_COUNTY        _("IDS_BR_BODY_ENTER_TOWN_CITY_COUNTY_ABB")
#define BR_STRING_ENTER_POSTCODE                _("IDS_BR_BODY_ENTER_POSTCODE_ABB")
#define BR_STRING_ENTER_COUNTRY_REGION          _("IDS_BR_BODY_ENTER_COUNTRY_REGION_ABB")
#define BR_STRING_TOWN_AUTO_FILL_CITY_COUNTY    _("IDS_BR_MBODY_TOWN_CITY_COUNTY")
#define BR_STRING_PROFILE                       _("IDS_BR_BODY_PROFILE")
#define BR_STRING_PROFILES                      _("IDS_BR_HEADER_PROFILES")
#define BR_STRING_AUTO_FILL_DATA_FULL_NAME      _("IDS_BR_BODY_FULL_NAME_ABB")
#define BR_STRING_AUTO_FILL_DATA_COMPANY_NAME   _("IDS_BR_BODY_COMPANY_NAME_ABB")
#define BR_STRING_AUTO_FILL_DATA_ADDRESS_LINE_1 _("IDS_BR_BODY_ADDRESS_LINE_1_ABB")
#define BR_STRING_AUTO_FILL_DATA_ADDRESS_LINE_2 _("IDS_BR_BODY_ADDRESS_LINE_2_ABB")
#define BR_STRING_AUTO_FILL_DATA_CITY_TOWN      _("IDS_BR_BODY_CITY_TOWN_ABB")
#define BR_STRING_AUTO_FILL_DATA_COUNTRY_REGION _("IDS_BR_MBODY_TOWN_CITY_COUNTY")
#define BR_STRING_AUTO_FILL_DATA_COUNTRY        _("IDS_BR_MBODY_COUNTRY_REGION")
#define BR_STRING_AUTO_FILL_DATA_PHONE          _("IDS_BR_BODY_PHONE")
#define BR_STRING_AUTO_FILL_DATA_EMAIL          _("IDS_BR_OPT_SENDURLVIA_EMAIL")
#define BR_STRING_AUTO_FILL_DATA_POST_CODE      _("IDS_BR_BODY_POSTCODE_ABB")
#define BR_STRING_AUTO_FILL_DESC                _("IDS_BR_SBODY_SET_TEXT_USED_TO_FILL_IN_ONLINE_FORMS_ABB")
#define BR_STRING_SETTINGS_THIS_PROFILE_DELETED _("IDS_BR_BODY_1_PROFILE_WILL_BE_DELETED")
/*************************************************************************************************************/

/* Html feature */
/*************************************************************************************************************/
#define BR_STRING_REQUEST_WEBPAGE_PREFIX              _("IDS_BR_OPT_SAVEWEBPAGE")
#define BR_STRING_EXCEEDED_QUOTA_POPUP_DESC           _("IDS_BR_POP_P1SS_HP2SS_IS_REQUESTING_PERMISSION_TO_STORE_DATA_ON_YOUR_DEVICE_FOR_OFFLINE_USE")
#define BR_STRING_PROTOCOL_CONTENT_HANDLER_POPUP_DESC _("IDS_BR_POP_P1SS_HP2SS_IS_REQUESTING_PERMISSION_TO_CHANGE_HOW_YOUR_BROWSER_WORKS_ON_MSG")
#define BR_STRING_APPCACHE_PERMISSION_POPUP_DESC      _("IDS_BR_POP_P1SS_HP2SS_IS_ATTEMPTING_TO_STORE_A_LARGE_AMOUNT_OF_DATA_ON_YOUR_DEVICE_FOR_OFFLINE_USE")
#define BR_STRING_USER_MEDIA_PERMISSION_POPUP_TITLE   _("IDS_BR_POP_REQUEST_PERMISSION_TO_USE_YOUR_CAMERA_TITLE")
#define BR_STRING_USER_MEDIA_PERMISSION_POPUP_DESC    _("IDS_BR_POP_P1SS_HP2SS_IS_REQUESTING_PERMISSION_TO_USE_YOUR_CAMERA")
#define BR_STRING_WEB_NOTI_PERMISSION_POPUP_DESC      _("IDS_BR_POP_P1SS_HP2SS_IS_REQUESTING_PERMISSION_TO_SHOW_NOTIFICATIONS")
/*************************************************************************************************************/

/* More menu */
/*************************************************************************************************************/
#define BR_STRING_HOME_SCREEN_AS_SHORTCUT_ABB _("IDS_BR_OPT_HOME_SCREEN_AS_SHORTCUT_ABB")
#define BR_STRING_BOOKMARK                    _("IDS_BR_OPT_BOOKMARK")
#define BR_STRING_DESKTOP_VIEW                _("IDS_BR_BODY_DESKTOP_VIEW")
#define BR_STRING_MOBILE_VIEW                 _("IDS_BR_BODY_MOBILE_VIEW")
#define BR_STRING_FIND_ON_PAGE                _("IDS_BR_OPT_FIND_ON_PAGE")
#define BR_STRING_BOOKMARKS                   _("IDS_BR_BODY_BOOKMARKS")
#define BR_STRING_SAVED_PAGE                  _("IDS_BR_OPT_SAVEDPAGES")
#define BR_STRING_DELETE_SAVED_PAGES          _("IDS_BR_OPT_DELETE_SAVED_PAGE_ABB")
#define BR_STRING_DELETE_THIS_SAVED_PAGE      _("IDS_BR_POP_DELETE_THIS_PAGE_Q")
#define BR_STRING_DELETE_MANY_SAVED_PAGES     _("IDS_BR_POP_DELETE_PD_SAVED_WEB_PAGES_Q")
/*************************************************************************************************************/

/*************************************************************************************************************/
#define BR_STRING_NEVER                               _("IDS_BR_POP_NEVER")
#define BR_STRING_SELECT_BOOKMARK                     _("IDS_BR_HEADER_SELECT_BOOKMARK")
#define BR_STRING_SELECT_BOOKMARKS                    _("IDS_BR_HEADER_SELECT_BOOKMARKS")
/*************************************************************************************************************/

/*settings*/
/*************************************************************************************************************/
#define BR_STRING_CACHE                              _("IDS_BR_OPT_CACHE")
#define BR_STRING_ALL								 _("IDS_BR_BODY_ALL")
#define BR_STRING_COOKIES                            _("IDS_BR_BODY_COOKIES")
#define BR_STRING_SELECT_ALL                         _("IDS_BR_OPT_SELECT_ALL")
#define BR_STRING_BASIC                              _("IDS_BR_BODY_BASIC")
#define BR_STRING_CONTENT                            _("IDS_BR_MBODY_CONTENT")
#define BR_STRING_LOCATION_INFO                      _("IDS_BR_BODY_LOCATION_M_INFORMATION")
#define BR_STRING_PREDICTIVE_SUGGESTION_DESC         _("IDS_BR_SBODY_SET_THE_DEVICE_TO_SUGGEST_QUERIES_AND_SITES_IN_THE_WEB_ADDRESS_BAR_AS_YOU_TYPE")
#define BR_STRING_PRELOAD_AVAILABLE_LINKS            _("IDS_BR_MBODY_PRELOAD_AVAILABLE_LINKS")
#define BR_STRING_REMEMBER_FORM_DATA                 _("IDS_BR_SBODY_REMEMBER_DATA_TYPED_IN_FORMS_FOR_LATER_USE")
#define BR_STRING_FORCE_ZOOM_DESC                    _("IDS_BR_SBODY_OVERRIDE_WEBSITE_REQUESTS_TO_CONTROL_ZOOM_LEVEL")
#define BR_STRING_TEXT_ENCODING_DESC                 _("IDS_BR_BODY_AUTO_DETECT")
#define BR_STRING_TEXT_BOOSTER_DESC                  _("IDS_BR_BODY_CHANGE_THE_TEXT_SIZE_FOR_EASIER_READING_ESPECIALLY_WHEN_VIEWING_DESKTOP_VERSIONS_OF_WEBSITES_ON_YOUR_DEVICE_MSG")
#define BR_STRING_ACCEPT_COOKIES_DESC                _("IDS_BR_POP_ALLOW_SITES_TO_SAVE_AND_READ_COOKIES")
#define BR_STRING_ENABLE_LOCATION_DESC               _("IDS_BR_BODY_ALLOW_SITES_TO_ACCESS_YOUR_LOCATION_DATA")
#define BR_STRING_DEFAULT_STORAGE                    _("IDS_BR_BODY_DEFAULT_STORAGE")
#define BR_STRING_FORMDATA                           _("IDS_BR_HEADER_FORM_DATA")
#define BR_STRING_WEBSITES                           _("IDS_BR_MBODY_WEBSITES")
#define BR_STRING_WEBSITES_DESC                      _("IDS_BR_SBODY_SET_ADVANCED_SETTINGS_FOR_INDIVIDUAL_WEBSITES")
#define BR_STRING_DELETE_WEBSITE_DATA_ABB            _("IDS_BR_HEADER_DELETE_WEBSITE_DATA_ABB")
#define BR_STRING_CLEAR_NOTIFICATIONS                _("IDS_BR_MBODY_CLEAR_NOTIFICATION_DATA")
#define BR_STRING_RESTORE_DEFAULT_DESC               _("IDS_BR_BODY_RESTORE_DEFAULT_SETTINGS")
#define BR_STRING_RESTORE_DEFAULT_POPUP_TITLE        _("IDS_BR_BODY_RESET_TO_DEFAULT")
#define BR_STRING_PASSWORD                           _("IDS_BR_BODY_PASSWORD")
#define BR_STRING_CLEAR_HISTORY                      _("IDS_BR_BODY_CLEAR_HISTORY")
#define BR_STRING_SEARCH                             _("IDS_BR_BODY_SEARCH")
#define BR_STRING_HOMEPAGE                           _("IDS_BR_BODY_HOMEPAGE")
#define BR_STRING_HOMEPAGE_ABB                       _("IDS_BR_BUTTON_HOMEPAGE_ABB")
#define BR_STRING_DRAG_AND_DROP                      _("IDS_BR_OPT_DRAG_AND_DROP")
#define BR_STRING_ENTER_ZIPCODE	                     _("IDS_BR_BODY_ENTER_POSTCODE")
#define BR_STRING_CLEAR_LOCATION_ACCESS              _("IDS_BR_BODY_CLEAR_LOCATION_ACCESS")
#define BR_STRING_CUSTOM_USER_AGENT                  _("IDS_BR_BODY_CUSTOM_USER_AGENT")
#define BR_STRING_LOCATION                           _("IDS_BR_BODY_LOCATION_M_INFORMATION")
#define BR_STRING_PASSWORDS                          _("IDS_BR_OPT_PASSWORDS")
#define BR_STRING_SETTINGS_RESET_MSG                 _("IDS_BR_POP_ALL_SETTINGS_WILL_BE_RESET_TO_THEIR_DEFAULTS")
/*************************************************************************************************************/

/* Popup, inform and warnings */
/*************************************************************************************************************/
#define BR_STRING_NOTI_DELETED_BOOKMARK		_("Bookmark deleted.")
#define BR_STRING_DELETED                       _("IDS_BR_POP_DELETED")
#define BR_STRING_ENTER_URL                     _("IDS_BR_POP_ENTER_URL")
#define BR_STRING_AUTH_REQUIRED                 _("IDS_BR_BODY_DESTINATIONS_AUTHENTICATION_REQUIRED")
#define BR_STRING_USER_NAME                     _("IDS_BR_BODY_AUTHUSERNAME")
#define BR_STRING_ALREADY_EXISTS                _("IDS_BR_POP_ALREADY_EXISTS")
#define BR_STRING_SAVED                         _("IDS_BR_POP_SAVED")
#define BR_STRING_CTXMENU_TRANSLATE             _("IDS_BR_OPT_TRANSLATE")
#define BR_STRING_ENTER_TITLE                   _("IDS_BR_POP_ENTER_TITLE_ABB")
#define BR_STRING_FAILED                        _("IDS_BR_POP_FAIL")
#define BR_STRING_SELECT_AVAILABLE_APP          _("IDS_BR_OPT_SELECT")
#define BR_STRING_DOWNLOAD_COMPLETE             _("IDS_BR_POP_DOWNLOADCOMPLETE")
#define BR_STRING_SETTING_SAVED                 _("IDS_BR_POP_SETTIGS_SAVED")
#define BR_STRING_NO_NETWORK_CONNECTION         _("IDS_BR_HEADER_NO_NETWORK_CONNECTION")
#define BR_STRING_SET_AS_HOMEPAGE               _("IDS_BR_POP_SET_AS_HOMEPAGE")
#define BR_STRING_HTTP_URL_CAN_BE_DOWNLOADED    _("IDS_BR_POP_ONLY_HTTP_OR_HTTPS_URLS_CAN_BE_DOWNLOADED")
#define BR_STRING_INVALID_URL                   _("IDS_BR_POP_INVALID_URL")
#define BR_STRING_ENABLE_SECRET_MODE_POPUP_MSG  _("IDS_BR_POP_PAGES_THAT_YOU_VIEW_WILL_NOT_APPEAR_IN_YOUR_BROWSER_HISTORY_OR_SEARCH_HISTORY_AND_THEY_WILL_NOT_LEAVE_OTHER_TRACES_LIKE_COOKIES")
#define BR_STRING_WIFI                          _("IDS_COM_BODY_WI_FI")
#define BR_STRING_CONNECT_TO_WI_FI              _("IDS_BR_POP_WI_FI_CONNECTION_REQUIRED_CONNECT_TO_WI_FI_NETWORK_AND_TRY_AGAIN")

// csr-framework
#if defined(CSRFW)
#define BR_STRING_OPEN                          _("IDS_BR_OPT_OPEN")
#define BR_STRING_TCS_GO_BACK                   _("IDS_BR_BUTTON_BACK_ABB")
#define BR_STRING_TCS_DESCRIPTION_LEVEL0        _("IDS_BR_POP_P1SS_HAS_BEEN_DETECTED_DETECTED_IN_C_P2SS_OPEN_WEBPAGE_ANYWAY_Q")
#define BR_STRING_TCS_DESCRIPTION_LEVEL1        _("IDS_BR_POP_P1SS_HAS_BEEN_DETECTED_DETECTED_IN_C_P2SS_SOME_CONTENT_HAS_BEEN_BLOCKED_TO_PROTECT_YOUR_DEVICE")
#endif
/*************************************************************************************************************/

/* Hard coded text */
/*************************************************************************************************************/
#define BR_STRING_ENTER_PHONE_NUMBER                _("Enter your phone number")
#define BR_STRING_ENTER_EMAIL_ADDRESS               _("Enter your Email address")
#define BR_STRING_ENABLE_NOTIFICATIONS              _("Enable notifications")
#define BR_STRING_HISTORY_WILLBE_CLEARED            _("IDS_BR_POP_YOUR_HISTORY_WILL_BE_CLEARED")
#define BR_STRING_RESTORE_DEFAULT_POPUP_BODY        _("Reset all the settings to default")
#define BR_STRING_WARNING_VIDEO_PLAYER              _("Can not launch video-player while video-call is running.")
#define BR_STRING_POPUPS_BLOCKED	            _("Pop-up blocked")
#define BR_STRING_SHOW		                    _("Show")
#define BR_STRING_LOW_MEMORY_WARNING                _("Low memory, Can't launch browser. Kill other applications")
#define BR_STRING_WEBPROCESS_CRASH                  _("WebProcess is crashed")
#define BR_STRING_FAILED_TO_GET_WEB_NOTI_ICON       _("Failed to get web notification icon")
#define BR_STRING_BOOKMARK_NOT_READY                _("bookmark is not ready")
#define BR_STRING_NOT_SUPPORT_ADDING_BOOKMARK       _("Not support adding items from bookmark")
#define BR_STRING_NO_VISITED_WEBSITES               _("No visited websites")
#define BR_STRING_STORED_DATA                       _("Stored Data")
#define BR_STRING_STORED_DATA_DESC                  _("This site is using 4KB.")
#define BR_STRING_STORED_DATA_EMPTY_DESC            _("This site is using 0KB.")
#define BR_STRING_LOCATION_DESC                     _("This site can access your location.")
#define BR_STRING_LOCATION_EMPTY_DESC               _("This site can not access your location.")
#define BR_STRING_LOCATION_DEL_MSG                  _("Location permission of this website will be deleted.")
#define BR_STRING_WEBSITES_DEL_MSG                  _("All data stored by these websites and location permissions will be deleted.")
#define BR_STRING_MAXIMUM_CHARACTER_WARNING         _("Maximum number of characters reached.")
#define BR_STRING_BROWSING_HISTORY_IS_DELETED       _("Browsing history is deleted.")
#define BR_STRING_ALL_BROWSING_HISTORY_WILLBE_DELETED _("IDS_BR_POP_ALL_ITEMS_WILL_BE_DELETED_FROM_BROWSING_HISTORY")
#define BR_STRING_SETTINGS_WEB_NOTIFICATIONS        _("Web notifications")
#define BR_STRING_CONTINUE			    _("IDS_BR_BUTTON_CONTINUE")
#define BR_STRING_AHEAD			            _("Ahead")
#define BR_STRING_ENABLE_DATA_ROAMING               _("Data roaming is disabled. Connect to a Wi-Fi network instead, or enable Data roaming and try again.")
#define BR_STRING_FLIGHT_MODE                       _("Unable to connect to mobile networks while Flight mode is enabled Connect to a Wi-Fi network instead, or disable Flight mode and try again.")
#define BR_STRING_MOBILE_DATA_TURNED_OFF            _("Mobile data is turn off. Connect to a Wi-Fi network instead, or enable mobile data and try again.")
/*************************************************************************************************************/

/* New string for new settings */
/*************************************************************************************************************/
#define BR_STRING_SETTINGS_SET_HOMEPAGE                        _("IDS_BR_MBODY_SET_HOMEPAGE")
#define BR_STRING_SETTINGS_DEFAULT_PAGE                        _("IDS_BR_BODY_DEFAULT_PAGE_ABB")
#define BR_STRING_SETTINGS_CURRENT_PAGE                        _("IDS_BR_BODY_CURRENT_PAGE")
#define BR_STRING_SETTINGS_OTHER                               _("IDS_BR_BODY_OTHER_ABB")
#define BR_STRING_SETTINGS_AUTO_FILL_FORMS                     _("IDS_BR_MBODY_AUTO_FILL_FORMS")
#define BR_STRING_SETTINGS_AUTO_FILL_FORMS_DESC                _("IDS_BR_BODY_SET_TEXT_FOR_WEB_FORM_AUTO_FILL")
#define BR_STRING_SETTINGS_ADVANCED                            _("IDS_BR_BODY_ADVANCED")
#define BR_STRING_SETTINGS_PRIVACY                             _("IDS_BR_BODY_PRIVACY")
#define BR_STRING_SETTINGS_SCREEN_AND_TEXT                     _("IDS_BR_BODY_SCREEN_AND_TEXT")
#define BR_STRING_SETTINGS_CONTENT_SETTINGS                    _("IDS_BR_BODY_CONTENT_SETTINGS")
#define BR_STRING_SETTINGS_BANDWIDTH_MANAGEMENT                _("IDS_BR_HEADER_BANDWIDTH_MANAGEMENT_ABB")
#define BR_STRING_SETTINGS_DEVELOPER_MODE                      _("IDS_BR_BODY_DEVELOPER_MODE")
#define BR_STRING_SETTINGS_PRELOAD_AVAILABLE_LINKS_BEFORE_DESC _("IDS_BR_BODY_IMPROVE_PERFORMANCE_WHEN_LOADING_PAGES")
#define BR_STRING_SETTINGS_REMEMBER_FORM_DATA                  _("IDS_BR_BODY_REMEMBER_FORM_DATA")
#define BR_STRING_SETTINGS_REMEMBER_FORM_DATA_DESC 			   _("IDS_BR_BODY_REMEMBER_DATA_I_TYPE_IN_FORMS_FOR_LATER_USE")
#define BR_STRING_SETTINGS_REMEMBER_PASSWORDS                  _("IDS_BR_BODY_REMEMBER_PASSWORDS")
#define BR_STRING_SETTINGS_REMEMBER_PASSWORDS_DESC             _("IDS_BR_BODY_SAVE_USER_NAMES_AND_PASSWORDS_FOR_WEBSITES")
#define BR_STRING_SETTINGS_DELETE_PERSONAL_DATA                _("IDS_BR_BODY_DELETE_PERSONAL_DATA")
#define BR_STRING_SETTINGS_DELETE_PERSONAL_DATA_DESC           _("IDS_BR_HEADER_CLEAR_PERSONALISED_DATA")
#define BR_STRING_SETTINGS_TEXT_SCAILING                       BR_STRING_FONT_SIZE//_("Text scailing")
#define BR_STRING_SETTINGS_FORCE_ZOOM                          _("IDS_BR_BODY_FORCE_ZOOM_ABB")
#define BR_STRING_SETTINGS_ACCEPT_COOKIES                      _("IDS_BR_BODY_ACCEPT_COOKIES")
#define BR_STRING_SETTINGS_ENABLE_LOCATION                     _("IDS_BR_BODY_ENABLE_LOCATION")
#define BR_STRING_SETTINGS_ENABLE_JAVASCRIPT                   _("IDS_BR_BODY_ENABLE_JAVASCRIPT")
#define BR_STRING_SETTINGS_ENABLE_JAVASCRIPT_DESC              _("IDS_BR_BODY_ALLOW_SITES_TO_RUN_JAVASCRIPT")
#define BR_STRING_SETTINGS_BLOCK_POPUPS                        _("IDS_BR_BODY_BLOCK_POP_UPS")
#define BR_STRING_SETTINGS_BLOCK_POPUPS_DESC                   _("IDS_BR_BODY_BLOCK_POP_UPS_ON_WEB_PAGES")
#define BR_STRING_SETTINGS_MEMORY_CARD                         _("IDS_BR_OPT_SD_CARD")
#define BR_STRING_SETTINGS_PHONE                               _("IDS_BR_OPT_DEVICE")
#define BR_STRING_SETTINGS_WEB_NOTIFICATION_TITLE              _("IDS_BR_HEADER_WEB_NOTIFICATION")
#define BR_STRING_SETTINGS_ENABLE_NOTIFICATIONS_ALWAYS         _("IDS_BR_BODY_ALWAYS_ON")
#define BR_STRING_SETTINGS_ENABLE_NOTIFICATIONS_ON_DEMAND      _("IDS_BR_BODY_ON_DEMAND")
#define BR_STRING_SETTINGS_ENABLE_NOTIFICATIONS_OFF            BR_STRING_OFF
#define BR_STRING_SETTINGS_CLEAR_NOTIFICATIONS_DESC            _("IDS_BR_BODY_CLEAR_NOTIFICATION_ACCESS_FOR_WEBSITES")
#define BR_STRING_SETTINGS_RESET_TO_DEFAULT                    _("IDS_BR_BODY_RESET_TO_DEFAULT")
#define BR_STRING_SETTINGS_PRELOAD_WEB_PAGES                   _("IDS_BR_MBODY_PRELOAD_WEBPAGES")
#define BR_STRING_SETTINGS_PRELOAD_WEB_PAGES_ALWAYS            _("IDS_BR_BODY_ALWAYS")
#define BR_STRING_SETTINGS_PRELOAD_WEB_PAGES_ONLY_VIA_WIFI     _("IDS_BR_BODY_ONLY_VIA_WI_FI_ABB")
#define BR_STRING_SETTINGS_PRELOAD_WEB_PAGES_NEVER             BR_STRING_NEVER
#define BR_STRING_SETTINGS_LOAD_IMAGES                         _("IDS_BR_OPT_LOAD_IMAGES")
#define BR_STRING_SETTINGS_LOAD_IMAGES_DESC                    _("IDS_BR_BODY_DISPLAY_IMAGES_ON_WEB_PAGES")
#define BR_STRING_SETTINGS_OPEN_PAGES_IN_OVERVIEW              _("IDS_BR_BODY_OPEN_PAGES_IN_OVERVIEW_ABB")
#define BR_STRING_SETTINGS_OPEN_PAGES_IN_OVERVIEW_DESC         _("IDS_BR_BODY_SHOW_OVERVIEW_OF_NEWLY_OPENED_PAGES")
//#define BR_STRING_SETTINGS_USER_AGENT                          _("IDS_BR_HEADER_USER_AGENT")
#define BR_STRING_SETTINGS_USER_AGENT                          _("User agent")
#define BR_STRING_SETTINGS_CUSTOM_USER_AGENT                   _("IDS_BR_BODY_CUSTOM_USER_AGENT")
#define BR_STRING_SETTINGS_PD_PROFILES_DELETED                 _("IDS_BR_POP_PD_PROFILES_WILL_BE_DELETED")
#define BR_STRING_SETTINGS_RESET_SETTINGS                      _("IDS_BR_BODY_RESET_SETTINGS")
#define BR_STRING_SETTINGS_COOKIES_AND_DATA                    _("IDS_BR_BODY_COOKIES_AND_SITE_DATA_ABB")
#define BR_STRING_SETTINGS_WEB_INSPECTOR                       _("Web inspector")
#define BR_STRING_SETTINGS_CLEAR_NOTIFICATIONS                 _("IDS_BR_BODY_CLEAR_NOTIFICATIONS")
#define BR_STRING_SETTINGS_LOCATION_ACCESS                     _("IDS_BR_OPT_LOCATION_ACCESS_PRIVILEGES")
#define BR_STRING_SETTINGS_WEBSITE_DEL_MSG                     _("IDS_BR_POP_ALL_STORED_AND_LOCATION_PERMISSION_DATA_FOR_THIS_WEBSITE_WILL_BE_CLEARED")
#define BR_STRING_DELETE_PERSONAL_DATA_MESSAGE                 _("IDS_BR_BODY_THE_SELECTED_PERSONAL_DATA_WILL_BE_DELETED")
#define BR_STRING_WEBSITE_SETTINGS_NO_CONTENT_MSG              _("IDS_BR_BODY_AFTER_YOU_VIEW_WEBSITES_THE_SIZE_OF_THEIR_STORED_DATA_AND_WHETHER_THEY_CAN_ACCESS_YOUR_LOCATION_WILL_BE_SHOWN_HERE")
#define BR_STRING_LOCATION_STORED_DATA_DEL_MSG                 _("IDS_BR_POP_ALL_STORED_AND_LOCATION_PERMISSION_DATA_FOR_THIS_WEBSITE_WILL_BE_CLEARED")
#define BR_STRING_STORED_DATA_DEL_MSG                          _("IDS_BR_POP_ALL_STORED_DATA_FOR_THIS_WEBSITE_WILL_BE_CLEARED")
#define BR_STRING_SETTINGS_BLOCK_POPUP_MENU_DESC               _("IDS_BR_BODY_BLOCK_POP_UPS_ON_WEBPAGES")
/*************************************************************************************************************/

/* email, phone number handling */
/*************************************************************************************************************/
#define BR_STRING_SEND_EMAIL     _("IDS_BR_OPT_SEND_EMAIL")
#define BR_STRING_SEND_MESSAGE   _("IDS_BR_OPT_SEND_MESSAGE_VODA")
#define BR_STRING_ADD_TO_CONTACT _("IDS_BR_BODY_ADD_TO_CONTACT")
#define BR_STRING_COPY           _("IDS_BR_OPT_COPY")
/*************************************************************************************************************/

/*TTS string*/
/*************************************************************************************************************/
#define BR_STRING_TEXT_FIELD_T                               _("IDS_BR_BODY_TEXT_FIELD_T_TTS")
#define BR_STRING_DOUBLE_TAP_TO_OPEN_THE_FOLDER_T            _("IDS_BR_BODY_DOUBLE_TAP_TO_OPEN_THE_FOLDER_T_TTS")
#define BR_STRING_ACCESS_PREV_BUT                            _("IDS_BR_SK_PREVIOUS")
#define BR_STRING_ACCESS_NEXT_BUT                            _("IDS_BR_SK_NEXT")
#define BR_STRING_ACCESS_MENU_BUT                            _("IDS_BR_SK_MENU")
#define BR_STRING_DOUBLE_TAP_TO_OPEN_THE_LIST_T		     _("IDS_BR_BODY_DOUBLE_TAP_TO_OPEN_THE_LIST_T_TTS")
#define BR_STRING_DOUBLE_TAP_TO_OPEN_KEYBOARD_T	             _("IDS_BR_BODY_DOUBLE_TAP_TO_OPEN_KEYBOARD_T_TTS")
#define BR_STRING_DOUBLE_TAP_TO_CANCEL_THE_SEARCH_T_TALKBACK _("IDS_BR_BODY_DOUBLE_TAP_TO_CANCEL_THE_SEARCH_T_TALKBACK")
#define BR_STRING_DOUBLE_TAP_TO_CLOSE_THE_LIST_T	     ("DOUBLE TAP TO CLOSE THE LIST")
#define BR_STRING_UPPER_FOLDER_T_TALKBACK	             _("IDS_BR_BODY_UPPER_FOLDER_T_TALKBACK")
#define BR_STRING_ACCESS_DOUBLE_TAP_TO_OPEN_WEBPAGE_T	     _("IDS_BR_BODY_DOUBLE_TAP_TO_OPEN_THE_WEBPAGE_T_TALKBACK")
#define BR_STRING_DOUBLE_TAP_TO_SELECT_A_SEARCH_ENGINE_T     _("IDS_BR_BODY_DOUBLE_TAP_TO_SELECT_A_SEARCH_ENGINE_T_TALKBACK")
#define BR_STRING_BUTTON_T	                             _("IDS_BR_BODY_BUTTON_T_TTS")
#define BR_STRING_SEARCH_FIELD_T                             _("IDS_BR_BODY_SEARCH_FIELD_T_TTS")
#define BR_STRING_WEBPAGE_LOADED_T			     _("IDS_BR_BODY_PAGE_LOADED_T_TTS")
#define BR_STRING_OPEN_WINDOWS_T	                     _("IDS_BR_BODY_WINDOW_MANAGER")
#define BR_STRING_ACCESS_DOUBLE_TAP_AND_DRAG_TO_REORDER	     _("IDS_BR_BODY_DOUBLE_TAP_AND_DRAG_TO_REORDER_TTS")
/*************************************************************************************************************/

/*certificate string*/
/*************************************************************************************************************/
#define BR_STRING_UNABLE_TO_VIEW_THE_CERTIFICATE_THE_PAGE_INFORMATION_HAS_BEEN_CHANGED	_("IDS_BR_POP_UNABLE_TO_VIEW_THE_CERTIFICATE_THE_PAGE_INFORMATION_HAS_BEEN_CHANGED")
#define BR_STRING_CERTIFICATE_SERIAL_NUMBER    _("IDS_BR_HEADER_SERIAL_NUMBER_C_ABB")
#define BR_STRING_ISSUED_ON                    _("IDS_BR_HEADER_ISSUED_ON_C")
#define BR_STRING_EXPIRES_ON_C                 _("IDS_BR_HEADER_EXPIRES_ON_C")
#define BR_STRING_FINGERPRINTS_SHA256          _("IDS_BR_BODY_SHA_256_FINGERPRINT_C")
#define BR_STRING_FINGERPRINTS_SHA1            _("IDS_BR_BODY_SHA_1_FINGERPRINT_C")
#define BR_STRING_ISSUED_BY_C                  _("IDS_BR_HEADER_ISSUED_BY_C")
#define BR_STRING_ORGANIZATION                 _("IDS_BR_HEADER_ORGANISATION_C_ABB")
#define BR_STRING_ORGANIZATION_UNIT            _("IDS_BR_HEADER_DEPARTMENT_C_ABB")
#define BR_STRING_ISSUED_TO_C                  _("IDS_BR_HEADER_ISSUED_TO_C")
#define BR_STRING_VALIDITY_C                   _("IDS_BR_HEADER_VALIDITY_C")
#define BR_STRING_FINGERPRINTS                 _("IDS_BR_BODY_FINGERPRINTS_C")
#define BR_STRING_CERTI_MESSAGE                _("IDS_BR_BODY_SECURITY_CERTIFICATE_PROBLEM_MSG")
#define BR_STRING_VALID_CERTIFICATE            _("IDS_BR_BODY_VALID_CERTIFICATE")
#define BR_STRING_SECURED_PAGE                  ("SECURED PAGE")
#define BR_STRING_UNTRUSTED_PAGE                ("UNTRUSTED PAGE")
#define BR_STRING_DOUBLE_TAP_VIEW_CERTIFICATE  _("IDS_BR_BODY_DOUBLE_TAP_TO_VIEW_CERTIFICATE_TTS")
#define BR_STRING_SECURITY_CERTIFICATE         _("IDS_BR_HEADER_SECURITY_CERTIFICATE")
#define BR_STRING_VIEW_CERTIFICATE             _("IDS_BR_OPT_VIEW_CERTIFICATE")
#define BR_STRING_SECURITY_WARNING_HEADER      _("IDS_BR_HEADER_SITE_NOT_TRUSTED_ABB")
#define BR_STRING_TRUSTED_AUTHORITY            _("IDS_BR_POP_THIS_CERTIFICATE_IS_FROM_A_TRUSTED_AUTHORITY")
#define BR_STRING_UNTRUSTED_AUTHORITY          _("IDS_BR_POP_THIS_CERTIFICATE_IS_NOT_FROM_A_TRUSTED_AUTHORITY")
#define BR_STRING_COMMON_NAME                  _("IDS_BR_HEADER_COMMON_NAME_C_ABB")
/*************************************************************************************************************/

/* browser context menu */
/*************************************************************************************************************/
#define BR_STRING_CTXMENU_OPEN_LINK_IN_NEW_WINDOW       _("IDS_BR_OPT_OPEN_IN_NEW_WINDOW_ABB")
#define BR_STRING_CTXMENU_COPY_LINK                     _("IDS_BR_OPT_COPY_LINK")
#define BR_STRING_CTXMENU_VIEW_IMAGE                    _("IDS_BR_BODY_VIEW_IMAGE")
#define BR_STRING_CTXMENU_COPY_IMAGE                    _("IDS_BR_OPT_COPY_IMAGE")
#define BR_STRING_CTXMENU_SAVE_IMAGE                    _("IDS_BR_OPT_SAVE_IMAGE")
#define BR_STRING_CTXMENU_SEND_EMAIL                    BR_STRING_SEND_EMAIL
#define BR_STRING_CTXMENU_SEND_MESSAGE                  BR_STRING_SEND_MESSAGE
#define BR_STRING_CTXMENU_ADD_TO_CONTACT                BR_STRING_ADD_TO_CONTACT
#define BR_STRING_PRIVATE_OFF                           _("IDS_BR_OPT_DISABLE_SECRET_MODE_ABB")
#define BR_STRING_PRIVATE_ON                            _("IDS_BR_OPT_ENABLE_SECRET_MODE_ABB")
#define BR_STRING_CTXMENU_COPY                          BR_STRING_COPY
#define BR_STRING_CTXMENU_ADD_TO                        _("IDS_BR_BODY_ADD_TO")
#define BR_STRING_CTXMENU_STOP                          _("IDS_BR_OPT_STOP")
#define BR_STRING_CTXMENU_FIND                          _("IDS_BR_OPT_FIND_ON_PAGE")
#define BR_STRING_CTXMENU_SAVE_LINK                     _("IDS_BR_BODY_SAVE_LINK")
#define BR_STRING_CTXMENU_SELECT_ALL                    _("IDS_BR_OPT_SELECT_ALL")
#define BR_STRING_CTXMENU_WEB_SEARCH                    _("IDS_BR_OPT_WEB_SEARCH")
#define BR_STRING_CTXMENU_SELECT_TEXT                   _("IDS_BR_OPT_SELECT_TEXT")
#define BR_STRING_CTXMENU_DRAG                          BR_STRING_DRAG_AND_DROP
#define BR_STRING_CTXMENU_CALL                          BR_STRING_CALL
#define BR_STRING_CTXMENU_GOOGLE                        _("Google")
#define BR_STRING_CTXMENU_YAHOO                         _("Yahoo")
#define BR_STRING_CTXMENU_BING                          _("Bing")
#define BR_STRING_CTXMENU_YANDEX                        _("Yandex")
/*************************************************************************************************************/

#define BR_STR_ACBTN_DELETE                            "IDS_TPLATFORM_ACBUTTON_DELETE_ABB"
#define BR_STR_ACBTN_DONE                              "IDS_TPLATFORM_ACBUTTON_DONE_ABB"
#define BR_STR_ACBTN_CANCEL                            "IDS_TPLATFORM_ACBUTTON_CANCEL_ABB"

/* browser main toolbar */
/*************************************************************************************************************/
#define BR_STRING_MAIN_TOOLBAR_BACK                     BR_STRING_BACK
#define BR_STRING_MAIN_TOOLBAR_FORWARD                  BR_STRING_GOFORWARD
#define BR_STRING_MAIN_TOOLBAR_HOME                     _("IDS_BR_BODY_HOMEPAGE")
#define BR_STRING_MAIN_TOOLBAR_BOOKMARKS                BR_STRING_BOOKMARKS
/*************************************************************************************************************/
#endif /* BROWSER_STRING_H */
