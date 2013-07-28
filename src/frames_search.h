#include <gtk/gtk.h>
#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

int frames_search (GdkPixbuf *pixbuf, int width, int height);
int right_way (void);
int left_way(void);
int left_way_sc (void);
int left_way_fc (void);
int line_separator (void);
void frame_map_clear(void);
int frame_coord (int i, int ii);