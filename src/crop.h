#define BORDER_SIZE 110 /*предельная глубина бордюра в пикселах*/
#define PIXEL_COLOR_MAX 5 /*максимум случайных цветных пикселей при опр.борд*/
#define PIXEL_RANDOM_MAX 4/*максимум любых случайных пикселов в зоне бордюра*/
#define PIXEL_RESET_COUNT 100/*через сколько "нормальных" пикселов подряд сбросится счетчик pixel_random_max*/
#define WHITE 150 /*белым цветом будет считатся от WHITE до 255*/
#define BLACK 40 /*черным цветом будет считатся от 0 до BLACK*/
#define GREY 10 /*максимальный процент наличия других тонов черно-белого при
определении цвета бордюра */

void find_x_crop (int height);
void find_y_crop (int width);
void find_width_crop (int width, int height);
void find_height_crop (int width, int height);
void find_crop_image_coords (const image *target);
int return_crop_coord (int i) __attribute__((pure));
