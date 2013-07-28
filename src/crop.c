/* Norin Maxim, 2011, Distributed under GPLv2 Terms
 *CROP FUNCTION*/
//на входе pixbuf на выходе координаты для обрезки
#include <gtk/gtk.h>
#include "crop.h"

static int x_crop;		// координаты для вырезания из pixbuf_src
static int y_crop;		//
static int width_crop;		//
static int height_crop;		//
static int coord_crop[4];
static int rowstride, n_channels;
static guchar *pixels, *p;

void crop_image (GdkPixbuf *pixbuf, int width, int height)
{
  x_crop = 0;
  y_crop = 0;
  width_crop = width;
  height_crop = height;
  
  n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);
  
  find_x_crop (height);    //поиск координат левого бордюра
  find_y_crop (width);  		//верхнего
  find_width_crop (width, height);		//правого
  find_height_crop (width, height);		//нижнего
  coord_crop[0] = x_crop;
  coord_crop[1] = y_crop;
  coord_crop[2] = width_crop - x_crop;
  coord_crop[3] = height_crop - y_crop;
  if (x_crop==0 && y_crop==0 && width_crop==width && height_crop==height){
    coord_crop[0] = -1;
    // 		g_print ("возврат без изменений\n");
  }
}

//#############################################################################
//###############################left border###################################
//#############################################################################
void find_x_crop (int height)
{
  int x, y, red, green, blue, b_color, w_color;
  int pcm = 0;//count random color pixel
  int prm = 0;//count any random pixel
  int xy_tmp = 1;//count for  PIXEL_RESET_COUNT
  
  b_color = 0;
  w_color = 0;
  x = 0;//width	//y = height;
  //////////////////////////////////определение цвета бордюра, черный или белый
  for  (y=0; y<height-1; y++) {
    p = pixels + y * rowstride + x * n_channels;
    red = p[0];
    green = p[1];
    blue = p[2];
    //цветная или нет
    if (red != green || red != blue || green != blue) {
      pcm++;
      if (pcm > PIXEL_COLOR_MAX){
        x_crop = 0;//color pixel
        return;
      }
    }
    if (red < BLACK && green<BLACK && blue < BLACK) b_color++;
    if (red > WHITE && green>WHITE && blue >WHITE) w_color++;
  }
  if (height - (b_color + w_color)> (height/100)*GREY) {
    x_crop = 0;
    return;
  }
  
  if (b_color <= w_color)
    w_color= 1;
  else
    w_color =0;
  
  ///////////////////////////////////////////определение толщины бордюра
  if (w_color == 1){
    for (x=1; x<BORDER_SIZE; x++) {
      for  (y=0; y<height-1; y++) {
        p = pixels + y * rowstride + x * n_channels;
        if (p[0] < WHITE) {
          prm++;
          if (y - xy_tmp > PIXEL_RESET_COUNT) prm = 0;
          xy_tmp = y;
          if (prm > PIXEL_RANDOM_MAX) {
            x_crop = x;
            return;
          }
        }
      }
    }
    x_crop = x;
    return;
    
  } else {
    
    for (x=1; x<BORDER_SIZE; x++) {
      for  (y=0; y<height-1; y++) {
        p = pixels + y * rowstride + x * n_channels;
        if (p[0] > BLACK) {
          prm++;
          if (x - xy_tmp > PIXEL_RESET_COUNT) prm = 0;
          xy_tmp = x;
          if (prm > PIXEL_RANDOM_MAX) {
            x_crop = x;
            return;
          }
        }
      }
    }
    x_crop = x;
  }
}

//#############################################################################
//################################top border###################################
//#############################################################################
void find_y_crop (int width)
{
  int x, y, red, green, blue, b_color, w_color;
  int pcm = 0;//count random color pixel
  int prm = 0;//count any random pixel
  int xy_tmp = 1;//count for  PIXEL_RESET_COUNT
  
  b_color = 0;
  w_color = 0;
  y = 0;//width	//y = height;
  //////////////////////////////////определение цвета бордюра, черный или белый
  for  (x=x_crop; x<width-1; x++) {
    p = pixels + y * rowstride + x * n_channels;
    red = p[0];
    green = p[1];
    blue = p[2];
    //цветная или нет
    if (red != green || red != blue || green != blue) {
      pcm++;
      //			g_print ("pcm=%d\n", pcm);
      if (pcm > PIXEL_COLOR_MAX){
        y_crop = 0;//color pixel
        return;
      }
    }
    if (red < BLACK && green<BLACK && blue < BLACK) b_color++;
    if (red > WHITE && green>WHITE && blue >WHITE) w_color++;
  }
  if ((width-x_crop) - (b_color + w_color)> (width/100)*GREY) {
    //		g_print ("grey\n");
    y_crop = 0;
    return;
  }
  
  if (b_color <= w_color)
    w_color= 1;
  else
    w_color =0;
  //g_print ("%d\n", w_color);
  ///////////////////////////////////////////определение толщины бордюра
  if (w_color == 1){
    for (y=1; y<BORDER_SIZE; y++) {
      for  (x=x_crop; x<width-1; x++) {
        p = pixels + y * rowstride + x * n_channels;
        if (p[0] < WHITE) {
          prm++;
          if (x - xy_tmp > PIXEL_RESET_COUNT) prm = 0;
          xy_tmp = x;
          if (prm > PIXEL_RANDOM_MAX) {
            y_crop = y;
            return;
          }
        }
      }
    }
    y_crop = y;
    return;
    
  } else {
    
    for (y=1; y<BORDER_SIZE; y++) {
      for  (x=x_crop; x<width-1; x++) {
        p = pixels + y * rowstride + x * n_channels;
        if (p[0] > BLACK) {
          prm++;
          if (x - xy_tmp > PIXEL_RESET_COUNT) prm = 0;
          xy_tmp = x;
          if (prm > PIXEL_RANDOM_MAX) {
            y_crop = y;
            return;
          }
        }
      }
    }
    y_crop = y;
  }
}

//#############################################################################
//#################################right border################################
//#############################################################################
void find_width_crop (int width, int height)
{
  int x, y, red, green, blue, b_color, w_color;
  int pcm = 0;//count random color pixel
  int prm = 0;//count any random pixel
  int xy_tmp = 1;//count for  PIXEL_RESET_COUNT
  
  b_color = 0;
  w_color = 0;
  x = width - 1;//width	//y = height;
  //////////////////////////////////определение цвета бордюра, черный или белый
  for  (y=y_crop; y<height-1; y++) {
    p = pixels + y * rowstride + x * n_channels;
    red = p[0];
    green = p[1];
    blue = p[2];
    //цветная или нет
    if (red != green || red != blue || green != blue) {
      pcm++;
      if (pcm > PIXEL_COLOR_MAX){
        width_crop = width;//color pixel
        return;
      }
    }
    if (red < BLACK && green<BLACK && blue < BLACK) b_color++;
    if (red > WHITE && green>WHITE && blue >WHITE) w_color++;
  }
  if ((height-y_crop) - (b_color + w_color)> (height/100)*GREY) {
    width_crop = width;
    return;
  }
  
  if (b_color <= w_color)
    w_color= 1;
  else
    w_color =0;
  //g_print ("%d\n", w_color);
  ///////////////////////////////////////////определение толщины бордюра
  if (w_color == 1){
    for (x=width - 1; x > width - BORDER_SIZE; x--) {
      for  (y=y_crop; y<height-1; y++) {
        p = pixels + y * rowstride + x * n_channels;
        if (p[0] < WHITE) {
          prm++;
          if (y - xy_tmp > PIXEL_RESET_COUNT) prm = 0;
          xy_tmp = y;
          if (prm > PIXEL_RANDOM_MAX) {
            width_crop = x;
            return;
          }
        }
      }
    }
    width_crop = x;
    return;
    
  } else {
    
    for (x=width - 1; x > width - BORDER_SIZE; x--) {
      for  (y=y_crop; y<height-1; y++) {
        p = pixels + y * rowstride + x * n_channels;
        if (p[0] > BLACK) {
          prm++;
          if (x - xy_tmp > PIXEL_RESET_COUNT) prm = 0;
          xy_tmp = x;
          if (prm > PIXEL_RANDOM_MAX) {
            width_crop = x;
            return;
          }
        }
      }
    }
    width_crop = x;
  }
}

//#############################################################################
//###############################buttom border#################################
//#############################################################################
void find_height_crop (int width, int height)
{
  int x, y, red, green, blue, b_color, w_color;
  int pcm = 0;//count random color pixel
  int prm = 0;//count any random pixel
  int xy_tmp = x_crop;//count for  PIXEL_RESET_COUNT
  
  b_color = 0;
  w_color = 0;
  y = height - 1;
  //////////////////////////////////определение цвета бордюра, черный или белый
  for  (x=x_crop; x<width_crop -1; x++) {
    p = pixels + y * rowstride + x * n_channels;
    red = p[0];
    green = p[1];
    blue = p[2];
    //цветная или нет
    if (red != green || red != blue || green != blue) {
      pcm++;
      
      if (pcm > PIXEL_COLOR_MAX){
        height_crop = height;//color pixel
        return;
      }
    }
    if (red < BLACK && green<BLACK && blue < BLACK) b_color++;
    if (red > WHITE && green>WHITE && blue >WHITE) w_color++;
  }
  if ((width_crop-x_crop) - (b_color + w_color)> (width/100)*GREY) {
    height_crop = height;
    return;
  }
  if (b_color <= w_color)
    w_color= 1;
  else
    w_color =0;
  
  ///////////////////////////////////////////определение толщины бордюра
  if (w_color == 1){
    for (y= height - 1; y > height - BORDER_SIZE; y--) {
      for  (x=x_crop; x<width_crop-1; x++) {
        p = pixels + y * rowstride + x * n_channels;
        if (p[0] < WHITE) {
          prm++;
          if (x - xy_tmp > PIXEL_RESET_COUNT) prm = 0;
          xy_tmp = x;
          if (prm > PIXEL_RANDOM_MAX) {
            height_crop = y;
            return;
          }
        }
      }
    }
    height_crop = y;
    return;
    
  } else {
    
    for (y= height - 1; y > height - BORDER_SIZE; y--) {
      for  (x=x_crop; x<width_crop-1; x++) {
        p = pixels + y * rowstride + x * n_channels;
        if (p[0] > BLACK) {
          prm++;
          if (x - xy_tmp > PIXEL_RESET_COUNT) prm = 0;
          xy_tmp = x;
          if (prm > PIXEL_RANDOM_MAX) {
            height_crop = y;
            return;
          }
        }
      }
    }
    height_crop = y;
  }
}

int return_crop_coord (int i)
{
  return coord_crop[i];
}
