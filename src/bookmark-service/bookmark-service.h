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


#ifndef BOOKMARK_SERVICE_H_
#define BOOKMARK_SERVICE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* bookmark item */
typedef struct
{	
	char* address;
	char* title;				
	char* creationdate;			
	char* updatedate;
	int id;					/* < uniq id of bookmark */
	int is_folder;				/* property bookmark or folder 0: bookmark 1: folder */
	int folder_id;
	int orderIndex;			/* < id of bookmark */
	int editable;				/* 1 : WRITABLE   0: READ ONLY */
							/* If editable is 0,  Although you try to call update or remove , all calls will be failed with return -1; */
	char *tag1;
	char *tag2;
	char *tag3;
	char *tag4;
} bookmark_entry;

/* bookmark list */
typedef struct
{
	int count;					/* total bookmarks cound */
	bookmark_entry* item;		/* bookmarks array */
} bookmark_list;

/* bookmark tag list */
typedef struct
{
	int count;
	char **tag;
} tag_list;



/* For SyncML & CSC */


/**
* @brief		Inform the unique id of Root Level folder.
* @return		the uniq id of Root Folder
*/
int internet_bookmark_get_root_id( );


/**
* @brief 					Check whether the same unique title is exist or not among the bookmarks in the folder.
* @param[in]	folder_id		The unique id of Folder you want to research, get from bookmark_entry->folder_id or internet_bookmark_get_root_id( )
* @param[in]	title			Title string you want to check the existence
* @param[in]	property_flag	The property of item ( 0 : bookmark, 1 : folder )
* @return		Operation result
* @retval		0>			Found ( The unique id of Bookmark or Folder already stored. )
* @retval		0			Not Found
* @retval		-1			Error
*/
int internet_bookmark_is_exist( int folder_id, char * title, int property_flag );


/**
* @brief 					Check whether the same unique title is exist or not among the bookmarks in the folder.
* @param[in]	folder_id		The unique id of Folder you want to research, get from bookmark_entry->folder_id or internet_bookmark_get_root_id( )
*						If -1, This call will research about all folders.
* @param[in]	property_flag	The property of item ( 0 : bookmark, 1 : folder, 2 : both )
* @return		Operation result
* @retval		0>=			The count of items searched.
* @retval		-1			Error
*/
int internet_bookmark_count( int folder_id, int property_flag  );


/**
* @brief 					Create new bookmark or folder.
* @param[in]	address		URL string  http://a.com
* @param[in]	title			The private name of bookmark
* @param[in]	folder_id		The unique id of folder which bookmark belong to., get from bookmark_entry->folder_id or internet_bookmark_get_root_id( )
* @param[in]	property_flag	The property of item ( 0 : bookmark, 1 : folder )
* @param[in]	overwrite_flag	The property of operation ( 0 : If found the same bookmark, overwrite, 1 : If found the same bookmark, ignore this call )
* @return		Operation result
* @retval		0			Success
* @retval		-1			Error
*/
int internet_bookmark_new( char * address, char * title , int folder_id, int property_flag, int overwrite_flag );


/**
* @brief 					Update the infomation of bookmark or folder.
* @param[in]	uid			The unique id of item you want to update.
* @param[in]	address		URL string  http://a.com
* @param[in]	title			The private name of bookmark
* @param[in]	folder_id		The unique id of folder which bookmark belong to., get from bookmark_entry->folder_id or internet_bookmark_get_root_id( )
* @param[in]	property_flag	The property of item ( 0 : bookmark, 1 : folder )
* @param[in]	create_flag	The property of operation 
*						( 0 : If not exist the bookmark having uid, create bookmark, 1 : If not exist the bookmark having uid, ignore this call )
* @return		Operation result
* @retval		0			Success
* @retval		-1			Error
*/
int internet_bookmark_update( int uid, char * address, char * title , int folder_id,  int property_flag, int create_flag );


/**
* @brief 					Remove the bookmark or folder.
* @param[in]	id			The unique id of item you want to remove.
* @param[in]	remove_flag	The property of operation
*						( 0 : If id means folder, remove all items having the folder.
*						  1 : If id means folder, only remove folder, and move all items having the folder to root-level folder. )
* @return		Operation result
* @retval		0			Success
* @retval		-1			Error
*/
int internet_bookmark_remove( int id , int remove_flag );


/**
* @brief 						Get the information of bookmarks and folders via the special structure.
* @param[in]	folder_id			The unique id of Folder you want to research, get from bookmark_entry->folder_id or internet_bookmark_get_root_id( )
*							( -1 : search from all folder )
* 							( recommand : 1. get folder list. 2. get bookmark list using folder id. )
* 							( if call with -1, you should arrange and construct the bookmark tree structure by yourself. )
* @param[in]	property_flag		The property of item ( 0 : bookmark, 1 : folder, 2 : both )
* @return		Operation result
* @retval		The pointer of Bookmarks Array	Success
* @retval		NULL						Error
*/
bookmark_list * internet_bookmark_list( int folder_id, int property_flag );


/**
* @brief					Create new bookmark from vbookmark file. not support yet. 
* @param[in]	vbmfile		The absolute path of vbookmark file.
* @param[in]	folder_id		The unique id of folder which bookmark belong to., get from bookmark_entry->folder_id or internet_bookmark_get_root_id( )
* @param[in]	overwrite_flag	The property of operation ( 0 : If found the same bookmark, overwrite, 1 : If found the same bookmark, ignore this call )
* @return		Operation result
* @retval		0			Success
* @retval		-1			Error
*/
int internet_bookmark_new_by_vbm( char * vbmfile, int folder_id, int overwrite_flag );


/**
* @brief		Clear all items beside the read only items.
* @return		Operation result
* @retval		0			Success
* @retval		-1			Error
*/
int internet_bookmark_reset( );


/**
* @brief				Free the complex memory getted from internet_bookmark_list().
* @param[in]	m_list	The pointer of the structure you want to free.
*/
void internet_bookmark_free( bookmark_list * m_list);





/* For KIES backup & restore	*/
/*#define INTERNET_BOOKMARK_DB_NAME	"/opt/usr/dbspace/.internet_bookmark.db"*/
/*#define INTERNET_BOOKMARK_BACKUP_FILE_NAME	"/opt/usr/dbspace/internet_bookmark_backup"*/


/**
* @brief		Save all bookmark and folder to a specail file.
*			After use this API, free return string with checking NULL, and caller should remove a backup file.
* @return		Operation result
* @retval		The absolute path of backup file	Success
* @retval		NULL						Error
*/
char* internet_bookmark_backup( );


/**
* @brief		Renew all bookmark and folder from backup file.
* @param[in]	restore_path		The absolute path of backup file you want to restore.
* @return		Operation result
* @retval		0			Success
* @retval		-1			Error
*/
int internet_bookmark_restore( char * restore_path );

void internet_bookmark_entry_free(bookmark_entry *entry);
bookmark_entry *internet_bookmark_get_bookmark_by_id(int id);

/* APIs for browser */
bookmark_entry * bmsvc_get_bookmark_by_id(int id);
void bmsvc_free_bookmark_entry(bookmark_entry *entry);
//tag_list *bmsvc_get_tag_list();
int bmsvc_get_tag_count();
void bmsvc_free_tag_list(tag_list *list);
bookmark_list *bmsvc_get_bookmark_list_by_tag(const char *tag);
int bmsvc_get_bookmark_count_by_tag(const char *tag);

/**
 * @brief Enumerations for bmsvc error.
 */
typedef enum bmsvc_error{
	BMSVC_ERROR_NONE		= 0,			/**< Successful */
	BMSVC_ERROR_INVALID_PARAMETER,	/**< Invalid parameter */
	BMSVC_ERROR_DB_FAILED,			/**< Database operation failure */
	BMSVC_ERROR_ITEM_ALREADY_EXIST,	/**< Requested data already exists */
	BMSVC_ERROR_ITEM_IS_NOT_EDITABLE,/**< Requested data is not editable */
	BMSVC_ERROR_UNKNOWN
} bmsvc_error_e;

int bmsvc_add_bookmark(const char *title, const char *address, int parent_id,
						const char *tag1,
						const char *tag2,
						const char *tag3,
						const char *tag4,
						int *saved_bookmark_id);
int bmsvc_add_folder(const char *title, int parent_id, int editable, int *saved_folder_id);

int bmsvc_get_max_tag_count_for_a_bookmark_entry(int *max_count);
int bmsvc_get_bookmark_id(const char *address, int *bookmark_id);
int bmsvc_get_folder_id(const char *title, int parent_id, int *folder_id);
int bmsvc_delete_bookmark(int id, int check_editable);
int bmsvc_update_bookmark(int id, const char *title, const char *address, int parent_id, int order,
						const char *tag1,
						const char *tag2,
						const char *tag3,
						const char *tag4,
						int check_editable);
int bmsvc_set_thumbnail(int id, void *image_data, int w, int h, int len);
int bmsvc_get_thumbnail(int id, void **image_data, int *w, int *h, int *len);
int bmsvc_set_favicon(int id, void *image_data, int w, int h, int len);
int bmsvc_get_favicon(int id, void **image_data, int *w, int *h, int *len);

#ifdef __cplusplus
};
#endif

#endif






/*  The Examples , How to use the bookmark APIs */

/*

#include <stdio.h>
#include <bookmark-service.h>

void _usage()
{
	printf("\n");
	printf("\tbookmark-service-test [new|remove|list|backup|restore]\n");
	printf("\tCommands\n");
	printf("\t\tlist\n");
	printf("\t\tremove uid flag\n");
	printf("\t\tnew folderid[0] type[0:bookmark/1:folder] flag[0:overwrite/1:ignore] title address\n");
	printf("\t\treset\n");
	printf("\t\tbackup\n");
	printf("\t\trestore Apsolute File Path\n");
	printf("\n");
}

int main( int argc, char *argv[])
{
	if( argc < 2 )
	{
		_usage();
		return -1;
	}
	if( memcmp( argv[1], "list", 4 ) == 0 )
	{
		bookmark_list* m_list = internet_bookmark_list( internet_bookmark_get_root_id(),  2 );
		if( m_list == NULL )	
		{
			printf("fail to get bookmark list\n");
			return -1;
		}
		int i =0;
		for( i =0; i < m_list->count; i++ )
		{
			if( m_list->item[i].is_folder == 1 )
			{
				printf("Folder   Uid[%d] Editable[%d] Folderid[%d] Order[%d] Title[%s] Cdate[%s] Udate[%s]\n",  m_list->item[i].id, m_list->item[i].editable,  m_list->item[i].folder_id,  m_list->item[i].orderIndex,  m_list->item[i].title, m_list->item[i].creationdate,m_list->item[i].updatedate);
				bookmark_list* m_sub_list = internet_bookmark_list( m_list->item[i].id,  2 );
				if( m_sub_list == NULL )	
				{
					continue;
				}
				int j =0;
				for( j=0; j < m_sub_list->count; j++ )
				{
					if( m_sub_list->item[j].is_folder == 1 )
					{
						printf("-- Folder   Uid[%d] Editable[%d] Folderid[%d] Order[%d] Title[%s] Cdate[%s] Udate[%s]\n",  m_sub_list->item[j].id, m_sub_list->item[j].editable,  m_sub_list->item[j].folder_id,  m_sub_list->item[j].orderIndex,  m_sub_list->item[j].title, m_sub_list->item[j].creationdate,m_sub_list->item[j].updatedate);
					}
					else
					{
						printf("-- Bookmark Uid[%d] Editable[%d] Folderid[%d] Order[%d] Title[%s] Address[%s] Cdate[%s] Udate[%s]\n",  m_sub_list->item[j].id, m_list->item[j].editable,  m_sub_list->item[j].folder_id,  m_sub_list->item[j].orderIndex,  m_sub_list->item[j].title,  m_sub_list->item[j].address, m_sub_list->item[j].creationdate,m_sub_list->item[j].updatedate);			
					}
				}
				internet_bookmark_free(m_sub_list );
				continue;
			}
			printf("Bookmark Uid[%d] Editable[%d] Folderid[%d] Order[%d] Title[%s] Address[%s] Cdate[%s] Udate[%s]\n",  m_list->item[i].id,  m_list->item[i].editable, m_list->item[i].folder_id,  m_list->item[i].orderIndex,  m_list->item[i].title,  m_list->item[i].address, m_list->item[i].creationdate,m_list->item[i].updatedate);

		}
		internet_bookmark_free(m_list );
	}
	else if( memcmp( argv[1], "new", 3 ) == 0 )
	{
		if( argc < 6 )
		{
			_usage();
			return -1;
		}
		int folder_id = 0;
		folder_id = atoi( argv[2]);
		if( folder_id < internet_bookmark_get_root_id() ) folder_id = internet_bookmark_get_root_id();

		int type =0;	// means bookmark
		type = atoi( argv[3]);
		if( type != 1 )	type = 0;
		
		int overwrite_flag = 0;	// means overwrite
		overwrite_flag = atoi( argv[4]);
		if( type == 1 )	internet_bookmark_new( NULL, argv[5], folder_id, type, overwrite_flag );
		else 			internet_bookmark_new( argv[6], argv[5], folder_id, type, overwrite_flag );
	}
	else if( memcmp( argv[1], "remove", 6 ) == 0 )
	{
		if( argc < 3 )
		{
			_usage();
			return -1;
		}
		int remove_flag = 0;
		if( argc == 4 ) remove_flag = atoi( argv[3]);
		internet_bookmark_remove( atoi( argv[2]), remove_flag);
	}
	else if( memcmp( argv[1], "reset", 5 ) == 0 )
	{
		return  internet_bookmark_reset( );
	}
	else if( memcmp( argv[1], "backup", 6 ) == 0 )
	{
		char* backupfile = internet_bookmark_backup( );
		if( backupfile != NULL )
		{
			printf("Stored to [%s]\n",backupfile);
			free( backupfile );
		}
	}
	else if( memcmp( argv[1], "restore", 7 ) == 0 )
	{
		if( argc < 3 )
		{
			_usage();
			return -1;
		}
		printf("restore file path : %s\n", argv[2] );
		internet_bookmark_restore( argv[2] );
	}

	return 0;
}



*/
