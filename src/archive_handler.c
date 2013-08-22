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
  size_t len;       /* Signature length          */
  int type;               /* One of ZIP_FILE, RAR_FILE */
  const char *sign;          /* Signature to compare to   */
} magic_sign;

char *escape(const char *input) /* Экранирование нежелательных символов для грепа (прежде всего квадратных скобок) */
{
  int i=0;
  size_t idx;
  char  *escaped;
  if (input==NULL)
  {
    #ifdef debug_printf
    printf("escape() called with NULL string!\n");
    #endif
    return (strdup(""));
  }
  escaped = (char *)malloc(2*strlen(input) + 1); /* Аллоцируем память */
  if (escaped==NULL)
  {
    #ifdef debug_printf
    if (errno==ENOMEM)
      printf("Failed to allocate memory in escape() - no memory!\n");
    else
      printf("Failed to allocate memory in escape() - something BAD happened!\n");
    #endif
    shutdown(FALSE);
  }
  else
  {
  for (idx = 0; idx < strlen(input);  idx++) {
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
  #ifdef debug_printf
  printf("ESCAPED = '%s'\n", escaped);
  #endif
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

int get_archive_list(const char *archive, const char *list_file) /* Создание списка файлов в архиве */
{
  char *command = NULL;
  int asprintf_result;
  switch (file_type_of(archive)) /* Архивно-зависимая часть */
  { 
    case ZIP_FILE:
      #ifdef debug_printf
      printf("File type ZIP: '%s'\n", archive);
      #endif
      asprintf_result=asprintf(&command, "zipinfo -1 \"%s\" | %s > /tmp/ziplist; xargs -n1 dirname < /tmp/ziplist | uniq | sed '/^.$/d;s $ / g' > /tmp/list ; grep -v /$ /tmp/ziplist >> /tmp/list ; %s < /tmp/list > %s", archive, SORT_COMMAND, SORT_COMMAND, list_file); /* Злоебучая команда, потому как бывают архивы, которые не содержат каталогов, только файлы (каталоги в каноничном списке необходимы!) */
      if (asprintf_result == -1 || command == NULL)
      {
        #ifdef debug_printf
        printf("asprintf() failed in get_archive_list (no memory?)\n");
        #endif
        shutdown(FALSE);
      }
      else
        xsystem(command); /* Получаем список каталогов и файлов в каноничном формате (каталоги должны завершаться слэшем) */          
      xfree(&command);      
      (void)remove("/tmp/ziplist");
      (void)remove("/tmp/list");      
      return TRUE;

    case RAR_FILE:
      #ifdef debug_printf
      printf("File type RAR: '%s'\n", archive);
      #endif
      asprintf_result=asprintf(&command, "unrar vt \"%s\" > /tmp/rarlist ; grep -B1 -- 'd[r-][w-][x-][r-][w-][x-][r-][w-][x-]\\|\\.D\\.' /tmp/rarlist | grep -v '^--$\\|d[r-][w-][x-][r-][w-][x-][r-][w-][x-]\\|\\.D\\.'  | cut -c 2- | sed 's $ / g' > /tmp/list ; grep -B1 -- '-[r-][w-][x-][r-][w-][x-][r-][w-][x-]\\|\\.*A\\.' /tmp/rarlist | grep -v '^--$\\|-[r-][w-][x-][r-][w-][x-][r-][w-][x-]\\|\\.*A\\.'  | sed '1d;$d'| cut -c 2- >> /tmp/list; %s < /tmp/list > %s", archive, SORT_COMMAND, list_file); /* Злоебучая команда, ага. Но зато экономим аж три дорогущих вызова system() */
      if (asprintf_result == -1 || command == NULL)
      {
        #ifdef debug_printf
        printf("asprintf() failed in get_archive_list (no memory?)\n");
        #endif
        shutdown(FALSE);
      }
      else /* Получаем список каталогов и файлов в каноничном формате (каталоги должны завершаться слэшем) */
        xsystem(command); 
      xfree(&command);      
      (void)remove("/tmp/rarlist");
      (void)remove("/tmp/list");
      return TRUE;

    default:
      #ifdef debug_printf
      printf("Unknown file type:%s\n", archive);
      #endif
      return FALSE;
  }
}    

char **archive_get_files_list(struct_panel *panel, const char *cwd) /* Получение списка файлов в подкаталоге архива */
{
  char *bff = NULL,*command = NULL, **names, *escaped;
  escaped=escape(cwd);
  if ((asprintf(&command, "grep '^%s[^/]\\+$' %s > /tmp/files.list", escaped, panel->archive_list) == -1) || command == NULL)
  {
    #ifdef debug_printf
    printf("asprintf() failed in archive_get_files_list (no memory?)\n");
    #endif
    shutdown(FALSE);
  }
  else /* Получаем список каталогов и файлов в каноничном формате (каталоги должны завершаться слэшем) */
    xsystem(command); /* Вызываем команду */
  free(escaped);
  xfree(&command);
  if (g_file_test("/tmp/files.list", G_FILE_TEST_EXISTS) == FALSE) /* Если файл со списком не существует */
  {
    #ifdef debug_printf
    printf("Cannot open /tmp/files.list\n");
    #endif
    xfree(&bff); /* Освобождаем временный буфер для содержимого файла */
    return NULL;
  }
  (void)g_file_get_contents("/tmp/files.list", &bff, NULL, NULL); /* Считываем весь файл во временный буфер */
  (void)remove ("/tmp/files.list"); /* И удаляем его */
  names = g_strsplit(bff, "\n", 0); /* Разделяем считанный файл по строкам */
  g_free(bff); /* Освобождаем временный буфер */
  return names;
}

char **archive_get_directories_list(struct_panel *panel, const char *directory) /* Получение списка подкаталогов нижнего уровня в подкаталоге архива */
{
  char *bff = NULL,*command = NULL, **names, *escaped;
  escaped=escape(directory);
  asprintf(&command, "grep '^%s[^/]*/$' %s | uniq > /tmp/dirs.list ", escaped, panel->archive_list);
  free(escaped);
  xsystem(command); /* Вызываем команду */
  xfree(&command);
  if (g_file_test("/tmp/dirs.list", G_FILE_TEST_EXISTS) != TRUE) /* Если файл со списком не существует */
  {
    #ifdef debug_printf
    printf("Cannot open /tmp/dirs.list\n");
    #endif
    g_free(bff); /* Освобождаем временный буфер для содержимого файла */
    return NULL;
  }
  (void)g_file_get_contents("/tmp/dirs.list", &bff, NULL, NULL); /* Считываем весь файл во временный буфер */
  (void)remove ("/tmp/dirs.list"); /* И удаляем его */
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
  free(name);
}

void enter_archive(const char *name, struct_panel *panel, int update_config)
{
  char *saved_work_dir=xgetcwd(NULL);
  #ifdef debug_printf
  printf("Entering into '%s'\n", name);
  #endif
  (void)chdir(panel->path); /* Переходим в каталог где лежит архив */
  if (get_archive_list(name, panel->archive_list))
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
    move_selection("1", panel); /* Переходим на первый же файл в списке, чтобы не прокручивать */
    text=xconcat_path_file(panel->archive_stack[panel->archive_depth],panel->archive_cwd);
    gtk_label_set_text (GTK_LABEL(panel->path_label), text); /* Пишем имя архива с путём в поле снизу */
    free(text);
  }
  free(saved_work_dir);
}

void enter_subarchive(const char *name, struct_panel *panel) /* Вход во вложенный архив - принимает полный путь к архиву */
{
  char *subarchive=NULL;
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

void leave_archive(struct_panel *panel)
{
  char *iter;
  #ifdef debug_printf
  printf("Leaving archive '%s' to dir '%s'\n",panel->archive_stack[panel->archive_depth], panel->path);
  #endif
  
  panel->archive_depth=panel->archive_depth-1;
  if (panel->archive_depth > 0) /* Если мы ешё не достигли ФС */
  {
    (void)remove(panel->archive_list);
    (void)remove(panel->archive_stack[panel->archive_depth+1]); /* То удаляем архив который покинули - он был вложеным! */
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
  iter=iter_from_filename (panel->archive_stack[panel->archive_depth+1], panel);
  move_selection(iter, panel); /* И выбираем файл архива курсором FIXME: Сработает только если покинутый вложенный архив в корне родительского архива, или же при покидании архива в реальную ФС. */
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

int find_prev_archive_directory(struct_panel *panel)
{
  char **directories_list;
  int i=0, n=0;
  char *slash=NULL, *up_dir;
  up_dir=strdup(panel->archive_cwd);
  trim_line(up_dir); /* Удяляем последний символ (слэш) из текущего имени */
  slash=strrchr(up_dir, '/');
  if (slash==NULL) /* Если значение пути вырождается в NULL (слэша больше не оказалось) */
    up_dir[0]='\0'; /* То делаем archive_cwd нулевой строкой */
  else
    *(slash+1)='\0'; /* А иначе просто обрезаем путь в архиве на один уровень */
  directories_list=archive_get_directories_list(panel, up_dir);
  n=(int)sizeof(directories_list);
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

        /*Очищаем оставшийся список каталогов*/
        do free(directories_list[i++]);  
        while (directories_list[i] != NULL);
        free(directories_list);

        return FALSE;  /* То возвращаем что переход не удался */
      }
      else
      {
        free(panel->archive_cwd);
        panel->archive_cwd=strdup(directories_list[i-1]);
        if ( panel == &top_panel )
          write_config_string("top_panel.archive_cwd", panel->archive_cwd);
        else
          write_config_string("bottom_panel.archive_cwd", panel->archive_cwd);
        
        /*Очищаем оставшийся список каталогов*/
        do free(directories_list[i++]);  
        while (directories_list[i] != NULL);
        free(directories_list);
        
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

  do free(directories_list[i++]);  
  while (directories_list[i] != NULL);
  free(directories_list);
  return FALSE; /* И возвращаем что переход не удался */
}

int find_next_archive_directory(struct_panel *panel)
{
  char **directories_list;
  int i=0;
  char *slash=NULL, *up_dir;
  up_dir=strdup(panel->archive_cwd);
  trim_line(up_dir); /* Удяляем последний символ (слэш) из текущего имени */
  slash=strrchr(up_dir, '/'); /* Ищем последний слэш в пути */
  if (slash==NULL) /* Если значение пути вырождается в NULL (слэша больше не оказалось) */
    up_dir[0]='\0'; /* То делаем archive_cwd нулевой строкой */
  else
    *(slash+1)='\0'; /* А иначе просто обрезаем путь в архиве на один уровень */
  directories_list=archive_get_directories_list(panel, up_dir);
  while (TRUE)
  {
    if (directories_list[i]==NULL) 
    {
      free(directories_list);
      return FALSE; /* Если достигли конца списка */
    }
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
        free(directories_list[i]);
        free(directories_list[i+1]);
        free(directories_list);
        return FALSE;  /* То возвращаем что переход не удался */
      }
      else
      {
        free(panel->archive_cwd);
        panel->archive_cwd=strdup(directories_list[i+1]);
        if ( panel == &top_panel )
          write_config_string("top_panel.archive_cwd", panel->archive_cwd);
        else        
          write_config_string("bottom_panel.archive_cwd", panel->archive_cwd);
        #ifdef debug_printf
        printf("JUMPING TO %s\n", panel->archive_cwd);
        #endif
        
        /*Очищаем оставшийся список каталогов*/
        do free(directories_list[i++]);  
        while (directories_list[i] != NULL);
        free(directories_list);
        
        return TRUE;  /* Иначе возвращаем успех */
      }
    }
    #ifdef debug_printf
    printf ("NOT matched dirname '%s', need '%s'\n", directories_list[i], panel->archive_cwd);
    #endif
    free(directories_list[i]);
    i++;
  }
  #ifdef debug_printf
  printf ("Dirname not matched!\n");
  #endif

  /*Очищаем оставшийся список каталогов*/
  do free(directories_list[i++]);  
  while (directories_list[i] != NULL);
  free(directories_list);

  return FALSE; /* И возвращаем значение текущего каталога */
}

void archive_go_upper(struct_panel *panel) /* Переходим на уровень выше внутри архива */
{
  if (panel->archive_cwd[0] == '\0') /* Если на верхнем уровне архива */
    leave_archive(panel); /* То покидаем его */
  else /* А если нет - */
  {
    char *slash=NULL, *path, *iter;
    trim_line(panel->archive_cwd); /* Удяляем последний символ (слэш) из текущего имени */
    archive_cwd_prev=xconcat(basename(panel->archive_cwd),"/");
    slash=strrchr(panel->archive_cwd, '/'); /* Ищем последний слэш в пути */
    if (slash==NULL) /* Если значение пути вырождается в NULL (слэша больше не оказалось) */
      panel->archive_cwd[0]='\0'; /* То делаем archive_cwd нулевой строкой */
    else
      *(slash+1)='\0'; /* А иначе просто обрезаем путь в архиве на один уровень */
    
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
  #ifdef debug_printf
  printf("archive_enter_subdir '%s'\n", subdir);
  #endif
  temp=panel->archive_cwd;
  panel->archive_cwd=xconcat(temp, subdir);
  free(temp);
  if ( panel == &top_panel )
    write_config_string("top_panel.archive_cwd", panel->archive_cwd);
  else
    write_config_string("bottom_panel.archive_cwd", panel->archive_cwd);
  update(panel); /* Перерисовываем список */
  move_selection("1", panel); /* Выбираем сразу первый элемент, чтобы не скроллить */
  path=xconcat_path_file(panel->archive_stack[panel->archive_depth], panel->archive_cwd);
  gtk_label_set_text (GTK_LABEL(panel->path_label), path); /* Пишем имя архива с путём в поле снизу */
  free(path);
}
