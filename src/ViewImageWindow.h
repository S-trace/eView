#ifndef __have_view_image_window
#define __have_view_image_window
#define L_SHIFT 0   /*смещение левой части изображения к левому краю экрана */
#ifdef __amd64
#define R_SHIFT (width_display + 2) /*смещение правой части изображения к правому краю */
#else
#define R_SHIFT (width_display) /*смещение правой части изображения к правому краю // ЯХЕЗ какого гхыра, но на ARM возвращается ширина экрана полностью! =_=" */
#endif
typedef struct { 
  char *name; 
  GdkPixbuf *pixbuf; 
  int width, height; /* Размеры картинки */
  int frames;   /*количество найденых кадров */
  double aspect_rate; /*aspect rate */
  int keepaspect;
  int crop;
  int rotate;
  int frame;
  int valid;
} image;
extern image current, preloaded, screensaver;
extern int in_picture_viewer;

gint which_key_press (GtkWidget *, GdkEventKey *, panel *panel);
void image_resize (int mode_rotate, int mode_crop, int keep_aspect, image *target);
void image_zoom_rotate (image *target);
void image_rotate (image *target);
void reset_image(image *target);
void die_viewer_window (void);
gboolean show_image(image *image, panel *panel, int enable_actions); /* Показываем картинку */
/************** Create a Image Viewer Window  *************************/
void ViewImageWindow(char *filename, panel *panel, int enable_actions);
gboolean load_image(char *filename, panel *panel, int enable_actions, image *target);
gboolean show_image(image *image, panel *panel, int enable_actions);
extern GtkWidget *ImageWindow;
#endif