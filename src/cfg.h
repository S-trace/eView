// #include "debug_msg_win.h"
#define PATHSIZE 256
void write_config_int(const char *name, int value);
void write_config_string(const char *name, const char *value);
void create_cfg (void);
void read_configuration (void);
void write_archive_stack(const char *name, panel *panel);

char *cfg_file_path(void);
void reset_config(void);
extern char *filename, *archive_name, *archive_cwd;
extern char archive_stack[16][PATHSIZE+1];//Стек имён архивов|
extern int crop;   //0 выкл 1 вкл - автообрезка бордюра вокруг картинки
extern int rotate; //0 выкл 1 вкл - удвоение размера с поворотом
extern int frame;  //0 выкл 1 вкл - режим сдвига с учетом кадров на картинке
extern int manga;  //0 выкл 1 вкл - режим просмотра как манги
extern int keepaspect;//0 выкл 1 вкл - растянуть ли картинку на весь экран
extern int fm_toggle;    //чекбокс файлменеджер
extern int move_toggle;  //чекбокс подтверждение перемещения
extern int speed_toggle; //режим обновления 1 для нормального
extern int clock_toggle; //чекбокс показывать панельку с часами
extern int top_panel_active;//какая панель в фокусе
extern int loop_dir; // Режим зацикливания при окончании файлов, 0 - ничего, 1 - зацикливание, 2 - переход в следующий каталог, 3 - переход в файлменеджер
extern int double_refresh;//двойное обновление в смотрелке
extern int viewed_pages;//Статистика - число просмотренных страниц
extern int preload_enable;//Разрешить ли предзагружать спедующее изображение
extern int suppress_panel;//Подавлять ли панель и сообщения об нехватке батареи
extern int show_hidden_files;//Показывать ли в файл-менеджере скрытные каталоги и файлы
extern int LED_notify; // Оповещение светодиодом об обновлении панелей и загрузке
extern int backlight; // Уровень яркости подсветки
extern int sleep_timeout; // Таймер сна на Qt прошивках

#ifndef _loop_dir
#define _loop_dir
enum
{
  LOOP_NONE,
  LOOP_LOOP,
  LOOP_NEXT,
  LOOP_EXIT,
  LOOP_DIRECTORY
};
#endif
