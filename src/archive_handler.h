char **archive_get_directories_list(panel *panel, char *directory);
char **archive_get_files_list(panel *panel, char *archive_cwd);
void archive_extract_file(char *archive, char *file, char *to);
void enter_archive(char *name, panel *panel, int update_config);
void enter_subarchive(char *name);
void leave_archive(void);
int find_prev_archive_directory(panel *panel);
int find_next_archive_directory(panel *panel);
void archive_go_upper(panel *panel);
void archive_enter_subdir(char *subdir, panel *panel);
extern char *archive_cwd_prev; // Предыдущий текущий каталог в архиве