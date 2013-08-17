/*
 * 
 * Gtk+2.x Utility routines
 * Copyright (C) 2002-2006 by Tito Ragusa <tito-wolit@tiscali.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Please read the COPYING and README file!!!
 *
 * Report bugs to <tito-wolit@tiscali.it>
 *
 */
#ifndef	__HAVE_MYGTK_
#define	__HAVE_MYGTK_ 1
/*
 *	This is added to avoid a gcc warning with -Wshadow and gtk+2.6.x:
 *	gtkaboutdialog.h:108: warning: declaration of 'link' shadows a global declaration
 *	/usr/include/unistd.h:722: warning: shadowed declaration is here
 *	as mygtk doesn't use gtkaboutdialog
 */
#define __GTK_ABOUT_DIALOG_H__

#define set_editable(x,y)				gtk_editable_set_editable(GTK_EDITABLE (x),y)
#define get_radio_button_group(x)		gtk_radio_button_get_group(GTK_RADIO_BUTTON (x))
#define	set_position					gtk_window_set_position
#define clear_list(x)					gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(x))))
#define signal_handlers_destroy			g_signal_handlers_destroy
#define shrink_widget(x,y,z)			gtk_widget_set_size_request(x,y,z)
#define timeout_remove					g_source_remove
#define timeout_add						g_timeout_add

#define DEF_BUFFER_SIZE		 512
#define NOT_EDITABLE	((int) 0)
#define EDITABLE		((int) 1)
#define CUSTOM			((int)-1)
#define ACTIVE 			((int) 0)
#define INACTIVE		((int) 1)
#define HORIZONTAL		((int) 0)
#define VERTICAL		((int) 1)
#define	NOT_MODAL		((int) 0)
#define	MODAL			((int) 1)
#define SHOW			((int) 0)
#define HIDE			((int) 1)
#define WRAP			((int) 0)
#define NO_WRAP			((int) 1)
#define SCROLL			((int) 0)
#define NO_SCROLL		((int) 1)
#define AUTOSCROLL		((int) 1)
#define NO_AUTOSCROLL	((int) 0)

#define	SHOW_FILE_OP_BUTTONS	((int) 0)
#define	HIDE_FILE_OP_BUTTONS	((int) 1)

#define	ONE_BUTTON				((int) 1)
#define	ONE_BUTTON_ONE_CHECK	((int) 2)
#define	TWO_BUTTONS				((int) 3)
#define	TWO_BUTTONS_ONE_CHECK	((int) 4)
#define	THREE_BUTTONS			((int) 5)

#define	NOT_HOMOGENEUS	((int) 0)
#define HOMOGENEUS		((int) 1)

#define	NOT_FIRST_TIME	((int) 0)
#define FIRST_TIME		((int) 1)

#ifdef DEBUG
#define RESET(x)		err_msg(#x": %d",(x)=0)
#define SET(x) 			err_msg(#x": %d",(x)=1)
#define SET2(x,y) 		err_msg(#x": %d",(x)=(y))
#else
#define RESET(x) 		(x)=0
#define SET(x) 			(x)=1
#define SET2(x,y)		(x)=(y)
#endif

#define IS_SET(x)		(x)==1
#define IS_NOT_SET(x)	(x)==0

#define ACTIVITY		TRUE
#define PULSE			FALSE

#define SCREEN_MARGIN	100

# define ATTRIBUTE_UNUSED __attribute__ ((__unused__))

void window_close(GtkWidget *widget, GtkWidget *window);
void main_loop(GtkWidget **window);
void attach_to_table(GtkWidget *table, GtkWidget *widget, int start_col, int end_col, int start_row, int end_row);

/********************* Text box widget ***************************************/
GtkWidget *text_box_create(char *text, int editable);
GtkWidget *default_text_box_create_on_table(char *text, int editable, GtkWidget *table,
                                            int start_col, int end_col, int start_row, int end_row);
GtkWidget *text_box_create_on_table(char *text, int editable, GtkWidget * table,
                                    int start_col, int end_col, int start_row, int end_row,
                                    GtkAttachOptions xoptions,  GtkAttachOptions yoptions,
                                    guint xpadding, guint ypadding, int maxlen, int width);
void my_gtk_entry_set_text(GtkEntry *entry, char *data);
char *my_gtk_entry_get_text(GtkEntry *entry);
/********************* End of Text box widget ********************************/

/********************* Standard radio button widget **************************/

GtkWidget *radio_create(GtkWidget *last_of_group, char *label,
                        GtkSignalFunc func, gpointer func_data, int active );

/****************End of Standard radio button widget *************************/

/********************* Radio button on table widget **************************/

GtkWidget *radio_create_on_table(GtkWidget *last_of_group, char *label, GtkSignalFunc func, gpointer func_data,
                                 int active, GtkWidget *table, int start_col, int end_col,
                                 int start_row, int end_row );
GtkWidget *default_radio_create_on_table(GtkWidget *last_of_group, char *label, GtkSignalFunc func,
                                         int active, GtkWidget *table, int start_col, int end_col,
                                         int start_row, int end_row );

/****************End of radio button on table widget *************************/

/********************* Check button widgets **************************/

GtkWidget *check_create(char *label, GtkSignalFunc func, gpointer func_data, int active );
GtkWidget *check_create_on_table(char *label, GtkSignalFunc func, gpointer func_data, int active,
                                 GtkWidget *table, int start_col, int end_col, int start_row, int end_row );
GtkWidget *default_check_create_on_table(char *label, GtkSignalFunc func, int active,
                                         GtkWidget *table, int start_col, int end_col, int start_row, int end_row );

/***************End of Check button widgets **********************************/

/********************* Standard button widget ********************************/

GtkWidget *button_create(char *label, GtkSignalFunc func, gpointer func_data );

/****************End of Standard button widget *******************************/

/********************* Button on table widget ********************************/

GtkWidget *button_create_on_table(char *label, GtkSignalFunc func, gpointer func_data,
                                  GtkWidget *table, int start_col, int end_col, int start_row, int end_row );

/****************End of button on table widget ******************************/

/********************* Button in container widget ***************************/

GtkWidget *button_create_in_container(char *label, GtkSignalFunc func, gpointer func_data,
                                      GtkWidget *container);

/*************End of Button in container widget *****************************/

/********************* Buttonbox on table widget ****************************/

GtkWidget *button_box_create_on_table(int orientation, int layout,  int border, int spacing, GtkWidget *table,
                                      int start_col, int end_col, int start_row, int end_row );
GtkWidget *default_button_box_create_on_table(	int orientation, GtkWidget *table,
                                                int start_col, int end_col,
                                               int start_row, int end_row );

/***************End of Buttonbox on table widget ****************************/

/********************* Standard table widget ********************************/
GtkWidget *new_table_create(int cols, int rows, int border, int homogeneus);
GtkWidget *table_create(int cols, int rows, GtkWidget* window , int border, int homogeneus);
void start_menu (void);
void start_pmenu (panel *panel, GtkWidget *win);
// GtkWidget *table_create_on_table(int cols , int rows,  GtkWidget* table, int border, int homogeneus, int start_col, int end_col, int start_row, int end_row);
/********************* Standard table widget ********************************/

/********************* Standard window widget ********************************/

GtkWidget *window_create( int x, int y, int border, const char *title,
                          int modal);

/************** Create a separator on table   ********************************/

GtkWidget *separator_create_on_table(int direction, GtkWidget *table,
                                     int start_col,int end_col, int start_row, int end_row );

/**********End of  Create a separator on table********************************/

/************** Create a Multi column scrolled list ************************/
GtkWidget *string_list_create(int num, int show_hide, int editable,...);
GtkTreeView *string_list_create_on_table(int num, GtkWidget *table, int start_col, int end_col,
                                       int start_row, int end_row ,int show_hide, int editable, ...);
void add_data_to_list(GtkTreeView *tree, const char *data_string, int n_columns, int autoscroll, const char *fs);

/************** Create a Permissions window **********************************/
typedef	struct	Perm_data			Perm_data;
struct Perm_data {
  GtkWidget	*window;
  int *ptr_dialog_retval;
  GtkWidget	*user_entry;
  GtkWidget	*group_entry;
  char* user;
  char* group;
  long uid;
  long gid;
};

int PermissionWindow(char *filename);

/*****************End of Permissions window **********************************/

/************** Text window Common  functions ********************************/

FILE *open_file(char *filename );
int read_and_convert(char * temp_ptr, GtkTextBuffer *buffer);
void read_from_stream_and_display_text(GtkWidget *text, FILE *fd, unsigned long filesize, gdouble *progress);
void clear_the_buffer(GtkWidget *text, int *text_modified);


void wait_state(GtkWidget *window);
/************** Create a Image Viewer Window  *************************/
/************** Create a Pack box  ************************************/

GtkWidget *box_create( int orientation, int homogeneous, int spacing);
GtkWidget *default_box_create_on_table( int orientation, int homogeneous, GtkWidget * table,
                                        int spacing, int start_col, int end_col, int start_row, int end_row);
GtkWidget *box_create_on_table(int orientation, int homogeneous, GtkWidget * table,
                               int spacing, int start_col, int end_col, int start_row, int end_row,
                               GtkAttachOptions xoptions,  GtkAttachOptions yoptions,
                               guint xpadding, guint ypadding);
GtkWidget *path_label_to_box(void);
void focus_on_fmanager (void);
void up_panel_activated (int a);
void menu_options_activated(void);
/************** Create a Pack box  ************************************/

#endif /* __HAVE_MYGTK_  */
gboolean e_ink_refresh_part(void);
gboolean e_ink_refresh_local(void);
gboolean e_ink_refresh_full(void);
gboolean e_ink_refresh_default(void);
#define COLUMN_W (width_display * 83 / 100) // Первая колонка в файлменеджере (для имён). 80 - ширина в процентах
#define COLUMN_W2 (width_display * 17 / 100)// Вторая колонка в файлменеджере (для размеров). 20 - ширина в процентах
void Message(const char *title,const char *message);
extern GtkWidget *MessageWindow;
extern int enable_refresh;//Принудительно запретить обновлять экран в особых случаях
// extern char focus_in_processed; // Количество обработанных событий получения фокуса
// gint focus_in_callback (void); // реакция на получение фокуса
// gint focus_out_callback (void); // реакция на потерю фокуса
void wait_for_draw (void); 
gint confirm_request(const char *title, const char *confirm_button, const char *reject_button);
void create_panel (panel *panel);
char *get_current_iter (panel *panel); //возвращает итератор текущего файла из списка
void Qt_error_message(const char *message); // Функция открывает на прошивках Qt сообщение об ошибке в стандартной читалке
void enter_suspend(panel *panel); // Показывает скринсейвер и усыпляет железо
int check_key_press(int keyval, panel *panel); // Проработка нажатия кнопки питания при сне
