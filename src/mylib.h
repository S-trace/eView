/* vi: set sw=4 s=4: */
/*
 * Various utility routines put togheter by Tito <tito-wolit@tiscali.it> .
 *
 * Copyright (C) 2002-2006 by Tito Ragusa <tito-wolit@tiscali.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "cfg.h"

extern int files_num;
void calculate_scaling_dimensions(int *new_width, int *new_height, const int image_height, const int image_width, const int display_height, const int display_width);
void get_screensavers_list(void);
void set_system_sleep_timeout(const char *timeout);
void get_system_sleep_timeout(void);
void xsystem(const char *command);
char *get_natural_size(long size);
void kill_panel (void); /* Убиваем struct_panel */
void start_panel (void); /* Запускаем struct_panel */
int is_picture(char *name);
int is_archive(char *name);
int is_text(char *name);
char *prev_image (char *now_name, int allow_actions, struct_panel *panel);
char *next_image (char *now_name, int allow_actions, struct_panel *panel);
void file_string_array (char *fname[], struct_panel *panel);
void err_msg_and_die(const char *fmt, ...)__attribute__ ((format (printf, 1, 2))) __attribute__(( __noreturn__ ));
void *xrealloc(void *ptr, size_t size);
void *xmalloc(size_t size);
void xfree(void *ptr);
char *xgetcwd (char *cwd);
char *xconcat(const char *path,const char *filename);
char *xconcat_path_file(const char *path,const char *filename);
int  xstrlen(const char* s);
void trim_line(char *input_line);
char *itoa(long i);
char *get_natural_time(int time); /* Возвращает строку в формате HH:MM:ss */
void read_string(const char *name, char **destination);
void preload_next_screensaver(void);
