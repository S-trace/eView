// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is an open source non-commercial project. Dear PVS-Studio, please check it.
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
#include <libgen.h> 

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

void calculate_scaling_dimensions(int *new_width, int *new_height, const int image_height, const int image_width, const int display_height, const int display_width)
{
  // cppcheck-suppress "variableScope"
  double scale = 0, scale_width = 0, scale_height = 0;
  TRACE("calculate_scaling_dimensions called (image %dx%d, display %dx%d)\n", image_height, image_width, display_height, display_width);
  if (display_height > 0 && display_width > 0)
  {
    scale_width  = (double)display_width / image_width;
    scale_height = (double)display_height  / image_height;
    scale = scale_height < scale_width ? scale_height : scale_width;
  }
  else if (display_height > 0)
    scale = (double)display_height / image_height;
  else if (display_width > 0)
    scale = (double)display_width / image_width;
  else
    scale=1;
  TRACE("scale_width=%f, scale_height=%f, scale=%f\n", scale_width, scale_height, scale);

  *new_width = scale * image_width;
  *new_height = scale * image_height;
  TRACE("new_width=%d, new_height=%d\n", *new_width, *new_height);
}

// cppcheck-suppress "unusedFunction"
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
    TRACE("Reading system sleep timeout from process failed\n");
    temp_buffer[0]='\0';
  }
  else if (feof(process))
    temp_buffer[0]='\0';
  else
    trim_line(temp_buffer);
  pclose(process);
  system_sleep_timeout=strdup(temp_buffer);
  TRACE("Sleep timeout is %s\n", system_sleep_timeout);
}

// cppcheck-suppress "unusedFunction"
void set_system_sleep_timeout(const char *timeout)
{
  if (timeout) {
    char *command;
    asprintf(&command,"dbus-send --print-reply --type=method_call --dest=com.sibrary.BoeyeServer /PowerManager com.sibrary.Service.PowerManager.setSuspendTime int32:%s", timeout);
    xsystem(command);
    free(command);
  }
  else
  {
    TRACE("timeout is NULL, skipping dbus-send call!");
  }
}

// cppcheck-suppress "unusedFunction"
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
      TRACE("Reading next screensaver filename failed\n");
      pclose(list_of_screensavers);
      TRACE("Process closed\n");
      break;
    }
    trim_line(temp_buffer);
    TRACE("read %s\n", temp_buffer);
    strcpy(screensavers_array[screensavers_count], temp_buffer);
    screensavers_count++;
  }
}

void read_string(const char *name, char **destination) /*Чтение строкового параметра из файла name в переменную destination */
{
  FILE *file_descriptor=fopen(name,"rt");
  if (!file_descriptor)
  {
    TRACE("UNABLE TO OPEN %s FILE FOR READ!\n", name);
    *destination=strdup("\0");
    return;
  }
  else
  {
    char temp[256];
    if (fgets(temp, PATHSIZE, file_descriptor) == 0)
    {
      (void)fclose(file_descriptor);
      TRACE("Reading from %s failed!\n", name);
      *destination=strdup("\0");
      return;
    }
    *destination=strdup(temp);
    (void)fclose(file_descriptor);
    trim_line(*destination);
    TRACE("Read '%s' from %s\n", *destination, name);
    return;
  }
}

void kill_panel (void) /* Убиваем struct_panel */
{
  TRACE ("killing panel\n");
  xsystem("killall panel");
  return;
}

void start_panel (void) /* Перезапускаем struct_panel */
{
  TRACE ("starting panel\n");
  xsystem("panel &");
  return;
}

// cppcheck-suppress "unusedFunction"
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
  TRACE("Executing '%s'\n", command);
  (void)system(command);
}

void trim_line(char *input_line) /* Удаляет последний символ у строки */
{
  TRACE("trim_line called for line '%s'\n", input_line);
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
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *tmp;
  gboolean valid;
  TRACE("entering find_first_picture_name\n");
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
  TRACE("entering find_next_picture_name\n");
  model = gtk_tree_view_get_model (panel->list);
  if (gtk_tree_model_get_iter_from_string (model, &iter, panel->selected_iter))
  {
    gboolean valid;
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
  }
  //   free(model); // Не надо - карается abort()ом
  return NULL;
}

char *find_prev_picture_name(struct_panel *panel)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *tmp, *last_found_image=NULL;
  gboolean valid;
  TRACE("entering find_prev_picture_name\n");
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
  free(last_found_image);
  return NULL;
}

char *find_last_picture_name(struct_panel *panel)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *tmp, *last_found_image=NULL;
  gboolean valid;
  TRACE("entering find_last_picture_name\n");
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

char *find_next_node(struct_panel *panel, int reset_position) /* Поиск следующей директории или архива в списке */
{
  char *tmp;
  gboolean valid;
  GtkTreeIter iter;
  GtkTreeModel *model;
  TRACE("entering find_first_picture_name\n");
  model = gtk_tree_view_get_model (panel->list);
  if (reset_position == TRUE) // Получаем итератор первого объекта в списке
    gtk_tree_model_get_iter_first (model, &iter);
  else // Получаем итератор текущего выбранного объекта
    gtk_tree_model_get_iter_from_string (model, &iter, panel->selected_iter);
  valid = gtk_tree_model_iter_next (model, &iter); // И начинаем поиск со следующей
  while (valid)
  {
    char *current_position_name;
    gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
    current_position_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    
    // Если под курсором оказалась строка с каталогом или архивом - выдаём её
    if (is_directory(current_position_name, panel) || is_archive(current_position_name))
      return (strdup(current_position_name));
    else
      g_free(current_position_name);
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  if (strcmp(panel->path, "/") == 0) // Если мы достигли верха в структуре каталогов, и ниже нет ничего
    return NULL; 
  
  // Если управление всё ещё в функции (мы не встретили ни каталогов, ни архивов) - поднимаемся на уровень вверх
  go_upper(panel);
  wait_for_draw();
  // А затем - рекурсивно вызываем себя же не сбрасывая позицию курсора, пока не найдём подходящий каталог
  return(find_next_node(panel, FALSE));
}

char *find_prev_node(struct_panel *panel) /* Поиск следующей директории или архива в списке */
{
  char *tmp;
  int current_row;
  gboolean valid;
  GtkTreeIter iter;
  GtkTreeModel *model;
  TRACE("entering find_prev_node\n");
  model = gtk_tree_view_get_model (panel->list);
  go_upper(panel);
  wait_for_draw();
  current_row = atoi(panel->selected_iter) - 1; // Получаем текущий выбор
  valid = gtk_tree_model_get_iter_from_string (model, &iter, itoa(current_row));
  while (valid && current_row > 0)
  {
    char *current_position_name;
    gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
    current_position_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    if (is_archive(current_position_name))
    {
      if (panel->archive_depth == 0)
      {
        enter_archive(current_position_name, panel, TRUE);
        current_row = panel->dirs_num + panel->files_num - 1; // Сбрасываем номер текущей строки
      }
      else
      {
        char *subarchive=xconcat(panel->archive_cwd, current_position_name);
        enter_subarchive(subarchive, panel);
        free(subarchive);
        current_row = panel->dirs_num + panel->files_num - 1; // Сбрасываем номер текущей строки
      }
    }
    else if (is_directory(current_position_name, panel))
    {
      enter_subdir(current_position_name, panel);
      current_row = panel->dirs_num + panel->files_num - 1; // Сбрасываем номер текущей строки
    }
    else
      g_free(current_position_name);
    valid = gtk_tree_model_get_iter_from_string (model, &iter, itoa(current_row--));
  }
  if (strcmp(panel->path, "/") == 0) // Если мы достигли верха в структуре каталогов, и выше нет ничего
    return NULL;
  return (char *) -1;
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
  TRACE("Finding next image (now at '%s'), panel->files_num=%d\n", now_name, panel->files_num);
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
      TRACE("Already on last file!\n");
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
      TRACE("Loop reached for forward\n");
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
        char *next_node;
        TRACE("Finding next directory\n");
        next_node=find_next_node(panel, TRUE); /* Get next directory */
        if (next_node == NULL)
        {
          TRACE("JUMP FORWARD FAILED!\n");
          Message (ERROR, UNABLE_TO_ENTER_NEXT_DIRECTORY);
          update(active_panel);
          free(next_name);
          return find_next_picture_name(panel);
        }
        else
        {
          char *next;
          TRACE("NEXT_NODE=%s\n",next_node);
          if (is_archive(next_node)) // Если find_next_node() вернула имя архива - входим в него
          {
            if (panel->archive_depth == 0)
              enter_archive(next_node, panel, TRUE);
            else
            {
              char *subarchive=xconcat(panel->archive_cwd, next_node);
              enter_subarchive(subarchive, panel);
              free(subarchive);
            }
          }
          else
            enter_subdir(next_node, panel);
          free(next_node);
          next=find_first_picture_name(panel);
          if ( next == NULL)
            Message(ERROR, NO_IMAGES_IN_CURRENT_DIRECTORY);
          return find_first_picture_name(panel);
        }
      }
      else
      {
        TRACE("Got end of directory!\n");
        free(next_name);
        return NULL;
      }
    }
    if (loop_dir == LOOP_EXIT)
    {
      TRACE("Got last image, exiting\n");
      if (allow_actions)
      {
        GtkWidget *message;
        pthread_t MessageDieDelayed_tid;
        interface_is_locked=TRUE; // Чтобы не было двойного обновления при закрытии окна сообщения
        enable_refresh=FALSE;
        die_viewer_window(); /* Если действия разрешены */
        wait_for_draw();
        enable_refresh=TRUE;
        message=Message (INFORMATION,LAST_FILE_REACHED_EXIT);
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
  TRACE("Finding previous image (now at '%s'), panel->files_num=%d\n", now_name, panel->files_num);
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
      TRACE("Already on first image!\n");
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
      TRACE("Loop reached for backward\n");
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
      if (allow_actions) /* Предзагрузка не будет срабатывать на границе директорий - ну и гхыр с ней! */
      {
        char *prev_node;
        TRACE("Finding previous directory\n");
        prev_node=find_prev_node(panel); /* Get prev directory */
        if (prev_node == NULL)
        {
          TRACE("JUMP BACKWARD FAILED!\n");
          Message (ERROR, UNABLE_TO_ENTER_PREVIOUS_DIRECTORY);
          update(active_panel);
          free(prev_name);
          return find_first_picture_name(panel);
        }
        else
        {
          if (prev_node != (char *) -1) // Сюда мы никогда не попадём, но пусть будет
          {
            TRACE("PREV_NODE=%s\n", prev_node);
            if (is_archive(prev_node)) // Если find_prev_node() вернула имя архива - входим в него
            {
              if (panel->archive_depth == 0)
                enter_archive(prev_node, panel, TRUE);
              else
              {
                char *subarchive=xconcat(panel->archive_cwd, prev_node);
                enter_subarchive(subarchive, panel);
                free(subarchive);
              }
            }
            else
              enter_subdir(prev_node, panel);
            free(prev_node);
          }
          free(prev_name);
          if (panel->files_num==0)
            return find_first_picture_name(panel);
          else
            return find_last_picture_name(panel);
        }
      }
      else
      {
        TRACE("Got end of directory!\n");
        free(prev_name);
        return NULL;
      }
    }    
    if (loop_dir == LOOP_EXIT)
    {
      TRACE("Got first image, exiting\n");
      if (allow_actions)
      {
        GtkWidget *message;
        pthread_t MessageDieDelayed_tid;
        interface_is_locked=TRUE; // Чтобы не было двойного обновления при закрытии окна сообщения
        enable_refresh=FALSE;
        die_viewer_window(); /* Если действия разрешены */
        wait_for_draw();
        enable_refresh=TRUE;
        message=Message (INFORMATION,FIRST_FILE_REACHED_EXIT);
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
  size_t n;
  if (strlen(name)<4) return FALSE; // Если имя файла слишком короткое

  n = strlen(name) - 4; /* File extension start position */
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
  size_t n;
  if (strlen(name)<4) return FALSE; // Если имя файла слишком короткое

  n = strlen(name) - 4; /* File extension start position */
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
  size_t n;
  if (strlen(name)<4) return FALSE; // Если имя файла слишком короткое

  n = strlen(name) - 4; /* File extension start position */
  if(strcasecmp((name+n), ".txt") != 0)
    return FALSE;
  else
    return TRUE;
}

int is_directory(char *name, struct_panel *panel) /* Является ли каталогом */
{
  GtkTreeIter iter;
  char *tmp, *iter_string=iter_from_filename(name, panel);
  GtkTreeModel *model = gtk_tree_view_get_model (panel->list);
  if (gtk_tree_model_get_iter_from_string (model, &iter, iter_string))
  {
    char *file_size;
    int res;
    free (iter_string);
    gtk_tree_model_get (model, &iter, SIZE_COLUMN , &tmp, -1);
    file_size = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    res = strcmp(file_size, "dir ");
    g_free(file_size);

    if (res == 0) /* каталог */
      return TRUE;
    else
      return FALSE;
  }    
  else 
  {
    free (iter_string);
    return FALSE;
  }
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
  void *tmpptr = realloc(ptr, (size) ? size : 1);
  if (tmpptr) return tmpptr;
  err_msg_and_die(msg_memory_exhausted);
  return(NULL);
}

void xfree(void *ptr)
{
  /*	 sets the pointer to NULL
   *	after it has been freed.*/
  /*   TRACE("called xfree\n"); */

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
  TRACE("GET_CWD=%s\n", cwd);
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
  char *buffer;
  if (path < (char *)2)
  {
    TRACE("Path passed to xconcat_path_file() is NULL!\n");
    return (strdup(filename));
  }
  /*
   *      Concatenate path and file name to new allocated buffer,
   *      not adding '/' if path name already have it.
   */

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

  TRACE("Preloading screensaver %s\n", screensavers_array[suspend_count]);

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

/*
 * Recursive mkdir() call (like bash's mkdir -p do)
 * Origin; http://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
 */
int mkpath(char *dir, mode_t mode)
{
	struct stat sb;
	
	if (!dir) {
		errno = EINVAL;
		return 1;
	}
	
	if (!stat(dir, &sb))
		return 0;
	
	mkpath(dirname(strdupa(dir)), mode);
	
	return mkdir(dir, mode);
}