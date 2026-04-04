/*
 * storage_controller.c
 *
 *  Created on: Mar 31, 2026
 *      Author: arnol
 */

#include "storage_controller.h"

StorageCatalog_t g_storage_catalog;

static StorageStatus_t storage_build_child_path(char *out_path, uint32_t out_len, const char *parent,
																					const char *child);

static StorageStatus_t storage_alloc_file(StorageCatalog_t *catalog, File_t **file_out);

static FileType_t storage_get_file_type(const char *fname);

static StorageStatus_t storage_alloc_directory(StorageCatalog_t *catalog, Directory_t **dir_out);

static void storage_copy_name(char *dest, const char *src, uint32_t max_len);

static void storage_print_indent(uint8_t depth);
static const char *storage_file_type_to_string(FileType_t type);

void test_storage_scan(void)
{
    StorageStatus_t status;

    status = storage_scan_root("0:", &g_storage_catalog);
    if (status != STORAGE_OK) {
        print_msg("Storage scan failed: %u\r\n", status);
        return;
    }

    print_msg("Root: %s\r\n", g_storage_catalog.root.root_name);
    print_msg("Root files: %u\r\n", g_storage_catalog.root.file_count);
    print_msg("Root dirs : %u\r\n", g_storage_catalog.root.dir_count);

    for (uint16_t i = 0; i < g_storage_catalog.root.file_count; i++) {
        print_msg("  FILE: %s size=%lu type=%u\r\n",
                  g_storage_catalog.root.file_list[i].file_name,
                  g_storage_catalog.root.file_list[i].file_size,
                  g_storage_catalog.root.file_list[i].file_type);
    }

    for (uint16_t i = 0; i < g_storage_catalog.root.dir_count; i++) {
        Directory_t *dir = &g_storage_catalog.root.directory_list[i];

        print_msg("  DIR: %s\r\n", dir->dir_name);
        print_msg("    files=%u subdirs=%u\r\n", dir->file_count, dir->dir_count);
    }
}

/* --------------------------- Internal Helpers --------------------------- */

static void storage_copy_name(char *dest, const char *src, uint32_t max_len)
{
    if ((dest == NULL) || (src == NULL) || (max_len == 0U)) {
        return;
    }

    strncpy(dest, src, max_len - 1U);
    dest[max_len - 1U] = '\0';
}

static FileType_t storage_get_file_type(const char *fname)
{
    const char *ext;

    if (fname == NULL) {
        return FILE_TYPE_UNKNOWN;
    }

    ext = strrchr(fname, '.');
    if (ext == NULL) {
        return FILE_TYPE_UNKNOWN;
    }

    if (strcasecmp(ext, ".wav") == 0) {
        return FILE_TYPE_WAV;
    }
    if (strcasecmp(ext, ".bin") == 0) {
        return FILE_TYPE_BIN;
    }
    if (strcasecmp(ext, ".txt") == 0) {
        return FILE_TYPE_TXT;
    }

    return FILE_TYPE_UNKNOWN;
}

static StorageStatus_t storage_alloc_directory(StorageCatalog_t *catalog, Directory_t **dir_out)
{
    if ((catalog == NULL) || (dir_out == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    if (catalog->dir_pool_used >= MAX_TOTAL_DIRECTORIES) {
        *dir_out = NULL;
        return STORAGE_CAPACITY_REACHED;
    }

    *dir_out = &catalog->dir_pool[catalog->dir_pool_used];
    memset(*dir_out, 0, sizeof(Directory_t));
    catalog->dir_pool_used++;

    return STORAGE_OK;
}

static StorageStatus_t storage_alloc_file(StorageCatalog_t *catalog, File_t **file_out)
{
    if ((catalog == NULL) || (file_out == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    if (catalog->file_pool_used >= MAX_TOTAL_FILES) {
        *file_out = NULL;
        return STORAGE_CAPACITY_REACHED;
    }

    *file_out = &catalog->file_pool[catalog->file_pool_used];
    memset(*file_out, 0, sizeof(File_t));
    catalog->file_pool_used++;

    return STORAGE_OK;
}

static StorageStatus_t storage_build_child_path(char *out_path, uint32_t out_len,
                                                const char *parent, const char *child)
{
    int written;

    if ((out_path == NULL) || (parent == NULL) || (child == NULL) || (out_len == 0U)) {
        return STORAGE_INVALID_ARG;
    }

    /* Handle root paths like "0:" or "0:/" cleanly */
    if ((parent[strlen(parent) - 1U] == '/') || (parent[strlen(parent) - 1U] == '\\')) {
        written = snprintf(out_path, out_len, "%s%s", parent, child);
    } else {
        written = snprintf(out_path, out_len, "%s/%s", parent, child);
    }

    if ((written < 0) || ((uint32_t)written >= out_len)) {
        return STORAGE_ERROR;
    }

    return STORAGE_OK;
}

/* --------------------------- Public Init --------------------------- */

void storage_catalog_init(StorageCatalog_t *catalog)
{
    if (catalog == NULL) {
        return;
    }

    memset(catalog, 0, sizeof(StorageCatalog_t));
}

/* --------------------------- Root Scanning --------------------------- */

StorageStatus_t get_storage_files(const char *storage_path, StorageCatalog_t *catalog){
    FRESULT fres;
    DIR dir;
    FILINFO fno;
    File_t *file_obj;
    StorageStatus_t status;

    if ((storage_path == NULL) || (catalog == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    /* Root file list starts at current free file pool position */
    catalog->root.file_list = &catalog->file_pool[catalog->file_pool_used];
    catalog->root.file_count = 0U;

    fres = f_opendir(&dir, storage_path);
    if (fres != FR_OK) {
        return STORAGE_FATFS_ERROR;
    }

    for (;;) {
        fres = f_readdir(&dir, &fno);
        if (fres != FR_OK) {
            f_closedir(&dir);
            return STORAGE_FATFS_ERROR;
        }

        if (fno.fname[0] == '\0') {
            break;
        }

        if (fno.fattrib & AM_DIR) {
            continue;
        }

        if (catalog->root.file_count >= MAX_FILES_PER_DIR) {
            break;
        }

        status = storage_alloc_file(catalog, &file_obj);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }

        storage_copy_name(file_obj->file_name, fno.fname, STORAGE_NAME_LEN);

        // Populate the file_path for root files
        status = storage_build_child_path(file_obj->file_path,
                                          STORAGE_PATH_LEN,
                                          storage_path,
                                          fno.fname);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }

        file_obj->file_type = storage_get_file_type(fno.fname);
        file_obj->file_size = fno.fsize;

        catalog->root.file_count++;
    }

    f_closedir(&dir);
    return STORAGE_OK;
}

StorageStatus_t get_directory_files(const char *dir_path, Directory_t *dir_obj, StorageCatalog_t *catalog){
    FRESULT fres;
    DIR dir;
    FILINFO fno;
    File_t *file_obj;
    StorageStatus_t status;

    if ((dir_path == NULL) || (dir_obj == NULL) || (catalog == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    dir_obj->file_list = &catalog->file_pool[catalog->file_pool_used];
    dir_obj->file_count = 0U;

    fres = f_opendir(&dir, dir_path);
    if (fres != FR_OK) {
        return STORAGE_FATFS_ERROR;
    }

    for (;;) {
        fres = f_readdir(&dir, &fno);
        if (fres != FR_OK) {
            f_closedir(&dir);
            return STORAGE_FATFS_ERROR;
        }

        if (fno.fname[0] == '\0') {
            break;
        }

        if (fno.fattrib & AM_DIR) {
            continue;
        }

        if (dir_obj->file_count >= MAX_FILES_PER_DIR) {
            break;
        }

        status = storage_alloc_file(catalog, &file_obj);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }

        storage_copy_name(file_obj->file_name, fno.fname, STORAGE_NAME_LEN);

        status = storage_build_child_path(file_obj->file_path,
                                          STORAGE_PATH_LEN,
                                          dir_path,
                                          fno.fname);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }

        file_obj->file_type = storage_get_file_type(fno.fname);
        file_obj->file_size = fno.fsize;

        dir_obj->file_count++;
    }

    f_closedir(&dir);
    return STORAGE_OK;
}

StorageStatus_t get_directory_directories(const char *dir_path, Directory_t *dir_obj,
                                          StorageCatalog_t *catalog, uint8_t depth)
{
    FRESULT fres;
    DIR dir;
    FILINFO fno;
    Directory_t *child_dir;
    StorageStatus_t status;
    char child_path[128];

    if ((dir_path == NULL) || (dir_obj == NULL) || (catalog == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    if (depth >= MAX_DIR_DEPTH) {
        dir_obj->directory_list = NULL;
        dir_obj->dir_count = 0U;
        return STORAGE_OK;
    }

    dir_obj->directory_list = &catalog->dir_pool[catalog->dir_pool_used];
    dir_obj->dir_count = 0U;

    fres = f_opendir(&dir, dir_path);
    if (fres != FR_OK) {
        return STORAGE_FATFS_ERROR;
    }

    for (;;) {
        fres = f_readdir(&dir, &fno);
        if (fres != FR_OK) {
            f_closedir(&dir);
            return STORAGE_FATFS_ERROR;
        }

        if (fno.fname[0] == '\0') {
            break;
        }

        if (!(fno.fattrib & AM_DIR)) {
            continue;
        }

        /* Skip "." and ".." just in case */
        if ((strcmp(fno.fname, ".") == 0) || (strcmp(fno.fname, "..") == 0)) {
            continue;
        }

        if (dir_obj->dir_count >= MAX_SUBDIRS_PER_DIR) {
            break;
        }

        status = storage_alloc_directory(catalog, &child_dir);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }

        storage_copy_name(child_dir->dir_name, fno.fname, STORAGE_NAME_LEN);
        child_dir->directory_list = NULL;
        child_dir->dir_count = 0U;
        child_dir->file_list = NULL;
        child_dir->file_count = 0U;

        dir_obj->dir_count++;

        status = storage_build_child_path(child_path, sizeof(child_path), dir_path, fno.fname);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }

        /* Fill this child directory's files */
        status = get_directory_files(child_path, child_dir, catalog);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }

        /* Recurse into this child directory's subdirectories */
        status = get_directory_directories(child_path, child_dir, catalog, depth + 1U);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }
    }

    f_closedir(&dir);
    return STORAGE_OK;
}

StorageStatus_t get_storage_directories(const char *storage_path, StorageCatalog_t *catalog)
{
    FRESULT fres;
    DIR dir;
    FILINFO fno;
    Directory_t *dir_obj;
    StorageStatus_t status;
    char child_path[128];

    if ((storage_path == NULL) || (catalog == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    catalog->root.directory_list = &catalog->dir_pool[catalog->dir_pool_used];
    catalog->root.dir_count = 0U;

    fres = f_opendir(&dir, storage_path);
    if (fres != FR_OK) {
        return STORAGE_FATFS_ERROR;
    }

    for (;;) {
        fres = f_readdir(&dir, &fno);
        if (fres != FR_OK) {
            f_closedir(&dir);
            return STORAGE_FATFS_ERROR;
        }

        if (fno.fname[0] == '\0') {
            break;
        }

        if (!(fno.fattrib & AM_DIR)) {
            continue;
        }

        if ((strcmp(fno.fname, ".") == 0) || (strcmp(fno.fname, "..") == 0)) {
            continue;
        }

        if (catalog->root.dir_count >= MAX_SUBDIRS_PER_DIR) {
            break;
        }

        status = storage_alloc_directory(catalog, &dir_obj);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }

        storage_copy_name(dir_obj->dir_name, fno.fname, STORAGE_NAME_LEN);
        dir_obj->directory_list = NULL;
        dir_obj->dir_count = 0U;
        dir_obj->file_list = NULL;
        dir_obj->file_count = 0U;

        catalog->root.dir_count++;

        status = storage_build_child_path(child_path, sizeof(child_path), storage_path, fno.fname);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }

        status = get_directory_files(child_path, dir_obj, catalog);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }

        status = get_directory_directories(child_path, dir_obj, catalog, 1U);
        if (status != STORAGE_OK) {
            f_closedir(&dir);
            return status;
        }
    }

    f_closedir(&dir);
    return STORAGE_OK;
}

StorageStatus_t storage_scan_root(const char *storage_path, StorageCatalog_t *catalog)
{
    StorageStatus_t status;

    if ((storage_path == NULL) || (catalog == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    storage_catalog_init(catalog);

    storage_copy_name(catalog->root.root_name, storage_path, STORAGE_NAME_LEN);

    status = get_storage_files(storage_path, catalog);
    if (status != STORAGE_OK) {
        return status;
    }

    status = get_storage_directories(storage_path, catalog);
    if (status != STORAGE_OK) {
        return status;
    }

    return STORAGE_OK;
}

static void storage_print_indent(uint8_t depth)
{
    for (uint8_t i = 0; i < depth; i++) {
        print_msg("  ");
    }
}

static const char *storage_file_type_to_string(FileType_t type)
{
    switch (type) {
        case FILE_TYPE_WAV: return "WAV";
        case FILE_TYPE_BIN: return "BIN";
        case FILE_TYPE_TXT: return "TXT";
        case FILE_TYPE_UNKNOWN:
        default:
            return "UNKNOWN";
    }
}

void storage_print_directory(const Directory_t *dir, uint8_t depth)
{
    if (dir == NULL) {
        return;
    }

    storage_print_indent(depth);
    print_msg("[DIR] %s  (files=%u, subdirs=%u)\r\n",
              dir->dir_name,
              dir->file_count,
              dir->dir_count);

    /* Print files in this directory */
    for (uint16_t i = 0; i < dir->file_count; i++) {
        const File_t *file = &dir->file_list[i];

        storage_print_indent(depth + 1U);
        print_msg("- %s  [type=%s, size=%lu]\r\n",
                  file->file_name,
                  storage_file_type_to_string(file->file_type),
                  file->file_size);
    }

    /* Recurse into subdirectories */
    for (uint16_t i = 0; i < dir->dir_count; i++) {
        const Directory_t *child = &dir->directory_list[i];
        storage_print_directory(child, depth + 1U);
    }
}

void storage_print_catalog(const StorageCatalog_t *catalog)
{
    if (catalog == NULL) {
        print_msg("Storage catalog is NULL.\r\n");
        return;
    }

    print_msg("\r\n========== STORAGE CATALOG ==========\r\n");
    print_msg("Root: %s\r\n", catalog->root.root_name);
    print_msg("Root files: %u\r\n", catalog->root.file_count);
    print_msg("Root dirs : %u\r\n", catalog->root.dir_count);
    print_msg("Dir pool used : %u / %u\r\n", catalog->dir_pool_used, MAX_TOTAL_DIRECTORIES);
    print_msg("File pool used: %u / %u\r\n", catalog->file_pool_used, MAX_TOTAL_FILES);
    print_msg("-------------------------------------\r\n");

    /* Print root-level files */
    if (catalog->root.file_count > 0U) {
        print_msg("[ROOT FILES]\r\n");
        for (uint16_t i = 0; i < catalog->root.file_count; i++) {
            const File_t *file = &catalog->root.file_list[i];

            print_msg("  - %s  [type=%s, size=%lu]\r\n",
                      file->file_name,
                      storage_file_type_to_string(file->file_type),
                      file->file_size);
        }
    } else {
        print_msg("[ROOT FILES] none\r\n");
    }

    print_msg("-------------------------------------\r\n");

    /* Print root-level directories recursively */
    if (catalog->root.dir_count > 0U) {
        print_msg("[ROOT DIRECTORIES]\r\n");
        for (uint16_t i = 0; i < catalog->root.dir_count; i++) {
            const Directory_t *dir = &catalog->root.directory_list[i];
            storage_print_directory(dir, 1U);
        }
    } else {
        print_msg("[ROOT DIRECTORIES] none\r\n");
    }

    print_msg("=====================================\r\n\r\n");
}

static StorageStatus_t storage_find_file_by_name_in_dir(const Directory_t *dir, const char *file_name,
														File_t **file_out)
{
	if ((dir == NULL) || (file_name == NULL) || (file_out == NULL)) {
		return STORAGE_INVALID_ARG;
	}

	for (uint16_t i = 0; i < dir->file_count; i++) {
		if (strcmp(dir->file_list[i].file_name, file_name) == 0) {
			*file_out = &dir->file_list[i];
			return STORAGE_OK;
		}
	}

	for (uint16_t i = 0; i < dir->dir_count; i++) {
		StorageStatus_t status =
			storage_find_file_by_name_in_dir(&dir->directory_list[i], file_name, file_out);

		if (status == STORAGE_OK) {
			return STORAGE_OK;
		}
	}

	return STORAGE_ERROR;
}

StorageStatus_t storage_find_file_by_name(const StorageCatalog_t *catalog, const char *file_name,
										  File_t **file_out)
{
	if ((catalog == NULL) || (file_name == NULL) || (file_out == NULL)) {
		return STORAGE_INVALID_ARG;
	}

	*file_out = NULL;

	for (uint16_t i = 0; i < catalog->root.file_count; i++) {
		if (strcmp(catalog->root.file_list[i].file_name, file_name) == 0) {
			*file_out = &catalog->root.file_list[i];
			return STORAGE_OK;
		}
	}

	for (uint16_t i = 0; i < catalog->root.dir_count; i++) {
		StorageStatus_t status =
			storage_find_file_by_name_in_dir(&catalog->root.directory_list[i], file_name, file_out);

		if (status == STORAGE_OK) {
			return STORAGE_OK;
		}
	}

	return STORAGE_ERROR;
}

static StorageStatus_t storage_find_first_file_by_type_in_dir(const Directory_t *dir, FileType_t type,
															  File_t **file_out)
{
	if ((dir == NULL) || (file_out == NULL)) {
		return STORAGE_INVALID_ARG;
	}

	for (uint16_t i = 0; i < dir->file_count; i++) {
		if (dir->file_list[i].file_type == type) {
			*file_out = &dir->file_list[i];
			return STORAGE_OK;
		}
	}

	for (uint16_t i = 0; i < dir->dir_count; i++) {
		StorageStatus_t status =
			storage_find_first_file_by_type_in_dir(&dir->directory_list[i], type, file_out);

		if (status == STORAGE_OK) {
			return STORAGE_OK;
		}
	}

	return STORAGE_ERROR;
}

StorageStatus_t storage_find_first_file_by_type(const StorageCatalog_t *catalog, FileType_t type,
												File_t **file_out)
{
	if ((catalog == NULL) || (file_out == NULL)) {
		return STORAGE_INVALID_ARG;
	}

	*file_out = NULL;

	for (uint16_t i = 0; i < catalog->root.file_count; i++) {
		if (catalog->root.file_list[i].file_type == type) {
			*file_out = &catalog->root.file_list[i];
			return STORAGE_OK;
		}
	}

	for (uint16_t i = 0; i < catalog->root.dir_count; i++) {
		StorageStatus_t status =
			storage_find_first_file_by_type_in_dir(&catalog->root.directory_list[i], type, file_out);

		if (status == STORAGE_OK) {
			return STORAGE_OK;
		}
	}

	return STORAGE_ERROR;
}

StorageStatus_t storage_read_file(const File_t *file, uint8_t *buffer, uint32_t buffer_len, uint32_t *bytes_read)
{
	FIL fil;
	FRESULT fres;
	UINT br = 0;

	if ((file == NULL) || (buffer == NULL) || (bytes_read == NULL)) {
		return STORAGE_INVALID_ARG;
	}

	*bytes_read = 0;

	fres = f_open(&fil, file->file_path, FA_READ);
	if (fres != FR_OK) {
		print_msg("  f_open: Failed to open file\r\n");
		return STORAGE_FATFS_ERROR;
	}

	fres = f_read(&fil, buffer, buffer_len, &br);
	f_close(&fil);

	if (fres != FR_OK) {
		print_msg("  f_read: Failed to read file\r\n");
		return STORAGE_FATFS_ERROR;
	}

	*bytes_read = (uint32_t)br;
	return STORAGE_OK;
}

StorageStatus_t storage_read_file_at(const File_t *file, uint32_t offset, uint8_t *buffer, uint32_t buffer_len,
                                     uint32_t *bytes_read)
{
    FIL fil;
    FRESULT fres;
    UINT br = 0;

    if ((file == NULL) || (buffer == NULL) || (bytes_read == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    *bytes_read = 0U;

    fres = f_open(&fil, file->file_path, FA_READ);
    if (fres != FR_OK) {
        return STORAGE_FATFS_ERROR;
    }

    fres = f_lseek(&fil, offset);
    if (fres != FR_OK) {
        f_close(&fil);
        return STORAGE_FATFS_ERROR;
    }

    fres = f_read(&fil, buffer, buffer_len, &br);
    f_close(&fil);

    if (fres != FR_OK) {
        return STORAGE_FATFS_ERROR;
    }

    *bytes_read = (uint32_t)br;
    return STORAGE_OK;
}

StorageStatus_t storage_open_file_for_stream(const File_t *file, FIL *fil)
{
    FRESULT fres;

    if ((file == NULL) || (fil == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    fres = f_open(fil, file->file_path, FA_READ);
    if (fres != FR_OK) {
        return STORAGE_FATFS_ERROR;
    }

    return STORAGE_OK;
}

StorageStatus_t storage_seek_stream(FIL *fil, uint32_t offset)
{
    FRESULT fres;

    if (fil == NULL) {
        return STORAGE_INVALID_ARG;
    }

    fres = f_lseek(fil, offset);
    if (fres != FR_OK) {
        return STORAGE_FATFS_ERROR;
    }

    return STORAGE_OK;
}

StorageStatus_t storage_read_stream(FIL *fil, uint8_t *buffer, uint32_t buffer_len, uint32_t *bytes_read)
{
    FRESULT fres;
    UINT br = 0U;

    if ((fil == NULL) || (buffer == NULL) || (bytes_read == NULL)) {
        return STORAGE_INVALID_ARG;
    }

    *bytes_read = 0U;

    fres = f_read(fil, buffer, buffer_len, &br);
    if (fres != FR_OK) {
        return STORAGE_FATFS_ERROR;
    }

    *bytes_read = (uint32_t)br;
    return STORAGE_OK;
}

StorageStatus_t storage_close_stream(FIL *fil)
{
    if (fil == NULL) {
        return STORAGE_INVALID_ARG;
    }

    (void)f_close(fil);
    return STORAGE_OK;
}
