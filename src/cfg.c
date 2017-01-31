// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is an open source non-commercial project. Dear PVS-Studio, please check it.
/* Soul Trace, 2013, Distributed under GPLv2 Terms
 * save & load settings fuctions*/
#include <stdlib.h> /* atoi() */
#include <string.h>
#include <stdio.h>
#include <sys/stat.h> /*mkdir() */
#include "gtk_file_manager.h"
#include "mylib.h"
#include "cfg.h"
#include "digma_hw.h"

static char *cfg_directory; /*путь к файлу с настройками */
int crop, split_spreads, rotate, frame, web_manga_mode, overlap, keepaspect, fm_toggle, move_toggle, speed_toggle, show_clock, top_panel_active, loop_dir, double_refresh, viewed_pages, preload_enable, caching_enable, suppress_panel, show_hidden_files, manga, HD_scaling, boost_contrast, refresh_type, LED_notify=TRUE;
int backlight, sleep_timeout;
char *system_sleep_timeout;

int read_config_int(const char *name) /*Чтение числового параметра конфига */
{
  char *config_file_single = xconcat_path_file(cfg_directory, name); /* Имя файла с настройкой */
  FILE *file_descriptor=fopen(config_file_single,"rt");
  if (!file_descriptor)
  {
    TRACE("UNABLE TO OPEN %s SETTING FILE FOR READ! IT'S BAD!\n", config_file_single);
    write_config_int(name, 0);
    free(config_file_single);
    return 0;
  }
  else
  {
    int value;
    char value_string[33];
    (void)fgets(value_string,32,file_descriptor);
    value=atoi(value_string);
    (void)fclose(file_descriptor);
    TRACE("Reading %s from %s (%d)\n", name, config_file_single, value);
    free(config_file_single);
    return(value);
  }
}

void read_config_string(char *name, char **destination) /*Чтение строкового параметра конфига из файла name в переменную destination */
{
  char *config_file_single = xconcat_path_file(cfg_directory, name); /* Имя файла с настройкой */
  FILE *file_descriptor=fopen(config_file_single,"rt");
  if (!file_descriptor)
  {
    TRACE("UNABLE TO OPEN %s SETTING FILE FOR READ! IT'S BAD!\n", config_file_single);
    write_config_string(name, "");
    free(config_file_single);
    return;
  }
  else
  {
    char temp[257];
    if (fgets(temp, PATHSIZE, file_descriptor) == 0)
    {
      TRACE("Reading %s from %s failed\n", name, config_file_single);
      temp[0]='\0';
    }
    else
    {
      TRACE("Reading %s from %s (%s) successed\n", name, config_file_single, *destination);
    }
    *destination=strdup(temp);
    free(config_file_single);
    (void)fclose(file_descriptor);
    return;
  }
}

void read_archive_stack(const char *name, struct_panel *panel) /*Чтение стека архивов */
{
  char *config_file_single = xconcat_path_file(cfg_directory, name); /* Имя файла с настройкой */
  FILE *file_descriptor=fopen(config_file_single,"rt");
  if (!file_descriptor)
  {
    TRACE("UNABLE TO OPEN %s SETTING FILE FOR READ! IT'S BAD!\n", config_file_single);
    write_config_string(name, "filesystem\n");
    free(config_file_single);
    return;
  }
  else
  {
    int i=0;
    TRACE("Reading %s from '%s' (archive stack)\n", name, config_file_single);
    while((feof(file_descriptor) == FALSE) && i <= MAX_ARCHIVE_DEPTH )
    {
      TRACE("Reading %d element from '%s' (archive stack)\n", i, config_file_single);
      (void)fgets(panel->archive_stack[i], PATHSIZE, file_descriptor);
      if (panel->archive_stack[i][0] == '\0')
      {
        TRACE("Readed empty line, break\n");
        break; /* Прерывание при обнаружении пустой строки в стеке архивов */
      }
      trim_line(panel->archive_stack[i]);
      i++;
    }
    TRACE("Readed %d elements from '%s' (archive stack)\n", i, config_file_single);
    free(config_file_single);
    (void)fclose(file_descriptor);
    panel->archive_depth=--i;
    return;
  }
}

void write_config_int(const char *name, int value) /*Запись числового параметра конфига */
{
  char *config_file_single = xconcat_path_file(cfg_directory, name); /* Имя файла с настройкой */
  FILE *file_descriptor=fopen(config_file_single,"wt");
  TRACE("writing %d to %s\n", value, config_file_single);
  if (!file_descriptor)
  {
    TRACE("UNABLE TO OPEN %s SETTING FILE FOR WRITING! IT'S BAD!\n", config_file_single);
    free(config_file_single);
    return;
  }
  else
  {
    char *value_string=itoa(value);
    (void)fputs(value_string,file_descriptor);
    free(config_file_single);
    free(value_string);
    (void)fclose(file_descriptor);
  }
}

void write_config_string(const char *name, const char *value) /*Запись строкового параметра конфига */
{
  char *config_file_single = xconcat_path_file(cfg_directory, name); /* Имя файла с настройкой */
  FILE *file_descriptor=fopen(config_file_single,"wt");
  TRACE("writing %s to %s\n", value, config_file_single);
  if (!file_descriptor)
  {
    TRACE("UNABLE TO OPEN %s SETTING FILE FOR WRITING! IT'S BAD!\n", config_file_single);
    free(config_file_single);
    return ;
  }
  else
  {
    fprintf(file_descriptor, "%s", value);
    free(config_file_single);
    (void)fclose(file_descriptor);
  }
}

void write_archive_stack(const char *name, struct_panel *panel) /*Запись массива имён архивов из верхней панели */
{
  char *config_file_single = xconcat_path_file(cfg_directory, name); /* Имя файла с настройкой */
  FILE *file_descriptor=fopen(config_file_single,"wt");
  TRACE("writing array to '%s'\n", config_file_single);
  if (!file_descriptor)
  {
    TRACE("UNABLE TO OPEN '%s' SETTING FILE FOR WRITING! IT'S BAD!\n", config_file_single);
    free(config_file_single);
    return ;
  }
  else
  {
    int i=0;
    while (panel->archive_stack[i][0] != '\0')
    {
      TRACE("writing '%s' to '%s'\n", panel->archive_stack[i], config_file_single);
      fprintf(file_descriptor, "%s\n", panel->archive_stack[i]);
      i++;
    }
    free(config_file_single);
    (void)fclose(file_descriptor);
  }
}

// cppcheck-suppress "unusedFunction"
void create_cfg (void)  /*создание файлов настроек по умолчанию */
{
  char *current_dir=xgetcwd (cfg_directory);
  cfg_directory = xconcat_path_file(current_dir, ".eView");
  free(current_dir);
  if ((mkdir (cfg_directory, S_IRWXU)) == -1)
  {
    TRACE("UNABLE TO mkdir %s! IT'S BAD!\n", cfg_directory);
  }
  current_dir = get_current_dir_name();
  write_config_int("crop", crop=TRUE);
  write_config_int("split_spreads", split_spreads=FALSE);
  write_config_int("rotate", rotate=FALSE);
  write_config_int("web_manga_mode", web_manga_mode=FALSE);
  write_config_int("frame", frame=FALSE);
  write_config_int("overlap", overlap=10);
  write_config_int("manga", manga=FALSE);
  write_config_int("keepaspect", keepaspect=TRUE);
  write_config_int("fm_toggle", fm_toggle=FALSE);
  write_config_int("move_toggle", move_toggle=TRUE);
  write_config_int("speed_toggle", speed_toggle=FALSE);
  write_config_int("show_clock", show_clock=TRUE);
  write_config_int("top_panel_active", top_panel_active=TRUE);
  write_config_int("loop_dir", loop_dir=LOOP_NONE);
  write_config_int("double_refresh", double_refresh=FALSE);
  write_config_int("viewed_pages", viewed_pages=0);
  write_config_int("preload_enable", preload_enable=TRUE);
  write_config_int("caching_enable", caching_enable=TRUE);
  write_config_int("suppress_panel", suppress_panel=FALSE);
  write_config_int("show_hidden_files", show_hidden_files=FALSE);
  write_config_int("LED_notify", LED_notify=TRUE);
  write_config_int("backlight", backlight=FALSE);
  write_config_int("sleep_timeout", sleep_timeout=180);
  write_config_int("HD_scaling", HD_scaling=FALSE);
  write_config_int("boost_contrast", boost_contrast=FALSE);
  write_config_int("refresh_type", refresh_type=detect_refresh_type());

  write_config_string("top_panel.path", top_panel.path=xgetcwd(NULL));
  write_config_string("top_panel.selected_name", top_panel.selected_name=strdup("../"));
  write_config_string("top_panel.archive_cwd", top_panel.archive_cwd=strdup(""));
  write_config_string("top_panel.last_name", top_panel.last_name=strdup(""));
  strcpy(top_panel.archive_stack[0],"filesystem");
  strcpy(top_panel.archive_stack[1],"");
  write_archive_stack("top_panel.archive_stack", &top_panel);

  write_config_string("bottom_panel.path", bottom_panel.path=xgetcwd(NULL));
  write_config_string("bottom_panel.selected_name", bottom_panel.selected_name=strdup("../"));
  write_config_string("bottom_panel.archive_cwd", bottom_panel.archive_cwd=strdup(""));
  write_config_string("bottom_panel.last_name", bottom_panel.last_name=strdup(""));
  strcpy(bottom_panel.archive_stack[0],"filesystem");
  strcpy(bottom_panel.archive_stack[1],"");
  write_archive_stack("bottom_panel.archive_stack", &bottom_panel);
  xfree(&current_dir);
}

void read_panel_configuration(struct_panel *panel)
{
  const char *name_prefix;
  char *path_file, *selected_name_file, *archive_cwd_file, *last_name_file, *archive_stack_file;
  if (panel == &top_panel)
    name_prefix = "top";
  else
    name_prefix = "bottom";
  TRACE("Reading %s panel configuration\n", name_prefix);

  path_file=xconcat(name_prefix, "_panel.path");
  selected_name_file=xconcat(name_prefix, "_panel.selected_name");
  archive_cwd_file=xconcat(name_prefix, "_panel.archive_cwd");
  last_name_file=xconcat(name_prefix, "_panel.last_name");
  archive_stack_file=xconcat(name_prefix, "_panel.archive_stack");
  read_config_string(path_file, &panel->path);
  read_config_string(selected_name_file, &panel->selected_name);
  read_config_string(archive_cwd_file, &panel->archive_cwd);
  read_config_string(last_name_file, &panel->last_name);
  read_archive_stack(archive_stack_file, panel);
  free (path_file);
  free (selected_name_file);
  free (archive_cwd_file);
  free (last_name_file);
  free (archive_stack_file);
}

// cppcheck-suppress "unusedFunction"
void read_configuration (void)
{
  char *current_dir=xgetcwd (cfg_directory);
  cfg_directory = xconcat_path_file(current_dir, ".eView");

  crop=read_config_int("crop");
  split_spreads=read_config_int("split_spreads");
  rotate=read_config_int("rotate");
  frame=read_config_int("frame");
  keepaspect=read_config_int("keepaspect");
  manga=read_config_int("manga");
  web_manga_mode=read_config_int("web_manga_mode");
  overlap=read_config_int("overlap");
  fm_toggle=read_config_int("fm_toggle");
  move_toggle=read_config_int("move_toggle");
  speed_toggle=read_config_int("speed_toggle");
  show_clock=read_config_int("show_clock");
  top_panel_active=read_config_int("top_panel_active");
  loop_dir=read_config_int("loop_dir");
  double_refresh=read_config_int("double_refresh");
  viewed_pages=read_config_int("viewed_pages");
  preload_enable=read_config_int("preload_enable");
  caching_enable=read_config_int("caching_enable");
  suppress_panel=read_config_int("suppress_panel");
  show_hidden_files=read_config_int("show_hidden_files");
  LED_notify=read_config_int("LED_notify");
  backlight=read_config_int("backlight");
  sleep_timeout=read_config_int("sleep_timeout");
  HD_scaling=read_config_int("HD_scaling");
  boost_contrast=read_config_int("boost_contrast");
  refresh_type=read_config_int("refresh_type");
  if (refresh_type == REFRESH_UNKNOWN) // Для корректного обновления с прошлых версий
    write_config_int("refresh_type", refresh_type=detect_refresh_type());

  read_panel_configuration(&top_panel);
  read_panel_configuration(&bottom_panel);

  free (current_dir);
}

void reset_config(void)
{
  char *command=NULL;
  TRACE("Removing '%s'\n", cfg_directory);
  asprintf(&command, "rm -rf \"%s\"", cfg_directory);
  xsystem(command);
  xfree(&command);
}
