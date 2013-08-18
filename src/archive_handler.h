char **archive_get_directories_list(struct_panel *panel, const char *directory);
char **archive_get_files_list(struct_panel *panel, const char *archive_cwd);
void archive_extract_file(const char *archive, const char *file, const char *to);
void enter_archive(const char *name, struct_panel *panel, int update_config);
void enter_subarchive(const char *name, struct_panel *panel);
void leave_archive(void);
int find_prev_archive_directory(struct_panel *panel);
int find_next_archive_directory(struct_panel *panel);
void archive_go_upper(struct_panel *panel);
void archive_enter_subdir(const char *subdir, struct_panel *panel);
extern char *archive_cwd_prev; /* Предыдущий текущий каталог в архиве */