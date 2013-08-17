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

#define FILE_COLUMN	((int) 0)
#define SIZE_COLUMN     ((int) 1)
#define ACTION_NULL	((int) 0)
#define ACTION_COPY	((int) 1)
#define ACTION_SYMLINK	((int) 2)
#define ACTION_HARDLINK	((int) 3)
#define ACTION_MOVE	((int) 4)
#define ACTION_MKDIR	((int) 5)
#define ACTION_DELETE	((int) 6)
#define ACTION_PERM	((int) 7)

#ifndef COMM_LEN
#ifdef TASK_COMM_LEN
enum { COMM_LEN = TASK_COMM_LEN };
#else
/* synchronize with sizeof(task_struct.comm) in /usr/include/linux/sched.h */
enum { COMM_LEN = 16 };
#endif
#endif
typedef struct {
	DIR *dir;
	/* Fields are set to 0/NULL if failed to determine (or not requested) */
	char *cmd;
	unsigned long rss;
	unsigned long stime, utime;
	unsigned pid;
	unsigned ppid;
	unsigned pgid;
	unsigned sid;
	unsigned uid;
	unsigned gid;
	/* basename of executable file in call to exec(2), size from */
	/* sizeof(task_struct.comm) in /usr/include/linux/sched.h */
	char state[4];
	char comm[COMM_LEN];
	//	user/group? - use passwd/group parsing functions
} procps_status_t;
enum {
	PSSCAN_PID      = 1 << 0,
	PSSCAN_PPID     = 1 << 1,
	PSSCAN_PGID     = 1 << 2,
	PSSCAN_SID      = 1 << 3,
	PSSCAN_UIDGID   = 1 << 4,
	PSSCAN_COMM     = 1 << 5,
	PSSCAN_CMD      = 1 << 6,
	PSSCAN_STATE    = 1 << 7,
	PSSCAN_RSS      = 1 << 8,
	PSSCAN_STIME    = 1 << 9,
	PSSCAN_UTIME    = 1 << 10,
	/* These are all retrieved from proc/NN/stat in one go: */
	PSSCAN_STAT     = PSSCAN_PPID | PSSCAN_PGID | PSSCAN_SID
	| PSSCAN_COMM | PSSCAN_STATE
	| PSSCAN_RSS | PSSCAN_STIME | PSSCAN_UTIME,
	PSSCAN_DEF      = PSSCAN_PID | PSSCAN_UIDGID | PSSCAN_RSS
	| PSSCAN_STATE | PSSCAN_COMM | PSSCAN_CMD,
};

enum {	/* DO NOT CHANGE THESE VALUES!  cp.c depends on them. */
FILEUTILS_PRESERVE_STATUS = 1,
FILEUTILS_DEREFERENCE = 2,
FILEUTILS_RECUR = 4,
FILEUTILS_FORCE = 8,
FILEUTILS_INTERACTIVE = 0x10,
FILEUTILS_MAKE_HARDLINK = 0x20,
FILEUTILS_MAKE_SOFTLINK = 0x40,
};

enum {
	KILOBYTE = 1024,
	MEGABYTE = (KILOBYTE*1024),
	GIGABYTE = (MEGABYTE*1024)
};

#define OPT_FILEUTILS_FORCE       1
#define OPT_FILEUTILS_INTERACTIVE 2
#define ENABLE_FEATURE_PRESERVE_HARDLINKS 1


/* to port bb_functions */
#define bb_xstrdup							xstrdup
#ifdef DEBUG
#define bb_perror_msg						err_msg
#define bb_error_msg						err_msg
#define bb_error_msg_and_die(...)			do{err_msg(__VA_ARGS__);exit(1);}while(0)
#define bb_perror_msg_and_die(...)			do{err_msg(__VA_ARGS__);exit(1);}while(0)
#define debug_err_msg						err_msg
#define debug(x)							x
#else
#define bb_perror_msg
#define bb_perror_msg_and_die(...)
#define bb_error_msg
#define bb_error_msg_and_die				exit(1)
#define debug(x)
#endif
#define concat_subpath_file					xconcat_subpath_file
#define concat_path_file					xconcat_path_file

#define bb_wfopen							xwfopen
#define bb_msg_write_error					"Write Error"
#define bb_msg_read_error					"Read error"
#define RESERVE_CONFIG_BUFFER(buffer,len)	char *buffer=xmalloc(len)
#define RESERVE_CONFIG_UBUFFER(buffer,len)	unsigned char *buffer=xmalloc(len)
#define RELEASE_CONFIG_BUFFER(buffer)		free (buffer)
#define applet_name			 				"Gtkfm"
#define NEEDED_GTK_PARTS_VERSION 3

/* Busybox stuff */
typedef int (*stat_func)(const char *fn, struct stat *ps);
typedef ssize_t (*action)(int , void *, size_t);


// Максимальная глубина вложенных архивов
#define MAX_ARCHIVE_DEPTH 16 
/* Структура описывающая панель файлового менеджера и её состояние. 
 * Принимается большинством функций работающих с файлами и/или панелями (обычно как указатель на конкретную структуру top_panel или bottom_panel). 
 * НЕ реомендуется использовать в функциях указатели *active_panel и *inactive_panel - это может легко вести к глюкам 
 * Исключение - функции, в которых необходимо использовать обе панели (файловые операции к примеру) */ 
typedef struct { 
  GtkWidget *table; // Таблица с именами файлов 
  GtkWidget *path_label; // Строка с путём ниже таблцицы
  GtkTreeView *list; // Список файлов
  char *path; // Текущий путь
  char *selected_name; // Имя выбранного объекта
  char *selected_size; // Размер выбранного объекта
  char *selected_iter; // Итератор (номер в строковом виде) выбранного объекта
  char *last_name; // Последний просмотренный файл в панели
  char *archive_cwd; // Текущий каталог в архиве
  char *archive_list; // имя файла со списком архива (для возможности входа в разные архивы с разных панелей)
  char archive_stack[MAX_ARCHIVE_DEPTH][256]; // Стек архивов в панели
  int files_num; // Количество файлов в каталоге на панели
  int dirs_num; // Количество подкаталогов в каталоге на панели
  int archive_depth; // Глубина вложенных архивов (0 = FS) - количество ненулевых элементов в стеке архивов
} panel; 

int recursive_action(const char *fileName,
		     int recurse, int followLinks, int depthFirst,
		     int (*fileAction)(const char *fileName, struct stat *statbuf, void* userData, int depth),
		     int (*dirAction) (const char *fileName, struct stat *statbuf, void* userData, int depth),
		     void* userData,
		     int depth);
ssize_t safe_write(int fd, const void *buf, size_t count);
ssize_t full_write(int fd, const void *buf, size_t len);
ssize_t safe_read(int fd, void *buf, size_t count);
ssize_t open_read_close(const char *filename, void *buf, size_t size);
int bb_copyfd(int fd1, int fd2, const off_t chunksize);
int is_directory(const char *fileName, const int followLinks, struct stat *statBuf);
char *xreadlink(const char *path);
int remove_file(const char *path, int flags);
int bb_make_directory (char *path, long mode, int flags);
int cp_mv_stat2(const char *fn, struct stat *fn_stat, stat_func sf);
int cp_mv_stat(const char *fn, struct stat *fn_stat);
char *bb_get_last_path_component(char *path);
int move_rename(const char *source, const char *dest , int flags);
int copy_file(const  char *source,const  char *dest, int flags);
const char *make_human_readable_str(unsigned long long size,unsigned long block_size, unsigned long display_unit);
const char *bb_mode_string(int mode);
off_t bb_copyfd_size(int fd1, int fd2, off_t size);
off_t bb_copyfd_eof(int fd1, int fd2);
char *find_block_device(char *path);
struct mntent *find_mount_point(const char *name, const char *table);
const char* get_cached_username(uid_t uid);
const char* get_cached_groupname(gid_t gid);
procps_status_t* procps_scan(procps_status_t* sp, int flags);
int is_in_ino_dev_hashtable(const struct stat *statbuf, char **name);
void add_to_ino_dev_hashtable(const struct stat *statbuf, const char *name);
void erase_mtab(const char *name);
char *bb_getgrgid(char *group, long gid, int bufsize);
char *bb_getpwuid(char *name, long uid, int bufsize);

void update (panel *panel);//обновление списка
void move_selection(char *move_to, panel *panel); //сдвиг выделения
char *iter_from_filename (char *fname, panel *panel); //возвращает итератор файла из списка
extern int width_display, height_display;
extern char *top_cwd;
extern GtkWidget *main_window; // Окно файлменеджера
extern GtkWidget *panels_vbox; // vBox для панелей
extern panel top_panel, bottom_panel, *active_panel, *inactive_panel;
extern int interface_is_locked;
extern int QT;
extern volatile int sleep_timer;
extern pthread_t sleep_timer_tid;
extern guint idle_call_handler;

void start_sleep_timer(void);
void *sleep_thread(__attribute__((unused))void* arg);
void dirlist_focus (void);
void second_panel_hide(void);
void second_panel_show(void);
void delete_dir_or_file (void);
void move_dir_or_file (void);
void copy_dir_or_file (void);
void move_confirmation (int n);
void refresh_for_dirlist(int tru);
void panel_selector (panel *select_panel);
void menu_destroy (GtkWidget *dialog);
char *first_panel_path (void);
void first_panel_update (void);
void after_delete_update (panel *panel);
void update_topcwd(void);
int top_str_iter (void);
void list_f(panel *panel);
void list_fd(panel *panel);
void shutdown(int exit_code);
#endif /* _HAVE_GTK_FILE_MANAGER_ */
