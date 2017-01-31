int  enter_archive(const char *name, struct_panel *panel, int update_config);
void enter_subarchive(const char *name, struct_panel *panel);
void leave_archive(struct_panel *panel);
void archive_go_upper(struct_panel *panel);
void archive_enter_subdir(const char *subdir, struct_panel *panel);
extern char *archive_cwd_prev; /* Previous directory in archive */
