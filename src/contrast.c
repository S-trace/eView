#include <gtk/gtk.h>
#include "gtk_file_manager.h" /* Инклюдить первой среди своих, ибо typedef panel! */
#include "ViewImageWindow.h"
#include "contrast.h"
// http://habrahabr.ru/post/139428/
void adjust_contrast(image *target, int contrast) // contrast (256 - normal)
{
  guchar *imageData = gdk_pixbuf_get_pixels (target->pixbuf); // free() не требует!
  int i;
  int dataSize = gdk_pixbuf_get_rowstride (target->pixbuf)*(target->height-1) + target->width *((gdk_pixbuf_get_n_channels (target->pixbuf) * gdk_pixbuf_get_bits_per_sample(target->pixbuf) + 7) / 8) - 2;
  unsigned char buf[256];

  long long int midBright = 0, midBright1 = 0, midBright2 = 0;
  for (i = 0; i < dataSize; )
  {
    midBright += imageData[i++];
    midBright1 += imageData[i++];
    midBright2 += imageData[i++];
  }
  midBright = (midBright * 77 + midBright1 * 150 + midBright2 * 29) / (256 * dataSize / 3);

  for (i = 0; i < 256; i++)
  {
    int a = (((i - midBright) * contrast) >> 8) + midBright;
    if (a < 0) buf[i] = 0;
    else if (a > 255) buf[i] = 255;
    else buf[i] = a;
  }

  for (i = 0; i < dataSize; i++)
    imageData[i] = buf[imageData[i]];
}
