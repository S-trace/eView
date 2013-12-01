/* Norin Maxim, 2011, Distributed under GPLv2 Terms
 *FRAME_FIND_MODE*/
/*определение координат "кадров" на изображении, pixbuf на входе */
/*надо сказать, этот код излишне переусложнен, особенно где ветвления... */
#include <stdlib.h>
#include "gtk_file_manager.h" /* Инклюдить первой среди своих, ибо typedef panel! */
#include "ViewImageWindow.h"
#include "frames_search.h"
#include "cfg.h"
#define PIXEL_RANDOM_MAX 80/*максимум любых случайных пикселов,
чем это число больше, тем больше кадров может найтись, но также возрастает вероятность ложного определения*/
#define WHITE (guchar)160 /*белым цветом будет считатся от WHITE до 255*/
#define BLACK (guchar)60 /*черным цветом будет считатся от 0 до BLACK*/
// call gdk_pixbuf_save (target->pixbuf[0], "/home/starrk/developement/Sibrary/eView/test.png", "png", &top_panel.archive_stack[0])

static int rowstride, n_channels;
static guchar *pixels, *p;
static int f_num;            /*индекс столбца для frame_map */
static int f_count;	     /*счетчик кадров */
static int s_count;	     /*счетчик линий-разделителей */
static int y;
static int wh, ht;

#ifdef debug
void mark_frames(image *target, int page, int frame_map[][FRAMES_MAX]) // Пометка границ найденных кадров на изображении тонкими красными линиями
{
  guchar *imageData = gdk_pixbuf_get_pixels (target->pixbuf[page]); // free() не требует!
  int dataSize = gdk_pixbuf_get_rowstride (target->pixbuf[page])*(target->height[page]-1) + target->width[page] *((gdk_pixbuf_get_n_channels (target->pixbuf[page]) * gdk_pixbuf_get_bits_per_sample(target->pixbuf[page]) + 7) / 8) - 2;

  int i, ii, pixel_y;
  for (i=0; i < 2; i++)
    for (ii=0; ii < FRAMES_MAX; ii++)
      for (pixel_y=0; pixel_y < wh; pixel_y++)
      {
        int offset = frame_map[i][ii] * rowstride + pixel_y * n_channels;
        if (offset < dataSize && offset > 0)
        {
          guchar *pixel = imageData + offset;
          pixel[0] = 255;
          pixel[1] = 0;
          pixel[2] = 0;
        }
      }
}
#endif

/*воозвращает количество кадров и сохраняет их соординаты в frame_map[][] */
int frames_search (image *target, int page, int frame_map[][FRAMES_MAX])
{
  int f;
  y = FRAME_SIZE;
  f_num = 0;
  ht = target->height[page];
  wh = target->width[page];
  f = 0; /*флаг разрыва цикла */
  f_count = 0;
  s_count = 0;

  n_channels = gdk_pixbuf_get_n_channels (target->pixbuf[page]);
  rowstride = gdk_pixbuf_get_rowstride (target->pixbuf[page]);
  pixels = gdk_pixbuf_get_pixels (target->pixbuf[page]);

  frame_map_clear(frame_map);

  if (frame == FALSE) // Если покадровое листание не разрешено - возвращаем всего один кадр на всё изображение размером
  {
    frame_map[FRAME_START][0] = 0;
    frame_map[FRAME_END][0] = target->height[page]-1;
    return 1;
  }
  for(;;){
    if (line_separator()) f = left_way(frame_map); else f = right_way(frame_map);
    if (f) break;
  }
  if (f_count>1) frame_map[0][0] = 0;
  #ifdef debug
  mark_frames(target, page, frame_map);
  #endif
  return f_count;
}

int right_way (int frame_map[][FRAMES_MAX]) /*ветвление */
{
  if (ht-FRAME_SIZE > y) {
    y++;
    return 0;
  }
  if (f_count>=1) {
    frame_map[FRAME_END][f_num] = ht;
    f_count++;
    return 1;
  }
  frame_map_clear(frame_map);
  return 1;
}

int left_way(int frame_map[][FRAMES_MAX]) /*ветвление */
{
  int f = 0;
  frame_map[FRAME_END][f_num] = y;
  y++;
  s_count++;
  f_count++;
  f_num++;
  for (;;){
    if (line_separator()) f=left_way_sc(frame_map); else break;
    if (f) return 1;
  }
  s_count = 0;
  frame_map[FRAME_START][f_num] = y;
  y = y + FRAME_SIZE;
  if (ht - FRAME_SIZE > y) return 0;
  frame_map[FRAME_END][f_num] = ht;
  f_count++;
  return 1;
}

int left_way_sc(int frame_map[][FRAMES_MAX]) /*ветвление с проверкой пустоты между кадрами */
{
  int f = 0;
  for(;;){
    if (ht-FRAME_SIZE < y)  f = left_way_fc(frame_map);
    if (f) break;
    if (s_count > SEPARATOR_LINE_MAX) {
      f = left_way_fc(frame_map);
      if (f) break;
    }
    y++;
    return 0;
  }
  return 1;
}

int left_way_fc (int frame_map[][FRAMES_MAX]) /*ветвление */
{
  if (f_count>1){
    frame_map[FRAME_END][f_num] = ht;
    f_count++;
    return 1;
  }
  frame_map_clear(frame_map);
  return 1;
}

/*заполняет frame_map значением "-1" */
void frame_map_clear(int frame_map[][FRAMES_MAX])
{
  int i, ii;
  for (i=0; i < 2; i++)
    for (ii=0; ii < FRAMES_MAX; ii++)
      frame_map[i][ii] = -1;
}

/*здесь определяется, что является разделителем */
/*возвращает 1 если  линия-разделитель найдена */
int line_separator (void)
{
  int x, b_color, w_color, prev, line, rp;
  rp = 0;		/* count  random pixel */
  prev = 1;	/* previous pixel   0 black     1  white */
  line = 1;	/* 1 сплошная линия или нет 0 */
  b_color = 0; 	/* 0  не черный */
  w_color = 0; 	/* 0  не белый */

  /*поиск горизонтальной линии относительно равномерного цвета */
  for  (x =0; x<wh-1; x++) {
    guchar red, green, blue;
    p = pixels + y * rowstride + x * n_channels;
    red = p[0];
    green = p[1];
    blue = p[2];

    /*цветной пиксель или нет */
    if (red != green || red != blue || green != blue)
      rp++;

    /*черный или белый пиксель */
    if (red < BLACK && green<BLACK && blue < BLACK) b_color = 1;
    if (red > WHITE && green>WHITE && blue >WHITE) w_color =1;

    /*какой был пиксель до этого и какой сейчас */
    if (prev == 1 && b_color == 1) rp++;
    if (prev == 0 && w_color == 1) rp++;

    if (rp > PIXEL_RANDOM_MAX){
      line = 0;
      break;/*эта линия не сплошная */
    }
    if (w_color) prev = 1;
    if (b_color) prev = 0;
  }
  return line;
}
