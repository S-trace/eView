/* Norin Maxim, 2011, Soul Trace, 2013, Based on  Mini Gtk-file-manager Copyright (C) 2002-2006
 * # * by Tito Ragusa <tito-wolit@tiscali.it>
 * #  Distributed under GPLv2 Terms*/

#define _GNU_SOURCE
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
#include <fcntl.h> //open()
#include <X11/Xlib.h> //XOpenDisplay()

#include "gtk_file_manager.h" // Инклюдить первой среди своих, ибо typedef panel!
#include "mylib.h"
#include "mygtk.h"
#include "cfg.h"
#include "digma_hw.h"
#include "ViewImageWindow.h"
#include "debug_msg_win.h"
#include "archive_handler.h"
#include "translations.h"
#include "interface.h"

int interface_is_locked=TRUE; // Сразу блокируем интерфейс программы, чтобы при запуске она не падала если зажата клавиша
panel top_panel, bottom_panel, *active_panel, *inactive_panel;
GtkWidget *main_window, *panels_vbox; // Окно файлменеджера
static int table_visible; //видима нижняя панель или нет
int width_display, height_display;
int framebuffer_descriptor=0; // Дескриптор файла фреймбуффера (для обновления)
int QT=FALSE; // Обнаружен ли QT (влияет на IOCTL обновления и запуск Xfbdev)

void wait_state(void) // Возврат после смотрелки
{
  update(active_panel);
  gtk_widget_show_all(main_window);
  active_panel->selected_name=basename(active_panel->selected_name); // Бля! >_<
  move_selection(iter_from_filename (active_panel->selected_name, active_panel), active_panel);
  gtk_widget_queue_draw(GTK_WIDGET(active_panel->list)); // Заставляем GTK перерисовать список каталогов
  //   g_signal_connect (G_OBJECT (window), "focus_in_event",
  //                     G_CALLBACK (focus_in_callback), NULL);
  //   g_signal_connect (G_OBJECT (window), "focus_out_event",
  //                     G_CALLBACK (focus_out_callback), NULL);
  //   printf ("FMAN CONNECTED!\n");
}

void list_fd(panel *panel) //добавление списка имен каталогов, файлов и их размеров в панель panel
{
  int i = 0, n;
  char *fsize, *text;
  panel->files_num = 0;
  panel->dirs_num=0;
  if (panel->archive_depth > 0) // Поведение в архиве
  {
    char  **namelist, *back="../", *text, *full_name;
    add_data_to_list(panel->list, &back, 1, NO_AUTOSCROLL, "dir");
    panel->dirs_num++;
    namelist=archive_get_directories_list(panel, panel->archive_cwd);
    if ( namelist[0] != NULL)
    {
      while( namelist[i] != 0 && namelist[i][0] != '\0' && GTK_IS_WIDGET(main_window) )
      {
        if (!show_hidden_files && namelist[i][0] == '.') {continue;}
        panel->dirs_num++;
        text=strdup(namelist[i]);
        trim_line(text); // Ампутируем последний слэш
        text=basename(text);
        full_name=xconcat(text,"/");
        #ifdef debug_printf
        printf("Adding %d '%s'\n", i, text);
        #endif
        add_data_to_list(panel->list, &full_name, 1, NO_AUTOSCROLL, "dir");
        xfree(&namelist[i]);
        i++;
      }
      xfree(&namelist);
    }
    
    namelist=archive_get_files_list(panel, panel->archive_cwd);
    if ( namelist[0] != NULL)
    {
      i=0; // Сбрасываем счётчик, иначе карается сегфолтом!
      while(namelist[i][0] != '\0' && GTK_IS_WIDGET(main_window) ) // Пока имя файла не вырождается в пустую строку (грёбаный греп с его повадками!)
      {
        if (!show_hidden_files && namelist[i][0] == '.') {continue;}
        text=basename(namelist[i]);
        add_data_to_list(panel->list, &text, 1, NO_AUTOSCROLL, "file");
        panel->files_num++;
        xfree (&namelist[i]);
        i++;
      }
      xfree(&namelist);
    }
  }
  else // Поведение в ФС
  {    
    struct dirent  **namelist;
    if (panel->path == NULL) return;
    if ((n = scandir(panel->path, &namelist, 0, versionsort)) >= 0)
    {
      for( i = 0; i < n && GTK_IS_WIDGET(main_window); i++ ) // Первый обход списка - каталоги
      {
        struct stat stat_p;
        //убрать точку из списка папок
        if (strcmp(namelist[i]->d_name, ".") == 0) {continue;}
        //убрать двоеточку из списка папок при присутствии в корневом каталоге
        if (strcmp(namelist[i]->d_name, "..") == 0 && strcmp (panel->path, "/") == 0) {continue;}
        //Убрать скрытные каталоги
        if (!show_hidden_files && namelist[i]->d_name[0] == '.' && strcmp(namelist[i]->d_name, "..") != 0) {continue;}
        stat(namelist[i]->d_name, &stat_p);
        if (S_ISDIR(stat_p.st_mode)) 
        { 
          text = xconcat_path_file(namelist[i]->d_name, "");
          panel->dirs_num++;
          add_data_to_list(panel->list, &text, 1, NO_AUTOSCROLL, "dir");
          xfree(&text);
        }
      }
      for( i = 0; i < n && GTK_IS_WIDGET(main_window); i++ )  // Второй обход списка - файлы
      {
        struct stat stat_p;
        //Убрать скрытные файлы
        if (!show_hidden_files && namelist[i]->d_name[0] == '.') {continue;}
        text = namelist[i]->d_name;
        #ifdef debug_printf
        printf("Adding %d '%s'\n", i, text);
        #endif
        panel->files_num++;
        stat(namelist[i]->d_name, &stat_p);
        if (!S_ISDIR(stat_p.st_mode)) 
        { 
          fsize = get_natural_size(stat_p.st_size); //размер файла
          add_data_to_list(panel->list, &text, 1, NO_AUTOSCROLL, fsize);
        }
        xfree(&namelist[i]);
      }
    }
    //     xfree(&namelist);
  }
}

char *iter_from_filename (char *fname, panel *panel) //возвращает итератор (номер) файла с именем fname из списка в панели panel
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  gboolean valid = TRUE;
  char *tmp;
  char *inlist;
  
  model = gtk_tree_view_get_model (panel->list);
  gtk_tree_model_get_iter_first (model, &iter);
  if (iter.stamp != 0)
  {
    while (valid) {
      gtk_tree_model_get (model, &iter, FILE_COLUMN, &tmp, -1);
      inlist = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
      xfree(&tmp);
      if (strcmp (inlist, fname) == 0){
        inlist =  gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL(model), &iter);
        return inlist;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
    }
  }
  else
  {
    #ifdef debug_printf
    printf("Iter stamp is 0, something bad happened!\n");
    #endif
  }
  return "0";
}

void update(panel *panel) //обновление списка
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  gboolean valid = TRUE;
  char *title;
  set_led_state (LED_BLINK_FAST); // Индикация активности
  panel->files_num=0; // Обнуляем число файлов в просмотрщике
  clear_list(panel->list);
  #ifdef debug_printf
  if (panel->archive_depth > 0)
  {
    printf ("We are now IN ARCHIVE!\n");
  }
  #endif
  list_fd(panel);
  
  // подсчет числа строк в листе папок и файлов
  model = gtk_tree_view_get_model (panel->list);
  gtk_tree_model_get_iter_first (model, &iter);
  while (valid) 
  {
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  //инфа о числе папок и файлов в загловок окна
  asprintf(&title, "Dirs: %d  Files: %d %s", panel->dirs_num-1, panel->files_num, "    "VERSION);
  gtk_window_set_title(GTK_WINDOW(main_window), title);
  xfree (&title);
  if (panel->archive_depth > 0)
  {
    gtk_label_set_text (GTK_LABEL(panel->path_label), xconcat_path_file(panel->archive_stack[panel->archive_depth], panel->archive_cwd)); // Пишем имя архива с путём в поле снизу
  }
  else
  {
    gtk_label_set_text (GTK_LABEL(panel->path_label), panel->path);
  }
  move_selection(iter_from_filename (panel->selected_name, panel), panel);
  set_led_state (LED_OFF); // Индикация активности
}

void move_selection(char *move_to, panel *panel) // сдвигает курсор на заданную строку в символьном виде
{
  wait_for_draw();
  if (move_to[0] == '\0') move_to="0";
  GtkTreePath *path = gtk_tree_path_new_from_string (move_to);
  gtk_tree_view_scroll_to_cell (panel->list, path, NULL, TRUE, 0.5, 0.5);
  gtk_tree_view_set_cursor (panel->list, path, NULL, FALSE);
  wait_for_draw();
}

void second_panel_hide(void)
{
  #ifdef debug_printf
  printf("hiding second_panel_hide\n");
  #endif
  gtk_widget_destroy (bottom_panel.table);
  gtk_widget_destroy (bottom_panel.path_label);
//   table_visible = 0;
  top_panel_active = 1;
  active_panel=&top_panel;
  inactive_panel=NULL;
  panel_selector(active_panel);
  write_config_string("bottom_panel.selected_name", bottom_panel.selected_name);
  write_config_int ("top_panel_active", top_panel_active);
  e_ink_refresh_local();
}

void menu_destroy (GtkWidget *dialog)
{
  #ifdef debug_printf
  printf("Destroying menu\n");
  #endif
  gtk_widget_destroy(dialog);
  gtk_widget_grab_focus (GTK_WIDGET(active_panel->list));
  //   g_signal_handlers_unblock_by_func( window, focus_in_callback, NULL );
  //   g_signal_handlers_unblock_by_func( window, focus_out_callback, NULL );
}

void after_delete_update (panel *panel)
{
  #ifdef debug_printf
  printf ("after_delete_update\n");
  #endif
  char *str_iter=get_current_iter(active_panel);
  update(panel);
  move_selection (str_iter, panel);
  e_ink_refresh_local();
}

void delete_dir_or_file (void)
{
  if (confirm_request(DELETE_CONFIRM, GTK_STOCK_DELETE, GTK_STOCK_CANCEL))
  {
    if (!strncmp (active_panel->selected_name, "../", 3)) 
      return;
    char *src;
    asprintf (&src, "rm -f -r -R \"%s\"", active_panel->selected_name);
    xsystem(src);
    xfree (&src);
    after_delete_update (active_panel);
    if (inactive_panel != NULL && !strcmp (active_panel->path, inactive_panel->path))
    {
      after_delete_update (inactive_panel);
      chdir(active_panel->path);
    }
    e_ink_refresh_local();
  }
}

void move_dir_or_file (void)
{
  if (!table_visible) return;
  if ((!move_toggle) || (move_toggle && confirm_request(MOVE_CONFIRM, MOVE, GTK_STOCK_CANCEL)))
  {
    if (!strcmp (active_panel->selected_name, "../")) return;
    char *src, *str_iter;
    asprintf (&src, "mv -f \"%s\" \"%s\"", active_panel->selected_name, inactive_panel->path);
    xsystem(src);
    xfree (&src);
    str_iter=get_current_iter(active_panel);
    update(active_panel);
    move_selection (str_iter, active_panel);
    str_iter=get_current_iter(inactive_panel);
    update (inactive_panel);
    e_ink_refresh_local();
    move_selection (str_iter, inactive_panel);
  }
}

void copy_dir_or_file (void)
{
  if (!strcmp (active_panel->selected_name, "../")) return;
  if (table_visible)
  {
    char *src, *str_iter;
    asprintf (&src, "cp -fpR \"%s\" \"%s\"", active_panel->selected_name, inactive_panel->path);
    xsystem(src);
    xfree (&src);
    str_iter=get_current_iter(inactive_panel);
    update(inactive_panel);
    move_selection (str_iter, inactive_panel);
    chdir (active_panel->path);
  }
}

void panel_selector (panel *focus_to) // Принимает указатель на panel - &top_panel, &bottom_panel или inactive_panel
{
  if GTK_IS_WIDGET(GTK_WIDGET(focus_to->list)) // Если таблица на которую мы собираемся переключиться существует
  {
    gtk_widget_grab_focus (GTK_WIDGET(focus_to->list)); // То фокуссируемся на ней
  }
  else // А иначе исходим из того, что верхняя панель существует всегда
  {
    if GTK_IS_WIDGET(GTK_WIDGET(top_panel.list))
    {
      #ifdef debug_printf
      printf ("Specified panel is not exist, it's bad - check Your code!\n");
      #endif
      gtk_widget_grab_focus(GTK_WIDGET(top_panel.list)); 
    }
    else
    {
      #ifdef debug_printf
      printf ("NO TABLES ARE FOUND! SOMETHING REALLY BAD HAPPENED!\n");
      #endif
    }
  }
}

void second_panel_show(void)
{
  chdir (bottom_panel.path);
  create_panel(&bottom_panel);
  move_selection(iter_from_filename (bottom_panel.selected_name, &bottom_panel), &bottom_panel); // Восстанавливаем позицию указателя
  table_visible = 1;
  gtk_widget_show_all (main_window);
  g_signal_connect_swapped (G_OBJECT (bottom_panel.table), "destroy", G_CALLBACK (panel_selector), &top_panel);
  e_ink_refresh_local();
}

void init (void)
{
  #ifndef __amd64
  #ifdef debug_printf
  char *name="/media/mmcblk0p1/eView_debug_log.txt";
  printf("Trying to open '%s' for writing log\n", name);
  int file_descriptor=creat(name,O_CREAT|O_SYNC|O_TRUNC);
  if (file_descriptor < 0)
    printf("UNABLE TO OPEN %s FILE FOR WRITING!\n", name);
  else
  {
    printf("'%s' opened for writing log as %d fd, will now write it into this file!\n", name, file_descriptor);
    dup2 (file_descriptor, 0);
    dup2 (file_descriptor, 1);
    dup2 (file_descriptor, 2);
  }
  #endif
  #endif
  // Ранняя инициализация программы
  detect_hardware();
  #ifndef __amd64
  char *string, *message;
  read_string("/home/root/.GTK_parts.version", &string);
  if (atoi(string) < NEEDED_GTK_PARTS_VERSION)
  {
    asprintf(&message, GTK_PARTS_IS_OUTDATED, string, NEEDED_GTK_PARTS_VERSION);
    Qt_error_message(message);
  }
  set_led_state (LED_ON);
  if (XOpenDisplay(NULL))
  {
    #ifdef debug_printf
    printf ("X is up and running!\n");
    #endif
  }
  else
  {
    QT=TRUE;
    #ifdef debug_printf
    printf ("X is down! Assuming QT\nTrying to start Xfbdev\n");
    #endif
    xsystem("Xfbdev :0 -br -pn -hide-cursor -dpi 150 -rgba vrgb & ");
    usleep(1000000);
    xsystem("matchbox-window-manager -theme Sato -use_desktop_mode decorated &");
    if (!hardware_has_backlight)
    {
      usleep(2000000);
      xsystem("xrandr -d :0 -o left");
    }
//     usleep(3000000);
    if (! XOpenDisplay(NULL))
    {
      #ifdef debug_printf
      printf ("Failed to start Xfbdev!\n");
      #endif
      Qt_error_message(FAILED_TO_START_XFBDEV);
    }
    
  }
  #endif
}

void shutdown(void)
{
  #ifdef debug_printf
  printf("Shutting down eView\n");
  #endif
  if (top_panel.selected_name != NULL) write_config_string("top_panel.selected_name", top_panel.selected_name);
  if (bottom_panel.selected_name != NULL) write_config_string("bottom_panel.selected_name", bottom_panel.selected_name);
  remove(top_panel.archive_list);
  remove(bottom_panel.archive_list);
  gtk_main_quit();
  if (QT)
  {
    #ifdef debug_printf
    printf("Shutting down Xfbdev\n");
    #endif
    xsystem("killall Xfbdev");
  }
  #ifdef debug_printf
  printf("\n\neView shutudown done. Bye! =^_^=/~\n");
  #endif
}

int main (int argc, char **argv)
{
  init();
  gtk_init (&argc, &argv);
  set_led_state (LED_BLINK_SLOW);
  #ifdef __amd64
  width_display = 570 ;
  height_display = 762 ; // Для отладки на ПК
  #else // -6 - ГРЯЗНЫЙ ХАК, потому как по умолчанию ViewImageWindow создаёт окно с рамкой в 3 пиксела вокруг картинки, так что она смещена на 3 пиксела вниз-вправо и 6 пикселов внизу-справа оказываются обрезаны.
  GdkScreen *screen;
  screen = gdk_screen_get_default(); // Текущий screen
  width_display = gdk_screen_get_width (screen) - 6; // Ширина экрана
  height_display = gdk_screen_get_height (screen) - 6; // Высота экрана
  framebuffer_descriptor = open("/dev/fb0", O_RDWR); // Открываем фреймбуффер
  if (framebuffer_descriptor == 0)
  {
    if (!QT)
    {
      Message(ERROR, FAILED_TO_OPEN_DEV_FB0);
    }
    else
    {
      Qt_error_message(FAILED_TO_OPEN_DEV_FB0);
    }
  }
  #endif
  #ifdef debug_printf
  printf ("Starting eView in directory '%s'\n", xgetcwd(NULL));
  #endif
  
  Message(EVIEW_IS_STARTING, PLEASE_WAIT);
  top_panel.path = cfg_file_path (); // Получаем месторасположения конфига (текущий каталог в момент запуска) - на книге это /home/root/
  FILE *fp;
  
  if ((fp = fopen (top_panel.path, "rb"))==NULL) // Действия когда каталог не существует: 
  {
    create_cfg ();
    read_configuration();
    chdir("/media/mmcblk0p1/");
    top_panel.path = xgetcwd(top_panel.path);
    write_config_string("top_panel.path", top_panel.path);
  }
  else 
  {
    fclose(fp);
    read_configuration();
  }
  //debug_msg_win (); //окно только для отладки
  
  main_window = window_create(width_display, height_display, 0, VERSION, NOT_MODAL);
  g_signal_connect (G_OBJECT (main_window), "destroy", G_CALLBACK (shutdown), NULL);
  panels_vbox = gtk_vbox_new (TRUE, 0);
  gtk_box_set_homogeneous (GTK_BOX (panels_vbox), FALSE);
  gtk_container_add (GTK_CONTAINER (main_window), panels_vbox);
  create_panel(&top_panel);
  if (top_panel.archive_depth > 0 )
  {
    enable_refresh=0;
  }
  else
  {
    update(&top_panel);
  }
  
  if ( fm_toggle == 1 )
  {
    second_panel_show();
    if ( top_panel_active == 1 )
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
  
  if (! chdir (active_panel->path)) // переход в последний рабочий каталог
  {
    #ifdef debug_printf
    printf ("Chdir to '%s' failed because %s!\n", active_panel->path, strerror(errno));
    #endif
  }  
  if (active_panel->archive_depth > 0 || (inactive_panel != NULL && inactive_panel->archive_depth > 0) )
  {
    enable_refresh=0;
    if ( active_panel->archive_depth > 0 )
    {
      enter_archive(active_panel->archive_stack[active_panel->archive_depth], active_panel, FALSE);
      move_selection(iter_from_filename (active_panel->selected_name, active_panel), active_panel); // Восстанавливаем состояние выбранных элементов в списке файлов
    }
    if ( inactive_panel != NULL && inactive_panel->archive_depth > 0 )
    {
      enter_archive(inactive_panel->archive_stack[inactive_panel->archive_depth], inactive_panel, FALSE);
      move_selection(iter_from_filename (inactive_panel->selected_name, inactive_panel), inactive_panel); // Восстанавливаем состояние выбранных элементов в списке файлов
    }
    enable_refresh=1;
  }
  update(active_panel); // Наполняем список каталогов
  #ifndef __amd64
  if (clock_toggle) {
    gtk_window_unfullscreen  (GTK_WINDOW(main_window)); // Показываем часики
  } else {
    gtk_window_fullscreen  (GTK_WINDOW(main_window)); // Скрываем их
  }
  #endif
  panel_selector (active_panel); // Переключаемся в активную панель!
  gtk_widget_destroy(MessageWindow);
  gtk_widget_show_all(main_window); // Рисуем интерфейс
  enable_refresh=0;
  wait_for_draw();// Ожидаем отрисовки всего
  enable_refresh=1;
  if (is_picture(active_panel->last_name) )
  {
    ViewImageWindow (active_panel->last_name, active_panel); // Открываем последнюю отображённую картинку
  }
  else
  {
    e_ink_refresh_full();
  }
  //   g_signal_connect (G_OBJECT (window), "show", G_CALLBACK (e_ink_refresh_full), NULL);
  //   g_signal_connect_after (current_panel->list, "move_cursor", G_CALLBACK (e_ink_refresh_default), NULL ); // Обновление экрана при сдвиге выделения
  interface_is_locked=FALSE; // Снимаем блокировку интерфейса
  set_led_state (LED_OFF);
  gtk_main ();
  return 0;
}