#include <gtk/gtk.h>
#include "gtk_file_manager.h" /* Инклюдить первой среди своих, ибо typedef panel! */
#include "ViewImageWindow.h"
#include "contrast.h"
// http://habrahabr.ru/post/139428/
// http://habrahabr.ru/post/304210/
void adjust_contrast(image *target, int contrast, int page) // contrast (256 - normal)
{
  guchar *imageData;
  ssize_t i;
  long long int dataSize;
  unsigned char buf[256];
  long long int midBright = 0, midBright1 = 0, midBright2 = 0;

  if (! GDK_IS_PIXBUF (target->pixbuf[page]))
    return;
  imageData = gdk_pixbuf_get_pixels (target->pixbuf[page]); // free() не требует!
  // from gdk-pixbuf/gdk-pixbuf.c gdk_pixbuf_get_byte_length (const GdkPixbuf *pixbuf) implementation, coz Sibarary toolchain not have gdk_pixbuf_get_byte_length():
  // (pixbuf->height - 1) * pixbuf->rowstride + pixbuf->width * ((pixbuf->n_channels * pixbuf->bits_per_sample + 7) / 8);
  dataSize  = (target->height[page]-1) * gdk_pixbuf_get_rowstride (target->pixbuf[page]) + target->width[page] *((gdk_pixbuf_get_n_channels (target->pixbuf[page]) * gdk_pixbuf_get_bits_per_sample(target->pixbuf[page]) + 7) / 8);

  if (dataSize <= 0)
    return;
  
  for (i = 0; i < dataSize; )
  {
    midBright += imageData[i++];
    midBright1 += imageData[i++];
    midBright2 += imageData[i++];
  }

  // 0,212656*255; 0,715158*255; 0,072186*255 => 54; 183; 19 (for sRGB colorspace)
  midBright = (midBright * 54 + midBright1 * 183 + midBright2 * 19) / (256 * dataSize / 3);

  for (i = 0; i < 256; i++)
  {
    int a = ((((int)i - (int)midBright) * contrast) >> 8) + (int)midBright;
    if (a < 0) buf[i] = 0;
    else if (a > 255) buf[i] = 255;
    else buf[i] = a;
  }

  for (i = 0; i < dataSize; i++)
    imageData[i] = buf[imageData[i]];
  
  target->boost_contrast=TRUE;
}
