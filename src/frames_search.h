#include <gtk/gtk.h>
#include <string.h>
#define FRAME_START 0 /*индексы массива frame_map*/
#define FRAME_END 1
#define SHIFT_MAP 2 // Массив значений для shift()
#define FRAME_SIZE 32 /*минимально возможный размер кадра в пикселах*/ // Было 160
#define SEPARATOR_LINE_MAX 2048 /*сколько линий-разделителей может быт подряд*/ // Было 200

int frames_search(image *target, int page, int frames_map[][FRAMES_MAX]);
int right_way (int frames_map[][FRAMES_MAX]);
int left_way(int frames_map[][FRAMES_MAX]);
int left_way_sc (int frames_map[][FRAMES_MAX]);
int left_way_fc (int frames_map[][FRAMES_MAX]);
int line_separator (void);
void frame_map_clear(int frames_map[][FRAMES_MAX]);
