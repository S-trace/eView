#ifndef __have_view_image_window
#define __have_view_image_window
#define L_SHIFT 0   /*смещение левой части изображения к левому краю экрана */
#ifdef __amd64
#define R_SHIFT (width_display + 2) /*смещение правой части изображения к правому краю */
#else
#define R_SHIFT (width_display) /*смещение правой части изображения к правому краю // ЯХЕЗ какого гхыра, но на ARM возвращается ширина экрана полностью! =_=" */
#endif
#define PAGE_FULL 0
#define PAGE_LEFT 1
#define PAGE_RIGHT 2

typedef struct {
  char name[PATHSIZE+1];
  GdkPixbuf *pixbuf;
  GdkPixbuf *left_page_subpixbuf;
  GdkPixbuf *right_page_subpixbuf;
  int width, height; /* Размеры картинки */
  int frames;   /*количество найденых кадров */
  double aspect_rate; /*aspect rate */
  int keepaspect;
  int crop;
  int rotate;
  int frame;
  int HD_scaling;
  int boost_contrast;
  gboolean valid;
} image;
extern image current, preloaded, cached, screensaver;
extern int in_picture_viewer;
extern int current_page; // Текущая страница при просмотре в режиме поворота
gint which_key_press (GtkWidget *, GdkEventKey *, struct_panel *panel);
void image_resize (image *target);
void image_zoom_rotate (image *target);
void image_rotate (image *target);
void reset_image(image *const target);
void die_viewer_window (void);
/************** Create a Image Viewer Window  *************************/
void ViewImageWindow(const char *filename, struct_panel *panel, int enable_actions);
gboolean load_image(const char *const filename, const  struct_panel *const panel, const int enable_actions, image *const target);
gboolean show_image(image *image, struct_panel *panel, int enable_actions, int page); /* Показываем картинку */
extern GtkWidget *ImageWindow;
#endif
