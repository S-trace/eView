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
#include <pthread.h>

#include "gtk_file_manager.h"
#include "mylib.h"
#include "mygtk.h"
#include "cfg.h"
#include "archive_handler.h"
#include "ViewImageWindow.h" /* die_viewer_window() */
#include "os-specific.h"
#include "translations.h"
#include "digma_hw.h"

/* Various error message routines.*/
const char msg_memory_exhausted[] = "memory exhausted";

void get_system_sleep_timeout(void)
{
  #ifdef __amd64
  FILE *process=popen("echo 20", "r");
  #else
  FILE *process=popen("dbus-send --print-reply --type=method_call --dest=com.sibrary.BoeyeServer /PowerManager com.sibrary.Service.PowerManager.getSuspendTime|cut -d ' ' -f 5|tail -n 1", "r");
  #endif
  char temp_buffer[PATHSIZE+1];
  if (fgets(temp_buffer, PATHSIZE, process) == 0)
  {
    #ifdef debug_printf
    printf("Reading system sleep timeout from process failed\n");
    #endif
    temp_buffer[0]='\0';
  }
  else if (feof(process))
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
    if (fgets(temp_buffer, PATHSIZE, list_of_screensavers) == 0)
    {
      #ifdef debug_printf
      printf("Reading next screensaver filename failed\n");
      #endif
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
      (void)fclose(file_descriptor);
      #ifdef debug_printf
      printf("Reading from %s failed!\n", name);
      #endif
      *destination=strdup("\0");
      return;
    }
    *destination=strdup(temp);
    (void)fclose(file_descriptor);
    trim_line(*destination);
    #ifdef debug_printf
    printf("Read '%s' from %s\n", *destination, name);
    #endif
    return;
  }
}

void kill_panel (void) /* Убиваем struct_panel */
{
  #ifdef debug_printf
  printf ("killing panel\n");
  #endif
  xsystem("killall panel");
  return;
}

void start_panel (void) /* Перезапускаем struct_panel */
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
    asprintf(&value, "%0.2f K ", size/(float)KILOBYTE); /*Пробел в конце нужен, чтобы скроллбар не перекрывал величину измерения */
    return(value);
  }
  if (size>=MEGABYTE && size<GIGABYTE)
  {
    asprintf(&value, "%0.2f M ", size/(float)MEGABYTE);
    return(value);
  }
  if (size>=GIGABYTE)
  {
    asprintf(&value, "%0.2f G ", size/(float)GIGABYTE);
    return(value);
  }
  asprintf(&value, "%ld B ", size);
  return(value); /* Надеюсь, что эта книга никогда не столкнётся с файлами терабайтного размера */
}

char *get_natural_time(int seconds) /* Возвращает строку в формате HH:MM:ss */
{
  char *value;
  if (seconds/3600>0) /* Если число секунд > 3600 */
  {
    asprintf(&value,"%02d:%02d:%02d", seconds/3600, (seconds%3600)/60, seconds%60);
    return(value);
  }
  else
  {
    if (seconds/60>0) /* Если 3600 > число секунд > 60 */
    {
      asprintf(&value,"%02d:%02d", seconds/60, seconds%60);
      return(value);
    }
    else /* Если 60 > число секунд */
    {
      value=strdup(itoa(seconds));
      return(value);
    }
  }
}

void xsystem(const char *command) /* Вывод на экран и запуск команды */
{
  #ifdef debug_printf
  printf("Executing '%s'\n", command);
  #endif
  (void)system(command);
}

void trim_line(char *input_line) /* Удаляет последний символ у строки */
{
  #ifdef debug_printf
  printf("trim_line called for line '%s'\n", input_line);
  #endif
  if (input_line[0] != '\0')
  {
    size_t len=strlen(input_line)-1;
    input_line[len]='\0';
    return;
  }
  else
    return;
}

char *find_first_picture_name(struct_panel *panel)
{
  #ifdef DEBUG_PRINTF
  printf("entering find_first_picture_name\n");
  #endif
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *tmp;
  gboolean valid;
  model = gtk_tree_view_get_model (panel->list);
  valid=gtk_tree_model_get_iter_first (model, &iter);
  while (valid)
  {
    char *current_position_name;
    gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
    current_position_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    if (is_picture(current_position_name))
    {
      //       free(model); // Не надо - карается abort()ом
      return current_position_name;
    }
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  //   free(model); // Не надо - карается abort()ом
  return NULL;
}

char *find_next_picture_name(struct_panel *panel)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *tmp;
  gboolean valid;
  #ifdef DEBUG_PRINTF
  printf("entering find_next_picture_name\n");
  #endif
  model = gtk_tree_view_get_model (panel->list);
  (void)gtk_tree_model_get_iter_from_string (model, &iter, panel->selected_iter);
  gtk_tree_model_get (model, &iter, FILE_COLUMN , &tmp, -1);
  valid = gtk_tree_model_iter_next (model, &iter);
  while (valid)
  {
    char *current_position_name;
    gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
    current_position_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    if (is_picture(current_position_name))
    {
      //       free(model); // Не надо - карается abort()ом
      return current_position_name;
    }
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  //   free(model); // Не надо - карается abort()ом
  return NULL;
}

char *find_prev_picture_name(struct_panel *panel)
{
  #ifdef DEBUG_PRINTF
  printf("entering find_prev_picture_name\n");
  #endif
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *tmp, *last_found_image=NULL;
  gboolean valid;
  model = gtk_tree_view_get_model (panel->list);
  valid=gtk_tree_model_get_iter_first (model, &iter);
  while (valid)
  {
    char *current_position_name;
    gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
    current_position_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    if (is_picture(current_position_name))
    {
      if (strcmp(current_position_name, panel->selected_name) == 0)
      {
        //         free(model); // Не надо - карается abort()ом
        return last_found_image;
      }
      else
        last_found_image=strdup(current_position_name);
    }
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  //   free(model); // Не надо - карается abort()ом
  return NULL;
}

char *find_last_picture_name(struct_panel *panel)
{
  #ifdef DEBUG_PRINTF
  printf("entering find_last_picture_name\n");
  #endif
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *tmp, *last_found_image=NULL;
  gboolean valid;
  model = gtk_tree_view_get_model (panel->list);
  valid=gtk_tree_model_get_iter_first (model, &iter);
  while (valid)
  {
    char *current_position_name;
    gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
    current_position_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    if (is_picture(current_position_name))
      last_found_image=strdup(current_position_name);
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  //   free(model); // Не надо - карается abort()ом
  return last_found_image;
}

char *find_next_directory(struct_panel *panel) /* Поиск следующей директории в списке TODO: Переписать с обработкой не через system() а через список */
{
  FILE *fp; /* Указатель на файл */
  char next_directory[PATHSIZE+1];
  char *command;
  asprintf(&command, "find \"$(dirname \"`pwd`\")\" -type d|sed 's-$-/-g'|%s > dirlist", SORT_COMMAND);
  xsystem(command);  /* Получаем список каталогов */
  free (command);
  fp = fopen("dirlist", "r"); /* Открываем его */
  (void)remove ("dirlist"); /* И тут же удаляем, открытый он останется висеть как дескриптор */
  while(! feof(fp)) { /* Пока не конец файла */
    if (fgets ( next_directory, PATHSIZE+1, fp) == 0) /* Читаем строку из файла */
    {
      #ifdef debug_printf
      printf("Reading next directory failed (we in last directory?)\n");
      #endif
      (void)fclose(fp);/* Закрываем файл при неудачном чтении */
      return strdup(panel->path); /* И возвращаем значение текущего каталога */
    }
    trim_line(next_directory); /* Удаляем \n с конца строки */
    if ((strcmp (next_directory, panel->path) == 0)) /* Сравниваем строку с текущим каталогом */
    {
      if (fgets (next_directory, PATHSIZE+1, fp) == 0) /* При совпадении читаем ещё одну строку из файла */
      {
        #ifdef debug_printf
        printf("Reading next directory failed (we in last directory?)\n");
        #endif
        (void)fclose(fp);/* Закрываем файл при неудачном чтении */
        return strdup(panel->path); /* И возвращаем значение текущего каталога */
      }

      (void)fclose(fp);/* Закрываем файл */
      trim_line(next_directory); /* Удаляем \n с конца строки */
      #ifdef debug_printf
      printf ("Matched filename '%s'\n", next_directory);
      #endif
      return (strdup (next_directory));  /* И возвращаем значение очередной строки */
    }
  }
  (void)fclose(fp);/* Закрываем файл при неудачном поиске */
  return strdup(panel->path); /* И возвращаем значение текущего каталога */
}

char *find_prev_directory(struct_panel *panel) /* Поиск предыдущей директории в списке TODO: Переписать с обработкой не через system() а через список */
{
  FILE *fp; /* Указатель на файл */
  char next_line[PATHSIZE+1]={'\0'}; /* Строка для следующего каталога */
  char *command;
  char prev_directory[PATHSIZE+1];
  asprintf(&command, "find \"$(dirname \"`pwd`\")\" -type d|sed 's-$-/-g'|%s > dirlist", SORT_COMMAND);
  xsystem(command);  /* Получаем список каталогов */
  free (command);
  fp = fopen("dirlist", "r"); /* Открываем его */
  (void)remove ("dirlist"); /* И тут же удаляем, открытый он останется висеть как дескриптор */
  while(! feof(fp)) { /* Пока не конец файла */
    strcpy(prev_directory, next_line); /* Копируем считанную ранее строку в выходную */

    if (fgets (next_line, PATHSIZE+1, fp) == 0) /* Читаем ещё одну строку из файла */
    {
      #ifdef debug_printf
      printf("Reading next directory failed (we in last directory?)\n");
      #endif
      (void)fclose(fp);/* Закрываем файл при неудачном чтении */
      return strdup(panel->path); /* И возвращаем значение текущего каталога */
    }
    trim_line(next_line); /* Удаляем \n с конца строки */
    #ifdef debug_printf
    printf ("Filename '%s'\n", next_line);
    #endif
    if (strcmp (next_line, panel->path) == 0) /* Сравниваем строку с текущим каталогом */
    {
      (void)fclose(fp);/* Закрываем файл */
      #ifdef debug_printf
      printf ("Matched filename %s\n", prev_directory);
      #endif
      return strdup(prev_directory);  /* И возвращаем значение предыдущей строки */
    }
    else
    {
      #ifdef debug_printf
      printf ("Filename '%s' not matched!\n", next_line);
      #endif
    }
  }
  (void)fclose(fp); /* Закрываем файл при неудачном поиске */
  #ifdef debug_printf
  printf ("Filename not matched!\n");
  #endif
  return strdup(panel->path); /* И возвращаем значение текущего каталога */
}

char *next_image (char *input_name, int allow_actions, struct_panel *panel) /*выбор следующей картинки. allow_actions - разрешить ли действовать (закрывать окно) - для предзагрузки */
{
  char *now_name, *next_name;
  int next_number;
  now_name=basename(input_name);/* Хрен уследишь, откуда с путём прилетит, а откуда без! */ // Не нуждается во free()!
  if (panel->archive_depth > 0)
    now_name=xconcat(panel->archive_cwd,now_name);
  else
    now_name=strdup(now_name);
  #ifdef debug_printf
  printf("Finding next image (now at '%s'), panel->files_num=%d\n", now_name, panel->files_num);
  #endif
  free(now_name);
  next_name=find_next_picture_name(panel);
  if (next_name == NULL)
    next_number=panel->files_num;
  else
  {
    char *iter=iter_from_filename(next_name, panel);
    next_number=atoi(iter)-1;
    free(iter);
  }

  if (panel->files_num == next_number) /* При достижении конца списка */
  {
    if (loop_dir == LOOP_NONE)
    {
      #ifdef debug_printf
      printf("Already on last file!\n");
      #endif
      if (allow_actions)
      {
        GtkWidget *message=Message (INFORMATION,LAST_FILE_REACHED);
        pthread_t MessageDieDelayed_tid;
        pthread_create(&MessageDieDelayed_tid, NULL, MessageDieDelayed, (void *)message);
      }
      free(next_name);
      return (strdup(panel->selected_name));
    }
    if (loop_dir == LOOP_LOOP)
    {
      #ifdef debug_printf
      printf("Loop reached for forward\n");
      #endif
      if (allow_actions)
      {
        GtkWidget *message=Message (INFORMATION,LAST_FILE_REACHED_LOOP);
        pthread_t MessageDieDelayed_tid;
        pthread_create(&MessageDieDelayed_tid, NULL, MessageDieDelayed, (void *)message);
      }
      free(next_name);
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
            free(next_name);
            return find_first_picture_name(panel);
          }
          else
          {
            #ifdef debug_printf
            printf("JUMP FORWARD FAILED!\n");
            #endif
            Message (ERROR, UNABLE_TO_ENTER_NEXT_DIRECTORY);
            update(active_panel);
            free(next_name);
            return find_next_picture_name(panel);
          }
        }
        else
        {
          char *next_dir=find_next_directory(panel); /* Получаем следующий каталог */
          #ifdef debug_printf
          printf("NEXT_DIR=%s\n",next_dir);
          #endif
          strcpy(panel->path, next_dir);
          free(next_dir);
          if (panel == &top_panel)
            write_config_string("top_panel.path", top_panel.path);
          else
            write_config_string("bottom_panel.path", bottom_panel.path);
          #ifdef debug_printf
          printf("CHDIR to %s\n", panel->path);
          #endif
          (void)chdir (panel->path); /* Переходим в него */
        }
        update(active_panel);
        free(next_name);
        return find_first_picture_name(panel);
      }
      else
      {
        #ifdef debug_printf
        printf("Got end of directory!\n");
        #endif
        free(next_name);
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
        interface_is_locked=TRUE; // Чтобы не было двойного обновления при закрытии окна сообщения
        enable_refresh=FALSE;
        die_viewer_window(); /* Если действия разрешены */
        wait_for_draw();
        enable_refresh=TRUE;
        GtkWidget *message=Message (INFORMATION,LAST_FILE_REACHED_EXIT);
        pthread_t MessageDieDelayed_tid;
        pthread_create(&MessageDieDelayed_tid, NULL, MessageDieDelayed, (void *)message);
      }
      free(next_name);
      return(strdup(panel->selected_name));
    }
  }
  return next_name;
}

char *prev_image (char *input_name, int allow_actions, struct_panel *panel) /*выбор предыдущей картинки. allow_actions - разрешить ли действовать (закрывать окно) - для предзагрузки */
{
  char *now_name, *prev_name;
  int prev_number;
  now_name=basename(input_name);/* Хрен уследишь, откуда с путём прилетит, а откуда без! */ // Не требует free()!
  if (panel->archive_depth > 0)
    now_name=xconcat(panel->archive_cwd,now_name);
  else
    now_name=strdup(now_name);
  #ifdef debug_printf
  printf("Finding previous image (now at '%s'), panel->files_num=%d\n", now_name, panel->files_num);
  #endif
  free(now_name);
  prev_name=find_prev_picture_name(panel);
  if (prev_name == NULL)
    prev_number=0;
  else
  {
    char *iter=iter_from_filename(prev_name, panel);
    prev_number=atoi(iter);
    free(iter);
  }

  if (prev_number == 0) /* При достижении начала списка */
  {
    if (loop_dir == LOOP_NONE)
    {
      #ifdef debug_printf
      printf("Already on first image!\n");
      #endif
      if (allow_actions)
      {
        GtkWidget *message=Message (INFORMATION,FIRST_FILE_REACHED);
        pthread_t MessageDieDelayed_tid;
        pthread_create(&MessageDieDelayed_tid, NULL, MessageDieDelayed, (void *)message);
      }
      free(prev_name);
      return find_first_picture_name(panel);
    }
    if (loop_dir == LOOP_LOOP)
    {
      #ifdef debug_printf
      printf("Loop reached for backward\n");
      #endif
      if (allow_actions)
      {
        GtkWidget *message=Message (INFORMATION,FIRST_FILE_REACHED_LOOP);
        pthread_t MessageDieDelayed_tid;
        pthread_create(&MessageDieDelayed_tid, NULL, MessageDieDelayed, (void *)message);
      }
      free(prev_name);
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
          free(prev_name);
          return find_last_picture_name(panel);
        }
        else
        {
          #ifdef debug_printf
          printf("JUMP BACKWARD FAILED!\n");
          #endif
          if (allow_actions)
          {
            interface_is_locked=FALSE;
            Message (ERROR, UNABLE_TO_ENTER_PREVIOUS_DIRECTORY);
          }
          update(active_panel);
          free(prev_name);
          return find_first_picture_name(panel);
        }
      }
      else
      {
        char *prev_directory=find_prev_directory(panel); /* Получаем следующий каталог */
        strcpy(panel->path, prev_directory);
        free(prev_directory);
        if (panel == &top_panel)
          write_config_string("top_panel.path", top_panel.path);
        else
          write_config_string("bottom_panel.path", bottom_panel.path);
        #ifdef debug_printf
        printf("CHDIR to %s\n", panel->path);
        #endif
        (void)chdir (panel->path); /* Переходим в него */
      }
      update(panel);
      free(prev_name);
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
        interface_is_locked=TRUE; // Чтобы не было двойного обновления при закрытии окна сообщения
        enable_refresh=FALSE;
        die_viewer_window(); /* Если действия разрешены */
        wait_for_draw();
        enable_refresh=TRUE;
        GtkWidget *message=Message (INFORMATION,FIRST_FILE_REACHED_EXIT);
        pthread_t MessageDieDelayed_tid;
        pthread_create(&MessageDieDelayed_tid, NULL, MessageDieDelayed, (void *)message);
      }
      free(prev_name);
      return find_first_picture_name(panel);
    }
  }
  return prev_name;
}

int is_picture(char *name) __attribute__((pure));
int is_picture(char *name) /* Является ли изображением */
{
  if (strlen(name)<4) return FALSE; // Если имя файла слишком короткое
  
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
  if (strlen(name)<4) return FALSE; // Если имя файла слишком короткое
  
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
  if (strlen(name)<4) return FALSE; // Если имя файла слишком короткое
  
  size_t n = strlen(name) - 4; /* Позиция начала расширения */
  if(strcasecmp((name+n), ".txt") != 0)
    return FALSE;
  else
    return TRUE;
}

void err_msg_and_die(const char *fmt, ...)
{
  va_list p;
  (void)fflush(stdout);
  #ifdef PROGNAME
  fprintf(stderr, "%s: ", PROGNAME);
  #endif
  va_start(p, fmt);
  (void)vfprintf(stderr,fmt, p);
  va_end(p);
  (void)putc('\n', stderr);
  exit(EXIT_FAILURE);
}

/* Memory allocation */

void *xmalloc(size_t size)
{
  void *ptr;
  if ((ptr = malloc((size) ? size : 1))) return ptr;
  err_msg_and_die(msg_memory_exhausted);
  return(NULL);
}

void *xrealloc(void *ptr, size_t size)
{
  /* It avoids to free ptr if size = 0 */
  /* if ptr == NULL it does malloc(size) */
  if ((ptr = realloc(ptr, (size) ? size : 1))) return ptr;
  err_msg_and_die(msg_memory_exhausted);
  return(NULL);
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
  size_t path_max = (size_t) PATH_MAX;

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
  char *a = NULL;
  asprintf(&a, "%ld", i);
  return(a);
}

void preload_next_screensaver(void)
{
  /* сохраняем предыдущие настройки смотрелки */
  int saved_crop=crop;
  int saved_rotate=rotate;
  int saved_frame=frame;
  int saved_preload_enable=preload_enable;
  int saved_keepaspect=keepaspect;
  int saved_suspended=suspended;
  int saved_boost_contrast=boost_contrast;

  if (++suspend_count==screensavers_count-1) /* Закольцовываем список картинок для скринсейвера (последняя запись - фигня!) */
    suspend_count=0;

  #ifdef debug_printf
  printf("Preloading screensaver %s\n", screensavers_array[suspend_count]);
  #endif

  crop=rotate=frame=preload_enable=boost_contrast=FALSE; /* Грязно перенастраиваем смотрелку */
  suspended=keepaspect=TRUE;
  (void)load_image(screensavers_array[suspend_count], active_panel, FALSE, &screensaver);
  /* Восстанавливаем предыдущие настройки */
  crop=saved_crop;
  rotate=saved_rotate;
  frame=saved_frame;
  preload_enable=saved_preload_enable;
  keepaspect=saved_keepaspect;
  suspended=saved_suspended;
  boost_contrast=saved_boost_contrast;
}
