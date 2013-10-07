/*
 *
 * Gtk+2.x Utility routines
 * Copyright (C) 2002-2006 by Tito Ragusa <tito-wolit@tiscali.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Please read the COPYING and README file!!!
 *
 * Report bugs to <tito-wolit@tiscali.it>
 *
 */
#ifndef	__HAVE_MYGTK_
#define	__HAVE_MYGTK_ 1

#define clear_list(x)					gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(x))))

#define NOT_EDITABLE	((int) 0)
#define EDITABLE		((int) 1)

#define NOT_MODAL		((int) 0)
#define MODAL			((int) 1)

#define SHOW			((int) 0)
#define HIDE			((int) 1)

#define AUTOSCROLL		((int) 1)
#define NO_AUTOSCROLL	        ((int) 0)

#define NOT_HOMOGENEUS	        ((int) 0)
#define HOMOGENEUS		((int) 1)

GtkWidget *window_create( int x, int y, guint border, const char *title, int modal);
GtkTreeView *string_list_create_on_table(size_t num, GtkWidget *table, guint start_col, guint end_col,
                                         guint start_row, guint end_row, guint show_hide, guint editable,...);
void add_data_to_list(GtkTreeView *tree, const char *data_string, int n_columns, int autoscroll, const char *fs);

#endif /* __HAVE_MYGTK_  */
void e_ink_refresh_part(void);
void e_ink_refresh_local(void);
void e_ink_refresh_full(void);
void e_ink_refresh_default(void);
#define COLUMN_W (width_display * 83 / 100) /* Первая колонка в файлменеджере (для имён). 83 - ширина в процентах */
#define COLUMN_W2 (width_display * 17 / 100)/* Вторая колонка в файлменеджере (для размеров). 17 - ширина в процентах */
void *MessageDieDelayed (void *arg);
int MessageDie (GtkWidget *Window);
GtkWidget *Message (const char *const title, const char *const message);
extern int enable_refresh;/*Принудительно запретить обновлять экран в особых случаях */
/* extern char focus_in_processed; // Количество обработанных событий получения фокуса */
/* gint focus_in_callback (void); // реакция на получение фокуса */
/* gint focus_out_callback (void); // реакция на потерю фокуса */
void wait_for_draw (void);
gboolean confirm_request(const char *title, const char *confirm_button, const char *reject_button);
void create_panel (struct_panel *panel);
char *get_current_iter (struct_panel *panel); /*возвращает итератор текущего файла из списка */
void Qt_error_message(const char *message); /* Функция открывает на прошивках Qt сообщение об ошибке в стандартной читалке */
void enter_suspend(struct_panel *panel); /* Показывает скринсейвер и усыпляет железо */
int check_key_press(guint keyval, struct_panel *panel); /* Проработка нажатия кнопки питания при сне */
void select_file_by_name(const char * const name, const struct_panel * const panel);
extern int suspend_count;
void go_upper(struct_panel *panel); /* Переход на уровень вверх в дереве */
void enter_subdir(char *name, struct_panel *panel);/* Переход на уровень вниз в дереве panel->list */
