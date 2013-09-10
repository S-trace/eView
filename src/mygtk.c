/* modification 2011, Soul Trace, 2013, Based on  Mini Gtk-file-manager Copyright (C) 2002-2006
 * # ******************************************************************  by Tito Ragusa <tito-wolit@tiscali.it>
 * #  Distributed under GPLv2 Terms
 * #  Gtk+2.x Utility routines: Base widgets*/
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdlib.h> /*system()*/
#include <unistd.h>  /*chdir()*/
#include <pthread.h>

#include "gtk_file_manager.h" /*update()*/
#include "cfg.h"
#include "mygtk.h"
#include "mylib.h"
#include "digma_hw.h"
#include "ViewImageWindow.h" /*reset_preloaded_image*/
#include "os-specific.h"
#include "translations.h"
#include "archive_handler.h"
#include "interface.h"

int enable_refresh=1;
int suspend_count=-1; /* Счётчик засыпаний книги - для выбора номера заставки */
static int need_full_refresh; /* Тип необходимого обновления экрана при перемещении курсора по меню */
static guint MessageDie_idle_call_handler;

int check_key_press(guint keyval, struct_panel *panel) /* Возвращает TRUE если всё сделано */
{
  if (interface_is_locked)
  {
    #ifdef debug_printf
    printf("Interface was locked, keypress ignored!\n");
    #endif
    return TRUE;
  }
  if (suspended)
  {
    if (keyval == KEY_POWER_QT) /* Выход из сна */
    {
      pthread_cancel(suspend_helper_tid); // Принудительно завершаем работу потока, который усыпляет книгу (если он сам не завершился ещё)
      if(was_in_picture_viewer)
      {
        (void)show_image(&current, panel, FALSE, current_page);
        e_ink_refresh_full();
      }
      else
        die_viewer_window();
      was_in_picture_viewer=suspended=FALSE;
      set_brightness(backlight);
      sleep_timer=sleep_timeout;
      return TRUE;
    }
    else /* Продолжаем спать */
    {
      #ifdef debug_printf
      printf("Program is suspended, keypress ignored!\n");
      #endif
      suspend_hardware();
    }
    return TRUE;
  }
  else /* Если не спим */
  {
    if (keyval == KEY_POWER_QT)
    {
      enter_suspend(panel);
      return TRUE;
    }
    else
    {
      sleep_timer=sleep_timeout; /* При любом нажатии клавиши (где угодно) сбрасываем таймер сна на значение из настроек */
      return FALSE;
    }
  }
}

int keys_in_confitm_request(__attribute__((unused)) GtkWidget *dialog, GdkEventKey *event)
{
  if (check_key_press(event->keyval, NULL)) return TRUE;
  switch (event->keyval)
  {
    case   KEY_OK:
      return FALSE;
      
    default:
      e_ink_refresh_default();
      return FALSE;
  }
}

gboolean confirm_request(const char *title, const char *confirm_button, const char *reject_button)
{
  GtkWidget *dialog;
  int answer;
  gint result;
  dialog = gtk_dialog_new_with_buttons (title, NULL,
                                        GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, reject_button, GTK_RESPONSE_REJECT, confirm_button, GTK_RESPONSE_ACCEPT, NULL);
  gtk_dialog_set_default_response (GTK_DIALOG(dialog),GTK_RESPONSE_REJECT);
  gtk_widget_show_all (dialog);
  wait_for_draw();
  e_ink_refresh_local();
  (void)g_signal_connect (G_OBJECT (dialog), "key-press-event", G_CALLBACK (keys_in_confitm_request), NULL);
  result = gtk_dialog_run (GTK_DIALOG (dialog));
  switch (result)
  {
    case GTK_RESPONSE_ACCEPT:
      answer=TRUE;
      break;
    default:
      answer=FALSE;
      break;
  }
  gtk_widget_destroy (dialog);
  return answer;
}

/* char focus_in_processed; // Количество обработанных сигналов 
 gint focus_in_callback () // Обработка события получения фокуса окном. Теоретически должна выстреливать только при реальной потере-получении фокуса окном (извне)! 
 { 
   #ifdef debug_printf 
   printf ("GOT FOCUS_IN\n"); 
   #endif 
   if (focus_in_processed == 1) 
   { 
     e_ink_refresh_local(); 
     focus_in_processed = -1; 
   } 
   else 
   { 
     focus_in_processed++; 
   } 
   return TRUE; 
 } 
  
 gint focus_out_callback (void) // реакция на потерю фокуса 
 { 
   #ifdef debug_printf 
   printf ("GOT FOCUS_OUT\n"); 
   #endif 
   focus_in_processed = 0; // Чтобы не выстреливать focus_in_callback когда попало 
   return FALSE; 
 } */
/*           asprintf(&command, "dbus-send  --type=method_call --dest=com.test.reader /reader/registry com.test.reader.registry.input string:\"%s%s\"",panel->path, panel->selected_name);  */
/* signal sender=:1.1 -> dest=(null destination) serial=131 path=/PowerManager; interface=com.sibrary.Service.PowerManager; member=requestSuspend */

/* DBUS-вызовы на Qt прошивке:
 * method call sender=:1.3 -> dest=com.sibrary.BoeyeServer serial=47 path=/StartupSplash; interface=com.sibrary.Service.StartupSplash; member=closeSplash
 * dbus-send --type=method_call --dest=com.sibrary.BoeyeServer /StartupSplash com.sibrary.Service.StartupSplash.showSplash
 * dbus-send --type=method_call --dest=com.sibrary.BoeyeServer /StartupSplash com.sibrary.Service.StartupSplash.closeSplash
 *
 * dbus-send --type=method_call --dest=com.test.reader /reader/registry com.test.reader.registry.input string:"/path/file.txt"
 *
 * dbus-send --print-reply --type=method_call --dest=com.sibrary.BoeyeServer /PowerManager com.sibrary.Service.PowerManager.getSuspendTime|cut -d ' ' -f 5|tail -n 1
 * dbus-send --print-reply --type=method_call --dest=com.sibrary.BoeyeServer /PowerManager com.sibrary.Service.PowerManager.setSuspendTime int32:1024
 *
 * cat /home/root/Settings/boeye/boeyeserver.conf|grep ScreenSaver | cut -d = -f 2|tr -d ' '| tr ',' '\n' - список файлов для заставки
 *
 * method call sender=:1.2 -> dest=com.test.reader serial=113 path=/reader/registry; interface=com.test.reader.registry; member=input ; string "/media/mmcblk0p1/passwd.txt"
 *
 * dbus-send /PowerManager com.sibrary.Service.PowerManager.requestSuspend
 * signal sender=:1.1  -> dest=(null destination) serial=151 path=/PowerManager; interface=com.sibrary.Service.PowerManager; member=requestSuspend
 *
 */

void Qt_error_message(const char *message)
{
  const char *name="/tmp/eView_error_message.txt";
  #ifdef debug_printf
  printf("writing %s to %s\n", message, name);
  #endif
  FILE *file_descriptor=fopen(name,"wt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN %s FILE FOR WRITING!\n", name);
    #endif
    shutdown(EXIT_FAILURE);
    return ;
  }
  else
  {
    fprintf(file_descriptor, "%s", message);
    (void)fclose(file_descriptor);
    xsystem("dbus-send --type=method_call --dest=com.test.reader /reader/registry com.test.reader.registry.input string:\"/tmp/eView_error_message.txt\"");
    shutdown(EXIT_FAILURE);
  }
}

int MessageDie (GtkWidget *Window)
{
  char *iter;
  gtk_idle_remove (MessageDie_idle_call_handler); /* Удаляем вызов этой функции из очереди вызовов (иначе она будет вызываться вечно) */
  iter=iter_from_filename(active_panel->selected_name, active_panel);
  #ifdef debug_printf
  printf ("Destroying message window\n");
  #endif
  gtk_widget_destroy(Window);
  move_selection(iter, active_panel);
  free(iter);
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  if (interface_is_locked)
    interface_is_locked=FALSE;
  else
    e_ink_refresh_full();
//   interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
  return TRUE;
}

void *MessageDieDelayed (void *window)
{
  (void)usleep(2000000); /* Спим 2 секунды */
  MessageDie_idle_call_handler=g_idle_add ((GSourceFunc) MessageDie, GTK_WIDGET(window));
  return NULL;
}

GtkWidget *Message (const char *const title, const char *const message) 
{
  GtkWidget *MessageWindow, *label;
  /*   interface_is_locked=TRUE; //Блокируем остальной интерфейс программы */
  /* Создаём виджеты */
  #ifdef debug_printf
  printf ("Show message '%s', data: '%s'\n", title, message);
  #endif
  MessageWindow = gtk_dialog_new_with_buttons (title, NULL, GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT |GTK_DIALOG_NO_SEPARATOR, NULL);
  gtk_window_set_position (GTK_WINDOW (MessageWindow), GTK_WIN_POS_CENTER_ALWAYS);
  label = gtk_label_new (message);
  gtk_label_set_justify (GTK_LABEL (label),GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap(GTK_LABEL (label), TRUE);
  /* Гарантирует закрытие диалога когда пользователь ответил. */
  (void)g_signal_connect (G_OBJECT (MessageWindow), "key_press_event", G_CALLBACK (MessageDie), MessageWindow);
  /* Добавляет ярлык и отображает всё что мы добавили к диалогу. */
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(MessageWindow)->vbox), label);
  gtk_widget_show_all (MessageWindow);
  if (!QT) usleep(GTK_REFRESH_DELAY*2);
  e_ink_refresh_full();
  return MessageWindow;
}

void wait_for_draw (void)
{
  while (gtk_events_pending ())    (void)gtk_main_iteration ();
}

char *get_current_iter (struct_panel *panel) /*возвращает итератор текущего файла из списка */
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  char *inlist;
  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection (panel->list);
  model = gtk_tree_view_get_model (panel->list);
  (void)gtk_tree_selection_get_selected (selection, &model, &iter);
  //   free(selection); // Не надо - карается abort()ом
  inlist =  gtk_tree_model_get_string_from_iter (model, &iter);
  if (inlist == NULL)
    return (strdup("0"));
  else
    return (inlist);
}

void e_ink_refresh_part(void)
{
  #ifdef __amd64
  printf("Updating eINK (part)\n");
  #endif
  wait_for_draw();
  epaperUpdatePart();
}

void e_ink_refresh_local(void)
{
  #ifdef __amd64
  printf("Updating eINK (local)\n");
  #endif
  wait_for_draw();
  epaperUpdateLocal();
}

void e_ink_refresh_full(void)
{
  #ifdef __amd64
  printf("Updating eINK (full)\n");
  #endif
  wait_for_draw();
  epaperUpdateFull();
}

void e_ink_refresh_default(void) /* Рефреш экрана по умолчанию (из настроек) */
{
  if (speed_toggle)
    e_ink_refresh_local();
  else
    e_ink_refresh_part ();
}

void enter_subdir(char *name, struct_panel *panel)/* Переход на уровень вниз в дереве panel->list */
{
  enable_refresh=FALSE;
  if (panel->archive_depth > 0) /* Если мы в архиве - дёргаем архивную функцию */
    archive_enter_subdir (name, panel);
  else
  {
    char *path=xconcat_path_file(panel->path, name);
    (void)chdir (path);
    free (panel->path);
    panel->path=strdup(path);
    free (path);
    update(panel);
    move_selection("0", panel);
    if (panel == &top_panel)
    {
      write_config_string("top_panel.last_name", ""); /* Сбрасываем имя картинки (она всё равно не существует в новом каталоге), чтобы при запуске не было ошибки */
      write_config_string("top_panel.path", top_panel.path);
    }
    else
    {
      write_config_string("bottom_panel.last_name", "");
      write_config_string("bottom_panel.path", bottom_panel.path);
    }
  }
  gtk_widget_queue_draw(GTK_WIDGET(panel->list)); /* Заставляем GTK перерисовать список каталогов */
  enable_refresh=TRUE;
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_full(); /*А иначе - грязь на экране */
}

void dirlist_select(__attribute__((unused)) GtkWidget *const widget, struct_panel *const panel) /* Что происходит при перемещении выделенной строки по списку */
{
  char *tmp;
  GtkTreeIter iter;
  GtkTreeModel *model;
  GtkTreeSelection *selection = gtk_tree_view_get_selection (panel->list);
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    GtkTreePath *start_path, *end_path, *path;
    char *selection_row;
    gtk_tree_model_get (model, &iter, FILE_COLUMN , &tmp, -1);
    free(panel->selected_name);
    panel->selected_name = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    gtk_tree_model_get (model, &iter, SIZE_COLUMN , &tmp, -1);
    free(panel->selected_size);
    panel->selected_size = g_locale_from_utf8(tmp, -1, NULL, NULL, NULL);
    xfree(&tmp);
    free(panel->selected_iter);
    panel->selected_iter = gtk_tree_model_get_string_from_iter (model, &iter);
    
    /*поведение прокрутки при подходах к краю окна */
    selection_row = iter_from_filename (panel->selected_name, panel);
    path = gtk_tree_path_new_from_string (selection_row);
    if (gtk_tree_view_get_visible_range (panel->list, &start_path, &end_path))
    {
      char *path_string=gtk_tree_path_to_string (end_path);
      if (strcmp(path_string, selection_row) == 0) // При подходе к нижнему краю окна
      {
        gtk_tree_view_scroll_to_cell (panel->list, path, NULL, TRUE, (gfloat)0.0, (gfloat)0.0);
        need_full_refresh=TRUE;
      }
      free(path_string);

      path_string=gtk_tree_path_to_string (start_path);
      if (strcmp(path_string, selection_row) == 0) // При подходе к верхнему краю окна
      {
        gtk_tree_view_scroll_to_cell (panel->list, path, NULL, TRUE, (gfloat)1.0, (gfloat)1.0);
        need_full_refresh=TRUE;
      }
      free(path_string);
      gtk_tree_path_free(start_path);
      gtk_tree_path_free(end_path);
    }
    gtk_tree_path_free(path);
    free(selection_row);
  }
  //     free(selection); // Не надо - карается abort()ом
}

void after_dirlist_select(__attribute__((unused)) GtkWidget *const widget, struct_panel *const panel) /* Что происходит при перемещении выделенной строки по списку (часть 2) */
{
  if (interface_is_locked) /* Чтобы не дёргалось при просмотре изображений (каждый раз, приводя к двойному обновлению) */
    return;
  
  if (need_full_refresh)
  {
    wait_for_draw();
    gtk_widget_queue_draw(GTK_WIDGET(panel->list)); /* Заставляем GTK перерисовать список каталогов */
    wait_for_draw();
    if (!QT) usleep(GTK_REFRESH_DELAY);
    e_ink_refresh_full();
    need_full_refresh=FALSE;
  }
  else
    e_ink_refresh_default();
}

void panel_focussed(struct_panel *panel)
{
  if (interface_is_locked) /* Чтобы игнорировать сигнал о фокуссировке на панели во время закрытия смотрелки */
  {
    #ifdef debug_printf
    printf("Interface was locked, panel focus change signal ignored!\n");
    #endif
    return;
  }
  if (panel == &top_panel)
  {
    active_panel=&top_panel;
    inactive_panel=&bottom_panel;
    (void)chdir(top_panel.path);
    write_config_int("top_panel_active", top_panel_active=TRUE);
  }
  else
  {
    active_panel=&bottom_panel;
    inactive_panel=&top_panel;
    (void)chdir(bottom_panel.path);
    write_config_int("top_panel_active", top_panel_active=FALSE);
  }
  wait_for_draw();
}

void go_upper(struct_panel *panel) /* Переход на уровень вверх в дереве */
{
  enable_refresh=FALSE;
  interface_is_locked=TRUE;
  if (panel->archive_depth > 0) /* Если мы в архиве */
    archive_go_upper(panel);
  else
  {
    char *saved_path, *new_path;
    if (strcmp (panel->path, "/") == 0)
      return;
    trim_line(panel->path);
    saved_path=xconcat_path_file(strrchr(panel->path,'/')+1, ""); /* Сохраняем текущий каталог */
    #ifdef debug_printf
    printf("saved_path=%s\n", saved_path);
    #endif
    (void)chdir("..");
    new_path=strrchr(panel->path, '/')+1; // Находим начало имени покинутого каталога
    new_path[0]='\0'; // Обрезаем строку panel->path на уровень выше
    update(panel);
    select_file_by_name(saved_path, panel);
    if (panel == &top_panel)
    {
      write_config_string("top_panel.path", top_panel.path);
      write_config_string("top_panel.selected_name", top_panel.selected_name);
      write_config_string("top_panel.last_name", "");
    }
    else
    {
      write_config_string("bottom_panel.path", bottom_panel.path);
      write_config_string("bottom_panel.selected_name", bottom_panel.selected_name);
      write_config_string("bottom_panel.last_name", "");
    }
    free (saved_path);
  }
  gtk_widget_queue_draw(GTK_WIDGET(panel->list)); /* Заставляем GTK перерисовать список каталогов */
  wait_for_draw();
  enable_refresh=TRUE;
  interface_is_locked=FALSE;
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_full();
}

void actions(struct_panel *panel) /*выбор что делать по клику переход или запуск */
{
  #ifdef debug_printf
  printf("CWD=%s\n", panel->path);
  #endif
  if (strcmp(panel->selected_size, "dir ") == 0) /* Если кликнули не каталог */
  {
    if (panel == &top_panel) /* Сбрасываем имя последнего просмотренного файла, чтобы при следующем запуске не было ошибки */
    {
      top_panel.last_name[0]='\0';
      write_config_string("top_panel.last_name", top_panel.last_name);
    }
    else
    {
      bottom_panel.last_name[0]='\0';
      write_config_string("bottom_panel.last_name", bottom_panel.last_name);
    }

    if (strcmp(panel->selected_name, "../")== 0)  /* Если кликнули '../' */
    {
      #ifdef debug_printf
      printf("../ clicked\n");
      #endif
      go_upper(panel);
    }
    else  /* Если кликнули что-то иное, не '../' */
    {
      #ifdef debug_printf
      printf("'%s' clicked\n", panel->selected_name);
      #endif
      enter_subdir(panel->selected_name, panel);
    }
  }
  else /* Если кликнули на файл - проверкa по типу файла */
  {
    /* Если картинка - пуск смотрелки, после отработки смотрелки возврат  идет не сюда, а в wait_state */
    if (is_picture(panel->selected_name)) ViewImageWindow(panel->selected_name, panel, TRUE);
    if (is_archive(panel->selected_name))
    {
      if (panel == &top_panel) /* Сбрасываем имя последнего просмотренного файла, чтобы при следующем запуске не было ошибки */
      {
        top_panel.last_name[0]='\0';
        write_config_string("top_panel.last_name", top_panel.last_name);
      }
      else
      {
        bottom_panel.last_name[0]='\0';
        write_config_string("bottom_panel.last_name", bottom_panel.last_name);
      }
      if (panel->archive_depth == 0)
      {
        enable_refresh=FALSE;
        enter_archive(panel->selected_name, panel, TRUE); /* Вход в архив, если не в архиве */
        enable_refresh=TRUE;
        if (QT) usleep (QT_REFRESH_DELAY);
        e_ink_refresh_full();
      }
      else
      {
        char *subarchive=xconcat(panel->archive_cwd, panel->selected_name);
        enter_subarchive(subarchive, panel);
        free(subarchive);
        if (QT) usleep (QT_REFRESH_DELAY);
        e_ink_refresh_local();
      }
    }
    if (is_text(panel->selected_name) && (QT == FALSE))
    {
      char *command;
      if (panel->archive_depth > 0)
      {
        char *filename, *file;
        #ifdef debug_printf
        printf("Extracting %s\n", panel->selected_name);
        #endif
        filename=xconcat(panel->archive_cwd, panel->selected_name);
        file=xconcat("/tmp", filename);
        archive_extract_file(panel->archive_stack[panel->archive_depth], filename, "/tmp/");
        asprintf(&command, "/usr/bin/reader '%s'", file);
        xsystem(command);
        free(command);
        e_ink_refresh_full();

        #ifdef debug_printf
        printf("Removing extracted '%s'\n", file);
        #endif
        (void)remove(file);
        free(filename);
        free(file);
      }
      else
      {
        asprintf(&command, "/usr/bin/fbreader '%s'", panel->selected_name);
        xsystem(command);
        xfree (&command);
      }
    }
  }
}

gint which_keys_main (__attribute__((unused))GtkWidget *window, GdkEventKey *event, struct_panel *panel) /*реакция на кнопки */
{
  if (check_key_press(event->keyval, panel)) return TRUE;
  switch (event->keyval){
    case   GDK_m:
    case   KEY_MENU:
    case   KEY_MENU_LIBROII:
    case   KEY_MENU_QT:
      start_main_menu (panel);
      return FALSE;

    case   GDK_h:
    case   KEY_HOME: /*panel focus change */
      panel_selector (active_panel == &top_panel ? &bottom_panel : &top_panel);
      return TRUE;

    case   KEY_LEFT:
      copy_dir_or_file ();
      return TRUE;

    case    KEY_RIGHT:
      move_dir_or_file ();
      return TRUE;

    case   KEY_REFRESH_LIBROII:
    case   KEY_REFRESH_QT:
      e_ink_refresh_full();
      return FALSE;

    case   KEY_UP:
    {
      char *str_iter;
      str_iter=get_current_iter(panel);
      if (atoi(str_iter) == 0)
      {
        char *last_row_string=itoa(panel->files_num+panel->dirs_num-1);
        need_full_refresh=TRUE;
        move_selection (last_row_string, panel);
        free (last_row_string);
        free (str_iter);
        return TRUE;
      }
      free (str_iter);
      return FALSE;
    }

    case   KEY_DOWN:
    {
      char *str_iter=get_current_iter(panel);
      if (atoi(str_iter) == panel->files_num+panel->dirs_num-1)
      {
        need_full_refresh=TRUE;
        move_selection ("0", panel);
        free (str_iter);
        return TRUE;
      }
      free (str_iter);
      return FALSE;
    }

    case KEY_BACK:/*GDK_x: */
      go_upper(panel);
      return TRUE;

    case KEY_PGDOWN:
    case KEY_PGUP:
      need_full_refresh=TRUE;
      return FALSE;
      
    case KEY_OK:
      return FALSE;

    default:
      #ifdef debug_printf
      printf("got unknown keycode 0x%x in main\n", event->keyval);
      #endif
      return FALSE;
  }
}

// gint which_keys_main_after (__attribute__((unused))GtkWidget *window, GdkEventKey *event, struct_panel *panel) /*реакция на кнопки */
// {
//   if (check_key_press(event->keyval, panel)) return TRUE;
//   switch (event->keyval){
//     case KEY_PGDOWN:
//     case KEY_PGUP:
//       wait_for_draw();
//     
//     default:
//       return FALSE;
//   }
// }


void select_file_by_name(const char * const name, const struct_panel * const panel)
{
  #ifdef debug_printf
  printf("Selecting file '%s'\n", name);
  #endif
  char *iter=iter_from_filename (name, panel);
  move_selection(iter, panel);
  free(iter);
}

void create_panel (struct_panel *panel)
{
  /*   GtkWidget *vbox = gtk_vbox_new (FALSE, 0); */
  panel->table = gtk_table_new(30, 1, TRUE);
  gtk_table_set_homogeneous(GTK_TABLE(panel->table ), HOMOGENEUS);
  gtk_box_pack_start (GTK_BOX (panels_vbox), panel->table , TRUE, TRUE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(panel->table ), 0);

  panel->path_label  = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (panel->path_label ), (gfloat)0, (gfloat)0);
  gtk_box_pack_start (GTK_BOX (panels_vbox), panel->path_label , (gfloat)FALSE, (gfloat)FALSE, 0);
  gtk_label_set_text (GTK_LABEL(panel->path_label), panel->path);
  gtk_label_set_ellipsize (GTK_LABEL(panel->path_label), PANGO_ELLIPSIZE_START);
  panel->list = string_list_create_on_table(2, panel->table, 0, 1, 0, 30, SHOW, NOT_EDITABLE, " Name", "size/dir", 0.0,0.0);
  wait_for_draw(); /* Ожидаем отрисовки всего */
  (void)g_signal_connect (GTK_TREE_SELECTION(gtk_tree_view_get_selection (panel->list)), "changed", G_CALLBACK (dirlist_select), panel); /*реакция на сдвиг выделения */
  (void)g_signal_connect_after (GTK_TREE_SELECTION(gtk_tree_view_get_selection (panel->list)), "changed", G_CALLBACK (after_dirlist_select), panel); /*обновление экрана после перемещения курсора */
  (void)g_signal_connect_swapped (panel->list, "row-activated", G_CALLBACK (actions), panel); /*реакция на клик по выбору */
  (void)g_signal_connect (G_OBJECT (panel->list), "key_press_event", G_CALLBACK (which_keys_main), panel); /* Обработка назначенных кнопок */
//   (void)g_signal_connect_after (G_OBJECT (panel->list), "key_press_event", G_CALLBACK (which_keys_main_after), panel); /* Обработка назначенных кнопок */
  (void)g_signal_connect_swapped (G_OBJECT (panel->list), "focus_in_event", G_CALLBACK (panel_focussed), panel);
}

/* ****************** Standard window widget******************************** */
GtkWidget *window_create(int x, int y, guint border, const char *title, int modal)
{
  #ifdef debug_printf
  printf("constructing window %dx%d\n",x,y);
  #endif
  GtkWidget *new_window;
  /* get a handle to the screen for its measurements */
  new_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(new_window), x, y);
  gtk_window_set_position (GTK_WINDOW (new_window), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_set_title(GTK_WINDOW(new_window), title);

  /*   g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (wait_state), NULL); */
  (void)g_signal_connect (G_OBJECT (new_window), "delete_event", G_CALLBACK  (gtk_main_quit), NULL);

  gtk_container_set_border_width(GTK_CONTAINER(new_window), border);
  gtk_window_set_modal(GTK_WINDOW(new_window), (modal == MODAL) ? TRUE : FALSE);
  return new_window;
}

/**** Create a Multi column scrolled filename list ************************/
void add_data_to_list(GtkTreeView *tree, const char *data_string, int n_columns, int autoscroll, const char *fs)
{
  GtkTreeIter iter;
  GtkListStore *store;
  char *data;
  int i;

  store = (GtkListStore *)gtk_tree_view_get_model(tree);
  gtk_list_store_append(GTK_LIST_STORE(store), &iter);
  for (i = 0; i < n_columns; i++) {
    if (!data_string) {
      data_string = "";
      g_print("					mygtk\n");
    }
    if (g_utf8_validate(data_string, -1, NULL) != TRUE) {
      if ((data = g_locale_to_utf8(data_string, -1, NULL, NULL, NULL))) {
        gtk_list_store_set(GTK_LIST_STORE(store), &iter, i, data, -1);
        free(data);
      }
    } else {
      gtk_list_store_set(GTK_LIST_STORE(store), &iter, i, data_string, -1);
      gtk_list_store_set(GTK_LIST_STORE(store), &iter, i+1, fs, -1);
    }
  }
  if (autoscroll) {
    GtkTreeModel* model = gtk_tree_view_get_model(tree);
    GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), &iter);
    gtk_tree_view_scroll_to_cell(tree, path, NULL, TRUE, (gfloat)0.5, (gfloat)0.5);
    gtk_tree_path_free(path);
    //     free(model); // Не надо - карается abort()ом
  }
}

GtkTreeView *string_list_create_on_table(size_t num,
                                         GtkWidget *table, guint start_col, guint end_col,
                                         guint start_row, guint end_row, guint show_hide, guint editable,...)
{
  GtkTreeViewColumn *column;
  GtkListStore *store;
  GtkWidget *scrolled_window;
  GtkTreeView *tree;
  GtkTreeSelection *selection;
  GType *types;
  va_list titles;
  size_t i;
  const char *label;

  /* This is the scrolled window to put the cList widget inside */
  scrolled_window = gtk_scrolled_window_new(NULL, NULL);

  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  /*adjust =gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window)); */
  gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 0);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_ETCHED_IN);

  types = (GType *) xmalloc(num * sizeof(GType));
  for (i = 0; i < num; i++)
    types[i] = G_TYPE_STRING;
  store = gtk_list_store_newv((int)num, types);
  free(types);

  tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(store)));
  /*дерево с разными по цвету строками */
  gtk_tree_view_set_rules_hint(tree, FALSE);
  /* Setup the selection handler */
  selection = gtk_tree_view_get_selection(tree);
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

  /*    if (func) { */
  /*            signal_connect(selection, "changed", G_CALLBACK(func), NULL); */
  /*    } */
  /*    g_signal_connect (G_OBJECT (adjust), "value_changed", */
  /*                   G_CALLBACK (print_adjust), NULL); */

  gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(tree));
  if (table)
    gtk_table_attach_defaults(GTK_TABLE(table), GTK_WIDGET(scrolled_window), start_col, end_col, start_row, end_row);

  /* The view now holds a reference.  We can get rid of our own reference */
  g_object_unref(G_OBJECT(store));

  va_start(titles, editable);
  for (i = 0; i < num; i++) {
    /*создаются строки - ячейки */
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    const char *tmp = va_arg(titles, char *);
    double align = va_arg(titles, double);
    if (!tmp || show_hide == HIDE ) tmp = "";

    /* Create a column, associating the "text" attribute of the  cell_renderer to the column of the model */
    if (g_utf8_validate(tmp, -1, NULL) != TRUE) {
      label = g_locale_to_utf8(tmp, -1, NULL, NULL, NULL);
      column = gtk_tree_view_column_new_with_attributes(label, renderer, "text", i, NULL);
      xfree(&label);
    } else {
      /*название колонки из тмп */
      column = gtk_tree_view_column_new_with_attributes(tmp, renderer, "text", i, NULL);
    }
    g_object_set (renderer,"xalign", align, NULL);
    /* позиция и размер столбцов в дереве */
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    if (i==0) gtk_tree_view_column_set_fixed_width (column, COLUMN_W);
    if (i){/*выравнивание по правому краю */
      gtk_tree_view_column_set_fixed_width (column, COLUMN_W2);
      g_object_set (renderer, "xalign", 1.0, NULL);
    }
    (void)gtk_tree_view_append_column(tree, column);
    //     free(renderer); // Не надо - карается abort()ом
    //     free(column); // То же самое
  }
  va_end(titles);
  return tree;
}

void enter_suspend(struct_panel *panel)
{
  int saved_crop=crop;
  int saved_rotate=rotate;
  int saved_frame=frame;
  int saved_preload_enable=preload_enable;
  int saved_keepaspect=keepaspect;
  int saved_boost_contrast=boost_contrast;

  gtk_idle_remove (idle_call_handler); /* Удаляем вызов этой функции из очереди вызовов (иначе на ARM она будет вызываться вечно) */
  if (suspended == FALSE)
  {
    #ifdef debug_printf
    printf("Entering suspend\n");
    #endif
    xsystem("dbus-send /PowerManager com.sibrary.Service.PowerManager.requestSuspend");
    #ifdef debug_printf
    printf("DBUS sent\n");
    #endif

    boost_contrast=crop=rotate=frame=preload_enable=FALSE; /* Грязно перенастраиваем смотрелку */
    suspended=keepaspect=TRUE;
    if (in_picture_viewer)
    {
      (void)show_image(&screensaver, panel, FALSE, PAGE_FULL);
      e_ink_refresh_full();
      was_in_picture_viewer=in_picture_viewer;
    }
    else
      ViewImageWindow(screensavers_array[suspend_count], active_panel, FALSE);
    /* Восстанавливаем предыдущие настройки */
    crop=saved_crop;
    rotate=saved_rotate;
    frame=saved_frame;
    preload_enable=saved_preload_enable;
    keepaspect=saved_keepaspect;
    boost_contrast=saved_boost_contrast;
    set_brightness(0);
    preload_next_screensaver();
    suspend_hardware();
    #ifdef debug_printf
    printf("Suspend done\n");
    #endif
  }
  else
  {
    #ifdef debug_printf
    printf("eView is already suspended!\n");
    #endif
  }
}
