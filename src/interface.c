/* Soul Trace, 2013, Distributed under GPLv2 Terms */
// Главное меню, меню опций ФМ и картинки и их callback'и

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

static GtkWidget *create, *copy, *moving, *del, *options, *exit_button; // Кнопки в главном меню
static GtkWidget *fmanager, *move_chk, *clock_panel, *ink_speed, *show_hidden_files_chk, *LED_notify_checkbox, *reset_configuration, *backlight_scale, *sleep_timeout_scale, *about_program; // Пункты в настройках ФМ
static GtkWidget *crop_image, *split_spreads_button, *manga_mode, *rotate_image, *web_manga_mode_checkbox, *frame_image, *overlap_scale, *overlap_frame, *keepaspect_image, *double_refresh_image, *viewed, *preload_enabled_button, *caching_enabled_button, *suppress_panel_button, *HD_scaling_button, *boost_contrast_button, *power_information_button; // Чекбоксы в настройках вьювера
static GtkWidget *loop_dir_none, *loop_dir_loop, *loop_dir_next, *loop_dir_exit, *loop_dir_frame, *loop_dir_vbox; // Радиобаттон в настройках вьювера
static GtkWidget *picture_menu, *main_menu, *options_menu;
int need_refresh=FALSE;

void add_toggle_button_to_menu(GtkWidget *widget, GtkWidget *menu_vbox, void *clicked_callback, void *keypress_callback, int is_active, struct_panel *panel)
{
  if (is_active) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget), TRUE);
  gtk_button_set_relief (GTK_BUTTON(widget), GTK_RELIEF_NONE);
  gtk_button_set_alignment (GTK_BUTTON(widget), (gfloat)0.0, (gfloat)0.0);
  gtk_box_pack_start (GTK_BOX (menu_vbox), widget, TRUE, TRUE, 0);
  (void)g_signal_connect (G_OBJECT (widget), "key_press_event", G_CALLBACK (keypress_callback), panel);
  if (clicked_callback)
    (void)g_signal_connect (G_OBJECT (widget), "clicked", G_CALLBACK (clicked_callback), NULL);
}


// **************************************************  Picture menu  ***********************************************************
void power_information(void)
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
    char *time_to_empty;
    read_string(BATTERY_TIME_TO_EMPTY,  &battery_time_to_empty); // Время до разрядки
    time_to_empty=get_natural_time(atoi(battery_time_to_empty));
    asprintf(&label_text, POWER_SOURCE_IS_BATTERY, battery_capacity, battery_chip_temp, battery_chip_volt, battery_current, power_supplier, battery_status, battery_temp, battery_voltage, time_to_empty);
    free(battery_time_to_empty);
    free(time_to_empty);
  }
  else
  {
    char *time_to_full;
    read_string(BATTERY_TIME_TO_FULL,  &battery_time_to_full); // Время до зарядки
    time_to_full=get_natural_time(atoi(battery_time_to_full));
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
      asprintf(&label_text, POWER_SOURCE_IS_AC, battery_capacity, battery_chip_temp, battery_chip_volt, battery_current, power_supplier, battery_status, battery_temp, battery_voltage, time_to_full, ac_current, ac_voltage);
      xfree(&ac_current);
      xfree(&ac_voltage);
    }
    free(time_to_full);
    free(battery_time_to_full);
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
}

void crop_image_toggler () // Callback для галки обрезки полей
{
  write_config_int("crop", crop=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(crop_image)));
  need_refresh=TRUE;
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local ();
}

void split_spreads_toggler () // Callback для галки поворота
{
  write_config_int("split_spreads", split_spreads=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(split_spreads_button)));
  if (split_spreads)
    gtk_widget_set_sensitive(manga_mode, TRUE);
  else
    gtk_widget_set_sensitive(manga_mode, FALSE);
  need_refresh=TRUE;
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local ();
}

void rotate_image_toggler() // Callback для галки поворота
{
  write_config_int("rotate", rotate=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rotate_image)));
  if (rotate)
  {
    gtk_widget_set_sensitive(web_manga_mode_checkbox, FALSE);
    gtk_widget_set_sensitive(frame_image, TRUE);
    gtk_widget_set_sensitive(overlap_frame, TRUE);
    gtk_widget_set_sensitive(overlap_scale, TRUE);
  }
  else
  {
    gtk_widget_set_sensitive(web_manga_mode_checkbox, TRUE);
    gtk_widget_set_sensitive(frame_image, FALSE);
    gtk_widget_set_sensitive(overlap_frame, FALSE);
    gtk_widget_set_sensitive(overlap_scale, FALSE);
  }
  need_refresh=TRUE;
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local ();
}

void web_manga_mode_toggler() // Callback для галки режима веб-манги
{
  write_config_int("web_manga_mode", web_manga_mode=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(web_manga_mode_checkbox)));
  enable_refresh=FALSE;  
  if (web_manga_mode)
  {
    gtk_widget_set_sensitive(rotate_image, FALSE);
    gtk_widget_set_sensitive(frame_image, TRUE);
    gtk_widget_set_sensitive(overlap_frame, TRUE);
    gtk_widget_set_sensitive(overlap_scale, TRUE);
    gtk_widget_set_sensitive(split_spreads_button, FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(split_spreads_button), FALSE);
    gtk_widget_set_sensitive(split_spreads_button, FALSE);
  }
  else
  {
    gtk_widget_set_sensitive(rotate_image, TRUE);
    gtk_widget_set_sensitive(frame_image, FALSE);
    gtk_widget_set_sensitive(overlap_frame, FALSE);
    gtk_widget_set_sensitive(overlap_scale, FALSE);
    gtk_widget_set_sensitive(split_spreads_button, TRUE);
  }
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  enable_refresh=TRUE;
  need_refresh=TRUE;
  e_ink_refresh_local ();
}


void frame_image_toggler () // Callback для галки умного листания
{
  write_config_int("frame", frame=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(frame_image)));
  if (frame)
  {
    gtk_widget_set_sensitive(rotate_image, FALSE);
    gtk_widget_set_sensitive(web_manga_mode_checkbox, FALSE);
  }
  else 
  {
    if (rotate) gtk_widget_set_sensitive(rotate_image, TRUE);
    if (web_manga_mode) gtk_widget_set_sensitive(web_manga_mode_checkbox, TRUE);
  }
  need_refresh=TRUE;
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local ();
}

void overlap_changed(GtkWidget *scalebutton)
{
  write_config_int("overlap", overlap = gtk_range_get_value(GTK_RANGE(scalebutton)));
  #ifdef debug_printf
  printf("Overlap set to %d\n", overlap);
  #endif
  e_ink_refresh_default();
}

void manga_mode_toggler () // Callback для галки просмотра как манги
{
  write_config_int("manga", manga=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(manga_mode)));
  if (manga)
    gtk_widget_set_sensitive(split_spreads_button, FALSE);
  else
    gtk_widget_set_sensitive(split_spreads_button, TRUE);
  need_refresh=TRUE;
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local ();
}

void keepaspect_image_toggler () // Callback для галки сохранения пропорций
{
  write_config_int("keepaspect", keepaspect = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(keepaspect_image)));
  need_refresh=TRUE;
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local ();
}

void double_refresh_toggler () // Callback для галки двойного обновления
{
  write_config_int("double_refresh", double_refresh=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(double_refresh_image)));
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local ();
}

void preload_toggler () // Callback для галки включения предзагрузки
{
  write_config_int("preload_enable", preload_enable=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(preload_enabled_button)));
  if (preload_enable == FALSE) reset_image(&preloaded);
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local ();
}

void caching_toggler () // Callback для галки включения кэширования
{
  write_config_int("caching_enable", caching_enable=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(caching_enabled_button)));
  if (caching_enable == FALSE) reset_image(&cached);
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local ();
}

void suppress_panel_callback () // Callback для галки подавления панели
{
  write_config_int("suppress_panel", suppress_panel=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(suppress_panel_button)));
  if (suppress_panel)
    kill_panel();
  else
    start_panel();
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local ();
}

void HD_scaling_callback () // Callback для галки качественного скалирования
{
  write_config_int("HD_scaling", HD_scaling=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(HD_scaling_button)));
  need_refresh=TRUE;
  e_ink_refresh_local ();
}

void boost_contrast_callback () // Callback для галки качественного скалирования
{
  write_config_int("boost_contrast", boost_contrast=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(boost_contrast_button)));
  need_refresh=TRUE;
  e_ink_refresh_local ();
}

void reset_statistics() // Callback для кнопки статистики (сбросс)
{
  if(confirm_request(RESET_VIEWED_PAGES, GTK_STOCK_OK, GTK_STOCK_CANCEL))
  {
    #ifdef debug_printf
    printf("Resetting static!\n");
    #endif
    write_config_int("viewed_pages", viewed_pages=0);
    gtk_button_set_label(GTK_BUTTON(viewed), VIEWED_PAGES" 0");
    wait_for_draw();
    if (QT) usleep (QT_REFRESH_DELAY);
  }
  e_ink_refresh_local();
}

void picture_menu_destroy (struct_panel *panel) // Уничтожаем меню настроек отображения
{
  enable_refresh=FALSE;
  gtk_widget_destroy(picture_menu);
  //   gtk_widget_grab_focus (win);
  if (need_refresh)
  {
    char *current_name=strdup(current.name);
    (void)load_image(current_name, panel, TRUE, &current); // Повторно загружаем и показываем картинку для учёта изменений
    (void)show_image(&current, panel, TRUE, current_page, 0); // Ноль потому, что мы не знаем, что именно изменилось в настройках. FIXME: сделать дополнительную переменную чтобы знали!
    free(current_name);
    viewed_pages--; // Откатываем назад счётчик страниц - после открытия-закрытия меню он не должен изменяться
  }
  enable_refresh=TRUE;
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_local();
  //   g_signal_handlers_unblock_by_func( win, focus_in_callback, NULL );
  //   g_signal_handlers_unblock_by_func( win, focus_out_callback, NULL );
}

gint keys_in_picture_menu (GtkWidget *dialog, GdkEventKey *event, struct_panel *panel) //задействует кнопки
{
  if (check_key_press(event->keyval, panel)) return TRUE;
  switch (event->keyval){
    case   KEY_MENU:
    case   GDK_m:
    case   KEY_BACK:
    case   KEY_MENU_LIBROII:
    case   KEY_MENU_QT:
      picture_menu_destroy (panel);
      return FALSE;

    case   KEY_REFRESH_LIBROII:
    case   KEY_REFRESH_QT:
      e_ink_refresh_full();
      return FALSE;

    case   KEY_OK:
      if (dialog==loop_dir_none || dialog==loop_dir_loop || dialog==loop_dir_next || dialog==loop_dir_exit)
      {
        if (dialog==loop_dir_none) loop_dir = LOOP_NONE;
        if (dialog==loop_dir_loop) loop_dir = LOOP_LOOP;
        if (dialog==loop_dir_next) loop_dir = LOOP_NEXT;
        if (dialog==loop_dir_exit) loop_dir = LOOP_EXIT;
        write_config_int("loop_dir", loop_dir);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog), TRUE);
        wait_for_draw();
        if (QT) usleep (QT_REFRESH_DELAY); else usleep(GTK_REFRESH_DELAY);
        e_ink_refresh_local ();
        return TRUE;
      }
      return FALSE;

    case   KEY_UP:
      if (dialog==loop_dir_none || dialog==loop_dir_loop || dialog==loop_dir_next || dialog==loop_dir_exit)
      {
        if (dialog == loop_dir_none) gtk_widget_grab_focus(double_refresh_image);
        if (dialog == loop_dir_loop) gtk_widget_grab_focus(loop_dir_none);
        if (dialog == loop_dir_next) gtk_widget_grab_focus(loop_dir_loop);
        if (dialog == loop_dir_exit) gtk_widget_grab_focus(loop_dir_next);
        if (QT) usleep (QT_REFRESH_DELAY); else usleep(GTK_REFRESH_DELAY);
        e_ink_refresh_local ();
        return TRUE;
      }

      if (gtk_widget_is_focus (crop_image))
      {
        gtk_widget_grab_focus (viewed);
        e_ink_refresh_local ();
        return TRUE;
      }
      else if (gtk_widget_is_focus (frame_image) && GTK_WIDGET_SENSITIVE(crop_image) == FALSE)
      {
        gtk_widget_grab_focus (viewed);
        e_ink_refresh_local ();
        return TRUE;
      }
      else if (gtk_widget_is_focus (preload_enabled_button))
      {
        gtk_widget_grab_focus (loop_dir_exit);
        e_ink_refresh_local ();
        return TRUE;
      }
      else if (gtk_widget_is_focus (overlap_scale))
      {
        gtk_widget_grab_focus (frame_image);
        e_ink_refresh_local ();
        return TRUE;
      }

      e_ink_refresh_local ();
      return FALSE;

    case   KEY_DOWN:
      if (dialog==loop_dir_none || dialog==loop_dir_loop || dialog==loop_dir_next || dialog==loop_dir_exit)
      {
        if (dialog == loop_dir_none) gtk_widget_grab_focus(loop_dir_loop);
        if (dialog == loop_dir_loop) gtk_widget_grab_focus(loop_dir_next);
        if (dialog == loop_dir_next) gtk_widget_grab_focus(loop_dir_exit);
        if (dialog == loop_dir_exit) gtk_widget_grab_focus(preload_enabled_button);
        if (QT) usleep (QT_REFRESH_DELAY); else usleep(GTK_REFRESH_DELAY);
        e_ink_refresh_local ();
        return TRUE;
      }

      if (gtk_widget_is_focus (viewed))
      {
        if (GTK_WIDGET_SENSITIVE(crop_image))
          gtk_widget_grab_focus (crop_image);
        else if (GTK_WIDGET_SENSITIVE(frame_image))
          gtk_widget_grab_focus (frame_image);
        else
          gtk_widget_grab_focus (manga_mode);
        wait_for_draw();
        if (QT) usleep (QT_REFRESH_DELAY);
        e_ink_refresh_local ();
        return TRUE;
      }
      else if (gtk_widget_is_focus (overlap_scale))
      {
        gtk_widget_grab_focus (keepaspect_image);
        e_ink_refresh_local ();
        return TRUE;
      }
      else if (gtk_widget_is_focus (double_refresh_image))
      {
        gtk_widget_grab_focus (loop_dir_none);
        e_ink_refresh_local ();
        return TRUE;
      }

      e_ink_refresh_local ();
      return FALSE;

    default:
      if (QT) usleep (QT_REFRESH_DELAY);
      e_ink_refresh_local ();
      return FALSE;
  }
}

void loop_dir_toggler () // Callback для радиобаттона по действию при окончании каталога
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loop_dir_none))) loop_dir = LOOP_NONE;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loop_dir_loop))) loop_dir = LOOP_LOOP;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loop_dir_next))) loop_dir = LOOP_NEXT;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loop_dir_exit))) loop_dir = LOOP_EXIT;
  write_config_int("loop_dir", loop_dir);
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  e_ink_refresh_part ();
}

void start_picture_menu (struct_panel *panel, GtkWidget *win) // Создаём меню настроек картинки
{
  GtkWidget *menu_vbox = gtk_vbox_new (FALSE, 0);
  char *battery_capacity, *name, *viewed_count, *viewed_text;
  need_refresh=FALSE;

  crop_image = gtk_check_button_new_with_label (CROP_IMAGE);
  add_toggle_button_to_menu(crop_image, menu_vbox, crop_image_toggler, keys_in_picture_menu, crop, panel);

  split_spreads_button = gtk_check_button_new_with_label(SPLIT_DOUBLE_PAGES);
  add_toggle_button_to_menu(split_spreads_button, menu_vbox, split_spreads_toggler, keys_in_picture_menu, split_spreads, panel);

  manga_mode = gtk_check_button_new_with_label(MANGA_MODE);
  add_toggle_button_to_menu(manga_mode, menu_vbox, manga_mode_toggler, keys_in_picture_menu, manga, panel);

  rotate_image = gtk_check_button_new_with_label(ROTATE_IMAGE);
  add_toggle_button_to_menu(rotate_image, menu_vbox, rotate_image_toggler, keys_in_picture_menu, rotate, panel);

  web_manga_mode_checkbox = gtk_check_button_new_with_label(WEB_MANGA_MODE);
  add_toggle_button_to_menu(web_manga_mode_checkbox, menu_vbox, web_manga_mode_toggler, keys_in_picture_menu, web_manga_mode, panel);
  
  frame_image = gtk_check_button_new_with_label(FRAME_IMAGE);
  add_toggle_button_to_menu(frame_image, menu_vbox, frame_image_toggler, keys_in_picture_menu, frame, panel);

  overlap_frame = gtk_frame_new (OVERLAP_VALUE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), overlap_frame, FALSE, TRUE, 0);
  overlap_scale = gtk_hscale_new_with_range (0, 50, 1);
  gtk_range_set_value (GTK_RANGE(overlap_scale), (gdouble) overlap);
  g_signal_connect(overlap_scale, "value-changed", G_CALLBACK(overlap_changed), NULL);
  g_signal_connect(overlap_scale, "key_press_event", G_CALLBACK(keys_in_picture_menu), NULL);
  gtk_container_add (GTK_CONTAINER (overlap_frame), overlap_scale);

  keepaspect_image = gtk_check_button_new_with_label(KEEP_ASPECT);
  add_toggle_button_to_menu(keepaspect_image, menu_vbox, keepaspect_image_toggler, keys_in_picture_menu, keepaspect, panel);

  double_refresh_image = gtk_check_button_new_with_label(DOUBLE_REFRESH);
  add_toggle_button_to_menu(double_refresh_image, menu_vbox, double_refresh_toggler, keys_in_picture_menu, double_refresh, panel);

  loop_dir_frame = gtk_frame_new (ACTION_ON_LAST_FILE);
  gtk_box_pack_start (GTK_BOX (menu_vbox), loop_dir_frame, FALSE, TRUE, 0);
  loop_dir_vbox = gtk_vbox_new (TRUE, 0);
  gtk_container_add (GTK_CONTAINER (loop_dir_frame), loop_dir_vbox);
  loop_dir_none = gtk_radio_button_new_with_label (NULL, DO_NOTHING);
  if (loop_dir == LOOP_NONE)
    add_toggle_button_to_menu(loop_dir_none, loop_dir_vbox, NULL, keys_in_picture_menu, TRUE, panel);
  else
    add_toggle_button_to_menu(loop_dir_none, loop_dir_vbox, NULL, keys_in_picture_menu, FALSE, panel);
  loop_dir_loop = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (loop_dir_none), LOOP_DIRECTORY);
  if (loop_dir == LOOP_LOOP)
    add_toggle_button_to_menu(loop_dir_loop, loop_dir_vbox, NULL, keys_in_picture_menu, TRUE, panel);
  else
    add_toggle_button_to_menu(loop_dir_loop, loop_dir_vbox, NULL, keys_in_picture_menu, FALSE, panel);
  loop_dir_next = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (loop_dir_none), NEXT_DIRECTORY);
  if (loop_dir == LOOP_NEXT)
    add_toggle_button_to_menu(loop_dir_next, loop_dir_vbox, NULL, keys_in_picture_menu, TRUE, panel);
  else
    add_toggle_button_to_menu(loop_dir_next, loop_dir_vbox, NULL, keys_in_picture_menu, FALSE, panel);
  loop_dir_exit = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (loop_dir_none), EXIT_TO_FILEMANAGER);
  if (loop_dir == LOOP_EXIT)
    add_toggle_button_to_menu(loop_dir_exit, loop_dir_vbox, NULL, keys_in_picture_menu, TRUE, panel);
  else
    add_toggle_button_to_menu(loop_dir_exit, loop_dir_vbox, NULL, keys_in_picture_menu, FALSE, panel);
  (void)g_signal_connect (G_OBJECT (loop_dir_none), "group-changed", G_CALLBACK (loop_dir_toggler), NULL); // Один callback на всех


  preload_enabled_button = gtk_check_button_new_with_label(ALLOW_PRELOADING);
  add_toggle_button_to_menu(preload_enabled_button, menu_vbox, preload_toggler, keys_in_picture_menu, preload_enable, panel);

  caching_enabled_button = gtk_check_button_new_with_label(ALLOW_CACHING);
  add_toggle_button_to_menu(caching_enabled_button, menu_vbox, caching_toggler, keys_in_picture_menu, caching_enable, panel);

  HD_scaling_button = gtk_check_button_new_with_label(HD_SCALING);
  add_toggle_button_to_menu(HD_scaling_button, menu_vbox, HD_scaling_callback, keys_in_picture_menu, HD_scaling, panel);

  boost_contrast_button = gtk_check_button_new_with_label(BOOST_CONTRAST);
  add_toggle_button_to_menu(boost_contrast_button, menu_vbox, boost_contrast_callback, keys_in_picture_menu, boost_contrast, panel);
  if (QT == FALSE)
  {
    suppress_panel_button = gtk_check_button_new_with_label (SUPPRESS_BATTERY_WARNINGS);
    add_toggle_button_to_menu(suppress_panel_button, menu_vbox, suppress_panel_callback, keys_in_picture_menu, suppress_panel, panel);
  }

  read_string(BATTERY_CAPACITY, &battery_capacity); // Заряд в процентах
  asprintf(&name, BATTERY_CHARGE_PERCENT, battery_capacity);
  power_information_button=gtk_button_new_with_label (name);
  add_toggle_button_to_menu(power_information_button, menu_vbox, power_information, keys_in_picture_menu, FALSE, panel);
  xfree(&battery_capacity);
  xfree(&name);

  viewed_count=itoa(viewed_pages);
  viewed_text=xconcat(VIEWED_PAGES, viewed_count);
  viewed = gtk_button_new_with_label (viewed_text);
  add_toggle_button_to_menu(viewed, menu_vbox, reset_statistics, keys_in_picture_menu, FALSE, panel);
  free(viewed_count);
  free(viewed_text);

  if (split_spreads == FALSE)
  { // Если деление разворотов отключено
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(manga_mode), FALSE); // Выключаем галку режима манги
    gtk_widget_set_sensitive(manga_mode, FALSE); // И блокируем её
  }

  if (rotate == FALSE && web_manga_mode == FALSE)
  {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(frame_image), FALSE); // Выключаем галку режима покадрового просмотра
    gtk_widget_set_sensitive(frame_image, FALSE); // И блокируем её
    gtk_widget_set_sensitive(overlap_frame, FALSE);
    gtk_widget_set_sensitive(overlap_scale, FALSE);
  }
  
  if (frame)
  {
    gtk_widget_set_sensitive(rotate_image, FALSE);
    gtk_widget_set_sensitive(web_manga_mode_checkbox, FALSE);
  }
  else
  {
    if (web_manga_mode)
    {    
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(split_spreads_button), FALSE);
      gtk_widget_set_sensitive(split_spreads_button, FALSE);
      gtk_widget_set_sensitive(rotate_image, FALSE);
    }
    
    if (rotate)
      gtk_widget_set_sensitive(web_manga_mode_checkbox, FALSE);
  }

  if (manga) // Если включен режим манги - блокируем деление разворотов
    gtk_widget_set_sensitive(split_spreads_button, FALSE);

  picture_menu = gtk_dialog_new_with_buttons (SETTINGS, GTK_WINDOW(win),
                                              GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_NO_SEPARATOR,
                                              NULL);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(picture_menu)->vbox), menu_vbox);
  gtk_window_set_position (GTK_WINDOW(picture_menu), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_widget_show_all (picture_menu);
  wait_for_draw();
  e_ink_refresh_local();
}

// **************************************************  Options menu  ***********************************************************

void fm_start () // Callback для галки включения ФМ в настройках
{
  if ((fm_toggle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fmanager)))) {
    second_panel_show();
    update(&bottom_panel);
    gtk_widget_set_sensitive(copy, TRUE);
    gtk_widget_set_sensitive(moving, TRUE);
    write_config_int ("fm_toggle", fm_toggle);
  } else {
    enable_refresh=FALSE;
    #ifdef debug_printf
    printf("hiding second panel\n");
    #endif
    gtk_widget_destroy (bottom_panel.path_label);
    gtk_widget_destroy (bottom_panel.table);
    inactive_panel=NULL;
    write_config_string("bottom_panel.selected_name", bottom_panel.selected_name);
    gtk_widget_set_sensitive(copy, FALSE);
    gtk_widget_set_sensitive(moving, FALSE);
    if (!QT) usleep(GTK_REFRESH_DELAY);
    enable_refresh=TRUE;
    e_ink_refresh_full();
  }
  write_config_int ("fm_toggle", fm_toggle);
}

void move_confirm ()
{
  write_config_int("move_toggle", move_toggle=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(move_chk)));
  e_ink_refresh_part ();
}

void clock_panel_toggler ()
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(clock_panel))) {
    show_clock = TRUE;
    gtk_window_unfullscreen  (GTK_WINDOW(main_window));
  } else {
    show_clock = FALSE;
    gtk_window_fullscreen  (GTK_WINDOW(main_window));
  }
  write_config_int("show_clock", show_clock);
  e_ink_refresh_part ();
}

void type_refresh ()
{
  write_config_int("speed_toggle", speed_toggle=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ink_speed)));
  e_ink_refresh_part ();
}

void show_hidden_files_callback ()
{
  enable_refresh=FALSE;
  interface_is_locked=TRUE;
  write_config_int("show_hidden_files", show_hidden_files=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_hidden_files_chk)));
  update(&top_panel);
  if (fm_toggle) update(&bottom_panel);
  wait_for_draw();
  interface_is_locked=FALSE;
  enable_refresh=TRUE;
  e_ink_refresh_default();
}

void LED_notify_callback ()
{
  write_config_int("LED_notify", LED_notify = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(LED_notify_checkbox)));
  e_ink_refresh_part ();
}

void reset_configuration_callback() // Callback для кнопки сброса конфигов
{
  if(confirm_request(RESET_CONFIGURATION"?", GTK_STOCK_OK, GTK_STOCK_CANCEL))
  {
    #ifdef debug_printf
    printf("Resetting configuration!\n");
    #endif
    reset_config();
    gtk_main_quit();
    if (QT) xsystem("killall Xfbdev");
    #ifdef debug_printf
    printf("Resetting configuration done, bye!\n");
    #endif
  }
  e_ink_refresh_local();
}

void backlight_changed(GtkWidget *scalebutton)
{
  write_config_int("backlight", backlight = gtk_range_get_value(GTK_RANGE(scalebutton)));
  set_brightness(backlight);
  e_ink_refresh_default();
  #ifdef debug_printf
  printf("Brightness set to %d\n", backlight);
  #endif
}

void sleep_timeout_changed(GtkWidget *scalebutton)
{
  write_config_int("sleep_timeout", sleep_timeout = gtk_range_get_value(GTK_RANGE(scalebutton)));
  sleep_timer=sleep_timeout;
  #ifdef debug_printf
  printf("Sleep timeout set to %d\n", sleep_timeout);
  #endif
  e_ink_refresh_default();
  if(pthread_kill(sleep_timer_tid, 0) == ESRCH) // Если поток таймера умер
    start_sleep_timer();
}

#ifdef debug_printf
static void led_changed(GtkWidget *scalebutton)
{
  set_led_state(gtk_range_get_value(GTK_RANGE(scalebutton)));
}
#endif

void about_program_callback() // Callback для кнопки информации о программе
{
  Message(ABOUT_PROGRAM, ABOUT_PROGRAM_TEXT);
}

void options_destroy (GtkWidget *dialog) // Уничтожаем меню настроек ФМ
{
  enable_refresh=FALSE;
  gtk_widget_destroy(dialog);
  wait_for_draw();
  if (QT) usleep (QT_REFRESH_DELAY);
  enable_refresh=TRUE;
  e_ink_refresh_local();
}

gint keys_in_options (GtkWidget *dialog, GdkEventKey *event, struct_panel *panel)
{
  if (check_key_press(event->keyval, panel)) return TRUE;
  switch (event->keyval){
    case   KEY_MENU:
    case   GDK_m:
    case   KEY_BACK:
    case   KEY_MENU_LIBROII:
    case   KEY_MENU_QT:
      options_destroy (options_menu);
      return FALSE;

    case   KEY_REFRESH_LIBROII:
    case   KEY_REFRESH_QT:
      e_ink_refresh_full();
      return FALSE;

    case KEY_UP:
      if (dialog == sleep_timeout_scale)
      {
        if(hardware_has_backlight)
          gtk_widget_grab_focus (backlight_scale);
        else
          gtk_widget_grab_focus (LED_notify_checkbox);
        e_ink_refresh_local();
        return TRUE;
      }

      if (dialog == backlight_scale)
      {
        gtk_widget_grab_focus (LED_notify_checkbox);
        e_ink_refresh_local();
        return TRUE;
      }

      if (gtk_widget_is_focus (fmanager))
      {
        gtk_widget_grab_focus (about_program);
        e_ink_refresh_local();
        return TRUE;
      }
      e_ink_refresh_local();
      return FALSE;

    case KEY_DOWN:
      if (dialog == sleep_timeout_scale)
      {
        gtk_widget_grab_focus (reset_configuration);
        e_ink_refresh_local();
        return TRUE;
      }

      if (dialog == backlight_scale)
      {
        gtk_widget_grab_focus (sleep_timeout_scale);
        e_ink_refresh_local();
        return TRUE;
      }

      if (gtk_widget_is_focus (about_program))
      {
        gtk_widget_grab_focus (fmanager);
        e_ink_refresh_local();
        return TRUE;
      }
      e_ink_refresh_local();
      return FALSE;

    case KEY_OK:
    default:
      return FALSE;
  }
}

void options_menu_create(void) //Создание меню опций в ФМ
{
  GtkWidget *sleep_timeout_frame;
  options_menu = gtk_dialog_new_with_buttons (SETTINGS, GTK_WINDOW(main_menu),
                                              GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_NO_SEPARATOR, NULL);
  GtkWidget *menu_vbox = gtk_vbox_new (FALSE, 0);

  fmanager = gtk_check_button_new_with_label (FILEMANAGER_MODE);
  add_toggle_button_to_menu(fmanager, menu_vbox, fm_start, keys_in_options, fm_toggle, active_panel);

  ink_speed = gtk_check_button_new_with_label (PARTIAL_UPDATE);
  add_toggle_button_to_menu(ink_speed, menu_vbox, type_refresh, keys_in_options, speed_toggle, active_panel);

  move_chk = gtk_check_button_new_with_label (CONFIRM_MOVE);
  add_toggle_button_to_menu(move_chk, menu_vbox, move_confirm, keys_in_options, move_toggle, active_panel);

  show_hidden_files_chk = gtk_check_button_new_with_label (SHOW_HIDDEN_FILES);
  add_toggle_button_to_menu(show_hidden_files_chk, menu_vbox, show_hidden_files_callback, keys_in_options, show_hidden_files, active_panel);

  if (QT == FALSE)
  {
    clock_panel = gtk_check_button_new_with_label(SHOW_PANEL);
    add_toggle_button_to_menu(clock_panel, menu_vbox, clock_panel_toggler, keys_in_options, show_clock, active_panel);
  }

  LED_notify_checkbox = gtk_check_button_new_with_label (LED_NOTIFY);
  add_toggle_button_to_menu(LED_notify_checkbox, menu_vbox, LED_notify_callback, keys_in_options, LED_notify, active_panel);

  if (hardware_has_backlight)
  {
    GtkWidget *backlight_frame = gtk_frame_new (BACKLIGHT);

    gtk_box_pack_start (GTK_BOX (menu_vbox), backlight_frame, FALSE, TRUE, 0);
    backlight_scale = gtk_hscale_new_with_range ((gdouble)0, (gdouble)8, (gdouble)1);
    (void)g_signal_connect(backlight_scale, "value-changed", G_CALLBACK(backlight_changed), NULL);
    gtk_range_set_value (GTK_RANGE(backlight_scale), (gdouble) backlight);
    gtk_container_add (GTK_CONTAINER (backlight_frame), backlight_scale);
    (void)g_signal_connect (G_OBJECT (backlight_frame), "key_press_event", G_CALLBACK (keys_in_options), NULL);
  }

  if (QT)
  {
    sleep_timeout_frame = gtk_frame_new (SLEEP_TIMEOUT);
    gtk_box_pack_start (GTK_BOX (menu_vbox), sleep_timeout_frame, FALSE, TRUE, 0);
    sleep_timeout_scale = gtk_hscale_new_with_range ((gdouble)0,(gdouble) 600,(gdouble) 5);
    gtk_range_set_value (GTK_RANGE(sleep_timeout_scale), (gdouble)sleep_timeout);
    (void)g_signal_connect (G_OBJECT (sleep_timeout_scale), "key_press_event", G_CALLBACK (keys_in_options), NULL);
    (void)g_signal_connect(sleep_timeout_scale, "value-changed", G_CALLBACK(sleep_timeout_changed), NULL);
    gtk_container_add (GTK_CONTAINER (sleep_timeout_frame), sleep_timeout_scale);
  }

  #ifdef debug_printf
  GtkWidget *LED_test_frame = gtk_frame_new ("LED_TEST");
  gtk_box_pack_start (GTK_BOX (menu_vbox), LED_test_frame, FALSE, TRUE, 0);
  GtkWidget *led_test_scale = gtk_hscale_new_with_range (0, 255, 1);
  g_signal_connect(led_test_scale, "value-changed", G_CALLBACK(led_changed), NULL);
  gtk_container_add (GTK_CONTAINER (LED_test_frame), led_test_scale);
  #endif

  reset_configuration = gtk_button_new_with_label ("   "RESET_CONFIGURATION);
  add_toggle_button_to_menu(reset_configuration, menu_vbox, reset_configuration_callback, keys_in_options, FALSE, active_panel);

  about_program = gtk_button_new_with_label ("   "ABOUT_PROGRAM);
  add_toggle_button_to_menu(about_program, menu_vbox, about_program_callback, keys_in_options, FALSE, active_panel);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(options_menu)->vbox), menu_vbox);
  gtk_window_set_position (GTK_WINDOW(options_menu), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_widget_show_all (options_menu);
  e_ink_refresh_local();
}

// ********************************  Main menu!  **************************************
void create_folder(void)
{
  enable_refresh=FALSE;
  interface_is_locked=TRUE;
  xsystem("mkdir -p temp000");
  if ((inactive_panel != NULL) && (strcmp (active_panel->path, inactive_panel->path) == 0))
    update(inactive_panel);
  update(active_panel);
  enable_refresh=TRUE;
  interface_is_locked=FALSE;
  wait_for_draw();
  e_ink_refresh_local ();
}

void menu_destroy (GtkWidget *dialog)
{
  #ifdef debug_printf
  printf("Destroying menu\n");
  #endif
  gtk_widget_destroy(dialog);
  gtk_widget_grab_focus (GTK_WIDGET(active_panel->list));
  /*   g_signal_handlers_unblock_by_func( window, focus_in_callback, NULL ); */
  /*   g_signal_handlers_unblock_by_func( window, focus_out_callback, NULL ); */
}

gint keys_in_main_menu (GtkWidget *dialog, GdkEventKey *event, struct_panel *panel) //задействует кнопку М в меню
{
  if (check_key_press(event->keyval, panel)) return TRUE;
  switch (event->keyval){
    case   KEY_BACK:
    case   KEY_MENU:
    case   KEY_MENU_LIBROII:
    case   KEY_MENU_QT:
    case   GDK_m:
      interface_is_locked=TRUE;
      menu_destroy (main_menu);
      (void)chdir(active_panel->path);
      wait_for_draw();
      if (QT) usleep(QT_REFRESH_DELAY);
      e_ink_refresh_local();
      interface_is_locked=FALSE;
      return TRUE;

    case   KEY_REFRESH_LIBROII:
    case   KEY_REFRESH_QT:
      e_ink_refresh_full();
      return FALSE;

    case   KEY_UP:
      if (dialog == create)
      {
        gtk_widget_grab_focus (exit_button);
        e_ink_refresh_local();
        return TRUE;
      }
      else if ((dialog == options) && GTK_WIDGET_SENSITIVE(create) == FALSE)
      {
        printf("options\n");
        gtk_widget_grab_focus (exit_button);
        e_ink_refresh_local();
        return TRUE;
      }
      e_ink_refresh_local();
      return FALSE;

    case   KEY_DOWN:
      if (dialog == exit_button)
      {
        if (GTK_WIDGET_SENSITIVE(create))
          gtk_widget_grab_focus (create);
        else
          gtk_widget_grab_focus (options);
        e_ink_refresh_local();
        return TRUE;
      }
      e_ink_refresh_local();
      return FALSE;

    default:
      return FALSE;
  }
}

void start_main_menu (struct_panel *panel)
{
  enable_refresh=FALSE;
  main_menu = gtk_dialog_new_with_buttons (MAIN_MENU, GTK_WINDOW(main_window),
                                           GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_NO_SEPARATOR, NULL);
  GtkWidget *menu_vbox = gtk_vbox_new (TRUE, 0);

  create = gtk_button_new_with_label (CREATE_TEMPORARY_DIRECTORY);
  add_toggle_button_to_menu(create, menu_vbox, create_folder, keys_in_main_menu, FALSE, panel);
  if (active_panel->archive_depth > 0 ) gtk_widget_set_sensitive(create, FALSE); // В архиве не поддерживается

  copy = gtk_button_new_with_label (COPY);
  add_toggle_button_to_menu(copy, menu_vbox, copy_dir_or_file, keys_in_main_menu, FALSE, panel);
  if (top_panel.archive_depth > 0 || bottom_panel.archive_depth > 0) gtk_widget_set_sensitive(copy, FALSE); // В архиве не поддерживается
  if (fm_toggle == FALSE) gtk_widget_set_sensitive(copy, FALSE);

  moving = gtk_button_new_with_label (MOVE_FILE);
  add_toggle_button_to_menu(moving, menu_vbox, move_dir_or_file, keys_in_main_menu, FALSE, panel);
  if (top_panel.archive_depth > 0 || bottom_panel.archive_depth > 0) gtk_widget_set_sensitive(moving, FALSE); // В архиве не поддерживается
  if (fm_toggle == FALSE) gtk_widget_set_sensitive(moving, FALSE);

  del = gtk_button_new_with_label (DELETE);
  add_toggle_button_to_menu(del, menu_vbox, delete_dir_or_file, keys_in_main_menu, FALSE, panel);
  if (active_panel->archive_depth > 0) gtk_widget_set_sensitive(del, FALSE); // В архиве не поддерживается

  options = gtk_button_new_with_label (OPTIONS);
  add_toggle_button_to_menu(options, menu_vbox, options_menu_create, keys_in_main_menu, FALSE, panel);

  exit_button = gtk_button_new_with_label (EXIT);
  add_toggle_button_to_menu(exit_button, menu_vbox, shutdown, keys_in_main_menu, FALSE, panel);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(main_menu)->vbox), menu_vbox);
  gtk_window_set_position (GTK_WINDOW(main_menu), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_widget_show_all (main_menu);
  wait_for_draw();
  enable_refresh=TRUE;
  if (!QT) usleep(GTK_REFRESH_DELAY);
  e_ink_refresh_local();
}
