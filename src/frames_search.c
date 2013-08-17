/* Norin Maxim, 2011, Distributed under GPLv2 Terms
 *FRAME_FIND_MODE*/
/*определение координат "кадров" на изображении, pixbuf на входе */
/*надо сказать, этот код излишне переусложнен, особенно где ветвления... */
#include "gtk_file_manager.h" /* Инклюдить первой среди своих, ибо typedef panel! */
#include "ViewImageWindow.h"
#include "frames_search.h"

#define FRAME_SIZE 160 /*минимально возможный размер кадра в пикселах*/
#define SEPARATOR_LINE_MAX 200 /*сколько линий-разделителей может быт подряд*/
#define F_START 0 /*индексы массива frame_map*/
#define F_END 1
#define PIXEL_RANDOM_MAX 8/*максимум любых случайных пикселов,
чем это число больше, тем больше кадров может найтись, но также возрастает вероятность ложного определения*/
#define WHITE 160 /*белым цветом будет считатся от WHITE до 255*/
#define BLACK 60 /*черным цветом будет считатся от 0 до BLACK*/

static int rowstride, n_channels;
static guchar *pixels, *p;
static int f_num;            /*индекс столбца для frame_map */
static int frame_map [2][14]; /*массив координат, где 14 - максимум кадров */
static int f_count;	     /*счетчик кадров */
static int s_count;	     /*счетчик линий-разделителей */
static int y;
static int wh, ht;

/*воозвращает количество кадров и сохраняет их соординаты в frame_map[][] */
int frames_search (image *target)
{  
  y = FRAME_SIZE;
  f_num = 0;
  ht = gdk_pixbuf_get_height (target->pixbuf);
  wh = gdk_pixbuf_get_width  (target->pixbuf);
  int f = 0; /*флаг разрыва цикла */
  f_count = 0;
  s_count = 0;

  n_channels = gdk_pixbuf_get_n_channels (target->pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (target->pixbuf);
  pixels = gdk_pixbuf_get_pixels (target->pixbuf);

  frame_map_clear();
  for(;;){
    if (line_separator()) f = left_way(); else f = right_way();
    if (f) break;
  }
  if (f_count>1) frame_map[0][0] = 0;
  return f_count;
}

int right_way () /*ветвление */
{
  if (ht-FRAME_SIZE > y) {
    y++;
    return 0;
  }
  if (f_count>=1) {
    frame_map[F_END][f_num] = ht;
    f_count++;
    return 1;
  }
  frame_map_clear();
  return 1;
}

int left_way() /*ветвление */
{
  int f = 0;
  frame_map[F_END][f_num] = y;
  y++;
  s_count++;
  f_count++;
  f_num++;
  for (;;){
    if (line_separator()) f=left_way_sc(); else break;
    if (f) return 1;
  }
  s_count = 0;
  frame_map[F_START][f_num] = y;
  y = y + FRAME_SIZE;
  if (ht - FRAME_SIZE > y) return 0;
  frame_map[F_END][f_num] = ht;
  f_count++;
  return 1;
}

int left_way_sc() /*ветвление с проверкой пустоты между кадрами */
{
  int f = 0;
  for(;;){
    if (ht-FRAME_SIZE < y)  f = left_way_fc();
    if (f) break;
    if (s_count > SEPARATOR_LINE_MAX) {
      f = left_way_fc();
      if (f) break;
    }
    y++;
    return 0;
  }
  return 1;
}

int left_way_fc () /*ветвление */
{
  if (f_count>1){
    frame_map[F_END][f_num] = ht;
    f_count++;
    return 1;
  }
  frame_map_clear();
  return 1;
}

/*заполняет frame_map значением "-1" */
void frame_map_clear()
{
  int i, ii;
  for (i=0; i <= 1; i++)
    for (ii=0; ii <= 13; ii++)
      frame_map[i][ii] = -1;
}

/*здесь определяется, что является разделителем */
/*возвращает 1 если  линия-разделитель найдена */
int line_separator ()
{
  int x, red, green, blue, b_color, w_color, prev, line, rp;
  rp = 0;		/* count  random pixel */
  prev = 1;	/* previous pixel   0 black     1  white */
  line = 1;	/* 1 сплошная линия или нет 0 */
  b_color = 0; 	/* 0  не черный */
  w_color = 0; 	/* 0  не белый */

  /*поиск горизонтальной линии относительно равномерного цвета */
  for  (x =0; x<wh-1; x++) {
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

int frame_coord (int i, int ii)
{
  return frame_map[i][ii];
}
