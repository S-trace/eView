/* Norin Maxim, 2011, Soul Trace, 2013, Based on  Mini Gtk-file-manager Copyright (C) 2002-2006
 * # * by Tito Ragusa <tito-wolit@tiscali.it>
 * #  Distributed under GPLv2 Terms*/

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h> /*open() */
#include <X11/Xlib.h> /*XOpenDisplay() */
#include <signal.h> /*signal() */
#include <pthread.h>

#ifdef debug
#include <execinfo.h> /*backtrace() */
#endif

#include "gtk_file_manager.h" /* Инклюдить первой среди своих, ибо typedef panel! */
#include "mylib.h"
#include "mygtk.h"
#include "cfg.h"
#include "digma_hw.h"
#include "ViewImageWindow.h"
#include "debug_msg_win.h"
#include "archive_handler.h"
#include "translations.h"
#include "interface.h"

int interface_is_locked=TRUE; /* Сразу блокируем интерфейс программы, чтобы при запуске она не падала если зажата клавиша */
struct_panel top_panel, bottom_panel, *active_panel, *inactive_panel;
GtkWidget *main_window, *panels_vbox; /* Окно файлменеджера */
int width_display, height_display;
int framebuffer_descriptor=0; /* Дескриптор файла фреймбуффера (для обновления) */
int screensavers_count=0;
char screensavers_array[16][PATHSIZE+1];

volatile int sleep_timer;
pthread_t sleep_timer_tid;
guint idle_call_handler; /* Хэндлер вызова режима ожидания, нужен чтобы снять вызов функции ожидания после первого вызова */

void /*@null@*/  *sleep_thread(__attribute__((unused)) /*@unused@*/ void* arg)
{
  TRACE("Sleep timeout thread started (timer=%d)\n", sleep_timer);
  for (;;)
  {
    (void)usleep(1000000); /* Спим 1 секунду */
    if (--sleep_timer <= 0)
    {
      if (sleep_timeout == 0)
      {
        TRACE("Sleep timeout disabled, ending thread\n");
        break;
      }
      TRACE("Sleep timeout reached, go sleep\n");
      sleep_timer=999999999; /* не дёргаемся по пустякам, когда надо - таймер сбросят */
      idle_call_handler=g_idle_add ((GSourceFunc) enter_suspend, active_panel);
    }
  }
  TRACE("Sleep thread ended\n");
  return(NULL);
}

// void wait_state(GtkWidget *window) /* Возврат после смотрелки */
// {
//   TRACE("wait_state called\n");
//   enable_refresh=FALSE;
//   update(active_panel);
//   gtk_widget_show_all(window);
//
//   gtk_widget_queue_draw(GTK_WIDGET(active_panel->list)); /* Заставляем GTK перерисовать список каталогов */
//   /*   g_signal_connect (G_OBJECT (window), "focus_in_event", */
//   /*                     G_CALLBACK (focus_in_callback), NULL); */
//   /*   g_signal_connect (G_OBJECT (window), "focus_out_event", */
//   /*                     G_CALLBACK (focus_out_callback), NULL); */
//   /*   TRACE("FMAN CONNECTED!\n"); */
// }

void list_fd(struct_panel *panel) /*добавление списка имен каталогов, файлов и их размеров в панель struct_panel */
{
  int i = 0;
  panel->files_num=0;
  panel->dirs_num=0;
  if (panel->archive_depth > 0) /* Поведение в архиве */
  {
    char const *back="../";
    char **namelist, *text;
    add_data_to_list(panel->list, back, 1, NO_AUTOSCROLL, "dir ");
    panel->dirs_num++;
    namelist=archive_get_directories_list(panel, panel->archive_cwd);
    if ( namelist[0] != NULL)
    {
      while( namelist[i] != 0 && namelist[i][0] != '\0' && GTK_IS_WIDGET(main_window) )
      {
        char *full_name, *text_basename;
        if ((show_hidden_files == FALSE) && namelist[i][0] == '.') {continue;}
        panel->dirs_num++;
        text=strdup(namelist[i]);
        trim_line(text); /* Ампутируем последний слэш */
        text_basename=basename(text);
        full_name=xconcat(text_basename,"/");
        free(text);
        TRACE("Adding %d archive file '%s'\n", i, full_name);
        add_data_to_list(panel->list, full_name, 1, NO_AUTOSCROLL, "dir ");
        xfree(&namelist[i]);
        free(full_name);
        i++;
      }
    }
    free(namelist);

    namelist=archive_get_files_list(panel, panel->archive_cwd);
    if ( namelist[0] != NULL)
    {
      i=0; /* Сбрасываем счётчик, иначе карается сегфолтом! */
      while(namelist[i][0] != '\0' && GTK_IS_WIDGET(main_window) ) /* Пока имя файла не вырождается в пустую строку (грёбаный греп с его повадками!) */
      {
        if ((show_hidden_files == FALSE) && namelist[i][0] == '.') {continue;}
        text=basename(namelist[i]);
        add_data_to_list(panel->list, text, 1, NO_AUTOSCROLL, "file ");
        panel->files_num++;
        xfree (&namelist[i]);
        //         free(text); // basename() - free() противопоказан!
        i++;
      }
    }
    free(namelist);
  }
  else /* Поведение в ФС */
  {
    int n = 0;
    char *previous_path=xgetcwd(NULL);
    struct dirent  **namelist;
    if (panel->path == NULL) return;
    (void)chdir(panel->path);
    if ((n = scandir(panel->path, &namelist, 0, versionsort)) >= 0)
    {
      for( i = 0; i < n && GTK_IS_WIDGET(main_window); i++ ) /* Первый обход списка - каталоги */
      {
        struct stat stat_p;
        /*убрать точку из списка папок */
        if (strcmp(namelist[i]->d_name, ".") == 0) {continue;}
        /*убрать двоеточку из списка папок при присутствии в корневом каталоге */
        if (strcmp(namelist[i]->d_name, "..") == 0 && strcmp (panel->path, "/") == 0) {continue;}
        /*Убрать скрытные каталоги */
        if ((show_hidden_files == FALSE) && namelist[i]->d_name[0] == '.' && strcmp(namelist[i]->d_name, "..") != 0) {continue;}
        errno=0;
        if (stat(namelist[i]->d_name, &stat_p) == 0)
        {
          if (S_ISDIR(stat_p.st_mode))
          {
            char *text;
            text = xconcat_path_file(namelist[i]->d_name, "");
            panel->dirs_num++;
            TRACE("Adding %d dir '%s'\n", i, text);
            add_data_to_list(panel->list, text, 1, NO_AUTOSCROLL, "dir ");
            free(text);
          }
        }
        else
        {
          TRACE("stat() for '%s' failed (%s)\n",namelist[i]->d_name, strerror(errno));
        }
      }
      for( i = 0; i < n && GTK_IS_WIDGET(main_window); i++ )  /* Второй обход списка - файлы */
      {
        struct stat stat_p;
        (void)stat(namelist[i]->d_name, &stat_p);
        errno=0;
        if (stat(namelist[i]->d_name, &stat_p) == 0)
        {
          if (!S_ISDIR(stat_p.st_mode))
          {
            /*Убрать скрытные файлы */
            char *fsize, *text;
            if ((show_hidden_files == FALSE) && namelist[i]->d_name[0] == '.') {continue;}
            text = namelist[i]->d_name;
            panel->files_num++;
            fsize = get_natural_size(stat_p.st_size); /*размер файла */
            TRACE("Adding %d file '%s', size %s\n", i, text, fsize);
            add_data_to_list(panel->list, text, 1, NO_AUTOSCROLL, fsize);
            free (fsize);
          }
        }
        else
        {
          TRACE("stat() for '%s' failed (%s)\n",namelist[i]->d_name, strerror(errno));
        }

        xfree(&namelist[i]);
      }
    }
    (void)chdir(previous_path);
    free(previous_path);
    free(namelist);
  }
}

char *iter_from_filename (const char *const fname, const struct_panel *const panel) /*возвращает итератор (номер) файла с именем fname из списка в панели struct_panel */
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  gboolean valid = TRUE;
  char *tmp;
  if (fname == NULL)
  {
    TRACE("Filename passed to iter_from_filename() is NULL!\n");
    return NULL;
  }
  TRACE("Trying to find iter for name '%s'\n", fname);

  model = gtk_tree_view_get_model (panel->list);
  (void)gtk_tree_model_get_iter_first (model, &iter);
  if (iter.stamp != 0)
  {
    while (valid) {
      char *inlist;
      gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
      inlist = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
      xfree(&tmp);
      if (strcmp (inlist, fname) == 0)
      {
        free(inlist);
        inlist=gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL(model), &iter);
        return inlist;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
      free(inlist);
    }
  }
  //   free(model); // Не надо - карается abort()ом
  TRACE("Iter not found!\n");
  return (NULL);
}

void update_window_title(struct_panel *panel)
{
  char *title;
  asprintf(&title, "Dirs: %d Files: %d %s", panel->dirs_num-1, panel->files_num, VERSION); // Устанавливаем правильное количество файлов в каталоге
  gtk_window_set_title(GTK_WINDOW(main_window), title);
  xfree (&title);
}

void update(struct_panel *panel) /*обновление списка */
{
  if (access(panel->path, R_OK))
  {
    TRACE("Unable to access path %s\n", panel->path);  
    go_upper(panel);
  }
  if (LED_notify) set_led_state (LED_state[LED_BLINK_FAST]);
  panel->files_num=0; /* Обнуляем число файлов в просмотрщике */
  clear_list(panel->list);
  #ifdef debug
  if (panel->archive_depth > 0)
    TRACE("We are now IN ARCHIVE!\n");
  #endif
  list_fd(panel);

  if (panel == active_panel)
  {
    update_window_title(panel);
  }
  if (panel->archive_depth > 0) /* Пишем имя архива с путём в поле снизу */
  {
    char *path=xconcat_path_file(panel->archive_stack[panel->archive_depth], panel->archive_cwd);
    gtk_label_set_text (GTK_LABEL(panel->path_label), path);
    free(path);
  }
  else
    gtk_label_set_text (GTK_LABEL(panel->path_label), panel->path);
  //   free(model); // Не надо - карается abort()ом
  select_file_by_name (panel->selected_name, panel);
  gtk_widget_queue_draw(GTK_WIDGET(active_panel->list)); /* Заставляем GTK перерисовать список каталогов */
  wait_for_draw();
  if (LED_notify) set_led_state (LED_state[LED_OFF]); /* Индикация активности */
}

void move_selection(const char *const move_to, const struct_panel *const panel) /* сдвигает курсор на заданную строку в символьном виде */
{
  GtkTreePath *path;
  wait_for_draw();
  if (move_to == NULL || move_to[0] == '\0' ) return;
  path = gtk_tree_path_new_from_string (move_to);
  gtk_tree_view_scroll_to_cell (panel->list, path, NULL, TRUE, (gfloat)0.5, (gfloat)0.5);
  gtk_tree_view_set_cursor (panel->list, path, NULL, FALSE);
  gtk_tree_path_free(path);
  wait_for_draw();
}

void after_delete_update (struct_panel *panel)
{
  char *str_iter=get_current_iter(active_panel);
  TRACE("after_delete_update\n");
  update(panel);
  move_selection (str_iter, panel);
  free(str_iter);
}

void delete_dir_or_file (void)
{
  if (confirm_request(DELETE_CONFIRM, GTK_STOCK_DELETE, GTK_STOCK_CANCEL))
  {
    enable_refresh=FALSE;
    char *src;
    if (strncmp (active_panel->selected_name, "../", 3) == 0)
      return;
    asprintf (&src, "rm -f -r -R \"%s\"", active_panel->selected_name);
    xsystem(src);
    xfree (&src);
    after_delete_update (active_panel);
    if ((inactive_panel != NULL) && (strstr(inactive_panel->path, active_panel->path) != NULL))
    {
      after_delete_update (inactive_panel);
      (void)chdir(active_panel->path);
    }
    wait_for_draw();
    enable_refresh=TRUE;
  }
  e_ink_refresh_local();
}

void move_dir_or_file (void)
{
  if ( fm_toggle == FALSE) return;
  if ((move_toggle == FALSE) || ((move_toggle == TRUE) && confirm_request(MOVE_CONFIRM, MOVE, GTK_STOCK_CANCEL)))
  {
    enable_refresh=FALSE;
    char *src, *str_iter;
    if (strcmp (active_panel->selected_name, "../" ) == 0) return;
    asprintf (&src, "mv -f \"%s\" \"%s\"", active_panel->selected_name, inactive_panel->path);
    xsystem(src);
    xfree (&src);
    str_iter=get_current_iter(active_panel);
    update(active_panel);
    move_selection (str_iter, active_panel);
    free(str_iter);
    str_iter=get_current_iter(inactive_panel);
    update (inactive_panel);
    e_ink_refresh_local();
    move_selection (str_iter, inactive_panel);
    free(str_iter);
    wait_for_draw();
    enable_refresh=TRUE;
  }
  e_ink_refresh_full();
}

void copy_dir_or_file (void)
{
  if ( fm_toggle == FALSE) return;
  if (strcmp (active_panel->selected_name, "../") == 0) return;
  enable_refresh=FALSE;
  char *src, *str_iter;
  asprintf (&src, "cp -fpR \"%s\" \"%s\"", active_panel->selected_name, inactive_panel->path);
  xsystem(src);
  xfree (&src);
  str_iter=get_current_iter(inactive_panel);
  update(inactive_panel);
  move_selection (str_iter, inactive_panel);
  free(str_iter);
  (void)chdir (active_panel->path);
  wait_for_draw();
  enable_refresh=TRUE;
  e_ink_refresh_full();
}

void panel_selector (struct_panel *panel) /* Принимает указатель на panel - &top_panel, &bottom_panel или inactive_struct_panel */
{
  if (GTK_IS_WIDGET(GTK_WIDGET(panel->list))) /* Если таблица на которую мы собираемся переключиться существует - то фокуссируемся на ней */
  {
    gtk_widget_grab_focus (GTK_WIDGET(panel->list));
    update_window_title(panel);
  }
  else /* А иначе исходим из того, что верхняя панель существует всегда */
  {
    if (GTK_IS_WIDGET(GTK_WIDGET(top_panel.list)))
    {
      TRACE("Specified panel is not exist, it's bad - check Your code!\n");
      gtk_widget_grab_focus(GTK_WIDGET(top_panel.list));
      update_window_title(&top_panel);
    }
    else
    {
      TRACE("NO TABLES ARE FOUND! SOMETHING REALLY BAD HAPPENED!\n");
    }
  }
  wait_for_draw();
  e_ink_refresh_default();
}

void second_panel_show(void)
{
  char *iter;
  (void)chdir (bottom_panel.path);
  create_panel(&bottom_panel);
  iter=iter_from_filename (bottom_panel.selected_name, &bottom_panel);
  move_selection(iter, &bottom_panel); /* Восстанавливаем позицию указателя */
  free(iter);
  gtk_widget_show_all (main_window);
  (void)g_signal_connect_swapped (G_OBJECT (bottom_panel.table), "destroy", G_CALLBACK (panel_selector), &top_panel);
  wait_for_draw();
}

void init (void)
{
  #ifndef __amd64
  #ifdef debug
  const char *name="/media/mmcblk0p1/eView_debug_log.txt";
  TRACE("Trying to open '%s' for writing log\n", name);
  int file_descriptor=creat(name,O_CREAT|O_SYNC|O_TRUNC);
  if (file_descriptor < 0)
    TRACE("UNABLE TO OPEN %s FILE FOR WRITING!\n", name);
  else
  {
    TRACE("'%s' opened for writing log as %d fd, will now write it into this file!\n", name, file_descriptor);
    dup2 (file_descriptor, 1);
    dup2 (file_descriptor, 2);
  }
  #endif
  #endif
  /* Ранняя инициализация программы */
  detect_hardware();
  #ifdef debug
  set_led_state (LED_state[LED_ON]);
  xsystem("uname -a"); // Проверка машины:
  // GTK Ritmix RBK700HD:
  // Linux sibrary 2.6.24.2-Boeye #26 PREEMPT Sat Oct 22 11:30:10 CST 2011 armv5tejl unknown
  // Qt GMini M6HD:
  // Linux boeye 2.6.24.2-Boeye #346 PREEMPT Tue Jul 17 13:50:49 CST 2012 armv5tejl GNU/Linux
  #endif
  Display *disp = NULL;
  if (hw_platform != HW_PLATFORM_KOBO)
  {
    disp = XOpenDisplay(NULL); // Is painfully slow on Kobo
  }

  if (disp)
  {
    TRACE("X is up and running, assuming HW_PLATFORM_SIBRARY_GTK!\n");
    hw_platform = HW_PLATFORM_SIBRARY_GTK;
  }
  else
  {
    char *string, *message, *ROT;
    int timer=0;
    hw_platform = HW_PLATFORM_SIBRARY_QT;
    TRACE("X is down! Assuming QT\n");
    if (access("/etc/GTK_parts.version", F_OK))
      read_string("/home/root/.GTK_parts.version", &string);
    else
      read_string("/etc/GTK_parts.version", &string);
    if (atoi(string) < NEEDED_GTK_PARTS_VERSION)
    {
      asprintf(&message, GTK_PARTS_IS_OUTDATED, string, NEEDED_GTK_PARTS_VERSION);
      Qt_error_message(message);
      free(message); // Хотя этого уже никто не увидит...
    }

    TRACE("Trying to start Xfbdev\n");
    int pid=fork();
    if (!pid) /* Child process */
     {
      close(0);
      close(1);
      close(2);
      execlp("Xfbdev", ":0", "-br", "-pn", "-hide-cursor", "-dpi", "150", "-rgba", "vrgb", NULL); // Sibrary QT
      TRACE("execlp() call failed while trying to start Xfbdev");
      _exit(1);
    }
    else /* Parent process */
    {
      usleep (200000); /* Some sleep to allow X server to go online */
    }

    ROT=getenv("ROT"); // Получаем поворот экрана из переменной окружения
    while (!XOpenDisplay(NULL))
    {
      usleep (1000);
      if (++timer > 5000)
      {
        TRACE("Failed to start Xfbdev - timed out!\n");
        Qt_error_message(XFBDEV_STARTUP_TIMEOUT);
      }
    }
    TRACE("Xfbdev started after 0,%d seconds\n", timer);

    // Запускаем window manager - без него окно с изображением отображается поверх файл-менеджера и вокруг проглядывают файлы. А ещё он нужен для корректной работы xrandr.
    xsystem("matchbox-window-manager -theme Sato -use_desktop_mode decorated &"); 
    (void)usleep(2000000);
    
    TRACE("ROT=%s\n", ROT);
    if (strcmp (ROT, "0") == 0)
    {
      get_system_sleep_timeout();
      set_system_sleep_timeout("86400"); /* Боремся со злостным усыплятором */
    }
    if (strcmp (ROT, "90" ) == 0) xsystem("xrandr -d :0 -o left");
    if (strcmp (ROT, "180") == 0) xsystem("xrandr -d :0 -o inverted");
    if (strcmp (ROT, "270") == 0) xsystem("xrandr -d :0 -o right");

    get_screensavers_list();
  }
  current.name[0]='\0';
  preloaded.name[0]='\0';
  cached.name[0]='\0';
  screensaver.name[0]='\0';
  for (int page_number=0; page_number <= PAGE_RIGHT; page_number++)
  {
    current.pixbuf[page_number]=NULL;
    preloaded.pixbuf[page_number]=NULL;
    cached.pixbuf[page_number]=NULL;
    screensaver.pixbuf[page_number]=NULL;
  }
}

void shutdown(int exit_code) __attribute__((noreturn));
void shutdown(int exit_code)
{
  TRACE("Shutting down eView\n");
  if (top_panel.selected_name != NULL) write_config_string("top_panel.selected_name", top_panel.selected_name);
  if (bottom_panel.selected_name != NULL) write_config_string("bottom_panel.selected_name", bottom_panel.selected_name);
  (void)remove(top_panel.archive_list);
  (void)remove(bottom_panel.archive_list);
  set_brightness(previous_backlight_level);
  gtk_main_quit();
  if (hw_platform != HW_PLATFORM_SIBRARY_GTK)
  {
    TRACE("Shutting down Xfbdev\n");
    xsystem("killall Xfbdev");
    set_system_sleep_timeout(system_sleep_timeout);
  }
  TRACE("\n\neView shutudown done. Bye! =^_^=/~\n");
  exit (exit_code);
}

void start_sleep_timer(void)
{
  sleep_timer=sleep_timeout;
  if(pthread_create(&sleep_timer_tid, NULL, sleep_thread, NULL) != 0)
  {
    TRACE("Unable to start sleep timer thread!\n");
  }
}

void sigsegv_handler(void) __attribute__((noreturn));
void sigsegv_handler(void) /* Обработчик для вывода Backtrace сегфолта */
{
  #ifdef debug
  void *backtrace_buffer[1024];
  int depth=backtrace(backtrace_buffer, 1023);
  TRACE("got sigsegv_handler\n");
  TRACE("got backtrace of %d calls\n\nBacktrace:\n", depth);
  backtrace_symbols_fd(backtrace_buffer, depth, 2);
  #endif
  shutdown(EXIT_FAILURE);
}

int main (int argc, char **argv)
{
  GtkWidget *starting_message;
  signal(SIGSEGV, (__sighandler_t)sigsegv_handler);
  signal(SIGABRT, (__sighandler_t)sigsegv_handler);
  init();
  gtk_init (&argc, &argv);
  #ifdef __amd64
  width_display = 570 ;
  height_display = 762 ; /* Для отладки на ПК */
  #else /* -6 - ГРЯЗНЫЙ ХАК, потому как по умолчанию ViewImageWindow создаёт окно с рамкой в 3 пиксела вокруг картинки, так что она смещена на 3 пиксела вниз-вправо и 6 пикселов внизу-справа оказываются обрезаны. */
  GdkScreen *screen = gdk_screen_get_default(); /* Текущий screen */
  if(hardware_has_backlight)
  {
    width_display = gdk_screen_get_width (screen) - 1; /* Ширина экрана без менеджера окон */
    height_display = gdk_screen_get_height (screen) - 1; /* Высота экрана без менеджера окон*/
  }
  else
  {
    width_display = gdk_screen_get_width (screen) - 6; /* Ширина экрана с менеджером окон*/
    height_display = gdk_screen_get_height (screen) - 6; /* Высота экрана с менеджером окон*/
  }
  //   free(screen); // Это не надо (сегфолт на книге)
  framebuffer_descriptor = open("/dev/fb0", O_RDWR); /* Открываем фреймбуффер */
  if (framebuffer_descriptor == 0)
  {
    if (hw_platform == HW_PLATFORM_SIBRARY_GTK)
      Message(ERROR, FAILED_TO_OPEN_DEV_FB0);
    else
      Qt_error_message(FAILED_TO_OPEN_DEV_FB0);
  }
  #endif
  #ifdef debug
  char *directory=xgetcwd(NULL);
  TRACE("Starting eView in directory '%s'\n",directory);
  free (directory);
  #endif

  if (access(".eView/", F_OK) != 0) /* Действия когда каталог не существует:  */
  {
    create_cfg ();
    (void)chdir("/media/mmcblk0p1/"); /* Для новых книг */
    (void)chdir("/userdata/media/mmcblk0p1/"); /* Для старых книг */
    /* Неизвестно, где мы оказались после предыдущих двух переходов (сработал только один):  */
    top_panel.path = xgetcwd(top_panel.path);
    bottom_panel.path = xgetcwd(bottom_panel.path);
    write_config_string("top_panel.path", top_panel.path );
    write_config_string("bottom_panel.path", bottom_panel.path );
  }
  else
    read_configuration();

  starting_message=Message(EVIEW_IS_STARTING, PLEASE_WAIT);
  wait_for_draw();
  enable_refresh=FALSE;
  /*debug_msg_win (); //окно только для отладки */
  set_brightness(backlight);
  main_window = window_create(width_display, height_display, 0, VERSION, NOT_MODAL);
  (void)g_signal_connect (G_OBJECT (main_window), "destroy", G_CALLBACK (shutdown), NULL);
  panels_vbox = gtk_vbox_new (TRUE, 0);
  gtk_box_set_homogeneous (GTK_BOX (panels_vbox), FALSE);
  gtk_container_add (GTK_CONTAINER (main_window), panels_vbox);
  create_panel(&top_panel);

  if ( fm_toggle)
  {
    second_panel_show();
    if ( top_panel_active)
    {
      active_panel=&top_panel;
      inactive_panel=&bottom_panel;
    }
    else
    {
      active_panel=&bottom_panel;
      inactive_panel=&top_panel;
    }
  }
  else
  {
    active_panel=&top_panel;
    inactive_panel=NULL;
  }

  errno=0;
  if (chdir (active_panel->path) == -1) /* переход в последний рабочий каталог */
  {
    TRACE("Chdir to '%s' failed because %s!\n", active_panel->path, strerror(errno));
  }

  #ifndef __amd64
  if (show_clock == FALSE) /* Скрываем часики */
    gtk_window_fullscreen (GTK_WINDOW(main_window));
  #endif
  panel_selector (active_panel); /* Переключаемся в активную панель! */
  gtk_widget_show_all(main_window); /* Рисуем интерфейс */

  // Строим списки файлов в панелях
  while (active_panel->archive_depth > 0)
  {
    if (enter_archive(active_panel->archive_stack[active_panel->archive_depth], active_panel, FALSE)) 
      break;
    else
    {
      active_panel->archive_stack[active_panel->archive_depth][0] = '\0';
      active_panel->archive_depth--;
      if ( active_panel == &top_panel )
        write_archive_stack("top_panel.archive_stack", &top_panel);
      else
        write_archive_stack("bottom_panel.archive_stack", &bottom_panel);
    }
  }
  if (active_panel->archive_depth == 0)
    update(active_panel);

  select_file_by_name(active_panel->selected_name, active_panel);

  if (inactive_panel != NULL)
  {
    while (inactive_panel->archive_depth > 0)
    {
      if (enter_archive(inactive_panel->archive_stack[inactive_panel->archive_depth], inactive_panel, FALSE)) 
        break;
      else
      {
        inactive_panel->archive_stack[inactive_panel->archive_depth][0] = '\0';
        inactive_panel->archive_depth--;
        if ( inactive_panel == &top_panel )
          write_archive_stack("top_panel.archive_stack", &top_panel);
        else
          write_archive_stack("bottom_panel.archive_stack", &bottom_panel);
      }
    }
    if (inactive_panel->archive_depth == 0)
      update(inactive_panel);
    select_file_by_name(inactive_panel->selected_name, inactive_panel);
  }

  gtk_widget_destroy(starting_message);
  wait_for_draw();/* Ожидаем отрисовки всего */

  enable_refresh=TRUE;
  if (is_picture(active_panel->last_name)) /* Открываем последнюю отображённую картинку */
    ViewImageWindow (active_panel->last_name, active_panel, TRUE);
  else
    e_ink_refresh_full();
  if (hw_platform != HW_PLATFORM_SIBRARY_GTK)
  {
    preload_next_screensaver(); // Загружаем первую заставку в память для мгновенного отображения
    start_sleep_timer();
  }
  if (LED_notify)
    set_led_state (LED_state[LED_OFF]);
  interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
  gtk_main ();
  return 0;
}
