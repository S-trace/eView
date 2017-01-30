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

/*
 * Get sorted archive content list to list_file
 * Input:
 * archive: Path to archive file (absolute or relative)
 * list_file: Path to list (absolute or relative)
 */
static int get_archive_list(const char *archive, const char *list_file)
{
	int res;
	size_t bytes_in_list = 0, records_in_list = 0;
	char **listarr = NULL;
	FILE *list;
	size_t buffsize=16384;
	struct archive_entry *entry;
	size_t i=0;
	TRACE("%s caled with %s %s\n", __func__, archive, list_file);
	
	struct archive *a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	
	res = archive_read_open_filename(a, archive, buffsize);
	if (res != ARCHIVE_OK) {
		TRACE("Unable to open archive '%s'\n", archive);
		goto out_archive_read;
	}
	
	list=fopen(list_file, "w");
	if (!list) {
		TRACE("can't open target file '%s' (%s)\n", list_file, strerror(errno));
		goto out_archive_read;
	}
	do {
		res = archive_read_next_header(a, &entry);
		if (res == ARCHIVE_EOF )
			break;
		if (res != ARCHIVE_OK)
			goto out_close_list;
		bytes_in_list += sizeof(*listarr);
		listarr = realloc(listarr, bytes_in_list);
		const char *pathname = archive_entry_pathname(entry);
		if (archive_entry_filetype(entry) == AE_IFDIR && pathname[strlen(pathname)-1] != '/'){
			listarr[records_in_list] = malloc(sizeof(**listarr)*strlen(pathname)+1);
			strcpy(listarr[records_in_list], pathname);
			strcat(listarr[records_in_list], "/");
		} else {
			listarr[records_in_list] = strdup(pathname);
		}
		++records_in_list;
	} while (TRUE);

	qsort(listarr, bytes_in_list/sizeof(*listarr), sizeof(*listarr), (__compar_fn_t)strverscmp_qsort_wrapper);

	for(i=0; i<records_in_list; ++i) {
		fputs(listarr[i], list);
		fputc('\n', list);
		free(listarr[i]);
	}
	fclose(list);
	archive_read_free(a);
	return TRUE;
	
	out_close_list:
	fclose(list);
	out_archive_read:
	archive_read_free(a);
	return FALSE;
}

// cppcheck-suppress "unusedFunction"
char **archive_get_files_list(struct_panel *panel, const char *cwd) /* Получение списка файлов в подкаталоге архива */
{
  char *bff = NULL,*command = NULL, **names, *escaped;
  escaped=escape(cwd);
  if ((asprintf(&command, "grep '^%s[^/]\\+$' %s > /tmp/files.list", escaped, panel->archive_list) == -1) || command == NULL)
  {
    TRACE("asprintf() failed in archive_get_files_list (no memory?)\n");
    shutdown(FALSE);
  }
  else /* Получаем список каталогов и файлов в каноничном формате (каталоги должны завершаться слэшем) */
    xsystem(command);
  free(escaped);
  xfree(&command);
  if (g_file_test("/tmp/files.list", G_FILE_TEST_EXISTS) == FALSE) /* Если файл со списком не существует */
  {
    TRACE("Cannot open /tmp/files.list\n");
    xfree(&bff); /* Освобождаем временный буфер для содержимого файла */
    return NULL;
  }
  (void)g_file_get_contents("/tmp/files.list", &bff, NULL, NULL); /* Считываем весь файл во временный буфер */
  (void)remove ("/tmp/files.list"); /* И удаляем его */
  names = g_strsplit(bff, "\n", 0); /* Разделяем считанный файл по строкам */
  g_free(bff); /* Освобождаем временный буфер */
  return names;
}

// cppcheck-suppress "unusedFunction"
char **archive_get_directories_list(struct_panel *panel, const char *directory) /* Получение списка подкаталогов нижнего уровня в подкаталоге архива */
{
  char *bff = NULL,*command = NULL, **names, *escaped;
  escaped=escape(directory);
  asprintf(&command, "grep '^%s[^/]*/$' %s | uniq > /tmp/dirs.list ", escaped, panel->archive_list);
  free(escaped);
  xsystem(command);
  xfree(&command);
  if (g_file_test("/tmp/dirs.list", G_FILE_TEST_EXISTS) != TRUE) /* Если файл со списком не существует */
  {
    TRACE("Cannot open /tmp/dirs.list\n");
    g_free(bff); /* Освобождаем временный буфер для содержимого файла */
    return NULL;
  }
  (void)g_file_get_contents("/tmp/dirs.list", &bff, NULL, NULL); /* Считываем весь файл во временный буфер */
  (void)remove ("/tmp/dirs.list"); /* И удаляем его */
  names = g_strsplit(bff, "\n", 0); /* Разделяем считанный файл по строкам */
  g_free(bff); /* Освобождаем временный буфер */
  return names;
}

/*
 * Extract file to target_directory from archive
 * Input:
 * archive: Path to archive file (absolute or relative)
 * file: Full path to file in archive (without leading /)
 * target_directory: Path where file will be extracted with archived path
 */
void archive_extract_file(const char* archive, const char* file, const char* target_directory)
{
	int res;
	int target_fd;
	ssize_t size;
	size_t buffsize=16384;
	char buff[buffsize], *target_full_path=NULL, *target_dir=NULL, *td_bak=NULL;
	struct archive_entry *entry;
	struct archive *a;
	TRACE("%s caled with %s %s %s\n", __func__, archive, file, target_directory);

	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);

	res = archive_read_open_filename(a, archive, buffsize);
	if (res != ARCHIVE_OK) {
		TRACE("Unable to open archive '%s'\n", archive);
		goto out_archive_read;
	}

	do {
		if (archive_read_next_header(a, &entry) != ARCHIVE_OK) {
			TRACE("Archive parsing failed or no such file in archive, exiting\n");
			goto out_archive_read;
		}
	} while (strncmp(archive_entry_pathname(entry), file, strlen(file)) != 0);

	target_full_path=xconcat_path_file(target_directory, file);
	target_dir=xconcat_path_file(target_directory, file);
	td_bak=target_dir;
	target_dir=dirname(target_dir);
	if (!mkpath(target_dir, S_IRWXU))
		TRACE("mkdir '%s' failed (%s)\n", target_dir, strerror(errno));
	else
		TRACE("mkdir '%s' done!\n", target_dir);
	target_fd=open(target_full_path, O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR);
	if (target_fd < 0 ) {
		TRACE("can't open target file '%s' (%s)\n", target_full_path, strerror(errno));
		goto out_strings;
	}

	for (;;) {
		size = archive_read_data(a, buff, buffsize);
		if (size < 0) {
			TRACE("Something was wrong while extracting file!\n");
			goto out_strings;
		}
		if (size == 0) {
			TRACE("File extracted completely\n");
			break;
		}
		write(target_fd, buff, (size_t)size);
	}
	close(target_fd);

	out_strings:
	free (target_full_path);
	free (td_bak);
	out_archive_read:
	archive_read_free(a);
	return;
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
    (void)remove(panel->archive_list);
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
