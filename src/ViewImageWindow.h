#ifndef __have_view_image_window
#define __have_view_image_window

#define PAGE_FULL 0
#define PAGE_LEFT 1
#define PAGE_RIGHT 2

#define FRAMES_MAX 128 // Для веб-манги (длиннющая простыня из нескольких тысяч пикселей высоты и одного экрана ширины)
typedef struct {
  char name[PATHSIZE+1];
  GdkPixbuf *pixbuf[3]; // 3 - количество пиксбуфов - 0 для всей картинки, 1 - левая страница, 2 - правая
  int width[3], height[3]; /* Размеры картинки */
  int frames[3];     /* Количество найденых кадров */
  int frame_map[3][2][FRAMES_MAX]; // Карта размещения кадров
  double aspect_rate[3]; /*aspect rate */
  int keepaspect;
  int crop;
  int rotate;
  int web_manga_mode;
  int frame;
  int split_spreads;
  int HD_scaling;
  int boost_contrast;
  int pages_count;
  gboolean valid;
} image;

extern GtkWidget *scrolled_window; // Для переключения adjust в interface.c
extern GtkWidget *gimage; // Для перерисовки изображения в boost_contrast_callback()

extern image current, preloaded, cached, screensaver;
extern int in_picture_viewer;
extern int current_page; // Текущая страница при просмотре в режиме поворота
extern int current_position; // Текущее положение на странице (в режиме поворота/веб-манги)
gint which_key_press (GtkWidget *, GdkEventKey *, struct_panel *panel);
void image_resize (image *target);
void image_zoom_rotate (image *target);
void image_rotate (image *target);
void reset_image(image *const target);
void die_viewer_window (void);
/************** Create a Image Viewer Window  *************************/
void ViewImageWindow(const char *filename, struct_panel *panel, int enable_actions);
gboolean load_image(const char *const filename, const  struct_panel *const panel, const int enable_actions, image *const target);
gboolean show_image(image *image, struct_panel *panel, int enable_actions, int page, int position); /* Показываем картинку */
extern GtkWidget *ImageWindow;
#endif
