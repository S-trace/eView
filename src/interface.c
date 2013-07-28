/* Soul Trace, 2013, Distributed under GPLv2 Terms */
// Главное меню, меню опций ФМ и картинки и их callback'и

#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdlib.h> // atoi()

#include "gtk_file_manager.h" // Инклюдить первой среди своих, ибо typedef panel!
#include "mylib.h"
#include "mygtk.h"
#include "digma_hw.h"
#include "translations.h"
#include "ViewImageWindow.h"/*reset_preloaded_image*/
#include "os-specific.h"

static GtkWidget *create, *copy, *moving, *delete, *options, *exit_button; // Кнопки в главном меню
static GtkWidget *fmanager, *move_chk, *clock_panel, *ink_speed, *show_hidden_files_chk, *LED_notify_checkbox, *reset_configuration, *backlight_scale, *about_program; // Пункты в настройках ФМ
static GtkWidget *crop_image, *rotate_image, *manga_mode, *frame_image, *keepaspect_image, *double_refresh_image, *viewed, *preload_enabled_button, *suppress_panel_button; // Чекбоксы в настройках вьювера
static GtkWidget *loop_dir_none, *loop_dir_loop, *loop_dir_next, *loop_dir_exit, *loop_dir_frame, *loop_dir_vbox; // Радиобаттон в настройках вьювера
int need_refresh=FALSE;


// **************************************************  Picture menu  ***********************************************************
static void power_information(void)
{
  char *label_text, *battery_capacity, *battery_chip_temp, *battery_chip_volt, *battery_current, *power_supplier, *battery_status, *battery_temp, *battery_voltage, *battery_time_to_empty, *battery_time_to_full, *usb_current, *usb_voltage, *ac_current, *ac_voltage;
  /* Создаём виджеты */
  #ifdef debug_printf
  printf ("Show battery_information\n");
  #endif
  
  read_string(BATTERY_POWER_SUPPLIER, &power_supplier); // источник питания (USB)
  read_string(BATTERY_CAPACITY, &battery_capacity); // Заряд в процентах
  read_string(BATTERY_CHIP_TEMP,  &battery_chip_temp); // Температура чипа (градусы)
  read_string(BATTERY_CHIP_VOLT,  &battery_chip_volt); // Вольтаж чипа (миливольты)
  read_string(BATTERY_STATUS, &battery_status); // ход зарядки или разрядки (Full/Discharging/Charging)
  read_string(BATTERY_CURRENT, &battery_current); // Ток батареи (милиамперы)
  read_string(BATTERY_TEMP,  &battery_temp); // Температура батареи (градусы)
  read_string(BATTERY_VOLT,  &battery_voltage); // Вольтаж батареи (миливольты)
  
  if (strcmp(power_supplier, "Baterry") == 0)
  {
    read_string(BATTERY_TIME_TO_EMPTY,  &battery_time_to_empty); // Время до разрядки
    char *time_to_empty=get_natural_time(atoi(battery_time_to_empty));
    asprintf(&label_text, POWER_SOURCE_IS_BATTERY, battery_capacity, battery_chip_temp, battery_chip_volt, battery_current, power_supplier, battery_status, battery_temp, battery_voltage, time_to_empty);
    xfree(&battery_time_to_empty);
    xfree(&time_to_empty);
  }
  else
  {
    read_string(BATTERY_TIME_TO_FULL,  &battery_time_to_full); // Время до зарядки
    char *time_to_full=get_natural_time(atoi(battery_time_to_full));
    if (strcmp(power_supplier, "USB") == 0)
    {
      read_string(USB_CURRENT,  &usb_current); // Ток USB
      read_string(USB_VOLTAGE,  &usb_voltage); // Вольтаж USB  
      asprintf(&label_text, POWER_SOURCE_IS_USB, battery_capacity, battery_chip_temp, battery_chip_volt, battery_current, power_supplier, battery_status, battery_temp, battery_voltage, time_to_full, usb_current, usb_voltage);      
      xfree(&usb_current); 
      xfree(&usb_voltage);
    }
    else
    {
      read_string(AC_CURRENT,  &ac_current); // Ток AC
      read_string(AC_VOLTAGE,  &ac_voltage); // Вольтаж AC
      char *time_to_full=get_natural_time(atoi(battery_time_to_full));
      asprintf(&label_text, POWER_SOURCE_IS_AC, battery_capacity, battery_chip_temp, battery_chip_volt, battery_current, power_supplier, battery_status, battery_temp, battery_voltage, time_to_full, ac_current, ac_voltage);
      xfree(&ac_current); 
      xfree(&ac_voltage);
    }
    xfree(&time_to_full);
    xfree(&battery_time_to_full); 
  }
  Message(POWER_STATUS, label_text);
  xfree(&label_text);
  xfree(&power_supplier); 
  xfree(&battery_capacity); 
  xfree(&battery_chip_temp); 
  xfree(&battery_chip_volt); 
  xfree(&battery_status); 
  xfree(&battery_current); 
  xfree(&battery_temp); 
  xfree(&battery_voltage); 
  e_ink_refresh_full();
}

static void crop_image_toggler () // Callback для галки обрезки полей
{
  write_config_int("crop", crop=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(crop_image)));
  need_refresh=TRUE;
  e_ink_refresh_part ();
}

static void rotate_image_toggler () // Callback для галки поворота
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rotate_image))) { // Если галка активируется
    rotate = TRUE; // Включаем поворот
    gtk_widget_set_sensitive(frame_image, TRUE); // И включаем галку умного листания
    gtk_widget_set_sensitive(manga_mode, TRUE); // И включаем галку режима манги
  } else { // Если галка снимается
    rotate = FALSE; // Отключаем поворот
    
    write_config_int("frame", frame=FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(frame_image), FALSE); // Отключаем с ним умное листание
    gtk_widget_set_sensitive(frame_image, FALSE); // И блокируем его
    
    write_config_int("manga", manga=FALSE); // Отключаем режим манги
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(manga_mode), FALSE); // Отключаем с ним режим манги
    gtk_widget_set_sensitive(manga_mode, FALSE); // И блокируем его
  }
  write_config_int("rotate", rotate); // Сохраняем конфиги
  need_refresh=TRUE;
  e_ink_refresh_part ();
}

static void frame_image_toggler () // Callback для галки умного листания
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(frame_image))) { // Если галка активируется
    frame = TRUE; // Включаем умное листание
    crop = TRUE; // И обрезку полей
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(crop_image), TRUE); // Принудительно активируем галку обрезки полей на экране (для красоты)
    gtk_widget_set_sensitive(crop_image, FALSE); // А затем отключаем выключатели обрезки полей
    gtk_widget_set_sensitive(rotate_image, FALSE); // Поворота
    gtk_widget_set_sensitive(manga_mode, FALSE); // И режима манги
  } else { // Если галка снимается
    frame = FALSE; // Отключаем умное листание
    gtk_widget_set_sensitive(crop_image, TRUE); // А затем включаем выключатели обрезки полей
    gtk_widget_set_sensitive(rotate_image, TRUE); // Поворота
    gtk_widget_set_sensitive(manga_mode, TRUE); // И режима манги
  }
  write_config_int("crop", crop);   // Сохраняем конфиги
  write_config_int("frame", frame); // Сохраняем конфиги
  need_refresh=TRUE;
  e_ink_refresh_part ();
}

static void manga_mode_toggler () // Callback для галки просмотра как манги
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(manga_mode))) { // Если галка активируется
    manga = TRUE; // Включаем режим манги
    crop = TRUE; // И обрезку полей
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(crop_image), TRUE); // Принудительно активируем галку обрезки полей на экране (для красоты)
    gtk_widget_set_sensitive(crop_image, FALSE); // А затем отключаем выключатели обрезки полей
    gtk_widget_set_sensitive(rotate_image, FALSE); // Поворота
    gtk_widget_set_sensitive(frame_image, FALSE); // И умного просмотра
  } else { // Если галка снимается
    manga = FALSE; // Отключаем листание манги
    gtk_widget_set_sensitive(crop_image, TRUE); // А затем включаем выключатели обрезки полей
    gtk_widget_set_sensitive(rotate_image, TRUE); // Поворота
    gtk_widget_set_sensitive(frame_image, TRUE); // И умного просмотра
  }
  write_config_int("crop", crop);   // Сохраняем конфиги
  write_config_int("manga", manga); // Сохраняем конфиги
  need_refresh=TRUE;
  e_ink_refresh_part ();
}

static void keepaspect_image_toggler () // Callback для галки сохранения пропорций
{
  write_config_int("keepaspect", keepaspect = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(keepaspect_image)));
  need_refresh=TRUE;
  e_ink_refresh_part ();
}

static void double_refresh_toggler () // Callback для галки двойного обновления
{
  write_config_int("double_refresh", double_refresh=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(double_refresh_image)));
  e_ink_refresh_part ();
}

static void preload_toggler () // Callback для галки включения предзагрузки
{
  write_config_int("preload_enable", preload_enable=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(preload_enabled_button)));
  e_ink_refresh_part ();
}

static void suppress_panel_callback () // Callback для галки подавления панели
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(suppress_panel_button))) { // Если галка активируется
    suppress_panel = TRUE; // Включаем подавление
    kill_panel();
  } else { // Если галка снимается
    suppress_panel = FALSE; // Отключаем подавление
    start_panel();
  }
  write_config_int("suppress_panel", suppress_panel); // Сохраняем конфиги
  e_ink_refresh_part ();
}

static void loop_dir_toggler () // Callback для радиобаттона по действию при окончании каталога
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loop_dir_none))) loop_dir = LOOP_NONE; 
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loop_dir_loop))) loop_dir = LOOP_LOOP; 
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loop_dir_next))) loop_dir = LOOP_NEXT; 
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loop_dir_exit))) loop_dir = LOOP_EXIT; 
  write_config_int("loop_dir", loop_dir);
  e_ink_refresh_part ();
}

static void reset_statistics() // Callback для кнопки статистики (сбросс)
{
  if(confirm_request(RESET_VIEWED_PAGES, GTK_STOCK_OK, GTK_STOCK_CANCEL))
  {
    #ifdef debug_printf
    printf("Resetting static!\n");
    #endif
    write_config_int("viewed_pages", viewed_pages=0);
    gtk_button_set_label(GTK_BUTTON(viewed), VIEWED_PAGES" 0");
    e_ink_refresh_part();
  }
}

static gint keys_rotation_picture_menu (__attribute__((unused))GtkWidget *window, GdkEventKey *event) //Круговое перемещение по меню в смотрелке
{
  set_brightness(backlight);
  switch (event->keyval){
    case   KEY_UP:
      if (gtk_widget_is_focus (crop_image))
      {
        gtk_widget_grab_focus (viewed);
        return TRUE;
      }
      else
        return FALSE;
      
    case   KEY_DOWN:
      if (gtk_widget_is_focus (viewed))
      {
        gtk_widget_grab_focus (crop_image);
        return TRUE;
      }
      else
        return FALSE;
      
    default:
      return FALSE;
  }
}

static void picture_menu_destroy (panel *panel, GtkWidget *dialog) // Уничтожаем меню настроек отображения
{
  enable_refresh=FALSE;
  gtk_widget_destroy(dialog);
  //   gtk_widget_grab_focus (win);
  reset_preloaded_image(); // Cбрасываем предзагрузку - всё равно она не сработает сейчас
  if (need_refresh)
  {
    show_image(panel->selected_name, panel); // Повторно показываем картинку для учёта изменений
    viewed_pages--; // Откатываем назад счётчик страниц - после открытия-закрытия меню он не должен изменяться
  }
  enable_refresh=TRUE;
  e_ink_refresh_local();
  //   g_signal_handlers_unblock_by_func( win, focus_in_callback, NULL );
  //   g_signal_handlers_unblock_by_func( win, focus_out_callback, NULL );
}

static gint keys_in_picture_menu (GtkWidget *dialog, GdkEventKey *event, panel *panel) //задействует кнопки
{
  set_brightness(backlight);
  if (interface_is_locked)
  {
    #ifdef debug_printf
    printf("Interface was locked, keypress ignored!\n");
    #endif
    return TRUE;
  }
  switch (event->keyval){
    case   KEY_MENU:
    case   GDK_m:
    case   KEY_BACK:
    case   KEY_MENU_LIBROII:
    case   KEY_MENU_QT:
      picture_menu_destroy (panel, dialog);
      return FALSE;
      break;
      
    case   KEY_REFRESH_LIBROII:
    case   KEY_REFRESH_QT:
      e_ink_refresh_full();
      return FALSE;
      
    default:
      e_ink_refresh_part();
      return FALSE;
  }
}

void start_picture_menu (panel *panel, GtkWidget *win) // Создаём меню настроек картинки
{
  need_refresh=FALSE;
  
  GtkWidget *menu_vbox = gtk_vbox_new (FALSE, 0);
  
  crop_image = gtk_check_button_new_with_label (CROP_IMAGE);
  if (crop) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(crop_image), TRUE);
  gtk_button_set_relief (GTK_BUTTON(crop_image), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), crop_image, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (crop_image), "clicked", G_CALLBACK (crop_image_toggler), NULL);
  g_signal_connect (G_OBJECT (crop_image), "key_press_event",G_CALLBACK (keys_rotation_picture_menu), NULL);
  
  rotate_image = gtk_check_button_new_with_label(ROTATE_IMAGE);
  if (rotate) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(rotate_image), TRUE);
  gtk_button_set_relief (GTK_BUTTON(rotate_image), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), rotate_image, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (rotate_image), "clicked", G_CALLBACK (rotate_image_toggler), NULL);
  
  frame_image = gtk_check_button_new_with_label(FRAME_IMAGE);
  if (frame) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(frame_image), TRUE);
  gtk_button_set_relief (GTK_BUTTON(frame_image), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), frame_image, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (frame_image), "clicked", G_CALLBACK (frame_image_toggler), NULL);
  
  manga_mode = gtk_check_button_new_with_label(MANGA_MODE);
  if (manga) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(manga_mode), TRUE);
  gtk_button_set_relief (GTK_BUTTON(manga_mode), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), manga_mode, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (manga_mode), "clicked", G_CALLBACK (manga_mode_toggler), NULL);
  
  keepaspect_image = gtk_check_button_new_with_label(KEEP_ASPECT);
  if (keepaspect) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(keepaspect_image), TRUE);
  gtk_button_set_relief (GTK_BUTTON(keepaspect_image), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), keepaspect_image, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (keepaspect_image), "clicked", G_CALLBACK (keepaspect_image_toggler), NULL);
  
  double_refresh_image = gtk_check_button_new_with_label(DOUBLE_REFRESH);
  if (double_refresh) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(double_refresh_image), TRUE);
  gtk_button_set_relief (GTK_BUTTON(double_refresh_image), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), double_refresh_image, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (double_refresh_image), "clicked", G_CALLBACK (double_refresh_toggler), NULL);
  
  loop_dir_frame = gtk_frame_new (ACTION_ON_LAST_FILE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), loop_dir_frame, FALSE, TRUE, 0);
  
  loop_dir_vbox = gtk_vbox_new (TRUE, 0);
  gtk_container_add (GTK_CONTAINER (loop_dir_frame), loop_dir_vbox);
  
  loop_dir_none = gtk_radio_button_new_with_label (NULL, DO_NOTHING);
  if (loop_dir == LOOP_NONE) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (loop_dir_none), TRUE);
  gtk_box_pack_start (GTK_BOX (loop_dir_vbox), loop_dir_none, TRUE, TRUE, 0);
  gtk_button_set_relief (GTK_BUTTON(loop_dir_none), GTK_RELIEF_NONE);
  
  loop_dir_loop = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (loop_dir_none), LOOP_DIRECTORY);
  if (loop_dir == LOOP_LOOP) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (loop_dir_loop), TRUE);
  gtk_button_set_relief (GTK_BUTTON(loop_dir_loop), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (loop_dir_vbox), loop_dir_loop, TRUE, TRUE, 0);
  
  loop_dir_next = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (loop_dir_none), NEXT_DIRECTORY);
  if (loop_dir == LOOP_NEXT) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (loop_dir_next), TRUE);
  gtk_box_pack_start (GTK_BOX (loop_dir_vbox), loop_dir_next, TRUE, TRUE, 0);
  gtk_button_set_relief (GTK_BUTTON(loop_dir_next), GTK_RELIEF_NONE);
  
  loop_dir_exit = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (loop_dir_none), EXIT_TO_FILEMANAGER);
  if (loop_dir == LOOP_EXIT) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (loop_dir_exit), TRUE);
  gtk_box_pack_start (GTK_BOX (loop_dir_vbox), loop_dir_exit, TRUE, TRUE, 0);
  gtk_button_set_relief (GTK_BUTTON(loop_dir_exit), GTK_RELIEF_NONE);
  g_signal_connect (G_OBJECT (loop_dir_none), "group-changed", G_CALLBACK (loop_dir_toggler), NULL); // Один callback на всех
  
  preload_enabled_button = gtk_check_button_new_with_label(ALLOW_PRELOADING);
  if (preload_enable) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(preload_enabled_button), TRUE);
  gtk_button_set_relief (GTK_BUTTON(preload_enabled_button), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), preload_enabled_button, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (preload_enabled_button), "clicked", G_CALLBACK (preload_toggler), NULL);
  
  if (! QT)
  {
    suppress_panel_button = gtk_check_button_new_with_label (SUPPRESS_BATTERY_WARNINGS);
    if (suppress_panel) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(suppress_panel_button), TRUE);
    gtk_button_set_alignment (GTK_BUTTON(suppress_panel_button), 0.0, 0.0);
    gtk_button_set_relief (GTK_BUTTON(suppress_panel_button), GTK_RELIEF_NONE);
    gtk_box_pack_start (GTK_BOX (menu_vbox), suppress_panel_button, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (suppress_panel_button), "clicked", G_CALLBACK (suppress_panel_callback), NULL);
  }
  char *battery_capacity, *name;
  read_string(BATTERY_CAPACITY, &battery_capacity); // Заряд в процентах
  asprintf(&name, BATTERY_CHARGE_PERCENT, battery_capacity);
  viewed = gtk_button_new_with_label (name);
  gtk_button_set_alignment (GTK_BUTTON(viewed), 0.0, 0.0);
  gtk_button_set_relief (GTK_BUTTON(viewed), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), viewed, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (viewed), "clicked", G_CALLBACK (power_information), NULL);
  xfree(&battery_capacity);
  xfree(&name);
  
  char *viewed_text=xconcat(VIEWED_PAGES, itoa(viewed_pages));
  viewed = gtk_button_new_with_label (viewed_text);
  gtk_button_set_alignment (GTK_BUTTON(viewed), 0.0, 0.0);
  gtk_button_set_relief (GTK_BUTTON(viewed), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), viewed, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (viewed), "clicked", G_CALLBACK (reset_statistics), NULL);
  g_signal_connect (G_OBJECT (viewed), "key_press_event",G_CALLBACK (keys_rotation_picture_menu), NULL);
  xfree(&viewed_text);
  
  if (!rotate) { // Если поворот отключен
    frame = FALSE; // Отключаем умное листание
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(frame_image), FALSE); // Выключаем галку умного листания
    gtk_widget_set_sensitive(frame_image, FALSE); // И блокируем её
    write_config_int("frame", frame=FALSE); // Сохраняем конфиги
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(manga_mode), FALSE); // Выключаем галку режима манги
    gtk_widget_set_sensitive(manga_mode, FALSE); // И блокируем её
    write_config_int("manga", manga=FALSE); // Сохраняем конфиги
  }
  
  if (frame) { // Если включено умное листание
    gtk_widget_set_sensitive(crop_image, FALSE); // Блокируем галки обрезки полей
    gtk_widget_set_sensitive(rotate_image, FALSE); // Поворота
    gtk_widget_set_sensitive(manga_mode, FALSE); // И режима манги
  }
  
  if (manga) { // Если включен режим манги
    gtk_widget_set_sensitive(crop_image, FALSE); // Блокируем галки обрезки полей
    gtk_widget_set_sensitive(rotate_image, FALSE); // Поворота
    gtk_widget_set_sensitive(frame_image, FALSE); // И умного просмотра
  }
  GtkWidget *dialog = gtk_dialog_new_with_buttons (SETTINGS,
                                                   GTK_WINDOW(win),
                                                   GTK_DIALOG_MODAL |GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   NULL);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), menu_vbox);
  gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_widget_show_all (dialog);
  
  g_signal_connect (G_OBJECT (dialog), "key_press_event", G_CALLBACK (keys_in_picture_menu), panel);
  e_ink_refresh_local();
}


// **************************************************  Options menu  ***********************************************************

static void fm_start () // Callback для галки включения ФМ в настройках
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fmanager))) {
    fm_toggle = TRUE;
    second_panel_show();
    gtk_widget_set_sensitive(create, TRUE);
    gtk_widget_set_sensitive(copy, TRUE);
    gtk_widget_set_sensitive(moving, TRUE);
    write_config_int ("fm_toggle", fm_toggle);
    e_ink_refresh_part ();
  } else {
    fm_toggle = FALSE;
    top_panel_active=TRUE;
    second_panel_hide();
    gtk_widget_set_sensitive(create, FALSE);
    gtk_widget_set_sensitive(copy, FALSE);
    gtk_widget_set_sensitive(moving, FALSE);
  }
  write_config_int ("fm_toggle", fm_toggle);
  e_ink_refresh_part ();
}

static void move_confirm ()
{
  write_config_int("move_toggle", move_toggle=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(move_chk)));
  e_ink_refresh_part ();
}

static void clock_panel_toggler ()
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(clock_panel))) {
    clock_toggle = FALSE;
    gtk_window_unfullscreen  (GTK_WINDOW(main_window));
  } else {
    clock_toggle = TRUE;
    gtk_window_fullscreen  (GTK_WINDOW(main_window));
  }
  write_config_int("clock_toggle", clock_toggle);
  e_ink_refresh_part ();
}

static void type_refresh ()
{
  write_config_int("speed_toggle", speed_toggle=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ink_speed)));
  e_ink_refresh_part ();
}

static void show_hidden_files_callback ()
{
  write_config_int("show_hidden_files", show_hidden_files=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_hidden_files_chk)));
  update(active_panel);
  if (fm_toggle) 
    update(&bottom_panel);
  e_ink_refresh_part ();
}

static void LED_notify_callback ()
{
  write_config_int("LED_notify", LED_notify = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(LED_notify_checkbox)));
  e_ink_refresh_part ();
}

static void reset_configuration_callback() // Callback для кнопки сброса конфигов
{
  if(confirm_request(RESET_CONFIGURATION"?", GTK_STOCK_OK, GTK_STOCK_CANCEL))
  {
    #ifdef debug_printf
    printf("Resetting configuration!\n");
    #endif
    reset_config();
    gtk_main_quit();
  }
}

static void backlight_changed(GtkWidget *scalebutton)
{
  write_config_int("backlight", backlight = gtk_range_get_value(GTK_RANGE(scalebutton)));
  set_brightness(backlight);
  #ifdef debug_printf
  printf("Brightness set to %d\n", backlight);
  #endif
}

// static void led_changed(GtkWidget *scalebutton)
// {
//   set_led_state(gtk_range_get_value(GTK_RANGE(scalebutton)));
// }

static void about_program_callback() // Callback для кнопки информации о программе
{
  Message(ABOUT_PROGRAM, ABOUT_PROGRAM_TEXT);
}

void options_destroy (GtkWidget *dialog) // Уничтожаем меню настроек отображения
{
  gtk_widget_destroy(dialog);
  e_ink_refresh_local();
}

static gint keys_rotation_options (__attribute__((unused))GtkWidget *window, GdkEventKey *event) //Круговое перемещение по меню в ФМ
{
  set_brightness(backlight);
  switch (event->keyval){
    case   KEY_UP:
      if (gtk_widget_is_focus (fmanager))
      {
        gtk_widget_grab_focus (about_program);
        return TRUE;
      }
      else
        return FALSE;
      
    case   KEY_DOWN:
      if (gtk_widget_is_focus (about_program))
      {
        gtk_widget_grab_focus (fmanager);
        return TRUE;
      }
      else
        return FALSE;
      
    default:
      return FALSE;
  }
}

static gint keys_updown_options (__attribute__((unused))GtkWidget *window, GdkEventKey *event) // Для возможности переключаться из регулятора подсветки вверх-вниз
{
  switch (event->keyval){
    case   KEY_UP:
        gtk_widget_grab_focus (LED_notify_checkbox);
        return TRUE;      
    case   KEY_DOWN:
      gtk_widget_grab_focus (reset_configuration);
    default:
      return FALSE;
  }
}

static gint keys_in_options (GtkWidget *dialog, GdkEventKey *event) //задействует кнопки
{
  set_brightness(backlight);
  if (interface_is_locked)
  {
    #ifdef debug_printf
    printf("Interface was locked, keypress ignored!\n");
    #endif
    return TRUE;
  }
  switch (event->keyval){
    case   KEY_MENU:
    case   GDK_m:
    case   KEY_BACK:
    case   KEY_MENU_LIBROII:
    case   KEY_MENU_QT:
      options_destroy (dialog);
      return FALSE;
      break;
      
    case   KEY_REFRESH_LIBROII:
    case   KEY_REFRESH_QT:
      e_ink_refresh_full();
      return FALSE;
      
    default:
      e_ink_refresh_part();
      return FALSE;
  }
}

void options_menu_create(GtkWidget *main_menu) //Создание меню опций в ФМ
{
  GtkWidget *options_dialog = gtk_dialog_new_with_buttons (SETTINGS,
                                                           GTK_WINDOW(main_menu),
                                                           GTK_DIALOG_MODAL |GTK_DIALOG_DESTROY_WITH_PARENT,
                                                           NULL);
  GtkWidget *menu_vbox = gtk_vbox_new (FALSE, 0);
  
  fmanager = gtk_check_button_new_with_label (FILEMANAGER_MODE);
  if (fm_toggle) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(fmanager), TRUE);
  gtk_button_set_relief (GTK_BUTTON(fmanager), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), fmanager, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (fmanager), "clicked", G_CALLBACK (fm_start), NULL);
  g_signal_connect (G_OBJECT (fmanager), "key_press_event", G_CALLBACK (keys_rotation_options), NULL);
  
  ink_speed = gtk_check_button_new_with_label (PARTIAL_UPDATE);
  if (!speed_toggle) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ink_speed), TRUE);
  gtk_button_set_relief (GTK_BUTTON(ink_speed), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), ink_speed, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (ink_speed), "clicked", G_CALLBACK (type_refresh), NULL);
  
  move_chk = gtk_check_button_new_with_label (CONFIRM_MOVE);
  if (move_toggle) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(move_chk), TRUE);
  gtk_button_set_relief (GTK_BUTTON(move_chk), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), move_chk, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (move_chk), "clicked", G_CALLBACK (move_confirm), NULL);
  
  show_hidden_files_chk = gtk_check_button_new_with_label (SHOW_HIDDEN_FILES);
  if (show_hidden_files) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(show_hidden_files_chk), TRUE);
  gtk_button_set_relief (GTK_BUTTON(show_hidden_files_chk), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), show_hidden_files_chk, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (show_hidden_files_chk), "clicked", G_CALLBACK (show_hidden_files_callback), NULL);
  
  if (! QT)
  {
    clock_panel = gtk_check_button_new_with_label(SHOW_PANEL);
    if (clock_toggle) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(clock_panel), TRUE);
    gtk_button_set_relief (GTK_BUTTON(clock_panel), GTK_RELIEF_NONE);
    gtk_box_pack_start (GTK_BOX (menu_vbox), clock_panel, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (clock_panel), "clicked", G_CALLBACK (clock_panel_toggler), NULL);
  }
  
  LED_notify_checkbox = gtk_check_button_new_with_label (LED_NOTIFY);
  if (LED_notify) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(LED_notify_checkbox), TRUE);
  gtk_button_set_relief (GTK_BUTTON(LED_notify_checkbox), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), LED_notify_checkbox, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (LED_notify_checkbox), "clicked", G_CALLBACK (LED_notify_callback), NULL);
  
  if (hardware_has_backlight)
  {
    GtkWidget *backlight_frame = gtk_frame_new (BACKLIGHT);
    gtk_box_pack_start (GTK_BOX (menu_vbox), backlight_frame, FALSE, TRUE, 0);
    backlight_scale = gtk_hscale_new_with_range (0, 8, 1);
    g_signal_connect(backlight_scale, "value-changed", G_CALLBACK(backlight_changed), NULL);
    gtk_range_set_value (GTK_RANGE(backlight_scale), backlight);
    gtk_container_add (GTK_CONTAINER (backlight_frame), backlight_scale);
    g_signal_connect (G_OBJECT (backlight_frame), "key_press_event", G_CALLBACK (keys_updown_options), NULL);
    
  }

//   GtkWidget *LED_test_frame = gtk_frame_new ("LED_TEST");
//   gtk_box_pack_start (GTK_BOX (menu_vbox), LED_test_frame, FALSE, TRUE, 0);
//   GtkWidget *led_test_scale = gtk_hscale_new_with_range (0, 255, 1);
//   g_signal_connect(led_test_scale, "value-changed", G_CALLBACK(led_changed), NULL);
//   gtk_container_add (GTK_CONTAINER (LED_test_frame), led_test_scale);
  
  reset_configuration = gtk_button_new_with_label ("   "RESET_CONFIGURATION);
  gtk_button_set_alignment (GTK_BUTTON(reset_configuration), 0.0, 0.0);
  gtk_button_set_relief (GTK_BUTTON(reset_configuration), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), reset_configuration, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (reset_configuration), "clicked", G_CALLBACK (reset_configuration_callback), NULL);  
  
  about_program = gtk_button_new_with_label ("   "ABOUT_PROGRAM);
  gtk_button_set_alignment (GTK_BUTTON(about_program), 0.0, 0.0);
  gtk_button_set_relief (GTK_BUTTON(about_program), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), about_program, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (about_program), "clicked", G_CALLBACK (about_program_callback), NULL);  
  g_signal_connect (G_OBJECT (about_program), "key_press_event", G_CALLBACK (keys_rotation_options), NULL);
  
  
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(options_dialog)->vbox), menu_vbox);
  gtk_window_set_position (GTK_WINDOW(options_dialog), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_widget_show_all (options_dialog);
  
  g_signal_connect (G_OBJECT (options_dialog), "key_press_event", G_CALLBACK (keys_in_options), NULL);
  e_ink_refresh_local();
}  


// ********************************  Main menu!  **************************************
void create_folder()
{
  xsystem("mkdir -p temp000");
  if (inactive_panel != NULL && !strcmp (active_panel->path, inactive_panel->path))
    update(inactive_panel);
  update(active_panel);
  e_ink_refresh_local ();
}

static gint keys_rotation_menu (__attribute__((unused))GtkWidget *window, GdkEventKey *event) //Круговое перемещение по меню в ФМ
{
  set_brightness(backlight);
  switch (event->keyval){
    case   KEY_UP:
      if (gtk_widget_is_focus (create))
      {
        gtk_widget_grab_focus (exit_button);
        return TRUE;
      }
      else
        return FALSE;
      
    case   KEY_DOWN:
      if (gtk_widget_is_focus (exit_button))
      {
        gtk_widget_grab_focus (create);
        return TRUE;
      }
      else
        return FALSE;
      
    default:
      return FALSE;
  }
}

static gint keys_in_main_menu (GtkWidget *dialog, GdkEventKey *event) //задействует кнопку М в меню
{
  set_brightness(backlight);
  if (interface_is_locked)
  {
    #ifdef debug_printf
    printf("Interface was locked, keypress ignored!\n");
    #endif
    return TRUE;
  }
  switch (event->keyval){
    case   KEY_BACK:
    case   KEY_MENU:
    case   KEY_MENU_LIBROII:
    case   KEY_MENU_QT:
    case   GDK_m:
      menu_destroy (dialog);
      chdir(top_panel.path);
      e_ink_refresh_local();
      return FALSE;
      
    case   KEY_REFRESH_LIBROII:
    case   KEY_REFRESH_QT:
      e_ink_refresh_full();
      return FALSE;
      
    default:
      e_ink_refresh_default ();
      return FALSE;
  }
}

void start_main_menu (void)
{
  GtkWidget *dialog = gtk_dialog_new_with_buttons (MAIN_MENU,
                                                   GTK_WINDOW(main_window),
                                                   GTK_DIALOG_MODAL |GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   NULL);
  GtkWidget *menu_vbox = gtk_vbox_new (TRUE, 0);
  
  create = gtk_button_new_with_label (CREATE_TEMPORARY_DIRECTORY);
  gtk_button_set_alignment (GTK_BUTTON(create), 0.0, 0.0);
  gtk_button_set_relief (GTK_BUTTON(create), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), create, FALSE, FALSE, 0);
  if (top_panel.archive_depth > 0 || bottom_panel.archive_depth > 0) gtk_widget_set_sensitive(create, FALSE); // В архиве не поддерживается
  g_signal_connect (G_OBJECT (create), "clicked", G_CALLBACK (create_folder), NULL);
  g_signal_connect (G_OBJECT (create), "key_press_event", G_CALLBACK (keys_rotation_menu), NULL);
  
  copy = gtk_button_new_with_label (COPY);
  gtk_button_set_alignment (GTK_BUTTON(copy), 0.0, 0.0);
  gtk_button_set_relief (GTK_BUTTON(copy), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), copy, FALSE, FALSE, 0);
  if (!fm_toggle) gtk_widget_set_sensitive(copy, FALSE);
  if (top_panel.archive_depth > 0 || bottom_panel.archive_depth > 0) gtk_widget_set_sensitive(copy, FALSE); // В архиве не поддерживается
  g_signal_connect (G_OBJECT (copy), "clicked", G_CALLBACK (copy_dir_or_file), NULL);
  
  moving = gtk_button_new_with_label (MOVE_FILE);
  gtk_button_set_alignment (GTK_BUTTON(moving), 0.0, 0.0);
  gtk_button_set_relief (GTK_BUTTON(moving), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), moving, FALSE, FALSE, 0);
  if (!fm_toggle) gtk_widget_set_sensitive(moving, FALSE);
  if (top_panel.archive_depth > 0 || bottom_panel.archive_depth > 0) gtk_widget_set_sensitive(moving, FALSE); // В архиве не поддерживается
  g_signal_connect (G_OBJECT (moving), "clicked", G_CALLBACK (move_dir_or_file), NULL);
  
  delete = gtk_button_new_with_label (DELETE);
  gtk_button_set_alignment (GTK_BUTTON(delete), 0.0, 0.0);
  gtk_button_set_relief (GTK_BUTTON(delete), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), delete, FALSE, FALSE, 0);
  if (top_panel.archive_depth > 0 || bottom_panel.archive_depth > 0) gtk_widget_set_sensitive(delete, FALSE); // В архиве не поддерживается
  g_signal_connect (G_OBJECT (delete), "clicked", G_CALLBACK (delete_dir_or_file), NULL);
  
  options = gtk_button_new_with_label (OPTIONS);
  gtk_button_set_alignment (GTK_BUTTON(options), 0.0, 0.0);
  gtk_button_set_relief (GTK_BUTTON(options), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), options, FALSE, FALSE, 0);
  g_signal_connect_swapped (G_OBJECT (options), "clicked", G_CALLBACK (options_menu_create), dialog);
  
  exit_button = gtk_button_new_with_label (EXIT);
  gtk_button_set_alignment (GTK_BUTTON(exit_button), 0.0, 0.0);
  gtk_button_set_relief (GTK_BUTTON(exit_button), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), exit_button, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (exit_button), "clicked", G_CALLBACK (shutdown), NULL);
  g_signal_connect (G_OBJECT (exit_button), "key_press_event", G_CALLBACK (keys_rotation_menu), NULL);
  
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), menu_vbox);
  gtk_widget_queue_draw(GTK_DIALOG(dialog)->vbox);
  gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_widget_show_all (dialog);
  
  g_signal_connect (GTK_WIDGET(dialog), "key_press_event", G_CALLBACK (keys_in_main_menu), NULL);
  e_ink_refresh_local();
}

