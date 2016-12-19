/* Norin Maxim, 2011, Distributed under GPLv2 Terms.
 *Debug message window special for eInk*/
#include <gtk/gtk.h>
#include <stdio.h>
#include "gtk_file_manager.h" /* Инклюдить первой среди своих, ибо typedef panel! */
#include "digma_hw.h"

static GtkWidget *dbg_win;
static GtkWidget *vbox;

gint if_key_press ()
{
  gtk_widget_hide (dbg_win);
  while (gtk_events_pending ())    gtk_main_iteration ();
  epaperUpdatePart(NULL);
  while (gtk_events_pending ())    gtk_main_iteration ();
  epaperUpdateFull(NULL);
  return FALSE;
}

void print_msg (char *msg)
{
  GtkWidget *label;
  /*TRACE("msg=%s\n", msg); */
  label = gtk_label_new (msg);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
  gtk_window_set_position (GTK_WINDOW (dbg_win), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  gtk_widget_show_all (dbg_win);
  while (gtk_events_pending ())    gtk_main_iteration ();
  epaperUpdatePart(NULL);
  while (gtk_events_pending ())    gtk_main_iteration ();
  epaperUpdateFull(NULL);
}

void debug_msg_win ()
{
  dbg_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(dbg_win), 200, 300);
  gtk_window_set_position (GTK_WINDOW (dbg_win), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_set_title (GTK_WINDOW (dbg_win), "debug message window");
  gtk_container_set_border_width(GTK_CONTAINER(dbg_win), 3);
  g_signal_connect (G_OBJECT (dbg_win), "key_press_event", G_CALLBACK (if_key_press), NULL);
  vbox = gtk_vbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (dbg_win), vbox);
}
