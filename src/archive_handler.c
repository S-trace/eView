// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is an open source non-commercial project. Dear PVS-Studio, please check it.
/* Soul Trace, 2013, Distributed under GPLv2 Terms */
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <archive.h>
#include <archive_entry.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>

#include "gtk_file_manager.h"
#include "mylib.h"
#include "cfg.h"
#include "mygtk.h"
#include "os-specific.h"
#include "translations.h"
#include "archive_handler.h"
#include "archive_routines.h"
char *archive_cwd_prev; /* Предыдущий текущий каталог в архиве */

enum
{
  RAR_FILE,
  ZIP_FILE,
  ARCH_TYPES
};

typedef struct {
  int     offs;           /* Offset to the signature   */
  size_t len;             /* Signature length          */
  int type;               /* One of ZIP_FILE, RAR_FILE */
  const char *sign;       /* Signature to compare to   */
} magic_sign;

char *detect_subarchive_prefix(void)
{
  if (access("/media/data/", R_OK))
  {
    TRACE("Unable to access /media/data/ - using / as subarchive extracting prefix\n");
    return (strdup("/"));
  }
  else
  {
    TRACE("Using /media/data/ as subarchive extracting prefix\n");
    return (strdup("/media/data/"));
  }
}

char *escape(const char *input) /* Экранирование нежелательных символов для грепа (прежде всего квадратных скобок) */
{
  char  *escaped;
  if (input==NULL)
  {
    TRACE("escape() called with NULL string!\n");
    return (strdup(""));
  }
  escaped = (char *)malloc(2*strlen(input) + 1); /* Аллоцируем память */
  if (escaped==NULL)
  {
    #ifdef debug
    if (errno==ENOMEM)
      TRACE("Failed to allocate memory in escape() - no memory!\n");
    else
      TRACE("Failed to allocate memory in escape() - something BAD happened!\n");
    #endif
    shutdown(FALSE);
  }
  else
  {
    size_t idx;
    int i=0;
    for (idx = 0; idx < strlen(input); idx++) {
      switch (input[idx])
      { /* Для нежелательных символов */
        case '[': case ']':
        case '{': case '}':
        case '"':
          escaped[i++] = '\\'; /* Вставляем перед ними backslash */
          break;
        default:
          break;
      };
      escaped[i++] = input[idx]; /* И копируем собственно символ */
    }
    escaped[i] = '\0'; /* Терминируем строку */
    TRACE("ESCAPED = '%s'\n", escaped);
    return escaped;
  }
  return strdup("");
}

magic_sign magic[ARCH_TYPES] = {
  { 0, 4, ZIP_FILE, "PK\003\004"}, /* Classic ZIP files */
  { 0, 4, RAR_FILE, "Rar!"      }, /* RAR archives      */
};

int file_type_of(const char *fname)
{
  FILE *f;
  char sign[20];
  int nr;

  f = fopen(fname, "rb");
  if (!f) return -1;

  for (nr = 0; nr < ARCH_TYPES; nr++) {
    if ((fseek(f, magic[nr].offs, SEEK_SET) == -1) || fread(sign, 1, magic[nr].len, f) != magic[nr].len)
      break;

    /* If the read went well, we need to compare the characters */
    /* strstr works here, but only if no \0's are in the string */
    /* and if we first terminate the string read too */
    sign[magic[nr].len] = '\0';
    if (strcmp(sign, magic[nr].sign) == 0)
    {
      (void)fclose(f);
      return magic[nr].type;
    }
  }
  (void)fclose(f);
  Message(ERROR, UNKNOWN_OR_DAMAGED_ARCHIVE);
  return -1;
}

int enter_archive(const char *name, struct_panel *panel, int update_config)
{
  char *saved_work_dir=xgetcwd(NULL);
  TRACE("Entering into '%s'\n", name);
  if (chdir(panel->path) == -1) /* Переходим в каталог где лежит архив */
  {
    char *message;
    asprintf(&message, UNABLE_TO_CHANGE_DIRECTORY_TO, panel->path, strerror(errno));
    Message(ERROR, message);
    free (message);
    return FALSE;
  }

  if (access(name, R_OK) == -1 )
  {
    char *message;
    asprintf(&message, UNABLE_TO_ACCESS_FILE, name, strerror(errno));
    Message(ERROR, message);
    free (message);
    return FALSE;
  }
  
  if (archive_supported(name))
  {
    char *text;
    if(update_config)
    {
      panel->archive_depth++;
      strcpy(panel->archive_stack[panel->archive_depth], name);
      if ( panel == &top_panel )
        write_archive_stack("top_panel.archive_stack", &top_panel);
      else
        write_archive_stack("bottom_panel.archive_stack", &bottom_panel);
      (void)chdir(saved_work_dir); /* Переходим в каталог откуда нас дёрнули */
    }
    update(panel); /* Строим список */
    move_selection("0", panel); /* Переходим на первый же файл в списке */
    text=xconcat_path_file(panel->archive_stack[panel->archive_depth],panel->archive_cwd);
    gtk_label_set_text (GTK_LABEL(panel->path_label), text); /* Пишем имя архива с путём в поле снизу */
    free(text);
  }
  free(saved_work_dir);
  return TRUE;
}

void enter_subarchive(const char *name, struct_panel *panel) /* Вход во вложенный архив - принимает полный путь к архиву */
{
  char *subarchive=NULL;
  #ifdef __amd64
  char *prefix=strdup("/tmp/");
  #else
  char *prefix=detect_subarchive_prefix();
  #endif
  archive_extract_file(panel->archive_stack[panel->archive_depth], name, prefix);
  asprintf(&subarchive, "%s%s/%s", prefix, panel->archive_cwd, name);
  TRACE("Entering into subarchive '%s' of archive '%s'\n", subarchive, panel->archive_stack[panel->archive_depth]);
  enter_archive(subarchive, panel, TRUE);
  xfree(&prefix);
}

void leave_archive(struct_panel *panel)
{
  char *iter;
  TRACE("Leaving archive '%s' to dir '%s'\n",panel->archive_stack[panel->archive_depth], panel->path);

  panel->archive_depth=panel->archive_depth-1;
  if (panel->archive_depth > 0) /* Если мы ешё не достигли ФС */
  {
    archive_list_free(panel->archive_list);
    (void)remove(panel->archive_stack[panel->archive_depth+1]); /* То удаляем архив который покинули - он был вложеным! */
    enter_archive(panel->archive_stack[panel->archive_depth], panel, FALSE);
  }
  else
  {
    update(panel); /* Обновляем список файлов */
    gtk_label_set_text (GTK_LABEL(panel->path_label), panel->path); /* Пишем текущий каталог в поле снизу */
  }
  TRACE("move_selection call '%s'\n",panel->archive_stack[panel->archive_depth+1]);
  iter=iter_from_filename (panel->archive_stack[panel->archive_depth+1], panel);
  move_selection(iter, panel); /* И выбираем файл архива курсором FIXME: Сработает только если покинутый вложенный архив в корне родительского архива, или же при покидании архива в реальную ФС. FIXME: этот прискорбный баг породит глюки при автоматическом переходе в следующий каталог! */
  free(iter);
  panel->archive_stack[panel->archive_depth+1][0]='\0'; /*Затираем имя покидаемого архива в стеке */
  if (panel == &top_panel)
  {
    write_archive_stack("top_panel.archive_stack", &top_panel);
    write_config_string("top_panel.archive_cwd", ""); /* FIXME: По идее надо бы завести ещё и стек путей в архивах, но это геморно( */
  }
  else
  {
    write_archive_stack("bottom_panel.archive_stack", &bottom_panel);
    write_config_string("bottom_panel.archive_cwd", "");
  }
}

void archive_go_upper(struct_panel *panel) /* Переходим на уровень выше внутри архива */
{
  if (panel->archive_cwd[0] == '\0') /* Если на верхнем уровне архива - то покидаем его*/
    leave_archive(panel);
  else /* А если нет - */
  {
    char *path, *iter, *slash;
    trim_line(panel->archive_cwd); /* Удяляем последний символ (слэш) из текущего имени */
    archive_cwd_prev=xconcat(basename(panel->archive_cwd),"/");
    slash=strrchr(panel->archive_cwd, '/'); /* Ищем последний слэш в пути */
    if (slash==NULL) /* Если значение пути вырождается в NULL (слэша больше не оказалось) то делаем archive_cwd нулевой строкой*/
      panel->archive_cwd[0]='\0';
    else /* А иначе просто обрезаем путь в архиве на один уровень */
      *(slash+1)='\0';

    if (panel == &top_panel)
    {
      write_config_string("top_panel.archive_cwd", panel->archive_cwd);
      top_panel.last_name[0]='\0';
      write_config_string("top_panel.last_name", top_panel.last_name);
    }
    else
    {
      write_config_string("bottom_panel.archive_cwd", panel->archive_cwd);
      bottom_panel.last_name[0]='\0';
      write_config_string("bottom_panel.last_name", bottom_panel.last_name);
    }
    update(panel); /* Перерисовываем список */
    iter=iter_from_filename (archive_cwd_prev, panel);
    move_selection(iter, panel); /* И выделяем предыдущий каталог в архиве */
    free(iter);
    path=xconcat_path_file(panel->archive_stack[panel->archive_depth],panel->archive_cwd);
    gtk_label_set_text (GTK_LABEL(panel->path_label), path); /* Пишем имя архива с путём в поле снизу */
    free(path);
  }
}

void archive_enter_subdir(const char *subdir, struct_panel *panel)
{
  char *path, *temp;
  TRACE("archive_enter_subdir '%s'\n", subdir);
  temp=panel->archive_cwd;
  panel->archive_cwd=xconcat(temp, subdir);
  free(temp);
  if ( panel == &top_panel )
    write_config_string("top_panel.archive_cwd", panel->archive_cwd);
  else
    write_config_string("bottom_panel.archive_cwd", panel->archive_cwd);
  update(panel); /* Перерисовываем список */
  move_selection("0", panel); /* Выбираем первый элемент */
  path=xconcat_path_file(panel->archive_stack[panel->archive_depth], panel->archive_cwd);
  gtk_label_set_text (GTK_LABEL(panel->path_label), path); /* Пишем имя архива с путём в поле снизу */
  free(path);
}
