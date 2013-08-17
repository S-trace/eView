/* Soul Trace, 2013, Distributed under GPLv2 Terms
 * save & load settings fuctions*/
#ifndef __cplusplus
#define _GNU_SOURCE
#endif
#include <stdlib.h> // atoi()
#include <string.h>
#include <stdio.h>
#include <sys/stat.h> //mkdir()
#include "gtk_file_manager.h"
#include "mylib.h"
#include "cfg.h"

static char *cfg_directory = NULL; //путь к файлу с настройками
int crop, rotate, frame, keepaspect, fm_toggle, move_toggle, speed_toggle, clock_toggle, top_panel_active, loop_dir, double_refresh, viewed_pages, preload_enable, suppress_panel, show_hidden_files, manga, LED_notify=TRUE, backlight, sleep_timeout;

char *cfg_file_path (void)
{
  cfg_directory = xconcat_path_file(xgetcwd (cfg_directory), ".eView");
  return cfg_directory;
}

int read_config_int(const char *name) //Чтение числового параметра конфига
{
  int value;
  char *config_file_single = xconcat_path_file(cfg_directory, name), value_string[32]; // Имя файла с настройкой
  FILE *file_descriptor=fopen(config_file_single,"rt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN %s SETTING FILE FOR READ! IT'S BAD!\n", config_file_single);
    #endif
    write_config_int(name, 0);
    return 0;
  }
  else
  {
    fgets(value_string,32,file_descriptor);
    value=atoi(value_string);
    fclose(file_descriptor);
    #ifdef debug_printf
    printf("reading %s from %s (%d)\n", name, config_file_single, value);
    #endif
    return(value);
  }
}

void read_config_string(char *name, char **destination) //Чтение строкового параметра конфига из файла name в переменную destination
{
  char temp[256], *config_file_single = xconcat_path_file(cfg_directory, name); // Имя файла с настройкой
  FILE *file_descriptor=fopen(config_file_single,"rt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN %s SETTING FILE FOR READ! IT'S BAD!\n", config_file_single);
    #endif
    write_config_string(name, "");
    return;
  }
  else
  {
    if (fgets(temp, PATHSIZE, file_descriptor) == 0)
    {
      fclose(file_descriptor);
      *destination='\0';
      #ifdef debug_printf
      printf("reading %s from %s (%s)\n", name, config_file_single,*destination);
      #endif
      return;
    }
    *destination=strdup(temp);
    fclose(file_descriptor);
    #ifdef debug_printf
    printf("reading %s from %s (%s)\n", name, config_file_single, *destination);
    #endif
    return;
  }
}

void read_archive_stack(const char *name, panel *panel) //Чтение стека архивов
{
  char *config_file_single = xconcat_path_file(cfg_directory, name); // Имя файла с настройкой
  FILE *file_descriptor=fopen(config_file_single,"rt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN %s SETTING FILE FOR READ! IT'S BAD!\n", config_file_single);
    #endif
    write_config_string(name, "filesystem\n");
    return;
  }
  else
  {
    int i=0;
    #ifdef debug_printf
    printf("reading %s from '%s' (archive stack)\n", name, config_file_single);
    #endif
    while(!feof(file_descriptor) && i <= MAX_ARCHIVE_DEPTH )
    {
      #ifdef debug_printf
      printf("Reading %d element from '%s' (archive stack)\n", i, config_file_single);
      #endif    
      fgets(panel->archive_stack[i], PATHSIZE, file_descriptor);
      if (panel->archive_stack[i][0] == '\0')
      {
        #ifdef debug_printf
        printf("Readed empty line, break\n");
        #endif    
        break; // Прерывание при обнаружении пустой строки в стеке архивов
      }
      trim_line(panel->archive_stack[i]);
      i++;
    }
    #ifdef debug_printf
    printf("Readed %d elements from '%s' (archive stack)\n", i, config_file_single);
    #endif    
    fclose(file_descriptor);
    panel->archive_depth=--i;
    return;
  }
}

void write_config_int(const char *name, int value) //Запись числового параметра конфига
{
  char *config_file_single = xconcat_path_file(cfg_directory, name), *value_string; // Имя файла с настройкой
  #ifdef debug_printf
  printf("writing %d to %s\n", value, config_file_single);
  #endif
  FILE *file_descriptor=fopen(config_file_single,"wt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN %s SETTING FILE FOR WRITING! IT'S BAD!\n", config_file_single);
    #endif
    return;
  }
  else
  {
    value_string=itoa(value);
    fputs(value_string,file_descriptor);
    fclose(file_descriptor);
  }
}

void write_config_string(const char *name, const char *value) //Запись строкового параметра конфига
{
  char *config_file_single = xconcat_path_file(cfg_directory, name); // Имя файла с настройкой
  #ifdef debug_printf
  printf("writing %s to %s\n", value, config_file_single);
  #endif
  FILE *file_descriptor=fopen(config_file_single,"wt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN %s SETTING FILE FOR WRITING! IT'S BAD!\n", config_file_single);
    #endif
    return ;
  }
  else
  {
    fprintf(file_descriptor, "%s", value);
    fclose(file_descriptor);
  }
}

void write_archive_stack(const char *name, panel *panel) //Запись массива имён архивов из верхней панели
{
  char *config_file_single = xconcat_path_file(cfg_directory, name); // Имя файла с настройкой
  #ifdef debug_printf
  printf("writing array to '%s'\n", config_file_single);
  #endif
  int i=0;
  FILE *file_descriptor=fopen(config_file_single,"wt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN '%s' SETTING FILE FOR WRITING! IT'S BAD!\n", config_file_single);
    #endif
    return ;
  }
  else
  {
    while (panel->archive_stack[i][0] != '\0')
    {
      #ifdef debug_printf
      printf("writing '%s' to '%s'\n", panel->archive_stack[i], config_file_single);
      #endif
      fprintf(file_descriptor, "%s\n", panel->archive_stack[i]);
      i++;
    }
    fclose(file_descriptor);
  }
}

void create_cfg (void)  //создание файлов настроек по умолчанию
{
  if ((mkdir (cfg_directory, S_IRWXU)) == -1)
  {
    #ifdef debug_printf
    printf("UNABLE TO mkdir %s! IT'S BAD!\n", cfg_directory);
    #endif
  }
  char *current_dir = get_current_dir_name();
  write_config_int("crop", TRUE);
  write_config_int("rotate", FALSE);
  write_config_int("frame", FALSE);
  write_config_int("manga", FALSE);
  write_config_int("keepaspect", TRUE);
  write_config_int("fm_toggle", FALSE);
  write_config_int("move_toggle", TRUE);
  write_config_int("speed_toggle", FALSE);
  write_config_int("clock_toggle", TRUE);
  write_config_int("top_panel_active", TRUE);
  write_config_int("loop_dir", LOOP_NONE);
  write_config_int("double_refresh", FALSE);
  write_config_int("in_archive", FALSE);
  write_config_int("viewed_pages", 0);
  write_config_int("preload_enable", TRUE);
  write_config_int("suppress_panel", FALSE);
  write_config_int("show_hidden_files", FALSE);
  write_config_int("LED_notify", TRUE);
  write_config_int("backlight", FALSE);
  write_config_int("sleep_timeout", 60);
  write_config_string("top_panel.path", "/");
  write_config_string("top_panel.selected_name", "../");
  write_config_string("top_panel.archive_cwd", "");
  write_config_string("top_panel.last_name", "");
  write_config_string("top_panel.archive_stack", "filesystem\n");
  write_config_string("top_panel.archive_list", "/tmp/top.archive_list");  
  
  write_config_string("bottom_panel.path", "/");
  write_config_string("bottom_panel.selected_name", "../");
  write_config_string("bottom_panel.archive_cwd", "");
  write_config_string("bottom_panel.last_name", "");
  write_config_string("bottom_panel.archive_stack", "filesystem\n");
  write_config_string("bottom_panel.archive_list", "/tmp/bottom.archive_list");
  xfree(&current_dir);
}

void read_panel_configuration(panel *panel)
{
  const char *name_prefix;
  if (panel == &top_panel)
    name_prefix = "top";
  else
    name_prefix = "bottom";
  #ifdef debug_printf
  printf("Reading %s panel configuration\n", name_prefix);
  #endif    
  
  read_config_string(xconcat(name_prefix, "_panel.path"), &panel->path);
  read_config_string(xconcat(name_prefix, "_panel.selected_name"), &panel->selected_name);
  read_config_string(xconcat(name_prefix, "_panel.archive_cwd"), &panel->archive_cwd);
  read_config_string(xconcat(name_prefix, "_panel.archive_list"), &panel->archive_list);
  read_config_string(xconcat(name_prefix, "_panel.last_name"), &panel->last_name);
  read_archive_stack(xconcat(name_prefix, "_panel.archive_stack"), panel);
}

void read_configuration (void)
{
  crop=read_config_int("crop");
  rotate=read_config_int("rotate");
  frame=read_config_int("frame");
  keepaspect=read_config_int("keepaspect");
  manga=read_config_int("manga");
  fm_toggle=read_config_int("fm_toggle");
  move_toggle=read_config_int("move_toggle");
  speed_toggle=read_config_int("speed_toggle");
  clock_toggle=read_config_int("clock_toggle");
  top_panel_active=read_config_int("top_panel_active");
  loop_dir=read_config_int("loop_dir");
  double_refresh=read_config_int("double_refresh");
  viewed_pages=read_config_int("viewed_pages");
  preload_enable=read_config_int("preload_enable");
  suppress_panel=read_config_int("suppress_panel");
  show_hidden_files=read_config_int("show_hidden_files");
  LED_notify=read_config_int("LED_notify");
  backlight=read_config_int("backlight");
  sleep_timeout=read_config_int("sleep_timeout");
  
  read_panel_configuration(&top_panel);
  read_panel_configuration(&bottom_panel);
}

void reset_config(void)
{
  #ifdef debug_printf
  printf("Removing '%s'\n", cfg_directory);
  #endif
  char *command;
  asprintf(&command, "rm -rf \"%s\"", cfg_directory);
  xsystem (command);
  xfree(&command);
}
