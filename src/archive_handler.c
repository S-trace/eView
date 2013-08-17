/* Soul Trace, 2013, Distributed under GPLv2 Terms */
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#include "gtk_file_manager.h"
#include "mylib.h"
#include "cfg.h"
#include "mygtk.h"
#include "os-specific.h"
#include "translations.h"
char *archive_cwd_prev; /* Предыдущий текущий каталог в архиве */

enum
{
  RAR_FILE,
  ZIP_FILE,
  ARCH_TYPES
};

typedef struct {
  int     offs;           /* Offset to the signature   */
  unsigned int len;       /* Signature length          */
  int type;               /* One of ZIP_FILE, RAR_FILE */
  const char *sign;          /* Signature to compare to   */
} magic_sign;

char *escape(const char *input) /* Экранирование нежелательных символов для грепа (прежде всего квадратных скобок) */
{
  int i=0;
  unsigned int idx;
  char  *escaped;
  escaped = (char *)g_malloc(2*strlen(input) + 1); /* Аллоцируем память */
  for (idx = 0; idx < strlen(input);  idx++) {
    switch (input[idx])
    { /* Для нежелательных символов */
      case '[': case ']':
      case '{': case '}':
      case '"':
        escaped[i++] = '\\'; /* Вставляем перед ними backslash */
    };
    escaped[i++] = input[idx]; /* И копируем собственно символ */
  }
  escaped[i] = '\0'; /* Терминируем строку */
  #ifdef debug_printf
  printf("ESCAPED = '%s'\n", escaped);
  #endif
  return escaped;
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
    if (fseek(f, magic[nr].offs, SEEK_SET) || fread(sign, 1, magic[nr].len, f) != magic[nr].len)
      break;
    
    /* If the read went well, we need to compare the characters */
    /* strstr works here, but only if no \0's are in the string */
    /* and if we first terminate the string read too */
    sign[magic[nr].len] = '\0';
    if (strcmp(sign, magic[nr].sign) == 0)
    {
      fclose(f);
      return magic[nr].type;
    }
  }
  fclose(f);
  Message(ERROR, UNKNOWN_OR_DAMAGED_ARCHIVE);
  return -1;
}

int get_archive_list(const char *archive, const char *list_file) /* Создание списка файлов в архиве */
{
  char *command = NULL;
  switch (file_type_of(archive)) /* Архивно-зависимая часть */
  { 
    case ZIP_FILE:
      #ifdef debug_printf
      printf("File type ZIP: '%s'\n", archive);
      #endif
      asprintf(&command, "zipinfo -1 \"%s\" | %s > /tmp/ziplist; xargs -n1 dirname < /tmp/ziplist | uniq | sed '/^.$/d;s $ / g' > /tmp/list ; grep -v /$ /tmp/ziplist >> /tmp/list ; %s < /tmp/list > %s", archive, SORT_COMMAND, SORT_COMMAND, list_file); /* Злоебучая команда, потому как бывают архивы, которые не содержат каталогов, только файлы (каталоги в каноничном списке необходимы!) */
      xsystem(command); /* Получаем список каталогов и файлов в каноничном формате (каталоги должны завершаться слэшем) */
      xfree(&command);      
      remove("/tmp/ziplist");
      remove("/tmp/list");      
      return TRUE;
      break;
    case RAR_FILE:
      #ifdef debug_printf
      printf("File type RAR: '%s'\n", archive);
      #endif
      asprintf(&command, "unrar vt \"%s\" > /tmp/rarlist ; grep -B1 -- 'd[r-][w-][x-][r-][w-][x-][r-][w-][x-]\\|\\.D\\.' /tmp/rarlist | grep -v '^--$\\|d[r-][w-][x-][r-][w-][x-][r-][w-][x-]\\|\\.D\\.'  | cut -c 2- | sed 's $ / g' > /tmp/list ; grep -B1 -- '-[r-][w-][x-][r-][w-][x-][r-][w-][x-]\\|\\.*A\\.' /tmp/rarlist | grep -v '^--$\\|-[r-][w-][x-][r-][w-][x-][r-][w-][x-]\\|\\.*A\\.'  | sed '1d;$d'| cut -c 2- >> /tmp/list; %s < /tmp/list > %s", archive, SORT_COMMAND, list_file); /* Злоебучая команда, ага. Но зато экономим аж три дорогущих вызова system() */
      xsystem(command); /* Получаем список каталогов и файлов в каноничном формате (каталоги должны завершаться слэшем) */
      xfree(&command);
      remove("/tmp/rarlist");
      remove("/tmp/list");
      return TRUE;
      break;
    default:
      #ifdef debug_printf
      printf("Unknown file type:%s\n", archive);
      #endif
      return FALSE;
  }
}    

char **archive_get_files_list(panel *panel, const char *archive_cwd) /* Получение списка файлов в подкаталоге архива */
{
  char *bff = NULL,*command = NULL, **names, *escaped;
  escaped=escape(archive_cwd);
  asprintf(&command, "grep '^%s[^/]\\+$' %s > /tmp/files.list", escaped, panel->archive_list);
  xsystem(command); /* Вызываем команду */
  xfree(&escaped);
  xfree(&command);
  if (!g_file_test("/tmp/files.list", G_FILE_TEST_EXISTS)) /* Если файл со списком не существует */
  {
    #ifdef debug_printf
    printf("Cannot open /tmp/files.list\n");
    #endif
    g_free(bff); /* Освобождаем временный буфер для содержимого файла */
    return NULL;
  }
  g_file_get_contents("/tmp/files.list", &bff, NULL, NULL); /* Считываем весь файл во временный буфер */
  remove ("/tmp/files.list"); /* И удаляем его */
  names = g_strsplit(bff, "\n", 0); /* Разделяем считанный файл по строкам */
  g_free(bff); /* Освобождаем временный буфер */
  return names;
}

char **archive_get_directories_list(panel *panel, const char *directory) /* Получение списка подкаталогов нижнего уровня в подкаталоге архива */
{
  char *bff = NULL,*command = NULL, **names, *escaped;
  escaped=escape(directory);
  asprintf(&command, "grep '^%s[^/]*/$' %s > /tmp/dirs.list", escaped, panel->archive_list);
  xfree(&escaped);
  xsystem(command); /* Вызываем команду */
  xfree(&command);
  if (!g_file_test("/tmp/dirs.list", G_FILE_TEST_EXISTS)) /* Если файл со списком не существует */
  {
    #ifdef debug_printf
    printf("Cannot open /tmp/dirs.list\n");
    #endif
    g_free(bff); /* Освобождаем временный буфер для содержимого файла */
    return NULL;
  }
  g_file_get_contents("/tmp/dirs.list", &bff, NULL, NULL); /* Считываем весь файл во временный буфер */
  remove ("/tmp/dirs.list"); /* И удаляем его */
  names = g_strsplit(bff, "\n", 0); /* Разделяем считанный файл по строкам */
  g_free(bff); /* Освобождаем временный буфер */
  return names;
}

void archive_extract_file(const char *archive, const char *file, const char *to)
{
  const char *archiver;
  char *command = NULL, *name=NULL;
  switch (file_type_of(archive)) {
    case ZIP_FILE:
      archiver="unzip -o ";
      name=escape(file);
      break;
    case RAR_FILE:
      archiver="unrar x -y -kb ";
      name=strdup(file);
      break;
    default:
      #ifdef debug_printf
      printf("Unknown file type:%s\n", archive);
      #endif
      return;
  }
  asprintf(&command, "%s \"%s\" \"%s\" -d \"%s\"", archiver, archive, name, to);
  xsystem(command);
  xfree(&command);
  xfree(&name);
}

void enter_archive(const char *name, panel *panel, int update_config)
{
  char *saved_work_dir=xgetcwd(NULL);
  #ifdef debug_printf
  printf("Entering into '%s'\n", name);
  #endif
  chdir(panel->path); /* Переходим в каталог где лежит архив */
  if (get_archive_list(name, panel->archive_list))
  {
    if(update_config)
    {
      panel->archive_depth++;
      strcpy(panel->archive_stack[panel->archive_depth], name);
      if ( panel == &top_panel )
        write_archive_stack("top_panel.archive_stack", &top_panel);
      else
        write_archive_stack("bottom_panel.archive_stack", &bottom_panel);
      chdir(saved_work_dir); /* Переходим в каталог откуда нас дёрнули */
      xfree(&saved_work_dir);
    }
    update(panel); /* Строим список */
    move_selection("1", panel); /* Переходим на первый же файл в списке, чтобы не прокручивать */
    gtk_label_set_text (GTK_LABEL(panel->path_label), xconcat_path_file(panel->archive_stack[panel->archive_depth],panel->archive_cwd)); /* Пишем имя архива с путём в поле снизу */
  }
}

void enter_subarchive(const char *name, panel *panel) /* Вход во вложенный архив - принимает полный путь к архиву */
{
  char *subarchive;
  #ifdef __amd64
  const char *prefix="/tmp/";
  #else
  const char *prefix="/";
  #endif
  archive_extract_file(panel->archive_stack[panel->archive_depth], name, prefix);
  asprintf(&subarchive, "%s%s/%s", prefix, panel->archive_cwd, name);  
  #ifdef debug_printf
  printf("Entering into subarchive '%s' of archive '%s'\n", subarchive, panel->archive_stack[panel->archive_depth]);
  #endif
  enter_archive(subarchive, panel, TRUE);
}

void leave_archive(panel *panel)
{
  #ifdef debug_printf
  printf("Leaving archive '%s' to dir '%s'\n",panel->archive_stack[panel->archive_depth], panel->path);
  #endif
  
  panel->archive_depth--;
  if (panel->archive_depth > 0) /* Если мы ешё не достигли ФС */
  {
    remove(panel->archive_list);
    remove(panel->archive_stack[panel->archive_depth+1]); /* То удаляем архив который покинули - он был вложеным! */
    enter_archive(panel->archive_stack[panel->archive_depth], panel, FALSE);
  }
  else
  {
    update(panel); /* Обновляем список файлов */
    gtk_label_set_text (GTK_LABEL(panel->path_label), panel->path); /* Пишем текущий каталог в поле снизу */
  }
  #ifdef debug_printf
  printf("move_selection call '%s'\n",panel->archive_stack[panel->archive_depth+1]);
  #endif
  move_selection(iter_from_filename (panel->archive_stack[panel->archive_depth+1], panel), panel); /* И выбираем файл архива курсором FIXME: Сработает только если покинутый вложенный архив в корне родительского архива, или же при покидании архива в реальную ФС. */
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

int find_prev_archive_directory(panel *panel)
{
  char **directories_list;
  int i=0;
  char *up_dir=strdup(panel->archive_cwd);
  trim_line(up_dir); /* Удяляем последний символ (слэш) из текущего имени */
  char *a=strrchr(up_dir, '/'); /* Ищем последний слэш в пути */
  if (a==NULL) /* Если значение пути вырождается в NULL (слэша больше не оказалось) */
    up_dir='\0'; /* То делаем archive_cwd нулевой строкой */
  else
    *(a+1)='\0'; /* А иначе просто обрезаем путь в архиве на один уровень */
  directories_list=archive_get_directories_list(panel, up_dir);
  int n=(int)sizeof(directories_list);
  #ifdef debug_printf
  printf("sizeof(directories_list)=%d\n",n);
  #endif
  while (i<=n)
  {
    #ifdef debug_printf
    printf ("Checking '%s', need '%s'\n", directories_list[i], panel->archive_cwd);
    #endif
    if (strcmp (directories_list[i], panel->archive_cwd) == 0) /* Сравниваем строку с текущим каталогом */
    {
      if (i == 0)
      {
        #ifdef debug_printf
        printf ("Matched dirname '%s', stay here\n", directories_list[i]);
        #endif
        /*         xfree(&up_dir); */
        return FALSE;  /* То возвращаем что переход не удался */
      }
      else
      {
        panel->archive_cwd=strdup(directories_list[i-1]);
        if ( panel == &top_panel )
          write_config_string("top_panel.archive_cwd", panel->archive_cwd);
        else
          write_config_string("bottom_panel.archive_cwd", panel->archive_cwd);
        /*         xfree(&up_dir); */
        return TRUE;  /* Иначе возвращаем успех */
      }
    }
    #ifdef debug_printf
    printf ("NOT matched dirname '%s', need '%s'\n", directories_list[i], panel->archive_cwd);
    #endif
    i++;
  }
  #ifdef debug_printf
  printf ("Dirname not matched (back)!\n");
  #endif
  return FALSE; /* И возвращаем что переход не удался */
}

int find_next_archive_directory(panel *panel)
{
  char **directories_list;
  int i=0;
  char *up_dir=strdup(panel->archive_cwd);
  trim_line(up_dir); /* Удяляем последний символ (слэш) из текущего имени */
  char *a=strrchr(up_dir, '/'); /* Ищем последний слэш в пути */
  if (a==NULL) /* Если значение пути вырождается в NULL (слэша больше не оказалось) */
    up_dir='\0'; /* То делаем archive_cwd нулевой строкой */
  else
    *(a+1)='\0'; /* А иначе просто обрезаем путь в архиве на один уровень */
  directories_list=archive_get_directories_list(panel, up_dir);
  while (TRUE)
  {
    if (directories_list[i]==NULL) 
      return FALSE; /* Если достигли конца списка */
    else
    {
      #ifdef debug_printf
      printf ("Checking '%s', need '%s'\n", directories_list[i], panel->archive_cwd);
      #endif
    }
    if (strcmp (directories_list[i], panel->archive_cwd) == 0) /* Сравниваем строку с текущим каталогом */
    {
      if (directories_list[i+1][0] == '\0') /* Если следующая строка пустая - */
      {
        #ifdef debug_printf
        printf ("Matched dirname '%s', stay here\n", directories_list[i]);
        #endif
        /*         xfree(&up_dir); */
        return FALSE;  /* То возвращаем что переход не удался */
      }
      else
      {
        panel->archive_cwd=strdup(directories_list[i+1]);
        if ( panel == &top_panel )
          write_config_string("top_panel.archive_cwd", panel->archive_cwd);
        else        
          write_config_string("bottom_panel.archive_cwd", panel->archive_cwd);
        /*         xfree(&up_dir); */
        #ifdef debug_printf
        printf("JUMPING TO %s\n", panel->archive_cwd);
        #endif
        return TRUE;  /* Иначе возвращаем успех */
      }
    }
    #ifdef debug_printf
    printf ("NOT matched dirname '%s', need '%s'\n", directories_list[i], panel->archive_cwd);
    #endif
    i++;
  }
  #ifdef debug_printf
  printf ("Dirname not matched!\n");
  #endif
  return FALSE; /* И возвращаем значение текущего каталога */
}

void archive_go_upper(panel *panel) /* Переходим на уровень выше внутри архива */
{
  if (panel->archive_cwd[0] == '\0') /* Если на верхнем уровне архива */
    leave_archive(panel); /* То покидаем его */
  else /* А если нет - */
  {
    trim_line(panel->archive_cwd); /* Удяляем последний символ (слэш) из текущего имени */
    archive_cwd_prev=xconcat(basename(panel->archive_cwd),"/");
    char *a=strrchr(panel->archive_cwd, '/'); /* Ищем последний слэш в пути */
    if (a==NULL) /* Если значение пути вырождается в NULL (слэша больше не оказалось) */
      panel->archive_cwd='\0'; /* То делаем archive_cwd нулевой строкой */
    else
      *(a+1)='\0'; /* А иначе просто обрезаем путь в архиве на один уровень */
    
    if (panel == &top_panel)
    {
      write_config_string("top_panel.archive_cwd", panel->archive_cwd);
      write_config_string("top_panel.last_name", top_panel.last_name='\0');
    }
    else
    {
      write_config_string("bottom_panel.archive_cwd", panel->archive_cwd);
      write_config_string("bottom_panel.last_name", bottom_panel.last_name='\0');
    }
    update(panel); /* Перерисовываем список */
    move_selection(iter_from_filename (archive_cwd_prev, panel), panel); /* И выделяем предыдущий каталог в архиве */
    gtk_label_set_text (GTK_LABEL(panel->path_label), xconcat_path_file(panel->archive_stack[panel->archive_depth],panel->archive_cwd)); /* Пишем имя архива с путём в поле снизу */
  }
}

void archive_enter_subdir(const char *subdir, panel *panel)
{
  #ifdef debug_printf
  printf("archive_enter_subdir '%s'\n", subdir);
  #endif
  if ( panel == &top_panel )
    write_config_string("top_panel.archive_cwd", panel->archive_cwd=xconcat(panel->archive_cwd, subdir));
  else
    write_config_string("bottom_panel.archive_cwd", panel->archive_cwd=xconcat(panel->archive_cwd, subdir));
  update(panel); /* Перерисовываем список */
  move_selection("1", panel); /* Выбираем сразу первый элемент, чтобы не скроллить */
  gtk_label_set_text (GTK_LABEL(panel->path_label), xconcat_path_file(panel->archive_stack[panel->archive_depth], panel->archive_cwd)); /* Пишем имя архива с путём в поле снизу */
}
