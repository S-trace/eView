char **archive_get_directories_list(struct_panel *panel, const char *directory);
char **archive_get_files_list(struct_panel *panel, const char *archive_cwd);

/*
 * Extract file to target_directory from archive
 * Input:
 * archive: Path to archive file (absolute or relative)
 * file: Full path to file in archive (without leading /)
 * target_directory: Path where file will be extracted with archived path
 */
void archive_extract_file(const char *archive, const char *file, const char *to);
int  enter_archive(const char *name, struct_panel *panel, int update_config);
void enter_subarchive(const char *name, struct_panel *panel);
void leave_archive(void);
void archive_go_upper(struct_panel *panel);
void archive_enter_subdir(const char *subdir, struct_panel *panel);
extern char *archive_cwd_prev; /* Предыдущий текущий каталог в архиве */
