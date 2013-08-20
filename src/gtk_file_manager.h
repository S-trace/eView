/*
 * GTK_FILE_MANAGER Copyright (C) 2002-2005
 * by Tito Ragusa <tito-wolit@tiscali.it>
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
#define DELAY 1000
#ifndef __HAVE_GTK_FILE_MANAGER_

#include <gtk/gtk.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

/* Максимальная глубина вложенных архивов */
#define MAX_ARCHIVE_DEPTH 16 
#define NEEDED_GTK_PARTS_VERSION 3
#define FILE_COLUMN	((int) 0)
#define SIZE_COLUMN     ((int) 1)
#define PATHSIZE 256

enum {
	KILOBYTE = 1024,
	MEGABYTE = (KILOBYTE*1024),
	GIGABYTE = (MEGABYTE*1024)
};

/* Структура описывающая панель файлового менеджера и её состояние. 
 * Принимается большинством функций работающих с файлами и/или панелями (обычно как указатель на конкретную структуру top_panel или bottom_panel). 
 * НЕ реомендуется использовать в функциях указатели *active_panel и *inactive_panel - это может легко вести к глюкам 
 * Исключение - функции, в которых необходимо использовать обе панели (файловые операции к примеру) */ 
typedef struct { 
  GtkWidget *table; /* Таблица с именами файлов  */
  GtkWidget *path_label; /* Строка с путём ниже таблцицы */
  GtkTreeView *list; /* Список файлов */
  char *path; /* Текущий путь */
  char *selected_name; /* Имя выбранного объекта */
  char *selected_size; /* Размер выбранного объекта */
  char *selected_iter; /* Итератор (номер в строковом виде) выбранного объекта */
  char *last_name; /* Последний просмотренный файл в панели */
  char *archive_cwd; /* Текущий каталог в архиве */
  char *archive_list; /* имя файла со списком архива (для возможности входа в разные архивы с разных панелей) */
  char archive_stack[MAX_ARCHIVE_DEPTH][PATHSIZE+1]; /* Стек архивов в панели */
  int files_num; /* Количество файлов в каталоге на панели */
  int dirs_num; /* Количество подкаталогов в каталоге на панели */
  int archive_depth; /* Глубина вложенных архивов (0 = FS) - количество ненулевых элементов в стеке архивов */
} struct_panel; 

void update (struct_panel *panel);/*обновление списка */
void move_selection(const char *move_to, const struct_panel *const panel); /*сдвиг выделения */
char *iter_from_filename (const char *fname, const struct_panel *panel); /*возвращает итератор файла из списка */
void start_sleep_timer(void);
void second_panel_hide(void);
void second_panel_show(void);
void delete_dir_or_file (void);
void move_dir_or_file (void);
void copy_dir_or_file (void);
void panel_selector (struct_panel *select_panel);
void menu_destroy (GtkWidget *dialog);
void shutdown(int exit_code);
extern int width_display, height_display;
extern char *top_cwd;
extern GtkWidget *main_window; /* Окно файлменеджера */
extern GtkWidget *panels_vbox; /* vBox для панелей */
extern struct_panel top_panel, bottom_panel, *active_panel, *inactive_panel;
extern int interface_is_locked;
extern int QT;
extern volatile int sleep_timer;
extern pthread_t sleep_timer_tid;
extern guint idle_call_handler;
extern int screensavers_count;
extern char screensavers_array[16][PATHSIZE+1];
#endif /* _HAVE_GTK_FILE_MANAGER_ */
