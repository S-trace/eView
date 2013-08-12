/**
 * Copyright (c) 2011 Alexander Drozdoff <hatred@inbox.ru>
 * Update for Qt support by S-trace <s-trace@list.ru>, 2013
 * Ditributed under GPLv2 Terms
 * http://hatred.homelinux.net/~hatred/digma
 */

#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h> // atoi()
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h> // dlopen()
#include "gtk_file_manager.h" // Инклюдить первой среди своих, ибо typedef panel!
#include "digma_hw.h"
#include "mylib.h"

int hardware_has_backlight=FALSE, hardware_has_LED=FALSE, hardware_has_APM=FALSE, hardware_has_sysfs_sleep=FALSE;
char *LED_path=NULL, *backlight_path=NULL, *sysfs_sleep_path=NULL;
int LED_state[LED_STATES]; // Массив содержащий значения которые надо писать в sysfs чтобы управлять LED
int previous_backlight_level; // Уровень подсветки перед запуском eView
int suspended=FALSE; // Текущее состояние книги
int was_in_picture_viewer=FALSE;
extern int framebuffer_descriptor;
extern int QT;
extern int enable_refresh;//Принудительно запретить обновлять экран в особых случаях
extern int LED_notify; // Оповещение светодиодом об обновлении панелей и загрузке

int (*apm_suspend)(int fd); // Функция из libapm.so, загружается через dlopen

/* Helper FB update function */
void epaperUpdate(int ioctl_call, int mode)
{
  if (! enable_refresh)
  {
    #ifdef debug_printf
    printf ("Display refresh was locked, IGNORED (ioctl %d mode %d)!\n", ioctl_call, mode);
    #endif
    return;
  }
  #ifndef __amd64
  if (framebuffer_descriptor >= 0)
  {
    if (QT)
      usleep(245000); // Иначе запись в видеопамять не успевает завершиться и получаем верхний левый угол новой картинки и нижний правый - прежней.
    ioctl(framebuffer_descriptor, ioctl_call, &mode);
  }
  #endif
}

/**
 * Update E-Ink screen fully
 */
void epaperUpdateFull(void)
{
  if (QT)
    epaperUpdate(EPAPER_UPDATE_DISPLAY_QT, 3);
  else
    epaperUpdate(EPAPER_UPDATE_FULL, 3); // Выполняется за 0,847s
}

/**
 * Update only local (???)
 */
void epaperUpdateLocal(void)
{
  if (QT)
    epaperUpdate(EPAPER_UPDATE_DISPLAY_QT, 2);
  else
    epaperUpdate(EPAPER_UPDATE_LOCAL, 2); // Выполняется за 0,847s
}

/**
 * Untested E-Ink function
 */
void epaperUpdatePart(void)
{
  if (QT)
    epaperUpdate(EPAPER_UPDATE_DISPLAY_QT, 1);
  else
    epaperUpdate(EPAPER_UPDATE_PART, 1); // Выполняется за 0,847s
}

int check_for_file (char *fpath)  // Проверка наличия файла в файловой системе
{
  if (access(fpath, F_OK))  // F_OK просто проверяет существование файла
    return FALSE;
  else
    return TRUE;
}

int read_int_from_file(char *name) //Чтение числа из файла name
{
  FILE *file_descriptor=fopen(name,"rt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN %s FILE FOR READ!\n", name);
    #endif
    return 0;
  }
  else
  {
    char temp[256];
    if (fgets(temp, 256, file_descriptor) == 0)
    {
      fclose(file_descriptor);
      #ifdef debug_printf
      printf("Reading from %s failed!\n", name);
      #endif
      return 0;
    }
    fclose(file_descriptor);
    #ifdef debug_printf
    printf("Read '%s' from %s\n", temp, name);
    #endif
    return (atoi (temp));    
  }
}

void detect_hardware(void) // Обнаружение оборудования и его возможностей
{  
  if (!hardware_has_backlight) // Digma R60G/GMini C6LHD (Qt)
  {    
    hardware_has_backlight=check_for_file ("/sys/class/backlight/boeye_backlight/brightness");
    if (hardware_has_backlight) 
    {
      backlight_path="/sys/class/backlight/boeye_backlight/brightness";
      #ifdef debug_printf
      printf ("Found backlight control at file %s\n", backlight_path);
      #endif
      previous_backlight_level=read_int_from_file(backlight_path);
    }
  }
  
  if (!hardware_has_LED) // Digma R60G/GMini C6LHD (Qt)
  {
    hardware_has_LED=check_for_file ("/sys/class/leds/charger-led/brightness");
    if (hardware_has_LED) 
    {
      LED_path="/sys/class/leds/charger-led/brightness";
      #ifdef debug_printf
      printf ("Found LED control at file %s\n", LED_path);
      #endif
      LED_state[LED_ON]=2; // Неверно!
      LED_state[LED_OFF]=0;
      LED_state[LED_BLINK_SLOW]=1;
      LED_state[LED_BLINK_FAST]=4;
    }
  }
  
  if (!hardware_has_LED) // Ritmix RBK700HD GTK/Qt
  {
    hardware_has_LED=check_for_file ("/sys/class/leds/axp192-led-classdev/brightness");
    if (hardware_has_LED)
    {
      LED_path="/sys/class/leds/axp192-led-classdev/brightness";
      LED_state[LED_ON]=3;
      LED_state[LED_OFF]=4;
      LED_state[LED_BLINK_SLOW]=5;
      LED_state[LED_BLINK_FAST]=6;
      #ifdef debug_printf
      printf ("Found LED control at file %s\n", LED_path);
      #endif
    }
  }

  if (!hardware_has_LED) // Digma E600 GTK
  {
    hardware_has_LED=check_for_file ("/sys/devices/platform/boeye-leds/leds/da9030_led/brightness");
    if (hardware_has_LED)
    {
      LED_path="/sys/devices/platform/boeye-leds/leds/da9030_led/brightness";
      LED_state[LED_ON]=3; // Проверить!
      LED_state[LED_OFF]=4;
      LED_state[LED_BLINK_SLOW]=5;
      LED_state[LED_BLINK_FAST]=6;
      #ifdef debug_printf
      printf ("Found LED control at file %s\n", LED_path);
      #endif
    }
  }
  
  if (!hardware_has_APM)
  {
    hardware_has_APM=check_for_file ("/dev/apm_bios");
    if (hardware_has_APM)
    {      
      dlerror();    /* Clear any existing error */
      void *libapm_handle=dlopen("libapm.so", RTLD_NOW);
      char *error;
      if ((error = dlerror()) != 0)  
      {
        #ifdef debug_printf
        printf("dlopen failed because %s\n", error);
        #endif
        hardware_has_APM=FALSE; // Не можем управлять через APM, хотя существование файла в /dev/ даёт робкую надежду
      }
      dlerror();
      apm_suspend=dlsym(libapm_handle,"apm_suspend");
      if ((error = dlerror()) != NULL)  
      {
        #ifdef debug_printf
        printf("dlsym failed because %s\n", error);
        #endif
        hardware_has_APM=FALSE;
      }
    }
    
    if (!hardware_has_sysfs_sleep) 
    {    
      hardware_has_sysfs_sleep=check_for_file ("/sys/power/state");
      if (hardware_has_sysfs_sleep) 
      {
        sysfs_sleep_path="/sys/power/state";
        #ifdef debug_printf
        printf ("Found sysfs sleep trigger at file %s\n", sysfs_sleep_path);
        #endif
      }
    }
  }
  
  #ifdef debug_printf
  if (! hardware_has_LED)
    printf ("LED control not found\n");
  if (! hardware_has_backlight)
    printf ("Backlight control not found\n");
  if (! hardware_has_APM)
    printf ("APM not found\n");
  if (! hardware_has_sysfs_sleep)
    printf ("Sysfs sleep trigger not found\n");
  #endif
}

void write_int_to_file(char *file, int value)
{
  #ifdef debug_printf
  printf("writing %d to %s\n", value, file);
  #endif
  FILE *file_descriptor=fopen(file,"wt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN %s FILE FOR WRITING!\n", file);
    #endif
    return;
  }
  else
  {
    fprintf(file_descriptor, "%d", value);
    fclose(file_descriptor);
  }
}

void write_string_to_file(char *file, char *value)
{
  #ifdef debug_printf
  printf("writing %s to %s\n", value, file);
  #endif
  FILE *file_descriptor=fopen(file,"wt");
  if (!file_descriptor)
  {
    #ifdef debug_printf
    printf("UNABLE TO OPEN %s FILE FOR WRITING!\n", file);
    #endif
    return ;
  }
  else
  {
    fprintf(file_descriptor, "%s", value);
    fclose(file_descriptor);
  }
}

void set_brightness(int value)
{
  if (hardware_has_backlight)
    write_int_to_file(backlight_path, value);
}

void set_led_state (int state)
{
  if (LED_notify && hardware_has_LED)
    write_int_to_file(LED_path, state);
}

void suspend_hardware(void)
{
  fflush (stdout);
  sync();
  #ifdef debug_printf
  printf("Suspending hardware\n");
  #endif
  int count=0;
  do
  {
    set_led_state(LED_ON);
    time_t startTime = time(NULL);
    if (hardware_has_APM)
    {
      #ifdef debug_printf
      printf("Using APM\n");
      #endif
      int apm_bios=open("/dev/apm_bios", O_RDWR);
      (*apm_suspend) (apm_bios);
      close(apm_bios);
    }
    else if (hardware_has_sysfs_sleep)
    {
      #ifdef debug_printf
      printf("Using sysfs trigger\n");
      #endif
      write_string_to_file (sysfs_sleep_path,"mem");
    }
    else
    {
      #ifdef debug_printf
      printf("Unable to suspend hardware - no ways to suspend!\n");
      #endif
      break;
    }
    time_t endTime = time(NULL);
    double duration = difftime(endTime, startTime);
    if ((int) duration <= 1)
    {
      #ifdef debug_printf
      printf("failed\n");
      #endif
    }
    else 
    {
      #ifdef debug_printf
      printf("successed after %d attempts (sleeped %4.0f seconds)\n", count, duration);
      set_led_state(LED_OFF);
      #endif
      break;
    }
    #ifdef debug_printf
    set_led_state(LED_OFF);
    #endif
    if (count++>128)
    {
      #ifdef debug_printf
      printf("Too many failed attempts, break\n");
      break;
      #endif
    }
    usleep(100000);  
  }
  while (TRUE);
  sync();
}
