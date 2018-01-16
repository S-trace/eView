// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/**
 * @brief Abstract archive routines for libarchive
 * Copyright (C) 2017 Soul Trace <S-trace@list.ru>
 */

// #define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include "archive_routines.h"

// For standalone testing
// #ifndef VERSION
#define FALSE (0)
#define TRUE !(FALSE)
#define TRACE printf
// #else

// #endif

/**
 * @brief Simple wrapper for strverscmp() to use it with qsort()
 * @param first First pointer to string to compare
 * @param second Second pointer to string to compare
 * @eturn strverscmp() result
 */
static int strverscmp_qsort_wrapper(const void *first, const void *second)
{
	const char **First = (const char **)(size_t) first;
	const char **Second = (const char **)(size_t) second;
	return strverscmp(*First, *Second);
}

/**
 * @brief Open archive 
 * @param arch_out Pointer to struct arch* for opened archive
 * @param archive_name Name of archive file
 * @param buffsize Buffer size for operations
 * @return TRUE if archive was opened, FALSE if not
 * Returned in arch_out pointer should be passed to archive_read_free()
 */
int archive_open(struct archive **arch_out, const char *archive_name,
		 size_t buffsize)
{
	int res;
	struct archive *archive_s = archive_read_new();
	(void)archive_read_support_filter_all(archive_s);
	(void)archive_read_support_format_all(archive_s);

	res = archive_read_open_filename(archive_s, archive_name, buffsize);
	if (res != ARCHIVE_OK) {
		TRACE("Unable to open archive '%s': archive_errno=%d, archive_error_string='%s'\n",
		      archive_name, archive_errno(archive_s), archive_error_string(archive_s));
		return FALSE;
	}
	*arch_out = archive_s;
	return TRUE;
}

/**
 * @brief Check if archive supported by program
 * @param archive_name Archive file name
 * @return TRUE if supported, FALSE if not
 */
int archive_supported(const char *archive_name)
{
	struct archive *archive_s;
	int res = FALSE;
	if (archive_open(&archive_s, archive_name, 16384)) {
		archive_read_free(archive_s);
		res = TRUE;
	}
	return res;
}

/**
 * @brief Get sorted archive content list
 * @param archive Path to archive file (absolute or relative)
 * @param items_type Type of items to list (AE_IFDIR or AE_IFREG)
 * @return newly-allocated NULL-terminated strverscmp-sorted array of newly-allocated strings 
 *         (directories have trailing '/', files - not)
 * All strings from array and then array itself should be passed to free()
 */
char **archive_list_get(const char *archive)
{
	size_t items = 0;
	char **list = NULL;
	size_t buffsize = 16384;
	struct archive *archive_s = NULL;
	struct archive_entry *entry;
	TRACE("%s caled with %s\n", __func__, archive);
	list = calloc(items + 1, sizeof(*list));

	if (!archive_open(&archive_s, archive, buffsize))
		return list;

	do {
		int res;
		const char *pathname;
		char **tmp = NULL;
		int finished = FALSE;

		res = archive_read_next_header(archive_s, &entry);
		switch (res) {
			case ARCHIVE_OK:
				// do nothing - all is fine
				break;
			case ARCHIVE_EOF:
				TRACE("ARCHIVE_EOF (element %d)\n", (int)items);
				finished = TRUE;
				break;
			case ARCHIVE_FATAL:
				TRACE("archive_read_next_header() returned %d (ARCHIVE_FATAL) (element %d), "
				      "archive_errno=%d, archive_error_string='%s'\n",
				      res, (int)items, archive_errno(archive_s), archive_error_string(archive_s));
				finished = TRUE;
				break;
			case ARCHIVE_RETRY:
				TRACE("archive_read_next_header() returned %d (ARCHIVE_RETRY) (element %d), "
				      "archive_errno=%d, archive_error_string='%s'\n",
				      res, (int)items, archive_errno(archive_s), archive_error_string(archive_s));
				continue;
				break;
			case ARCHIVE_WARN:
				TRACE("archive_read_next_header() returned %d (ARCHIVE_RETRY) (element %d), "
				      "archive_errno=%d, archive_error_string='%s' - trying to continue\n",
				      res, (int)items, archive_errno(archive_s), archive_error_string(archive_s));
				break;
			default:
				TRACE("archive_read_next_header() returned %d (unknown code) (element %d), "
				      "archive_errno=%d, archive_error_string='%s' - trying to continue\n",
				      res, (int)items, archive_errno(archive_s), archive_error_string(archive_s));
				break;
		}
		if (finished) {
			break;
		}
		pathname = archive_entry_pathname(entry);
		if (archive_entry_filetype(entry) == AE_IFDIR
		    && pathname[strlen(pathname) - 1] != '/') {
			list[items] =
			    malloc(sizeof(**list) * strlen(pathname) + 2);
			strcpy(list[items], pathname);
			strcat(list[items], "/");
		} else {
			list[items] = strdup(pathname);
		}
		TRACE("Added '%s' (element %d) to archive list\n", list[items], (int)items);
		++items;
		tmp = realloc(list, (items + 1) * sizeof(*list));
		if (tmp == NULL)
			TRACE("Memory exhausted!\n");
		else
			list = tmp;
		list[items] = NULL;	// Keep list NULL-terminated
	} while (TRUE);

	qsort(list, items, sizeof(*list), strverscmp_qsort_wrapper);
	archive_read_free(archive_s);
	return list;
}

/**
 * @brief Free archive list
 * @param archive_list Archive list to free (allocated by archive_get_list or archive_filter_list)
 */
void archive_list_free(char **archive_list)
{
	size_t item = 0;
	if (!archive_list)
		return;
	while (archive_list[item])
		free(archive_list[item++]);
	free(archive_list);
}

/**
 * @brief Check if type of archive list item belongs directory and have specific type
 * @param item Full name of archive item
 * @param directory Parent directory for item
 * @param type Desired item type (AE_IFDIR or AE_IFREG)
 * @return TRUE if item belongs directory and its type matches desired type, FALSE if not
 * Returned list should be passed to archive_list_free() after use
 */
static int archive_list_item_check_path_and_type(const char *item,
						 const char *directory,
						 mode_t type)
{
	int result = FALSE;
	size_t item_len = strlen(item);
	char *tmp = strdup(item);
	if (type == AE_IFDIR &&
	    item[item_len - 1] == '/' && strcmp(dirname(tmp), directory) == 0)
		result = TRUE;
	if (type == AE_IFREG &&
	    item[item_len - 1] != '/' && strcmp(dirname(tmp), directory) == 0)
		result = TRUE;
	free(tmp);
	return result;
}

/**
 * @brief Filter archive list to get files or directories in ceritain archive directory
 * @param list Full archive list
 * @param archive_dir Full archive path to directory for listing
 * @param items_type Desired item type (AE_IFDIR or AE_IFREG)
 * @return Newly-allocated NULL-terminated strverscmp-sorted array of newly-allocated strings
 * (directories have trailing /, files - not)
 * Retruned list should be passed to archive_list_free() after use
 */
char **archive_list_filter(char **full_list, const char *archive_dir,
			   mode_t items_type)
{
	char **list, *archive_dirname;
	size_t item = 0, items = 0;
	list = calloc(items + 1, sizeof(*list));

	if (!archive_dir || !strlen(archive_dir)) {
		archive_dirname = strdup(".");	// Take archive's root if archive_dir was NULL or empty
	} else {
		size_t last_char_pos;
		archive_dirname = strdup(archive_dir);
		last_char_pos = strlen(archive_dirname) - 1;
		if (archive_dirname[last_char_pos] == '/')
			archive_dirname[last_char_pos] = '\0';	// Zap trailing '/' in archive_dir
	}

	while (full_list[item]) {
		if (archive_list_item_check_path_and_type
		    (full_list[item], archive_dirname, items_type)) {
			char **tmp;
			list[items++] = strdup(full_list[item]);
			tmp = realloc(list, (items + 1) * sizeof(*list));
			if (!tmp) {
				TRACE("Memory exhausted!\n");
				return list;
			}
			list = tmp;

			list[items] = NULL;	// Keep list NULL-terminated
		}
		++item;
	}

	free(archive_dirname);
	return list;
}

/**
 * @brief Concatenate path from dir and name, not adding '/' if directory name already have it.
 * @param dir Directory name
 * @param file File name
 * @return Newly allocated string with concatenated full path to file
 * Retruned string should be passed to free() after use
 */
static char *xconcat_path_file(const char *dir, const char *file)
{
	size_t len;
	char *buffer;
	const char *dir_l = dir;
	if (!dir_l)
		dir_l = "";
	len = strlen(dir_l);
	if (len > 1 && dir_l[len - 1] == '/')
		asprintf(&buffer, "%s%s", dir_l, file);
	else
		asprintf(&buffer, "%s%s%s", dir_l, "/", file);
	return buffer;
}

/**
 * @brief Recursive mkdir() call (like bash's mkdir -p do)
 * @param dir Directory name to create
 * @param mode Permissions for created directories
 * @return TRUE - success, FALSE - not (and sets errno to EINVAL)
 * Origin: http://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
 */
static int mkpath(char *dir, mode_t mode)
{
	struct stat stat_buffer;

	if (!dir) {
		errno = EINVAL;
		return FALSE;
	}

	if (!stat(dir, &stat_buffer))
		return TRUE;

	mkpath(dirname(strdupa(dir)), mode);

	return mkdir(dir, mode);
}

/**
 * @brief Extract file to target_directory from archive
 * @param archive Path to archive file (absolute or relative)
 * @param file Full path to file in archive (without leading /)
 * @param target_directory: Path where file will be extracted with archived path
 * @return TRUE if success, FALSE if not
 */
int archive_extract_file(const char *archive, const char *file,
			 const char *target_directory)
{
	int res = FALSE;
	int target_fd;
	ssize_t size;
	size_t buffsize = 16384;
	char buff[buffsize], *target_full_path = NULL, *target_dir =
	    NULL, *td_bak = NULL;
	struct archive *arch;
	struct archive_entry *entry;
	TRACE("%s caled with %s %s %s\n", __func__, archive, file,
	      target_directory);

	if (!archive_open(&arch, archive, buffsize))
		return FALSE;

	do {
		if (archive_read_next_header(arch, &entry) != ARCHIVE_OK) {
			TRACE
			    ("Archive parsing failed or no such file in archive, exiting\n");
			goto out_archive_read;
		}
	} while (strncmp(archive_entry_pathname(entry), file, strlen(file)) !=
		 0);

	target_full_path = xconcat_path_file(target_directory, file);
	target_dir = strdup(target_full_path);
	td_bak = target_dir;
	target_dir = dirname(target_dir);
	mkpath(target_dir, S_IRWXU);
	target_fd =
	    open(target_full_path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if (target_fd < 0) {
		TRACE("can't open target file '%s' (%s)\n", target_full_path,
		      strerror(errno));
		goto out_strings;
	}

	for (;;) {
		size = archive_read_data(arch, buff, buffsize);
		if (size < 0) {
			TRACE("Something was wrong while extracting file!\n");
			goto out_fd;
		}
		if (size == 0) {
			TRACE("File extracted completely\n");
			res = TRUE;
			break;
		}
		write(target_fd, buff, (size_t) size);
	}

out_fd:
	close(target_fd);
out_strings:
	free(target_full_path);
	free(td_bak);
out_archive_read:
	archive_read_free(arch);
	return res;
}

// Standalone testing
/*
int main(int argc, char **argv)
{
	size_t item;
	char **archive_list, **list;
	
	(void)argc;
	(void)argv;
	archive_list=archive_list_get("test.zip");
	list=archive_list_filter(archive_list, "02/03/01", AE_IFDIR);
	
	TRACE("DIRS:\n");
	item = 0;
	while(list[item])
		TRACE("%s\n",list[item++]);
	archive_list_free(list);
	
	list=archive_list_filter(archive_list, "02/03/01", AE_IFREG);
	TRACE("FILES:\n");
	item = 0;
	while(list[item])
		TRACE("%s\n",list[item++]);
	archive_list_free(list);
	
	archive_list_free(archive_list);
	
	archive_extract_file("test.zip", "02/03/01/0001.jpg", "/tmp/");
	return 0;
}
*/
