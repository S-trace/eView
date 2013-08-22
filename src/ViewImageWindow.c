/* Norin Maxim, 2011, Soul Trace, 2013 Distributed under GPLv2 Terms.
 * ImageViewew for Digma E600 & compatible*/
#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgen.h> /*basename() */
#include <gtk/gtk.h>
#include <stdlib.h>

#include "gtk_file_manager.h"
#include "digma_hw.h"
#include "mygtk.h"
#include "mylib.h"
#include "ViewImageWindow.h"
#include "crop.h"
#include "cfg.h"
/* #include "debug_msg_win.h" */
#include "frames_search.h"
#include "shift.h"
#include "archive_handler.h"
#include "translations.h"
#include "interface.h"

image current, screensaver, preloaded;
GtkObject *adjust;
GtkWidget *ImageWindow, *scrolled_window, *gimage;
int shift_val; /*на сколько сдвигать картинку */
int value;     /*текущая позиция сдвига */
int move_left_to_left; /*флаг слайдера, нужен только для правильного обновления на книге */
int in_picture_viewer=FALSE; /* Индикатор, что открыто окно картинки (для корректной работы скринсейвера) */

/* void print_adjust(GtkAdjustment *adjust, gpointer data) */
/* { // это только для отладки */
/* */
/* g_print("      изменение значения adjust %f\n", gtk_adjustment_get_value         (GTK_ADJUSTMENT(adjust))); */
/* } */

void update_image_dimentions(image *target) /* Перерассчёт размера изображения в структуре (вызывать каждый раз после изменения пиксбуфа) */
{
  target->width  = gdk_pixbuf_get_width  (target->pixbuf);
  target->height = gdk_pixbuf_get_height (target->pixbuf);
  target->aspect_rate = (double) target->width / (double) target->height;
}

void pixbuf_unref(GdkPixbuf *pixbuf)
{
  #ifdef debug_printf
  printf("pixbuf %p unreferenced\n", pixbuf);
  #endif  
  if (G_IS_OBJECT(pixbuf)) g_object_unref(pixbuf);
}

void die_viewer_window (void)
{
  #ifdef debug_printf
  printf("Destroying ViewImageWindow\n");
  #endif
  enable_refresh=FALSE;
  gtk_widget_destroy(ImageWindow);
  in_picture_viewer=FALSE;
  if ((suppress_panel == TRUE) && QT != TRUE)
    start_panel();
  wait_for_draw();
  enable_refresh=TRUE;
}

int check_image_settings(const image *const target) __attribute__((pure));
int check_image_settings(const image *const target)
{
  #ifdef debug_printf
  printf("Checking image %p settings\n", target);
  #endif
  if (target->valid)
  {
    if (target->crop  != crop)   return FALSE;
    if (target->frame != frame)  return FALSE;
    if (target->rotate!= rotate) return FALSE;
    if (target->keepaspect != keepaspect) return FALSE;
    #ifdef debug_printf
    printf("Image %p is correct!\n", target);
    #endif
    return TRUE;
  }
  else
  {
    #ifdef debug_printf
    printf("Image %p is invalid!\n", target);
    #endif
  }
  return FALSE;
}

void copy_image_settings(image *target, image *source)
{
  target->crop=source->crop;
  target->frame=source->frame;
  target->rotate=source->rotate;
  target->keepaspect=source->keepaspect;
  target->height=source->height;
  target->width=source->width;
  target->aspect_rate=source->aspect_rate;
  target->valid=source->valid;
}

void set_image_settings(image *target)
{
  target->crop=crop;
  target->frame=frame;
  target->rotate=rotate;
  target->keepaspect=keepaspect;
  target->valid=TRUE;
}

void reset_image(image *const target)
{
  #ifdef debug_printf
  printf("Resetting image %p\n", target);
  #endif
  target->name[0] = '\0';
  target->valid=target->keepaspect=target->rotate=target->frame=target->crop=target->width=target->height=0; /* Сбрасываем всё остальное в структуре */
  target->aspect_rate=(double)0;
  pixbuf_unref(target->pixbuf);
  target->pixbuf=NULL;
}

gboolean load_image(const char *const filename, const  struct_panel *const panel, const int enable_actions, image *const target) /* Загружаем и готовим к показу картинку */
{
  if (filename==NULL || filename[0]=='\0') return FALSE; /*Если функция вызвана с пустым именем для загрузки*/
    if ((strcmp(target->name,filename) == 0) && (check_image_settings(target) == TRUE)) /*Если уже загружено нужное изображение с нужными настройками*/
    {
      #ifdef debug_printf
      printf("Correct image is already loaded, nothing to do!\n");
      #endif
      return TRUE;
    }

  reset_image(target); /* Сбрасываем изображение которое уже хранилось ранее в этом target */
  #ifdef debug_printf
  printf("Going to load '%s' (enable_actions=%d)\n", filename, enable_actions);
  #endif
  if ((strcmp(preloaded.name,filename) == 0) && (check_image_settings(&preloaded) == TRUE)) /*Если предзагруженное изображение и текущее совпали */
  {
    #ifdef debug_printf
    printf("USING PRELOADED IMAGE\n");    
    #endif
    if (GDK_IS_PIXBUF(preloaded.pixbuf))
    {
      #ifdef debug_printf
      printf("preloaded_image correct\n");
      #endif
      strncpy(target->name, preloaded.name, PATHSIZE);
      target->name[PATHSIZE]='\0';
      
      target->pixbuf=preloaded.pixbuf; /* Не копирование - только указатель! */
      preloaded.name[0]='\0';
      preloaded.pixbuf=NULL;
      update_image_dimentions(target);
      copy_image_settings(target,&preloaded);
      reset_image(&preloaded);
      return TRUE;
    }
    else
    {
      #ifdef debug_printf
      printf("preloaded data is incorrect (should never happend), trying to load image standart way!\n");
      #endif
      reset_image(target);
      reset_image(&preloaded);
      return (load_image(filename, panel, enable_actions, target));
    }
  }
  else
  {
    char *name=NULL, *extracted_file_name=NULL;
    #ifdef debug_printf
    if (GDK_IS_PIXBUF(preloaded.pixbuf))/*Если в пиксбуфе что-то лежало */
    {
      if (check_image_settings(&preloaded))
        printf("PRELOADED IMAGE SETTINGS IS WRONG!!!\n");
      else
        printf("PRELOADED IMAGE IS WRONG!!! have '%s', want '%s'\n",preloaded.name,filename);
    }
    #endif
    name=strdup(filename);
    strncpy(target->name,basename(name),PATHSIZE); // basename() - free() не требует
    target->name[PATHSIZE]='\0';
    free(name);
    if (panel->archive_depth > 0 && (suspended == FALSE))
    {
      char *archive_file_name;
      archive_file_name=xconcat(panel->archive_cwd, target->name);
      archive_extract_file(panel->archive_stack[panel->archive_depth], archive_file_name, "/tmp/");
      extracted_file_name=xconcat("/tmp/", archive_file_name);
      free (archive_file_name);
      target->pixbuf=gdk_pixbuf_new_from_file (extracted_file_name, NULL);
    }
    else
      target->pixbuf=gdk_pixbuf_new_from_file (filename, NULL);
    #ifdef debug_printf
    printf("pixbuf %p loaded from file\n",target->pixbuf);
    #endif    
    update_image_dimentions(target);
    set_image_settings(target);
    
    if (target->pixbuf == NULL)
    {
      if (enable_actions)
      {
        char *body=xconcat(UNABLE_TO_SHOW, filename);
        Message (ERROR, body);
        free(body);
      }
      reset_image(target);
      return FALSE;
    }
    
    if (panel->archive_depth > 0 && (suspended == FALSE))
    {
      #ifdef debug_printf
      printf("Removing extracted '%s'\n",extracted_file_name);
      #endif
      (void)remove(extracted_file_name);
      free (extracted_file_name);
    }
    image_resize (target);
  }
  
  return TRUE;
}

gboolean show_image(image *target, struct_panel *panel, int enable_actions) /* Показываем картинку */
{
  #ifdef debug_printf
  printf("Going to show '%s' (enable_actions=%d)\n", target->name, enable_actions);
  #endif
  gtk_image_set_from_pixbuf (GTK_IMAGE(gimage), target->pixbuf);
  printf("showed '%s' (enable_actions=%d)\n", target->name, enable_actions);
  if (enable_actions)
  {
    char *iter;
    write_config_int("viewed_pages", ++viewed_pages); /* Сохраняем на диск счётчик страниц */
    free(panel->last_name);
    panel->last_name=strdup(target->name);
    if (panel == &top_panel)
      write_config_string("top_panel.last_name", panel->last_name);
    else
      write_config_string("bottom_panel.last_name", panel->last_name);
    iter=iter_from_filename (basename(panel->last_name), panel); // basename() - free() не требует
    move_selection(iter, panel);
    free(iter);
  }
  gtk_window_set_title(GTK_WINDOW(ImageWindow), target->name);
  wait_for_draw(); /* Ожидаем отрисовки всего */
  return TRUE;
}

gint which_key_press (__attribute__((unused))GtkWidget *window, GdkEventKey *event, struct_panel *panel) /*реакция на кнопки */
{
  char *next_file;
  if (check_key_press(event->keyval, panel)) return TRUE;
  #ifdef debug_printf
  printf("Caught in viewer: %d\n",event->keyval);
  #endif
  switch (event->keyval){
    
    case KEY_PGDOWN:/*GDK_Right */
    case KEY_RIGHT:/*GDK_Right */
      interface_is_locked=TRUE; /* Блокируем интерфейс на время длительной операции по показу картинки */
      move_left_to_left = FALSE;
      if (frame && current.frames >= 2)/* Действия при просмотре в покадровом режиме */
      {
        value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adjust));
        if (gdk_pixbuf_get_height (current.pixbuf) - width_display - 2 > value)
        {
          shift_val = shift (gdk_pixbuf_get_height (current.pixbuf), current.frames, value);
          gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), value + shift_val +2);
          if (double_refresh) e_ink_refresh_local ();
          e_ink_refresh_full ();
          interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
          return FALSE;
        }
        else move_left_to_left = TRUE;
      }
      else
      {
        if (rotate) /* Действия при просмотре с поворотом */
        {
          if (manga)
          {
            value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adjust)); /* Получаем положение картинки в окне */
            #ifdef debug_printf
            printf("In manga mode gtk_adjustment_get_value returned %d, but R_SHIFT=%d\n", value, R_SHIFT);
            #endif
            if (value == R_SHIFT) /* Если она у правого края */
            {
              #ifdef debug_printf
              printf("In manga mode scrolling to %d\n", L_SHIFT);
              #endif
              gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), L_SHIFT); /* Перемещаем её в левый край */
              value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adjust)); /* Получаем положение картинки в окне */
              wait_for_draw();
              #ifdef debug_printf
              printf("In manga mode gtk_adjustment_get_value returned new value %d\n", value);
              #endif
              if (double_refresh) e_ink_refresh_local();
              e_ink_refresh_full ();
              interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
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
              interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
              return FALSE;
            }
          }
        }
      }
      next_file = next_image (panel->selected_name, TRUE, panel);
      if (next_file==NULL) 
      {
        interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
        return FALSE;
      }
      char *full_name=strdup(next_file);
      char *iter=iter_from_filename (basename(full_name), panel);
      move_selection(iter, panel);
      free(iter);
      free(full_name);
      if (is_picture(next_file) == FALSE) 
      {
        interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
        free(next_file);
        return FALSE;
      }
      /*g_print ("загрузка новой картинки\n"); */
      (void)load_image (next_file, panel, TRUE, &current);
      (void)show_image (&current, panel, TRUE);
      free(next_file);
      
      if (frame && current.frames >= 2) 
        move_left_to_left = TRUE;
      if (manga)
        gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), R_SHIFT);
      else
        gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), L_SHIFT);
      if (move_left_to_left) move_left_to_left = FALSE;
      wait_for_draw(); /* Ожидаем отрисовки всего */
      if (panel == &top_panel)
        write_config_string("top_panel.last_name", panel->selected_name); /* Сохраняем конфиги */
        else
          write_config_string("bottom_panel.last_name", panel->selected_name);
        if (double_refresh) e_ink_refresh_local();
        e_ink_refresh_full ();
      if(preload_enable) /* Предзагрузка */
      {
        char *next=next_image (panel->selected_name, FALSE, panel);
        (void)load_image(next, panel, FALSE, &preloaded);
        free(next);
      }
      interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
      return FALSE;
      
      case KEY_PGUP:/*GDK_Left: */
      case KEY_LEFT:/*GDK_Left: */
        interface_is_locked=TRUE; /* Блокируем интерфейс на время длительной операции по показу картинки */
        if ((frame == TRUE) && current.frames >= 2)
        {
          value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adjust));
          if (value > 2) {
            shift_val = shift_back (gdk_pixbuf_get_height (current.pixbuf), current.frames, value) * -1;
            if (value + width_display > gdk_pixbuf_get_height (current.pixbuf)) value = value -2;
            gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), value + shift_val);
            if (double_refresh) e_ink_refresh_local ();
            e_ink_refresh_full ();
            interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
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
                interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
                return FALSE;
              }
            }
            else
            {
              if (value == R_SHIFT) {
                gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), L_SHIFT);
                if (double_refresh) e_ink_refresh_local();
                e_ink_refresh_full ();
                interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
                return FALSE;
              }
            }
          }
        }
        if (rotate) gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), R_SHIFT);
        next_file = prev_image (panel->selected_name, TRUE, panel);
        if (next_file==NULL) 
        {
          interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
          return FALSE;
        }
        select_file_by_name(next_file, panel);
        if (is_picture(panel->selected_name) == FALSE) 
        {
          free(next_file);
          interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
          return FALSE;
        }
        (void)load_image (panel->selected_name, panel, TRUE, &current);
        (void)show_image (&current, panel, TRUE);
        free(next_file);
        
        if (manga)
          gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), L_SHIFT);
        else
          gtk_adjustment_set_value(GTK_ADJUSTMENT(adjust), R_SHIFT);
        
        if (frame)
        {
          value = gdk_pixbuf_get_height (current.pixbuf) + 2 - width_display;
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
        if(preload_enable) /* Предзагрузка */
        {
          char *next=next_image(panel->selected_name, FALSE, panel);
          (void)load_image(next, panel, FALSE, &preloaded);
          free(next);
        }
        interface_is_locked=FALSE; /* Снимаем блокировку интерфейса */
        return FALSE;
        
        case KEY_BACK:/*GDK_x: */
          die_viewer_window();
          return FALSE;
          
        case   GDK_m:
        case   KEY_MENU:
        case   KEY_MENU_LIBROII:
        case   KEY_MENU_QT:
          start_picture_menu (panel, ImageWindow); /* открываем меню картинки */
          return FALSE;
          
        case   KEY_REFRESH_LIBROII:
        case   KEY_REFRESH_QT:
          e_ink_refresh_full();
          return FALSE;
          
        default:
          #ifdef debug_printf
          printf("got unknown keycode %x in main\n", event->keyval);
          #endif  
          return FALSE;
  }
}

void image_zoom_rotate (image *target)
{
  #ifdef debug_printf
  printf("image_zoom_rotate called\n");
  #endif
  double height = gdk_pixbuf_get_height (target->pixbuf);
  double width  = gdk_pixbuf_get_width  (target->pixbuf);
  int scaling_quality=GDK_INTERP_BILINEAR;
  if (HD_scaling) scaling_quality=GDK_INTERP_HYPER;
    
  GdkPixbuf *pixbuf_key;
  current.frames = 0;
  if (height < 300 && width < 200) /*оригинал слишком маленький - ничего не менять */
    return;
  
  /*оригинал слишком широкий и большой  - растянуть в ширину */
  if (width > height && (int)width >= width_display) 
  {
    pixbuf_key = gdk_pixbuf_scale_simple (target->pixbuf, width_display * 2, height_display, scaling_quality);
    pixbuf_unref(target->pixbuf);
    target->pixbuf = gdk_pixbuf_copy (pixbuf_key);
    update_image_dimentions(target);
    pixbuf_unref(pixbuf_key);
    return;
  }
  
  if (frame) 
  {
    target->frames = frames_search(target);
    if (target->frames >= 2)
    {
      target->aspect_rate = height / width * height_display;
      pixbuf_key = gdk_pixbuf_scale_simple (target->pixbuf, height_display, target->aspect_rate, scaling_quality);
    } 
    else
      pixbuf_key = gdk_pixbuf_scale_simple (target->pixbuf, height_display, width_display * 2, scaling_quality);
  } 
  else 
    pixbuf_key = gdk_pixbuf_scale_simple (target->pixbuf, height_display, width_display * 2, scaling_quality);
  pixbuf_unref(target->pixbuf);
  target->pixbuf = gdk_pixbuf_rotate_simple (pixbuf_key, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
  update_image_dimentions(target);
  pixbuf_unref(pixbuf_key);
  target->frames = frames_search(target);
}

void image_resize (image *target) /* изменение разрешения и подрезка полей */
{
  int scaling_quality=GDK_INTERP_BILINEAR;
  if (HD_scaling) scaling_quality=GDK_INTERP_HYPER;
  
  #ifdef debug_printf
  printf("image_resize called\n");
  #endif
  
  if (crop == TRUE && target->width>115 && target->height>115) 
  {
    find_crop_image_coords (target);
    if (return_crop_coord(0) != -1) {
      int x = return_crop_coord(0);
      int y = return_crop_coord(1);
      int w = return_crop_coord(2);
      int h = return_crop_coord(3);
      
      GdkPixbuf *pixbuf_key = gdk_pixbuf_new_subpixbuf(target->pixbuf, x, y, w, h);
      #ifdef debug_printf
      printf("pixbuf %p created by gdk_pixbuf_new_subpixbuf() (crop margins), width=%d, height=%d\n",pixbuf_key, gdk_pixbuf_get_width(pixbuf_key), gdk_pixbuf_get_height(pixbuf_key));
      #endif
      pixbuf_unref(target->pixbuf);
      target->pixbuf=pixbuf_key;
      update_image_dimentions(target);      
    }
  }
  
  if (rotate) /*удвоение размера с поворотом */
  {
    image_zoom_rotate (target);
    return;
  }
  
  /*если оригинал слишком широкий - повернуть на 90 */
  if (target->width > target->height)
  {
    GdkPixbuf *pixbuf_key = gdk_pixbuf_rotate_simple (target->pixbuf, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
    pixbuf_unref(target->pixbuf);
    target->pixbuf = gdk_pixbuf_copy (pixbuf_key);
    pixbuf_unref(pixbuf_key);
    update_image_dimentions(target);
  }
  
  /* Скалировать оригинал до экрана */
  if (keepaspect)
  {
    GdkPixbuf *temp;
    double display_ar = (double)(width_display)/(double)(height_display); /* Соотношение сторон экрана */
    #ifdef debug_printf
    printf("resolution: rx=%i ry=%i dar=%f iar=%f\n", target->width, target->height, display_ar, target->aspect_rate);
    #endif
    int scaled_x,scaled_y;
    if (display_ar < target->aspect_rate ) /* Если рисунок шире экрана */
    {
      scaled_x=width_display; /* Ширина экрана */
      scaled_y=width_display*target->height/target->width; /* Высчитываем требуемую высоту */
    }
    else /* Если рисунок Уже экрана */
    {
      scaled_y=height_display;/* Высота экрана */
      scaled_x=(int)((float)target->width*scaled_y/target->height);/* Высчитываем требуемую ширину */
    }
    #ifdef debug_printf
    printf("scaling pic from %dx%d to %dx%d\n",target->width,target->height,scaled_x,scaled_y);
    #endif
    temp = gdk_pixbuf_scale_simple (target->pixbuf, scaled_x, scaled_y, scaling_quality);
    #ifdef debug_printf
    printf("pixbuf %p created by gdk_pixbuf_scale_simple ()\n",temp);
    #endif
    pixbuf_unref(target->pixbuf);
    target->pixbuf = gdk_pixbuf_copy (temp);
    #ifdef debug_printf
    printf("pixbuf %p created by gdk_pixbuf_copy()\n",target->pixbuf);
    #endif
    update_image_dimentions(target);
    pixbuf_unref(temp);
  }
  else
  {
    #ifdef debug_printf
    printf("aspect not preserved\n");
    #endif
    GdkPixbuf *pixbuf_key = gdk_pixbuf_scale_simple (target->pixbuf, width_display, height_display, scaling_quality);
    pixbuf_unref(target->pixbuf);
    target->pixbuf = gdk_pixbuf_copy (pixbuf_key);
    update_image_dimentions(target);
    pixbuf_unref(pixbuf_key);
  }
  return;
}

void ViewImageWindow(const char *file, struct_panel *panel, int enable_actions) /*создание изображения через GdkPixbuf */
{
  /* Убираем хэндлер с обработки фокуса фэйлменеджером */
  /*   g_signal_handlers_disconnect_by_func( window, focus_in_callback, NULL ); */
  /*   g_signal_handlers_disconnect_by_func( window, focus_out_callback, NULL ); */
  /*   focus_in_processed=0; */
  if (GTK_IS_WIDGET(ImageWindow))
  {
    #ifdef debug_printf
    printf("Image Viewer Window already exists, refusing to open new (should never happends)!\n");
    #endif
    return;
  }
  if (suppress_panel && (QT == FALSE))
    kill_panel();
  if (preloaded.name == NULL)
    preloaded.name[0]='\0';
  #ifdef debug_printf
  printf("Opening viewer for '%s'\n", file);
  #endif
  ImageWindow = window_create (width_display, height_display, 0, "", NOT_MODAL);
  gtk_window_set_decorated (GTK_WINDOW(ImageWindow), FALSE);
  /*   g_signal_connect (G_OBJECT (ImageWindow), "expose-event", G_CALLBACK (e_ink_refresh_full), NULL); */
  #ifndef __amd64
  gtk_window_fullscreen  (GTK_WINDOW(ImageWindow));  /*блокировка окошка "часиков", нужно отключать для отображения на пк */
  #endif
  (void)g_signal_connect (G_OBJECT (ImageWindow), "key_press_event", G_CALLBACK (which_key_press), panel);
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 0); /* Хрен знает работает ли, но хуже не стало */
  gtk_container_add (GTK_CONTAINER (ImageWindow), scrolled_window);
  
  /*GTK_POLICY_ALWAYS для отображения на пк так же как на книжке */
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
  gimage = gtk_image_new (); 
  
  (void)load_image(file, panel, enable_actions, &current);
  (void)show_image(&current, panel, enable_actions);
  
  adjust = gtk_adjustment_new (0.0, 0.0, 200.0, 0.1, 1.0, 1.0);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrolled_window), GTK_WIDGET(gimage));
  gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW(scrolled_window), GTK_ADJUSTMENT(adjust));
  /*сигнал для проверки работы горизонтального сдвига рисунка */
  /* g_signal_connect (G_OBJECT (adjust), "value_changed", G_CALLBACK (print_adjust), NULL); */
  /*     if (enable_actions) */
  /*     { */
  /*       if (panel == &top_panel) */
  /*         write_config_string("top_panel.last_name", panel->selected_name); */
  /*       else */
  /*         write_config_string("bottom_panel.last_name", panel->selected_name); */
  /*     } */
  /*     else */
  /*       write_config_int("viewed_pages", --viewed_pages); */
  /*      */
  gtk_widget_show_all(ImageWindow);
  gtk_widget_grab_focus(ImageWindow);
  if (enable_actions)
  {
    select_file_by_name(file, panel);
    in_picture_viewer=TRUE;
  }
  if (double_refresh) e_ink_refresh_local();
  e_ink_refresh_full ();
  /*     g_signal_connect_after (GTK_WINDOW (win), "focus", G_CALLBACK (focus_in_callback), NULL); */
  /*     g_signal_connect (G_OBJECT (win), "focus-out-event", G_CALLBACK (focus_out_callback), NULL); */
  if(preload_enable && (suspended == FALSE)) /* Предзагрузка */
  {
    char *next=next_image (panel->selected_name, FALSE, panel);
    (void)load_image(next, panel, FALSE, &preloaded);
    free(next);
  }
}
