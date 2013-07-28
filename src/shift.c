/* Norin Maxim, 2011, Distributed under GPLv2 Terms.
 *computing shift positions for frames*/

#include <stdio.h>
#include "gtk_file_manager.h" // Инклюдить первой среди своих, ибо typedef panel!
#include "frames_search.h"
#include "ViewImageWindow.h"
#include "shift.h"
extern int width_display, height_display;

//вычиляет позиции смещения вправо, чтобы очередной кадр попал в экран полностью
int shift (int height, int sum_frame, int l_scr)
{
  int r_scr = l_scr + width_display;	//правая граница экрана
  int fn;			//индекс для frame_coord()
  int s_frame;		//начало кадра
  int e_frame;		//конец кадра
  
  for(fn=0; fn <= sum_frame; fn++) {
    if (frame_coord(0, fn) == -1 || frame_coord(1, fn)== -1) {
      fn--;
      break;
    }
    if (frame_coord (1,fn) > r_scr) break;
  }//fn - индекс текущего кадра
  s_frame = frame_coord(0, fn);
  e_frame = frame_coord(1, fn);
  
  if(s_frame >= r_scr) {
    if(height - r_scr<= width_display)
      return height - r_scr;
    else
      return s_frame - l_scr;
  }
  if (height - r_scr <= width_display) {
    if (height - s_frame <= width_display)
      return height - r_scr;
    else
      return e_frame - r_scr;
  }
  if (s_frame <= l_scr)
    return width_display;
  else
    return s_frame - l_scr;
  return 0;
}

//то же что и shift() но в другую сторону, влевo
int shift_back (int height, int sum_frame, int l_scr)
{
  int r_scr;
  int fn;			//индекс для frame_coord()
  int s_frame;		//начало кадра
  int e_frame;		//конец кадра
  
  if (l_scr + width_display > height) l_scr = l_scr - 2;
  r_scr = l_scr + width_display;
  
  for(fn = sum_frame -1; fn >= 0; fn--)
    if (frame_coord(0,fn) < l_scr) break;
  s_frame = frame_coord(0, fn);
  e_frame = frame_coord(1, fn);
  if (e_frame <= l_scr) {
    if (e_frame < width_display)
      return l_scr- s_frame;
    else
      return r_scr - e_frame;
  }
  if (l_scr < width_display)
    return l_scr;
  else {
    if (e_frame >= r_scr) {
      if (l_scr - s_frame < width_display)
        return l_scr - s_frame;
      else
        return width_display;
    }
    return r_scr - e_frame;
  }
  return 0;
}