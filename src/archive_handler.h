char **archive_get_directories_list(panel *panel, const char *directory);
char **archive_get_files_list(panel *panel, const char *archive_cwd);
void archive_extract_file(const char *archive, const char *file, const char *to);
void enter_archive(const char *name, panel *panel, int update_config);
void enter_subarchive(const char *name, panel *panel);
void leave_archive(void);
int find_prev_archive_directory(panel *panel);
int find_next_archive_directory(panel *panel);
void archive_go_upper(panel *panel);
void archive_enter_subdir(const char *subdir, panel *panel);
extern char *archive_cwd_prev; /* Предыдущий текущий каталог в архиве */