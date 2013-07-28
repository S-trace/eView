#define L_SHIFT 0   //смещение левой части изображения к левому краю экрана
#ifdef __amd64
  #define R_SHIFT (width_display + 2) //смещение правой части изображения к правому краю
#else
  #define R_SHIFT (width_display) //смещение правой части изображения к правому краю // ЯХЕЗ какого гхыра, но на ARM возвращается ширина экрана полностью! =_="
#endif
gint which_key_press (GtkWidget *, GdkEventKey *, panel *panel);
void image_resize (int mode_rotate, int mode_crop, int keep_aspect);
void image_zoom_rotate (double width, double height);
void image_rotate (int angle);
void reset_preloaded_image(void);
void die_viewer_window (void);
gboolean show_image(char *filename, panel *panel); // Показываем картинку