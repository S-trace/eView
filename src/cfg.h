void write_config_int(const char *name, int value);
void write_config_string(const char *name, const char *value);
void create_cfg (void);
void read_configuration (void);
void write_archive_stack(const char *name, struct_panel *panel);

char *cfg_file_path (void);
void reset_config(void);
extern char archive_stack[16][PATHSIZE+1];/*Стек имён архивов */
extern gboolean crop;   /* автообрезка бордюра вокруг картинки */
extern gboolean split_spreads; /* разрезать странички с отсканированными разворотами вместо того чтобы поворачивать их и показывать целиком */
extern gboolean rotate; /* режим просмотра с поворотом картинки */
extern gboolean frame;  /* режим сдвига с учетом кадров на картинке (требует rotate)*/
extern gboolean web_manga_mode; /* Режим веб-манги (без поворота, прокрутка вертикально) */
extern int overlap;     /* Значение (в процентах), на которое будет накладываться изображение при прокрутке в режиме поворота */
extern gboolean manga;  /* режим просмотра как манги */
extern gboolean keepaspect;/* растянуть ли картинку на весь экран */
extern gboolean fm_toggle;    /*чекбокс файлменеджер */
extern gboolean move_toggle;  /*чекбокс подтверждение перемещения */
extern gboolean speed_toggle; /*режим обновления (1 для нормального) */
extern gboolean show_clock; /*чекбокс показывать панельку с часами */
extern gboolean top_panel_active;/*какая панель в фокусе */
extern gboolean loop_dir; /* Режим зацикливания при окончании файлов, 0 - ничего, 1 - зацикливание, 2 - переход в следующий каталог, 3 - переход в файлменеджер */
extern gboolean double_refresh;/*двойное обновление в смотрелке */
extern gboolean viewed_pages;/*Статистика - число просмотренных страниц */
extern gboolean preload_enable;/*Разрешить ли предзагружать спедующее изображение */
extern gboolean caching_enable;/*Разрешить ли кэщировать предыдущее изображение */
extern gboolean suppress_panel;/*Подавлять ли панель и сообщения об нехватке батареи */
extern gboolean show_hidden_files;/*Показывать ли в файл-менеджере скрытные каталоги и файлы */
extern gboolean LED_notify; /* Оповещение светодиодом об обновлении панелей и загрузке */
extern gboolean HD_scaling; /* Высококачественное масштабирование изображений */
extern gboolean boost_contrast; /* Усиление контраста */
extern int refresh_type; /* Тип используемого обновления экрана - определяется автоматически, не нужно устанавливать вручную! */
extern int backlight; /* Уровень яркости подсветки */
extern int sleep_timeout; /* Таймер сна на Qt прошивках */
extern char *system_sleep_timeout;  /* Таймер сна в системе на Qt прошивках */
#ifndef _loop_dir
#define _loop_dir
enum
{
  LOOP_NONE,
  LOOP_LOOP,
  LOOP_NEXT,
  LOOP_EXIT,
  LOOP_DIR
};
#endif
