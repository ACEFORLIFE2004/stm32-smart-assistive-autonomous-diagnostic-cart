/*
 * storage_controller.h
 *
 *  Created on: Mar 31, 2026
 *      Author: arnol
 */

#ifndef INC_STORAGE_CONTROLLER_H_
#define INC_STORAGE_CONTROLLER_H_

#include "project.h"
#include "fatfs.h"

#define MAX_FILES_PER_DIR		15
#define MAX_SUBDIRS_PER_DIR		10
#define MAX_DIR_DEPTH			3

#define MAX_TOTAL_DIRECTORIES	30
#define MAX_TOTAL_FILES			120

#define STORAGE_NAME_LEN        48
#define STORAGE_PATH_LEN		128

typedef enum {
    FILE_TYPE_UNKNOWN,
    FILE_TYPE_WAV,
    FILE_TYPE_BIN,
    FILE_TYPE_TXT
} FileType_t;

typedef struct __attribute__((packed)) {
    char riff[4];
    uint32_t file_size;
    char wave[4];

    char fmt[4];
    uint32_t fmt_size;

    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;

    char data[4];
    uint32_t data_size;

} WAV_Header_t;

typedef enum{
    STORAGE_OK = 0,
    STORAGE_ERROR,
    STORAGE_INVALID_ARG,
	STORAGE_CAPACITY_REACHED,
    STORAGE_FATFS_ERROR
} StorageStatus_t;

typedef struct{
	char				file_name[STORAGE_NAME_LEN];
	char				file_path[STORAGE_PATH_LEN];

	FileType_t			file_type;
	uint32_t			file_size;
}File_t;

typedef struct Directory{
	char				dir_name[STORAGE_NAME_LEN];

	struct Directory	*directory_list;
	uint16_t 			dir_count;

	File_t 				*file_list;
	uint16_t 			file_count;
}Directory_t;

typedef struct{
	char 			root_name[STORAGE_NAME_LEN];

	Directory_t 	*directory_list;
	uint16_t 		dir_count;

	File_t			*file_list;
	uint16_t 		file_count;
}StorageObjects_t;

typedef struct {
    StorageObjects_t root;

    Directory_t dir_pool[MAX_TOTAL_DIRECTORIES];
    File_t      file_pool[MAX_TOTAL_FILES];

    uint16_t    dir_pool_used;
    uint16_t    file_pool_used;
} StorageCatalog_t;

extern StorageCatalog_t g_storage_catalog;

void storage_catalog_init(StorageCatalog_t *catalog);

StorageStatus_t storage_scan_root(const char *storage_path, StorageCatalog_t *catalog);

//static StorageStatus_t storage_build_child_path(char *out_path, uint32_t out_len,
//                                                const char *parent, const char *child);

StorageStatus_t get_storage_directories(const char *storage_path, StorageCatalog_t *catalog);

//static StorageStatus_t storage_alloc_file(StorageCatalog_t *catalog, File_t **file_out);
StorageStatus_t get_storage_files(const char *storage_path, StorageCatalog_t *catalog);
//static FileType_t storage_get_file_type(const char *fname);

StorageStatus_t get_directory_directories(const char *dir_path, Directory_t *dir_obj, StorageCatalog_t *catalog,
																								uint8_t depth);
//static StorageStatus_t storage_alloc_directory(StorageCatalog_t *catalog, Directory_t **dir_out);

StorageStatus_t get_directory_files(const char *dir_path, Directory_t *dir_obj, StorageCatalog_t *catalog);

//static void storage_copy_name(char *dest, const char *src, uint32_t max_len);

//static void storage_print_indent(uint8_t depth);
//static const char *storage_file_type_to_string(FileType_t type);


void storage_print_catalog(const StorageCatalog_t *catalog);
void storage_print_directory(const Directory_t *dir, uint8_t depth);

void test_storage_scan(void);


StorageStatus_t storage_find_file_by_name(const StorageCatalog_t *catalog, const char *file_name, File_t **file_out);

StorageStatus_t storage_find_first_file_by_type(const StorageCatalog_t *catalog, FileType_t type, File_t **file_out);

StorageStatus_t storage_read_file(const File_t *file, uint8_t *buffer, uint32_t buffer_len, uint32_t *bytes_read);

StorageStatus_t storage_read_file_at(const File_t *file, uint32_t offset, uint8_t *buffer, uint32_t buffer_len,
                                     uint32_t *bytes_read);

StorageStatus_t storage_open_file_for_stream(const File_t *file, FIL *fil);
StorageStatus_t storage_seek_stream(FIL *fil, uint32_t offset);
StorageStatus_t storage_read_stream(FIL *fil, uint8_t *buffer, uint32_t buffer_len, uint32_t *bytes_read);
StorageStatus_t storage_close_stream(FIL *fil);

#endif /* INC_STORAGE_CONTROLLER_H_ */
