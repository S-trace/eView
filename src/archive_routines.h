#ifndef ARCHIVE_ROUTINES_H
#define ARCHIVE_ROUTINES_H
#include <archive.h>
#include <archive_entry.h>

/**
 * @brief Check if archive supported by program
 * @param archive_name Archive file name
 * @return TRUE if supported, FALSE if not
 */
int archive_supported(const char *archive_name);

/**
 * @brief Open archive 
 * @param arch_out Pointer to struct arch* for opened archive
 * @param archive_name Name of archive file
 * @param buffsize Buffer size for operations
 * @return TRUE if archive was opened, FALSE if not
 * Returned in arch_out pointer should be passed to archive_read_free()
 */
int archive_open(struct archive **arch_out, const char *archive_name,
		 size_t buffsize);

/**
 * @brief Get sorted archive content list
 * @param archive Path to archive file (absolute or relative)
 * @param items_type Type of items to list (AE_IFDIR or AE_IFREG)
 * @return newly-allocated NULL-terminated strverscmp-sorted array of newly-allocated strings 
 *         (directories have trailing '/', files - not)
 * All strings an array itself should be passed to free()
 */
char **archive_list_get(const char *archive);

/**
 * @brief Free archive list
 * @param archive_list Archive list to free (allocated by archive_get_list or archive_filter_list)
 */
void archive_list_free(char **archive_list);

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
			   mode_t items_type);

/**
 * @brief Extract file to target_directory from archive
 * @param archive Path to archive file (absolute or relative)
 * @param file Full path to file in archive (without leading /)
 * @param target_directory: Path where file will be extracted with archived path
 * @return TRUE if success, FALSE if not
 */
int archive_extract_file(const char *archive, const char *file,
			 const char *target_directory);

#endif //ARCHIVE_ROUTINES_H
