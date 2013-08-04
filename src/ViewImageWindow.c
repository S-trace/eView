/* Norin Maxim, 2011, Soul Trace, 2013 Distributed under GPLv2 Terms.
 * ImageViewew for Digma E600 & compatible*/
#include "digma_hw.h"
#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgen.h> //basename()
#include <gtk/gtk.h>
#include "gtk_file_manager.h"
#include "mygtk.h"
#include "mylib.h"
#include "ViewImageWindow.h"
#include "crop.h"
#include "cfg.h"
// #include "debug_msg_win.h"
#include "frames_search.h"
#include "shift.h"
#include "archive_handler.h"
#include "translations.h"
#include "interface.h"

typedef struct { char *name; GdkPixbuf *pixbuf; } image;
static image preloaded;//Предзагруженное изображение
static gint width_real;
static gint height_real;
static GtkObject *adjust;
static GtkWidget *scrolled_window;
static GtkWidget *win;
static GtkWidget *gimage;
static GdkPixbuf *pixbuf_image;
static GdkPixbuf *pixbuf_key;//промежуточный
static GdkPixbuf *pixbuf_rotate;//промежуточный для вращения
static int shift_val; //на сколько сдвигать картинку
static int fr;    //количество найденых кадров
static double ar; //aspect rate
static int value; //текущая позиция сдвига
static int move_left_to_left; //флаг слайдера, нужен только для правильного обновления на книге

// void print_adjust(GtkAdjustment *adjust, gpointer data)
// { // это только для отладки
//
// g_print("      изменение значения adjust %f\n", gtk_adjustment_get_value         (GTK_ADJUSTMENT(adjust)));
// }

void pixbuf_unref(GdkPixbuf *pixbuf)
{
  if (G_IS_OBJECT(pixbuf)) g_object_unref(pixbuf);
}

void die_viewer_window (void)
{
  reset_preloaded_image(); // Очищаем предзагруженные данные
  gtk_widget_destroy(win);
  if (suppress_panel && ! QT)
    start_panel();
  enable_refresh=FALSE;
  wait_state();
  enable_refresh=TRUE;
  wait_for_draw();
  e_ink_refresh_full();  
}

void reset_preloaded_image(void)
{
  #ifdef debug_printf
  printf("Resetting preloaded data!\n");
  #endif
  preloaded.name = strdup("");
  pixbuf_unref(preloaded.pixbuf);
}

void preload_image(char *new_file, panel *panel)
{
  GdkPixbuf *pixbuf_saved;
  if (new_file==NULL) return;
  if (! is_picture(new_file)) return;
  #ifdef debug_printf
  printf("Preloading '%s'\n", new_file);
  printf ("pixbuf_unref(preloaded_image)\n");
  #endif
  pixbuf_unref(preloaded.pixbuf);
  preloaded.name=strdup(new_file);
  if (panel->archive_depth > 0)
  {
    new_file=basename(new_file); // Хрен уследишь, откуда с путём прилетит, а откуда без!
    new_file=xconcat(panel->archive_cwd, new_file);
    archive_extract_file(panel->archive_stack[panel->archive_depth], new_file, "/tmp/");
    new_file=xconcat("/tmp/", new_file);
    pixbuf_key = gdk_pixbuf_new_from_file (new_file, NULL);    
    #ifdef debug_printf
    printf("Removing extracted '%s'\n", new_file);
    #endif
    remove(new_file);
  }
  else
    pixbuf_key = gdk_pixbuf_new_from_file (new_file, NULL);
  #ifdef debug_printf
  if (GDK_IS_PIXBUF(pixbuf_key))
    printf("Preloaded file successfully!\n");
  else
    printf("Preloading file failed!\n");
  #endif
  pixbuf_saved = pixbuf_image;
  image_resize (rotate, crop, keepaspect);
  preloaded.pixbuf = pixbuf_image;
  pixbuf_image = pixbuf_saved;
  #ifdef debug_printf
  if (GDK_IS_PIXBUF(preloaded.pixbuf))
    printf("Preloaded done successfully\n");
  else
    printf("Preloading image failed!\n");
  #endif
  return;
}

gboolean show_image(char *filename, panel *panel) // Показываем картинку
{
  #ifdef debug_printf
  printf("Going to show '%s'\n", filename);
  #endif
  if (strcmp(preloaded.name,filename) != 0) //Если предзагруженное изображение и текущее не совпали
  {
    if (GDK_IS_PIXBUF(preloaded.pixbuf))//Если в пиксбуфе что-то лежало
    {
      #ifdef debug_printf
      printf("PRELOADED IMAGE IS WRONG!!! have '%s', want '%s'\n",preloaded.name,filename);
      #endif
      reset_preloaded_image();
    }
    if (panel->archive_depth > 0)
    {
      filename=basename(filename); // Хрен уследишь, откуда с путём прилетит, а откуда без!
      filename=xconcat(panel->archive_cwd, filename);
      archive_extract_file(panel->archive_stack[panel->archive_depth], filename, "/tmp/");
      filename=xconcat("/tmp/", filename);
    }
    if (filename == NULL || (pixbuf_key=gdk_pixbuf_new_from_file (filename, NULL)) == NULL)
    {
      Message (ERROR, xconcat(UNABLE_TO_SHOW, filename));
      die_viewer_window ();
      return FALSE;
    }
    if (panel->archive_depth > 0)
    {
      #ifdef debug_printf
      printf("Removing extracted '%s'\n",filename);
      #endif
      remove(filename);
    }
    image_resize (rotate, crop, keepaspect);
    
    if (GTK_IS_WIDGET(gimage)) // Если виджет картинки существует (а он существует всегда кроме первого открытия картинки)
    {
      gtk_image_set_from_pixbuf (GTK_IMAGE(gimage), pixbuf_image);
      pixbuf_unref(pixbuf_image);
    }
  }
  else
  {
    #ifdef debug_printf
    printf("USING PRELOADED IMAGE\n");
    #endif
    if (GTK_IS_WIDGET(gimage)) // Если виджет картинки существует (а он существует всегда кроме первого открытия картинки)
    {
      #ifdef debug_printf
      if (GDK_IS_PIXBUF(preloaded.pixbuf))
        printf("preloaded_image correct\n");
      else
        printf("preloaded_image incorrect!\n");
      #endif
      gtk_image_set_from_pixbuf (GTK_IMAGE(gimage), preloaded.pixbuf);
      wait_for_draw(); // Ожидаем отрисовки всего
      //     pixbuf_unref(preloaded_image); // Не требуется - он будет освобождён при прелоаде следующего изображения, а тут - провоцирует варнинги и глюки!
    }
  }
  write_config_int("viewed_pages", ++viewed_pages); // Сохраняем на диск счётчик страниц
  panel->last_name=strdup(filename);
  if (panel == &top_panel)
    write_config_string("top_panel.last_name", panel->last_name); // И имена просмотренных страниц
    else
      write_config_string("bottom_panel.last_name", panel->last_name);
    move_selection(iter_from_filename (basename(panel->last_name), panel), panel);
  gtk_window_set_title(GTK_WINDOW(win), filename);
  wait_for_draw(); // Ожидаем отрисовки всего
  return TRUE;
}

gint which_key_press (__attribute__((unused))GtkWidget *window, GdkEventKey *event, panel *panel) //реакция на кнопки
{
  set_brightness(backlight);
  if (interface_is_locked)
  {
    #ifdef debug_printf
    printf("Interface was locked, keypress ignored!\n");
    #endif
    return TRUE;
  }
  char *new_file; //имя следующего файла с картинкой
  #ifdef debug_printf
  printf("Caught in viewer: %d\n",event->keyval);
  #endif
  switch (event->keyval){
    
    case KEY_PGDOWN://GDK_Right
    case KEY_RIGHT://GDK_Right
      interface_is_locked=TRUE; // Блокируем интерфейс на время длительной операции по показу картинки
      move_left_to_left = FALSE;
      if (frame && fr >= 2)// Действия при просмотре в покадровом режиме
      {
        value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adjust));
        if (height_real - width_display - 2 > value)
        {
          shift_val = shift (height_real, fr, value);
          gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), value + shift_val +2);
          if (double_refresh) e_ink_refresh_local ();
          e_ink_refresh_full ();
          interface_is_locked=FALSE; // Снимаем блокировку интерфейса
          return FALSE;
        }
        else move_left_to_left = TRUE;
      }
      else
      {
        if (rotate) // Действия при просмотре с поворотом
        {
          if (manga)
          {
            value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adjust)); // Получаем положение картинки в окне
            #ifdef debug_printf
            printf("In manga mode gtk_adjustment_get_value returned %d, but R_SHIFT=%d\n", value, R_SHIFT);
            #endif
            if (value == R_SHIFT) // Если она у правого края
            {
              #ifdef debug_printf
              printf("In manga mode scrolling to %d\n", L_SHIFT);
              #endif
              gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), L_SHIFT); // Перемещаем её в левый край
              value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adjust)); // Получаем положение картинки в окне
              wait_for_draw();
              #ifdef debug_printf
              printf("In manga mode gtk_adjustment_get_value returned new value %d\n", value);
              #endif
              if (double_refresh) e_ink_refresh_local();
              e_ink_refresh_full ();
              interface_is_locked=FALSE; // Снимаем блокировку интерфейса
              return FALSE;
            }            
          }
          else
          {
            value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adjust));
            if (value == L_SHIFT)
            {
              gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), R_SHIFT);
              if (double_refresh) e_ink_refresh_local();
              e_ink_refresh_full ();
              interface_is_locked=FALSE; // Снимаем блокировку интерфейса
              return FALSE;
            }
          }
        }
      }
      new_file = next_image (panel->selected_name, TRUE, panel);
      if (new_file==NULL) 
      {
        interface_is_locked=FALSE; // Снимаем блокировку интерфейса
        return FALSE;
      }
      panel->selected_name = basename(new_file);
      if (! is_picture(new_file)) 
      {
        interface_is_locked=FALSE; // Снимаем блокировку интерфейса
        return FALSE;
      }
      //g_print ("загрузка новой картинки\n");
      show_image (new_file, panel);
      if (frame && fr >= 2) 
        move_left_to_left = TRUE;
      if (manga)
        gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), R_SHIFT);
      else
        gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), L_SHIFT);
      if (move_left_to_left) move_left_to_left = FALSE;
      wait_for_draw(); // Ожидаем отрисовки всего
      if (panel == &top_panel)
        write_config_string("top_panel.last_name", panel->selected_name); // Сохраняем конфиги
        else
          write_config_string("bottom_panel.last_name", panel->selected_name);
        if (double_refresh) e_ink_refresh_local();
        e_ink_refresh_full ();
      if(preload_enable) // Предзагрузка
        preload_image(next_image (panel->selected_name, FALSE, panel), panel);
      interface_is_locked=FALSE; // Снимаем блокировку интерфейса
      return FALSE;
      break;
      
      case KEY_PGUP://GDK_Left:
      case KEY_LEFT://GDK_Left:
        interface_is_locked=TRUE; // Блокируем интерфейс на время длительной операции по показу картинки
        if (frame && fr >= 2)
        {
          value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adjust));
          if (value > 2) {
            shift_val = shift_back (height_real, fr, value) * -1;
            if (value + width_display > height_real) value = value -2;
            gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), value + shift_val);
            if (double_refresh) e_ink_refresh_local ();
            e_ink_refresh_full ();
            interface_is_locked=FALSE; // Снимаем блокировку интерфейса
            return FALSE;
          }
        }
        else
        {
          if (rotate)
          {
            value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adjust));
            if (manga)
            {
              if (value == L_SHIFT) {
                gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), R_SHIFT);
                if (double_refresh) e_ink_refresh_local();
                e_ink_refresh_full ();
                interface_is_locked=FALSE; // Снимаем блокировку интерфейса
                return FALSE;
              }
            }
            else
            {
              if (value == R_SHIFT) {
                gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), L_SHIFT);
                if (double_refresh) e_ink_refresh_local();
                e_ink_refresh_full ();
                interface_is_locked=FALSE; // Снимаем блокировку интерфейса
                return FALSE;
              }
            }
          }
        }
        if (rotate) gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), R_SHIFT);
        new_file = prev_image (panel->selected_name, TRUE, panel);
        if (new_file==NULL) 
        {
          interface_is_locked=FALSE; // Снимаем блокировку интерфейса
          return FALSE;
        }
        panel->selected_name = basename(new_file);
        
        if (! is_picture(panel->selected_name)) 
        {
          interface_is_locked=FALSE; // Снимаем блокировку интерфейса
          return FALSE;
        }
        show_image (panel->selected_name, panel);
        if (manga)
          gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), L_SHIFT);
        else
          gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), R_SHIFT);
        
        if (frame)
        {
          value = height_real + 2 - width_display;
          gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), value);
          e_ink_refresh_local();
          if (double_refresh) e_ink_refresh_full ();
        }
        if (double_refresh) e_ink_refresh_local ();
        e_ink_refresh_full ();
        if (panel == &top_panel)
          write_config_string("top_panel.last_name", panel->selected_name);
        else
          write_config_string("bottom_panel.last_name", panel->selected_name);
        if(preload_enable) // Предзагрузка
          preload_image(next_image (panel->selected_name, FALSE, panel), panel);
        interface_is_locked=FALSE; // Снимаем блокировку интерфейса
        return FALSE;
        break;
        
        case KEY_BACK://GDK_x:
          die_viewer_window();
          return FALSE;
          break;
          
        case   GDK_m:
        case   KEY_MENU:
        case   KEY_MENU_LIBROII:
        case   KEY_MENU_QT:
          start_picture_menu (panel, win); // открываем меню картинки
          return FALSE;
          break;
          
        case   KEY_REFRESH_LIBROII:
        case   KEY_REFRESH_QT:
          e_ink_refresh_full();
          return FALSE;
          
        default:
          #ifdef debug_printf
          printf("got unknown keycode %d in main\n", event->keyval);
          #endif  
          return FALSE;
  }
}

void image_rotate(int angle)//вращение 90 -против часовой стрелки
{
  pixbuf_rotate = gdk_pixbuf_rotate_simple (pixbuf_key, angle);
  pixbuf_unref(pixbuf_key);
  return;
}

void image_zoom_rotate (double width, double height) //оригинал слишком маленький - ничего не менять
{
  fr = 0;
  if (height < 300 && width < 200){
    return;
  }
  //оригинал слишком широкий и большой  - растянуть в ширину
  if (width > height && width >= width_display) {
    pixbuf_image = gdk_pixbuf_scale_simple (pixbuf_key, width_display * 2, height_display, GDK_INTERP_BILINEAR);
    pixbuf_unref(pixbuf_key);
    return;
  }
  
  if (frame) {
    fr = frames_search(pixbuf_key, width_real, height_real);
    if (fr >= 2){
      ar = height / width * height_display;
      pixbuf_rotate = gdk_pixbuf_scale_simple (pixbuf_key, height_display, ar, GDK_INTERP_BILINEAR);
      pixbuf_unref(pixbuf_key);
      width_real  = gdk_pixbuf_get_width  (pixbuf_rotate);
      height_real = gdk_pixbuf_get_height (pixbuf_rotate);
      fr = frames_search(pixbuf_rotate, width_real, height_real);
    } else {
      pixbuf_rotate = gdk_pixbuf_scale_simple (pixbuf_key, height_display, width_display * 2, GDK_INTERP_BILINEAR);
      pixbuf_unref(pixbuf_key);
    }
  } else {
    pixbuf_rotate = gdk_pixbuf_scale_simple (pixbuf_key, height_display, width_display * 2, GDK_INTERP_BILINEAR);
    pixbuf_unref(pixbuf_key);
  }
  pixbuf_image = gdk_pixbuf_rotate_simple (pixbuf_rotate, 90);
  pixbuf_unref(pixbuf_rotate);
}

void image_resize (int mode_rotate, int mode_crop, int keep_aspect) // изменение разрешения и модификация
{
  #ifdef debug_printf
  printf("image_resize called\n");
  #endif
  int x, y, w, h;
  // реальное разрешение картинки
  width_real  = gdk_pixbuf_get_width  (pixbuf_key);
  height_real = gdk_pixbuf_get_height (pixbuf_key);
  if (mode_crop && width_real>115 && height_real>115) {
    crop_image (pixbuf_key, width_real, height_real);
    if (return_crop_coord(0) != -1) {
      x = 0;// на всякий случай
      y = 0;
      w = width_real;
      h = height_real;
      x = return_crop_coord(0);
      y = return_crop_coord(1);
      w = return_crop_coord(2);
      h = return_crop_coord(3);
      
      pixbuf_image = gdk_pixbuf_new_subpixbuf(pixbuf_key, x, y, w, h);
      pixbuf_unref(pixbuf_key);
      width_real  = gdk_pixbuf_get_width  (pixbuf_image);
      height_real = gdk_pixbuf_get_height (pixbuf_image);
      pixbuf_key = gdk_pixbuf_copy (pixbuf_image);
      pixbuf_unref(pixbuf_image);
    }
  }
  
  if (mode_rotate) //удвоение размера с поворотом
  {
    image_zoom_rotate (width_real, height_real);
    return;
  }
  
  //если оригинал слишком широкий - повернуть на 90
  if (width_real>height_real){
    image_rotate(90);
    pixbuf_key = gdk_pixbuf_copy (pixbuf_rotate);
    pixbuf_unref(pixbuf_rotate);
    width_real  = gdk_pixbuf_get_width  (pixbuf_key);
    height_real = gdk_pixbuf_get_height (pixbuf_key);
  }
  
  // Скалировать оригинал до экрана
  if (keep_aspect)
  {
    double image_ar = (double)width_real/(double)height_real;     // Соотношение сторон картинки
    double display_ar = (double)(width_display)/(double)(height_display); // Соотношение сторон экрана
    #ifdef debug_printf
    printf("resolution: rx=%i ry=%i dar=%f iar=%f\n", width_real, height_real, display_ar, image_ar);
    #endif
    int scaled_x,scaled_y;
    if (display_ar < image_ar ) // Если рисунок шире экрана
    {
      scaled_x=width_display; // Ширина экрана
      scaled_y=width_display*height_real/width_real; // Высчитываем требуемую высоту
    }
    else // Если рисунок Уже экрана
    {
      scaled_y=height_display;// Высота экрана
      scaled_x=(int)((float)width_real*scaled_y/height_real);// Высчитываем требуемую ширину
    }
    #ifdef debug_printf
    printf("scaling pic from %dx%d to %dx%d\n",width_real,height_real,scaled_x,scaled_y);
    #endif
    pixbuf_image = gdk_pixbuf_scale_simple (pixbuf_key, scaled_x,scaled_y,GDK_INTERP_BILINEAR);
  }
  else
  {
    #ifdef debug_printf
    printf("aspect not preserved\n");
    #endif
    pixbuf_image = gdk_pixbuf_scale_simple (pixbuf_key, width_display, height_display, GDK_INTERP_BILINEAR);
  }
  pixbuf_unref(pixbuf_key);
  return;
}

void ViewImageWindow(char *file, panel *panel) //создание изображения через GdkPixbuf
{
  // Убираем хэндлер с обработки фокуса фэйлменеджером
  //   g_signal_handlers_disconnect_by_func( window, focus_in_callback, NULL );
  //   g_signal_handlers_disconnect_by_func( window, focus_out_callback, NULL );
  //   focus_in_processed=0;
  
  if (suppress_panel && ! QT)
    kill_panel();
  preloaded.name=strdup("");
  
  panel->selected_name=strdup(file);
  #ifdef debug_printf
  printf("Opening viewer for '%s'\n", panel->selected_name);
  #endif
  win = window_create (width_display, height_display, 0, "", NOT_MODAL);
  gtk_window_set_decorated (GTK_WINDOW(win), FALSE);
  //   g_signal_connect (G_OBJECT (win), "expose-event", G_CALLBACK (e_ink_refresh_full), NULL);
  #ifndef __amd64
  gtk_window_fullscreen  (GTK_WINDOW(win));  //блокировка окошка "часиков", нужно отключать для отображения на пк
  #endif
  g_signal_connect (G_OBJECT (win), "key_press_event", G_CALLBACK (which_key_press), panel);
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 0); // Хрен знает работает ли, но хуже не стало
  gtk_container_add (GTK_CONTAINER (win), scrolled_window);
  
  //GTK_POLICY_ALWAYS для отображения на пк так же как на книжке
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_NEVER, GTK_POLICY_NEVER);
  if (show_image(panel->selected_name, panel)) // Создаём пиксбуф
  {
    gimage = gtk_image_new_from_pixbuf (pixbuf_image);
    pixbuf_unref(pixbuf_image);
    adjust = gtk_adjustment_new (0.0, 0.0, 200.0, 0.1, 1.0, 1.0);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrolled_window), GTK_WIDGET(gimage));
    
    gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW(scrolled_window),
                                         GTK_ADJUSTMENT(adjust));
    //сигнал для проверки работы горизонтального сдвига рисунка
    // g_signal_connect (G_OBJECT (adjust), "value_changed", G_CALLBACK (print_adjust), NULL);
    if (panel == &top_panel)
      write_config_string("top_panel.last_name", panel->selected_name);
    else
      write_config_string("bottom_panel.last_name", panel->selected_name);
    gtk_widget_show_all(win);
    if (double_refresh) e_ink_refresh_local();
    e_ink_refresh_full ();
    //     g_signal_connect_after (GTK_WINDOW (win), "focus", G_CALLBACK (focus_in_callback), NULL);
    //     g_signal_connect (G_OBJECT (win), "focus-out-event", G_CALLBACK (focus_out_callback), NULL);
    if(preload_enable) // Предзагрузка
      preload_image(next_image (panel->selected_name, FALSE, panel), panel);
  }
}
