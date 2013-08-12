/* Soul Trace, 2013, Distributed under GPLv2 Terms */
extern int need_refresh;//Показатель, что изображение необходимо перерисовать из-за изменения настроек
void start_main_menu (panel *panel);
void options_menu_create(GtkWidget *main_menu);
void start_picture_menu (panel *panel, GtkWidget *win); // Создаём меню настроек картинки