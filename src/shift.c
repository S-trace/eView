// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is an open source non-commercial project. Dear PVS-Studio, please check it.
/* Norin Maxim, 2011, Distributed under GPLv2 Terms.
 *computing shift positions for frames*/

#include <stdio.h>
#include "gtk_file_manager.h" /* Инклюдить первой среди своих, ибо typedef panel! */
#include "ViewImageWindow.h"
#include "frames_search.h"
#include "shift.h"
#include "cfg.h"

/*вычиляет позиции смещения вправо, чтобы очередной кадр попал в экран полностью */
int shift (int left_screen_edge, int frames_count, int frame_map[][FRAMES_MAX], int display_size)
{
  int frame_number;   /* Индекс для frame_map[][] */
  int start_of_frame; /* Начало кадра */
  int right_screen_edge = left_screen_edge + display_size; /*правая граница куска картинки на экране */
  int overlap_pixels=display_size*overlap/100;             // Величина на которую накладывается на предыдущее новое изображение при прокрутке
  int gap_size;    // Зазор между кадрами
  int shift_value; // Величина сдвига изображения по кадрам без учёта наложения

  if (frames_count < 2) // Если кадров мы не нашли - отдаём ширину экрана за вычетом наложения
    return display_size-overlap_pixels;

  for (frame_number=0; frame_number <= frames_count; frame_number++) // Ищем первый кадр, не влезающий в экран целиком
  {
    if (frame_map[FRAME_START][frame_number] == -1 || frame_map[FRAME_END][frame_number] == -1)
    {
      frame_number--; // Если последний кадр уже пропустили, и дальше в карте кадров нет совсем ничего - берём последний кадр
      break;
    }
    if (frame_map[FRAME_END][frame_number] > right_screen_edge) // Если конец кадра за правой границей экрана - берём его
      break;
  }

  start_of_frame = frame_map[FRAME_START][frame_number];

  // Если начало кадра мы уже пролистали и оно осталось за левым краем экрана - прокручиваем одну страницу (за вычетом наложения)
  if (start_of_frame <= left_screen_edge)
    return display_size-overlap_pixels;

  // Если начало кадра на странице или же за правым краем - возвращаем разницу между текущим положением экрана и началом кадра:
  gap_size = frame_map[FRAME_START][frame_number]-frame_map[FRAME_END][frame_number-1]; // Зазор между кадрами
  shift_value = start_of_frame-left_screen_edge;
  // Если зазор больше величины наложения - наложение убираем, потому как там всё равно ничего интересного не будет.
  // Также наложение убираем, если мы не можем сделать его (величина сдвига получилась меньше величины наложения)
  if (gap_size > overlap_pixels || shift_value < overlap_pixels)
    return start_of_frame-left_screen_edge;
  else
  {
    int shift_candidate=start_of_frame-left_screen_edge-overlap_pixels; // Кандидат на значение для сдвига
    int next_frame_start=frame_map[FRAME_START][frame_number+1]; // Начало следующего за выбранным кадра
    if (next_frame_start > left_screen_edge+shift_candidate) // Если справа от выбранного кадра только пустота - не надо делать наложение!
      return next_frame_start-left_screen_edge;
    else
      return shift_candidate;
  }
}

/*то же что и shift() но в другую сторону, влевo */
int shift_back (int left_screen_edge, int frames_count, int frame_map[][FRAMES_MAX], int display_size)
{
  int frame_number;	/*индекс для frame_map[][] */
  int end_of_frame;	/*конец кадра */
  int right_screen_edge=left_screen_edge+display_size; // Правый край экрана
  int overlap_pixels=display_size*overlap/100; // Величина на которую накладывается на предыдущее новое изображение при прокрутке
  int gap_size;

  if (frames_count < 2) // Если кадров мы не нашли - отдаём ширину экрана
    return display_size-overlap_pixels;

  for(frame_number = frames_count - 1; frame_number >= 0; frame_number--)
  {
    if (frame_map[FRAME_START][frame_number] < left_screen_edge) // Если начало кадра за левой границей экрана - берём его
      break;
  }

  end_of_frame = frame_map[FRAME_END][frame_number];
  gap_size = frame_map[FRAME_START][frame_number+1] - frame_map[FRAME_END][frame_number]; // Зазор между кадрами

  if (gap_size > display_size) // Если зазор между кадрами больше экрана - возвращаем значение чтобы максимально отобразить интересное до этого зазора:
  {
    if (left_screen_edge < frame_map[FRAME_START][frame_number+1]) // Если хоть пиксели из зазора отображаются на экране - вычитаем их из смещения
      return gap_size+display_size-(frame_map[FRAME_START][frame_number+1]-left_screen_edge);
    else // А иначе - возвращаем значение чтобы конец кадра совпал с концом экрана
      return gap_size+display_size;
  }

  // Если зазор между кадрами больше величины наложения - наложение убираем, потому как там всё равно ничего интересного не будет
  if (gap_size > overlap_pixels)
    overlap_pixels=0;

  // Если и конец кадра тоже за левым краем экрана - возвращаем разницу между его концом и текущим положением правого края экрана
  if (end_of_frame <= left_screen_edge)
    return left_screen_edge-end_of_frame < overlap_pixels ? right_screen_edge-end_of_frame : right_screen_edge-end_of_frame-overlap_pixels;
  else
  {
    if (end_of_frame < left_screen_edge+display_size) // Если конец кадра показан на экране до правой границы - значит кадр не влез на экран) - стараемся показать максимально полно этот не влезший кадр (без наложения!)
      return right_screen_edge-end_of_frame;
    else // А иначе (если конец кадра показан на экране, а значит кадр больше экрана) - прокручиваем ровно на страницу
      return display_size-overlap_pixels;
  }
}
