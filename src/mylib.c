/* modification by Norin Maxim, 2011, Soul Trace, 2013, Based on  Mini Gtk-file-manager Copyright (C) 2002-2006
 * # * by Tito Ragusa <tito-wolit@tiscali.it>
 * # Distributed under GPLv2 Terms
 * # Various utility routines put together by Tito <tito-wolit@tiscali.it> .
 * # Copyright (C) 2002-2006 by Tito Ragusa <tito-wolit@tiscali.it> */
#include <gtk/gtk.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "gtk_file_manager.h"
#include "mylib.h"
#include "mygtk.h"
#include "cfg.h"
#include "archive_handler.h"
#include "ViewImageWindow.h" /* die_viewer_window() */
#include "os-specific.h"
#include "translations.h"

/* Various error message routines.*/
const char msg_memory_exhausted[] = "memory exhausted";
char next_directory[PATHSIZE+1];  /* Директория, в которую переходим при окончании которого */

void get_system_sleep_timeout(void)
{
//   FILE *process=popen("dbus-send --print-reply --type=method_call --dest=com.sibrary.BoeyeServer /PowerManager com.sibrary.Service.PowerManager.getSuspendTime|cut -d ' ' -f 5|tail -n 1", "r");
  FILE *process=popen("echo 20", "r");
  char temp_buffer[PATHSIZE+1];
  fgets(temp_buffer, PATHSIZE, process);
  if (feof(process))
    temp_buffer[0]='\0';
  else
    trim_line(temp_buffer);
  pclose(process);
  system_sleep_timeout=strdup(temp_buffer);
  #ifdef debug_printf
  printf("Sleep timeout is %s\n", system_sleep_timeout);
  #endif
}

void set_system_sleep_timeout(const char *timeout)
{  
  char *command;
  asprintf(&command,"dbus-send --print-reply --type=method_call --dest=com.sibrary.BoeyeServer /PowerManager com.sibrary.Service.PowerManager.setSuspendTime int32:%s", timeout);
  xsystem(command);
  free(command);
}


void get_screensavers_list(void)
{
  #ifdef __amd64
  FILE *list_of_screensavers=popen("cat boeyeserver.conf|grep ScreenSaver | cut -d = -f 2|tr -d ' '| tr ',' '\n'","r");
  #else
  FILE *list_of_screensavers=popen("cat /home/root/Settings/boeye/boeyeserver.conf|grep ScreenSaver | cut -d = -f 2|tr -d ' '| tr ',' '\n'","r");
  #endif
  char temp_buffer[PATHSIZE+1];
  while(screensavers_count <= 16 )
  {
    fgets(temp_buffer, PATHSIZE, list_of_screensavers);
    if (feof(list_of_screensavers))
    {
      pclose(list_of_screensavers);
      #ifdef debug_printf
      printf("Process closed\n");
      #endif
      break;
    }
    trim_line(temp_buffer);
    #ifdef debug_printf
    printf("read %s\n", temp_buffer);
    #endif
    strcpy(screensavers_array[screensavers_count], temp_buffer);
    screensavers_count++;
  }
}

void read_string(const char *name, char **destination) /*Чтение строкового параметра из файла name в переменную destination */
{
  FILE *file_descriptor=fopen(name,"rt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN %s FILE FOR READ!\n", name);
    #endif
    *destination=strdup("\0");
    return;
  }
  else
  {
    char temp[256];
    if (fgets(temp, PATHSIZE, file_descriptor) == 0)
    {
      fclose(file_descriptor);
      #ifdef debug_printf
      printf("Reading from %s failed!\n", name);
      #endif
      *destination=strdup("\0");
      return;
    }
    *destination=strdup(temp);
    fclose(file_descriptor);
    #ifdef debug_printf
    #endif
    trim_line(*destination);
    printf("Read '%s' from %s\n", *destination, name);
    return;
  }
}

void kill_panel (void) /* Убиваем panel */
{
  #ifdef debug_printf
  printf ("killing panel\n");
  #endif
  xsystem("killall panel");
  return;
}

void start_panel (void) /* Перезапускаем panel */
{
  #ifdef debug_printf
  printf ("starting panel\n");
  #endif
  xsystem("panel &");
  return;
}

char *get_natural_size(long size) /* Возвращает размер строки в килобайтах или мегабайтах (округлённо) */
{
  char *value;
  if (size>=KILOBYTE && size<MEGABYTE)
  {
    asprintf(&value, "%0.2f K  ", size/(float)KILOBYTE); /*Пробелы в конце нужны, чтобы скроллбар не перекрывал величину измерения */
    return(value);
  }
  if (size>=MEGABYTE && size<GIGABYTE)
  {
    asprintf(&value, "%0.2f M  ", size/(float)MEGABYTE);
    return(value);
  }
  if (size>=GIGABYTE)
  {
    asprintf(&value, "%0.2f G  ", size/(float)GIGABYTE);
    return(value);
  }
  return(xconcat(itoa(size), " B  ")); /* Надеюсь, что эта книга никогда не столкнётся с файлами терабайтного размера */
}

char *get_natural_time(int time) /* Возвращает строку в формате HH:MM:ss */
{
  char *value;
  if (time/3600>0) /* Если число секунд > 3600 */
  {
    asprintf(&value,"%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);
    return(value);
  }
  else 
  {
    if (time/60>0) /* Если 3600 > число секунд > 60 */
    {
      asprintf(&value,"%02d:%02d", time/60, time%60);
      return(value);
    }
    else /* Если 60 > число секунд */
    {
      value=strdup(itoa(time));
      return(value);
    }
  }
  return(value);
}

void xsystem(const char *command) /* Вывод на экран и запуск команды */
{
  #ifdef debug_printf
  printf("Executing '%s'\n", command);
  #endif
  system(command);
}

char *trim_line(char *input_line) /* Удаляет последний символ у строки */
{
  #ifdef debug_printf
  printf("trim_line called for line '%s'\n", input_line);
  #endif  
  if (input_line[0] != '\0')
  {
    size_t len=strlen(input_line)-1;
    input_line[len]='\0';
    return input_line;
  }
  else
    return input_line;
}

char *find_first_picture_name(panel *panel) 
{
  #ifdef DEBUG_PRINTF
  printf("entering find_first_picture_name\n");
  #endif
  GtkTreeIter iter;
  GtkTreeModel *model;
  gboolean valid = TRUE;
  char *tmp, *current_position_name=NULL;
  model = gtk_tree_view_get_model (panel->list);
  valid=gtk_tree_model_get_iter_first (model, &iter);  
  while (valid) 
  {
    gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
    current_position_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    if (is_picture(current_position_name))
      return current_position_name;
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  return NULL;
}

char *find_next_picture_name(panel *panel) 
{
  #ifdef DEBUG_PRINTF
  printf("entering find_next_picture_name\n");
  #endif
  GtkTreeIter iter;
  GtkTreeModel *model;
  gboolean valid = TRUE;
  char *tmp, *current_position_name=NULL;
  model = gtk_tree_view_get_model (panel->list);
  gtk_tree_model_get_iter_from_string (model, &iter, panel->selected_iter);  
  gtk_tree_model_get (model, &iter, FILE_COLUMN , &tmp, -1);
  valid = gtk_tree_model_iter_next (model, &iter);
  while (valid) 
  {
    gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
    current_position_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    if (is_picture(current_position_name))
      return current_position_name;
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  return NULL;
}

char *find_prev_picture_name(panel *panel) 
{
  #ifdef DEBUG_PRINTF
  printf("entering find_prev_picture_name\n");
  #endif
  GtkTreeIter iter;
  GtkTreeModel *model;
  gboolean valid = TRUE;
  char *tmp, *last_found_image=NULL, *current_position_name;
  model = gtk_tree_view_get_model (panel->list);
  valid=gtk_tree_model_get_iter_first (model, &iter);  
  while (valid) 
  {
    gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
    current_position_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    if (is_picture(current_position_name))
    {
      if (strcmp(current_position_name, panel->selected_name) == 0)
        return last_found_image;
      else
        last_found_image=strdup(current_position_name);
    }
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  return NULL;
}

char *find_last_picture_name(panel *panel)
{
  #ifdef DEBUG_PRINTF
  printf("entering find_last_picture_name\n");
  #endif
  GtkTreeIter iter;
  GtkTreeModel *model;
  gboolean valid = TRUE;
  char *tmp, *last_found_image=NULL, *current_position_name;
  model = gtk_tree_view_get_model (panel->list);
  valid=gtk_tree_model_get_iter_first (model, &iter);  
  while (valid) 
  {
    gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
    current_position_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    if (is_picture(current_position_name))
      last_found_image=strdup(current_position_name);
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  return last_found_image;
}

char *find_next_directory(panel *panel) /* Поиск следующей директории в списке TODO: Переписать с обработкой не через system() а через список */
{
  FILE *fp; /* Указатель на файл */
  static char *command; 
  asprintf(&command, "find \"$(dirname \"`pwd`\")\" -type d|sed 's-$-/-g'|%s > dirlist", SORT_COMMAND); 
  xsystem(command);  /* Получаем список каталогов */
  free (command);
  fp = fopen("dirlist", "r"); /* Открываем его */
  remove ("dirlist"); /* И тут же удаляем, открытый он останется висеть как дескриптор */
  while(! feof(fp)) { /* Пока не конец файла */
    fgets ( next_directory, PATHSIZE+1, fp); /* Читаем строку из файла */
    trim_line(next_directory); /* Удаляем \n с конца строки */
    if (!strcmp (next_directory, panel->path) || !strcmp (trim_line(next_directory), panel->path)) /* Сравниваем строку с текущим каталогом */
    {
      fgets ( next_directory, PATHSIZE+1, fp); /* При совпадении читаем ещё одну строку из файла */
      fclose(fp);/* Закрываем файл */
      trim_line(next_directory); /* Удаляем \n с конца строки */
      #ifdef debug_printf
      printf ("Matched filename '%s'\n", next_directory);
      #endif
      return next_directory;  /* И возвращаем значение предыдущей строки */
    }
  }
  fclose(fp);/* Закрываем файл при неудачном поиске */
  return panel->path; /* И возвращаем значение текущего каталога */
}

char *find_prev_directory(panel *panel) /* Поиск предыдущей директории в списке TODO: Переписать с обработкой не через system() а через список */
{
  FILE *fp; /* Указатель на файл */
  char next_line[PATHSIZE+1]={'\0'}; /* Строка для следующего каталога */
  char *command;
  asprintf(&command, "find \"$(dirname \"`pwd`\")\" -type d|sed 's-$-/-g'|%s > dirlist", SORT_COMMAND); 
  xsystem(command);  /* Получаем список каталогов */
  free (command);
  fp = fopen("dirlist", "r"); /* Открываем его */
  remove ("dirlist"); /* И тут же удаляем, открытый он останется висеть как дескриптор */
  while(! feof(fp)) { /* Пока не конец файла */
    strcpy(next_directory, next_line); /* Копируем считанную ранее строку в выходную */
    fgets (next_line, PATHSIZE+1, fp); /* Читаем следующую строку из файла */
    trim_line(next_line); /* Удаляем \n с конца строки */
    #ifdef debug_printf
    printf ("Filename '%s'\n", next_line);
    #endif
    if (!strcmp (next_line, panel->path) || !strcmp (trim_line(next_line), panel->path)) /* Сравниваем строку с текущим каталогом */
    {
      fclose(fp);/* Закрываем файл */
      #ifdef debug_printf
      printf ("Matched filename %s\n", next_directory);
      #endif
      return next_directory;  /* И возвращаем значение предыдущей строки */
    }
    else
    {
      #ifdef debug_printf
      printf ("Filename '%s' not matched!\n", next_line);
      #endif
    }
  }
  fclose(fp); /* Закрываем файл при неудачном поиске */
  #ifdef debug_printf
  printf ("Filename not matched!\n");
  #endif
  return panel->path; /* И возвращаем значение текущего каталога */
}

char *next_image (char *input_name, int allow_actions, panel *panel) /*выбор следующей картинки. allow_actions - разрешить ли действовать (закрывать окно) - для предзагрузки */
{
  char *now_name, *next_name;
  int next_number;
  now_name=basename(input_name);/* Хрен уследишь, откуда с путём прилетит, а откуда без! */
  if (panel->archive_depth > 0) now_name=xconcat(panel->archive_cwd,now_name);
  #ifdef debug_printf
  printf("Finding next image (now at '%s'), panel->files_num=%d\n", now_name, panel->files_num);
  #endif
  next_name=find_next_picture_name(panel);
  if (next_name == NULL) 
    next_number=panel->files_num;
  else
    next_number=atoi(iter_from_filename(next_name, panel))-1;
  
  if (panel->files_num == next_number) /* При достижении конца списка */
  {
    if (loop_dir == LOOP_NONE)
    {
      #ifdef debug_printf
      printf("Already on last file!\n");
      #endif
      if (allow_actions)
        Message (INFORMATION,LAST_FILE_REACHED);
      return panel->selected_name;
    }
    if (loop_dir == LOOP_LOOP)
    {
      #ifdef debug_printf
      printf("Loop reached for forward\n");
      #endif
      if (allow_actions) 
        Message (INFORMATION,LAST_FILE_REACHED_LOOP);
      return find_first_picture_name(panel);
    }
    if (loop_dir == LOOP_NEXT)
    {
      if (allow_actions) /* Предзагрузка не будет срабатывать на границе директорий - ну и гхыр с ней! */
      {
        #ifdef debug_printf
        printf("Finding next directory\n");
        #endif
        if (panel->archive_depth > 0)
        {
          if(find_next_archive_directory(panel)) /* Получаем следующий каталог */
          {
            #ifdef debug_printf
            printf("JUMP FORWARD DONE!\n");
            #endif
            update(active_panel);
            return find_first_picture_name(panel);
          }
          else
          {
            #ifdef debug_printf
            printf("JUMP FORWARD FAILED!\n");
            #endif
            Message (ERROR, UNABLE_TO_ENTER_NEXT_DIRECTORY);
            update(active_panel);
            return find_next_picture_name(panel);
          }
        }
        else
        {
          char *next_directory=find_next_directory(panel); /* Получаем следующий каталог */
          #ifdef debug_printf
          printf("NEXT_DIR=%s\n",next_directory);
          #endif
          strcpy(panel->path, next_directory); 
          write_config_string("top_panel.path", top_panel.path); /* Сохраняем его в конфиг */
          write_config_string("bottom_panel.path", bottom_panel.path); /* Сохраняем его в конфиг */
          #ifdef debug_printf
          printf("CHDIR to %s\n", panel->path);
          #endif
          chdir (panel->path); /* Переходим в него */
        }
        update(active_panel);
        return find_first_picture_name(panel);
      }
      else
      {
        #ifdef debug_printf
        printf("Got end of directory!\n");
        #endif
        return NULL;
      }
    }
    if (loop_dir == LOOP_EXIT)
    {
      #ifdef debug_printf
      printf("Got last image, exiting\n");
      #endif
      if (allow_actions) 
      {
        Message (INFORMATION,LAST_FILE_REACHED_EXIT);
        die_viewer_window(); /* Если действия разрешены */
      }
      return panel->selected_name;
    }
  }
  return next_name;
}

char *prev_image (char *input_name, int allow_actions, panel *panel) /*выбор предыдущей картинки. allow_actions - разрешить ли действовать (закрывать окно) - для предзагрузки */
{
  char *now_name, *prev_name;
  int prev_number;
  now_name=basename(input_name);/* Хрен уследишь, откуда с путём прилетит, а откуда без! */
  if (panel->archive_depth > 0) now_name=xconcat(panel->archive_cwd,now_name);
  #ifdef debug_printf
  printf("Finding previous image (now at '%s'), panel->files_num=%d\n", now_name, panel->files_num);
  #endif
  prev_name=find_prev_picture_name(panel);
  if (prev_name == NULL) 
    prev_number=0;
  else
    prev_number=atoi(iter_from_filename(prev_name, panel));
  
  if (prev_number == 0) /* При достижении начала списка */
  {
    if (loop_dir == LOOP_NONE)
    {
      #ifdef debug_printf
      printf("Already on first image!\n");
      #endif
      if (allow_actions)
        Message (INFORMATION,FIRST_FILE_REACHED);
      return find_first_picture_name(panel);
    }
    if (loop_dir == LOOP_LOOP)
    {
      #ifdef debug_printf
      printf("Loop reached for backward\n");
      #endif
      if (allow_actions) 
        Message (INFORMATION,FIRST_FILE_REACHED_LOOP);
      return find_last_picture_name(panel);
    }
    if (loop_dir == LOOP_NEXT)
    {
      #ifdef debug_printf
      printf("Finding previous directory\n");
      #endif
      if (panel->archive_depth > 0)
      {
        if(find_prev_archive_directory(panel)) /* Получаем предыдущий каталог */
        {
          #ifdef debug_printf
          printf("JUMP BACKWARD DONE!\n");
          #endif
          update(active_panel);
          return find_last_picture_name(panel);
        }
        else
        {
          #ifdef debug_printf
          printf("JUMP BACKWARD FAILED!\n");
          #endif
          if (allow_actions) 
            Message (ERROR, UNABLE_TO_ENTER_PREVIOUS_DIRECTORY);
          update(active_panel);
          return find_first_picture_name(panel);
        }
      }
      else
      {
        char *prev_directory=find_prev_directory(panel); /* Получаем следующий каталог */
        strcpy(panel->path, prev_directory); 
        write_config_string("top_panel.path", top_panel.path); /* Сохраняем его в конфиг */
        write_config_string("bottom_panel.path", bottom_panel.path); /* Сохраняем его в конфиг */
        #ifdef debug_printf
        printf("CHDIR to %s\n", panel->path);
        #endif
        chdir (panel->path); /* Переходим в него */
      }
      update(panel);
      if (panel->files_num==0)
        return find_first_picture_name(panel);
      else
        return find_last_picture_name(panel);
    }
    if (loop_dir == LOOP_EXIT)
    {
      #ifdef debug_printf
      printf("Got first image, exiting\n");
      #endif
      if (allow_actions) 
      {
        Message (INFORMATION,FIRST_FILE_REACHED_EXIT);
        die_viewer_window(); /* Если действия разрешены */
      }
      return find_first_picture_name(panel);
    }
  }
  return prev_name;
}

int is_picture(char *name) __attribute__((pure));
int is_picture(char *name) /* Является ли изображением */
{
  size_t n = strlen(name) - 4; /* Позиция начала расширения */
  if(strcasecmp((name+n), ".jpg") != 0 &&
    strcasecmp((name+n), "jpeg") != 0 &&
    strcasecmp((name+n), ".bmp") != 0 &&
    strcasecmp((name+n), ".png") != 0 &&
    strcasecmp((name+n), ".tif") != 0 &&
    strcasecmp((name+n), "tiff") != 0 &&
    strcasecmp((name+n), ".gif") != 0)
    return FALSE;
  else
    return TRUE;
}

int is_archive(char *name) __attribute__((pure));
int is_archive(char *name) /* Является ли архивом */
{
  size_t n = strlen(name) - 4; /* Позиция начала расширения */
  if(strcasecmp((name+n), ".rar") != 0 &&
    strcasecmp((name+n), ".cbr") != 0 &&
    strcasecmp((name+n), ".zip") != 0 &&
    strcasecmp((name+n), ".cbz") != 0 )
    return FALSE;
  else
    return TRUE;
}

int is_text(char *name) __attribute__((pure));
int is_text(char *name) /* Является ли текстом */
{
  size_t n = strlen(name) - 4; /* Позиция начала расширения */
  if(strcasecmp((name+n), ".txt") != 0)
    return FALSE;
  else
    return TRUE;
}

void err_msg_and_die(const char *fmt, ...)
{
  va_list p;
  fflush(stdout);
  #ifdef PROGNAME
  fprintf(stderr, "%s: ", PROGNAME);
  #endif
  va_start(p, fmt);
  vfprintf(stderr,fmt, p);
  va_end(p);
  putc('\n', stderr);
  exit(EXIT_FAILURE);
}

/* Memory allocation */

void *xmalloc(size_t size)
{
  void *ptr;
  if ((ptr = malloc((size) ? size : 1))) return ptr;
  err_msg_and_die(msg_memory_exhausted);
}

void *xrealloc(void *ptr, size_t size)
{
  /* It avoids to free ptr if size = 0 */
  /* if ptr == NULL it does malloc(size) */
  if ((ptr = realloc(ptr, (size) ? size : 1))) return ptr;  
  err_msg_and_die(msg_memory_exhausted);
}

void xfree(void *ptr)
{
  /*	 sets the pointer to NULL
   *	after it has been freed.*/
  #ifdef debug_printf
  /*   printf("called xfree\n"); */
  #endif
  
  void **pp = (void **)ptr;
  
  if (*pp == NULL) return;
  
  free(*pp);
  *pp = NULL;
}

char *xgetcwd (char *cwd)
{
  char *ret;
  unsigned path_max;
  
  path_max = (ssize_t) PATH_MAX;
  path_max += 2;                /* The getcwd docs say to do this. */
  
  if (cwd == 0) cwd = (char*)xmalloc (path_max);
  
  while ((ret = getcwd (cwd, path_max)) == NULL && errno == ERANGE) {
    path_max += 32;
    cwd = (char*)xrealloc (cwd, path_max);
  }
  #ifdef debug_printf
  printf("GET_CWD=%s\n", cwd);
  #endif
  if (ret == NULL) xfree(&cwd);
  return cwd;
}

char *xconcat(const char *path,const char *filename)/* просто слияние */
{
  char *buffer;  
  if (!path) path = "";
  
  asprintf(&buffer, "%s%s", path, filename);
  return buffer;
}

char *xconcat_path_file(const char *path,const char *filename)
{
  /*
   *      Concatenate path and file name to new allocated buffer,
   *      not adding '/' if path name already have it.
   */
  
  char *buffer;
  if (!path) 
    path = "";
  if (path[strlen(path)-1] == '/')
    asprintf(&buffer, "%s%s", path, filename);  
  else
    asprintf(&buffer, "%s%s%s", path, "/", filename);  
  return buffer;
}

char *itoa(long i)
{
  size_t digits = 2; /* automatically has room for the trailing null */
  char *a = NULL;
  long cp_i = i; /* copy of i, used for counting the digits */
  
  /* if the number is negative, we'll need to store space for the '-' */
  if(i < 0)
    digits++;
  
  /* while the copy of i has more than one digit */
  /* incrememnt the digit count, and divide copy of i by ten */
  while(cp_i > 10) {
    digits++;
    cp_i /= 10;
  }
  
  a = (char *) xmalloc(digits+2);
  
  (void) sprintf(a, "%ld", i);
  return(a);
}
